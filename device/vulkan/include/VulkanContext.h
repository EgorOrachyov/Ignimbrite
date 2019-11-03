//
// Created by Egor Orachyov on 2019-11-03.
//

#ifndef RENDERINGLIBRARY_VULKANCONTEXT_H
#define RENDERINGLIBRARY_VULKANCONTEXT_H

#include <vulkan/vulkan.h>

/**
 * Handles vulkan instance setup. Defines physical
 * device and creates logical device for application.
 * defines queue families, finds graphics, present and transfer queues
 */
class VulkanContext {
public:

private:
    VkInstance instance = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;

    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;
    VkQueue transferQueue = VK_NULL_HANDLE;

};


#endif //RENDERINGLIBRARY_VULKANCONTEXT_H
