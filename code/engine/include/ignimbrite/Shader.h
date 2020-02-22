/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov, Sultim Tsyrendashiev                  */
/**********************************************************************************/

#ifndef IGNIMBRITE_SHADER_H
#define IGNIMBRITE_SHADER_H

#include <ignimbrite/CacheItem.h>
#include <ignimbrite/RenderDevice.h>
#include <ignimbrite/RenderDeviceDefinitions.h>
#include <string>
#include <unordered_map>
#include <vector>

namespace ignimbrite {

    class Shader : public CacheItem {
    public:

        enum class DataType {
            Bool,
            Bool2,
            Bool3,
            Bool4,
            Int,
            Int2,
            Int3,
            Int4,
            UInt,
            UInt2,
            UInt3,
            UInt4,
            Float,
            Float2,
            Float3,
            Float4,
            Mat2,
            Mat3,
            Mat4,
            Sampler2D,
            SamplerCubemap
        };

        struct ParameterInfo {
            uint32           binding;
            uint32           offset;
            uint32           blockSize;
            DataType         type;
            ShaderStageFlags stageFlags;
        };

        struct UniformBufferInfo {
            uint32 size;
            std::vector<std::string> members;
        };

        ~Shader() override = default;

    private:
        // todo: program handle
        // todo: source code
        //
        std::unordered_map<std::string, ParameterInfo> mVariables;
        std::unordered_map<std::string, UniformBufferInfo> mBuffer;

    };

}

#endif //IGNIMBRITE_SHADER_H