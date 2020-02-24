/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov, Sultim Tsyrendashiev                  */
/**********************************************************************************/

#ifndef IGNIMBRITE_VULKANAPPLICATIONTEST_H
#define IGNIMBRITE_VULKANAPPLICATIONTEST_H

#include <VulkanRenderDevice.h>
#include <VulkanExtensions.h>
#include <cassert>
#include <fstream>
#include <iterator>
#include <utility>

using namespace ignimbrite;

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

        surface = VulkanExtensions::createSurfaceGLFW(
                device,
                window,
                widthFrameBuffer, heightFrameBuffer,
                name
        );

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

        RenderDevice::PipelineDepthStencilStateDesc depthStencilStateDesc = {};
        depthStencilStateDesc.depthTestEnable = false;
        depthStencilStateDesc.stencilTestEnable = false;

        graphicsPipeline = device.createGraphicsPipeline(
                surface,
                topology,
                shaderProgram,
                vertexLayout,
                uniformLayout,
                rasterizationDesc,
                blendStateDesc,
                depthStencilStateDesc
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
        std::ifstream vertFile("shaders/spirv/vert.spv", std::ios::binary);
        std::ifstream fragFile("shaders/spirv/frag.spv", std::ios::binary);

        if (!vertFile.is_open() || !fragFile.is_open()) {
            throw std::runtime_error("Failed to open spir-v files");
        }

        std::vector<uint8> vertSpv(std::istreambuf_iterator<char>(vertFile), {});
        std::vector<uint8> fragSpv(std::istreambuf_iterator<char>(fragFile), {});

        RenderDevice::ProgramDesc programDesc;
        programDesc.language = ShaderLanguage::SPIRV;
        programDesc.shaders.resize(2);

        programDesc.shaders[0].type = ShaderType::Vertex;
        programDesc.shaders[0].source = std::move(vertSpv);

        programDesc.shaders[1].type = ShaderType::Fragment;
        programDesc.shaders[1].source = std::move(fragSpv);

        shaderProgram = device.createShaderProgram(programDesc);
    }

    void loop() {
        auto &device = *pDevice;

        RenderDevice::Color clearColor = { {0.1f, 0.4f, 0.7f, 0.0f} };

        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            glfwGetFramebufferSize(window, &widthFrameBuffer, &heightFrameBuffer);

            RenderDevice::Region area = { 0, 0, { (uint32) widthFrameBuffer, (uint32) heightFrameBuffer } };

            if (area.extent.x == 0 || area.extent.y == 0)
            {
                continue;
            }

            device.drawListBegin();
            device.drawListBindSurface(surface, clearColor, area);
            device.drawListBindPipeline(graphicsPipeline);

            device.drawListBindUniformSet(uniformSet);
            device.drawListBindVertexBuffer(vertexBuffer, 0, 0);
            device.drawListBindIndexBuffer(indexBuffer, IndicesType::Uint16, 0);
            device.drawListDrawIndexed(sizeof(indices) / sizeof(uint16), 1);

            device.drawListEnd();

            device.flush();
            device.synchronize();
            device.swapBuffers(surface);
        }
    }

private:

    std::string name = "Test";

    ID<RenderDevice::Surface> surface;
    GLFWwindow *window = nullptr;
    int32 width = 640;
    int32 height = 480;
    int32 widthFrameBuffer = 0;
    int32 heightFrameBuffer = 0;
    uint32 extensionsCount = 0;
    const char *const *extensions = nullptr;

    VulkanRenderDevice *pDevice;

    ID<RenderDevice::VertexLayout> vertexLayout;
    ID<RenderDevice::VertexBuffer> vertexBuffer;
    ID<RenderDevice::IndexBuffer> indexBuffer;
    ID<RenderDevice::UniformBuffer> uniformBuffer;
    ID<RenderDevice::UniformLayout> uniformLayout;
    ID<RenderDevice::UniformSet> uniformSet;
    ID<RenderDevice::ShaderProgram> shaderProgram;
    ID<RenderDevice::GraphicsPipeline> graphicsPipeline;

    struct Transform {
        float32 values[16] = {
                1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f
        };
    } transform;

    float32 vertices[9] = {
            0.0f, 0.5f, 0.0f,
            0.5f, -0.5f, 0.0f,
            -0.5f, -0.5f, 0.0f
    };

    uint16 indices[3] = {
            0, 1, 2
    };

};

int32 main() {
    VulkanApplication application;
    application.loop();
}

#endif //IGNIMBRITE_VULKANAPPLICATIONTEST_H
