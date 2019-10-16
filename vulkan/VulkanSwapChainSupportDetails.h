//
// Created by Egor Orachyov on 2019-10-16.
//

#ifndef VULKANRENDERER_VULKANSWAPCHAINSUPPORTDETAILS_H
#define VULKANRENDERER_VULKANSWAPCHAINSUPPORTDETAILS_H

#include <VulkanGlfw.h>
#include <vector>

struct VulkanSwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

#endif //VULKANRENDERER_VULKANSWAPCHAINSUPPORTDETAILS_H
