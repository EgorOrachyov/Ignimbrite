//
// Created by Egor Orachyov on 2019-11-09.
//

#ifndef RENDERINGLIBRARY_VULKANSTARTUP_H
#define RENDERINGLIBRARY_VULKANSTARTUP_H

#include "VulkanContext.h"
#include "VulkanRenderDevice.h"
#include "VulkanExtensions.h"

struct VulkanStartUp {

    static void test1() {
        VulkanContext context;
        context.fillRequiredExt(0, nullptr);
        context.createInstance();
        context.setupDebugMessenger();
        context.pickPhysicalDevice();
        context.createLogicalDevice();

        context.destroyLogicalDevice();
        context.destroyDebugMessenger();
        context.destroyInstance();
    }

    static void test2() {
        ObjectID surface;
        GLFWwindow* window;
        std::string name = "Test";
        uint32 width = 640, height = 480, widthFrameBuffer, heightFrameBuffer;
        uint32 extensionsCount;
        const char* const* extensions;

        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
        glfwGetFramebufferSize(window, (int32*) &widthFrameBuffer, (int32*) &heightFrameBuffer);
        extensions = glfwGetRequiredInstanceExtensions(&extensionsCount);

        VulkanRenderDevice device(extensionsCount, extensions);
        surface = VulkanExtensions::createSurfaceGLFW(device, window, width, height, widthFrameBuffer, heightFrameBuffer, name);

        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            glfwSwapBuffers(window);
        }

        VulkanExtensions::destroySurface(device, surface);
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    static void run() {
        // test1();
        test2();
    }

};

#endif //RENDERINGLIBRARY_VULKANSTARTUP_H
