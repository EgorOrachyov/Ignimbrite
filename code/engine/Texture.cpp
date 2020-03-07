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

    Texture::Texture(RefCounted<ignimbrite::IRenderDevice> device)
        : mDevice(std::move(device)) {

    }

    Texture::~Texture() {
        releaseHandle();
    }

    void Texture::setSampler(RefCounted<Sampler> sampler) {
        mSampler = std::move(sampler);
    }

    void Texture::setAsRGBA8(ignimbrite::uint32 width, ignimbrite::uint32 height) {
        Texture::setDataAsRGBA8(width, height, nullptr);
    }

    void Texture::setAsD32S8(ignimbrite::uint32 width, ignimbrite::uint32 height) {
        if (mHandle.isNotNull())
            return;

        mWidth = width;
        mHeight = height;
        mStride = 4 * width;
        mDataFormat = DataFormat::D32_SFLOAT_S8_UINT;

        IRenderDevice::TextureDesc textureDesc{};
        textureDesc.data = nullptr;
        textureDesc.format = mDataFormat;
        textureDesc.width = mWidth;
        textureDesc.height = mHeight;
        textureDesc.size = mStride * mWidth;
        textureDesc.type = TextureType::Texture2D;
        textureDesc.usageFlags = (uint32) TextureUsageBit::ShaderSampling | (uint32) TextureUsageBit::DepthStencilAttachment;

        mHandle = mDevice->createTexture(textureDesc);

        if (mHandle.isNull())
            throw std::runtime_error("Failed to create texture object");
    }

    void Texture::setDataAsRGBA8(uint32 width, uint32 height, const uint8* data) {
        if (mHandle.isNotNull()) {
            return;
        }

        mWidth = width;
        mHeight = height;
        mStride = 4 * width;
        mDataFormat = DataFormat::R8G8B8A8_UNORM;

        if (data != nullptr) {
            mData.reserve(getSize());
            for (uint32 i = 0; i < getSize(); i++) {
                mData.push_back(data[i]);
            }
        }

        IRenderDevice::TextureDesc textureDesc{};
        textureDesc.data = data;
        textureDesc.format = mDataFormat;
        textureDesc.width = mWidth;
        textureDesc.height = mHeight;
        textureDesc.size = mStride * mWidth;
        textureDesc.type = TextureType::Texture2D;
        textureDesc.usageFlags = (uint32) TextureUsageBit::ShaderSampling | (uint32) TextureUsageBit::ColorAttachment;

        mHandle = mDevice->createTexture(textureDesc);

        if (mHandle.isNull()) {
            // todo: do something
        }
    }

    void Texture::releaseHandle() {
        if (mHandle.isNotNull()) {
            mDevice->destroyTexture(mHandle);
            mHandle = ID<IRenderDevice::Texture>();
        }
    }

    bool Texture::isValidHandle() {
        return mHandle.isNotNull();
    }

}