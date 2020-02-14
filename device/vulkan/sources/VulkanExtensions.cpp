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

    RenderDevice::ID VulkanExtensions::createSurfaceGLFW(VulkanRenderDevice &device,
                                                         GLFWwindow *handle,
                                                         uint32 widthFramebuffer,
                                                         uint32 heightFramebuffer,
                                                         const std::string &name) {
        VkSurfaceKHR surfaceKHR;

        auto &context = VulkanContext::getInstance();
        auto result = glfwCreateWindowSurface(context.instance, handle, nullptr, &surfaceKHR);
        VK_RESULT_ASSERT(result, "Failed to create window surface");

        return createSurfaceFromKHR(device, surfaceKHR, widthFramebuffer, heightFramebuffer, name);
    }
#endif

#ifdef WITH_IGNIMBRITE_QT

    VulkanExtensions::ID VulkanExtensions::createSurfaceQtWindow(VulkanRenderDevice &device, QVulkanInstance *qvkInstance, QWindow *qwindow) {

        setVulkanInstance(device, qvkInstance);

        // create qt vulkan instance
        if (!qvkInstance->create()) {
            qFatal("Failed to create Vulkan instance: %d", qvkInstance->errorCode());
        }

        // bind instance to qwindow
        qwindow->setVulkanInstance(qvkInstance);
        // init
        qwindow->show();

        return createSurfaceQtWidget(device, qwindow);
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
                                                      windowTitle);
    }
#endif

    void VulkanExtensions::destroySurface(VulkanRenderDevice &device, RenderDevice::ID surface) {
        auto &vulkanSurface = device.mSurfaces.get(surface);
        auto &context = VulkanContext::getInstance();

        context.deviceWaitIdle();
        vulkanSurface.destroyFramebuffers();
        vulkanSurface.destroyFramebufferFormat();
        vulkanSurface.destroySwapChain();
        vkDestroySurfaceKHR(context.instance, vulkanSurface.surfaceKHR, nullptr);

        device.mSurfaces.remove(surface);
    }

    RenderDevice::ID
    VulkanExtensions::createSurfaceFromKHR(VulkanRenderDevice &device, VkSurfaceKHR surfaceKhr,
                                           uint32 widthFramebuffer, uint32 heightFramebuffer,
                                           const std::string &name) {

        VulkanSurface surface(widthFramebuffer, heightFramebuffer, name, surfaceKhr);
        surface.findPresentsFamily();
        surface.updateSurfaceCapabilities();
        surface.createSwapChain();
        surface.createFramebufferFormat();
        surface.createFramebuffers();
        surface.acquireFirstImage();

        return device.mSurfaces.move(surface);
    }

} // namespace ignimbrite
