//
// Created by Egor Orachyov on 2019-11-11.
//

#include "include/VulkanExtensions.h"
#include "include/VulkanErrors.h"

void VulkanExtensions::createSurfaceGLFW(VulkanRenderDevice &device, GLFWwindow *handle, uint32 width, uint32 height,
                                         uint32 widthFramebuffer, uint32 heightFramebuffer, const std::string &name) {
    VkSurfaceKHR surface;
    VkResult result = glfwCreateWindowSurface(device.context.getInstance(), handle, nullptr, &surface);

    if (result != VK_SUCCESS) {
        throw VulkanException("Failed to create window surface");
    }

    VulkanRenderDevice::Window window = { };
    window.name = name;
    window.width = width;
    window.widthFramebuffer = widthFramebuffer;
    window.heightFramebuffer = heightFramebuffer;
    window.height = height;
    window.surface = surface;

    device.mWindows.move(window);
}
