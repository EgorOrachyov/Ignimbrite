/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                        */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                 */
/**********************************************************************************/


#include "MeshLoader.h"
#include <tiny_obj_loader.h>

namespace ignimbrite {

    MeshLoader::MeshLoader(String filePath)
        : mFilePath(std::move(filePath)) {

    }

    RefCounted<Mesh> MeshLoader::importMesh(Mesh::VertexFormat preferredFormat) {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        bool loaded = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, mFilePath.data());

        if (!loaded) {
            throw std::runtime_error("Failed to load mesh file: " + mFilePath);
        }

        struct ObjVertex {
            float32 Position[3];
            float32 Normal[3];
            float32 UV[2];
        };

        uint32 vertexCount = (uint32) attrib.vertices.size();
        uint32 indexCount = 0;
        for (const auto &shape : shapes) {
            indexCount += shape.mesh.indices.size();
        }

        RefCounted<Mesh> mesh = std::make_shared<Mesh>(preferredFormat, vertexCount, indexCount);

        bool hasNormals = !attrib.normals.empty();
        bool hasTextureCoords = !attrib.texcoords.empty();

        uint32 currentIndex = 0;
        uint32 formatMask = (uint32) preferredFormat;

        for (const auto &shape : shapes) {
            for (const auto &index : shape.mesh.indices) {

                ObjVertex vertex = {};


                // Pos3f
                if (formatMask & Mesh::BasicAttributes::Pos3f) {
                        vertex.Position[0] = attrib.vertices[3 * index.vertex_index + 0];
                        vertex.Position[1] = attrib.vertices[3 * index.vertex_index + 1];
                        vertex.Position[2] = attrib.vertices[3 * index.vertex_index + 2];
                }

                // Pos3f | Norm3f
                if (formatMask & Mesh::BasicAttributes::Norm3f) {
                    if (hasNormals) {
                        vertex.Normal[0] = attrib.normals[3 * index.normal_index + 0];
                        vertex.Normal[1] = attrib.normals[3 * index.normal_index + 1];
                        vertex.Normal[2] = attrib.normals[3 * index.normal_index + 2];
                    } else {
                        vertex.Normal[0] = 0.0f;
                        vertex.Normal[1] = 1.0f;
                        vertex.Normal[2] = 0.0f;
                    }
                }

                // Pos3f | Norm3f | TexCoords2f
                if (formatMask & Mesh::BasicAttributes::TexCoords2f) {
                    if (hasTextureCoords) {
                        vertex.UV[0] = attrib.texcoords[2 * index.texcoord_index + 0];
                        vertex.UV[1] = 1.0f - attrib.texcoords[2 * index.texcoord_index + 1];
                    }
                    else {
                        vertex.UV[0] = 0.0f;
                        vertex.UV[1] = 0.0f;
                    }
                }

                mesh->updateVertexData(index.vertex_index, 1, (uint8*)&vertex);
                mesh->updateIndexData(currentIndex, 1, (uint32*)&index.vertex_index);

                currentIndex++;
            }
        }

        return mesh;
    }

}

