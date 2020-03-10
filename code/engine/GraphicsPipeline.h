/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                        */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                 */
/**********************************************************************************/


#ifndef IGNIMBRITE_GRAPHICSPIPELINE_H
#define IGNIMBRITE_GRAPHICSPIPELINE_H

#include <CacheItem.h>
#include <IRenderDevice.h>
#include <Shader.h>
#include <RenderTarget.h>

namespace ignimbrite {

    class GraphicsPipeline : public CacheItem {
    public:

        explicit GraphicsPipeline(RefCounted<IRenderDevice> device);
        ~GraphicsPipeline() override;

        /** Specify shader of this pipeline */
        void setShader(RefCounted<Shader> shader);
        /** Target surface for rendering by this pipeline (also could be used for other compatible surfaces) */
        void setSurface(ID<IRenderDevice::Surface> surface);
        /** Format specification for offscreen pipelines */
        void setTargetFormat(RefCounted<RenderTarget::Format> format);
        /** Specify number of attached vertex buffers to the pipeline */
        void setVertexBuffersCount(uint32 count);
        /** Specify vertex attributes layout for vertex buffer with index */
        void setVertexBufferDesc(uint32 index, const IRenderDevice::VertexBufferLayoutDesc &desc);

        void setPrimitiveTopology(PrimitiveTopology topology);
        void setPolygonMode(PolygonMode mode);
        void setPolygonCullMode(PolygonCullMode cullMode);
        void setPolygonFrontFace(PolygonFrontFace frontFace);
        void setLineWidth(float32 lineWidth);

        void setBlendEnable(bool enable);
        void setBlendLogicOp(LogicOperation logicOp);
        void setBlendConstants(const std::array<float32,4> &blendConstants);
        void setBlendAttachment(uint32 index, const IRenderDevice::BlendAttachmentDesc &desc);
        
        void setDepthTestEnable(bool enable);
        void setDepthWriteEnable(bool enable);
        void setStencilTestEnable(bool enable);
        void setDepthCompareOp(CompareOperation depthCompareOp);
        void setStencilFrontDesc(const IRenderDevice::StencilOpStateDesc &front);
        void setStencilBackDesc(const IRenderDevice::StencilOpStateDesc &back);

        void createPipeline();
        void releasePipeline();
        void bindPipeline();

        const RefCounted<Shader> &getShader() const { return mShader; }
        const RefCounted<RenderTarget::Format> &getTargetFormat() const { return mTargetFormat; }
        const ID<IRenderDevice::GraphicsPipeline> &getHandle() const { return mHandle; }

    private:

        void checkShaderPresent() const;
        void checkSurfacePresent() const;
        void checkTargetFormatPresent() const;
        void createVertexLayout();

        /** Types of the result targets for rendering by this pipeline */
        enum class TargetType : uint32 {
            None,
            Surface,        /** Suitable only for surface rendering */
            Framebuffer     /** Suitable only for offscreen (FBO) rendering */
        };

        TargetType mTarget;
        PrimitiveTopology mTopology;

        IRenderDevice::PipelineRasterizationDesc mRasterizationDesc;
        IRenderDevice::PipelineBlendStateDesc mBlendDesc;
        IRenderDevice::PipelineDepthStencilStateDesc mDepthStencilDesc;
        std::vector<IRenderDevice::VertexBufferLayoutDesc> mVertexBuffersDesc;

        ID<IRenderDevice::Surface> mSurface;
        ID<IRenderDevice::VertexLayout> mVertexLayout;
        ID<IRenderDevice::GraphicsPipeline> mHandle;

        RefCounted<RenderTarget::Format> mTargetFormat;
        RefCounted<ignimbrite::Shader> mShader;
        RefCounted<IRenderDevice> mDevice;


    };
}

#endif //IGNIMBRITE_GRAPHICSPIPELINE_H
