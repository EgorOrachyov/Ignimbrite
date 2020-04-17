/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                        */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#ifndef IGNIMBRITE_SAMPLER_H
#define IGNIMBRITE_SAMPLER_H

#include <IRenderDevice.h>
#include <CacheItem.h>
#include <IncludeStd.h>

namespace ignimbrite {

    class Sampler : public CacheItem {
    public:
        explicit Sampler(RefCounted<IRenderDevice> device);
        ~Sampler() override;

        void setHighQualityFiltering(SamplerRepeatMode mode = SamplerRepeatMode::Repeat);

        void releaseHandle();
        bool isValidHandle();

        SamplerFilter getFilter() const { return mFilter; }
        SamplerFilter getMipmapFilter() const { return mMipmapFilter; }
        SamplerBorderColor getBorderColor() const { return mBorderColor; }
        SamplerRepeatMode getRepeatMode() const { return mRepeatMode; }
        const ID<IRenderDevice::Sampler> &getHandle() const { return mHandle; }

    private:
        SamplerFilter mFilter;
        SamplerFilter mMipmapFilter;
        SamplerBorderColor mBorderColor;
        SamplerRepeatMode mRepeatMode;
        /** Actual resource */
        ID<IRenderDevice::Sampler> mHandle;
        /** Render device for lower API access */
        RefCounted<IRenderDevice> mDevice;
    };

}

#endif //IGNIMBRITE_SAMPLER_H