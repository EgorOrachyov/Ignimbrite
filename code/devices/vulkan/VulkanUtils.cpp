/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019, 2020  Egor Orachyov                                        */
/* Copyright (c) 2019, 2020  Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#include <VulkanDefinitions.h>
#include <VulkanContext.h>
#include <VulkanUtils.h>
#include <cstring>
#include <array>
#include <set>

namespace ignimbrite {

    void VulkanUtils::getSupportedFormats(std::vector<ignimbrite::DataFormat> &formats) {
        const DataFormat known[] = {
                DataFormat::R8G8B8_UNORM,
                DataFormat::R8G8B8A8_UNORM,

                DataFormat::R32_SFLOAT,
                DataFormat::R32G32_SFLOAT,
                DataFormat::R32G32B32_SFLOAT,
                DataFormat::R32G32B32A32_SFLOAT,

                DataFormat::D24_UNORM_S8_UINT,
                DataFormat::D32_SFLOAT_S8_UINT,
        };

        for (auto f: known) {
            auto properties = getDeviceFormatProperties(VulkanDefinitions::dataFormat(f));

            if (properties.bufferFeatures || properties.linearTilingFeatures || properties.optimalTilingFeatures)
                formats.push_back(f);
        }
    }

    VkFormatProperties VulkanUtils::getDeviceFormatProperties(VkFormat format) {
        auto& context = VulkanContext::getInstance();
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(context.physicalDevice, format, &properties);
        return properties;
    }

    VkFormat VulkanUtils::findSupportedFormat(const VkFormat *candidates,
                                              uint32 candidatesCount, VkImageTiling tiling,
                                              VkFormatFeatureFlags features) {
        for (uint32 i = 0; i < candidatesCount; i++) {
            auto format = candidates[i];
            auto properties = getDeviceFormatProperties(candidates[i]);

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

    uint32 VulkanUtils::getMemoryTypeIndex(uint32 memoryTypeBits, VkFlags requirementsMask) {
        auto& context = VulkanContext::getInstance();

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

    void VulkanUtils::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                                   VkMemoryPropertyFlags properties,
                                   VkBuffer &outBuffer, VulkanAllocation &outAllocation) {
        auto& context = VulkanContext::getInstance();

        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = (VkDeviceSize) size;
        bufferInfo.usage = usage;

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.requiredFlags = properties;

        VmaAllocationInfo outAllocInfo;
        VkResult r = vmaCreateBuffer(context.vmAllocator, &bufferInfo, &allocInfo, &outBuffer, &outAllocation.vmaAllocation, &outAllocInfo);
        VK_RESULT_ASSERT(r, "Failed to create buffer with Vulkan memory allocator");

        outAllocation.memory = outAllocInfo.deviceMemory;
        outAllocation.offset = outAllocInfo.offset;
    }

    void
    VulkanUtils::createBufferLocal(const void *data, VkDeviceSize size,
                                   VkBufferUsageFlags usage,
                                   VkBuffer &outBuffer, VulkanAllocation &outAllocation) {
        auto& context = VulkanContext::getInstance();

        // create staging buffer
        VkBufferCreateInfo stagingBufferInfo = {};
        stagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        stagingBufferInfo.size = (VkDeviceSize) size;
        stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

        VmaAllocationCreateInfo stagingAllocInfo = {};
        stagingAllocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
        //stagingAllocInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

        VkBuffer stagingBuffer;
        VmaAllocationInfo outStagingAllocInfo;
        VulkanAllocation stagingAllocation = {};

        VkResult r = vmaCreateBuffer(context.vmAllocator, &stagingBufferInfo, &stagingAllocInfo,
                &stagingBuffer, &stagingAllocation.vmaAllocation, &outStagingAllocInfo);
        VK_RESULT_ASSERT(r, "Failed to create buffer with Vulkan memory allocator");

        // map and fill it
        updateBufferMemory(stagingAllocation, 0, size, data);

        // create main buffer
        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = (VkDeviceSize) size;
        // also set it as copy destination
        bufferInfo.usage = usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

        VmaAllocationCreateInfo allocInfo = {};
        // fastest access from gpu
        allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        //allocInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        VmaAllocationInfo outAllocInfo;
        r = vmaCreateBuffer(context.vmAllocator, &bufferInfo, &allocInfo, &outBuffer, &outAllocation.vmaAllocation, &outAllocInfo);
        VK_RESULT_ASSERT(r, "Failed to create buffer");

        outAllocation.memory = outAllocInfo.deviceMemory;
        outAllocation.offset = outAllocInfo.offset;

        // submit a transfer from staging to main
        VkBufferCopy copyRegion = {};
        copyRegion.size = size;
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;

        copyBuffer(stagingBuffer, outBuffer, &copyRegion);

        destroyBuffer(stagingBuffer, stagingAllocation);
    }

    void VulkanUtils::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, const VkBufferCopy *copyRegion) {
        auto& context = VulkanContext::getInstance();

        VkCommandBuffer commandBuffer = beginTmpCommandBuffer(context.transferTmpCommandPool);
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, copyRegion);

        endTmpCommandBuffer(commandBuffer, context.transferQueue, context.transferTmpCommandPool);
    }

    void VulkanUtils::updateBufferMemory(const VulkanAllocation &allocation, VkDeviceSize offset,
                                         VkDeviceSize size,
                                         const void *data) {
        if (data == nullptr) {
            return;
        }

        auto& context = VulkanContext::getInstance();

        void *mappedData;
        VkResult result = vmaMapMemory(context.vmAllocator, allocation.vmaAllocation, &mappedData);
        VK_RESULT_ASSERT(result, "Failed to map memory buffer");

        std::memcpy((uint8*)mappedData + offset, data, (size_t) size);
        vmaUnmapMemory(context.vmAllocator, allocation.vmaAllocation);
    }

    void
    VulkanUtils::createTextureImage(const void *imageData, uint32 dataSize, uint32 width, uint32 height, uint32 depth,
                                    uint32 mipLevels,
                                    VkImageType imageType, VkFormat format, VkImageTiling tiling,
                                    VkImage &outTextureImage,
                                    VulkanAllocation &outAllocation, VkImageLayout textureLayout) {
        VkBuffer stagingBuffer;
        VulkanAllocation stagingAllocation = {};

        // Create staging buffer to create image in device local memory
        createBuffer(dataSize,
                     VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     stagingBuffer, stagingAllocation);

        if (imageData != nullptr) {
            updateBufferMemory(stagingAllocation, 0, dataSize, imageData);
        }

        createImage(width, height, depth, mipLevels, false, imageType, format, tiling,
                    // for copying and sampling in shaders
                    VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    outTextureImage, outAllocation);

        // Transition layout to copy data
        transitionImageLayout(
                outTextureImage,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                mipLevels, 1);

        copyBufferToImage(stagingBuffer, outTextureImage, width, height, depth);

        destroyBuffer(stagingBuffer, stagingAllocation);

        if (mipLevels > 1) {
            // generate mipmaps and layout transition
            // from transfer destination to shader readonly
            generateMipmaps(outTextureImage, format, width, height, mipLevels, 1, textureLayout);
        }
        else {
            transitionImageLayout(
                    outTextureImage,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, textureLayout,
                    mipLevels, 1);
        }
    }

    void
    VulkanUtils::createCubemapImage(const void *imageData, uint32 dataSize, uint32 width, uint32 height, uint32 depth,
                                    uint32 mipLevels, uint32 cubemapLayerSize,
                                    VkImageType imageType, VkFormat format, VkImageTiling tiling,
                                    VkImage &outTextureImage,
                                    VulkanAllocation &outAllocation, VkImageLayout textureLayout) {
        VkBuffer stagingBuffer;
        VulkanAllocation stagingAllocation = {};

        if (cubemapLayerSize * 6 > dataSize) {
            throw std::runtime_error("Cubemap dataSize must not be greater than (6 * cubemapLayerSize)");
        }

        // Create staging buffer to create image in device local memory
        createBuffer(dataSize,
                     VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     stagingBuffer, stagingAllocation);

        if (imageData != nullptr) {
            updateBufferMemory(stagingAllocation, 0, dataSize, imageData);
        }

        createImage(width, height, depth, mipLevels, true, imageType, format, tiling,
                // for copying and sampling in shaders
                    VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    outTextureImage, outAllocation);

        // Transition layout to copy data
        transitionImageLayout(
                outTextureImage,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                mipLevels, 6);

        copyBufferToCubemapImage(stagingBuffer, outTextureImage, width, height, depth, cubemapLayerSize);

        destroyBuffer(stagingBuffer, stagingAllocation);

        if (mipLevels > 1) {
            // generate mipmaps and layout transition
            // from transfer destination to shader readonly
            generateMipmaps(outTextureImage, format, width, height, mipLevels, 6, textureLayout);
        }
        else {
            transitionImageLayout(
                    outTextureImage,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, textureLayout,
                    mipLevels, 6);
        }
    }

    void VulkanUtils::createImage(uint32 width, uint32 height,
                                  uint32 depth, uint32 mipLevels, bool isCubemap,
                                  VkImageType imageType, VkFormat format, VkImageTiling tiling,
                                  VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
                                  VkImage &outImage, VulkanAllocation &outAllocation) {
        auto& context = VulkanContext::getInstance();

        VkImageCreateInfo imageInfo = {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = imageType;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = depth;
        imageInfo.mipLevels = mipLevels;
        imageInfo.arrayLayers = isCubemap ? 6 : 1;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.flags = isCubemap ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0;

        VmaAllocationCreateInfo allocInfo = {};
        // allocInfo.usage = ;
        allocInfo.requiredFlags = properties;

        VmaAllocationInfo outAllocInfo;
        VkResult result = vmaCreateImage(context.vmAllocator, &imageInfo, &allocInfo, &outImage, &outAllocation.vmaAllocation, &outAllocInfo);
        VK_RESULT_ASSERT(result, "Failed to create image");

        outAllocation.offset = outAllocInfo.offset;
        outAllocation.memory = outAllocInfo.deviceMemory;
    }

    void VulkanUtils::copyBufferToImage(VkBuffer buffer, VkImage image, uint32 width, uint32 height, uint32 depth) {
        auto& context = VulkanContext::getInstance();

        VkCommandBuffer commandBuffer = beginTmpCommandBuffer(context.transferTmpCommandPool);

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

        endTmpCommandBuffer(commandBuffer, context.transferQueue, context.transferTmpCommandPool);
    }

    void VulkanUtils::copyBufferToCubemapImage(VkBuffer buffer, VkImage image, uint32 width, uint32 height, uint32 depth, uint32 layerSize) {
        auto& context = VulkanContext::getInstance();

        VkCommandBuffer commandBuffer = beginTmpCommandBuffer(context.transferTmpCommandPool);

        VkBufferImageCopy regions[6];

        for (int i = 0; i < 6; i++) {
            auto &region = regions[i];
            region = {};

            region.bufferOffset = i * layerSize;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;
            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            // this function copies without mipmaps
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.baseArrayLayer = i;
            region.imageSubresource.layerCount = 1;
            region.imageOffset = {0, 0, 0};
            region.imageExtent = {width, height, depth};
        }

        vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 6, regions);

        endTmpCommandBuffer(commandBuffer, context.transferQueue, context.transferTmpCommandPool);
    }

    void VulkanUtils::transitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, uint32 mipLevels, uint32 layerCount) {
        auto& context = VulkanContext::getInstance();
        VkCommandBuffer commandBuffer = beginTmpCommandBuffer(context.transferTmpCommandPool);

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
        barrier.subresourceRange.layerCount = layerCount;

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

        endTmpCommandBuffer(commandBuffer, context.transferQueue, context.transferTmpCommandPool);
    }

    void VulkanUtils::createImageView(VkImageView &outImageView, VkImage image,
                                      VkImageViewType viewType,
                                      VkFormat format, const VkImageSubresourceRange &subResourceRange,
                                      VkComponentMapping components) {
        auto& context = VulkanContext::getInstance();

        VkImageViewCreateInfo imageViewInfo = {};
        imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewInfo.image = image;
        imageViewInfo.viewType = viewType;
        imageViewInfo.format = format;
        imageViewInfo.components = components;
        imageViewInfo.subresourceRange = subResourceRange;

        VkResult result = vkCreateImageView(context.device, &imageViewInfo, nullptr, &outImageView);
        VK_RESULT_ASSERT(result, "Failed to create image view");
    }

    void
    VulkanUtils::generateMipmaps(VkImage image, VkFormat format, uint32 width, uint32 height,
                                 uint32 mipLevels, uint32 layerCount, VkImageLayout newLayout) {
        auto& context = VulkanContext::getInstance();

        VkFormatProperties formatProperties = getDeviceFormatProperties(format);

        if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
            throw VulkanException("Failed to generate mipmaps as specified format doesn't support linear blitting");
        }

        VkCommandBuffer commandBuffer = beginTmpCommandBuffer(context.transferTmpCommandPool);

        for (uint32 layer = 0; layer < layerCount; layer++) {

            VkImageMemoryBarrier barrier = {};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.image = image;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseArrayLayer = layer;
            barrier.subresourceRange.layerCount = 1;
            barrier.subresourceRange.levelCount = 1;

            int32 mipWidth = width;
            int32 mipHeight = height;

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
                blit.srcSubresource.baseArrayLayer = layer;
                blit.srcSubresource.layerCount = 1;

                // destination, divided
                blit.dstOffsets[0] = {0, 0, 0};
                blit.dstOffsets[1] = {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1};
                blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                blit.dstSubresource.mipLevel = i;
                blit.dstSubresource.baseArrayLayer = layer;
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
        }

        endTmpCommandBuffer(commandBuffer, context.transferQueue, context.transferTmpCommandPool);
    }

    void VulkanUtils::createDepthStencilBuffer(uint32 width, uint32 height, uint32 depth,
                                               VkImageType imageType, VkFormat format, VkImage &outImage,
                                               VulkanAllocation &outAllocation, VkImageUsageFlags usageFlags) {

        // get properties of depth stencil format
        const VkFormatProperties &properties = getDeviceFormatProperties(format);

        VkImageTiling tiling;

        if (properties.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            tiling = VK_IMAGE_TILING_LINEAR;
        } else if (properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            tiling = VK_IMAGE_TILING_OPTIMAL;
        } else {
            throw VulkanException("Failed to find supported format");
        }

        createImage(width, height, depth, 1, false,
                    imageType, format, tiling, usageFlags,
                // depth stencil buffer is device local
                // TODO: make visible from cpu
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    outImage, outAllocation
        );
    }

    void VulkanUtils::createVertexInputState(const VulkanVertexLayout &layout, VkPipelineVertexInputStateCreateInfo &state) {
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

    void VulkanUtils::createRasterizationState(const IRenderDevice::PipelineRasterizationDesc &rasterizationDesc,
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

    void VulkanUtils::createPipelineLayout(const VulkanUniformLayout &uniformLayout,
                                           VkPipelineLayout &pipelineLayout) {
        auto& context = VulkanContext::getInstance();

        VkResult result;

        VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &uniformLayout.properties.layout;
        pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
        pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

        result = vkCreatePipelineLayout(context.device, &pipelineLayoutInfo, nullptr, &pipelineLayout);
        VK_RESULT_ASSERT(result, "Failed to create pipeline layout");
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

    void VulkanUtils::createColorBlendAttachmentState(const IRenderDevice::BlendAttachmentDesc &attachmentDesc,
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
    VulkanUtils::createColorBlendState(const IRenderDevice::PipelineBlendStateDesc &stateDesc, uint32 attachmentsCount,
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

    void VulkanUtils::createSurfaceColorBlendState(const IRenderDevice::PipelineSurfaceBlendStateDesc &stateDesc,
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

    void VulkanUtils::createDepthStencilState(const IRenderDevice::PipelineDepthStencilStateDesc &desc,
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

    VkStencilOpState VulkanUtils::createStencilOperationState(const IRenderDevice::StencilOpStateDesc &desc) {
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

    VkCommandPool VulkanUtils::createCommandPool(VkCommandPoolCreateFlags flags, uint32 queueFamilyIndex) {
        auto& context = VulkanContext::getInstance();

        VkCommandPoolCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        info.pNext = nullptr;
        info.queueFamilyIndex = queueFamilyIndex;
        info.flags = flags;

        VkCommandPool commandPool;
        VkResult result = vkCreateCommandPool(context.device, &info, nullptr, &commandPool);
        VK_RESULT_ASSERT(result, "Failed to create command pool");

        return commandPool;
    }

    VkCommandBuffer VulkanUtils::beginTmpCommandBuffer(VkCommandPool commandPool) {
        auto& context = VulkanContext::getInstance();

        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        VkResult result = vkAllocateCommandBuffers(context.device, &allocInfo, &commandBuffer);
        VK_RESULT_ASSERT(result, "Failed to allocate command buffer");

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
        VK_RESULT_ASSERT(result, "Failed to begin command buffer");

        return commandBuffer;
    }

    void VulkanUtils::endTmpCommandBuffer(VkCommandBuffer commandBuffer,
                                          VkQueue queue, VkCommandPool commandPool) {
        auto& context = VulkanContext::getInstance();

        VkResult result = vkEndCommandBuffer(commandBuffer);
        VK_RESULT_ASSERT(result, "Failed to end command buffer");

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        result = vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
        VK_RESULT_ASSERT(result, "Failed to submit queue");

        result = vkQueueWaitIdle(queue);
        VK_RESULT_ASSERT(result, "Error on vkQueueWaitIdle");

        vkFreeCommandBuffers(context.device, commandPool, 1, &commandBuffer);
    }

    void VulkanUtils::destroyTmpComandBuffer(VkCommandBuffer commandBuffer, VkCommandPool commandPool) {
        auto& context = VulkanContext::getInstance();
        vkFreeCommandBuffers(context.device, commandPool, 1, &commandBuffer);
    }

    void VulkanUtils::destroyBuffer(VkBuffer buffer, VulkanAllocation &allocation) {
        auto& context = VulkanContext::getInstance();
        vmaDestroyBuffer(context.vmAllocator, buffer, allocation.vmaAllocation);

        allocation.vmaAllocation = VK_NULL_HANDLE;
        allocation.memory = VK_NULL_HANDLE;
        allocation.offset = 0;
    }

    void VulkanUtils::destroyImage(VkImage image, VulkanAllocation &allocation) {
        auto& context = VulkanContext::getInstance();
        vmaDestroyImage(context.vmAllocator, image, allocation.vmaAllocation);

        allocation.vmaAllocation = VK_NULL_HANDLE;
        allocation.memory = VK_NULL_HANDLE;
        allocation.offset = 0;
    }

} // namespace ignimbrite