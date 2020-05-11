/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                       */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                */
/**********************************************************************************/


#include "PresentationPass.h"
#include <PipelineContext.h>
#include <Geometry.h>
#include <IRenderEngine.h>

namespace ignimbrite {

    PresentationPass::PresentationPass(RefCounted<IRenderDevice> device, RefCounted<Texture> defaultTexture, RefCounted<Material> presentationMaterial) {
        mDevice = std::move(device);
        mDepthBufferArea = { 0.3f, 0.3f, 0.95f, 0.95f };

        mPresentationMaterial = std::move(presentationMaterial);
        mPresentationMaterial->setAll2DTextures(defaultTexture);
        mDepthPresentationMaterial = mPresentationMaterial->clone();

        Geometry::createFullscreenQuad(mFullscreenQuad, mDevice);
        Geometry::createRegionQuad(
                mDepthRegionQuad,
                mDepthBufferArea.x, mDepthBufferArea.y,
                mDepthBufferArea.z, mDepthBufferArea.w,
                mDevice);
    }

    PresentationPass::~PresentationPass() {
        if (mFullscreenQuad.isNotNull()) {
            mDevice->destroyVertexBuffer(mFullscreenQuad);
            mFullscreenQuad = ID<IRenderDevice::VertexBuffer>();
        }
        if (mDepthRegionQuad.isNotNull()) {
            mDevice->destroyVertexBuffer(mDepthRegionQuad);
            mDepthRegionQuad = ID<IRenderDevice::VertexBuffer>();
        }
    }

    void ignimbrite::PresentationPass::present(ignimbrite::ID<ignimbrite::IRenderDevice::Surface> targetSurface,
                                               ignimbrite::IRenderDevice::Region surfaceRegion,
                                               ignimbrite::RefCounted<ignimbrite::RenderTarget> source) {

        if (source->getColorAttachmentsCount() == 0) {
            throw std::runtime_error("Source render target for presentation pass must have at leasts one color attachment");
        }

        IRenderDevice::Color color = {0.0f, 0.0f, 0.0f, 0.0f};

        mDevice->drawListBindSurface(targetSurface, color, surfaceRegion);
        PipelineContext::cacheSurfaceBinding(targetSurface);

        const auto &colorTexture = source->getAttachment(0);

        if (!colorTexture->getSampler()) {
            throw std::runtime_error("Depth stencil attachment must have a sampler");
        }

        mPresentationMaterial->setTexture("texScreen", colorTexture);
        mPresentationMaterial->updateUniformData();

        mPresentationMaterial->bindGraphicsPipeline();
        mPresentationMaterial->bindUniformData();
        mDevice->drawListBindVertexBuffer(mFullscreenQuad, 0, 0);
        mDevice->drawListDraw(6, 1);

        if (mShowDepthBuffer && source->hasDepthStencilAttachment()) {
            const auto &depthTexture = source->getDepthStencilAttachment();

            if (!depthTexture->getSampler()) {
                throw std::runtime_error("Depth stencil attachment must have a sampler");
            }

            mDepthPresentationMaterial->setTexture("texScreen", depthTexture);
            mDepthPresentationMaterial->updateUniformData();

            mDepthPresentationMaterial->bindGraphicsPipeline();
            mDepthPresentationMaterial->bindUniformData();
            mDevice->drawListBindVertexBuffer(mDepthRegionQuad, 0, 0);
            mDevice->drawListDraw(6, 1);
        }
    }

    void PresentationPass::enableDepthShow(const Vec2f &depthBufferAreaLU, const Vec2f &depthBufferAreaRB) {
        mShowDepthBuffer = true;

        if (mDepthBufferArea.x == depthBufferAreaLU.x && mDepthBufferArea.y == depthBufferAreaLU.y
            && mDepthBufferArea.y == depthBufferAreaRB.y && mDepthBufferArea.y == depthBufferAreaRB.y)  {
            return;
        }

        mDepthBufferArea = {
                depthBufferAreaLU.x, depthBufferAreaLU.y,
                depthBufferAreaRB.x, depthBufferAreaRB.y };

        if (mDepthRegionQuad.isNotNull()) {
            mDevice->destroyVertexBuffer(mDepthRegionQuad);

            Geometry::createRegionQuad(
                    mDepthRegionQuad,
                    mDepthBufferArea.x, mDepthBufferArea.y,
                    mDepthBufferArea.z, mDepthBufferArea.w,
                    mDevice);
        }
    }

    void PresentationPass::enableDepthShow() {
        mShowDepthBuffer = true;
    }

    void PresentationPass::disableDepthShow() {
        mShowDepthBuffer = false;
    }

    bool PresentationPass::isDepthShown() const {
        return mShowDepthBuffer;
    }
}