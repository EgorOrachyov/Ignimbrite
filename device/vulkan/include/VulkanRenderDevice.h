//
// Created by Egor Orachyov on 2019-11-02.
//

#ifndef VULKANRENDERER_VULKANRENDERDEVICE_H
#define VULKANRENDERER_VULKANRENDERDEVICE_H

#include <renderer/ObjectIDBuffer.h>
#include <VulkanObjects.h>
#include <VulkanContext.h>

/** Vulkan implementation for Render Device interface */
class VulkanRenderDevice : public RenderDevice {
public:

    VulkanRenderDevice(uint32 extensionsCount, const char* const* extensions);
    ~VulkanRenderDevice() override = default;

    ID createVertexLayout(const std::vector<VertexBufferLayoutDesc> &vertexBuffersDesc) override;
    void destroyVertexLayout(ID layout) override;

    ID createVertexBuffer(BufferUsage usage, uint32 size, const void *data) override;
    void updateVertexBuffer(ID buffer, uint32 size, uint32 offset, const void *data) override;
    void destroyVertexBuffer(ID buffer) override;

    ID createIndexBuffer(BufferUsage usage, uint32 size, const void *data) override;
    void updateIndexBuffer(ID buffer, uint32 size, uint32 offset, const void *data) override;
    void destroyIndexBuffer(ID buffer) override;

    ID createFramebufferFormat(const std::vector<FramebufferAttachmentDesc> &attachments) override;
    void destroyFramebufferFormat(ID framebufferFormat) override;

    ID createSampler(const SamplerDesc &samplerDesc) override;
    void destroySampler(ID sampler) override;

    ID createTexture(const TextureDesc &textureDesc) override;
    void destroyTexture(ID texture) override;

    ID getSurface(const std::string &surfaceName) override;
    void getSurfaceSize(ID surface, uint32 &width, uint32 &height) override;

private:

    friend class VulkanExtensions;

    VulkanContext context;

    ObjectIDBuffer<VulkanSurface> mSurfaces;
    ObjectIDBuffer<VulkanVertexLayout> mVertexLayouts;
    ObjectIDBuffer<VulkanVertexBuffer> mVertexBuffers;
    ObjectIDBuffer<VulkanIndexBuffer> mIndexBuffers;
    ObjectIDBuffer<VulkanFrameBufferFormat> mFrameBufferFormats;
    ObjectIDBuffer<VkSampler> mSamplers;
    ObjectIDBuffer<VulkanImageObject> mImageObjects;
};

#endif //VULKANRENDERER_VULKANRENDERDEVICE_H