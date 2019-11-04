//
// Created by Egor Orachyov on 2019-11-03.
//

#ifndef RENDERINGLIBRARY_VULKANCONTEXT_H
#define RENDERINGLIBRARY_VULKANCONTEXT_H

#include <vulkan/vulkan.h>
#include <renderer/DeviceDefinitions.h>

/**
 * Handles vulkan instance setup. Defines physical
 * device and creates logical device for application.
 * defines queue families, finds graphics, present and transfer queues
 */
class VulkanContext {
public:
    VkInstance getInstance() const;
    VkDevice getDevice() const;
    VkCommandPool getCommandPool() const;
    VkQueue getTransferQueue() const;

    const VkPhysicalDeviceMemoryProperties &getDeviceMemoryProperties() const;
    VkFormatProperties getDeviceFormatProperties(VkFormat format) const;
    uint32_t getMemoryTypeIndex(uint32_t memoryTypeBits, VkFlags requirementsMask) const;

    struct BufferObject {
        VkBuffer buffer;
        VkDeviceMemory memory;
    };

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
     * Create buffer object
     * @param type dynamic / static
     * @param usage specifies usage of this buffer: vertex, index etc
     * @param outBuffer result buffer
     */
    void createBufferObject(BufferUsage type, VkBufferUsageFlags usage, uint32 size, const void *data, BufferObject &outBuffer);

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

    void copyBufferToImage(const VkBuffer buffer, const VkImage image, uint32_t width, uint32_t height, uint32_t depth);

    void transitionImageLayout(const VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);

    void createImageView(VkImageView& outImageView, const VkImage image, VkImageViewType viewType, VkFormat format, const VkImageSubresourceRange &subresourceRange,
        VkComponentMapping components = {});

private:
    VkInstance instance = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;

    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;
    VkQueue transferQueue = VK_NULL_HANDLE;

    // TODO: init deviceMemoryProperties
    VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
    // TODO: init command pool
    VkCommandPool commandPool;
};


#endif //RENDERINGLIBRARY_VULKANCONTEXT_H
