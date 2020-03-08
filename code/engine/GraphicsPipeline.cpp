/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                       */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                */
/**********************************************************************************/


#include "GraphicsPipeline.h"

ignimbrite::GraphicsPipeline::GraphicsPipeline(ignimbrite::RefCounted<ignimbrite::IRenderDevice> device)
        : mDevice(std::move(device)), mTopology(PrimitiveTopology::TriangleList), mTarget(TargetType::None) {
    mBlendDesc.attachments.resize(1);
}

ignimbrite::GraphicsPipeline::~GraphicsPipeline() {
    releasePipeline();
}

void ignimbrite::GraphicsPipeline::createPipeline() {
    // if pipeline was previously created, destrou it
    releasePipeline();

    /*if (!mHandle.isNull())
        throw std::runtime_error("Graphics pipeline handle is already created");
    }*/

    switch (mTarget) {
        case TargetType::Surface: {
            // create blend state desc for surface
            IRenderDevice::PipelineSurfaceBlendStateDesc blendDesc = {};
            blendDesc.logicOpEnable = mBlendDesc.logicOpEnable;
            blendDesc.logicOp = mBlendDesc.logicOp;
            for (uint32 i = 0; i < 4; i++) {
                blendDesc.blendConstants[i] = mBlendDesc.blendConstants[i];
            }
            // using only first descriptor for surface
            blendDesc.attachment = mBlendDesc.attachments[0];

            mHandle = mDevice->createGraphicsPipeline(
                    mSurface, mTopology, mShader->getHandle(), mVertexLayout, mShader->getLayout(),
                    mRasterizationDesc, blendDesc, mDepthStencilDesc);
            break;
        }
        case TargetType::Framebuffer: {
            mHandle = mDevice->createGraphicsPipeline(
                    mTopology, mShader->getHandle(), mVertexLayout, mShader->getLayout(),
                    mFramebufferFormat, mRasterizationDesc, mBlendDesc, mDepthStencilDesc);
            break;
        }
        default:
            throw std::runtime_error("Surface or framebuffer format must be set before creating pipeline");
    }
}

void ignimbrite::GraphicsPipeline::releasePipeline() {
    if (mHandle.isNotNull()) {
        mDevice->destroyGraphicsPipeline(mHandle);
        mHandle = ID<IRenderDevice::GraphicsPipeline>();
    }
}

void ignimbrite::GraphicsPipeline::setShader(ignimbrite::RefCounted<ignimbrite::Shader> shader) {
    mShader = std::move(shader);
}

void ignimbrite::GraphicsPipeline::setSurface(ignimbrite::ID<ignimbrite::IRenderDevice::Surface> surface) {
    mSurface = surface;
    mTarget = TargetType::Surface;
}

void ignimbrite::GraphicsPipeline::setFramebufferFormat(
        ignimbrite::ID<ignimbrite::IRenderDevice::FramebufferFormat> format) {
    mFramebufferFormat = format;
    mTarget = TargetType::Framebuffer;
}

void
ignimbrite::GraphicsPipeline::setVertexLayout(ignimbrite::ID<ignimbrite::IRenderDevice::VertexLayout> vertexLayout) {
    mVertexLayout = vertexLayout;
}

void ignimbrite::GraphicsPipeline::setPrimitiveTopology(ignimbrite::PrimitiveTopology topology) {
    mTopology = topology;
}

void ignimbrite::GraphicsPipeline::setBlendEnable(bool enable) {
    mBlendDesc.logicOpEnable = enable;
}

void ignimbrite::GraphicsPipeline::setBlendLogicOp(ignimbrite::LogicOperation logicOp) {
    mBlendDesc.logicOp = logicOp;
}

void ignimbrite::GraphicsPipeline::setBlendConstants(const std::array<float, 4> &blendConstants) {
    for (uint32 i = 0; i < 4; i++) {
        mBlendDesc.blendConstants[i] = blendConstants[i];
    }
}

void ignimbrite::GraphicsPipeline::setBlendAttachment(ignimbrite::uint32 index,
                                                      const ignimbrite::IRenderDevice::BlendAttachmentDesc &desc) {
    if (index >= mBlendDesc.attachments.size()) {
        mBlendDesc.attachments.resize(index + 1);
    }

    mBlendDesc.attachments[index] = desc;
}

void ignimbrite::GraphicsPipeline::setPolygonMode(ignimbrite::PolygonMode mode) {
    mRasterizationDesc.mode = mode;
}

void ignimbrite::GraphicsPipeline::setPolygonCullMode(ignimbrite::PolygonCullMode cullMode) {
    mRasterizationDesc.cullMode = cullMode;
}

void ignimbrite::GraphicsPipeline::setPolygonFrontFace(ignimbrite::PolygonFrontFace frontFace) {
    mRasterizationDesc.frontFace = frontFace;
}

void ignimbrite::GraphicsPipeline::setLineWidth(float lineWidth) {
    mRasterizationDesc.lineWidth = lineWidth;
}

void ignimbrite::GraphicsPipeline::setDepthTestEnable(bool enable) {
    mDepthStencilDesc.depthTestEnable = enable;
}

void ignimbrite::GraphicsPipeline::setDepthWriteEnable(bool enable) {
    mDepthStencilDesc.depthWriteEnable = enable;
}

void ignimbrite::GraphicsPipeline::setStencilTestEnable(bool enable) {
    mDepthStencilDesc.stencilTestEnable = enable;
}


void ignimbrite::GraphicsPipeline::setDepthCompareOp(ignimbrite::CompareOperation depthCompareOp) {
    mDepthStencilDesc.depthCompareOp = depthCompareOp;
}

void ignimbrite::GraphicsPipeline::setStencilFrontDesc(const ignimbrite::IRenderDevice::StencilOpStateDesc &front) {
    mDepthStencilDesc.front = front;
}

void ignimbrite::GraphicsPipeline::setStencilBackDesc(const ignimbrite::IRenderDevice::StencilOpStateDesc &back) {
    mDepthStencilDesc.back = back;
}

void ignimbrite::GraphicsPipeline::bindPipeline() {
    mDevice->drawListBindPipeline(mHandle);
}
