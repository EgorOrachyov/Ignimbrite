/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                       */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                */
/**********************************************************************************/


#include "MeshLoader.h"
#include <tiny_obj_loader.h>

ignimbrite::MeshLoader::MeshLoader(const String &filePath) : filePath(filePath) {}

void ignimbrite::MeshLoader::importMesh(ignimbrite::Mesh &outMesh) {

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    bool loaded = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filePath.data());

    if (!loaded) {
        throw std::runtime_error("Can't load .obj mesh at: " + filePath);
    }

    struct ObjVertex {
        float Position[4];
        float Color[4];
        float Normal[3];
        float UV[2];
    };

    const uint32 stride = sizeof(ObjVertex);

    uint32 indexCount = 0;
    for (const auto &shape : shapes) {
        indexCount += shape.mesh.indices.size();
    }

    outMesh.init(stride, attrib.vertices.size(), indexCount);

    // position, color, normal, uv
    outMesh.addAttribute({Mesh::AttributeBaseType::Float, 4});
    outMesh.addAttribute({Mesh::AttributeBaseType::Float, 4});
    outMesh.addAttribute({Mesh::AttributeBaseType::Float, 3});
    outMesh.addAttribute({Mesh::AttributeBaseType::Float, 2});

    bool useColors = !attrib.colors.empty();
    bool useNormals = !attrib.normals.empty();
    bool useTexCoord = !attrib.texcoords.empty();

    uint32 i = 0;
    for (const auto &shape : shapes) {
        for (const auto &index : shape.mesh.indices) {
            ObjVertex vertex = {};

            vertex.Position[0] = attrib.vertices[3 * index.vertex_index + 0];
            vertex.Position[1] = attrib.vertices[3 * index.vertex_index + 1];
            vertex.Position[2] = attrib.vertices[3 * index.vertex_index + 2];
            vertex.Position[3] = 1.0f;

            if (useNormals) {
                vertex.Normal[0] = attrib.normals[3 * index.normal_index + 0];
                vertex.Normal[1] = attrib.normals[3 * index.normal_index + 1];
                vertex.Normal[2] = attrib.normals[3 * index.normal_index + 2];
            } else {
                vertex.Normal[0] = 0.0f;
                vertex.Normal[1] = 1.0f;
                vertex.Normal[2] = 0.0f;
            }

            if (useTexCoord) {
                vertex.UV[0] = attrib.texcoords[2 * index.texcoord_index + 0];
                vertex.UV[1] = 1.0f - attrib.texcoords[2 * index.texcoord_index + 1];
            }

            if (useColors) {
                vertex.Color[0] = attrib.colors[3 * index.vertex_index + 0];
                vertex.Color[1] = attrib.colors[3 * index.vertex_index + 1];
                vertex.Color[2] = attrib.colors[3 * index.vertex_index + 2];
                vertex.Color[3] = attrib.colors[3 * index.vertex_index + 3];
            } else {
                vertex.Color[0] = 1.0f;
                vertex.Color[1] = 1.0f;
                vertex.Color[2] = 1.0f;
                vertex.Color[3] = 1.0f;
            }

            outMesh.setVertex(index.vertex_index, (uint8*)&vertex);
            outMesh.setIndex(i, index.vertex_index);

            i++;
        }
    }
}
