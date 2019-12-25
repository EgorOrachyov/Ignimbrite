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

    RenderDevice::ID VulkanExtensions::createSurfaceGLFW(VulkanRenderDevice &device, GLFWwindow *handle, uint32 widthFramebuffer,
                                        uint32 heightFramebuffer, const std::string &name) {
        VkResult result;
        VkSurfaceKHR surfaceKHR;
        auto &context = VulkanContext::getSingleton();

        result = glfwCreateWindowSurface(
                context.instance,
                handle,
                nullptr,
                &surfaceKHR
        );

        if (result != VK_SUCCESS) {
            throw VulkanException("Failed to create window surface");
        }

        VulkanSurface surface = {};
        surface.name = name;
        surface.width = widthFramebuffer;
        surface.height = heightFramebuffer;
        surface.surface = surfaceKHR;

        context.updateSurfaceCapabilities(surface);
        context.findPresentsFamily(surface);
        context.createSwapChain(surface);
        context.createFramebufferFormat(surface);
        context.createFramebuffers(surface);

        return device.mSurfaces.move(surface);
    }

#endif

    void VulkanExtensions::destroySurface(VulkanRenderDevice &device, RenderDevice::ID surface) {
        auto &vulkanSurface = device.mSurfaces.get(surface);
        auto &context = VulkanContext::getSingleton();

        context.deviceWaitIdle();
        context.destroyFramebuffers(vulkanSurface);
        context.destroyFramebufferFormat(vulkanSurface);
        context.destroySwapChain(vulkanSurface);

        vkDestroySurfaceKHR(context.instance, vulkanSurface.surface, nullptr);

        device.mSurfaces.remove(surface);
    }

} // namespace ignimbrite
