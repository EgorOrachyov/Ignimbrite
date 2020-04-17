/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                        */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#ifndef IGNIMBRITE_VERTEXLAYOUTFACTORY_H
#define IGNIMBRITE_VERTEXLAYOUTFACTORY_H

#include <Mesh.h>
#include <IRenderDevice.h>

namespace ignimbrite {

    class VertexLayoutFactory {
    public:

        static void createVertexLayoutDesc(Mesh::VertexFormat format, IRenderDevice::VertexBufferLayoutDesc& bufferDesc);

    };

}

#endif //IGNIMBRITE_VERTEXLAYOUTFACTORY_H