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
#include <IRenderable.h>
#include <IRenderDevice.h>

namespace ignimbrite {

    /**
     * @brief Rendering engine interface
     */
    class IRenderEngine {
    public:

        virtual ~IRenderEngine() = default;

        virtual void setCamera(/* RefCounted<Camera> camera */);
        virtual void setRenderDevice(RefCounted<IRenderDevice> device) = 0;
        virtual void setTargetSurface(ID<IRenderDevice::Surface> surface) = 0;

        virtual void addRenderable(RefCounted<IRenderable> object) = 0;
        virtual void removeRenderable(RefCounted<IRenderable> object) = 0;

        virtual void addLightSource(RefCounted<Light> light) = 0;
        virtual void removeLightSource(RefCounted<Light> light) = 0;

        virtual void draw() = 0;

        virtual const String& getName();

    };

}

#endif //IGNIMBRITE_IRENDERENGINE_H