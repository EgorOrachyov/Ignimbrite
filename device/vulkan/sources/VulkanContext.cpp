//
// Created by Egor Orachyov on 2019-11-03.
//

#include "include/VulkanContext.h"
#include <exception>
#include <cstring>
#include <renderer/DeviceDefinitions.h>

VkInstance VulkanContext::getInstance() const {
    return instance;
}

VkDevice VulkanContext::getDevice() const {
    return device;
}

const VkPhysicalDeviceMemoryProperties &VulkanContext::getDeviceMemoryProperties() const {
    return deviceMemoryProperties;
}

VkFormatProperties VulkanContext::getDeviceFormatProperties(VkFormat format) const {
    VkFormatProperties properties;
    vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &properties);

    return properties;
}

uint32_t VulkanContext::getMemoryTypeIndex(uint32_t memoryTypeBits, VkFlags requirementsMask) const {
    // for each memory type available for this device
    for (uint32_t i = 0; i < deviceMemoryProperties.memoryTypeCount; i++)
    {
        // if type is available
        if ((memoryTypeBits & 1) == 1)
        {
            if ((deviceMemoryProperties.memoryTypes[i].propertyFlags & requirementsMask) == requirementsMask)
            {
                return i;
            }
        }

        memoryTypeBits >>= 1;
    }

    throw std::exception("Vulkan::Can't find memory type in device memory properties");
}

VkCommandPool VulkanContext::getCommandPool() const {
    return commandPool;
}

VkQueue VulkanContext::getTransferQueue() const {
    return transferQueue;
}

void VulkanContext::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                                       VkBuffer &outBuffer, VkDeviceMemory &outBufferMemory) {
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = (VkDeviceSize)size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult r = vkCreateBuffer(device, &bufferInfo, nullptr, &outBuffer);
    if (r != VK_SUCCESS)
    {
        throw std::exception("Vulkan::Can't create buffer for vertex data");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, outBuffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = getMemoryTypeIndex(memRequirements.memoryTypeBits, properties);

    r = vkAllocateMemory(device, &allocInfo, nullptr, &outBufferMemory);
    if (r != VK_SUCCESS)
    {
        throw std::exception("Vulkan::Can't allocate memory for vertex buffer");
    }

    r = vkBindBufferMemory(device, outBuffer, outBufferMemory, 0);
    if (r != VK_SUCCESS)
    {
        throw std::exception("Vulkan::Can't bind buffer memory for vertex buffer");
    }
}

void VulkanContext::createBufferLocal(const void *data, VkDeviceSize size, VkBufferUsageFlags usage,
                                            VkBuffer &outBuffer, VkDeviceMemory &outBufferMemory) {

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    // create staging buffer, visible to host
    createBuffer(size,
                  VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                  stagingBuffer, stagingBufferMemory);

    // copy data to staging buffer memory
    updateBufferMemory(stagingBufferMemory, data, size, 0);

    // create actual buffer
    createBuffer(size,
                  usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                  outBuffer,outBufferMemory);

    copyBuffer(commandPool, transferQueue, stagingBuffer, outBuffer, size);

    // delete staging buffer
    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void VulkanContext::createBufferObject(BufferUsage type, VkBufferUsageFlags usage, uint32 size, const void *data,
                                             BufferObject &outBuffer) {
    if (type == BufferUsage::Dynamic)
    {
        // create vertex buffer and allocate memory
        createBuffer(size, usage,
                // to be visible from host for updating buffer memory
                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                      outBuffer.buffer, outBuffer.memory);

        // copy data to allocated memory
        updateBufferMemory(outBuffer.memory, data, size, 0);
    }
    else
    {
        // allocate in device local memory
        // and set data
        createBufferLocal(data, size, usage,
                           outBuffer.buffer, outBuffer.memory);
    }
}

void VulkanContext::copyBuffer(VkCommandPool commandPool, VkQueue queue, VkBuffer srcBuffer, VkBuffer dstBuffer,
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

void VulkanContext::updateBufferMemory(const VkDeviceMemory &bufferMemory, const void *data, VkDeviceSize size, VkDeviceSize offset) {
    void* mappedData;
    vkMapMemory(device, bufferMemory, offset, size, 0, &mappedData);
    memcpy(mappedData, data, (size_t)size);

    vkUnmapMemory(device, bufferMemory);
}

void VulkanContext::createTextureImage(const void *imageData, uint32_t width, uint32_t height, uint32_t depth, 
                                                VkImageType imageType, VkFormat format, VkImageTiling tiling, 
                                                VkImage &outTextureImage, VkDeviceMemory &outTextureMemory,
                                                VkImageLayout textureLayout) {
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    VkDeviceSize imageSize = (VkDeviceSize)width * height * depth;

    // create staging buffer to create image in device local memory
    createBuffer(imageSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer, stagingBufferMemory);

    updateBufferMemory(stagingBufferMemory, imageData, imageSize, 0);

    createImage(width, height, depth, imageType, format, tiling, 
        // for copying and sampling in shaders
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        // TODO: updatable from cpu
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        outTextureImage, outTextureMemory);

    // layout transition from undefined 
    // to transfer destination to prepare image for copying
    transitionImageLayout(outTextureImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    
    copyBufferToImage(stagingBuffer, outTextureImage, width, height, depth);

    // layout transition from transfer destination to shader readonly
    transitionImageLayout(outTextureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, textureLayout);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void VulkanContext::createImage(uint32_t width, uint32_t height, uint32_t depth, 
                                    VkImageType imageType, VkFormat format, VkImageTiling tiling, 
                                    VkImageUsageFlags usage, VkMemoryPropertyFlags properties, 
                                    VkImage& outImage, VkDeviceMemory& outImageMemory) {

    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = imageType;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = depth;
    // TODO: mipmaps
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult r = vkCreateImage(device, &imageInfo, nullptr, &outImage);
    if (r != VK_SUCCESS)
    {
        throw std::exception("Vulkan::Can't create image");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, outImage, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = getMemoryTypeIndex(memRequirements.memoryTypeBits, properties);

    r = vkAllocateMemory(device, &allocInfo, nullptr, &outImageMemory);
    if (r != VK_SUCCESS)
    {
        throw std::exception("Vulkan::Can't allocate memory for image");
    }

    r = vkBindImageMemory(device, outImage, outImageMemory, 0);
    if (r != VK_SUCCESS)
    {
        throw std::exception("Vulkan::Can't bind image memory");
    }
}

void VulkanContext::copyBufferToImage(const VkBuffer buffer, const VkImage image, uint32_t width, uint32_t height, uint32_t depth)
{
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
    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = { width, height, depth };

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    // TODO
    // _endTempCommandBuffer(device, transitionQueue or graphicsQueue, commandPool, commandBuffer);
}

void VulkanContext::transitionImageLayout(const VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout)
{
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
    // TODO: mipmaps
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        // undefined to transfer destination

        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        // transfer destination to fragment shader

        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else
    {
        throw std::exception("Vulkan::Unimplemented layout transition");
    }

    vkCmdPipelineBarrier(
        commandBuffer, sourceStage, destinationStage,
        0, 0, nullptr, 0, nullptr, 1, &barrier);

    // TODO
    // _endTempCommandBuffer(device, transitionQueue or graphicsQueue, commandPool, commandBuffer);
}

void VulkanContext::createImageView(VkImageView &outImageView, const VkImage image, VkImageViewType viewType, VkFormat format, const VkImageSubresourceRange& subresourceRange, VkComponentMapping components)
{
    VkImageViewCreateInfo imageViewInfo = {};
    imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewInfo.image = image;
    imageViewInfo.viewType = viewType;
    imageViewInfo.format = format;
    imageViewInfo.components = components;
    imageViewInfo.subresourceRange = subresourceRange;

    VkResult r = vkCreateImageView(device, &imageViewInfo, nullptr, &outImageView);
    if (r != VK_SUCCESS)
    {
        throw std::exception("Vulkan::Can't create image view");
    }
}