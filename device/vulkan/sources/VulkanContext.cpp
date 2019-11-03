//
// Created by Egor Orachyov on 2019-11-03.
//

#include "include/VulkanContext.h"
#include <exception>

VkInstance VulkanContext::getInstance() const {
    return instance;
}

VkDevice VulkanContext::getDevice() const {
    return device;
}

const VkPhysicalDeviceMemoryProperties &VulkanContext::getDeviceMemoryProperties() const {
    return deviceMemoryProperties;
}

uint32_t VulkanContext::getMemoryTypeIndex(uint32_t memoryTypeBits, VkFlags requirementsMask) const {
    // for each memory type available for this device
    for (uint32_t i = 0; i < deviceMemoryProperties.memoryTypeCount; i++)
    {
        // if type is available
        if ((memoryTypeBits & 1) == 1)
        {
            if ((deviceMemoryProperties.memoryTypes[i].propertyFlags & requirementsMask) == requirementsMask)
            {
                return i;
            }
        }

        memoryTypeBits >>= 1;
    }

    throw std::exception("Vulkan::Can't find memory type in device memory properties");
}

VkCommandPool VulkanContext::getCommandPool() const {
    return commandPool;
}

VkQueue VulkanContext::getTransferQueue() const {
    return transferQueue;
}
