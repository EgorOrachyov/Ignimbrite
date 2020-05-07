/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                        */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#include <VulkanSurface.h>
#include <VulkanUtils.h>
#include <array>
#include <limits>

namespace ignimbrite {

    VulkanSurface::VulkanSurface(uint32 width, uint32 height, std::string name,
                                 VkSurfaceKHR surfaceKHR)
                                 :  name(std::move(name)), width(width), height(height), surfaceKHR(surfaceKHR) {
    }

    void VulkanSurface::createSwapChain() {
        auto& context = VulkanContext::getInstance();

        uint32 swapChainMinImageCount = VulkanContext::SWAPCHAIN_MIN_IMAGE_COUNT;
        VkSwapchainKHR swapChainKHR = VK_NULL_HANDLE;

        std::vector<VkSurfaceFormatKHR> surfaceFormats;
        std::vector<VkPresentModeKHR> presentModes;
        getSurfaceProperties(surfaceFormats, presentModes);

        VkSurfaceFormatKHR chosenSurfaceFormat;
        VkPresentModeKHR chosenPresentMode;

        chosenSurfaceFormat.format = surfaceFormats[0].format;
        chosenSurfaceFormat.colorSpace = surfaceFormats[0].colorSpace;

        for (auto &format : surfaceFormats) {
            if (format.format == VulkanContext::PREFERRED_FORMAT &&
                format.colorSpace == VulkanContext::PREFERRED_COLOR_SPACE) {
                chosenSurfaceFormat.format = format.format;
                chosenSurfaceFormat.colorSpace = format.colorSpace;
                break;
            }
        }

        chosenPresentMode = presentModes[0];

        for (auto &mode : presentModes) {
            if (mode == VulkanContext::PREFERRED_PRESENT_MODE) {
                chosenPresentMode = VulkanContext::PREFERRED_PRESENT_MODE;
                break;
            }
        }

        VkSwapchainCreateInfoKHR swapChainCreateInfo = {};
        swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapChainCreateInfo.pNext = NULL;
        swapChainCreateInfo.surface = surfaceKHR;

        swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;
        swapChainCreateInfo.clipped = true;
        swapChainCreateInfo.presentMode = chosenPresentMode;
        swapChainCreateInfo.imageFormat = chosenSurfaceFormat.format;
        swapChainCreateInfo.imageColorSpace = chosenSurfaceFormat.colorSpace;
        swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        std::array<uint32, 3> queueFamilyIndices = {
                context.familyIndices.graphicsFamily.get(),
                context.familyIndices.transferFamily.get(),
                presentsFamily
        };

        // check queues, if they are from same queue families
        auto equals = true;
        auto base = queueFamilyIndices[0];
        for (auto i: queueFamilyIndices) {
            if (i != base) {
                equals = false;
                break;
            }
        }

        if (equals) {
            swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            swapChainCreateInfo.queueFamilyIndexCount = 0;
            swapChainCreateInfo.pQueueFamilyIndices = NULL;
        } else {
            swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            swapChainCreateInfo.queueFamilyIndexCount = queueFamilyIndices.size();
            swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices.data();
        }

        swapChainCreateInfo.imageExtent = getSwapChainExtent(width, height);
        swapChainCreateInfo.compositeAlpha = getAvailableCompositeAlpha();

        if (surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
            swapChainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        } else {
            swapChainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
        }

        if (!(swapChainMinImageCount <= surfaceCapabilities.maxImageCount || surfaceCapabilities.maxImageCount == 0)) {
            throw VulkanException("Given swap chain min image count is not available on this surface and device");
        }

        if (surfaceCapabilities.minImageCount > swapChainMinImageCount) {
            swapChainMinImageCount = surfaceCapabilities.minImageCount;
        }

        swapChainCreateInfo.minImageCount = swapChainMinImageCount;
        swapChainCreateInfo.imageArrayLayers = 1;

        VkResult result = vkCreateSwapchainKHR(context.device, &swapChainCreateInfo, nullptr, &swapChainKHR);
        VK_RESULT_ASSERT(result, "Failed to create swap chain");

        uint32 swapChainImageCount;
        result = vkGetSwapchainImagesKHR(context.device, swapChainKHR, &swapChainImageCount, nullptr);
        VK_RESULT_ASSERT(result, "Failed to get images from swap chain");

        swapChain.images.resize(swapChainImageCount);
        swapChain.imageViews.resize(swapChainImageCount);

        result = vkGetSwapchainImagesKHR(context.device, swapChainKHR, &swapChainImageCount, swapChain.images.data());
        VK_RESULT_ASSERT(result, "Failed to get images from swap chain");

        for (uint32 i = 0; i < swapChainImageCount; i++) {
            VkImageViewCreateInfo viewInfo = {};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.pNext = nullptr;
            viewInfo.flags = 0;

            viewInfo.image = swapChain.images[i];
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = chosenSurfaceFormat.format;
            viewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
            viewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
            viewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
            viewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            result = vkCreateImageView(context.device, &viewInfo, nullptr, &swapChain.imageViews[i]);
            VK_RESULT_ASSERT(result, "Failed to create image view for swapchain");
        }

        // Setup depth stencil buffers for each swap chain image
        swapChain.depthStencilImages.resize(swapChainImageCount);
        swapChain.depthStencilImageViews.resize(swapChainImageCount);
        swapChain.depthStencilAllocation.resize(swapChainImageCount);

        VkFormat depthFormats[] = { VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };
        VkImageTiling imageTiling = VK_IMAGE_TILING_OPTIMAL;
        VkFormatFeatureFlags featureFlags = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
        VkFormat depthFormat = VulkanUtils::findSupportedFormat(depthFormats, 2, imageTiling, featureFlags);

        swapChain.depthFormat = depthFormat;

        for (uint32 i = 0; i < swapChainImageCount; i++) {
            VulkanUtils::createImage(
                    width, height, 1, 1, false,
                    VK_IMAGE_TYPE_2D, depthFormat,
                    VK_IMAGE_TILING_OPTIMAL,
                    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    swapChain.depthStencilImages[i],
                    swapChain.depthStencilAllocation[i]
            );

            VkImageSubresourceRange subresourceRange = {};
            subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
            subresourceRange.baseMipLevel = 0;
            subresourceRange.levelCount = 1;
            subresourceRange.baseArrayLayer = 0;
            subresourceRange.layerCount = 1;

            VkComponentMapping components = {
                    VK_COMPONENT_SWIZZLE_IDENTITY,
                    VK_COMPONENT_SWIZZLE_IDENTITY,
                    VK_COMPONENT_SWIZZLE_IDENTITY,
                    VK_COMPONENT_SWIZZLE_IDENTITY
            };

            VulkanUtils::createImageView(
                    swapChain.depthStencilImageViews[i],
                    swapChain.depthStencilImages[i],
                    VK_IMAGE_VIEW_TYPE_2D,
                    depthFormat,
                    subresourceRange,
                    components
            );
        }

        presentMode = chosenPresentMode;
        surfaceFormat = chosenSurfaceFormat;
        swapChain.extent = swapChainCreateInfo.imageExtent;
        swapChain.swapChainKHR = swapChainKHR;
    }

    void VulkanSurface::destroySwapChain() {
        auto& context = VulkanContext::getInstance();

        // Counts of images for all image related objects are equal
        uint32 swapChainObjects = swapChain.images.size();

        for (uint32 i = 0; i < swapChainObjects; i++) {
            // destroy only image views, images will be destroyed with swap chain
            vkDestroyImageView(context.device, swapChain.imageViews[i], nullptr);
            // destroy manually created depth stencil buffers
            vkDestroyImageView(context.device, swapChain.depthStencilImageViews[i], nullptr);

            VulkanUtils::destroyImage(swapChain.depthStencilImages[i], swapChain.depthStencilAllocation[i]);
        }

        vkDestroySwapchainKHR(context.device, swapChain.swapChainKHR, nullptr);
    }

    void VulkanSurface::createFramebufferFormat() {
        auto& context = VulkanContext::getInstance();

        VkAttachmentDescription descriptions[2] = {};

        descriptions[0].format = surfaceFormat.format;
        descriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
        descriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        descriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        descriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        descriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        descriptions[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        descriptions[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        descriptions[1].format = swapChain.depthFormat;
        descriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
        descriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        descriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        descriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        descriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
        descriptions[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        descriptions[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference references[2] = {};

        references[0].attachment = 0;
        references[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        references[1].attachment = 1;
        references[1].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDependency dependency = {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &references[0];
        subpass.pDepthStencilAttachment = &references[1];

        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 2;
        renderPassInfo.pAttachments = descriptions;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        VkResult result;
        VkRenderPass renderPass;

        result = vkCreateRenderPass(context.device, &renderPassInfo, nullptr, &renderPass);

        VK_RESULT_ASSERT(result, "Failed to create render pass for surface");

        auto &format = swapChain.framebufferFormat;
        format.renderPass = renderPass;
        format.useDepthStencil = true;
        format.numOfAttachments = 2;
    }

    void VulkanSurface::destroyFramebufferFormat() {
        auto& context = VulkanContext::getInstance();
        vkDestroyRenderPass(context.device, swapChain.framebufferFormat.renderPass, nullptr);
    }

    void VulkanSurface::createFramebuffers() {
        auto& context = VulkanContext::getInstance();
        auto &framebuffers = swapChain.framebuffers;

        framebuffers.resize(swapChain.imageViews.size());

        for (size_t i = 0; i < framebuffers.size(); i++) {
            VkImageView imageViews[2] = { swapChain.imageViews[i], swapChain.depthStencilImageViews[i] };

            VkFramebufferCreateInfo framebufferInfo = {};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.pNext = nullptr;
            framebufferInfo.flags = 0;
            framebufferInfo.width = swapChain.extent.width;
            framebufferInfo.height = swapChain.extent.height;
            framebufferInfo.layers = 1;
            framebufferInfo.attachmentCount = 2;
            framebufferInfo.pAttachments = imageViews;
            framebufferInfo.renderPass = swapChain.framebufferFormat.renderPass;

            auto result = vkCreateFramebuffer(context.device, &framebufferInfo, nullptr, &framebuffers[i]);
            VK_RESULT_ASSERT(result, "Filed to create framebuffer for surface");
        }
    }

    void VulkanSurface::destroyFramebuffers() {
        auto& context = VulkanContext::getInstance();

        for (auto &framebuffer: swapChain.framebuffers) {
            vkDestroyFramebuffer(context.device, framebuffer, nullptr);
        }
    }

    void VulkanSurface::updateSurfaceCapabilities() {
        auto& context = VulkanContext::getInstance();
        auto result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context.physicalDevice, surfaceKHR, &surfaceCapabilities);
        VK_RESULT_ASSERT(result, "Failed to get surface capabilities");
    }

    void VulkanSurface::resizeSurface() {
        updateSurfaceCapabilities();
        const auto& extent = surfaceCapabilities.currentExtent;

        if (extent.width != width || extent.height != height) {
            width = extent.width;
            height = extent.height;

            if (width == 0 || height == 0) {
                canPresentImages = false;
                return;
            }

            destroyFramebuffers();
            destroySwapChain();
            createSwapChain();
            createFramebuffers();

            canPresentImages = true;
        }
    }

    void VulkanSurface::acquireFirstImage() {
        imageAvailable.reset();
        acquireNextImage();
    }

    void VulkanSurface::acquireNextImage() {
        auto& context = VulkanContext::getInstance();
        bool acquire = false;

        while (!acquire) {
            auto result = vkAcquireNextImageKHR(
                    context.device,
                    swapChain.swapChainKHR,
                    UINT64_MAX,
                    VK_NULL_HANDLE,
                    imageAvailable.get(),
                    &currentImageIndex
            );

            if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
                resizeSurface();

                if (!canPresentImages) {
                    // if a window minimized
                    // go out and disallow rendering to the window
                    return;
                }

                continue;
            }
            else  {
                VK_RESULT_ASSERT(result, "Failed to acquire next image index");
            }

            acquire = true;
        }

        imageAvailable.wait();
        imageAvailable.reset();
    }

    void VulkanSurface::findPresentsFamily() {
        auto &context = VulkanContext::getInstance();

        uint32 queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(context.physicalDevice, &queueFamilyCount, nullptr);

        presentsFamily = 0xffffffff;
        uint32 graphicsFamily = context.familyIndices.graphicsFamily.get();

        VkBool32 supported = 0x0;
        vkGetPhysicalDeviceSurfaceSupportKHR(context.physicalDevice, graphicsFamily, surfaceKHR, &supported);

        if (supported) {
            presentsFamily = graphicsFamily;
        } else {
            for (uint32 i = 0; i < queueFamilyCount; i++) {
                vkGetPhysicalDeviceSurfaceSupportKHR(context.physicalDevice, i, surfaceKHR, &supported);
                if (supported) {
                    presentsFamily = i;
                }
            }
        }

        if (presentsFamily == 0xffffffff) {
            throw VulkanException("Surface does not support present queue mode");
        }

        if (presentsFamily == graphicsFamily) {
            presentQueue = context.graphicsQueue;
        } else {

            vkGetDeviceQueue(context.device, presentsFamily, 0, &presentQueue);

            if (presentQueue == VK_NULL_HANDLE) {
                throw VulkanException("Failed to get present queue");
            }
        }

#ifdef MODE_DEBUG
        printf("Found queue family [present: %u]\n", presentsFamily);
#endif
    }

    void VulkanSurface::getSurfaceProperties(std::vector<VkSurfaceFormatKHR> &outSurfaceFormats, std::vector<VkPresentModeKHR> &outPresentModes) {
        uint32 surfFormatCount;
        uint32 presentModeCount;
        VkResult result;
        VkPhysicalDevice physicalDevice = VulkanContext::getInstance().physicalDevice;

        result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surfaceKHR, &surfFormatCount, nullptr);
        VK_RESULT_ASSERT(result, "Failed to get VkSurfaceKHR formats");

        if (surfFormatCount == 0) {
            throw VulkanException("VkSurfaceKHR has no formats");
        }

        outSurfaceFormats.resize(surfFormatCount);

        result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surfaceKHR, &surfFormatCount, outSurfaceFormats.data());
        VK_RESULT_ASSERT(result, "Failed to get VkSurfaceKHR formats");

        result = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surfaceKHR, &presentModeCount, nullptr);
        VK_RESULT_ASSERT(result, "Failed to get VkSurfaceKHR present modes");

        if (presentModeCount == 0) {
            throw VulkanException("VkSurfaceKHR has no present modes");
        }

        outPresentModes.resize(presentModeCount);

        result = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surfaceKHR, &presentModeCount, outPresentModes.data());
        VK_RESULT_ASSERT(result, "Failed to get VkSurfaceKHR present modes");
    }

    VkExtent2D VulkanSurface::getSwapChainExtent(uint32 preferredWidth, uint32 preferredHeight) {
        if (surfaceCapabilities.currentExtent.width != UINT32_MAX) {
            // if current extent is defined, match swap chain size with it
            return surfaceCapabilities.currentExtent;
        } else {
            VkExtent2D ext;

            ext.width = preferredWidth;
            ext.height = preferredHeight;

            // min <= preferred width <= max
            if (ext.width < surfaceCapabilities.minImageExtent.width) {
                ext.width = surfaceCapabilities.minImageExtent.width;
            } else if (ext.width > surfaceCapabilities.maxImageExtent.width) {
                ext.width = surfaceCapabilities.maxImageExtent.width;
            }

            // min <= preferred height <= max
            if (ext.height < surfaceCapabilities.minImageExtent.height) {
                ext.height = surfaceCapabilities.minImageExtent.height;
            } else if (ext.height > surfaceCapabilities.maxImageExtent.height) {
                ext.height = surfaceCapabilities.maxImageExtent.height;
            }

            return ext;
        }
    }

    VkCompositeAlphaFlagBitsKHR VulkanSurface::getAvailableCompositeAlpha() {
        VkCompositeAlphaFlagBitsKHR compositeAlphaPr[4] = {
                VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
                VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
                VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR
        };

        for (auto compositeAlpha: compositeAlphaPr) {
            if (surfaceCapabilities.supportedCompositeAlpha & compositeAlpha) {
                return compositeAlpha;
            }
        }

        throw VulkanException("Failed to find available composite alpha");
    }
    
} // namespace ignimbrite