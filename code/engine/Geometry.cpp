/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019, 2020  Egor Orachyov                                        */
/* Copyright (c) 2019, 2020  Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#include <Geometry.h>

namespace ignimbrite {

    void Geometry::createFullscreenQuad(ID<IRenderDevice::VertexBuffer> &vertexBuffer, RefCounted<IRenderDevice> &device) {
        createRegionQuad(vertexBuffer, -1.0f, -1.0f, 1.0f, 1.0f, device);
    }

    void Geometry::createRegionQuad(
            ID<IRenderDevice::VertexBuffer> &vertexBuffer,
            float x0, float y0, float x1, float y1,
            RefCounted<IRenderDevice> &device) {

        float32 data[] = {
                x0, y0, 0.0f, 0.0f,
                x0, y1, 0.0f, 1.0f,
                x1, y1, 1.0f, 1.0f,
                x1, y1, 1.0f, 1.0f,
                x1, y0, 1.0f, 0.0f,
                x0, y0, 0.0f, 0.0f,
        };

        vertexBuffer = device->createVertexBuffer(BufferUsage::Static, sizeof(data), data);
    }
}