/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                        */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#ifndef IGNIMBRITE_RENDERTARGET_H
#define IGNIMBRITE_RENDERTARGET_H

#include <Texture.h>

namespace ignimbrite {

    /**
     * @brief Rendering target object
     *
     * Represents rendering target - set of textures, which can be bound
     * with rendering pipeline as an output data buffers (i.e frame buffers).
     * Provides default formats of render targets.
     *
     * Allows to create custom target with desired number of texture attachments
     * in the following manner:
     *
     * @code
     * target.setTargetProperties(w, h, N+1);
     * // all the attachments from 0
     * target.setAttachment(0, colorAttachment0);
     * // ... to N are set in this way
     * target.setAttachment(N, colorAttachmentN);
     * // optional depth stencil buffer
     * target.setDepthStencilAttachment(depthStencil);
     * // creates actual render device framebuffer
     * target.create();
     * @endcode
     */
    class RenderTarget : public CacheItem {
    public:

        /** Framebuffer format possibly shared among different render targets or pipelines */
        struct Format {
            explicit Format(RefCounted<IRenderDevice> device);
            ~Format();

            bool hasDepthStencilAttachment() const;
            const ID<IRenderDevice::FramebufferFormat> &getFormatHandle() const;
            const std::vector<IRenderDevice::FramebufferAttachmentDesc> &getAttachments() const;

        private:

            friend class RenderTarget;

            std::vector<IRenderDevice::FramebufferAttachmentDesc> mAttachments;
            ID<IRenderDevice::FramebufferFormat> mFormatHandle;
            RefCounted<IRenderDevice> mRenderDevice;

            bool mHasDepthStencilAttachment = false;
        };

        /** Default formats for render targets */
        enum class DefaultFormat : uint32 {
            Color0,
            DepthStencil,
            Color0AndDepthStencil
        };

        explicit RenderTarget(RefCounted<IRenderDevice> device);
        ~RenderTarget() override;

        void setTargetProperties(uint32 width, uint32 height, uint32 colorAttachmentsCount);
        void setColorAttachment(uint32 index, RefCounted<Texture> attachment);
        void setDepthStencilAttachment(RefCounted<Texture> attachment);
        void setFramebufferFormat(RefCounted<Format> framebufferFormat);
        void create();
        void releaseHandle();

        void createTargetFromFormat(uint32 width, uint32 height, DefaultFormat format);
        void getFramebufferFormatDescription(std::vector<IRenderDevice::FramebufferAttachmentDesc> &attachments);

        uint32 getWidth() const { return mWidth; }
        uint32 getHeight() const { return mHeight; }
        uint32 getColorAttachmentsCount() const { return (uint32) mColorAttachments.size(); }
        uint32 getTotalAttachmentsCount() const;
        bool hasDepthStencilAttachment() const { return mDepthStencilAttachment != nullptr; }
        const ID<IRenderDevice::Framebuffer> &getHandle() const { return mHandle; }
        const RefCounted<Texture> &getAttachment(uint32 index) const;
        const RefCounted<Texture> &getDepthStencilAttachment() const { return mDepthStencilAttachment; }
        const RefCounted<Format> &getFramebufferFormat() const { return mFramebufferFormat; }

    private:

        /** In pixels */
        uint32 mWidth = 0;
        /** In pixels */
        uint32 mHeight = 0;

        /** Render device framebuffer handle */
        ID<IRenderDevice::Framebuffer> mHandle;

        /** Render device access */
        RefCounted<IRenderDevice> mDevice;

        /** Optional depth stencil attachment (store separately for more clear usage) */
        RefCounted<Texture> mDepthStencilAttachment;

        /** Framebuffer format of this render target (possibly shared among other targets) */
        RefCounted<Format> mFramebufferFormat;

        /** Color attachments of the target (may be empty) */
        std::vector<RefCounted<Texture>> mColorAttachments;


    };

}

#endif //IGNIMBRITE_RENDERTARGET_H