/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019, 2020  Egor Orachyov                                        */
/* Copyright (c) 2019, 2020  Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#ifndef IGNIMBRITE_NOIRFILTER_H
#define IGNIMBRITE_NOIRFILTER_H

#include <IPostEffect.h>
#include <Material.h>

namespace ignimbrite {

    class NoirFilter : public IPostEffect {
    public:

        NoirFilter(RefCounted<IRenderDevice> device, String folderPath);

        ~NoirFilter() override;

        bool isActive() const override;

        void onAddedToPipeline(const RefCounted<ignimbrite::RenderTarget::Format> &targetFormat) override;

        void execute(RefCounted<RenderTarget> &input, RefCounted<RenderTarget> &output) override;

    private:

        bool mIsActive = true;
        String mPrefixPath;
        RefCounted<Texture> mCachedTedxture0;
        RefCounted<Material> mMaterial;
        RefCounted<IRenderDevice> mDevice;
        ID<IRenderDevice::VertexBuffer> mScreenQuad;

    };

}

#endif //IGNIMBRITE_NOIRFILTER_H