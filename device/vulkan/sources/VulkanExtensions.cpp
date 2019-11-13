//
// Created by Egor Orachyov on 2019-11-11.
//

#include <VulkanExtensions.h>
#include <VulkanErrors.h>

#ifdef WITH_GLFW
void VulkanExtensions::createSurfaceGLFW(VulkanRenderDevice &device, GLFWwindow *handle, uint32 width, uint32 height,
                                         uint32 widthFramebuffer, uint32 heightFramebuffer, const std::string &name) {
    VkSurfaceKHR surface;
    VkResult result;
    VulkanContext &context = device.context;

    result = glfwCreateWindowSurface(context.instance, handle, nullptr, &surface);

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

    result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.context.physicalDevice, surface, &window.surfaceCapabilities);

    if (result != VK_SUCCESS) {
        throw VulkanException("Failed to get surface capabilities");
    }

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context.physicalDevice, surface, &window.surfaceCapabilities);
    context.createSwapChain(window);

    device.mSurfaces.move(window);
}
#endif

void VulkanExtensions::destroySurface(VulkanRenderDevice &device, VulkanExtensions::ID surface) {
    VulkanSurface &vulkanSurface = device.mSurfaces.get(surface);
    VulkanContext &context = device.context;

    // todo: wait idle
    context.destroySwapChain(vulkanSurface);
    device.mSurfaces.remove(surface);
}
