/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov, Sultim Tsyrendashiev                  */
/**********************************************************************************/

#include <ignimbrite/Sampler.h>

namespace ignimbrite {

    Sampler::Sampler(std::shared_ptr<RenderDevice> device)
        : mDevice(std::move(device)) {
        mFilter = SamplerFilter::Linear;
        mMipmapFilter = SamplerFilter::Linear;
        mRepeatMode = SamplerRepeatMode::Repeat;
        mBorderColor = SamplerBorderColor::Black;
    }

    Sampler::~Sampler() {
        releaseHandle();
    }

    void Sampler::setHighQualityFiltering() {
        releaseHandle();

        mFilter = SamplerFilter::Linear;
        mMipmapFilter = SamplerFilter::Linear;
        mRepeatMode = SamplerRepeatMode::Repeat;
        mBorderColor = SamplerBorderColor::Black;

        RenderDevice::SamplerDesc samplerDesc{};
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
            mHandle = ID<RenderDevice::Sampler>();
        }
    }

    bool Sampler::isValidHandle() {
        return mHandle.isNotNull();
    }

}