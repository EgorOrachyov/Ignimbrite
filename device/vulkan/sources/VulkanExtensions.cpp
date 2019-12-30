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

        VulkanSurface surface(widthFramebuffer, heightFramebuffer, name, surfaceKHR);
        surface.findPresentsFamily();
        surface.updateSurfaceCapabilities();
        surface.createSwapChain();
        surface.createFramebufferFormat();
        surface.createFramebuffers();
        surface.acquireFirstImage();
        return device.mSurfaces.move(surface);
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

} // namespace ignimbrite
