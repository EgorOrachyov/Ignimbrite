/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                        */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#include <Sampler.h>

namespace ignimbrite {

    Sampler::Sampler(RefCounted<IRenderDevice> device)
        : mDevice(std::move(device)) {
        mFilter = SamplerFilter::Linear;
        mMipmapFilter = SamplerFilter::Linear;
        mRepeatMode = SamplerRepeatMode::Repeat;
        mBorderColor = SamplerBorderColor::Black;
    }

    Sampler::~Sampler() {
        releaseHandle();
    }

    void Sampler::setHighQualityFiltering(SamplerRepeatMode mode) {
        releaseHandle();

        mFilter = SamplerFilter::Linear;
        mMipmapFilter = SamplerFilter::Linear;
        mRepeatMode = mode;
        mBorderColor = SamplerBorderColor::White;

        IRenderDevice::SamplerDesc samplerDesc{};
        samplerDesc.u = mRepeatMode;
        samplerDesc.v = mRepeatMode;
        samplerDesc.w = mRepeatMode;
        samplerDesc.color = mBorderColor;
        samplerDesc.min = mFilter;
        samplerDesc.mag = mFilter;
        samplerDesc.mipmapMode = mMipmapFilter;
        samplerDesc.minLod = 0.0f;
        samplerDesc.maxLod = 1.0f;
        samplerDesc.useAnisotropy = true;
        samplerDesc.anisotropyMax = 16;
        samplerDesc.mipLodBias = 0;

        mHandle = mDevice->createSampler(samplerDesc);

        if (mHandle.isNull()) {
            // todo: do something
        }
    }

    void Sampler::releaseHandle() {
        if (mHandle.isNotNull()) {
            mDevice->destroySampler(mHandle);
            mHandle = ID<IRenderDevice::Sampler>();
        }
    }

    bool Sampler::isValidHandle() {
        return mHandle.isNotNull();
    }

}