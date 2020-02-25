/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov, Sultim Tsyrendashiev                  */
/**********************************************************************************/

#ifndef IGNIMBRITE_SHADER_H
#define IGNIMBRITE_SHADER_H

#include <CacheItem.h>
#include <RenderDevice.h>
#include <RenderDeviceDefinitions.h>
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
            uint32      location;
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
            uint32                   binding;
            uint32                   size;
            ShaderStageFlags         stageFlags;
            std::vector<std::string> members;
        };

    public:
        explicit Shader(std::shared_ptr<RenderDevice> device);
        ~Shader() override;

        void fromSources(ShaderLanguage language, const std::vector<uint8> &vertex, const std::vector<uint8> &fragment);
        void reflectData();
        void releaseHandle();

        ShaderLanguage getLanguage() const;
        const ID<RenderDevice::ShaderProgram> &getHandle() const;
        const std::vector<RenderDevice::ShaderDesc> &getShaders() const;
        const ParameterInfo& getParameterInfo(const std::string &name) const;
        const UniformBufferInfo& getBufferInfo(const std::string &name) const;

    private:
        friend class ShaderReflection;
        // TODO
        std::vector<AttributeInfo> mVertexShaderInputs;
        std::vector<AttributeInfo> mFragmentShaderOutputs;
        /** Program variables (samplers, and uniform blocks variables) */
        std::unordered_map<std::string, ParameterInfo> mVariables;
        /** Program uniform blocks info */
        std::unordered_map<std::string, UniformBufferInfo> mBuffers;
        /** Program descriptor with this shader's modules*/
        RenderDevice::ProgramDesc mProgramDesc;
        /** Actual program handle */
        ID<RenderDevice::ShaderProgram> mHandle;
        /** Render device, which is used for that shader creation */
        std::shared_ptr<RenderDevice> mDevice;

    };

}

#endif //IGNIMBRITE_SHADER_H