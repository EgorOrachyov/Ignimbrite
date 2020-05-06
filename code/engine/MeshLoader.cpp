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
#include <IncludeMath.h>

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

        struct ImportVertex {
            float32 Position[3];
            float32 Normal[3];
            float32 UV[2];
            float32 Tangent[3];
            float32 Bitangent[3];
        };

        struct ImportTgBn {
            float32 Tangent[3];
            float32 Bitangent[3];
        };

        uint32 vertexCount = (uint32) attrib.vertices.size();
        uint32 indexCount = 0;
        for (const auto &shape : shapes) {
            indexCount += shape.mesh.indices.size();
        }

        RefCounted<Mesh> mesh = std::make_shared<Mesh>(preferredFormat, vertexCount, indexCount);

        bool hasNormals = !attrib.normals.empty();
        bool hasTextureCoords = !attrib.texcoords.empty();

        std::vector<ImportTgBn> tgbns;

        // generate tangents/bitangents if required
        if (preferredFormat == Mesh::VertexFormat::PNTTB) {
            if (!hasTextureCoords) {
                throw std::runtime_error("To generate tangents/bitangents mesh must have texture coordinates");
            }
            if (!hasNormals) {
                throw std::runtime_error("To generate tangents/bitangents mesh must have normals");
            }

            tgbns.resize(vertexCount);

            for (const auto &shape : shapes) {
                // for each triangle
                for (uint32 i = 0; i < shape.mesh.indices.size(); i += 3) {
                    Vec3f positions[3];
                    Vec2f texCoords[3];

                    for (int j = 0; j < 3; j++) {
                        positions[j] = {
                                attrib.vertices[3 * shape.mesh.indices[i + j].vertex_index + 0],
                                attrib.vertices[3 * shape.mesh.indices[i + j].vertex_index + 1],
                                attrib.vertices[3 * shape.mesh.indices[i + j].vertex_index + 2]
                        };

                        texCoords[j] = {
                                attrib.texcoords[2 * shape.mesh.indices[i + j].texcoord_index + 0],
                                1.0f - attrib.texcoords[2 * shape.mesh.indices[i + j].texcoord_index + 1]
                        };
                    }

                    Vec3f q1 = positions[1] - positions[0];
                    Vec3f q2 = positions[2] - positions[0];

                    float s1 = texCoords[1].s - texCoords[0].s;
                    float t1 = texCoords[1].t - texCoords[0].t;

                    float s2 = texCoords[2].s - texCoords[0].s;
                    float t2 = texCoords[2].t - texCoords[0].t;

                    if (s1 * t2 == s2 * t1) {
                        s1 = 0.0f; t1 = 1.0f;
                        s2 = 1.0f; t2 = 0.0f;
                    }

                    // in tangent space
                    Vec3f tg = {
                            t2 * q1.x - t1 * q2.x,
                            t2 * q1.y - t1 * q2.y,
                            t2 * q1.z - t1 * q2.z
                    };
                    Vec3f btg = {
                            -s2 * q1.x + s1 * q2.x,
                            -s2 * q1.y + s1 * q2.y,
                            -s2 * q1.z + s1 * q2.z
                    };

                    float det = s1 * t2 - s2 * t1;
                    tg *= det;
                    btg *= det;

                    // recalculate to object space
                    for (int j = 0; j < 3; j++) {
                        Vec3f normal = {
                                attrib.normals[3 * shape.mesh.indices[i + j].normal_index + 0],
                                attrib.normals[3 * shape.mesh.indices[i + j].normal_index + 1],
                                attrib.normals[3 * shape.mesh.indices[i + j].normal_index + 2]
                        };

                        Vec3f oTg = tg - (glm::dot(tg, normal)) * normal;
                        Vec3f oBtg = btg - (glm::dot(btg, normal)) * normal - (glm::dot(btg, tg)) * tg;

                        float lTg = glm::length(oTg);
                        float lBtg = glm::length(oBtg);

                        if (lTg > 0) {
                            oTg /= lTg;
                        }
                        if (lBtg > 0) {
                            oBtg /= lBtg;
                        }

                        for (int k = 0; k < 3; k++) {
                            tgbns[3 * shape.mesh.indices[i + j].vertex_index].Tangent[k] = oTg[k];
                            tgbns[3 * shape.mesh.indices[i + j].vertex_index].Bitangent[k] = oBtg[k];
                        }
                    }
                }
            }
        }

        uint32 currentIndex = 0;
        uint32 formatMask = (uint32) preferredFormat;

        for (const auto &shape : shapes) {
            for (const auto &index : shape.mesh.indices) {

                ImportVertex vertex = {};

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

                if ((formatMask & Mesh::BasicAttributes::Tangent3f) && (formatMask & Mesh::BasicAttributes::Bitangent3f)) {
                    ImportTgBn &tgbn = tgbns[index.vertex_index];
                    vertex.Tangent[0] = tgbn.Tangent[0];
                    vertex.Tangent[1] = tgbn.Tangent[1];
                    vertex.Tangent[2] = tgbn.Tangent[2];
                    vertex.Bitangent[0] = tgbn.Bitangent[0];
                    vertex.Bitangent[1] = tgbn.Bitangent[1];
                    vertex.Bitangent[2] = tgbn.Bitangent[2];
                }

                // will be copied only <stride> bytes, not whole ImportVertex structure
                mesh->updateVertexData(index.vertex_index, 1, (uint8*)&vertex);
                mesh->updateIndexData(currentIndex, 1, (uint32*)&index.vertex_index);

                currentIndex++;
            }
        }

        // Update implicitly for safety
        mesh->updateBoundingVolume();

        return mesh;
    }

}

