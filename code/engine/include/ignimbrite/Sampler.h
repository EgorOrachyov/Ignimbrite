/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov, Sultim Tsyrendashiev                  */
/**********************************************************************************/

#ifndef IGNIMBRITE_SAMPLER_H
#define IGNIMBRITE_SAMPLER_H

#include <ignimbrite/RenderDevice.h>
#include <ignimbrite/CacheItem.h>
#include <memory>

namespace ignimbrite {

    class Sampler : public CacheItem {
    public:
        explicit Sampler(std::shared_ptr<RenderDevice> device);
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
        std::shared_ptr<RenderDevice> mDevice;
    };

}

#endif //IGNIMBRITE_SAMPLER_H