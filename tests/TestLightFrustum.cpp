/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                        */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#include <VulkanRenderDevice.h>
#include <VulkanExtensions.h>
#include <Shader.h>
#include <UniformBuffer.h>
#include <Frustum.h>
#include <Light.h>

#include <algorithm>
#include <fstream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtx/rotate_vector.hpp>

using namespace ignimbrite;

struct Window {
    GLFWwindow *glfwWindow = nullptr;
    int32 widthFrameBuffer = 0, heightFrameBuffer = 0;
    uint32 extensionsCount = 0;
    const char *const *extensions = nullptr;
};

struct Mesh {
    ID<IRenderDevice::VertexBuffer> vertexBuffer;
    ID<IRenderDevice::IndexBuffer> indexBuffer;
    uint32 indexCount = 0;
};

struct UniformBufferData {
    glm::mat4x4 viewProj;
    glm::mat4x4 model;
    glm::vec4 color = {1.0f, 1.0f, 1.0f, 1.0f};
};

struct MatData {
    UniformBuffer *buffer;
    UniformBufferData data;
    ID<IRenderDevice::UniformSet> uniformSet;
};

struct Model {
    Mesh mesh;
    MatData material;
};

struct Camera {
    glm::vec3 position;
    glm::vec3 direction;
    glm::vec3 up;
};

struct AABBModel {
    AABB aabb;
    Model model;
};

struct FrustumModel {
    Frustum frustum;
    Model model;
};

struct Scene {
    Camera camera;
    Light light;
    FrustumModel *lightFrModel;
    FrustumModel *frustum;
    std::vector<AABBModel *> aabbs;
    bool drawLightFrustum;
    bool drawBoxes;
};

class TestLightFrustum {
public:
    TestLightFrustum(const Frustum &frustum, const AABB *aabbs, uint32 aabbCount) {
        initWindow();

        device = std::make_shared<VulkanRenderDevice>(window.extensionsCount, window.extensions);
        surface = VulkanExtensions::createSurfaceGLFW(
                *device, window.glfwWindow,
                window.widthFrameBuffer, window.heightFrameBuffer, name);

        initVertexLayout();
        initShader();
        initGraphicsPipeline();

        initScene(frustum, aabbs, aabbCount);
    }

    ~TestLightFrustum() {
        destroy();
    }

    void loop() {
        IRenderDevice::Color clearColor = {{0.5f, 0.5f, 0.5f, 0.0f}};

        while (!glfwWindowShouldClose(window.glfwWindow)) {
            glfwPollEvents();
            glfwSwapBuffers(window.glfwWindow);
            glfwGetFramebufferSize(window.glfwWindow, &window.widthFrameBuffer, &window.heightFrameBuffer);

            IRenderDevice::Region area = {0, 0, {(uint32) window.widthFrameBuffer, (uint32) window.heightFrameBuffer}};

            if (area.extent.x == 0 || area.extent.y == 0) {
                continue;
            }

            updateScene();

            device->drawListBegin();
            {
                device->drawListBindSurface(surface, clearColor, area);
                device->drawListBindPipeline(graphicsPipeline);

                uint32 frustumCount = scene.drawLightFrustum ? 2 : 1;
                FrustumModel *frustums[] = { scene.frustum, scene.lightFrModel};

                for (uint32 i = 0; i < frustumCount; i++) {
                    Model &model = frustums[i]->model;

                    device->drawListBindUniformSet(model.material.uniformSet);
                    device->drawListBindVertexBuffer(model.mesh.vertexBuffer, 0, 0);
                    device->drawListBindIndexBuffer(model.mesh.indexBuffer, ignimbrite::IndicesType::Uint32, 0);
                    device->drawListDrawIndexed(model.mesh.indexCount, 1);
                }

                if (scene.drawBoxes) {
                    for (auto *aabbModel : scene.aabbs) {
                        Model &model = aabbModel->model;

                        device->drawListBindUniformSet(model.material.uniformSet);
                        device->drawListBindVertexBuffer(model.mesh.vertexBuffer, 0, 0);
                        device->drawListBindIndexBuffer(model.mesh.indexBuffer, ignimbrite::IndicesType::Uint32, 0);
                        device->drawListDrawIndexed(model.mesh.indexCount, 1);
                    }
                }
            }
            device->drawListEnd();

            device->flush();
            device->synchronize();
            device->swapBuffers(surface);
        }
    }

private:
    void initWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        window.widthFrameBuffer = 1280;
        window.heightFrameBuffer = 720;

        window.glfwWindow = glfwCreateWindow(window.widthFrameBuffer, window.heightFrameBuffer, name.c_str(), nullptr,
                                             nullptr);

        glfwGetFramebufferSize(window.glfwWindow, (int32 *) &window.widthFrameBuffer,
                               (int32 *) &window.heightFrameBuffer);

        // get required extensions for a surface
        window.extensions = glfwGetRequiredInstanceExtensions(&window.extensionsCount);
    }

    void initVertexLayout() {
        IRenderDevice::VertexBufferLayoutDesc vertexBufferLayoutDesc = {};

        IRenderDevice::VertexAttributeDesc attr = {};
        attr.location = 0;
        attr.format = DataFormat::R32G32B32A32_SFLOAT;
        attr.offset = 0;

        vertexBufferLayoutDesc.stride = 4 * sizeof(float32);
        vertexBufferLayoutDesc.usage = VertexUsage::PerVertex;
        vertexBufferLayoutDesc.attributes.push_back(attr);

        vertexLayout = device->createVertexLayout({vertexBufferLayoutDesc});
    }

    void initShader() {
        std::ifstream vertFile(vertShaderPath.c_str(), std::ios::binary);
        std::ifstream fragFile(fragShaderPath.c_str(), std::ios::binary);

        std::vector<uint8> vertSpv(std::istreambuf_iterator<char>(vertFile), {});
        std::vector<uint8> fragSpv(std::istreambuf_iterator<char>(fragFile), {});

        shader = std::make_shared<Shader>(device);
        shader->fromSources(ShaderLanguage::SPIRV, vertSpv, fragSpv);
        shader->reflectData();
        shader->generateUniformLayout();
    }

    void initGraphicsPipeline() {
        IRenderDevice::PipelineRasterizationDesc rasterizationDesc = {};
        rasterizationDesc.cullMode = PolygonCullMode::Disabled;
        rasterizationDesc.frontFace = PolygonFrontFace::FrontCounterClockwise;
        rasterizationDesc.lineWidth = 1.0f;
        rasterizationDesc.mode = PolygonMode::Fill;

        IRenderDevice::BlendAttachmentDesc blendAttachmentDesc = {};
        blendAttachmentDesc.blendEnable = false;
        IRenderDevice::PipelineSurfaceBlendStateDesc blendStateDesc = {};
        blendStateDesc.attachment = blendAttachmentDesc;
        blendStateDesc.logicOpEnable = false;
        blendStateDesc.logicOp = LogicOperation::NoOp;

        IRenderDevice::PipelineDepthStencilStateDesc depthStencilStateDesc = {};
        depthStencilStateDesc.depthCompareOp = CompareOperation::Less;
        depthStencilStateDesc.depthWriteEnable = true;
        depthStencilStateDesc.depthTestEnable = true;
        depthStencilStateDesc.stencilTestEnable = false;

        graphicsPipeline = device->createGraphicsPipeline(
                surface,
                PrimitiveTopology::TriangleList,
                shader->getHandle(),
                vertexLayout,
                shader->getLayout(),
                rasterizationDesc,
                blendStateDesc,
                depthStencilStateDesc
        );
    }

    void initAABBModel(const AABB &aabb, AABBModel &outAABBModel) {

        outAABBModel.aabb = aabb;
        Model &outModel = outAABBModel.model;

        const uint32 vertCount = 8;
        const uint32 indexCount = 3 * 2 * 6;

        glm::vec4 verts[vertCount];
        glm::vec3 extent = aabb.getExtent();

        verts[0] = glm::vec4(extent.x, extent.y, -extent.z, 1);
        verts[1] = glm::vec4(-extent.x, extent.y, -extent.z, 1);
        verts[2] = glm::vec4(-extent.x, -extent.y, -extent.z, 1);
        verts[3] = glm::vec4(extent.x, -extent.y, -extent.z, 1);

        verts[4] = glm::vec4(extent.x, extent.y, extent.z, 1);
        verts[5] = glm::vec4(-extent.x, extent.y, extent.z, 1);
        verts[6] = glm::vec4(-extent.x, -extent.y, extent.z, 1);
        verts[7] = glm::vec4(extent.x, -extent.y, extent.z, 1);

        uint32 indices[indexCount] = {
                0, 1, 2,
                0, 2, 3,
                4, 7, 6,
                4, 6, 5,
                1, 6, 2,
                1, 5, 6,
                1, 0, 4,
                1, 4, 5,
                3, 4, 0,
                3, 7, 4,
                6, 7, 3,
                6, 3, 2
        };

        outModel.mesh.vertexBuffer = device->createVertexBuffer(BufferUsage::Static,
                                                                vertCount * sizeof(glm::vec4), verts);

        outModel.mesh.indexCount = indexCount;
        outModel.mesh.indexBuffer = device->createIndexBuffer(BufferUsage::Static,
                                                              indexCount * sizeof(uint32), indices);

        outModel.material.buffer = new UniformBuffer(device);

        outModel.material.buffer->createBuffer(sizeof(UniformBufferData));
        outModel.material.data.model = glm::translate(glm::mat4x4(1.0f), aabb.getCenter());

        outModel.material.uniformSet = createUniformSet(*outModel.material.buffer);
    }

    void initFrustumModel(const Frustum &frustum, FrustumModel &outFrustumModel) {

        outFrustumModel.frustum = frustum;
        Model &outModel = outFrustumModel.model;

        const uint32 vertCount = 8;
        const uint32 indexCount = 3 * 2 * 6;

        const glm::vec3 *nearVerts = frustum.getNearVertices();
        const glm::vec3 *farVerts = frustum.getFarVertices();

        glm::vec4 verts[vertCount];

        verts[0] = glm::vec4(nearVerts[0], 1);
        verts[1] = glm::vec4(nearVerts[1], 1);
        verts[2] = glm::vec4(nearVerts[2], 1);
        verts[3] = glm::vec4(nearVerts[3], 1);

        verts[4] = glm::vec4(farVerts[0], 1);
        verts[5] = glm::vec4(farVerts[1], 1);
        verts[6] = glm::vec4(farVerts[2], 1);
        verts[7] = glm::vec4(farVerts[3], 1);

        uint32 indices[indexCount] = {
                0, 1, 2,
                0, 2, 3,
                4, 7, 6,
                4, 6, 5,
                1, 6, 2,
                1, 5, 6,
                1, 0, 4,
                1, 4, 5,
                3, 4, 0,
                3, 7, 4,
                6, 7, 3,
                6, 3, 2
        };

        outModel.mesh.vertexBuffer = device->createVertexBuffer(BufferUsage::Dynamic,
                                                                vertCount * sizeof(glm::vec4), verts);

        outModel.mesh.indexCount = indexCount;
        outModel.mesh.indexBuffer = device->createIndexBuffer(BufferUsage::Static,
                                                              indexCount * sizeof(uint32), indices);

        outModel.material.buffer = new UniformBuffer(device);

        outModel.material.buffer->createBuffer(sizeof(UniformBufferData));
        outModel.material.data.model = glm::mat4x4(1.0f);
        outModel.material.data.color = glm::vec4(0, 0, 1, 0.3f);

        outModel.material.uniformSet = createUniformSet(*outModel.material.buffer);
    }

    void updateFrustumMesh(const Frustum &frustum) {
        const uint32 vertCount = 8;

        const glm::vec3 *nearVerts = frustum.getNearVertices();
        const glm::vec3 *farVerts = frustum.getFarVertices();

        glm::vec4 verts[vertCount];

        verts[0] = glm::vec4(nearVerts[0], 1);
        verts[1] = glm::vec4(nearVerts[1], 1);
        verts[2] = glm::vec4(nearVerts[2], 1);
        verts[3] = glm::vec4(nearVerts[3], 1);

        verts[4] = glm::vec4(farVerts[0], 1);
        verts[5] = glm::vec4(farVerts[1], 1);
        verts[6] = glm::vec4(farVerts[2], 1);
        verts[7] = glm::vec4(farVerts[3], 1);

        device->updateVertexBuffer(scene.frustum->model.mesh.vertexBuffer, vertCount * sizeof(glm::vec4), 0, verts);

        scene.light.fitCameraFrustum(frustum);

        const glm::vec3 *nearLightFrVerts = scene.light.getFrustum().getNearVertices();
        const glm::vec3 *farLightFrVerts = scene.light.getFrustum().getFarVertices();

        glm::vec4 lightFrVerts[vertCount];

        lightFrVerts[0] = glm::vec4(nearLightFrVerts[0], 1);
        lightFrVerts[1] = glm::vec4(nearLightFrVerts[1], 1);
        lightFrVerts[2] = glm::vec4(nearLightFrVerts[2], 1);
        lightFrVerts[3] = glm::vec4(nearLightFrVerts[3], 1);

        lightFrVerts[4] = glm::vec4(farLightFrVerts[0], 1);
        lightFrVerts[5] = glm::vec4(farLightFrVerts[1], 1);
        lightFrVerts[6] = glm::vec4(farLightFrVerts[2], 1);
        lightFrVerts[7] = glm::vec4(farLightFrVerts[3], 1);

        device->updateVertexBuffer(scene.lightFrModel->model.mesh.vertexBuffer, vertCount * sizeof(glm::vec4), 0,
                                   lightFrVerts);
    }

    ID<IRenderDevice::UniformSet> createUniformSet(const UniformBuffer &unbuffer) {

        IRenderDevice::UniformBufferDesc uniformBufferDesc = {};
        uniformBufferDesc.binding = 0;
        uniformBufferDesc.offset = 0;
        uniformBufferDesc.range = sizeof(UniformBufferData);
        uniformBufferDesc.buffer = unbuffer.getHandle();

        IRenderDevice::UniformSetDesc uniformSetDesc = {};
        uniformSetDesc.buffers.push_back(uniformBufferDesc);

        return device->createUniformSet(uniformSetDesc, shader->getLayout());
    }

    void initScene(const Frustum &frustum, const AABB *aabbs, uint32 aabbCount) {

        scene.frustum = new FrustumModel();
        initFrustumModel(frustum, *scene.frustum);

        scene.light = Light(LightType::Directional);
        scene.light.setDirection(glm::vec3(1, 0, 0), glm::vec3(0, 1, 0));
        scene.light.fitCameraFrustum(scene.frustum->frustum);

        scene.lightFrModel = new FrustumModel();
        initFrustumModel(scene.light.getFrustum(), *scene.lightFrModel);

        for (uint32 i = 0; i < aabbCount; i++) {
            auto *aabbModel = new AABBModel();
            initAABBModel(aabbs[i], *aabbModel);

            scene.aabbs.push_back(aabbModel);
        }

        scene.camera.up = glm::vec3(0, 1, 0);
        scene.camera.direction = glm::vec3(0, 0, 1);
        scene.camera.position = glm::vec3(0, 0, -1);

        scene.drawLightFrustum = true;
    }

    void updateScene() {
        auto &camera = scene.camera;

        processInput(window.glfwWindow);

        auto view = glm::lookAt(camera.position, camera.position + camera.direction, camera.up);
        auto proj = glm::perspective(1.5f, (float32) window.widthFrameBuffer / window.heightFrameBuffer, 0.1f, 1000.0f);

        auto clip = glm::mat4(1.0f, 0.0f, 0.0f, 0.0f,
                              0.0f, -1.0f, 0.0f, 0.0f,
                              0.0f, 0.0f, 0.5f, 0.0f,
                              0.0f, 0.0f, 0.5f, 1.0f);

        glm::mat4x4 viewProj = clip * proj * view;

        for (AABBModel *aabbm : scene.aabbs) {

            bool isIn = scene.frustum->frustum.isInside(aabbm->aabb);
            UniformBufferData *data = &aabbm->model.material.data;

            data->viewProj = viewProj;
            data->color = isIn ? glm::vec4(0, 1, 0, 0.3f) : glm::vec4(1, 0, 0, 0.3f);

            aabbm->model.material.buffer->updateData(sizeof(UniformBufferData), 0, (uint8 *) data);
        }

        UniformBufferData *frdata = &scene.frustum->model.material.data;
        frdata->viewProj = viewProj;
        scene.frustum->model.material.buffer->updateData(sizeof(UniformBufferData), 0, (uint8 *) frdata);

        UniformBufferData *lightFrdata = &scene.lightFrModel->model.material.data;
        lightFrdata->viewProj = viewProj;
        lightFrdata->color = glm::vec4(1, 1, 0, 0.3f);
        scene.lightFrModel->model.material.buffer->updateData(sizeof(UniformBufferData), 0, (uint8 *) lightFrdata);
    }

    void destroyScene() {
        device->destroyUniformSet(scene.frustum->model.material.uniformSet);

        device->destroyVertexBuffer(scene.frustum->model.mesh.vertexBuffer);
        device->destroyIndexBuffer(scene.frustum->model.mesh.indexBuffer);

        device->destroyUniformSet(scene.lightFrModel->model.material.uniformSet);

        device->destroyVertexBuffer(scene.lightFrModel->model.mesh.vertexBuffer);
        device->destroyIndexBuffer(scene.lightFrModel->model.mesh.indexBuffer);

        delete scene.frustum->model.material.buffer;
        delete scene.frustum;

        delete scene.lightFrModel->model.material.buffer;
        delete scene.lightFrModel;

        for (AABBModel *aabbModel : scene.aabbs) {
            device->destroyUniformSet(aabbModel->model.material.uniformSet);

            device->destroyVertexBuffer(aabbModel->model.mesh.vertexBuffer);
            device->destroyIndexBuffer(aabbModel->model.mesh.indexBuffer);

            delete aabbModel->model.material.buffer;
            delete aabbModel;
        }
        scene.aabbs.clear();
    }

    void destroy() {
        destroyScene();

        device->destroyGraphicsPipeline(graphicsPipeline);
        device->destroyVertexLayout(vertexLayout);

        ignimbrite::VulkanExtensions::destroySurface(*device, surface);
        device = nullptr;

        glfwDestroyWindow(window.glfwWindow);
        glfwTerminate();
    }

    void processInput(GLFWwindow *window) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        auto &camera = scene.camera;

        float32 cameraSpeed = 20.0f / 60.0f;
        float32 cameraRotationSpeed = 1 / 60.0f;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.position += cameraSpeed * camera.direction;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.position -= cameraSpeed * camera.direction;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.position -= glm::normalize(glm::cross(camera.direction, camera.up)) * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.position += glm::normalize(glm::cross(camera.direction, camera.up)) * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            camera.position -= glm::normalize(camera.up) * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
            camera.position += glm::normalize(camera.up) * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
            camera.direction = glm::rotate(camera.direction, cameraRotationSpeed, glm::vec3(0, 1, 0));
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
            camera.direction = glm::rotate(camera.direction, -cameraRotationSpeed, glm::vec3(0, 1, 0));

        static double lastUpdateTime = 0;

        if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && glfwGetTime() > lastUpdateTime + 0.1f) {
            scene.drawBoxes = !scene.drawBoxes;
            lastUpdateTime = glfwGetTime();
        }
        if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS && glfwGetTime() > lastUpdateTime + 0.1f) {
            scene.drawLightFrustum = !scene.drawLightFrustum;
            lastUpdateTime = glfwGetTime();
        }

        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
            auto &f = scene.frustum->frustum;
            f.setViewProperties(f.getForward(), f.getUp());
            f.createPerspective(f.getPosition() + glm::vec3(0, 0, 0.1), M_PI / 4.0f, 16.0f / 9.0f, 0.1f, 20.0f);
            updateFrustumMesh(f);
        }

        if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
            auto &f = scene.frustum->frustum;
            f.setViewProperties(f.getForward(), f.getUp());
            f.createPerspective(f.getPosition() - glm::vec3(0, 0, 0.1), M_PI / 4.0f, 16.0f / 9.0f, 0.1f, 20.0f);
            updateFrustumMesh(f);
        }

        static float y = 0, x = 0;

        if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS ||
            glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS ||
            glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS ||
            glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {

            if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
                y -= 0.05f;
            }
            if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
                y += 0.05f;
            }

            if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) {
                x -= 0.05f;
            }
            if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
                x += 0.05f;
            }

            auto &f = scene.frustum->frustum;

            y = glm::clamp(y, (float) (-M_PI / 2.0f + 0.05f), (float) (M_PI / 2.0f - 0.05f));

            glm::quat q = glm::quat(glm::vec3(y, x, 0));
            glm::vec3 d = glm::normalize(q * glm::vec3(0, 0, 1));
            glm::vec3 r = glm::cross(d, glm::vec3(0, 1, 0));
            glm::vec3 u = glm::cross(r, d);

            f.setViewProperties(d, u);
            f.createPerspective(f.getPosition(), M_PI / 4.0f, 16.0f / 9.0f, 0.1f, 20.0f);
            updateFrustumMesh(f);
        }
    }

private:

    Scene scene;
    Window window;

    ID<IRenderDevice::Surface> surface;
    ID<IRenderDevice::VertexLayout> vertexLayout;
    ID<IRenderDevice::GraphicsPipeline> graphicsPipeline;

    RefCounted<Shader> shader;
    RefCounted<VulkanRenderDevice> device;

    String name = "Frustum Test";
    String vertShaderPath = "shaders/spirv/TestFrustumVert.spv";
    String fragShaderPath = "shaders/spirv/TestFrustumFrag.spv";
};

int32 main() {
    Frustum f = {};
    f.setViewProperties(glm::vec3(0, 0, 1), glm::vec3(0, 1, 0));
    f.createPerspective(glm::vec3(0, 0, 1), M_PI / 4.0f, 16.0f / 9.0f, 0.1f, 20.0f);

    const float32 range = 4;
    const int32 amount = 10;
    const float32 delta = range * 2 / amount;
    const int32 count = amount * amount * amount;
    AABB aabbs[count];

    for (int32 i = 0; i < amount; i++) {
        for (int32 j = 0; j < amount; j++) {
            for (int32 k = 0; k < amount; k++) {
                glm::vec3 s(-range + i * delta, -range + j * delta, -range + k * delta);
                glm::vec3 e = s + glm::vec3(delta / 2);

                aabbs[i * amount * amount + j * amount + k] = AABB(s, e);
            }
        }
    }

    TestLightFrustum testFrustum(f, aabbs, count);
    testFrustum.loop();

    return 0;
}