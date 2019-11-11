//
// Created by Egor Orachyov on 2019-11-07.
//

#ifndef RENDERINGLIBRARY_VULKANUTILS_H
#define RENDERINGLIBRARY_VULKANUTILS_H

#include <renderer/Optional.h>
#include <vulkan/vulkan.h>
#include <vector>

struct QueueFamilyIndices {
    Optional<uint32> graphicsFamily;
    Optional<uint32> presentFamily;
    Optional<uint32> transferFamily;

    bool isComplete() {
        return graphicsFamily.hasValue() &&
               presentFamily.hasValue() &&
               transferFamily.hasValue();
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

#endif //RENDERINGLIBRARY_VULKANUTILS_H
