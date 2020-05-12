/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                        */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#ifndef IGNIMBRITE_IRENDERENGINE_H
#define IGNIMBRITE_IRENDERENGINE_H

#include <Light.h>
#include <Camera.h>
#include <Texture.h>
#include <IPostEffect.h>
#include <IRenderable.h>
#include <IRenderDevice.h>
#include <IPresentationPass.h>

namespace ignimbrite {

    /**
     * @brief Rendering engine interface
     */
    class IRenderEngine {
    public:

        virtual ~IRenderEngine() = default;

        virtual void setCamera(RefCounted<Camera> camera) = 0;
        virtual void setRenderDevice(RefCounted<IRenderDevice> device) = 0;
        virtual void setTargetSurface(ID<IRenderDevice::Surface> surface) = 0;
        virtual void setShadowTarget(RefCounted<Light> light, RefCounted<RenderTarget> target) = 0;
        virtual void setRenderArea(uint32 x, uint32 y, uint32 w, uint32 h) = 0;
        virtual void setPresentationPass(RefCounted<IPresentationPass> presentationPass) = 0;

        virtual void addRenderable(RefCounted<IRenderable> object) = 0;
        virtual void removeRenderable(const RefCounted <IRenderable> &object) = 0;

        virtual void addLightSource(RefCounted<Light> light) = 0;
        virtual void removeLightSource(const RefCounted <Light> &light) = 0;

        virtual void addPostEffect(RefCounted<IPostEffect> effect) = 0;
        virtual void removePostEffect(const RefCounted<IPostEffect> &effect) = 0;

        virtual void addScreenPoint2d(const Vec2f &p, const Vec4f &color, float size) = 0;
        virtual void addScreenLine2d(const Vec2f &a, const Vec2f &b, const Vec4f &color, float width) = 0;
        virtual void addPoint3d(const Vec3f &p, const Vec4f &color, float size) = 0;
        virtual void addLine3d(const Vec3f &a, const Vec3f &b, const Vec4f &color, float width) = 0;

        virtual RefCounted<Texture> getDefaultWhiteTexture() = 0;

        virtual void draw() = 0;

        virtual const RefCounted<RenderTarget::Format> &getShadowTargetFormat() const = 0;
        virtual const RefCounted<RenderTarget::Format> &getOffscreenTargetFormat() const = 0;
        virtual const String& getName();

    };

}

#endif //IGNIMBRITE_IRENDERENGINE_H