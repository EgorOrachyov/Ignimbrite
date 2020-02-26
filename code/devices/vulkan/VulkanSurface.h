/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                        */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#ifndef IGNIMBRITE_VULKANSURFACE_H
#define IGNIMBRITE_VULKANSURFACE_H

#include <Types.h>
#include <VulkanFramebuffer.h>
#include <VulkanFence.h>
#include <VulkanObjects.h>
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
        std::vector<VulkanAllocation> depthStencilAllocation;
    };

    /** Represents window drawing area, created by native OS window system */
    class VulkanSurface {
    public:
        VulkanSurface(uint32 width, uint32 height, std::string name, VkSurfaceKHR surfaceKHR);
        ~VulkanSurface() = default;
        VulkanSurface(VulkanSurface && other) = default;

        void createSwapChain();
        void destroySwapChain();

        void createFramebufferFormat();
        void destroyFramebufferFormat();

        void createFramebuffers();
        void destroyFramebuffers();

        /** Get valid surface properties (if surface is resized, properties will be changed) */
        void updateSurfaceCapabilities();
        void findPresentsFamily();

        void resizeSurface();
        void acquireFirstImage();
        /** Get image ready for rendering and acquire next image */
        void acquireNextImage();

    private:
        void getSurfaceProperties(std::vector<VkSurfaceFormatKHR> &outSurfaceFormats,
                                  std::vector<VkPresentModeKHR> &outPresentModes);
        VkExtent2D getSwapChainExtent(uint32 preferredWidth, uint32 preferredHeight);
        VkCompositeAlphaFlagBitsKHR getAvailableCompositeAlpha();

    public:
        std::string name;
        uint32 width;
        uint32 height;
        uint32 presentsFamily = 0xffffffff;
        bool canPresentImages = true;
        VkQueue presentQueue = VK_NULL_HANDLE;
        /** Surface created via extension for specific WSI */
        VkSurfaceKHR surfaceKHR = VK_NULL_HANDLE;
        VkPresentModeKHR presentMode;
        VkSurfaceFormatKHR surfaceFormat;
        VkSurfaceCapabilitiesKHR surfaceCapabilities;
        VulkanSwapChain swapChain;
        uint32 currentImageIndex = 0;
        VulkanFence imageAvailable;

    };

} // namespace ignimbrite

#endif //IGNIMBRITE_VULKANSURFACE_H
