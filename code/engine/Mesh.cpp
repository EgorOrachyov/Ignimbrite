/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                       */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                */
/**********************************************************************************/


#include <stdexcept>
#include "Mesh.h"

namespace ignimbrite {

    Mesh::Mesh(uint32 attrAlignment) : alignment(attrAlignment) { }

    const uint8 *Mesh::getVertexData() const {
        return vertexData.data();
    }

    const uint32 *Mesh::getIndexData() const {
        return indexData.data();
    }

    uint32 Mesh::getVertexCount() const {
        return vertexCount;
    }

    uint32 Mesh::getIndexCount() const {
        return indexData.size();
    }

    void Mesh::init(uint32 vertStride, uint32 vertCount, uint32 indexCount) {
        stride = vertStride;
        vertexCount = vertCount;
        vertexData.resize(vertStride * vertCount);
        indexData.resize(indexCount);
    }

    void Mesh::setVertex(uint32 i, const uint8 *data) {
        if (i >= vertexCount) {
            throw std::runtime_error("Setting vertex with i that is not in [0..vertexCount-1]");
        }

        memcpy(&vertexData[i * stride], data, stride);
    }

    void Mesh::addAttribute(const Mesh::VertexAttribute &attr) {
        attributes.push_back(attr);
    }

    void Mesh::setIndex(uint32 i, uint32 value) {
        if (i >= indexData.size()) {
            throw std::runtime_error("Setting index with i that is not in [0..indexCount-1]");
        }

        indexData[i] = value;
    }
}