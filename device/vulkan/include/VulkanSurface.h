/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov, Sultim Tsyrendashiev                  */
/**********************************************************************************/

#ifndef IGNIMBRITELIBRARY_VULKANSURFACE_H
#define IGNIMBRITELIBRARY_VULKANSURFACE_H

#include <ignimbrite/Types.h>
#include <VulkanFramebuffer.h>
#include <string>
#include <vector>

namespace ignimbrite {

    /** Associated with chain data also needed for screen rendering (managed automatically) */
    struct VulkanSwapChain {
        VkSwapchainKHR swapChainKHR;
        VkExtent2D extent;
        VkFormat depthFormat;
        VulkanFrameBufferFormat framebufferFormat;
        std::vector<VkFramebuffer> framebuffers;
        /** Images and views color attachment 0 */
        std::vector<VkImage> images;
        std::vector<VkImageView> imageViews;
        /** Images and views for depth buffer (created by hand) */
        std::vector<VkImage> depthStencilImages;
        std::vector<VkImageView> depthStencilImageViews;
        std::vector<VkDeviceMemory> depthStencilImageMemory;
    };

    /** Represents window drawing area, created by native OS window system */
    struct VulkanSurface {
        std::string name;
        uint32 width;
        uint32 height;
        uint32 presentsFamily;
        VkQueue presentQueue;
        VkQueue graphicsQueue;
        /** Surface created vie extension for specific WSI */
        VkSurfaceKHR surface;
        VkPresentModeKHR presentMode;
        VkSurfaceFormatKHR surfaceFormat;
        VkSurfaceCapabilitiesKHR surfaceCapabilities;
        VulkanSwapChain swapChain;
        /** Swap buffer data */
        uint32 currentImageIndex = 0;
        uint32 currentFrameIndex = 0;
        uint32 maxFramesInFlight = 0;
        std::vector<VkSemaphore> imageAvailableSemaphores;
        std::vector<VkSemaphore> renderFinishedSemaphores;
        std::vector<VkFence> inFlightFences;
        std::vector<VkFence> imagesInFlight;
    };

} // namespace ignimbrite

#endif //IGNIMBRITELIBRARY_VULKANSURFACE_H
