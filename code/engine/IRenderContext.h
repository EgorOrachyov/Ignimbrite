/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                        */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#ifndef IGNIMBRITE_IRENDERCONTEXT_H
#define IGNIMBRITE_IRENDERCONTEXT_H

#include <IncludeStd.h>
#include <IncludeMath.h>
#include <Light.h>
#include <Camera.h>
#include <IRenderDevice.h>

namespace ignimbrite {

    /**
     * @brief Shared global state context
     * Rendering engine context to avoid unnecessary global state references.
     */
    class IRenderContext {
    public:

        virtual ~IRenderContext() = default;

        Camera* getCamera() const { return mCamera; }
        Camera* getDebugCamera() const { return mDebugCamera; }
        Frustum* getViewFrustum() const { return mViewFrustum; }
        Light* getGlobalLight() const { return mGlobalLight; }
        const RefCounted<Texture> &getShadowMap() const { return mShadowsRenderTarget->getDepthStencilAttachment(); }
        RenderTarget* getShadowsRenderTarget() const { return mShadowsRenderTarget; }

        IRenderDevice* getRenderDevice() const { return mRenderDevice; }

        bool renderShadows() const { return mRenderShadows; }
        bool renderDebugInfo() const { return mRenderDebugInfo; }

        void setRenderDevice(IRenderDevice* device) { mRenderDevice = device; }
        void setCamera(Camera* camera) { mCamera = camera; }
        void setGlobalLight(Light* light) { mGlobalLight = light; }
        void setShadowsRenderTarget(RenderTarget* target) { mShadowsRenderTarget = target; }

    protected:

        /** Device for low level API access */
        IRenderDevice* mRenderDevice = nullptr;

        /** Scene rendering camera */
        Camera* mCamera = nullptr;
        /** Debug view camera */
        Camera* mDebugCamera = nullptr;
        /** Scene custom view */
        Frustum* mViewFrustum = nullptr;

        /** Global (directional) scene light */
        Light* mGlobalLight = nullptr;
        /** Shadows render target for global directional light */
        RenderTarget* mShadowsRenderTarget;

        /** Rendering config flags */
        bool mRenderShadows = false;
        bool mRenderDebugInfo = false;

        // todo: Other scene light sources
        std::vector<Light*> mSceneLights;
    };

}

#endif //IGNIMBRITE_IRENDERCONTEXT_H