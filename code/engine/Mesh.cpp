/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                        */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                 */
/**********************************************************************************/


#include <stdexcept>
#include <cstring>
#include "Mesh.h"

namespace ignimbrite {

    Mesh::Mesh(Mesh::VertexFormat format, uint32 vertexCount, uint32 indexCount) {
        mVertexFormat = format;
        mStride = getSizeOfStride(format);
        mVertexCount = vertexCount;
        mVertexData.resize(mStride * vertexCount);
        mIndexData.resize(indexCount);
    }
    
    bool Mesh::updateVertexData(uint32 offset, uint32 vertexCount, const uint8 *data) {
        if ((offset + vertexCount) * mStride <= mVertexData.size()) {
            memcpy(mVertexData.data() + offset * mStride, data, vertexCount * mStride);
            return true;
        }

        return false;
    }
    
    bool Mesh::updateIndexData(uint32 offset, uint32 indexCount, const uint32 *data) {
        if (offset + indexCount <= mIndexData.size()) {
            memcpy(mIndexData.data() + offset, data, indexCount * sizeof(uint32));
            return true;
        }

        return false;
    }

    void Mesh::updateBoundingVolume() {
        uint32 offset = 0;
        mBoundingBox = AABB();
        const uint8* data = getVertexData();

        switch (mVertexFormat) {
            case VertexFormat::PNT: {
                for (uint32 i = 0; i < mVertexCount; i++) {
                    const Vec3f* pos = (Vec3f*)(data + offset);
                    mBoundingBox.expandToContain(*pos);
                    offset += getStride();
                }
            }
            break;

            default:
                throw std::runtime_error("Unsupported vertex format");
        }
    }

    uint32 Mesh::getNumberOfAttributes(Mesh::VertexFormat format) {
        switch (format) {
            case VertexFormat::P:
                return 1;
            case VertexFormat::PN:
                return 2;
            case VertexFormat::PNT:
                return 3;
            default:
                return 0;
        }
    }
    
    uint32 Mesh::getSizeOfStride(Mesh::VertexFormat format) {
        uint32 size = 0;
        uint32 mask = (uint32) format;

        if (mask & Pos3f) size += sizeof(float32) * 3;
        if (mask & Norm3f) size += sizeof(float32) * 3;
        if (mask & TexCoords2f) size += sizeof(float32) * 2;

        return size;
    }
    
}