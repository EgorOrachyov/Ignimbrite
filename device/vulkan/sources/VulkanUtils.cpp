//
// Created by Egor Orachyov on 2019-11-11.
//

#include <renderer/DeviceDefinitions.h>
#include <renderer/Compilation.h>
#include <VulkanUtils.h>
#include <exception>
#include <cstring>
#include <set>

VkFormatProperties VulkanUtils::getDeviceFormatProperties(VulkanContext &context, VkFormat format) {
    VkFormatProperties properties;
    vkGetPhysicalDeviceFormatProperties(context.physicalDevice, format, &properties);
    return properties;
}

uint32 VulkanUtils::getMemoryTypeIndex(VulkanContext &context, uint32 memoryTypeBits, VkFlags requirementsMask) {
    // for each memory type available for this device
    for (uint32 i = 0; i < context.deviceMemoryProperties.memoryTypeCount; i++) {
        // if type is available
        if ((memoryTypeBits & 1) == 1) {
            if ((context.deviceMemoryProperties.memoryTypes[i].propertyFlags & requirementsMask) ==
                requirementsMask) {
                return i;
            }
        }

        memoryTypeBits >>= 1;
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

    VkResult r = vkCreateBuffer(context.device, &bufferInfo, nullptr, &outBuffer);
    if (r != VK_SUCCESS) {
        throw VulkanException("Can't create buffer for vertex data");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(context.device, outBuffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = getMemoryTypeIndex(context, memRequirements.memoryTypeBits, properties);

    r = vkAllocateMemory(context.device, &allocInfo, nullptr, &outBufferMemory);
    if (r != VK_SUCCESS) {
        throw VulkanException("Can't allocate memory for vertex buffer");
    }

    r = vkBindBufferMemory(context.device, outBuffer, outBufferMemory, 0);
    if (r != VK_SUCCESS) {
        throw VulkanException("Can't bind buffer memory for vertex buffer");
    }
}

void
VulkanUtils::createBufferLocal(VulkanContext &context, const void *data, VkDeviceSize size, VkBufferUsageFlags usage,
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

    copyBuffer(context, context.commandPool, context.transferQueue, stagingBuffer, outBuffer, size);

    vkDestroyBuffer(context.device, stagingBuffer, nullptr);
    vkFreeMemory(context.device, stagingBufferMemory, nullptr);
}

void VulkanUtils::copyBuffer(VulkanContext &context, VkCommandPool commandPool, VkQueue queue, VkBuffer srcBuffer,
                             VkBuffer dstBuffer,
                             VkDeviceSize size) {
    // TODO: create _beginTempCommandBuffer and _endTempCommandBuffer
    VkCommandBuffer commandBuffer; // = _beginTempCommandBuffer(device, commandPool);

    VkBufferCopy copyRegion = {};
    copyRegion.size = size;
    copyRegion.dstOffset = 0;
    copyRegion.srcOffset = 0;

    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    // TODO
    // _endTempCommandBuffer(device, transitionQueue or graphicsQueue, commandPool, commandBuffer);
}

void VulkanUtils::updateBufferMemory(VulkanContext &context, VkDeviceMemory bufferMemory, VkDeviceSize offset,
                                     VkDeviceSize size,
                                     const void *data) {
    void *mappedData;
    vkMapMemory(context.device, bufferMemory, offset, size, 0, &mappedData);
    memcpy(mappedData, data, (size_t) size);
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

    updateBufferMemory(context, stagingBufferMemory, 0, imageSize, imageData);

    createImage(context, width, height, depth, mipLevels, imageType, format, tiling,
            // for copying and sampling in shaders
                VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            // TODO: updatable from cpu
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
    generateMipmaps(context, outTextureImage, format, width, height, depth, mipLevels);
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

    VkResult r = vkCreateImage(context.device, &imageInfo, nullptr, &outImage);
    if (r != VK_SUCCESS) {
        throw VulkanException("Can't create image");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(context.device, outImage, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = getMemoryTypeIndex(context, memRequirements.memoryTypeBits, properties);

    r = vkAllocateMemory(context.device, &allocInfo, nullptr, &outImageMemory);
    if (r != VK_SUCCESS) {
        throw VulkanException("Can't allocate memory for image");
    }

    r = vkBindImageMemory(context.device, outImage, outImageMemory, 0);
    if (r != VK_SUCCESS) {
        throw VulkanException("Can't bind image memory");
    }
}

void
VulkanUtils::copyBufferToImage(VulkanContext &context, VkBuffer buffer, VkImage image, uint32 width, uint32 height,
                               uint32 depth) {
    // TODO: create _beginTempCommandBuffer and _endTempCommandBuffer
    VkCommandBuffer commandBuffer; // = _beginTempCommandBuffer(device, commandPool);

    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    // TODO: mipmaps
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, depth};

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    // TODO
    // _endTempCommandBuffer(device, transitionQueue or graphicsQueue, commandPool, commandBuffer);
}

void VulkanUtils::transitionImageLayout(VulkanContext &context, VkImage image, VkImageLayout oldLayout,
                                        VkImageLayout newLayout, uint32 mipLevels) {
    // TODO: create _beginTempCommandBuffer and _endTempCommandBuffer
    VkCommandBuffer commandBuffer; // = _beginTempCommandBuffer(device, commandPool);

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
            commandBuffer, sourceStage, destinationStage,
            0, 0, nullptr, 0, nullptr, 1, &barrier);

    // TODO
    // _endTempCommandBuffer(device, transitionQueue or graphicsQueue, commandPool, commandBuffer);
}

void
VulkanUtils::createImageView(VulkanContext &context, VkImageView &outImageView, VkImage image, VkImageViewType viewType,
                             VkFormat format, const VkImageSubresourceRange &subResourceRange,
                             VkComponentMapping components) {
    VkImageViewCreateInfo imageViewInfo = {};
    imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewInfo.image = image;
    imageViewInfo.viewType = viewType;
    imageViewInfo.format = format;
    imageViewInfo.components = components;
    imageViewInfo.subresourceRange = subResourceRange;

    VkResult r = vkCreateImageView(context.device, &imageViewInfo, nullptr, &outImageView);
    if (r != VK_SUCCESS) {
        throw VulkanException("Can't create image view");
    }
}

void VulkanUtils::generateMipmaps(VulkanContext &context, VkImage image, VkFormat format,
                                  uint32 width, uint32 height, uint32 depth, uint32 mipLevels) {
    VkFormatProperties formatProperties = getDeviceFormatProperties(context, format);

    if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
        throw VulkanException("Can't generate mipmaps as specified format doesn't support linear blitting");
    }

    // TODO: create _beginTempCommandBuffer and _endTempCommandBuffer
    VkCommandBuffer commandBuffer; // = _beginTempCommandBuffer(device, commandPool);

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

    // i=0 is oiginal image
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

        vkCmdBlitImage(commandBuffer,
                       image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                       1, &blit,
                // using linear interpolation
                       VK_FILTER_LINEAR);

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                             0, nullptr,
                             0, nullptr,
                             1, &barrier);

        if (mipWidth > 1) {
            mipWidth /= 2;
        };

        if (mipHeight > 1) {
            mipHeight /= 2;
        }
    }

    barrier.subresourceRange.baseMipLevel = mipLevels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer,
                         VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                         0, nullptr,
                         0, nullptr,
                         1, &barrier);

    // TODO
    // _endTempCommandBuffer(device, transitionQueue or graphicsQueue, commandPool, commandBuffer);
}

void VulkanUtils::getSurfaceProperties(VkPhysicalDevice physicalDevice, VkSurfaceKHR surfaceKHR,
                                       std::vector<VkSurfaceFormatKHR> &outSurfaceFormats,
                                       std::vector<VkPresentModeKHR> &outPresentModes) {
    uint32 surfFormatCount;
    uint32 presentModeCount;
    VkResult r;

    r = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surfaceKHR, &surfFormatCount, nullptr);
    if (r != VK_SUCCESS) {
        throw VulkanException("Can't get VkSurfaceKHR formats");
    }

    if (surfFormatCount == 0) {
        throw VulkanException("VkSurfaceKHR has no formats");
    }

    outSurfaceFormats.resize(surfFormatCount);

    r = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surfaceKHR, &surfFormatCount, outSurfaceFormats.data());
    if (r != VK_SUCCESS) {
        throw VulkanException("Can't get VkSurfaceKHR formats");
    }

    r = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surfaceKHR, &presentModeCount, nullptr);
    if (r != VK_SUCCESS) {
        throw VulkanException("Can't get VkSurfaceKHR present modes");
    }

    if (presentModeCount == 0) {
        throw VulkanException("VkSurfaceKHR has no present modes");
    }

    outPresentModes.resize(presentModeCount);

    r = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surfaceKHR, &presentModeCount,
                                                  outPresentModes.data());
    if (r != VK_SUCCESS) {
        throw VulkanException("Can't get VkSurfaceKHR present modes");
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

    throw VulkanException("Can't find available composite alpha");
}

void VulkanUtils::createDepthStencilBuffer(VulkanContext &context, uint32 width, uint32 height, uint32 depth,
                                           VkImageType imageType, VkFormat format, VkImageViewType viewType,
                                           VkImage &outImage, VkDeviceMemory &outImageMemory,
                                           VkImageView &outImageView) {

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
                imageType, format, tiling, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            // depth stencil buffer is device local
            // TODO: make visible from cpu
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                outImage, outImageMemory);

    VkComponentMapping components;
    components.r = VK_COMPONENT_SWIZZLE_R;
    components.g = VK_COMPONENT_SWIZZLE_G;
    components.b = VK_COMPONENT_SWIZZLE_B;
    components.a = VK_COMPONENT_SWIZZLE_A;

    VkImageSubresourceRange subresourceRange;
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    // depth stencil doesn't have mipmaps
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = 1;
    subresourceRange.baseArrayLayer = 0;
    subresourceRange.layerCount = 1;

    createImageView(context, outImageView, outImage, viewType, format, subresourceRange, components);
}
