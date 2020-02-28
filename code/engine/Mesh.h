/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                       */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                */
/**********************************************************************************/


#ifndef IGNIMBRITE_MESH_H
#define IGNIMBRITE_MESH_H

#include <AABB.h>
#include <IncludeStd.h>

namespace ignimbrite {

    /**
     * @class Mesh class that holds list of its attributes,
     * and packed vertex data, i.e. array of vertices
     */
    class Mesh {
        friend class MeshLoader;

    public:

        // Base type for attribute
        enum class AttributeBaseType {
            Float,
            Int,
            UInt
        };

        // Size of base type in bytes
        static const uint32             AttributeBaseTypeSize = 4;

        // Mesh uses its own vertex attribute description
        // as it shouldn't be tied to render device
        struct VertexAttribute {
            AttributeBaseType           baseType;
            uint32                      dim;
        };

    public:

        /**
         * @param attrAlignment alignment for attributes in each vertex.
         */
        Mesh(uint32 attrAlignment = AttributeBaseTypeSize);

        /**
         * Get vertex data prepared for rendering
         */
        const uint8                     *getVertexData() const;
        uint32                          getVertexCount() const;
        /**
         * Get index data prepared for rendering
         */
        const uint32                    *getIndexData() const;
        uint32                          getIndexCount() const;

    private:
        void init(uint32 vertStride, uint32 vertCount, uint32 indexCount);
        void addAttribute(const VertexAttribute &attr);
        void setVertex(uint32 i, const uint8 *data);
        void setIndex(uint32 i, uint32 value);

    private:
        // alignment in bytes for attributes
        uint32                          alignment;

        // attributes in order as they appear in 'vertexData'
        std::vector<VertexAttribute>    attributes;

        // size of vertex in bytes
        uint32                          stride;

        // raw vertex data
        std::vector<uint8>              vertexData;
        uint32                          vertexCount;

        std::vector<uint32>             indexData;
    };

}

#endif //IGNIMBRITE_MESH_H
