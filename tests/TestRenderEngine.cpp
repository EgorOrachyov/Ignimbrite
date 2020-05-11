/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                        */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#include <Material.h>
#include <MeshLoader.h>
#include <NoirFilter.h>
#include <InverseFilter.h>
#include <RenderEngine.h>
#include <RenderableMesh.h>
#include <VulkanExtensions.h>
#include <MaterialFullscreen.h>
#include <VulkanRenderDevice.h>
#include <VertexLayoutFactory.h>
#include <PresentationPass.h>

#include <fstream>
#include <stb_image.h>

using namespace ignimbrite;

struct Window {
#ifdef __APPLE__
    int32 w = 1280 / 2;
    int32 h = 720 / 2;
#else
    int32 w = 1280;
    int32 h = 720;
#endif
    GLFWwindow* handle;
    ID<IRenderDevice::Surface> surface;
    String name = "Render Engine Test";
    uint32 extensionsCount;
    const char** extensions;
};

class RenderEngineTest {
public:

    RenderEngineTest() {
        init();
    }

    ~RenderEngineTest() {
        shutdown();
    }

    void initWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        window.handle = glfwCreateWindow(window.w, window.h, window.name.c_str(), nullptr, nullptr);
        glfwGetFramebufferSize(window.handle, &window.w, &window.h);

        window.extensions = glfwGetRequiredInstanceExtensions(&window.extensionsCount);
    }

    void initDevice() {
        device = std::make_shared<VulkanRenderDevice>(window.extensionsCount, window.extensions, true);
        window.surface = VulkanExtensions::createSurfaceGLFW((VulkanRenderDevice&)*device, window.handle, window.w, window.h, window.name);
    }

    void initCamera() {
        Mat4f clip = glm::mat4(1.0f, 0.0f, 0.0f, 0.0f,
                               0.0f, -1.0f, 0.0f, 0.0f,
                               0.0f, 0.0f, 0.5f, 0.0f,
                               0.0f, 0.0f, 0.5f, 1.0f);

        camera = std::make_shared<Camera>();
        camera->setType(Camera::Type::Perspective);
        camera->setAspect((float32)window.w / (float32)window.h);
        camera->setPosition(glm::vec3(0,0, 3));
        camera->rotate(glm::vec3(0,1,0), M_PI);
        camera->setNearView(0.1f);
        camera->setFarView(100.0f);
        camera->setClipMatrix(clip);
        camera->recalculate();
    }

    void initLight() {
        Mat4f clip = glm::mat4(1.0f, 0.0f, 0.0f, 0.0f,
                               0.0f, -1.0f, 0.0f, 0.0f,
                               0.0f, 0.0f, 0.5f, 0.0f,
                               0.0f, 0.0f, 0.5f, 1.0f);

        light = std::make_shared<Light>();
        light->setType(Light::Type::Directional);
        light->setCastShadow(true);
        light->rotate(light->getRight(), -M_PI / 2);
        light->setClipMatrix(clip);
    }

    void initEngine() {
        engine = std::make_shared<RenderEngine>();
        engine->setRenderDevice(device);
        engine->setTargetSurface(window.surface);
        engine->setCamera(camera);
        engine->addLightSource(light);
        engine->setRenderArea(0, 0, window.w, window.h);

        auto presentationMaterial = MaterialFullscreen::fullscreenQuad(SHADERS_FOLDER_PATH, window.surface, device);
        auto presentationPass = std::make_shared<PresentationPass>(device, engine->getDefaultWhiteTexture(), presentationMaterial);
        presentationPass->enableDepthShow();
        engine->setPresentationPass(presentationPass);

        auto shadowTarget = std::make_shared<RenderTarget>(device);
        shadowTarget->createTargetFromFormat(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, RenderTarget::DefaultFormat::DepthStencil);

        auto sampler = std::make_shared<Sampler>(device);
        sampler->setHighQualityFiltering(SamplerRepeatMode::ClampToBorder);

        shadowTarget->getDepthStencilAttachment()->setSampler(sampler);
        engine->setShadowTarget(light, shadowTarget);
    }

    void initPostEffects() {
        //auto inverse = std::make_shared<InverseFilter>(device, SHADERS_FOLDER_PATH);
        //engine->addPostEffect(inverse);

        //auto noir = std::make_shared<NoirFilter>(device, SHADERS_FOLDER_PATH);
        //engine->addPostEffect(noir);
    }

    void initMeshMaterial() {
        // Shader
        std::ifstream vertFile(MODEL3D_SHADER_PATH_VERT.c_str(), std::ios::binary);
        std::ifstream fragFile(MODEL3D_SHADER_PATH_FRAG.c_str(), std::ios::binary);

        std::vector<uint8> vertSpv(std::istreambuf_iterator<char>(vertFile), {});
        std::vector<uint8> fragSpv(std::istreambuf_iterator<char>(fragFile), {});

        RefCounted<Shader> shader = std::make_shared<Shader>(device);
        shader->fromSources(ShaderLanguage::SPIRV, vertSpv, fragSpv);
        shader->reflectData();
        shader->generateUniformLayout();

        // PBR shader
        std::ifstream vertReflFile(MODEL3D_SHADER_PATH_VERT.c_str(), std::ios::binary);
        std::ifstream fragReflFile(MODEL3D_REFL_SHADER_PATH_FRAG.c_str(), std::ios::binary);

        std::vector<uint8> vertReflSpv(std::istreambuf_iterator<char>(vertReflFile), {});
        std::vector<uint8> fragReflSpv(std::istreambuf_iterator<char>(fragReflFile), {});

        RefCounted<Shader> reflShader = std::make_shared<Shader>(device);
        reflShader->fromSources(ShaderLanguage::SPIRV, vertReflSpv, fragReflSpv);
        reflShader->reflectData();
        reflShader->generateUniformLayout();

        // Shadow shader
        std::ifstream shVertFile(SHADOWS_SHADER_PATH_VERT.c_str(), std::ios::binary);
        std::ifstream shFragFile(SHADOWS_SHADER_PATH_FRAG.c_str(), std::ios::binary);

        std::vector<uint8> shVertSpv(std::istreambuf_iterator<char>(shVertFile), {});
        std::vector<uint8> shFragSpv(std::istreambuf_iterator<char>(shFragFile), {});

        RefCounted<Shader> shadowShader = std::make_shared<Shader>(device);
        shadowShader->fromSources(ShaderLanguage::SPIRV, shVertSpv, shFragSpv);
        shadowShader->reflectData();
        shadowShader->generateUniformLayout();

        // Pipeline
        IRenderDevice::VertexBufferLayoutDesc vertexBufferLayoutDesc = {};
        VertexLayoutFactory::createVertexLayoutDesc(Mesh::VertexFormat::PNTTB, vertexBufferLayoutDesc);

        RefCounted<GraphicsPipeline> pipeline = std::make_shared<GraphicsPipeline>(device);
        pipeline->setTargetFormat(engine->getOffscreenTargetFormat());
        pipeline->setShader(shader);
        pipeline->setVertexBuffersCount(1);
        pipeline->setVertexBufferDesc(0, vertexBufferLayoutDesc);
        pipeline->setDepthTestEnable(true);
        pipeline->setDepthWriteEnable(true);
        //pipeline->setPolygonMode(PolygonMode::Line);
        pipeline->createPipeline();

        RefCounted<GraphicsPipeline> pbrPipeline = std::make_shared<GraphicsPipeline>(device);
        pbrPipeline->setTargetFormat(engine->getOffscreenTargetFormat());
        pbrPipeline->setShader(reflShader);
        pbrPipeline->setVertexBuffersCount(1);
        pbrPipeline->setVertexBufferDesc(0, vertexBufferLayoutDesc);
        pbrPipeline->setDepthTestEnable(true);
        pbrPipeline->setDepthWriteEnable(true);
        pbrPipeline->createPipeline();

        // Sampler
        RefCounted<Sampler> sampler = std::make_shared<Sampler>(device);
        sampler->setHighQualityFiltering();

        // Material
        material = std::make_shared<Material>(device);
        material->setGraphicsPipeline(pbrPipeline);
        material->createMaterial();

        setMaterialTexture(TEXTURE_ALBEDO_PATH.c_str(), "texAlbedo", material, sampler);
        setMaterialTexture(TEXTURE_EMISSIVE_PATH.c_str(), "texEmmisive", material, sampler);
        setMaterialTexture(TEXTURE_AO_PATH.c_str(), "texAO", material, sampler);
        setMaterialTexture(TEXTURE_METALROUGH_PATH.c_str(), "texMetalRough", material, sampler);
        setMaterialTexture(TEXTURE_NORMAL_PATH.c_str(), "texNormal", material, sampler);
        setMaterialCubemap("texEnvMap", material, sampler);

        material->setTexture("texShadowMap", engine->getDefaultWhiteTexture());
        material->updateUniformData();

        whiteMaterial = std::make_shared<Material>(device);
        whiteMaterial->setGraphicsPipeline(pipeline);
        whiteMaterial->createMaterial();
        whiteMaterial->setTexture("texShadowMap", engine->getDefaultWhiteTexture());
        whiteMaterial->updateUniformData();

        IRenderDevice::VertexBufferLayoutDesc vertShadowLayoutDesc = {};
        vertShadowLayoutDesc.stride = Mesh::getSizeOfStride(Mesh::VertexFormat::PNTTB);
        vertShadowLayoutDesc.usage = VertexUsage::PerVertex;
        vertShadowLayoutDesc.attributes.push_back({0, 0, DataFormat::R32G32B32_SFLOAT});

        RefCounted<GraphicsPipeline> shadowsPipeline = std::make_shared<GraphicsPipeline>(device);
        shadowsPipeline->setTargetFormat(engine->getShadowTargetFormat());
        shadowsPipeline->setShader(shadowShader);
        shadowsPipeline->setPolygonCullMode(PolygonCullMode::Back);
        shadowsPipeline->setDepthTestEnable(true);
        shadowsPipeline->setDepthWriteEnable(true);
        shadowsPipeline->setDepthCompareOp(CompareOperation::LessOrEqual);
        shadowsPipeline->setVertexBuffersCount(1);
        shadowsPipeline->setVertexBufferDesc(0, vertShadowLayoutDesc);
        shadowsPipeline->createPipeline();

        shadowMaterial = std::make_shared<Material>(device);
        shadowMaterial->setGraphicsPipeline(shadowsPipeline);
        shadowMaterial->createMaterial();
    }

    void setMaterialTexture(const char *path, const char *name, RefCounted<Material> mt, RefCounted<Sampler> sampler) {
        int w, h, channels;
        stbi_uc* pixels = stbi_load(path, &w, &h, &channels, STBI_rgb_alpha);
        if (pixels != nullptr) {
            RefCounted<Texture> texture = std::make_shared<Texture>(device);
            texture->setSampler(sampler);
            texture->setDataAsRGBA8(w, h, pixels, true);
            mt->setTexture(name, texture);
        } else {
            mt->setTexture(name, engine->getDefaultWhiteTexture());
        }

        stbi_image_free(pixels);
    }

    void setMaterialCubemap(const char *name, RefCounted<Material> mt, RefCounted<Sampler> sampler) {
        RefCounted<Texture> texture = std::make_shared<Texture>(device);
        texture->setSampler(sampler);

        const char *paths[] = {
                SKYBOX_PX_PATH.c_str(),
                SKYBOX_NX_PATH.c_str(),
                SKYBOX_PY_PATH.c_str(),
                SKYBOX_NY_PATH.c_str(),
                SKYBOX_PZ_PATH.c_str(),
                SKYBOX_NZ_PATH.c_str()
        };

        uint8 *data = nullptr;
        uint32 offset = 0;
        int w = 0, h = 0, channels = 0;

        for (int i = 0; i < 6; i++) {
            stbi_uc* pixels = stbi_load(paths[i], &w, &h, &channels, STBI_rgb_alpha);
            uint32 size = w * h * 4;

            if (data == nullptr) {
                // 6 textures
                data = new uint8[6 * w * h * 4];
            }

            memcpy(data + offset, pixels, size);
            offset += size;

            stbi_image_free(pixels);
        }

        texture->setDataAsCubemapRGBA8(w, h, data, true);

        mt->setTexture(name, texture);

        delete[] data;
    }

    void initMesh() {
        MeshLoader loader(MESH_PATH);
        RefCounted<Mesh> data = loader.importMesh(Mesh::VertexFormat::PNTTB);

        for (int32 x = -MESH_COUNT_X2; x <= MESH_COUNT_X2; x++) {
            for (int32 z = -MESH_COUNT_Z2; z <= MESH_COUNT_Z2; z++) {
                RefCounted<RenderableMesh> mesh = std::make_shared<RenderableMesh>();
                RefCounted<Material> mat = material->clone();
                RefCounted<Material> shadowMat = shadowMaterial->clone();

                mesh->setRenderDevice(device);
                mesh->setRenderMesh(data);
                mesh->setRenderMaterial(mat);
                mesh->setShadowRenderMesh(data);
                mesh->setShadowRenderMaterial(shadowMat);
                mesh->setCastShadows();
                mesh->translate(Vec3f(x * MESH_STEP, 0.0f, z * MESH_STEP));
                mesh->create();
                mesh->setVisible(true);
                mesh->setCanApplyCulling(true);
                mesh->setLayerID((uint32) IRenderable::DefaultLayers::Solid);
                mesh->setMaxViewDistance(50.0f);

                //mesh->rotate({1,0,0}, -M_PI / 2);
                mesh->setScale({2, 2, 2});

                engine->addRenderable(mesh);
                meshes.push_back(mesh);

                auto Rx = (float32)rand()/(float32)RAND_MAX - 0.5f;
                auto Ry = (float32)rand()/(float32)RAND_MAX - 0.5f;
                auto Rz = (float32)rand()/(float32)RAND_MAX - 0.5f;
                auto Ra = (float32)rand()/(float32)RAND_MAX - 0.5f;

                rotations.push_back(Vec4f(Rx, Ry, Rz, Ra));
            }
        }

        MeshLoader planeMeshLoader(MESH_PLANE_PATH);
        RefCounted<Mesh> planeMeshData = planeMeshLoader.importMesh(Mesh::VertexFormat::PNTTB);
        RefCounted<Material> mat = whiteMaterial->clone();
        RefCounted<Material> shadowMat = shadowMaterial->clone();

        RefCounted<RenderableMesh> planeMesh = std::make_shared<RenderableMesh>();
        planeMesh->setRenderDevice(device);
        planeMesh->setRenderMesh(planeMeshData);
        planeMesh->setRenderMaterial(mat);
        planeMesh->setShadowRenderMesh(planeMeshData);
        planeMesh->setShadowRenderMaterial(shadowMat);
        planeMesh->translate(Vec3f(0, -2, 0));
        planeMesh->create();
        planeMesh->setVisible(true);
        planeMesh->setCanApplyCulling(true);
        planeMesh->setLayerID((uint32) IRenderable::DefaultLayers::Solid);
        planeMesh->setMaxViewDistance(200.0f);

        engine->addRenderable(planeMesh);
    }

    void inputUpdate() {
        if (glfwGetKey(window.handle, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window.handle, true);

        float32 movementSpeed = 2.0f / 60.0f;
        float32 rotationSpeed = 1.0f / 60.0f;
        if (glfwGetKey(window.handle, GLFW_KEY_W) == GLFW_PRESS)
            camera->move(movementSpeed * camera->getDirection());
        if (glfwGetKey(window.handle, GLFW_KEY_S) == GLFW_PRESS)
            camera->move(-movementSpeed * camera->getDirection());
        if (glfwGetKey(window.handle, GLFW_KEY_A) == GLFW_PRESS)
            camera->move(-camera->getRight() * movementSpeed);
        if (glfwGetKey(window.handle, GLFW_KEY_D) == GLFW_PRESS)
            camera->move(camera->getRight() * movementSpeed);
        if (glfwGetKey(window.handle, GLFW_KEY_Q) == GLFW_PRESS)
            camera->move(-camera->getUp() * movementSpeed);
        if (glfwGetKey(window.handle, GLFW_KEY_E) == GLFW_PRESS)
            camera->move(camera->getUp() * movementSpeed);
        if (glfwGetKey(window.handle, GLFW_KEY_UP) == GLFW_PRESS)
            camera->rotate(camera->getRight(), rotationSpeed);
        if (glfwGetKey(window.handle, GLFW_KEY_DOWN) == GLFW_PRESS)
            camera->rotate(camera->getRight(), -rotationSpeed);
        if (glfwGetKey(window.handle, GLFW_KEY_LEFT) == GLFW_PRESS)
            camera->rotate(glm::vec3(0, 1, 0), rotationSpeed);
        if (glfwGetKey(window.handle, GLFW_KEY_RIGHT) == GLFW_PRESS)
            camera->rotate(glm::vec3(0, 1, 0), -rotationSpeed);

        if (glfwGetKey(window.handle, GLFW_KEY_T) == GLFW_PRESS)
            light->rotate(light->getRight(), rotationSpeed);
        if (glfwGetKey(window.handle, GLFW_KEY_G) == GLFW_PRESS)
            light->rotate(light->getRight(), -rotationSpeed);
        if (glfwGetKey(window.handle, GLFW_KEY_F) == GLFW_PRESS)
            light->rotate(glm::vec3(0, 1, 0), rotationSpeed);
        if (glfwGetKey(window.handle, GLFW_KEY_H) == GLFW_PRESS)
            light->rotate(glm::vec3(0, 1, 0), -rotationSpeed);
        
        camera->recalculate();
    }

    void meshUpdate() {
        for (uint32 i = 0; i < meshes.size(); i++) {
            auto& m = meshes[i];
            auto& R = rotations[i];
            //m->rotate(glm::vec3(R.x, R.y, R.z), 0.02f * R.w);
            m->updateAABB();
        }
    }

    void init() {
        initWindow();
        initDevice();
        initCamera();
        initLight();
        initEngine();
        initPostEffects();
        initMeshMaterial();
        initMesh();
    }

    void run() {
        while (!glfwWindowShouldClose(window.handle)) {
            glfwPollEvents();
            glfwSwapBuffers(window.handle);
            inputUpdate();
            meshUpdate();

            engine->addLine3d({0, 0, 0}, light->getDirection(), {1, 1, 0, 1}, 2);
            engine->addLine3d({0, 0, 0}, light->getRight(), {1, 0, 0, 1}, 2);
            engine->addLine3d({0, 0, 0}, light->getUp(), {0, 1, 0, 1}, 2);

            engine->draw();
        }
    }

    void shutdown() {
        VulkanExtensions::destroySurface((VulkanRenderDevice&)*device, window.surface);
    }

private:
    Window window;
    RefCounted<IRenderEngine>  engine;
    RefCounted<IRenderDevice>  device;
    RefCounted<Camera>         camera;
    RefCounted<Light>          light;
    RefCounted<Material>       material;
    RefCounted<Material>       whiteMaterial;
    RefCounted<Material>       shadowMaterial;
    RefCounted<Canvas>         canvas;

    std::vector<RefCounted<RenderableMesh>> meshes;
    std::vector<Vec4f>                      rotations;

    const uint32 SHADOW_MAP_SIZE = 4096;

    const int32 MESH_COUNT_X2 = 0;
    const int32 MESH_COUNT_Z2 = 0;
    const int32 MESH_STEP     = 2;

    String MODEL3D_SHADER_PATH_VERT = "shaders/spirv/shadowmapping/MeshShadowed.vert.spv";
    String MODEL3D_SHADER_PATH_FRAG = "shaders/spirv/shadowmapping/MeshShadowed.frag.spv";
    String MODEL3D_REFL_SHADER_PATH_FRAG = "shaders/spirv/shadowmapping/MeshReflectiveShadowed.frag.spv";
    String SHADOWS_SHADER_PATH_VERT = "shaders/spirv/shadowmapping/Shadows.vert.spv";
    String SHADOWS_SHADER_PATH_FRAG = "shaders/spirv/shadowmapping/Shadows.frag.spv";
    String SHADERS_FOLDER_PATH = "shaders/spirv/";

    String MESH_PATH                = "assets/models/DamagedHelmet.obj";
    String MESH_PLANE_PATH          = "assets/models/plane.obj";

    String TEXTURE_ALBEDO_PATH      = "assets/textures/DamagedHelmet_Albedo.jpg";
    String TEXTURE_EMISSIVE_PATH    = "assets/textures/DamagedHelmet_Emissive.jpg";
    String TEXTURE_AO_PATH          = "assets/textures/DamagedHelmet_AO.jpg";
    String TEXTURE_METALROUGH_PATH  = "assets/textures/DamagedHelmet_MetalRoughness.jpg";
    String TEXTURE_NORMAL_PATH      = "assets/textures/DamagedHelmet_Normal.jpg";

    String SKYBOX_PX_PATH       = "assets/textures/SkyboxPX.jpg";
    String SKYBOX_NX_PATH       = "assets/textures/SkyboxNX.jpg";
    String SKYBOX_PY_PATH       = "assets/textures/SkyboxPY.jpg";
    String SKYBOX_NY_PATH       = "assets/textures/SkyboxNY.jpg";
    String SKYBOX_PZ_PATH       = "assets/textures/SkyboxPZ.jpg";
    String SKYBOX_NZ_PATH       = "assets/textures/SkyboxNZ.jpg";


};

int main() {
    RenderEngineTest test;
    test.run();
    return 0;
}