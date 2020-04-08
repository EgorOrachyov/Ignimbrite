/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                        */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#ifndef IGNIMBRITE_GEOMETRY_H
#define IGNIMBRITE_GEOMETRY_H

#include <Types.h>
#include <IncludeStd.h>
#include <IRenderDevice.h>

namespace ignimbrite {

    class Geometry {
    public:

        static void createFullscreenQuad(ID<IRenderDevice::VertexBuffer> &vertexBuffer, const RefCounted<IRenderDevice> &device);

    };

}

#endif //IGNIMBRITE_GEOMETRY_H