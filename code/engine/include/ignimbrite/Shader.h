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
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>

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

        struct AttributeInfo {
            std::string name;
            uint32      binding;
            DataType    type;
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

        /** Actual program handle */
        ID<RenderDevice::ShaderProgram> mHandle;

        std::vector<AttributeInfo> mVertexShaderInputs;
        std::vector<AttributeInfo> mFragmentShaderOutputs;

        /** Program variables (samplers, and uniform blocks variables) */
        std::unordered_map<std::string, ParameterInfo> mVariables;

        /** Program uniform blocks info */
        std::unordered_map<std::string, UniformBufferInfo> mBuffers;

        /** Render device, which is used for that shader creation */
        std::shared_ptr<RenderDevice> mRenderDevice;

    };

}

#endif //IGNIMBRITE_SHADER_H