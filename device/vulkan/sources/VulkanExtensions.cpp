/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov, Sultim Tsyrendashiev                  */
/**********************************************************************************/

#include <VulkanExtensions.h>
#include <VulkanErrors.h>

namespace ignimbrite {

#ifdef WITH_GLFW

    VulkanExtensions::ID VulkanExtensions::createSurfaceGLFW(VulkanRenderDevice &device, GLFWwindow *handle, uint32 width, uint32 height,
                                        uint32 widthFramebuffer, uint32 heightFramebuffer, const std::string &name) {
        VkResult result;
        VkSurfaceKHR surface;
        VulkanContext &context = device.context;

        result = glfwCreateWindowSurface(context.instance, handle, nullptr, &surface);

        if (result != VK_SUCCESS) {
            throw VulkanException("Failed to create window surface");
        }

        return createSurfaceFromKHR(device, surface, width, height, widthFramebuffer, heightFramebuffer, name);
    }
#endif

#ifdef WITH_IGNIMBRITE_QT

    VulkanExtensions::ID VulkanExtensions::createSurfaceQtWindow(VulkanRenderDevice &device, QVulkanInstance *qvkInstance, QWindow *qwindow) {

        // register instance
        qvkInstance->setVkInstance(device.context.instance);

        // create qt vulkan instance
        if (!qvkInstance->create()) {
            qFatal("Failed to create Vulkan instance: %d", qvkInstance->errorCode());
        }

        // bind instance to qwindow
        qwindow->setVulkanInstance(qvkInstance);
        // init
        qwindow->show();

        // get VkSurfaceKHR from qt window
        VkSurfaceKHR surfaceKhr = QVulkanInstance::surfaceForWindow(qwindow);

        if (surfaceKhr == nullptr) {
            qFatal("Failed to get VkSurfaceKHR from QWindow \"%s\"", (char*)qwindow->filePath().data());
        }

        // get size of the window without its window frame
        uint32 width = qwindow->width();
        uint32 height = qwindow->height();

        const char *windowTitle = (char*)qwindow->title().data();

        // create result from VkSurfaceKHR
        return VulkanExtensions::createSurfaceFromKHR(device, surfaceKhr,
                                                      width, height,
                                                      width, height,
                                                      windowTitle);
    }

    void VulkanExtensions::setVulkanInstance(VulkanRenderDevice &device, QVulkanInstance *qvkInstance) {
        // register instance
        qvkInstance->setVkInstance(device.context.instance);
    }

    VulkanExtensions::ID VulkanExtensions::createSurfaceQtWidget(VulkanRenderDevice &device, QWindow *qwindow) {
        // get VkSurfaceKHR from qt window
        VkSurfaceKHR surfaceKhr = QVulkanInstance::surfaceForWindow(qwindow);

        if (surfaceKhr == nullptr) {
            qFatal("Failed to get VkSurfaceKHR from QWindow \"%s\"", (char*)qwindow->filePath().data());
        }

        // get size of the window without its window frame
        uint32 width = qwindow->width();
        uint32 height = qwindow->height();

        const char *windowTitle = (char*)qwindow->title().data();

        // create result from VkSurfaceKHR
        return VulkanExtensions::createSurfaceFromKHR(device, surfaceKhr,
                                                      width, height,
                                                      width, height,
                                                      windowTitle);
    }
#endif

    VulkanExtensions::ID VulkanExtensions::createSurfaceFromKHR(VulkanRenderDevice &device, VkSurfaceKHR surfaceKhr, uint32 width,
                                                                uint32 height, uint32 widthFramebuffer, uint32 heightFramebuffer,
                                                                const std::string &name) {
        VkResult result;
        VulkanContext &context = device.context;

        VulkanSurface window = {};
        window.name = name;
        window.width = width;
        window.widthFramebuffer = widthFramebuffer;
        window.heightFramebuffer = heightFramebuffer;
        window.height = height;
        window.surface = surfaceKhr;

        result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.context.physicalDevice, surfaceKhr,
                                                           &window.surfaceCapabilities);

        if (result != VK_SUCCESS) {
            throw VulkanException("Failed to get surface capabilities");
        }

        context.findPresentsFamily(window);
        context.createSwapChain(window);
        context.createFramebufferFormat(window);
        context.createFramebuffers(window);

        return device.mSurfaces.move(window);
    }

    void VulkanExtensions::destroySurface(VulkanRenderDevice &device, VulkanExtensions::ID surface) {
        VulkanSurface &vulkanSurface = device.mSurfaces.get(surface);
        VulkanContext &context = device.context;

        context.deviceWaitIdle();
        context.destroyCommandBuffers(vulkanSurface);
        context.destroyFramebuffers(vulkanSurface);
        context.destroyFramebufferFormat(vulkanSurface);
        context.destroySwapChain(vulkanSurface);

        vkDestroySurfaceKHR(context.instance, vulkanSurface.surface, nullptr);

        device.mSurfaces.remove(surface);
    }

} // namespace ignimbrite
