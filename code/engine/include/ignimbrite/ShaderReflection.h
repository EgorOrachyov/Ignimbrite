/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov, Sultim Tsyrendashiev                  */
/**********************************************************************************/

#ifndef IGNIMBRITE_SHADERREFLECTION_H
#define IGNIMBRITE_SHADERREFLECTION_H

#include <ignimbrite/Shader.h>

namespace ignimbrite {
    class ShaderReflection {
    public:
        static void reflect(Shader& shader);
    };
}

#endif //IGNIMBRITE_SHADERREFLECTION_H