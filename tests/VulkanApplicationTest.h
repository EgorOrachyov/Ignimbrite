//
// Created by Egor Orachyov on 2019-10-26.
//

#ifndef RENDERINGLIBRARY_VULKANAPPLICATIONTEST_H
#define RENDERINGLIBRARY_VULKANAPPLICATIONTEST_H

#include <VulkanRenderDevice.h>
#include <VulkanExtensions.h>
#include <cassert>
#include <fstream>
#include <iterator>
#include <utility>

class VulkanApplication {
public:

    VulkanApplication() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
        glfwGetFramebufferSize(window, (int32 *) &widthFrameBuffer, (int32 *) &heightFrameBuffer);
        extensions = glfwGetRequiredInstanceExtensions(&extensionsCount);

        pDevice = new VulkanRenderDevice(extensionsCount, extensions);
        auto &device = *pDevice;

        surface = VulkanExtensions::createSurfaceGLFW(device, window, width, height, widthFrameBuffer,
                                                      heightFrameBuffer, name);

        RenderDevice::VertexAttributeDesc vertexAttributeDesc = {};
        vertexAttributeDesc.format = DataFormat::R32G32B32_SFLOAT;
        vertexAttributeDesc.location = 0;
        vertexAttributeDesc.offset = 0;

        RenderDevice::VertexBufferLayoutDesc vertexBufferLayoutDesc = {};
        vertexBufferLayoutDesc.attributes.push_back(vertexAttributeDesc);
        vertexBufferLayoutDesc.stride = sizeof(float32) * 3;
        vertexBufferLayoutDesc.usage = VertexUsage::PerVertex;

        vertexLayout = device.createVertexLayout({vertexBufferLayoutDesc});

        vertexBuffer = device.createVertexBuffer(BufferUsage::Dynamic, sizeof(vertices), vertices);

        indexBuffer = device.createIndexBuffer(BufferUsage::Static, sizeof(indices), indices);

        uniformBuffer = device.createUniformBuffer(BufferUsage::Dynamic, sizeof(Transform), &transform);

        loadTestShader(device);

        RenderDevice::UniformLayoutBufferDesc uniformLayoutBufferDesc = {};
        uniformLayoutBufferDesc.binding = 0;
        uniformLayoutBufferDesc.flags = (uint32) ShaderStageFlagBits::VertexBit;

        RenderDevice::UniformLayoutDesc uniformLayoutDesc = {};
        uniformLayoutDesc.buffers.push_back(uniformLayoutBufferDesc);

        uniformLayout = device.createUniformLayout(uniformLayoutDesc);

        RenderDevice::UniformBufferDesc uniformBufferDesc = {};
        uniformBufferDesc.binding = 0;
        uniformBufferDesc.offset = 0;
        uniformBufferDesc.range = sizeof(Transform);
        uniformBufferDesc.buffer = uniformBuffer;

        RenderDevice::UniformSetDesc uniformSetDesc = {};
        uniformSetDesc.buffers.push_back(uniformBufferDesc);

        uniformSet = device.createUniformSet(uniformSetDesc, uniformLayout);

        RenderDevice::PipelineRasterizationDesc rasterizationDesc = {};
        rasterizationDesc.cullMode = PolygonCullMode::Back;
        rasterizationDesc.frontFace = PolygonFrontFace::FrontCounterClockwise;
        rasterizationDesc.lineWidth = 1.0f;
        rasterizationDesc.mode = PolygonMode::Fill;

        PrimitiveTopology topology = PrimitiveTopology::TriangleList;

        RenderDevice::BlendAttachmentDesc blendAttachmentDesc = {};
        blendAttachmentDesc.blendEnable = false;

        RenderDevice::PipelineSurfaceBlendStateDesc blendStateDesc = {};
        blendStateDesc.attachment = blendAttachmentDesc;
        blendStateDesc.logicOpEnable = false;
        blendStateDesc.logicOp = LogicOperation::Copy;

        graphicsPipeline = device.createGraphicsPipeline(
                surface,
                topology,
                shaderProgram,
                vertexLayout,
                uniformLayout,
                rasterizationDesc,
                blendStateDesc
        );

    }


    ~VulkanApplication() {
        auto &device = *pDevice;

        device.destroyGraphicsPipeline(graphicsPipeline);
        device.destroyShaderProgram(shaderProgram);
        device.destroyUniformSet(uniformSet);
        device.destroyUniformLayout(uniformLayout);
        device.destroyUniformBuffer(uniformBuffer);
        device.destroyVertexBuffer(vertexBuffer);
        device.destroyIndexBuffer(indexBuffer);
        device.destroyVertexLayout(vertexLayout);

        VulkanExtensions::destroySurface(*pDevice, surface);
        glfwDestroyWindow(window);
        glfwTerminate();

        delete pDevice;
    }

    void loadTestShader(VulkanRenderDevice &device) {
        std::vector<RenderDevice::ShaderDataDesc> shaderDescs(2);

        shaderDescs[0].language = ShaderLanguage::SPIRV;
        shaderDescs[1].language = ShaderLanguage::SPIRV;
        shaderDescs[0].type = ShaderType::Vertex;
        shaderDescs[1].type = ShaderType::Fragment;

        std::ifstream vertFile("shaders/vert.spv", std::ios::binary);
        std::ifstream fragFile("shaders/frag.spv", std::ios::binary);

        if (!vertFile.is_open() || !fragFile.is_open()) {
            throw std::runtime_error("Failed to open spir-v files");
        }

        std::vector<uint8> vertSpv(std::istreambuf_iterator<char>(vertFile), {});
        std::vector<uint8> fragSpv(std::istreambuf_iterator<char>(fragFile), {});

        shaderDescs[0].source = std::move(vertSpv);
        shaderDescs[1].source = std::move(fragSpv);

        shaderProgram = device.createShaderProgram(shaderDescs);
    }

    void loop() {
        auto &device = *pDevice;

        RenderDevice::Color clearColor = {
                {0.1f, 0.4f, 0.7f, 0.0f}
        };

        RenderDevice::Region area = {};
        area.extent.x = widthFrameBuffer;
        area.extent.y = heightFrameBuffer;

        // TODO: DELETE THIS
        device.swapBuffers(surface);

        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();

            device.drawListBegin();
            // TODO: default render area, which is same as surface size
            device.drawListBindSurface(surface, clearColor, area);

            device.drawListBindPipeline(graphicsPipeline);

            device.drawListBindUniformSet(uniformSet);
            device.drawListBindVertexBuffer(vertexBuffer, 0, 0);
            device.drawListBindIndexBuffer(indexBuffer, IndicesType::Uint16, 0);
            device.drawListDrawIndexed(sizeof(indices) / sizeof(uint16), 1);

            device.drawListEnd();

            device.swapBuffers(surface);
            //glfwSwapBuffers(window);
        }
    }

    static void run() {
        VulkanApplication application;
        application.loop();
    }

private:

    typedef ObjectID ID;

    std::string name = "Test";

    ID surface;
    GLFWwindow *window = nullptr;
    uint32 width = 640;
    uint32 height = 480;
    uint32 widthFrameBuffer = 0;
    uint32 heightFrameBuffer = 0;
    uint32 extensionsCount = 0;
    const char *const *extensions = nullptr;

    VulkanRenderDevice *pDevice;

    ID vertexLayout;
    ID vertexBuffer;
    ID indexBuffer;
    ID uniformBuffer;
    ID uniformLayout;
    ID uniformSet;
    ID shaderProgram;
    ID graphicsPipeline;

    struct Transform {
        float32 values[16] = {
                1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f
        };
    } transform;

    float32 vertices[9] = {
            -0.5f, -0.5f, 0.0f,
            0.5f, -0.5f, 0.0f,
            0.5f, 0.5f, 0.0f
    };

    uint16 indices[3] = {
            2, 1, 0
    };

};

#endif //RENDERINGLIBRARY_VULKANAPPLICATIONTEST_H
