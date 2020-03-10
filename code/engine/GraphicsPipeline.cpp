/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                       */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                */
/**********************************************************************************/

#include <GraphicsPipeline.h>

namespace ignimbrite {

    GraphicsPipeline::GraphicsPipeline(RefCounted <IRenderDevice> device)
            : mDevice(std::move(device)),
              mTopology(PrimitiveTopology::TriangleList),
              mTarget(TargetType::None) {

    }

    GraphicsPipeline::~GraphicsPipeline() {
        releasePipeline();
    }

    void GraphicsPipeline::setShader(RefCounted <Shader> shader) {
        mShader = std::move(shader);
    }

    void GraphicsPipeline::setSurface(ID <IRenderDevice::Surface> surface) {
        mSurface = surface;
        mTarget = TargetType::Surface;

        auto blendAttachmentsCount = 1; // For surface oly one available always !
        mBlendDesc.attachments.resize(blendAttachmentsCount);
    }

    void GraphicsPipeline::setTargetFormat(RefCounted<RenderTarget::Format> format) {
        mTargetFormat = std::move(format);
        mTarget = TargetType::Framebuffer;

        auto blendAttachmentsCount = (uint32) mTargetFormat->getAttachments().size();
             blendAttachmentsCount -= (mTargetFormat->hasDepthStencilAttachment() ? 1 : 0);

        mBlendDesc.attachments.resize(blendAttachmentsCount);
    }
    
    void GraphicsPipeline::setVertexBuffersCount(uint32 count) {
        mVertexBuffersDesc.resize(count);
    }
    
    void GraphicsPipeline::setVertexBufferDesc(uint32 index, const IRenderDevice::VertexBufferLayoutDesc &desc) {
        if (index >= mVertexBuffersDesc.size())
            throw std::runtime_error("Index of buffer descriptor is out of bounds");

        mVertexBuffersDesc[index] = desc;
    }

    void GraphicsPipeline::setPrimitiveTopology(PrimitiveTopology topology) {
        mTopology = topology;
    }

    void GraphicsPipeline::setPolygonMode(PolygonMode mode) {
        mRasterizationDesc.mode = mode;
    }

    void GraphicsPipeline::setPolygonCullMode(PolygonCullMode cullMode) {
        mRasterizationDesc.cullMode = cullMode;
    }

    void GraphicsPipeline::setPolygonFrontFace(PolygonFrontFace frontFace) {
        mRasterizationDesc.frontFace = frontFace;
    }

    void GraphicsPipeline::setLineWidth(float32 lineWidth) {
        mRasterizationDesc.lineWidth = lineWidth;
    }

    void GraphicsPipeline::setBlendEnable(bool enable) {
        mBlendDesc.logicOpEnable = enable;
    }

    void GraphicsPipeline::setBlendLogicOp(LogicOperation logicOp) {
        mBlendDesc.logicOp = logicOp;
    }

    void GraphicsPipeline::setBlendConstants(const std::array<float32,4> &blendConstants) {
        mBlendDesc.blendConstants = blendConstants;
    }

    void GraphicsPipeline::setBlendAttachment(uint32 index, const IRenderDevice::BlendAttachmentDesc &desc) {
        if (index >= mBlendDesc.attachments.size())
            throw std::runtime_error("Index of blend attachment descriptor is out of bounds");

        mBlendDesc.attachments[index] = desc;
    }

    void GraphicsPipeline::setDepthTestEnable(bool enable) {
        mDepthStencilDesc.depthTestEnable = enable;
    }

    void GraphicsPipeline::setDepthWriteEnable(bool enable) {
        mDepthStencilDesc.depthWriteEnable = enable;
    }

    void GraphicsPipeline::setStencilTestEnable(bool enable) {
        mDepthStencilDesc.stencilTestEnable = enable;
    }


    void GraphicsPipeline::setDepthCompareOp(CompareOperation depthCompareOp) {
        mDepthStencilDesc.depthCompareOp = depthCompareOp;
    }

    void GraphicsPipeline::setStencilFrontDesc(const IRenderDevice::StencilOpStateDesc &front) {
        mDepthStencilDesc.front = front;
    }

    void GraphicsPipeline::setStencilBackDesc(const IRenderDevice::StencilOpStateDesc &back) {
        mDepthStencilDesc.back = back;
    }

    void GraphicsPipeline::createPipeline() {
        if (mHandle.isNotNull())
            throw std::runtime_error("An attempt to recreate pipeline prior release");

        switch (mTarget) {

            case TargetType::Surface: {

                checkSurfacePresent();
                checkShaderPresent();
                createVertexLayout();

                // create blend state desc for surface
                IRenderDevice::PipelineSurfaceBlendStateDesc blendDesc = {};
                blendDesc.logicOpEnable = mBlendDesc.logicOpEnable;
                blendDesc.logicOp = mBlendDesc.logicOp;
                blendDesc.attachment = mBlendDesc.attachments[0];
                blendDesc.blendConstants = mBlendDesc.blendConstants;

                mHandle = mDevice->createGraphicsPipeline(
                        mSurface,
                        mTopology,
                        mShader->getHandle(),
                        mVertexLayout,
                        mShader->getLayout(),
                        mRasterizationDesc,
                        blendDesc,
                        mDepthStencilDesc
                );

                if (mHandle.isNull())
                    throw std::runtime_error("Failed to create graphics pipeline");
            }
            break;

            case TargetType::Framebuffer: {

                checkShaderPresent();
                checkTargetFormatPresent();
                createVertexLayout();

                mHandle = mDevice->createGraphicsPipeline(
                        mTopology,
                        mShader->getHandle(),
                        mVertexLayout,
                        mShader->getLayout(),
                        mTargetFormat->getFormatHandle(),
                        mRasterizationDesc,
                        mBlendDesc,
                        mDepthStencilDesc
                );

                if (mHandle.isNull())
                    throw std::runtime_error("Failed to create graphics pipeline");
            }
            break;

            default:
                throw std::runtime_error("Rendering target is not specified [TargetType::None]");

        }
    }

    void GraphicsPipeline::releasePipeline() {
        if (mHandle.isNotNull()) {
            mDevice->destroyGraphicsPipeline(mHandle);
            mHandle = ID<IRenderDevice::GraphicsPipeline>();
        }

        if (mVertexLayout.isNotNull()) {
            mDevice->destroyVertexLayout(mVertexLayout);
            mVertexLayout = ID<IRenderDevice::VertexLayout>();
        }
    }

    void GraphicsPipeline::bindPipeline() {
        mDevice->drawListBindPipeline(mHandle);
    }

    void GraphicsPipeline::checkShaderPresent() const {
        if (mShader == nullptr)
            throw std::runtime_error("Shader is not specified for pipeline");
    }

    void GraphicsPipeline::checkSurfacePresent() const {
        if (mSurface.isNull())
            throw std::runtime_error("Surface is not specified for pipeline");
    }

    void GraphicsPipeline::checkTargetFormatPresent() const {
        if (mTargetFormat == nullptr)
            throw std::runtime_error("Target format is not specified for pipeline");
    }

    void GraphicsPipeline::createVertexLayout() {
        mVertexLayout = mDevice->createVertexLayout(mVertexBuffersDesc);

        if (mVertexLayout.isNull())
            throw std::runtime_error("Failed to create vertex layout object");
    }

}