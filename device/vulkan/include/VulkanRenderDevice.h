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
        std::vector<VkVertexInputAttributeDescription> vertAttributes;
    };

    ObjectIDBuffer<VertexLayoutBatch> vertexLayoutBatches;
    ObjectIDBuffer<VulkanContext::BufferObject> vertexBuffers;
    ObjectIDBuffer<VulkanContext::BufferObject> indexBuffers;
};

#endif //VULKANRENDERER_VULKANRENDERDEVICE_H