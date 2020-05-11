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
#include <FileUtils.h>

#include <fstream>
#include <stb_image.h>

using namespace ignimbrite;

static const int W = 1920;
static const int H = 1280;

struct Window {
#ifdef __APPLE__
    int32 w = W / 2;
    int32 h = H / 2;
#else
    int32 w = W;
    int32 h = H;
#endif
    GLFWwindow* handle;
    ID<IRenderDevice::Surface> surface;
    String name = "PBR Helmet Test";
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
        device = std::make_shared<VulkanRenderDevice>(window.extensionsCount, window.extensions);
        window.surface = VulkanExtensions::createSurfaceGLFW((VulkanRenderDevice&)*device, window.handle, window.w, window.h, window.name);
    }

    void initCamera() {
        Mat4f clip = glm::mat4(1.0f, 0.0f, 0.0f, 0.0f,
                               0.0f, -1.0f, 0.0f, 0.0f,
                               0.0f, 0.0f, 0.5f, 0.0f,
                               0.0f, 0.0f, 0.5f, 1.0f);

        camera = std::make_shared<Camera>();
        camera->setType(Camera::Type::Perspective);
        camera->setFov(glm::radians(40.0f));
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
        light->rotate(light->getRight(), -M_PI / 2.0f);
        light->setClipMatrix(clip);
    }

    void initEngine() {
        engine = std::make_shared<RenderEngine>();
        engine->setRenderDevice(device);
        engine->setTargetSurface(window.surface);
        engine->setCamera(camera);
        engine->addLightSource(light);
        engine->setRenderArea(0, 0, window.w, window.h);

        auto presentationPass = MaterialFullscreen::fullscreenQuad(SHADERS_FOLDER_PATH, window.surface, device);
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

        // Sampler
        RefCounted<Sampler> sampler = std::make_shared<Sampler>(device);
        sampler->setHighQualityFiltering();

        defaultShadowTexture = std::make_shared<Texture>(device);
        defaultShadowTexture->setDataAsRGBA8(1, 1, blackPixel, true);
        defaultShadowTexture->setSampler(sampler);

        whiteMaterial = std::make_shared<Material>(device);
        whiteMaterial->setGraphicsPipeline(pipeline);
        whiteMaterial->createMaterial();
        whiteMaterial->setTexture2D("texShadowMap", defaultShadowTexture);
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
        RefCounted<Texture> texture = std::make_shared<Texture>(device);
        texture->setSampler(sampler);

        int w, h, channels;
        stbi_uc* pixels = stbi_load(path, &w, &h, &channels, STBI_rgb_alpha);
        if (pixels != nullptr) {
            texture->setDataAsRGBA8(w, h, pixels, true);
        } else {
            texture->setDataAsRGBA8(1, 1, whitePixel, true);
        }

        mt->setTexture2D(name, texture);

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

        mt->setTexture2D(name, texture);

        delete[] data;
    }

    void initMesh() {
//        MeshLoader loader(MESH_PATH);
//        RefCounted<Mesh> data = loader.importMesh(Mesh::VertexFormat::PNTTB);
//
//        for (int32 x = -MESH_COUNT_X2; x <= MESH_COUNT_X2; x++) {
//            for (int32 z = -MESH_COUNT_Z2; z <= MESH_COUNT_Z2; z++) {
//                RefCounted<RenderableMesh> mesh = std::make_shared<RenderableMesh>();
//                RefCounted<Material> mat = material->clone();
//                RefCounted<Material> shadowMat = shadowMaterial->clone();
//
//                mesh->setRenderDevice(device);
//                mesh->setRenderMesh(data);
//                mesh->setRenderMaterial(mat);
//                mesh->setShadowRenderMesh(data);
//                mesh->setShadowRenderMaterial(shadowMat);
//                mesh->setCastShadows();
//                mesh->translate(Vec3f(x * MESH_STEP, 0.0f, z * MESH_STEP));
//                mesh->create();
//                mesh->setVisible(true);
//                mesh->setCanApplyCulling(true);
//                mesh->setLayerID((uint32) IRenderable::DefaultLayers::Solid);
//                mesh->setMaxViewDistance(50.0f);
//
//                //mesh->rotate({1,0,0}, -M_PI / 2);
//                mesh->setScale({2, 2, 2});
//
//                engine->addRenderable(mesh);
//                meshes.push_back(mesh);
//
//                auto Rx = (float32)rand()/(float32)RAND_MAX - 0.5f;
//                auto Ry = (float32)rand()/(float32)RAND_MAX - 0.5f;
//                auto Rz = (float32)rand()/(float32)RAND_MAX - 0.5f;
//                auto Ra = (float32)rand()/(float32)RAND_MAX - 0.5f;
//
//                rotations.push_back(Vec4f(Rx, Ry, Rz, Ra));
//            }
//        }

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

    //////////////////////////////////////////////////////////////////

    RefCounted<Texture> loadTexture(const String& path, const RefCounted<Sampler> &sampler) {
        RefCounted<Texture> texture = std::make_shared<Texture>(device);
        texture->setSampler(sampler);

        int w, h, channels;
        stbi_uc* pixels = stbi_load(path.c_str(), &w, &h, &channels, STBI_rgb_alpha);
        if (pixels != nullptr) {
            texture->setDataAsRGBA8(w, h, pixels, true);
            std::cout << "Load texture " << path << '\n';
        } else {
            texture->setDataAsRGBA8(1, 1, blackPixel, true);
            std::cout << "Failed to load texture " << path << '\n';
        }

        stbi_image_free(pixels);
        return texture;
    }

    void initPbrMaterial() {
        // Shader
        std::vector<uint8> pbrVertSourceCode;
        std::vector<uint8> pbrFragSourceCode;

        FileUtils::loadBinary(SHADER_PBR_VERT, pbrVertSourceCode);
        FileUtils::loadBinary(SHADER_PBR_FRAG, pbrFragSourceCode);

        auto pbrShader = std::make_shared<Shader>(device);
        pbrShader->fromSources(ShaderLanguage::SPIRV, pbrVertSourceCode, pbrFragSourceCode);
        pbrShader->reflectData();
        pbrShader->generateUniformLayout();

        // Sampler
        RefCounted<Sampler> sampler = std::make_shared<Sampler>(device);
        sampler->setHighQualityFiltering();

        // Pipeline
        IRenderDevice::VertexBufferLayoutDesc vertexBufferLayoutDesc = {};
        VertexLayoutFactory::createVertexLayoutDesc(Mesh::VertexFormat::PNTTB, vertexBufferLayoutDesc);

        RefCounted<GraphicsPipeline> pbrPipeline = std::make_shared<GraphicsPipeline>(device);
        pbrPipeline->setTargetFormat(engine->getOffscreenTargetFormat());
        pbrPipeline->setShader(pbrShader);
        pbrPipeline->setVertexBuffersCount(1);
        pbrPipeline->setVertexBufferDesc(0, vertexBufferLayoutDesc);
        pbrPipeline->setDepthTestEnable(true);
        pbrPipeline->setDepthWriteEnable(true);
        pbrPipeline->createPipeline();

        pbrMaterial = std::make_shared<Material>(device);
        pbrMaterial->setGraphicsPipeline(pbrPipeline);
        pbrMaterial->createMaterial();

        auto albedo = loadTexture(TEXTURE_HELMET_ALBEDO, sampler);
        auto ao = loadTexture(TEXTURE_HELMET_AO, sampler);
        auto metalroughness = loadTexture(TEXTURE_HELMET_METALROUGHNESS, sampler);
        auto normal = loadTexture(TEXTURE_HELMET_NORMAL, sampler);
        auto emissive = loadTexture(TEXTURE_HELMET_EMISSIVE, sampler);

        pbrMaterial->setTexture2D("texShadowMap", defaultShadowTexture);
        pbrMaterial->setTexture2D("texAlbedo", albedo);
        pbrMaterial->setTexture2D("texAO", ao);
        pbrMaterial->setTexture2D("texMetalRoughness", metalroughness);
        pbrMaterial->setTexture2D("texNormal", normal);
        pbrMaterial->setTexture2D("texEmissive", emissive);
        pbrMaterial->updateUniformData();
    }

    void initPbrRenderMesh() {

        class PbrMesh : public RenderableMesh {
        public:

            void onRender(const IRenderContext &context) override {
                auto device = context.getRenderDevice();
                auto camera = context.getCamera();
                auto light = context.getGlobalLight();

                auto model = glm::translate(mWorldPosition) * mRotation * glm::scale(mScale);
                auto camViewProj = camera->getViewProjClipMatrix();

                if (light != nullptr) {
                    auto lightViewProj = light->getViewProjClipMatrix();

                    mRenderMaterial->setTexture2D("texShadowMap", context.getShadowMap());
                    mRenderMaterial->setMat4("CommonParams.lightSpace", lightViewProj);
                    mRenderMaterial->setVec3("CommonParams.lightDir", light->getDirection());
                }

                mRenderMaterial->setMat4("CommonParams.viewProj", camViewProj);
                mRenderMaterial->setMat4("CommonParams.model", model);
                mRenderMaterial->setVec3("CommonParams.cameraPos", camera->getPosition());

                mRenderMaterial->updateUniformData();
                mRenderMaterial->bindGraphicsPipeline();
                mRenderMaterial->bindUniformData();

                device->drawListBindVertexBuffer(mVertexBuffer, 0, 0);
                device->drawListBindIndexBuffer(mIndexBuffer, IndicesType::Uint32, 0);
                device->drawListDrawIndexed(mRenderMesh->getIndicesCount(), 1);
            }

        };

        MeshLoader loader(MESH_PATH);
        RefCounted<Mesh> data = loader.importMesh(Mesh::VertexFormat::PNTTB);

        RefCounted<PbrMesh> mesh = std::make_shared<PbrMesh>();
        RefCounted<Material> mat = pbrMaterial->clone();
        RefCounted<Material> shadowMat = shadowMaterial->clone();

        mesh->setRenderDevice(device);
        mesh->setRenderMesh(data);
        mesh->setRenderMaterial(mat);
        mesh->setShadowRenderMesh(data);
        mesh->setShadowRenderMaterial(shadowMat);
        mesh->setCastShadows();
        mesh->create();
        mesh->setVisible(true);
        mesh->setCanApplyCulling(true);
        mesh->setLayerID((uint32) IRenderable::DefaultLayers::Solid);
        mesh->setMaxViewDistance(200.0f);
        mesh->setScale({1, 1, 1});

        engine->addRenderable(mesh);

        pbrMesh = mesh;
    }

    //////////////////////////////////////////////////////////////////

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
        initPbrMaterial();
        initPbrRenderMesh();
    }

    void run() {
        while (!glfwWindowShouldClose(window.handle)) {
            glfwPollEvents();
            glfwSwapBuffers(window.handle);
            inputUpdate();
            meshUpdate();

            pbrMesh->rotate({0,1,0}, 0.002f);
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
    RefCounted<Material>       whiteMaterial;
    RefCounted<Material>       shadowMaterial;

    std::vector<RefCounted<RenderableMesh>> meshes;
    std::vector<Vec4f>                      rotations;

    const uint8_t whitePixel[4] = {255, 255, 255, 255};
    const uint8_t blackPixel[4] = {0, 0, 0, 0};

    const uint32 SHADOW_MAP_SIZE = 4096;

    String MODEL3D_SHADER_PATH_VERT = "shaders/spirv/shadowmapping/MeshShadowed.vert.spv";
    String MODEL3D_SHADER_PATH_FRAG = "shaders/spirv/shadowmapping/MeshShadowed.frag.spv";
    String SHADOWS_SHADER_PATH_VERT = "shaders/spirv/shadowmapping/Shadows.vert.spv";
    String SHADOWS_SHADER_PATH_FRAG = "shaders/spirv/shadowmapping/Shadows.frag.spv";
    String SHADERS_FOLDER_PATH = "shaders/spirv/";

    String MESH_PLANE_PATH          = "assets/models/plane.obj";

    String SKYBOX_PX_PATH       = "assets/textures/SkyboxPX.jpg";
    String SKYBOX_NX_PATH       = "assets/textures/SkyboxNX.jpg";
    String SKYBOX_PY_PATH       = "assets/textures/SkyboxPY.jpg";
    String SKYBOX_NY_PATH       = "assets/textures/SkyboxNY.jpg";
    String SKYBOX_PZ_PATH       = "assets/textures/SkyboxPZ.jpg";
    String SKYBOX_NZ_PATH       = "assets/textures/SkyboxNZ.jpg";

    //////////////// Helmet Assets and shaders ////////////////

    String MESH_PATH                = "assets/models/DamagedHelmet.obj";

    String TEXTURE_HELMET_ALBEDO         = "assets/textures/DamagedHelmet_Albedo.jpg";
    String TEXTURE_HELMET_AO             = "assets/textures/DamagedHelmet_AO.jpg";
    String TEXTURE_HELMET_EMISSIVE       = "assets/textures/DamagedHelmet_Emissive.jpg";
    String TEXTURE_HELMET_METALROUGHNESS = "assets/textures/DamagedHelmet_MetalRoughness.jpg";
    String TEXTURE_HELMET_NORMAL         = "assets/textures/DamagedHelmet_Normal.jpg";

    String SHADER_PBR_VERT = "shaders/spirv/pbr/PBRShadowed.vert.spv";
    String SHADER_PBR_FRAG = "shaders/spirv/pbr/PBRShadowed.frag.spv";

    RefCounted<RenderableMesh> pbrMesh;
    RefCounted<Material> pbrMaterial;
    RefCounted<Texture> defaultShadowTexture;
};

int main() {
    RenderEngineTest test;
    test.run();
    return 0;
}