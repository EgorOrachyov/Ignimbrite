/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                        */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#include <Geometry.h>

namespace ignimbrite {

    void Geometry::createFullscreenQuad(ID<IRenderDevice::VertexBuffer> &vertexBuffer, const RefCounted<IRenderDevice> &device) {

        float32 data[] = {
                -1.0f, -1.0f, 0.0f, 0.0f,
                -1.0f,  1.0f, 0.0f, 1.0f,
                 1.0f,  1.0f, 1.0f, 1.0f,
                 1.0f,  1.0f, 1.0f, 1.0f,
                 1.0f, -1.0f, 1.0f, 0.0f,
                -1.0f, -1.0f, 0.0f, 0.0f,
        };

        vertexBuffer = device->createVertexBuffer(BufferUsage::Static, sizeof(data), data);
    }

}