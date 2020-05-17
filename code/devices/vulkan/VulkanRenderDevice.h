/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019, 2020  Egor Orachyov                                        */
/* Copyright (c) 2019, 2020  Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#ifndef IGNIMBRITE_VULKANRENDERDEVICE_H
#define IGNIMBRITE_VULKANRENDERDEVICE_H

#include <ObjectIDBuffer.h>
#include <VulkanObjects.h>
#include <VulkanContext.h>
#include <VulkanSurface.h>
#include <VulkanUtils.h>
#include <VulkanDrawList.h>

namespace ignimbrite {

    /** Vulkan implementation for Render Device interface */
    class VulkanRenderDevice : public IRenderDevice {
    public:

        VulkanRenderDevice(uint32 extensionsCount, const char *const *extensions, bool enableValidation = false);
        ~VulkanRenderDevice() override;

        ID<VertexLayout> createVertexLayout(const std::vector<VertexBufferLayoutDesc> &vertexBuffersDesc) override;
        void destroyVertexLayout(ID<VertexLayout> layout) override;

        ID<VertexBuffer> createVertexBuffer(BufferUsage usage, uint32 size, const void *data) override;
        void updateVertexBuffer(ID<VertexBuffer> buffer, uint32 size, uint32 offset, const void *data) override;
        void destroyVertexBuffer(ID<VertexBuffer> buffer) override;

        ID<IndexBuffer> createIndexBuffer(BufferUsage usage, uint32 size, const void *data) override;
        void updateIndexBuffer(ID<IndexBuffer> buffer, uint32 size, uint32 offset, const void *data) override;
        void destroyIndexBuffer(ID<IndexBuffer> buffer) override;

        ID<FramebufferFormat> createFramebufferFormat(const std::vector<FramebufferAttachmentDesc> &attachments) override;
        void destroyFramebufferFormat(ID<FramebufferFormat> framebufferFormat) override;

        ID<Framebuffer> createFramebuffer(const std::vector<ID<Texture>> &attachments, ID<FramebufferFormat> framebufferFormat) override;
        void destroyFramebuffer(ID<Framebuffer> framebuffer) override;

        ID<Sampler> createSampler(const SamplerDesc &samplerDesc) override;
        void destroySampler(ID<Sampler> sampler) override;

        ID<Texture> createTexture(const TextureDesc &textureDesc) override;
        void destroyTexture(ID<Texture> texture) override;

        ID<UniformSet> createUniformSet(const UniformSetDesc &setDesc, ID<UniformLayout> uniformLayout) override;
        void destroyUniformSet(ID<UniformSet> set) override;

        ID<UniformLayout> createUniformLayout(const UniformLayoutDesc &layoutDesc) override;
        void destroyUniformLayout(ID<UniformLayout> layout) override;

        ID<UniformBuffer> createUniformBuffer(BufferUsage usage, uint32 size, const void *data) override;
        void updateUniformBuffer(ID<UniformBuffer> buffer, uint32 size, uint32 offset, const void *data) override;
        void destroyUniformBuffer(ID<UniformBuffer> buffer) override;

        ID<ShaderProgram> createShaderProgram(const ProgramDesc &programDesc) override;
        void destroyShaderProgram(ID<ShaderProgram> program) override;

        ID<GraphicsPipeline> createGraphicsPipeline(PrimitiveTopology topology,
                                  ID<ShaderProgram> program,
                                  ID<VertexLayout> vertexLayout,
                                  ID<UniformLayout> uniformLayout,
                                  ID<FramebufferFormat> framebufferFormat,
                                  const PipelineRasterizationDesc &rasterizationDesc,
                                  const PipelineBlendStateDesc &blendStateDesc,
                                  const PipelineDepthStencilStateDesc &depthStencilStateDesc) override;
        ID<GraphicsPipeline> createGraphicsPipeline(ID<Surface> surface,
                                  PrimitiveTopology topology,
                                  ID<ShaderProgram> program,
                                  ID<VertexLayout> vertexLayout,
                                  ID<UniformLayout> uniformLayout,
                                  const PipelineRasterizationDesc &rasterizationDesc,
                                  const PipelineSurfaceBlendStateDesc &blendStateDesc,
                                  const PipelineDepthStencilStateDesc &depthStencilStateDesc) override;
        void destroyGraphicsPipeline(ID<GraphicsPipeline> pipeline) override;

        void drawListBegin() override;
        void drawListEnd() override;

        void drawListBindSurface(ID<Surface> surface, const Color &color, const Region &area) override;
        void drawListBindFramebuffer(ID<Framebuffer> framebuffer, const std::vector<Color> &clearColors, const Region &area) override;
        void drawListBindFramebuffer(ID<Framebuffer> framebuffer, const std::vector<Color> &clearColors,
                                     float32 clearDepth, uint32 clearStencil, const Region &area) override;
        void drawListBindPipeline(ID<GraphicsPipeline> graphicsPipeline) override;
        void drawListBindUniformSet(ID<UniformSet> uniformSet) override;
        void drawListBindVertexBuffer(ID<VertexBuffer> vertexBuffer, uint32 binding, uint32 offset) override;
        void drawListBindIndexBuffer(ID<IndexBuffer> indexBuffer, IndicesType indicesType, uint32 offset) override;

        void drawListDraw(uint32 verticesCount, uint32 instancesCount) override;
        void drawListDrawIndexed(uint32 indicesCount, uint32 instancesCount) override;

        ID<Surface> getSurface(const std::string &surfaceName) override;
        void getSurfaceSize(ID<Surface> surface, uint32 &width, uint32 &height) override;
        void swapBuffers(ID<Surface> surfaceId) override;

        void flush() override;
        void synchronize() override;

        const std::vector<DataFormat> &getSupportedTextureFormats() const override;
        const std::vector<ShaderLanguage> &getSupportedShaderLanguages() override;
        const std::string &getDeviceName() const override;
        Type getDeviceType() const override;

    private:
        friend class VulkanExtensions;
        
        using CommandBuffers = std::vector<VkCommandBuffer>;
        using ClearValues = std::vector<VkClearValue>;

        using IRenderDevice::VertexLayout;
        using IRenderDevice::VertexBuffer;
        using IRenderDevice::IndexBuffer;
        using IRenderDevice::UniformBuffer;
        using IRenderDevice::UniformLayout;
        using IRenderDevice::UniformSet;
        using IRenderDevice::ShaderProgram;
        using IRenderDevice::GraphicsPipeline;
        using IRenderDevice::FramebufferFormat;
        using IRenderDevice::Framebuffer;
        using IRenderDevice::Surface;
        using IRenderDevice::Texture;
        using IRenderDevice::Sampler;

        VulkanDrawListStateControl mDrawListState;
        VulkanContext&  mContext = VulkanContext::getInstance();
        CommandBuffers  mDrawQueue;
        CommandBuffers  mSyncQueue;
        ClearValues     mClearValues;

        IDBuffer<VulkanSurface,          Surface>           mSurfaces;
        IDBuffer<VulkanVertexLayout,     VertexLayout>      mVertexLayouts;
        IDBuffer<VulkanVertexBuffer,     VertexBuffer>      mVertexBuffers;
        IDBuffer<VulkanIndexBuffer,      IndexBuffer>       mIndexBuffers;
        IDBuffer<VulkanFrameBufferFormat,FramebufferFormat> mFrameBufferFormats;
        IDBuffer<VulkanFramebuffer,      Framebuffer>       mFrameBuffers;
        IDBuffer<VkSampler,              Sampler>           mSamplers;
        IDBuffer<VulkanTextureObject,    Texture>           mTextureObjects;
        IDBuffer<VulkanUniformBuffer,    UniformBuffer>     mUniformBuffers;
        IDBuffer<VulkanUniformLayout,    UniformLayout>     mUniformLayouts;
        IDBuffer<VulkanUniformSet,       UniformSet>        mUniformSets;
        IDBuffer<VulkanShaderProgram,    ShaderProgram>     mShaderPrograms;
        IDBuffer<VulkanGraphicsPipeline, GraphicsPipeline>  mGraphicsPipelines;

        std::vector<DataFormat> mSupportedTextureDataFormats;
        std::vector<ShaderLanguage> mSupportedShaderLanguages = { ShaderLanguage::SPIRV };
    };

} // namespace ignimbrite

#endif //IGNIMBRITE_VULKANRENDERDEVICE_H