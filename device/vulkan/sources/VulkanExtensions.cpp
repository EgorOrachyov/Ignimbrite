//
// Created by Egor Orachyov on 2019-11-11.
//

#include <VulkanExtensions.h>
#include <VulkanErrors.h>

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

    VulkanSurface window = {};
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

    context.findPresentsFamily(window);
    context.createSwapChain(window);
    context.createFramebufferFormat(window);
    context.createFramebuffers(window);

    return device.mSurfaces.move(window);
}
#endif

void VulkanExtensions::destroySurface(VulkanRenderDevice &device, VulkanExtensions::ID surface) {
    VulkanSurface &vulkanSurface = device.mSurfaces.get(surface);
    VulkanContext &context = device.context;

    context.deviceWaitIdle();
    context.destroyFramebuffers(vulkanSurface);
    context.destroyFramebufferFormat(vulkanSurface);
    context.destroySwapChain(vulkanSurface);

    vkDestroySurfaceKHR(context.instance, vulkanSurface.surface, nullptr);

    device.mSurfaces.remove(surface);
}
