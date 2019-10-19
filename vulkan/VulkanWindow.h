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
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;
    std::vector<VkCommandBuffer> commandBuffers;
};

#endif //VULKANRENDERER_VULKANWINDOW_H
