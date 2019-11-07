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

    ID createSampler(const SamplerDesc &samplerDesc) override;
    void destroySampler(ID sampler) override;

    ID createTexture(const TextureDesc &textureDesc) override;
    void destroyTexture(ID texture) override;

private:
    VulkanContext context;

    struct VertexLayout {
        std::vector<VkVertexInputBindingDescription> vertBindings;
        std::vector<VkVertexInputAttributeDescription> vertAttributes;
    };

    struct ImageObject {
        VkImage image;
        VkDeviceMemory imageMemory;
        VkImageView imageView;
    };

    ObjectIDBuffer<VertexLayout> vertexLayoutBatches;
    ObjectIDBuffer<BufferObject> vertexBuffers;
    ObjectIDBuffer<BufferObject> indexBuffers;
    ObjectIDBuffer<VkSampler> samplers;
    ObjectIDBuffer<ImageObject> imageObjects;
};

#endif //VULKANRENDERER_VULKANRENDERDEVICE_H