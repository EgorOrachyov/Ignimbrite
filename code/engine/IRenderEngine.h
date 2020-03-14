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

#include <IncludeStd.h>
#include <IRenderable.h>
#include <IRenderDevice.h>

namespace ignimbrite {

    /**
     * @brief Rendering engine interface
     */
    class IRenderEngine {
    public:

        virtual ~IRenderEngine() = default;

        virtual void setRenderDevice(RefCounted<IRenderDevice> device) = 0;
        virtual void setTargetSurface(ID<IRenderDevice::Surface> surface) = 0;

        virtual void addRenderable(RefCounted<IRenderable> object) = 0;
        virtual void removeRenderable(RefCounted<IRenderable> object) = 0;

        virtual void addLightSource(/** RefCounted<LightSource> light */) = 0;
        virtual void removeLightSource(/** RefCounted<LightSource> light */) = 0;

        virtual void draw() = 0;

        virtual const String& getName();

    };

}

#endif //IGNIMBRITE_IRENDERENGINE_H