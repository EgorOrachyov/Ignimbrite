/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                       */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                */
/**********************************************************************************/


#ifndef IGNIMBRITE_PRESENTATIONPASS_H
#define IGNIMBRITE_PRESENTATIONPASS_H

#include <IPresentationPass.h>
#include <Material.h>

namespace ignimbrite {

    class PresentationPass : public IPresentationPass {
    public:
        explicit PresentationPass(
                RefCounted<IRenderDevice> device,
                RefCounted<Texture> default2dTexture,
                RefCounted<Material> presentationMaterial);
        ~PresentationPass();

        /**
         * Enable showing of depth buffer content.
         * @param depthBufferAreaLU left upper corner. (-1,-1) is screen's left upper corner
         * @param depthBufferAreaRB right bottom corner. (1,1) is screen's right bottom corner
         * */
        void enableDepthShow(const Vec2f &depthBufferAreaLU, const Vec2f &depthBufferAreaRB);
        void enableDepthShow();
        void disableDepthShow();
        bool isDepthShown() const;

        void present(ID<IRenderDevice::Surface> targetSurface, IRenderDevice::Region surfaceRegion,
                     RefCounted<RenderTarget> source) override;

    public:
        RefCounted<IRenderDevice> mDevice;

        ID<IRenderDevice::VertexBuffer> mFullscreenQuad;
        ID<IRenderDevice::VertexBuffer> mDepthRegionQuad;
        RefCounted<Material> mPresentationMaterial;
        RefCounted<Material> mDepthPresentationMaterial;

        bool mShowDepthBuffer;
        Vec4f mDepthBufferArea;
    };

}

#endif //IGNIMBRITE_PRESENTATIONPASS_H
