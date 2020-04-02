/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                        */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#ifndef IGNIMBRITE_RENDERENGINE_H
#define IGNIMBRITE_RENDERENGINE_H

#include <RenderTarget.h>
#include <IRenderEngine.h>
#include <RenderQueueElement.h>

namespace ignimbrite {

    class RenderEngine : public IRenderEngine {
    public:

        RenderEngine();

        ~RenderEngine() override = default;

        void setCamera(RefCounted<Camera> camera) override;

        void setRenderDevice(RefCounted<IRenderDevice> device) override;

        void setTargetSurface(ID<IRenderDevice::Surface> surface) override;

        void setRenderArea(uint32 x, uint32 y, uint32 w, uint32 h) override;

        void addRenderable(RefCounted<IRenderable> object) override;

        void removeRenderable(const RefCounted<IRenderable> &object) override;

        void addLightSource(RefCounted<Light> light) override;

        void removeLightSource(const RefCounted<Light> &light) override;

        void draw() override;

        const String &getName() override;

    private:

        void CHECK_CAMERA_PRESENT() const;
        void CHECK_DEVICE_PRESENT() const;
        void CHECK_SURFACE_PRESENT() const;

        struct RenderArea {
            uint32 x = 0 , y =0;
            uint32 w = 0, h = 0;
        };

        RenderArea                 mRenderArea;
        RefCounted<Camera>         mCamera;
        RefCounted<IRenderContext> mContext;
        RefCounted<IRenderDevice>  mRenderDevice;
        RefCounted<RenderTarget>   mOffscreenTarget;
        ID<IRenderDevice::Surface> mTargetSurface;

        std::vector<RenderQueueElement> mCollectQueue;
        std::vector<RenderQueueElement> mVisibleSortedQueue;

        std::vector<RefCounted<Light>>       mLightSources;
        std::vector<RefCounted<IRenderable>> mRenderObjects;

        std::unordered_map<uint32, std::vector<IRenderable*>> mRenderLayers;

    };


}

#endif //IGNIMBRITE_RENDERENGINE_H
