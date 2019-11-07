//
// Created by Egor Orachyov on 2019-11-03.
//

#ifndef RENDERINGLIBRARY_VULKANCONTEXT_H
#define RENDERINGLIBRARY_VULKANCONTEXT_H

#include "VulkanDefinitions.h"
#include "VulkanStructures.h"

/**
 * Handles vulkan instance setup. Defines physical
 * device and creates logical device for application.
 * defines queue families, finds graphics, present and transfer queues
 */
class VulkanContext {
public:
    VkInstance getInstance() const { return mInstance; }
    VkDevice getDevice() const { return mDevice; }
    VkCommandPool getCommandPool() const { return mCommandPool; }
    VkQueue getTransferQueue() const { return mTransferQueue; }

    const VkPhysicalDeviceMemoryProperties &getDeviceMemoryProperties() const;
    VkFormatProperties getDeviceFormatProperties(VkFormat format) const;
    uint32_t getMemoryTypeIndex(uint32_t memoryTypeBits, VkFlags requirementsMask) const;

    /**
     * Create vulkan buffer, allocate memory and bind this memory to buffer
     * @param size size in bytes of the buffer to create
     * @param usage specifies usage of this buffer: vertex, index etc
     * @param properties required properties for memory allocation
     * @param outBuffer result buffer
     * @param outBufferMemory result buffer memory
     */
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                       VkBuffer &outBuffer, VkDeviceMemory &outBufferMemory);
    /**
     * Create vulkan buffer using staging buffer,
     * @note should be used if buffer is meant to be device local
     * @param usage specifies usage of this buffer: vertex, index etc
     *      (actually will be transformed to "usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT"
     *      to copy data from staging buffer)
     * @param data data to fill the buffer
     * @param size size in bytes of data
     */
    void createBufferLocal(const void *data, VkDeviceSize size, VkBufferUsageFlags usage,
                            VkBuffer &outBuffer, VkDeviceMemory &outBufferMemory);

    /**
     * Copy buffer using command pool
     * @param commandPool command pool to create one time submit command buffer
     * @param queue queue to submit created command buffer
     * @param srcBuffer source buffer
     * @param dstBuffer destination buffer
     * @param size size in bytes to copy
     * @note don't use this function if there are many buffers to copy
     * @note assuming, that offsets in both buffers are 0
     */
    void copyBuffer(VkCommandPool commandPool, VkQueue queue,
                     VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    void updateBufferMemory(const VkDeviceMemory &bufferMemory, const void *data, VkDeviceSize size, VkDeviceSize offset);

    void createTextureImage(const void *imageData, uint32_t width, uint32_t height, uint32_t depth,
                                VkImageType imageType, VkFormat format, VkImageTiling tiling,
                                VkImage &outTextureImage, VkDeviceMemory &outTextureMemory,
                                VkImageLayout textureLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    void createImage(uint32_t width, uint32_t height, uint32_t depth, VkImageType imageType, VkFormat format,
                        VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
                        VkImage &outImage, VkDeviceMemory &outImageMemory);

    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t depth);

    void transitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);

    void createImageView(VkImageView &outImageView, VkImage image, VkImageViewType viewType, VkFormat format,
                         const VkImageSubresourceRange &subresourceRange,
                         VkComponentMapping components = {});

private:

    VkInstance mInstance = VK_NULL_HANDLE;
    VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
    VkDevice mDevice = VK_NULL_HANDLE;

    VkQueue mGraphicsQueue = VK_NULL_HANDLE;
    VkQueue mPresentQueue = VK_NULL_HANDLE;
    VkQueue mTransferQueue = VK_NULL_HANDLE;

    // TODO: init deviceMemoryProperties
    VkPhysicalDeviceMemoryProperties mDeviceMemoryProperties = { };
    // TODO: init command pool
    VkCommandPool mCommandPool = VK_NULL_HANDLE;
};


#endif //RENDERINGLIBRARY_VULKANCONTEXT_H
