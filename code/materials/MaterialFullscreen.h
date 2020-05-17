/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019, 2020  Egor Orachyov                                        */
/* Copyright (c) 2019, 2020  Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#ifndef IGNIMBRITE_MATERIALFULLSCREEN_H
#define IGNIMBRITE_MATERIALFULLSCREEN_H

#include <Material.h>

namespace ignimbrite {

    class MaterialFullscreen {
    public:

        static RefCounted<Material> screenMaterialSpv(const String &vertexName, const String &fragmentName, const RefCounted<RenderTarget::Format> &format, const RefCounted<IRenderDevice> &device);
        static RefCounted<Material> screenMaterialSpv(const String &vertexName, const String &fragmentName, const ID<IRenderDevice::Surface> &surface, const RefCounted<IRenderDevice> &device);

        static RefCounted<Material> fullscreenQuad(const String &shadersFolderPath, ID<IRenderDevice::Surface> surface, const RefCounted<IRenderDevice> &device);
        static RefCounted<Material> fullscreenQuadLinearDepth(const String &shadersFolderPath, ID<IRenderDevice::Surface> surface, const RefCounted<IRenderDevice> &device);

        static RefCounted<Material> noirFilter(const String &shadersFolderPath, const RefCounted<RenderTarget::Format> &format, const RefCounted<IRenderDevice> &device);

        static RefCounted<Material> inverseFilter(const String &shadersFolderPath, const RefCounted<RenderTarget::Format> &format, const RefCounted<IRenderDevice> &device);


    };
}

#endif //IGNIMBRITE_MATERIALFULLSCREEN_H