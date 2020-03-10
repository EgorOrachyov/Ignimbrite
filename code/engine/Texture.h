/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                        */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#ifndef IGNIMBRITE_TEXTURE_H
#define IGNIMBRITE_TEXTURE_H

#include <Sampler.h>

namespace ignimbrite {

    class Texture : public CacheItem {
    public:
        explicit Texture(RefCounted<IRenderDevice> device);
        ~Texture() override;

        void setSampler(RefCounted<Sampler> sampler);
        void setAsRGBA8(uint32 width, uint32 height);
        void setAsD32S8(uint32 width, uint32 height);
        void setDataAsRGBA8(uint32 width, uint32 height, const uint8 *data);
        void releaseHandle();
        bool isValidHandle();

        uint32 getWidth() const { return mWidth; }
        uint32 getHeight() const { return mHeight; }
        uint32 getStride() const { return mStride; }
        uint32 getSize() const { return mStride * mHeight; }
        DataFormat getDataFormat() const { return mDataFormat; }
        const std::vector<uint8> &getData() const { return mData; }
        const RefCounted<Sampler> &getSampler() const { return mSampler; }
        const ID<IRenderDevice::Texture> &getHandle() const { return mHandle; }

    private:
        /** In pixels */
        uint32 mWidth = 0;
        /** In pixels */
        uint32 mHeight = 0;
        /** Size of single line of image in bytes */
        uint32 mStride = 0;
        /** Format of pixels */
        DataFormat mDataFormat = DataFormat::R8G8B8A8_UNORM;
        /** Texture data on CPU (duplicate for some reason) */
        std::vector<uint8> mData;
        /** Sampler, for filtering this texture */
        RefCounted<Sampler> mSampler;
        /** Actual texture resource */
        ID<IRenderDevice::Texture> mHandle;
        /** Render device for lower API access */
        RefCounted<IRenderDevice> mDevice;
    };

}



#endif //IGNIMBRITE_TEXTURE_H