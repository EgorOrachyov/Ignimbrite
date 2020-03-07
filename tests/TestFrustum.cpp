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

#include <algorithm>
#include <fstream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtx/rotate_vector.hpp>

using namespace ignimbrite;

struct Window {
    GLFWwindow*             glfwWindow = nullptr;
    int32                   widthFrameBuffer = 0, heightFrameBuffer = 0;
    uint32                  extensionsCount = 0;
    const char* const*      extensions = nullptr;
};

struct Mesh {
    ID<IRenderDevice::VertexBuffer>     vertexBuffer;
    ID<IRenderDevice::IndexBuffer>      indexBuffer;
    uint32                              indexCount = 0;
};

struct UniformBufferData {
    glm::mat4x4     viewProj;
    glm::mat4x4     model;
    glm::vec4       color = { 1.0f, 1.0f, 1.0f, 1.0f };
};

struct Material {
    UniformBuffer                   *buffer;
    UniformBufferData               data;
    ID<IRenderDevice::UniformSet>   uniformSet;
};

struct Model {
    Mesh        mesh;
    Material    material;
};

struct Camera {
    glm::vec3   position;
    glm::vec3   direction;
    glm::vec3   up;
};

struct AABBModel {
    AABB        aabb;
    Model       model;
};

struct FrustumModel {
    Frustum     frustum;
    Model       model;
};

struct Scene {
    Camera                      camera;
    FrustumModel                *frustum;
    std::vector<AABBModel*>     aabbs;
};

class TestFrustum {
public:
    TestFrustum(const Frustum &frustum, const AABB *aabbs, uint32 aabbCount) {
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

    ~TestFrustum() {
        destroy();
    }

    void loop() {
        IRenderDevice::Color clearColor = { { 0.5f, 0.5f, 0.5f, 0.0f} };

        while (!glfwWindowShouldClose(window.glfwWindow)) {
            glfwPollEvents();
            glfwSwapBuffers(window.glfwWindow);
            glfwGetFramebufferSize(window.glfwWindow, &window.widthFrameBuffer, &window.heightFrameBuffer);

            IRenderDevice::Region area = { 0, 0, { (uint32)window.widthFrameBuffer, (uint32)window.heightFrameBuffer} };

            if (area.extent.x == 0|| area.extent.y == 0)
            {
                continue;
            }

            updateScene();

            device->drawListBegin();
            {
                device->drawListBindSurface(surface, clearColor, area);
                device->drawListBindPipeline(graphicsPipeline);

                Model &frmodel = scene.frustum->model;

                device->drawListBindUniformSet(frmodel.material.uniformSet);
                device->drawListBindVertexBuffer(frmodel.mesh.vertexBuffer, 0, 0);
                device->drawListBindIndexBuffer(frmodel.mesh.indexBuffer, ignimbrite::IndicesType::Uint32, 0);
                device->drawListDrawIndexed(frmodel.mesh.indexCount, 1);

                for (auto *aabbModel : scene.aabbs)
                {
                    Model &model = aabbModel->model;

                    device->drawListBindUniformSet(model.material.uniformSet);
                    device->drawListBindVertexBuffer(model.mesh.vertexBuffer, 0, 0);
                    device->drawListBindIndexBuffer(model.mesh.indexBuffer, ignimbrite::IndicesType::Uint32, 0);
                    device->drawListDrawIndexed(model.mesh.indexCount, 1);
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

        window.glfwWindow = glfwCreateWindow(window.widthFrameBuffer, window.heightFrameBuffer, name.c_str(), nullptr, nullptr);

        glfwGetFramebufferSize(window.glfwWindow, (int*) &window.widthFrameBuffer, (int*) &window.heightFrameBuffer);

        // get required extensions for a surface
        window.extensions = glfwGetRequiredInstanceExtensions(&window.extensionsCount);
//
//        glfwSetCursorPosCallback(window.glfwWindow, mouseCallback);
//        glfwSetScrollCallback(window.glfwWindow, scrollCallback);
    }

    void initVertexLayout() {
        IRenderDevice::VertexBufferLayoutDesc vertexBufferLayoutDesc = {};

        IRenderDevice::VertexAttributeDesc attr = {};
        attr.location = 0;
        attr.format = DataFormat::R32G32B32A32_SFLOAT;
        attr.offset = 0;

        vertexBufferLayoutDesc.stride = 4 * sizeof(float);
        vertexBufferLayoutDesc.usage = VertexUsage::PerVertex;
        vertexBufferLayoutDesc.attributes.push_back(attr);

        vertexLayout = device->createVertexLayout({ vertexBufferLayoutDesc });
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

        const glm::vec3 *nearVerts = frustum.getNearVerts();
        const glm::vec3 *farVerts = frustum.getFarVerts();

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

        outModel.mesh.vertexBuffer = device->createVertexBuffer(BufferUsage::Static,
                                                                vertCount * sizeof(glm::vec4), verts);

        outModel.mesh.indexCount = indexCount;
        outModel.mesh.indexBuffer = device->createIndexBuffer(BufferUsage::Static,
                                                              indexCount * sizeof(uint32), indices);

        outModel.material.buffer = new UniformBuffer(device);

        outModel.material.buffer->createBuffer(sizeof(UniformBufferData));
        outModel.material.data.model = glm::mat4x4(1.0f);
        outModel.material.data.color = glm::vec4(0,0,1,0.3f);

        outModel.material.uniformSet = createUniformSet(*outModel.material.buffer);
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

        for (uint32 i = 0; i < aabbCount; i++) {
            auto *aabbModel = new AABBModel();
            initAABBModel(aabbs[i], *aabbModel);

            scene.aabbs.push_back(aabbModel);
        }

        scene.camera.up = glm::vec3(0,1,0);
        scene.camera.direction = glm::vec3(0,0,1);
        scene.camera.position = glm::vec3(0,0, -1);
    }

    void updateScene() {
        processInput(window.glfwWindow);

        auto view = glm::lookAt(scene.camera.position, scene.camera.position + scene.camera.direction, scene.camera.up);
        auto proj = glm::perspective(1.5f, (float)window.widthFrameBuffer / window.heightFrameBuffer, 0.1f, 1000.0f);

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

            aabbm->model.material.buffer->updateData(sizeof(UniformBufferData), 0, (uint8*)data);
        }

        UniformBufferData *frdata = &scene.frustum->model.material.data;
        frdata->viewProj = viewProj;
        scene.frustum->model.material.buffer->updateData(sizeof(UniformBufferData), 0, (uint8*)frdata);
    }

    void destroyScene() {
        device->destroyUniformSet(scene.frustum->model.material.uniformSet);

        device->destroyVertexBuffer(scene.frustum->model.mesh.vertexBuffer);
        device->destroyIndexBuffer(scene.frustum->model.mesh.indexBuffer);

        delete scene.frustum->model.material.buffer;
        delete scene.frustum;

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

        float cameraSpeed = 2.0f / 60.0f;
        float cameraRotationSpeed = 0.5f / 60.0f;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            scene.camera.position += cameraSpeed * scene.camera.direction;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            scene.camera.position -= cameraSpeed * scene.camera.direction;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            scene.camera.position -= glm::normalize(glm::cross(scene.camera.direction, scene.camera.up)) * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            scene.camera.position += glm::normalize(glm::cross(scene.camera.direction, scene.camera.up)) * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            scene.camera.position -= glm::normalize(scene.camera.up) * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
            scene.camera.position += glm::normalize(scene.camera.up) * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
            scene.camera.direction = glm::rotate(scene.camera.direction, cameraRotationSpeed, glm::vec3(0,1,0));
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
            scene.camera.direction = glm::rotate(scene.camera.direction, -cameraRotationSpeed, glm::vec3(0,1,0));
    }

private:
    RefCounted<VulkanRenderDevice>      device;
    ID<IRenderDevice::Surface>          surface;
    Window                              window;

    RefCounted<Shader>                  shader;
    ID<IRenderDevice::GraphicsPipeline> graphicsPipeline;

    Scene                               scene;

    ID<IRenderDevice::VertexLayout>     vertexLayout;

    String          name                = "Frustum Test";
    String          vertShaderPath      = "shaders/spirv/TestFrustumVert.spv";
    String          fragShaderPath      = "shaders/spirv/TestFrustumFrag.spv";
};

int main() {
    Frustum f = {};
    f.setFov(90).setAspect(16.0f/9.0f)
        .setNear(0.1f).setFar(100.0f)
        .setVectors(glm::vec3(0,0,1),glm::vec3(1,0,0), glm::vec3(0,1,0))
        .build();

    /*const float range = 4;
    const int amount = 10;
    const float delta = range * 2 / amount;
    const int count = amount * amount * amount;
    AABB aabbs[count];

    for (int i = 0; i < amount; i++) {
        for (int j = 0; j < amount; j++) {
            for (int k = 0; k < amount; k++) {
                glm::vec3 s(-range + i * delta, -range + j * delta, -range + k * delta);
                glm::vec3 e = s + glm::vec3(delta);

                aabbs[i * amount * amount + j * amount + k] = AABB(s, e);
            }
        }
    }*/

    const int count = 5;
    AABB aabbs[count];

    aabbs[0] = AABB(glm::vec3(-8,-0.5f,-0.5f), glm::vec3(-7,0.5f,0.5f));
    aabbs[1] = AABB(glm::vec3(-0.5f,-0.5f,5), glm::vec3(0.5f,0.5f,6));
    aabbs[2] = AABB(glm::vec3(-0.5f,-5.5f,5), glm::vec3(0.5f,-6.5f,6));
    aabbs[3] = AABB(glm::vec3(-0.5f,-7.5f,5), glm::vec3(0.5f,-8.5f,6));
    aabbs[4] = AABB(glm::vec3(4,-0.5f,2), glm::vec3(5,0.5f,3));

    auto *testFrustum = new TestFrustum(f, aabbs, count);
    testFrustum->loop();

    delete testFrustum;
}