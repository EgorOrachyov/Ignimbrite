/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019, 2020  Egor Orachyov                                        */
/* Copyright (c) 2019, 2020  Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#ifndef IGNIMBRITE_RENDERENGINE_H
#define IGNIMBRITE_RENDERENGINE_H

#include <Material.h>
#include <RenderTarget.h>
#include <IRenderEngine.h>
#include <RenderQueueElement.h>
#include <Canvas.h>

namespace ignimbrite {

    class RenderEngine : public IRenderEngine {
    public:

        RenderEngine();

        ~RenderEngine() override;

        void setCamera(RefCounted<Camera> camera) override;

        void setRenderDevice(RefCounted<IRenderDevice> device) override;

        void setTargetSurface(ID<IRenderDevice::Surface> surface) override;

        void setShadowTarget(RefCounted<Light> light, RefCounted<RenderTarget> target) override;

        void setRenderArea(uint32 x, uint32 y, uint32 w, uint32 h) override;

        void setPresentationPass(RefCounted<IPresentationPass> presentationPass) override;

        void addRenderable(RefCounted<IRenderable> object) override;

        void removeRenderable(const RefCounted<IRenderable> &object) override;

        void addLightSource(RefCounted<Light> light) override;

        void removeLightSource(const RefCounted<Light> &light) override;

        void addPostEffect(RefCounted<IPostEffect> effect) override;

        void removePostEffect(const RefCounted<IPostEffect> &effect) override;

        void addScreenPoint2d(const Vec2f &p, const Vec4f &color, float size) override;

        void addScreenLine2d(const Vec2f &a, const Vec2f &b, const Vec4f &color, float width) override;

        void addPoint3d(const Vec3f &p, const Vec4f &color, float size) override;

        void addLine3d(const Vec3f &a, const Vec3f &b, const Vec4f &color, float width) override;

        void draw() override;

        const RefCounted<ignimbrite::RenderTarget::Format> &getShadowTargetFormat() const override;

        const RefCounted<RenderTarget::Format> &getOffscreenTargetFormat() const override;

        const String &getName() override;

        RefCounted<Texture> getDefaultWhiteTexture() override;

    private:

        void CHECK_CAMERA_PRESENT() const;
        void CHECK_DEVICE_PRESENT() const;
        void CHECK_SURFACE_PRESENT() const;
        void CHECK_FINAL_PASS_PRESENT() const;

        struct RenderArea {
            uint32 x = 0, y =0;
            uint32 w = 0, h = 0;
        };

        RenderArea                 mRenderArea;
        RefCounted<Camera>         mCamera;
        RefCounted<IRenderContext> mContext;
        RefCounted<IRenderDevice>  mRenderDevice;
        RefCounted<RenderTarget>   mOffscreenTarget1;
        RefCounted<RenderTarget>   mOffscreenTarget2;
        RefCounted<Canvas>         mCanvas;
        RefCounted<IPresentationPass>   mPresentationPass;
        RefCounted<Texture>             mDefaultWhiteTexture;

        ID<IRenderDevice::Surface>      mTargetSurface;
        ID<IRenderDevice::VertexBuffer> mFullscreenQuad;

        std::vector<RenderQueueElement> mCollectQueue;
        std::vector<RenderQueueElement> mVisibleSortedQueue;

        std::vector<RefCounted<Light>>       mLightSources;
        std::vector<RefCounted<IRenderable>> mRenderObjects;
        std::vector<RefCounted<IPostEffect>> mPostEffects;

        RefCounted<RenderTarget> mShadowsRenderTarget;
        RefCounted<RenderTarget::Format> mShadowTargetFormat;

        std::unordered_map<uint32, std::vector<IRenderable*>> mRenderLayers;

    };


}

#endif //IGNIMBRITE_RENDERENGINE_H
