//
// Created by Egor Orachyov on 2019-11-11.
//

#include <ignimbrite/RenderDeviceDefinitions.h>
#include <ignimbrite/Compilation.h>
#include <VulkanUtils.h>
#include <exception>
#include <cstring>
#include <set>
#include <array>

namespace ignimbrite {

    VkFormatProperties VulkanUtils::getDeviceFormatProperties(VulkanContext &context, VkFormat format) {
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(context.physicalDevice, format, &properties);
        return properties;
    }

    VkFormat VulkanUtils::findSupportedFormat(ignimbrite::VulkanContext &context, const VkFormat *candidates,
                                              ignimbrite::uint32 candidatesCount, VkImageTiling tiling,
                                              VkFormatFeatureFlags features) {
        for (uint32 i = 0; i < candidatesCount; i++) {
            auto format = candidates[i];
            auto properties = getDeviceFormatProperties(context, candidates[i]);

            if (tiling == VK_IMAGE_TILING_LINEAR &&
                (properties.linearTilingFeatures & features) == features) {
                return candidates[i];
            }
            else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
                (properties.optimalTilingFeatures & features) == features) {
                return format;
            }
        }

        throw VulkanException("Failed to find supported format");
    }

    uint32 VulkanUtils::getMemoryTypeIndex(VulkanContext &context, uint32 memoryTypeBits, VkFlags requirementsMask) {
        // for each memory type available for this device
        for (uint32 i = 0; i < context.deviceMemoryProperties.memoryTypeCount; i++) {
            // if type is available
            if ((memoryTypeBits & 1u) == 1) {
                if ((context.deviceMemoryProperties.memoryTypes[i].propertyFlags & requirementsMask) ==
                    requirementsMask) {
                    return i;
                }
            }

            memoryTypeBits >>= 1u;
        }

        throw VulkanException("Can't find memory type in device memory properties");
    }

    void VulkanUtils::createBuffer(VulkanContext &context, VkDeviceSize size, VkBufferUsageFlags usage,
                                   VkMemoryPropertyFlags properties,
                                   VkBuffer &outBuffer, VkDeviceMemory &outBufferMemory) {
        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = (VkDeviceSize) size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VkResult result = vkCreateBuffer(context.device, &bufferInfo, nullptr, &outBuffer);
        if (result != VK_SUCCESS) {
            throw VulkanException("Can't create buffer for vertex data");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(context.device, outBuffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = getMemoryTypeIndex(context, memRequirements.memoryTypeBits, properties);

        result = vkAllocateMemory(context.device, &allocInfo, nullptr, &outBufferMemory);
        if (result != VK_SUCCESS) {
            throw VulkanException("Can't allocate memory for vertex buffer");
        }

        result = vkBindBufferMemory(context.device, outBuffer, outBufferMemory, 0);
        if (result != VK_SUCCESS) {
            throw VulkanException("Can't bind buffer memory for vertex buffer");
        }
    }

    void
    VulkanUtils::createBufferLocal(VulkanContext &context, const void *data, VkDeviceSize size,
                                   VkBufferUsageFlags usage,
                                   VkBuffer &outBuffer, VkDeviceMemory &outBufferMemory) {
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;

        createBuffer(context,
                     size,
                     VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     stagingBuffer,
                     stagingBufferMemory);

        updateBufferMemory(context, stagingBufferMemory, 0, size, data);

        createBuffer(context,
                     size,
                     usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                     outBuffer,
                     outBufferMemory);

        copyBuffer(context, stagingBuffer, outBuffer, size);

        vkDestroyBuffer(context.device, stagingBuffer, nullptr);
        vkFreeMemory(context.device, stagingBufferMemory, nullptr);
    }

    void VulkanUtils::copyBuffer(VulkanContext &context, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
        VkCommandBuffer commandBuffer = beginTempCommandBuffer(context, context.transferTempCommandPool);

        VkBufferCopy copyRegion = {};
        copyRegion.size = size;
        copyRegion.dstOffset = 0;
        copyRegion.srcOffset = 0;

        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        endTempCommandBuffer(context, commandBuffer, context.transferQueue, context.transferTempCommandPool);
    }

    void VulkanUtils::updateBufferMemory(VulkanContext &context, VkDeviceMemory bufferMemory, VkDeviceSize offset,
                                         VkDeviceSize size,
                                         const void *data) {
        void *mappedData;
        VkResult result;
        result = vkMapMemory(context.device, bufferMemory, offset, size, 0, &mappedData);

        if (result != VK_SUCCESS) {
            throw VulkanException("Failed to map memory buffer");
        }

        std::memcpy(mappedData, data, (size_t) size);
        vkUnmapMemory(context.device, bufferMemory);
    }

    void VulkanUtils::createTextureImage(VulkanContext &context, const void *imageData,
                                         uint32 width, uint32 height,
                                         uint32 depth, uint32 mipLevels,
                                         VkImageType imageType, VkFormat format, VkImageTiling tiling,
                                         VkImage &outTextureImage, VkDeviceMemory &outTextureMemory,
                                         VkImageLayout textureLayout) {
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;

        VkDeviceSize imageSize = (VkDeviceSize) width * height * depth;

        // create staging buffer to create image in device local memory
        createBuffer(context,
                     imageSize,
                     VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     stagingBuffer, stagingBufferMemory);

        if (imageData != nullptr) {
            updateBufferMemory(context, stagingBufferMemory, 0, imageSize, imageData);
        }

        createImage(context, width, height, depth, mipLevels, imageType, format, tiling,
                // for copying and sampling in shaders
                    VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                // TODO: updatable from cpu ??
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    outTextureImage, outTextureMemory);

        // layout transition from undefined
        // to transfer destination to prepare image for copying
        transitionImageLayout(context, outTextureImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                              mipLevels);

        // copy without mipmaps
        copyBufferToImage(context, stagingBuffer, outTextureImage, width, height, depth);

        vkDestroyBuffer(context.device, stagingBuffer, nullptr);
        vkFreeMemory(context.device, stagingBufferMemory, nullptr);

        // generate mipmaps and layout transition
        // from transfer destination to shader readonly
        generateMipmaps(context, outTextureImage, format, width, height, mipLevels, textureLayout);
    }

    void VulkanUtils::createImage(VulkanContext &context, uint32 width, uint32 height,
                                  uint32 depth, uint32 mipLevels,
                                  VkImageType imageType, VkFormat format, VkImageTiling tiling,
                                  VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
                                  VkImage &outImage, VkDeviceMemory &outImageMemory) {

        VkImageCreateInfo imageInfo = {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = imageType;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = depth;
        imageInfo.mipLevels = mipLevels;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VkResult result = vkCreateImage(context.device, &imageInfo, nullptr, &outImage);

        if (result != VK_SUCCESS) {
            throw VulkanException("Failed to create image");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(context.device, outImage, &memRequirements);

        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = getMemoryTypeIndex(context, memRequirements.memoryTypeBits, properties);

        result = vkAllocateMemory(context.device, &allocInfo, nullptr, &outImageMemory);

        if (result != VK_SUCCESS) {
            throw VulkanException("Failed to allocate memory for image");
        }

        result = vkBindImageMemory(context.device, outImage, outImageMemory, 0);
        if (result != VK_SUCCESS) {
            throw VulkanException("Failed to bind image memory");
        }
    }

    void
    VulkanUtils::copyBufferToImage(VulkanContext &context, VkBuffer buffer, VkImage image, uint32 width, uint32 height,
                                   uint32 depth) {
        VkCommandBuffer commandBuffer = beginTempCommandBuffer(context, context.transferTempCommandPool);

        VkBufferImageCopy region = {};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        // this function copies without mipmaps
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = {0, 0, 0};
        region.imageExtent = {width, height, depth};

        vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        endTempCommandBuffer(context, commandBuffer, context.transferQueue, context.transferTempCommandPool);
    }

    void VulkanUtils::transitionImageLayout(VulkanContext &context, VkImage image, VkImageLayout oldLayout,
                                            VkImageLayout newLayout, uint32 mipLevels) {
        VkCommandBuffer commandBuffer = beginTempCommandBuffer(context, context.transferTempCommandPool);

        VkImageMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = mipLevels;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            // undefined to transfer destination

            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
                   newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            // transfer destination to fragment shader

            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else {
            throw VulkanException("Unimplemented layout transition");
        }

        vkCmdPipelineBarrier(
                commandBuffer,
                sourceStage,
                destinationStage,
                0, 0, nullptr, 0, nullptr, 1,
                &barrier
        );

        endTempCommandBuffer(context, commandBuffer, context.transferQueue, context.transferTempCommandPool);
    }

    void VulkanUtils::createImageView(VulkanContext &context, VkImageView &outImageView, VkImage image,
                                      VkImageViewType viewType,
                                      VkFormat format, const VkImageSubresourceRange &subResourceRange,
                                      VkComponentMapping components) {
        VkImageViewCreateInfo imageViewInfo = {};
        imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewInfo.image = image;
        imageViewInfo.viewType = viewType;
        imageViewInfo.format = format;
        imageViewInfo.components = components;
        imageViewInfo.subresourceRange = subResourceRange;

        VkResult result = vkCreateImageView(context.device, &imageViewInfo, nullptr, &outImageView);

        if (result != VK_SUCCESS) {
            throw VulkanException("Failed to create image view");
        }
    }

    void
    VulkanUtils::generateMipmaps(VulkanContext &context, VkImage image, VkFormat format, uint32 width, uint32 height,
                                 uint32 mipLevels, VkImageLayout newLayout) {
        VkFormatProperties formatProperties = getDeviceFormatProperties(context, format);

        if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
            throw VulkanException("Failed to generate mipmaps as specified format doesn't support linear blitting");
        }

        VkCommandBuffer commandBuffer = beginTempCommandBuffer(context, context.transferTempCommandPool);

        VkImageMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.image = image;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.subresourceRange.levelCount = 1;

        int32_t mipWidth = width;
        int32_t mipHeight = height;

        // i=0 is original image
        for (uint32 i = 1; i < mipLevels; i++) {
            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

            vkCmdPipelineBarrier(commandBuffer,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                                 0, nullptr,
                                 0, nullptr,
                                 1, &barrier);

            VkImageBlit blit = {};

            // source
            blit.srcOffsets[0] = {0, 0, 0};
            blit.srcOffsets[1] = {mipWidth, mipHeight, 1};
            blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.srcSubresource.mipLevel = i - 1;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount = 1;

            // destination, divided
            blit.dstOffsets[0] = {0, 0, 0};
            blit.dstOffsets[1] = {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1};
            blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.dstSubresource.mipLevel = i;
            blit.dstSubresource.baseArrayLayer = 0;
            blit.dstSubresource.layerCount = 1;

            vkCmdBlitImage(
                    commandBuffer,
                    image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    1, &blit,
                    // using linear interpolation
                    VK_FILTER_LINEAR
           );

            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(
                    commandBuffer,
                    VK_PIPELINE_STAGE_TRANSFER_BIT,
                    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                    0, nullptr,
                    0, nullptr,
                    1, &barrier
            );

            if (mipWidth > 1) {
                mipWidth /= 2;
            }

            if (mipHeight > 1) {
                mipHeight /= 2;
            }
        }

        barrier.subresourceRange.baseMipLevel = mipLevels - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = newLayout;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(
                commandBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                0, nullptr,
                0, nullptr,
                1, &barrier
        );

        endTempCommandBuffer(context, commandBuffer, context.transferQueue, context.transferTempCommandPool);
    }

    void VulkanUtils::getSurfaceProperties(VkPhysicalDevice physicalDevice, VkSurfaceKHR surfaceKHR,
                                           std::vector<VkSurfaceFormatKHR> &outSurfaceFormats,
                                           std::vector<VkPresentModeKHR> &outPresentModes) {
        uint32 surfFormatCount;
        uint32 presentModeCount;
        VkResult result;

        result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surfaceKHR, &surfFormatCount, nullptr);
        if (result != VK_SUCCESS) {
            throw VulkanException("Failed to get VkSurfaceKHR formats");
        }

        if (surfFormatCount == 0) {
            throw VulkanException("VkSurfaceKHR has no formats");
        }

        outSurfaceFormats.resize(surfFormatCount);

        result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surfaceKHR, &surfFormatCount,
                                                      outSurfaceFormats.data());
        if (result != VK_SUCCESS) {
            throw VulkanException("Failed to get VkSurfaceKHR formats");
        }

        result = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surfaceKHR, &presentModeCount, nullptr);
        if (result != VK_SUCCESS) {
            throw VulkanException("Failed to get VkSurfaceKHR present modes");
        }

        if (presentModeCount == 0) {
            throw VulkanException("VkSurfaceKHR has no present modes");
        }

        outPresentModes.resize(presentModeCount);

        result = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surfaceKHR, &presentModeCount,
                                                           outPresentModes.data());
        if (result != VK_SUCCESS) {
            throw VulkanException("Failed to get VkSurfaceKHR present modes");
        }
    }

    VkExtent2D VulkanUtils::getSwapChainExtent(uint32 preferredWidth, uint32 preferredHeight,
                                               const VkSurfaceCapabilitiesKHR &surfaceCapabilities) {
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

    VkCompositeAlphaFlagBitsKHR
    VulkanUtils::getAvailableCompositeAlpha(const VkSurfaceCapabilitiesKHR &surfaceCapabilities) {
        VkCompositeAlphaFlagBitsKHR compositeAlphaPr[4] = {
                VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
                VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
                VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR
        };

        for (uint32 i = 0; i < 4; i++) {
            if (surfaceCapabilities.supportedCompositeAlpha & compositeAlphaPr[i]) {
                return compositeAlphaPr[i];
            }
        }

        throw VulkanException("Failed to find available composite alpha");
    }

    void VulkanUtils::createDepthStencilBuffer(VulkanContext &context, uint32 width, uint32 height, uint32 depth,
                                               VkImageType imageType, VkFormat format, VkImage &outImage,
                                               VkDeviceMemory &outImageMemory, VkImageUsageFlags usageFlags) {

        // get properties of depth stencil format
        const VkFormatProperties &properties = getDeviceFormatProperties(context, format);

        VkImageTiling tiling;

        if (properties.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            tiling = VK_IMAGE_TILING_LINEAR;
        } else if (properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            tiling = VK_IMAGE_TILING_OPTIMAL;
        } else {
            throw VulkanException("Unsupported depth format");
        }

        createImage(context, width, height, depth, 1,
                    imageType, format, tiling, usageFlags,
                // depth stencil buffer is device local
                // TODO: make visible from cpu
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    outImage, outImageMemory
        );
    }

    void VulkanUtils::allocateDescriptorPool(VulkanContext &context, VulkanUniformLayout &layout) {
        VkResult result;
        VkDescriptorPool pool;

        VkDescriptorPoolSize poolSizes[2];
        uint32 poolSizesCount = 0;

        if (layout.buffersCount > 0) {
            poolSizes[poolSizesCount].descriptorCount = layout.buffersCount;
            poolSizes[poolSizesCount].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            poolSizesCount += 1;
        }

        if (layout.texturesCount > 0) {
            poolSizes[poolSizesCount].descriptorCount = layout.texturesCount;
            poolSizes[poolSizesCount].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            poolSizesCount += 1;
        }

        VkDescriptorPoolCreateInfo poolCreateInfo = {};
        poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolCreateInfo.poolSizeCount = poolSizesCount;
        poolCreateInfo.pPoolSizes = poolSizes;
        poolCreateInfo.maxSets = VulkanContext::DESCRIPTOR_POOL_MAX_SET_COUNT;

        result = vkCreateDescriptorPool(context.device, &poolCreateInfo, nullptr, &pool);

        if (result != VK_SUCCESS) {
            throw VulkanException("Failed to create descriptor pool");
        }

        VulkanDescriptorPool vulkanDescriptorPool = {};
        vulkanDescriptorPool.allocatedSets = 0;
        vulkanDescriptorPool.maxSets = VulkanContext::DESCRIPTOR_POOL_MAX_SET_COUNT;
        vulkanDescriptorPool.pool = pool;

        layout.pools.push_back(vulkanDescriptorPool);
    }

    VulkanDescriptorPool &VulkanUtils::getAvailableDescriptorPool(VulkanContext &context, VulkanUniformLayout &layout) {
        for (auto &pool: layout.pools) {
            if (pool.allocatedSets < pool.maxSets) {
                return pool;
            }
        }

        allocateDescriptorPool(context, layout);
        return layout.pools.back();
    }

    VkDescriptorSet VulkanUtils::getAvailableDescriptorSet(ignimbrite::VulkanContext &context,
                                                           ignimbrite::VulkanUniformLayout &layout) {
        VkDescriptorSet descriptorSet;

        if (layout.freeSets.empty()) {
            VkResult result;

            auto &pool = VulkanUtils::getAvailableDescriptorPool(context, layout);

            VkDescriptorSetAllocateInfo descSetAllocInfo = {};
            descSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            descSetAllocInfo.pNext = nullptr;
            descSetAllocInfo.descriptorPool = pool.pool;
            descSetAllocInfo.descriptorSetCount = 1;
            descSetAllocInfo.pSetLayouts = &layout.setLayout;

            result = vkAllocateDescriptorSets(context.device, &descSetAllocInfo, &descriptorSet);

            if (result != VK_SUCCESS) {
                throw VulkanException("Can't allocate descriptor set from descriptor pool");
            }

            pool.allocatedSets += 1;
            layout.usedDescriptorSets += 1;
        }
        else {
            descriptorSet = layout.freeSets.back();
            layout.freeSets.pop_back();
            layout.usedDescriptorSets += 1;
        }

        return descriptorSet;
    }

    void
    VulkanUtils::createVertexInputState(const VulkanVertexLayout &layout, VkPipelineVertexInputStateCreateInfo &state) {
        const auto &bindings = layout.vkBindings;
        const auto &attributes = layout.vkAttributes;

        state.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        state.vertexBindingDescriptionCount = (uint32) bindings.size();
        state.pVertexBindingDescriptions = bindings.data();
        state.vertexAttributeDescriptionCount = (uint32) attributes.size();
        state.pVertexAttributeDescriptions = attributes.data();
    }

    void VulkanUtils::createInputAssembly(PrimitiveTopology topology,
                                          VkPipelineInputAssemblyStateCreateInfo &inputAssembly) {
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VulkanDefinitions::primitiveTopology(topology);
        inputAssembly.primitiveRestartEnable = VK_FALSE;
    }

    void VulkanUtils::createViewportState(VkViewport &viewport, VkRect2D &scissor,
                                          VkPipelineViewportStateCreateInfo &state) {
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = 640;
        viewport.height = 480;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        scissor.offset = {0, 0};
        scissor.extent = {640, 480};

        state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        state.viewportCount = 1;
        state.pViewports = &viewport;
        state.scissorCount = 1;
        state.pScissors = &scissor;
    }

    void VulkanUtils::createRasterizationState(const RenderDevice::PipelineRasterizationDesc &rasterizationDesc,
                                               VkPipelineRasterizationStateCreateInfo &rasterizer) {
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VulkanDefinitions::polygonMode(rasterizationDesc.mode);
        rasterizer.lineWidth = rasterizationDesc.lineWidth;
        rasterizer.cullMode = VulkanDefinitions::cullModeFlagBits(rasterizationDesc.cullMode);
        rasterizer.frontFace = VulkanDefinitions::frontFace(rasterizationDesc.frontFace);
        rasterizer.depthBiasEnable = VK_FALSE;
        rasterizer.depthBiasConstantFactor = 0.0f; // Optional
        rasterizer.depthBiasClamp = 0.0f; // Optional
        rasterizer.depthBiasSlopeFactor = 0.0f; // Optional
    }

    void VulkanUtils::createPipelineLayout(VulkanContext &context, const VulkanUniformLayout &uniformLayout,
                                           VkPipelineLayout &pipelineLayout) {
        VkResult result;

        VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &uniformLayout.setLayout;
        pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
        pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

        result = vkCreatePipelineLayout(context.device, &pipelineLayoutInfo, nullptr, &pipelineLayout);

        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to create pipeline layout");
        }
    }

    void VulkanUtils::createMultisampleState(VkPipelineMultisampleStateCreateInfo &state) {
        state.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        state.sampleShadingEnable = VK_FALSE;
        state.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        state.minSampleShading = 1.0f; // Optional
        state.pSampleMask = nullptr; // Optional
        state.alphaToCoverageEnable = VK_FALSE; // Optional
        state.alphaToOneEnable = VK_FALSE; // Optional
    }

    void VulkanUtils::createColorBlendAttachmentState(const RenderDevice::BlendAttachmentDesc &attachmentDesc,
                                                      VkPipelineColorBlendAttachmentState &state) {
        state.colorWriteMask = VulkanDefinitions::colorComponentFlags(
                attachmentDesc.writeR,
                attachmentDesc.writeG,
                attachmentDesc.writeB,
                attachmentDesc.writeA
        );
        state.blendEnable = attachmentDesc.blendEnable;
        state.srcColorBlendFactor = VulkanDefinitions::blendFactor(attachmentDesc.srcColorBlendFactor);
        state.dstColorBlendFactor = VulkanDefinitions::blendFactor(attachmentDesc.dstColorBlendFactor);
        state.colorBlendOp = VulkanDefinitions::blendOperation(attachmentDesc.colorBlendOp);
        state.srcAlphaBlendFactor = VulkanDefinitions::blendFactor(attachmentDesc.srcAlphaBlendFactor);
        state.dstAlphaBlendFactor = VulkanDefinitions::blendFactor(attachmentDesc.dstAlphaBlendFactor);
        state.alphaBlendOp = VulkanDefinitions::blendOperation(attachmentDesc.alphaBlendOp);
    }

    void
    VulkanUtils::createColorBlendState(const RenderDevice::PipelineBlendStateDesc &stateDesc, uint32 attachmentsCount,
                                       const VkPipelineColorBlendAttachmentState *attachments,
                                       VkPipelineColorBlendStateCreateInfo &stateCreateInfo) {
        stateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        stateCreateInfo.logicOpEnable = stateDesc.logicOpEnable;
        stateCreateInfo.logicOp = VulkanDefinitions::logicOperation(stateDesc.logicOp);
        stateCreateInfo.attachmentCount = attachmentsCount;
        stateCreateInfo.pAttachments = attachments;
        stateCreateInfo.blendConstants[0] = stateDesc.blendConstants[0];
        stateCreateInfo.blendConstants[1] = stateDesc.blendConstants[1];
        stateCreateInfo.blendConstants[2] = stateDesc.blendConstants[2];
        stateCreateInfo.blendConstants[3] = stateDesc.blendConstants[3];
    }

    void VulkanUtils::createSurfaceColorBlendState(const RenderDevice::PipelineSurfaceBlendStateDesc &stateDesc,
                                                   const VkPipelineColorBlendAttachmentState *attachment,
                                                   VkPipelineColorBlendStateCreateInfo &stateCreateInfo) {
        stateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        stateCreateInfo.logicOpEnable = stateDesc.logicOpEnable;
        stateCreateInfo.logicOp = VulkanDefinitions::logicOperation(stateDesc.logicOp);
        stateCreateInfo.attachmentCount = 1;
        stateCreateInfo.pAttachments = attachment;
        stateCreateInfo.blendConstants[0] = stateDesc.blendConstants[0];
        stateCreateInfo.blendConstants[1] = stateDesc.blendConstants[1];
        stateCreateInfo.blendConstants[2] = stateDesc.blendConstants[2];
        stateCreateInfo.blendConstants[3] = stateDesc.blendConstants[3];
    }

    void VulkanUtils::createDepthStencilState(const RenderDevice::PipelineDepthStencilStateDesc &desc,
                                              VkPipelineDepthStencilStateCreateInfo &stateCreateInfo) {
        stateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        stateCreateInfo.pNext = nullptr;
        stateCreateInfo.depthTestEnable = desc.depthTestEnable;
        stateCreateInfo.depthWriteEnable = desc.depthWriteEnable;
        stateCreateInfo.minDepthBounds = 0.0f;
        stateCreateInfo.maxDepthBounds = 0.1f;
        stateCreateInfo.depthBoundsTestEnable = VK_FALSE;
        stateCreateInfo.depthCompareOp = VulkanDefinitions::compareOperation(desc.depthCompareOp);
        stateCreateInfo.stencilTestEnable = desc.stencilTestEnable;
        stateCreateInfo.front = createStencilOperationState(desc.front);
        stateCreateInfo.back = createStencilOperationState(desc.back);
    }

    VkStencilOpState VulkanUtils::createStencilOperationState(const RenderDevice::StencilOpStateDesc &desc) {
        VkStencilOpState state = {};
        state.compareMask = desc.compareMask;
        state.reference = desc.reference;
        state.writeMask = desc.writeMask;
        state.compareOp = VulkanDefinitions::compareOperation(desc.compareOp);
        state.failOp = VulkanDefinitions::stencilOperation(desc.failOp);
        state.depthFailOp = VulkanDefinitions::stencilOperation(desc.depthFailOp);
        state.passOp = VulkanDefinitions::stencilOperation(desc.passOp);
        return state;
    }

    VkCommandPool
    VulkanUtils::createCommandPool(VulkanContext &context, VkCommandPoolCreateFlags flags, uint32_t queueFamilyIndex) {
        VkCommandPoolCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        info.pNext = nullptr;
        info.queueFamilyIndex = queueFamilyIndex;
        info.flags = flags;

        VkCommandPool commandPool;
        VkResult result = vkCreateCommandPool(context.device, &info, nullptr, &commandPool);

        if (result != VK_SUCCESS) {
            throw VulkanException("Failed to create command pool");
        }

        return commandPool;
    }

    VkCommandBuffer VulkanUtils::beginTempCommandBuffer(VulkanContext &context, VkCommandPool commandPool) {
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        VkResult result = vkAllocateCommandBuffers(context.device, &allocInfo, &commandBuffer);

        if (result != VK_SUCCESS) {
            throw VulkanException("Failed to allocate command buffer");
        }

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        result = vkBeginCommandBuffer(commandBuffer, &beginInfo);

        if (result != VK_SUCCESS) {
            throw VulkanException("Failed to begin command buffer");
        }

        return commandBuffer;
    }

    void VulkanUtils::endTempCommandBuffer(VulkanContext &context, VkCommandBuffer commandBuffer,
                                           VkQueue queue, VkCommandPool commandPool) {

        VkResult result = vkEndCommandBuffer(commandBuffer);

        if (result != VK_SUCCESS) {
            throw VulkanException("Failed to end command buffer");
        }

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        result = vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);

        if (result != VK_SUCCESS) {
            throw VulkanException("Failed to submit queue");
        }

        result = vkQueueWaitIdle(queue);

        if (result != VK_SUCCESS) {
            throw VulkanException("Error on vkQueueWaitIdle");
        }

        vkFreeCommandBuffers(context.device, commandPool, 1, &commandBuffer);
    }

} // namespace ignimbrite