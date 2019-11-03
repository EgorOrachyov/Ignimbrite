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

    // _endTempCommandBuffer(device, queue, commandPool, commandBuffer);
}

void VulkanContext::updateBufferMemory(const VkDeviceMemory &bufferMemory, const void *data, VkDeviceSize size, VkDeviceSize offset) {
    void* mappedData;
    vkMapMemory(device, bufferMemory, offset, size, 0, &mappedData);
    memcpy(mappedData, data, (size_t)size);

    vkUnmapMemory(device, bufferMemory);
}

void VulkanContext::createTextureImage(const void *imageData, uint32_t width, uint32_t height, uint32_t depth,
                                       VkCommandPool commandPool, VkQueue queue) {
    VkDeviceSize imageSize = (VkDeviceSize)width * height * depth;

    //createBufferLocal(imageData, imageSize, VK_IMAGE_USAGE_SAMPLED_BIT)

}
