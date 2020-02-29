/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                        */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                 */
/**********************************************************************************/


#ifndef IGNIMBRITE_MESH_H
#define IGNIMBRITE_MESH_H

#include <CacheItem.h>
#include <IncludeMath.h>

namespace ignimbrite {

    /**
     * @brief Mesh 3d geometry
     * Holds list of its attributes and packed vertex data, i.e. array of vertices.
     * Has no rendering logic, provides only container for simple geometry data.
     */
    class Mesh : public CacheItem {
    public:

        enum BasicAttributes : uint32 {
            Pos3f       = 1u << 0u,
            Norm3f      = 1u << 1u,
            TexCoords2f = 1u << 2u
        };

        /** Fixed vertex layout with predefined types of attributes */
        enum class VertexFormat : uint32 {
            P = Pos3f,                              /** P.x P.y P.z */
            PN = Pos3f | Norm3f,                    /** P.x P.y P.z | N.x N.y N.z */
            PNT = Pos3f | Norm3f | TexCoords2f,     /** P.x P.y P.z | N.x N.y N.z | T.u T.v */
        };

        Mesh(VertexFormat format, uint32 vertexCount, uint32 indexCount);
        ~Mesh() override = default;

        /**
         * Update vertex data of the mesh
         * @param offset Number of first vertices to skip before write
         * @param vertexCount Number of vertices to write
         * @param data Data to be written
         * @return True on success
         */
        bool updateVertexData(uint32 offset, uint32 vertexCount, const uint8* data);
        /**
         * Update index buffer of the mesh
         * @param offset Number of the first indices to skip before write
         * @param indexCount Number of the indices to be written
         * @param data Data to be written
         * @return True on success
         */
        bool updateIndexData(uint32 offset, uint32 indexCount, const uint32* data);

        void updateBoundingVolume();

        VertexFormat getVertexFormat() const { return mVertexFormat; }
        const AABB& getBoundingBox() const { return mBoundingBox; }
        const uint8 *getVertexData() const { return mVertexData.data(); }
        const uint32 *getIndexData() const { return mIndexData.data(); }
        uint32 getStride() const { return mStride; }
        uint32 getVertexCount() const { return mVertexCount; }
        uint32 getIndexCount() const { return mIndexData.size(); }

        static uint32 getSizeOfStride(VertexFormat format);
        static uint32 getNumberOfAttributes(VertexFormat format);

    private:
        friend class MeshLoader;
        AABB                mBoundingBox;
        VertexFormat        mVertexFormat;
        uint32              mStride;
        uint32              mVertexCount;
        std::vector<uint8>  mVertexData;
        std::vector<uint32> mIndexData;
    };

}

#endif //IGNIMBRITE_MESH_H
