//
// Created by Egor Orachyov on 2019-10-26.
//

#ifndef RENDERINGLIBRARY_VULKANAPPLICATIONTEST_H
#define RENDERINGLIBRARY_VULKANAPPLICATIONTEST_H

#include <VulkanRenderDevice.h>
#include <VulkanExtensions.h>

class VulkanApplication {
public:

    VulkanApplication() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
        glfwGetFramebufferSize(window, (int32*) &widthFrameBuffer, (int32*) &heightFrameBuffer);
        extensions = glfwGetRequiredInstanceExtensions(&extensionsCount);

        pDevice = new VulkanRenderDevice(extensionsCount, extensions);
        auto& device = *pDevice;

        surface = VulkanExtensions::createSurfaceGLFW(device, window, width, height, widthFrameBuffer, heightFrameBuffer, name);

        RenderDevice::VertexAttributeDesc vertexAttributeDesc = {};
        vertexAttributeDesc.format = DataFormat::R32G32B32_SFLOAT;
        vertexAttributeDesc.location = 0;
        vertexAttributeDesc.offset = 0;

        RenderDevice::VertexBufferLayoutDesc vertexBufferLayoutDesc = {};
        vertexBufferLayoutDesc.attributes.push_back(vertexAttributeDesc);
        vertexBufferLayoutDesc.stride = sizeof(float32) * 3;
        vertexBufferLayoutDesc.usage = VertexUsage::PerVertex;

        vertexLayout = device.createVertexLayout({ vertexBufferLayoutDesc });

        vertexBuffer = device.createVertexBuffer(BufferUsage::Dynamic, sizeof(vertices), vertices);

        indexBuffer = device.createIndexBuffer(BufferUsage::Static, sizeof(indices), indices);

        uniformBuffer = device.createUniformBuffer(BufferUsage::Dynamic, sizeof(Transform), &transform);

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

    }

    void loop() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            glfwSwapBuffers(window);
        }
    }

    ~VulkanApplication() {
        VulkanExtensions::destroySurface(*pDevice, surface);
        glfwDestroyWindow(window);
        glfwTerminate();

        delete pDevice;
    }

    static void run() {
        VulkanApplication application;
        application.loop();
    }

private:

    typedef ObjectID ID;

    ID surface;
    GLFWwindow* window;
    std::string name = "Test";
    uint32 width = 640, height = 480;
    uint32 widthFrameBuffer, heightFrameBuffer;
    uint32 extensionsCount;
    const char* const* extensions;

    VulkanRenderDevice* pDevice;

    ID vertexLayout;
    ID vertexBuffer;
    ID indexBuffer;
    ID uniformBuffer;
    ID uniformLayout;
    ID uniformSet;
    ID shaderProgram;

    struct Transform {
        float32 values[16] = {
                1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f
        };
    } transform;

    float32 vertices[9] = {
            0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f
    };

    uint16 indices[3] = {
        0, 1, 2
    };

};

#endif //RENDERINGLIBRARY_VULKANAPPLICATIONTEST_H
