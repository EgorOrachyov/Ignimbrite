//
// Created by Egor Orachyov on 2019-11-02.
//

#ifndef VULKANRENDERER_VULKANRENDERDEVICE_H
#define VULKANRENDERER_VULKANRENDERDEVICE_H

#include <include/renderer/RenderDevice.h>
#include <include/renderer/ObjectIDBuffer.h>
#include <vulkan/vulkan.h>

class VulkanRenderDevice : public RenderDevice {
public:

    ID createVertexLayout(const std::vector<VertexBufferLayoutDesc> &vertexBuffersDesc) override;
    void destroyVertexLayout(ID layout) override;

    ID createVertexBuffer(BufferUsage usage, uint32 size, const void *data) override;
    void updateVertexBuffer(ID buffer, uint32 size, uint32 offset, const void *data) override;
    void destroyVertexBuffer(ID buffer) override;


private:
    struct VertexLayoutBatch{
        std::vector<VkVertexInputBindingDescription> vertBindings;
        std::vector<VkVertexInputAttributeDescription> vertAtributes;
    };

    ObjectIDBuffer<VertexLayoutBatch> vertexLayoutBatches;


};

#endif //VULKANRENDERER_VULKANRENDERDEVICE_H