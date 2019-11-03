//
// Created by Egor Orachyov on 2019-11-02.
//

#ifndef VULKANRENDERER_VULKANRENDERDEVICE_H
#define VULKANRENDERER_VULKANRENDERDEVICE_H

#include <include/renderer/RenderDevice.h>
#include <include/renderer/ObjectIDBuffer.h>
#include "VulkanContext.h"

class VulkanRenderDevice : public RenderDevice {
public:

    ID createVertexLayout(const std::vector<VertexBufferLayoutDesc> &vertexBuffersDesc) override;
    void destroyVertexLayout(ID layout) override;

    ID createVertexBuffer(BufferUsage usage, uint32 size, const void *data) override;
    void updateVertexBuffer(ID buffer, uint32 size, uint32 offset, const void *data) override;
    void destroyVertexBuffer(ID buffer) override;

    ID createIndexBuffer(BufferUsage usage, uint32 size, const void *data) override;
    void updateIndexBuffer(ID buffer, uint32 size, uint32 offset, const void *data) override;
    void destroyIndexBuffer(ID buffer) override;


private:
    VulkanContext context;

    struct VertexLayoutBatch {
        std::vector<VkVertexInputBindingDescription> vertBindings;
        std::vector<VkVertexInputAttributeDescription> vertAtributes;
    };

    struct BufferObject {
        VkBuffer buffer;
        VkDeviceMemory memory;
    };

    ObjectIDBuffer<VertexLayoutBatch> vertexLayoutBatches;
    ObjectIDBuffer<BufferObject> vertexBuffers;
    ObjectIDBuffer<BufferObject> indexBuffers;

private:
    /**
     * Create vulkan buffer, allocate memory and bind this memory to buffer
     * @param size size in bytes of the buffer to create
     * @param usage specifies usage of this buffer: vertex, index etc
     * @param properties required properties for memory allocation
     * @param outBuffer result buffer
     * @param outBufferMemory result buffer memory
     */
    void _createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
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
    void _createBufferLocal(const void *data, VkDeviceSize size, VkBufferUsageFlags usage,
            VkBuffer &outBuffer, VkDeviceMemory &outBufferMemory);

    /**
     * Create buffer object
     * @param type dynamic / static
     * @param usage specifies usage of this buffer: vertex, index etc
     * @param outBuffer result buffer
     */
    void _createBufferObject(BufferUsage type, VkBufferUsageFlags usage, uint32 size, const void *data, BufferObject &outBuffer);

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
    void _copyBuffer(VkCommandPool commandPool, VkQueue queue,
            VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    void _updateBufferMemory(const VkDeviceMemory &bufferMemory, const void *data, VkDeviceSize size, VkDeviceSize offset);
};

#endif //VULKANRENDERER_VULKANRENDERDEVICE_H