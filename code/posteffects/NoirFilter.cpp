/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                        */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#include <NoirFilter.h>
#include <MaterialFullscreen.h>
#include <Geometry.h>
#include <PipelineContext.h>

namespace ignimbrite {

    NoirFilter::NoirFilter(ignimbrite::RefCounted<ignimbrite::IRenderDevice> device, String folderPath) {
        mDevice = std::move(device);
        mPrefixPath = std::move(folderPath);
    }

    NoirFilter::~NoirFilter() {
        if (mScreenQuad.isNotNull()) {
            mDevice->destroyVertexBuffer(mScreenQuad);
            mScreenQuad = ID<IRenderDevice::VertexBuffer>();
        }
    }

    bool NoirFilter::isActive() const {
        return mIsActive;
    }

    void NoirFilter::onAddedToPipeline(const RefCounted<ignimbrite::RenderTarget::Format> &targetFormat) {
        mMaterial = MaterialFullscreen::noirFilter(mPrefixPath, targetFormat, mDevice);
        Geometry::createFullscreenQuad(mScreenQuad, mDevice);
    }

    void NoirFilter::execute(RefCounted<RenderTarget> &input, RefCounted<RenderTarget> &output) {
        static std::vector<IRenderDevice::Color> color = { {0.0f, 0.0f, 0.0f, 0.0f} };
        static IRenderDevice::Region region = { 0, 0, { output->getWidth(), output->getHeight() } };
        static String textureName = "texScreen";

        auto& texture0 = input->getAttachment(0);
        if (texture0 != mCachedTedxture0) {
            mCachedTedxture0 = texture0;
            mMaterial->setTexture2D(textureName, mCachedTedxture0);
            mMaterial->updateUniformData();
        }

        mDevice->drawListBindFramebuffer(output->getHandle(), color, region);
        PipelineContext::cacheFramebufferBinding(output->getHandle());
        mMaterial->bindGraphicsPipeline();
        mMaterial->bindUniformData();
        mDevice->drawListBindVertexBuffer(mScreenQuad, 0, 0);
        mDevice->drawListDraw(6, 1);
    }

}