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

        /**
         * Create vertex buffer that consists of 6 vertices (2 triangles)
         * with 2-float position and 2-float texture coords data.
         */
        static void createFullscreenQuad(ID<IRenderDevice::VertexBuffer> &vertexBuffer, RefCounted<IRenderDevice> &device);

        /**
         * Create vertex buffer that consists of 6 vertices (2 triangles)
         * with 2-float position and 2-float texture coords data.
         * If x0 == -1, y0 = -1 and x1 == 1, y1 == 1 then it will cover full screen.
         * @param x0 left upper corner horizontal
         * @param y0 left upper corner vertical
         * @param x1 right bottom corner horizontal
         * @param y1 right bottom corner vertical
         */
        static void createRegionQuad(ID<IRenderDevice::VertexBuffer> &vertexBuffer,
                float x0, float y0, float x1, float y1, RefCounted<IRenderDevice> &device);

    };

}

#endif //IGNIMBRITE_GEOMETRY_H