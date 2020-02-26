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

#include <RenderDevice.h>
#include <CacheItem.h>
#include <IncludeStd.h>

namespace ignimbrite {

    class Sampler : public CacheItem {
    public:
        explicit Sampler(RefCounted<RenderDevice> device);
        ~Sampler() override;

        void setHighQualityFiltering();

        void releaseHandle();
        bool isValidHandle();

        SamplerFilter getFilter() const { return mFilter; }
        SamplerFilter getMipmapFilter() const { return mMipmapFilter; }
        SamplerBorderColor getBorderColor() const { return mBorderColor; }
        SamplerRepeatMode getRepeatMode() const { return mRepeatMode; }

    private:
        SamplerFilter mFilter;
        SamplerFilter mMipmapFilter;
        SamplerBorderColor mBorderColor;
        SamplerRepeatMode mRepeatMode;
        /** Actual resource */
        ID<RenderDevice::Sampler> mHandle;
        /** Render device for lower API access */
        RefCounted<RenderDevice> mDevice;
    };

}

#endif //IGNIMBRITE_SAMPLER_H