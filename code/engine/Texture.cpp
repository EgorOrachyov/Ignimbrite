/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                        */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#include <Texture.h>

namespace ignimbrite {

    Texture::Texture(RefCounted<ignimbrite::RenderDevice> device)
        : mDevice(std::move(device)) {

    }

    Texture::~Texture() {
        releaseHandle();
    }

    void Texture::setSampler(RefCounted<Sampler> sampler) {
        mSampler = std::move(sampler);
    }

    void Texture::setDataAsRGBA8(uint32 width, uint32 height, const uint8* data) {
        releaseHandle();

        mWidth = width;
        mHeight = height;
        mStride = 4 * width;
        mDataFormat = DataFormat::R8G8B8A8_UNORM;

        mData.reserve(getSize());
        for (uint32 i = 0; i < getSize(); i++) {
            mData.push_back(data[i]);
        }

        RenderDevice::TextureDesc textureDesc{};
        textureDesc.data = mData.data();
        textureDesc.format = mDataFormat;
        textureDesc.width = mWidth;
        textureDesc.height = height;
        textureDesc.size = mStride * mWidth;
        textureDesc.type = TextureType::Texture2D;
        textureDesc.usageFlags = (uint32) TextureUsageBit::ShaderSampling;

        mHandle = mDevice->createTexture(textureDesc);

        if (mHandle.isNull()) {
            // todo: do something
        }
    }

    void Texture::releaseHandle() {
        if (mHandle.isNotNull()) {
            mDevice->destroyTexture(mHandle);
            mHandle = ID<RenderDevice::Texture>();
        }
    }

    bool Texture::isValidHandle() {
        return mHandle.isNotNull();
    }

}