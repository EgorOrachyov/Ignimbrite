/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov, Sultim Tsyrendashiev                  */
/**********************************************************************************/

#ifndef IGNIMBRITELIBRARY_VULKANRENDERDEVICE_H
#define IGNIMBRITELIBRARY_VULKANRENDERDEVICE_H

#include <ignimbrite/ObjectIDBuffer.h>
#include <VulkanObjects.h>
#include <VulkanContext.h>
#include <VulkanSurface.h>
#include <VulkanUtils.h>
#include <VulkanDrawList.h>

namespace ignimbrite {

    /** Vulkan implementation for Render Device interface */
    class VulkanRenderDevice : public RenderDevice {
    public:

        VulkanRenderDevice(uint32 extensionsCount, const char *const *extensions);
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

        ID createFramebuffer(const std::vector<ID> &attachments, ID framebufferFormat) override;
        void destroyFramebuffer(ID framebuffer) override;

        ID createSampler(const SamplerDesc &samplerDesc) override;
        void destroySampler(ID sampler) override;

        ID createTexture(const TextureDesc &textureDesc) override;
        void destroyTexture(ID texture) override;

        ID createUniformSet(const UniformSetDesc &setDesc, ID uniformLayout) override;
        void destroyUniformSet(ID set) override;

        ID createUniformLayout(const UniformLayoutDesc &layoutDesc) override;
        void destroyUniformLayout(ID layout) override;

        ID createUniformBuffer(BufferUsage usage, uint32 size, const void *data) override;
        void updateUniformBuffer(ID buffer, uint32 size, uint32 offset, const void *data) override;
        void destroyUniformBuffer(ID buffer) override;

        ID createShaderProgram(const ProgramDesc &programDesc) override;
        void destroyShaderProgram(ID program) override;

        ID createGraphicsPipeline(PrimitiveTopology topology,
                                  ID program, ID vertexLayout, ID uniformLayout, ID framebufferFormat,
                                  const PipelineRasterizationDesc &rasterizationDesc,
                                  const PipelineBlendStateDesc &blendStateDesc,
                                  const PipelineDepthStencilStateDesc &depthStencilStateDesc) override;
        ID createGraphicsPipeline(ID surface, PrimitiveTopology topology, ID program, ID vertexLayout, ID uniformLayout,
                                  const PipelineRasterizationDesc &rasterizationDesc,
                                  const PipelineSurfaceBlendStateDesc &blendStateDesc,
                                  const PipelineDepthStencilStateDesc &depthStencilStateDesc) override;
        void destroyGraphicsPipeline(ID pipeline) override;

        void drawListBegin() override;
        void drawListEnd() override;

        void drawListBindSurface(ID surface, const Color &color, const Region &area) override;
        void drawListBindFramebuffer(ID framebuffer, const std::vector<Color> &clearColors, const Region &area) override;
        void drawListBindFramebuffer(ID framebuffer, const std::vector<Color> &clearColors,
                                     float32 clearDepth, uint32 clearStencil, const Region &area) override;
        void drawListBindPipeline(ID graphicsPipeline) override;
        void drawListBindUniformSet(ID uniformLayout) override;
        void drawListBindVertexBuffer(ID vertexBuffer, uint32 binding, uint32 offset) override;
        void drawListBindIndexBuffer(ID indexBuffer, IndicesType indicesType, uint32 offset) override;

        void drawListDraw(uint32 verticesCount, uint32 instancesCount) override;
        void drawListDrawIndexed(uint32 indicesCount, uint32 instancesCount) override;

        ID getSurface(const std::string &surfaceName) override;
        void getSurfaceSize(ID surface, uint32 &width, uint32 &height) override;
        void swapBuffers(ID surfaceId) override;

        void flush() override;
        void synchronize() override;

    private:
        friend class VulkanExtensions;
        
        using CommandBuffers = std::vector<VkCommandBuffer>;
        using ClearValues = std::vector<VkClearValue>;

        VulkanContext& mContext = VulkanContext::getInstance();
        VulkanDrawListStateControl mDrawListState;
        CommandBuffers mDrawQueue;
        CommandBuffers mSyncQueue;
        ClearValues mClearValues;

        ObjectIDBuffer<VulkanSurface> mSurfaces;
        ObjectIDBuffer<VulkanVertexLayout> mVertexLayouts;
        ObjectIDBuffer<VulkanVertexBuffer> mVertexBuffers;
        ObjectIDBuffer<VulkanIndexBuffer> mIndexBuffers;
        ObjectIDBuffer<VulkanFrameBufferFormat> mFrameBufferFormats;
        ObjectIDBuffer<VulkanFramebuffer> mFrameBuffers;
        ObjectIDBuffer<VkSampler> mSamplers;
        ObjectIDBuffer<VulkanTextureObject> mTextureObjects;
        ObjectIDBuffer<VulkanUniformBuffer> mUniformBuffers;
        ObjectIDBuffer<VulkanUniformLayout> mUniformLayouts;
        ObjectIDBuffer<VulkanUniformSet> mUniformSets;
        ObjectIDBuffer<VulkanShaderProgram> mShaderPrograms;
        ObjectIDBuffer<VulkanGraphicsPipeline> mGraphicsPipelines;
    };

} // namespace ignimbrite

#endif //IGNIMBRITELIBRARY_VULKANRENDERDEVICE_H