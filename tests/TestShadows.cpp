/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                       */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                */
/**********************************************************************************/

#include <algorithm>
#include <fstream>
#include <VulkanExtensions.h>
#include <VulkanRenderDevice.h>
#include <Light.h>
#include <Material.h>
#include <GraphicsPipeline.h>
#include <MeshLoader.h>
#include <Camera.h>
#include <RenderTarget.h>

using namespace ignimbrite;

struct Window {
#ifdef __APPLE__
    int32 w = 1280 / 2;
    int32 h = 720 / 2;
#else
    int32 w = 1280;
    int32 h = 720;
#endif
    GLFWwindow *handle;
    ID<IRenderDevice::Surface> surface;
    String name = "Shadowmapping Test";
    uint32 extensionsCount;
    const char **extensions;
};

struct Vertex {
    float32 p[3];
    float32 n[3];
    float32 t[2];
};

struct ShadowsPass {
    RefCounted<Texture> depthTexture;
    RefCounted<RenderTarget> renderTarget;
    static const uint32 width = 2048;
    static const uint32 height = 2048;
};

struct RenderableMesh {
    ID<IRenderDevice::VertexBuffer> vertexBuffer;
    ID<IRenderDevice::IndexBuffer> indexBuffer;
    uint32 indexCount = 0;
    RefCounted<Material> material;
    RefCounted<Material> shadowMaterial;
    Mat4f transform = Mat4f(1.0f);
};

struct Scene {
    Camera camera;
    Light light;
    std::vector<RefCounted<RenderableMesh>> meshes;
};

class TestShadows {
public:

    TestShadows() {
        init();
    }

    ~TestShadows() {
        shutdown();
    }

    void run() {
        std::vector<IRenderDevice::Color> clearFbColors;

        while (!glfwWindowShouldClose(window.handle)) {
            glfwPollEvents();
            glfwSwapBuffers(window.handle);
            glfwGetFramebufferSize(window.handle, &window.w, &window.h);

            IRenderDevice::Region area = {0, 0, {(uint32) window.w, (uint32) window.h}};
            IRenderDevice::Region areaShadows = {0, 0, {ShadowsPass::width, ShadowsPass::height}};

            if (area.extent.x == 0 || area.extent.y == 0) {
                continue;
            }

            updateScene();

            device->drawListBegin();
            {
                device->drawListBindFramebuffer(shadowPass.renderTarget->getHandle(), clearFbColors, areaShadows);

                for (auto &mesh : scene->meshes) {
                    mesh->shadowMaterial->bindGraphicsPipeline();
                    mesh->shadowMaterial->bindUniformData();

                    device->drawListBindVertexBuffer(mesh->vertexBuffer, 0, 0);
                    device->drawListBindIndexBuffer(mesh->indexBuffer, ignimbrite::IndicesType::Uint32, 0);
                    device->drawListDrawIndexed(mesh->indexCount, 1);
                }
            }
            {
                float d = glm::dot(scene->light.getDirection(), glm::vec3(0,-1,0));
                IRenderDevice::Color clearSurfColor = {{0.8f * d, 0.95f * d, 1.0f * d, 0.0f}};

                device->drawListBindSurface(window.surface, clearSurfColor, area);

                for (auto &mesh : scene->meshes) {
                    mesh->material->bindGraphicsPipeline();
                    mesh->material->bindUniformData();

                    device->drawListBindVertexBuffer(mesh->vertexBuffer, 0, 0);
                    device->drawListBindIndexBuffer(mesh->indexBuffer, ignimbrite::IndicesType::Uint32, 0);
                    device->drawListDrawIndexed(mesh->indexCount, 1);
                }
            }
            device->drawListEnd();

            device->flush();
            device->synchronize();
            device->swapBuffers(window.surface);
        }
    }

private:

    void init() {
        initWindow();
        initDevice();
        initShadowPass();
        initMeshPass();
        initScene();
    }

    void initDevice() {
        device = std::make_shared<VulkanRenderDevice>(window.extensionsCount, window.extensions);
        window.surface = VulkanExtensions::createSurfaceGLFW(
                (VulkanRenderDevice &) *device, window.handle, window.w, window.h, window.name);

        // vulkan uses inverted y axis and half of z axis (for OpenGL use identity)
        clipMatrix = glm::mat4(
                1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, -1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 0.5f, 0.0f,
                0.0f, 0.0f, 0.5f, 1.0f);
    }

    void initShadowPass() {
        shadowsShader = loadShader(shadowsVertPath, shadowsFragPath);

        shadowPass.renderTarget = std::make_shared<RenderTarget>(device);
        shadowPass.renderTarget->createTargetFromFormat(
                ShadowsPass::width, ShadowsPass::height,
                RenderTarget::DefaultFormat::DepthStencil);
        shadowPass.depthTexture = shadowPass.renderTarget->getDepthStencilAttachment();

        RefCounted<Sampler> sampler = std::make_shared<Sampler>(device);
        sampler->setHighQualityFiltering(SamplerRepeatMode::ClampToBorder);
        shadowPass.depthTexture->setSampler(sampler);

        IRenderDevice::VertexBufferLayoutDesc desc = {};
        desc.stride = sizeof(Vertex);
        desc.usage = VertexUsage::PerVertex;
        desc.attributes.push_back({0, offsetof(Vertex, p), DataFormat::R32G32B32_SFLOAT});

        shadowsPipeline = std::make_shared<GraphicsPipeline>(device);
        shadowsPipeline->setTargetFormat(shadowPass.renderTarget->getFramebufferFormat());
        shadowsPipeline->setShader(shadowsShader);
        shadowsPipeline->setPolygonCullMode(PolygonCullMode::Front);

        shadowsPipeline->setDepthTestEnable(true);
        shadowsPipeline->setDepthWriteEnable(true);
        shadowsPipeline->setDepthCompareOp(CompareOperation::LessOrEqual);

        shadowsPipeline->setVertexBuffersCount(1);
        shadowsPipeline->setVertexBufferDesc(0, desc);

        shadowsPipeline->createPipeline();
    }

    void initMeshPass() {
        meshShader = loadShader(meshVertPath, meshFragPath);

        IRenderDevice::VertexBufferLayoutDesc desc = {};
        desc.stride = sizeof(Vertex);
        desc.usage = VertexUsage::PerVertex;
        desc.attributes.push_back({0, offsetof(Vertex, p), DataFormat::R32G32B32_SFLOAT});
        desc.attributes.push_back({1, offsetof(Vertex, n), DataFormat::R32G32B32_SFLOAT});
        desc.attributes.push_back({2, offsetof(Vertex, t), DataFormat::R32G32_SFLOAT});

        meshPipeline = std::make_shared<GraphicsPipeline>(device);
        meshPipeline->setSurface(window.surface);
        meshPipeline->setShader(meshShader);

        meshPipeline->setDepthTestEnable(true);
        meshPipeline->setDepthWriteEnable(true);
        meshPipeline->setDepthCompareOp(CompareOperation::LessOrEqual);

        meshPipeline->setVertexBuffersCount(1);
        meshPipeline->setVertexBufferDesc(0, desc);

        meshPipeline->createPipeline();
    }

    RefCounted<Shader> loadShader(const String &vertPath, const String &fragPath) {
        std::ifstream vertFile(vertPath.c_str(), std::ios::binary);
        std::ifstream fragFile(fragPath.c_str(), std::ios::binary);

        std::vector<uint8> vertSpv(std::istreambuf_iterator<char>(vertFile), {});
        std::vector<uint8> fragSpv(std::istreambuf_iterator<char>(fragFile), {});

        RefCounted<Shader> result = std::make_shared<Shader>(device);
        result->fromSources(ShaderLanguage::SPIRV, vertSpv, fragSpv);
        result->reflectData();
        result->generateUniformLayout();

        return result;
    }

    void initScene() {
        scene = std::make_shared<Scene>();

        scene->camera.setType(Camera::Type::Perspective);
        scene->camera.setPosition(glm::vec3(0, 0, -1));
        scene->camera.setNearView(0.1f);
        scene->camera.setFarView(500.0f);
        scene->camera.setClipMatrix(clipMatrix);

        scene->light.setType(Light::Type::Directional);
        scene->light.setCastShadow(true);
        scene->light.setRotation(glm::vec3(0,-1,0), 0);
        scene->light.setClipMatrix(clipMatrix);

        scene->meshes.push_back(std::move(createRenderable(planeMeshPath)));
        scene->meshes.push_back(std::move(createRenderable(sphereMeshPath)));

        scene->meshes[0]->transform = glm::translate(scene->meshes[0]->transform, glm::vec3(0, -2, 0));
        scene->meshes[1]->transform = glm::translate(scene->meshes[0]->transform, glm::vec3(0, 2, 0));

        for (auto &mesh : scene->meshes) {
            mesh->material = std::make_shared<Material>(device);
            mesh->material->setGraphicsPipeline(meshPipeline);
            mesh->material->createMaterial();
            mesh->material->setTexture2D("shadowMap", shadowPass.depthTexture);

            mesh->shadowMaterial = std::make_shared<Material>(device);
            mesh->shadowMaterial->setGraphicsPipeline(shadowsPipeline);
            mesh->shadowMaterial->createMaterial();
        }
    }

    void updateScene() {
        processInput(window.handle);

        scene->camera.setAspect((float32) window.w / window.h);
        scene->camera.recalculate();

        float distance = 20.0f;

        Frustum frustumCut = scene->camera.getFrustum();
        frustumCut.cutFrustum(distance / scene->camera.getFarClip());
        scene->light.buildViewFrustum(frustumCut);

        Mat4f camViewProj = scene->camera.getViewProjClipMatrix();
        Mat4f lightViewProj = scene->light.getViewProjClipMatrix();

        for (auto &mesh : scene->meshes) {
            mesh->material->setMat4("UBO.viewProj", camViewProj);
            mesh->material->setMat4("UBO.model", mesh->transform);
            mesh->material->setMat4("UBO.lightSpace", lightViewProj);
            mesh->material->setVec3("UBO.lightDir", scene->light.getDirection());
            mesh->material->updateUniformData();

            mesh->shadowMaterial->setMat4("UBO.depthMVP", lightViewProj * mesh->transform);
            mesh->shadowMaterial->updateUniformData();
        }
    }

    RefCounted<RenderableMesh> createRenderable(const String &path) {
        RefCounted<RenderableMesh> result = std::make_shared<RenderableMesh>();

        MeshLoader meshLoader(path);
        RefCounted<Mesh> cmesh = meshLoader.importMesh(Mesh::VertexFormat::PNT);

        result->vertexBuffer = device->createVertexBuffer(
                BufferUsage::Static, cmesh->getVertexCount() * sizeof(Vertex), cmesh->getVertexData());
        result->indexCount = cmesh->getIndicesCount();
        result->indexBuffer = device->createIndexBuffer(
                BufferUsage::Static, result->indexCount * sizeof(uint32), cmesh->getIndexData());

        return result;
    }

    void shutdown() {
        for (auto &mesh : scene->meshes) {
            device->destroyVertexBuffer(mesh->vertexBuffer);
            device->destroyIndexBuffer(mesh->indexBuffer);
        }

        VulkanExtensions::destroySurface((VulkanRenderDevice &) *device, window.surface);
    }

    void initWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        window.handle = glfwCreateWindow(window.w, window.h, window.name.c_str(), nullptr, nullptr);
        glfwGetFramebufferSize(window.handle, &window.w, &window.h);

        window.extensions = glfwGetRequiredInstanceExtensions(&window.extensionsCount);
    }

    void processInput(GLFWwindow *window) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        auto &camera = scene->camera;

        float32 cameraSpeed = 2.0f / 60.0f;
        float32 cameraRotationSpeed = 1.0f / 60.0f;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.move(cameraSpeed * camera.getDirection());
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.move(-cameraSpeed * camera.getDirection());
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.move(-camera.getRight() * cameraSpeed);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.move(camera.getRight() * cameraSpeed);
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            camera.move(-camera.getUp() * cameraSpeed);
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
            camera.move(camera.getUp() * cameraSpeed);
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
            camera.rotate(glm::vec3(0, 1, 0), cameraRotationSpeed);
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
            camera.rotate(glm::vec3(0, 1, 0), -cameraRotationSpeed);
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            glm::vec3 r = glm::cross(camera.getDirection(), glm::vec3(0, 1, 0));
            camera.rotate(r, cameraRotationSpeed);
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            glm::vec3 r = glm::cross(camera.getDirection(), glm::vec3(0, 1, 0));
            camera.rotate(r, -cameraRotationSpeed);
        }
        if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
            scene->light.rotate(glm::vec3(1, 0, 0), cameraRotationSpeed);
        if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
            scene->light.rotate(glm::vec3(1, 0, 0), -cameraRotationSpeed);
        if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
            scene->light.rotate(glm::vec3(0, 1, 0), cameraRotationSpeed);
        if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
            scene->light.rotate(glm::vec3(0, 1, 0), -cameraRotationSpeed);
        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
            scene->meshes[0]->transform = glm::translate(scene->meshes[0]->transform, glm::vec3(0.5f, 0, 0));
            scene->meshes[1]->transform = glm::translate(scene->meshes[1]->transform, glm::vec3(0.5f, 0, 0));
        }
        if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
            scene->meshes[0]->transform = glm::translate(scene->meshes[0]->transform, glm::vec3(-0.5f, 0, 0));
            scene->meshes[1]->transform = glm::translate(scene->meshes[1]->transform, glm::vec3(-0.5f, 0, 0));
        }
    }

private:
    Window window;
    RefCounted<IRenderDevice> device;
    Mat4f clipMatrix;

    ShadowsPass shadowPass;
    RefCounted<Scene> scene;

    RefCounted<Shader> meshShader;
    RefCounted<Shader> shadowsShader;
    RefCounted<GraphicsPipeline> meshPipeline;
    RefCounted<GraphicsPipeline> shadowsPipeline;

    String meshVertPath = "shaders/spirv/shadowmapping/MeshVert.spv";
    String meshFragPath = "shaders/spirv/shadowmapping/MeshFrag.spv";
    String shadowsVertPath = "shaders/spirv/shadowmapping/ShadowsVert.spv";
    String shadowsFragPath = "shaders/spirv/shadowmapping/ShadowsFrag.spv";

    String planeMeshPath = "assets/models/plane.obj";
    String sphereMeshPath = "assets/models/sphere.obj";
};

int main() {
    TestShadows test;
    test.run();
    return 0;
}