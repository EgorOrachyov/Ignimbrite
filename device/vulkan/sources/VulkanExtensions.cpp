//
// Created by Egor Orachyov on 2019-11-11.
//

#include <VulkanExtensions.h>
#include <VulkanErrors.h>

void VulkanExtensions::createSurfaceGLFW(VulkanRenderDevice &device, GLFWwindow *handle, uint32 width, uint32 height,
                                         uint32 widthFramebuffer, uint32 heightFramebuffer, const std::string &name) {
    VkSurfaceKHR surface;
    VkResult result = glfwCreateWindowSurface(device.context.instance, handle, nullptr, &surface);

    if (result != VK_SUCCESS) {
        throw VulkanException("Failed to create window surface");
    }

    VulkanSurface window = { };
    window.name = name;
    window.width = width;
    window.widthFramebuffer = widthFramebuffer;
    window.heightFramebuffer = heightFramebuffer;
    window.height = height;
    window.surface = surface;

    device.mSurfaces.move(window);
}
