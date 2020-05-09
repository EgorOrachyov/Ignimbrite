/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                        */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#include <VulkanExtensions.h>
#include <VulkanErrors.h>

namespace ignimbrite {

#ifdef IGNIMBRITE_WITH_GLFW

    ID<IRenderDevice::Surface> VulkanExtensions::createSurfaceGLFW(VulkanRenderDevice &device,
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

#ifdef IGNIMBRITE_WITH_QT

    ID<IRenderDevice::Surface> VulkanExtensions::createSurfaceQtWindow(VulkanRenderDevice &device, QVulkanInstance *qvkInstance, QWindow *qwindow) {

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
        qvkInstance->setVkInstance(device.mContext.instance);
    }

    ID<IRenderDevice::Surface> VulkanExtensions::createSurfaceQtWidget(VulkanRenderDevice &device, QWindow *qwindow) {
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

    void VulkanExtensions::destroySurface(VulkanRenderDevice &device, ID<IRenderDevice::Surface> surface, bool destroySurfKhr) {
        auto &vulkanSurface = device.mSurfaces.get(surface);
        auto &context = VulkanContext::getInstance();

        context.deviceWaitIdle();
        vulkanSurface.destroyFramebuffers();
        vulkanSurface.destroyFramebufferFormat();
        vulkanSurface.destroySwapChain();

        if (destroySurfKhr) {
            vkDestroySurfaceKHR(context.instance, vulkanSurface.surfaceKHR, nullptr);
        }

        device.mSurfaces.remove(surface);
    }

    ID<IRenderDevice::Surface> VulkanExtensions::createSurfaceFromKHR(VulkanRenderDevice &device, VkSurfaceKHR surfaceKhr,
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
