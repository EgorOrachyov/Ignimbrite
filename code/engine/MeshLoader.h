/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                       */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                */
/**********************************************************************************/


#ifndef IGNIMBRITE_MESHLOADER_H
#define IGNIMBRITE_MESHLOADER_H

#include <Mesh.h>

namespace ignimbrite {

    class MeshLoader {
    public:
        MeshLoader(const String &filePath);
        void importMesh(Mesh &outMesh);

    private:
        String                  filePath;
    };

}

#endif //IGNIMBRITE_MESHLOADER_H
