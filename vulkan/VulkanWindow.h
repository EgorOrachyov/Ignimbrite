//
// Created by Egor Orachyov on 2019-10-15.
//

#ifndef VULKANRENDERER_VULKANWINDOW_H
#define VULKANRENDERER_VULKANWINDOW_H

#include <Platform.h>
#include <Types.h>
#include <VulkanGlfw.h>
#include <vector>

struct VulkanWindow {
    uint32 width = 600;
    uint32 height = 400;
    GLFWwindow* handle = nullptr;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkSwapchainKHR swapChain = VK_NULL_HANDLE;
    std::vector<VkImage> swapChainImages;
};

#endif //VULKANRENDERER_VULKANWINDOW_H
