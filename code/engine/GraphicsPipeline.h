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

namespace ignimbrite {

    class GraphicsPipeline : public CacheItem {
    public:
        explicit GraphicsPipeline(RefCounted<IRenderDevice> device);
        ~GraphicsPipeline() override;

        void setShader(RefCounted<Shader> shader);
        void setSurface(ID<IRenderDevice::Surface> surface);
        void setFramebufferFormat(ID<IRenderDevice::FramebufferFormat> format);
        void setVertexLayout(ID<IRenderDevice::VertexLayout> vertexLayout);
        void setPrimitiveTopology(PrimitiveTopology topology);

        void setBlendEnable(bool enable);

        void setBlendLogicOp(LogicOperation logicOp);
        void setBlendConstants(const std::array<float, 4> &blendConstants);
        void setBlendAttachment(uint32 index, const IRenderDevice::BlendAttachmentDesc &desc);

        void setPolygonMode(PolygonMode mode);
        void setPolygonCullMode(PolygonCullMode cullMode);
        void setPolygonFrontFace(PolygonFrontFace frontFace);
        void setLineWidth(float lineWidth);

        void setDepthTestEnable(bool enable);
        void setDepthWriteEnable(bool enable);
        void setStencilTestEnable(bool enable);
        void setDepthCompareOp(CompareOperation depthCompareOp);
        void setStencilFrontDesc(const IRenderDevice::StencilOpStateDesc &front);
        void setStencilBackDesc(const IRenderDevice::StencilOpStateDesc &back);

        void createPipeline();
        void releasePipeline();

        void bindPipeline();

        const ID<IRenderDevice::GraphicsPipeline> &getHandle() const { return mHandle; }

    private:
        enum class TargetType {
            None, Surface, Framebuffer
        };

        RefCounted<IRenderDevice> mDevice;
        ID<IRenderDevice::GraphicsPipeline> mHandle;

        PrimitiveTopology mTopology;

        TargetType mTarget;
        ID<IRenderDevice::Surface> mSurface;
        ID<IRenderDevice::FramebufferFormat> mFramebufferFormat;

        IRenderDevice::PipelineRasterizationDesc mRasterizationDesc;
        IRenderDevice::PipelineBlendStateDesc mBlendDesc;
        IRenderDevice::PipelineDepthStencilStateDesc mDepthStencilDesc;

        ID<IRenderDevice::VertexLayout> mVertexLayout;
        RefCounted<ignimbrite::Shader> mShader;
    };
}

#endif //IGNIMBRITE_GRAPHICSPIPELINE_H
