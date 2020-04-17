/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                        */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#include <VertexLayoutFactory.h>

namespace ignimbrite {

    struct VertPf {
        float32 pos[3];
    };

    struct VertPNf {
        float32 pos[3];
        float32 norm[3];
    };

    struct VertPNTf {
        float32 pos[3];
        float32 norm[3];
        float32 texcoords[2];
    };


    void VertexLayoutFactory::createVertexLayoutDesc(Mesh::VertexFormat format, IRenderDevice::VertexBufferLayoutDesc &bufferDesc) {
        switch (format) {
            case Mesh::VertexFormat::P: {
                bufferDesc.usage = VertexUsage::PerVertex;
                bufferDesc.stride = sizeof(VertPf);
                bufferDesc.attributes.resize(1);
                auto& attr = bufferDesc.attributes;
                attr[0].format = DataFormat::R32G32B32_SFLOAT;
                attr[0].offset = offsetof(VertPf, pos);
                attr[0].location = 0;
            }
            break;
            case Mesh::VertexFormat::PN: {
                bufferDesc.usage = VertexUsage::PerVertex;
                bufferDesc.stride = sizeof(VertPNf);
                bufferDesc.attributes.resize(2);
                auto& attr = bufferDesc.attributes;
                attr[0].format = DataFormat::R32G32B32_SFLOAT;
                attr[0].offset = offsetof(VertPNf, pos);
                attr[0].location = 0;
                attr[1].format = DataFormat::R32G32B32_SFLOAT;
                attr[1].offset = offsetof(VertPNf, norm);
                attr[1].location = 1;
            }
            break;
            case Mesh::VertexFormat::PNT: {
                bufferDesc.usage = VertexUsage::PerVertex;
                bufferDesc.stride = sizeof(VertPNTf);
                bufferDesc.attributes.resize(3);
                auto& attr = bufferDesc.attributes;
                attr[0].format = DataFormat::R32G32B32_SFLOAT;
                attr[0].offset = offsetof(VertPNTf, pos);
                attr[0].location = 0;
                attr[1].format = DataFormat::R32G32B32_SFLOAT;
                attr[1].offset = offsetof(VertPNTf, norm);
                attr[1].location = 1;
                attr[2].format = DataFormat::R32G32_SFLOAT;
                attr[2].offset = offsetof(VertPNTf, texcoords);
                attr[2].location = 2;
            }
            break;
            default:
                throw std::runtime_error("Unsupported vertex format");
        }
    }

}