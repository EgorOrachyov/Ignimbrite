/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                        */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#ifndef IGNIMBRITE_INVERSEFILTER_H
#define IGNIMBRITE_INVERSEFILTER_H

#include <IPostEffect.h>
#include <Material.h>


namespace ignimbrite {

    class InverseFilter : public IPostEffect {
    public:

        InverseFilter(RefCounted<IRenderDevice> device, String path);

        ~InverseFilter() override;

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

#endif //IGNIMBRITE_INVERSEFILTER_H