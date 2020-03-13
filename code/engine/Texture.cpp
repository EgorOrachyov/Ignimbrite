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
        if (mHandle.isNotNull())
            throw std::runtime_error("An attempt to recreate texture");

        mWidth = width;
        mHeight = height;
        mStride = 4 * width;
        mDataFormat = DataFormat::R8G8B8A8_UNORM;

        IRenderDevice::TextureDesc textureDesc{};
        textureDesc.data = nullptr;
        textureDesc.format = mDataFormat;
        textureDesc.width = mWidth;
        textureDesc.height = mHeight;
        textureDesc.depth = 1;
        textureDesc.size = mStride * mWidth;
        textureDesc.type = TextureType::Texture2D;
        textureDesc.usageFlags = (uint32) TextureUsageBit::ShaderSampling | (uint32) TextureUsageBit::ColorAttachment;
        textureDesc.mipmaps = 1;

        mHandle = mDevice->createTexture(textureDesc);

        if (mHandle.isNull())
            throw std::runtime_error("Failed to create texture object");    }

    void Texture::setAsD32S8(ignimbrite::uint32 width, ignimbrite::uint32 height) {
        if (mHandle.isNotNull())
            throw std::runtime_error("An attempt to recreate texture");

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

    void Texture::setDataAsRGBA8(uint32 width, uint32 height, const uint8* data, bool genMipmaps) {
        if (mHandle.isNotNull())
            throw std::runtime_error("An attempt to recreate texture");

        if (data == nullptr)
            throw std::runtime_error("Data must be specified for texture creation");

        mWidth = width;
        mHeight = height;
        mStride = 4 * width;
        mDataFormat = DataFormat::R8G8B8A8_UNORM;

        IRenderDevice::TextureDesc textureDesc{};
        textureDesc.data = data;
        textureDesc.format = mDataFormat;
        textureDesc.width = mWidth;
        textureDesc.height = mHeight;
        textureDesc.depth = 1;
        textureDesc.size = mStride * mWidth;
        textureDesc.type = TextureType::Texture2D;
        textureDesc.usageFlags = (uint32) TextureUsageBit::ShaderSampling;
        textureDesc.mipmaps = (genMipmaps ? (uint32)std::floor(std::log2(std::max(width, height))) + 1 : 1);

        mHandle = mDevice->createTexture(textureDesc);

        if (mHandle.isNull())
            throw std::runtime_error("Failed to create texture object");
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