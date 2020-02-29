/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                        */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                 */
/**********************************************************************************/


#ifndef IGNIMBRITE_MESHLOADER_H
#define IGNIMBRITE_MESHLOADER_H

#include <Mesh.h>

namespace ignimbrite {

    class MeshLoader {
    public:
        explicit MeshLoader(String filePath);
        RefCounted<Mesh> importMesh(Mesh::VertexFormat preferredFormat);
    private:
        String mFilePath;
    };

}

#endif //IGNIMBRITE_MESHLOADER_H
