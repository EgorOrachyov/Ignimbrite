/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                        */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#include <RenderTarget.h>

namespace ignimbrite {

    void createColorTexture(uint32 width, uint32 height, RefCounted<Texture> &texture, RefCounted<IRenderDevice> &device) {
        texture = std::make_shared<Texture>(device);
        texture->setAsRGBA8(width, height);
    }

    void createDepthStencilTexture(uint32 width, uint32 height, RefCounted<Texture> &texture, RefCounted<IRenderDevice> &device) {
        texture = std::make_shared<Texture>(device);
        texture->setAsD32S8(width, height);
    }

    bool checkCompatibility(const RenderTarget::Format &format1, const RenderTarget::Format &format2) {
        if (format1.handle == format2.handle)
            return true;

        if (format1.attachments.size() != format2.attachments.size())
            return false;

        for (uint32 i = 0; i < format1.attachments.size(); i++) {
            const auto &attachment1 = format1.attachments[i];
            const auto &attachment2 = format2.attachments[i];

            if (attachment1.format != attachment2.format    ||
                attachment1.samples != attachment2.samples  ||
                attachment1.type != attachment2.type)
                return false;
        }

        return true;
    }

    bool checkAllNotNull(const std::vector<RefCounted<Texture>> &textures) {
        for (const auto &t: textures) {
            if (t == nullptr) {
                return false;
            }
        }

        return true;
    }

    RenderTarget::Format::Format(ignimbrite::RefCounted<ignimbrite::IRenderDevice> device)
        : device(std::move(device)) {

    }

    RenderTarget::Format::~Format() {
        if (handle.isNotNull()) {
            device->destroyFramebufferFormat(handle);
            handle = ID<IRenderDevice::FramebufferFormat>();
        }
    }

    RenderTarget::RenderTarget(RefCounted<IRenderDevice> device)
        : mDevice(std::move(device) ){
        
    }

    RenderTarget::~RenderTarget() {
        releaseHandle();
    }

    void RenderTarget::setTargetProperties(uint32 width, uint32 height, uint32 colorAttachmentsCount) {
        if (width == 0 || height == 0)
            throw std::runtime_error("Specified invalid render target size");

        mWidth = width;
        mHeight = height;
        mColorAttachments.resize(colorAttachmentsCount);
    }
    
    void RenderTarget::setColorAttachment(uint32 index, RefCounted<Texture> attachment) {
        if (index >= getColorAttachmentsCount())
            throw std::runtime_error("Index of attachment is out of bounds");

        if (attachment == nullptr)
            throw std::runtime_error("Null attachment pointer");

        mColorAttachments[index] = std::move(attachment);
    }
    
    void RenderTarget::setDepthStencilAttachment(RefCounted<Texture> attachment) {
        if (attachment == nullptr)
            throw std::runtime_error("Null attachment pointer");

        mDepthStencilAttachment = std::move(attachment);
    }

    void RenderTarget::setFramebufferFormat(RefCounted<RenderTarget::Format> framebufferFormat) {
        mFramebufferFormat = std::move(framebufferFormat);
    }
    
    void RenderTarget::create() {
        if (mHandle.isNotNull())
            throw std::runtime_error("An attempt to recreate render target");

        if (getTotalAttachmentsCount() == 0)
            throw std::runtime_error("An attempt to create render target with no attachments");

        if (!checkAllNotNull(mColorAttachments))
            throw std::runtime_error("Incomplete specification of color attachments");

        Format format(mDevice);
        getFramebufferFormatDescription(format.attachments);

        if (mFramebufferFormat != nullptr) {
            bool areCompatible = checkCompatibility(format, *mFramebufferFormat);

            if (!areCompatible)
                throw std::runtime_error("Specified framebuffer format is incompatible with render target");
        }
        else {
            mFramebufferFormat = std::make_shared<Format>(std::move(format));
            mFramebufferFormat->handle = mDevice->createFramebufferFormat(mFramebufferFormat->attachments);
        }

        std::vector<ID<IRenderDevice::Texture>> attachments;
        attachments.reserve(getTotalAttachmentsCount());
        for (const auto& color: mColorAttachments) {
            attachments.push_back(color->getHandle());
        }

        if (hasDepthStencilAttachment()) {
            attachments.push_back(mDepthStencilAttachment->getHandle());
        }

        mHandle = mDevice->createFramebuffer(attachments, mFramebufferFormat->handle);

        if (mHandle.isNull())
            throw std::runtime_error("Failed to create framebuffer object");
    }

    void RenderTarget::releaseHandle() {
        if (mHandle.isNotNull()) {
            mDevice->destroyFramebuffer(mHandle);
            mHandle = ID<IRenderDevice::Framebuffer>();
        }
    }

    void RenderTarget::createTargetFromFormat(uint32 width, uint32 height, DefaultFormat format) {
        mWidth = width;
        mHeight = height;

        switch (format) {
            case DefaultFormat::Color0: {
                RefCounted<Texture> color0;
                createColorTexture(mWidth, mHeight, color0, mDevice);

                setTargetProperties(mWidth, mHeight, 1);
                setColorAttachment(0, color0);
            }
                break;
            case DefaultFormat::DepthStencil: {
                RefCounted<Texture> depth;
                createDepthStencilTexture(mWidth, mHeight, depth, mDevice);

                setTargetProperties(mWidth, mHeight, 0);
                setDepthStencilAttachment(depth);
            }
                break;
            case DefaultFormat::Color0AndDepthStencil: {
                RefCounted<Texture> color0;
                createColorTexture(mWidth, mHeight, color0, mDevice);
                RefCounted<Texture> depth;
                createDepthStencilTexture(mWidth, mHeight, depth, mDevice);

                setTargetProperties(mWidth, mHeight, 1);
                setColorAttachment(0, color0);
                setDepthStencilAttachment(depth);
            }
                break;
        }

        create();
    }

    void RenderTarget::getFramebufferFormatDescription(std::vector<IRenderDevice::FramebufferAttachmentDesc> &attachments) {
        attachments.reserve(getTotalAttachmentsCount());

        for (const auto &color: mColorAttachments) {
            IRenderDevice::FramebufferAttachmentDesc attachmentDesc{};
            attachmentDesc.format = color->getDataFormat();
            attachmentDesc.samples = TextureSamples::Samples1;
            attachmentDesc.type = AttachmentType::Color;
            attachments.push_back(attachmentDesc);
        }

        if (hasDepthStencilAttachment()) {
            IRenderDevice::FramebufferAttachmentDesc attachmentDesc{};
            attachmentDesc.format = mDepthStencilAttachment->getDataFormat();
            attachmentDesc.samples = TextureSamples::Samples1;
            attachmentDesc.type = AttachmentType::DepthStencil;
            attachments.push_back(attachmentDesc);
        }
    }

    uint32 RenderTarget::getTotalAttachmentsCount() const {
        return getColorAttachmentsCount() + (hasDepthStencilAttachment() ? 1 : 0);
    }
    
    const RefCounted<Texture>& RenderTarget::getAttachment(uint32 index) const {
        if (index >= getColorAttachmentsCount())
            throw std::runtime_error("Index of color attachment is out of bounds");

        return mColorAttachments[index];
    }

}