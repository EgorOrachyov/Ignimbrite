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
    ~VulkanRenderDevice() override;

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

    ID createFramebuffer(const std::vector<ID>& attachments, ID framebufferFormat) override;
    void destroyFramebuffer(ID framebuffer) override;

    ID createSampler(const SamplerDesc &samplerDesc) override;
    void destroySampler(ID sampler) override;

    ID createTexture(const TextureDesc &textureDesc) override;
    void destroyTexture(ID texture) override;

    ID getSurface(const std::string &surfaceName) override;
    void getSurfaceSize(ID surface, uint32 &width, uint32 &height) override;

    ID createUniformSet(const UniformSetDesc &setDesc, ID uniformLayout) override;
    void destroyUniformSet(ID set) override;

    ID createUniformLayout(const UniformLayoutDesc &layoutDesc) override;
    void destroyUniformLayout(ID layout) override;

    ID createUniformBuffer(BufferUsage usage, uint32 size, const void *data) override;
    void updateUniformBuffer(ID buffer, uint32 size, uint32 offset, const void *data) override;
    void destroyUniformBuffer(ID buffer) override;

    ID createShaderProgram(const std::vector<ShaderDataDesc> &shaders) override;
    void destroyShaderProgram(ID program) override;

    ID createGraphicsPipeline(PrimitiveTopology topology,
                              ID program, ID vertexLayout, ID uniformLayout, ID framebufferFormat,
                              const PipelineRasterizationDesc &rasterizationDesc,
                              const PipelineBlendStateDesc &blendStateDesc,
                              const PipelineDepthStencilStateDesc &depthStencilStateDesc) override {
        return RenderDevice::ID();
    }

    void destroyGraphicsPipeline(ID pipeline) override {

    }

    ID drawListBegin(ID framebuffer, std::vector<Color> clearColors, const Region &drawArea) override {
        return RenderDevice::ID();
    }

    ID drawListBegin(ID framebuffer, std::vector<Color> clearColors, float32 clearDepth, uint32 clearStencil,
                     const Region &drawArea) override {
        return RenderDevice::ID();
    }

    ID drawListBegin(ID surface, Color clearColor, const Region &drawArea) override {
        return RenderDevice::ID();
    }

    ID drawListBegin(ID surface, Color clearColor, float32 clearDepth, uint32 clearStencil,
                     const Region &drawArea) override {
        return RenderDevice::ID();
    }

    void drawListBindPipeline(ID drawList, ID graphicsPipeline) override {

    }

    void drawListBindUniformSet(ID drawList, ID uniformLayout) override {

    }

    void drawListBindVertexBuffer(ID drawList, ID vertexBuffer, uint32 binding, uint32 offset) override {

    }

    void drawListBindIndexBuffer(ID drawList, ID indexBuffer, IndicesType indicesType, uint32 offset) override {

    }

    void drawListDraw(ID drawList, uint32 verticesCount, uint32 instancesCount) override {

    }

    void drawListDrawIndexed(ID drawList, uint32 indicesCount, uint32 instancesCount) override {

    }

    void drawListEnd(ID drawList) override {

    }

    void swapBuffers() override {

    }

private:

    friend class VulkanExtensions;

    template <typename T>
    using Buffer = ObjectIDBuffer<T>;

    VulkanContext context;

    Buffer<VulkanSurface> mSurfaces;
    Buffer<VulkanVertexLayout> mVertexLayouts;
    Buffer<VulkanVertexBuffer> mVertexBuffers;
    Buffer<VulkanIndexBuffer> mIndexBuffers;
    Buffer<VulkanFrameBufferFormat> mFrameBufferFormats;
    Buffer<VkFramebuffer> mFrameBuffers;
    Buffer<VkSampler> mSamplers;
    Buffer<VulkanTextureObject> mTextureObjects;
    Buffer<VulkanUniformBuffer> mUniformBuffers;
    Buffer<VulkanUniformLayout> mUniformLayouts;
    Buffer<VulkanUniformSet> mUniformSets;
    Buffer<VulkanShaderProgram> mShaderPrograms;

};

#endif //VULKANRENDERER_VULKANRENDERDEVICE_H