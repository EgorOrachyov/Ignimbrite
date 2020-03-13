/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                        */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#ifndef IGNIMBRITE_SHADER_H
#define IGNIMBRITE_SHADER_H

#include <IncludeStd.h>
#include <CacheItem.h>
#include <IRenderDevice.h>
#include <IRenderDeviceDefinitions.h>

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
            String name;
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
            uint32              binding;
            uint32              size;
            ShaderStageFlags    stageFlags;
            std::vector<String> members;
        };

        explicit Shader(RefCounted<IRenderDevice> device);
        ~Shader() override;

        void fromSources(ShaderLanguage language, const std::vector<uint8> &vertex, const std::vector<uint8> &fragment);
        void reflectData();
        void generateUniformLayout();
        void releaseHandle();
        void releaseLayout();

        ShaderLanguage getLanguage() const;
        const ID<IRenderDevice::ShaderProgram> &getHandle() const;
        const ID<IRenderDevice::UniformLayout> &getLayout() const;
        const std::vector<IRenderDevice::ShaderDesc> &getShaders() const;
        const ParameterInfo& getParameterInfo(const String &name) const;
        const UniformBufferInfo& getBufferInfo(const String &name) const;
        const std::unordered_map<String, UniformBufferInfo> &getBuffersInfo() const;

    private:
        friend class ShaderReflection;
        /** Input attributes of the vertex program (as main entry for graphics shader)*/
        std::vector<AttributeInfo> mVertexShaderInputs;
        /** Input attributes of the fragment program */
        std::vector<AttributeInfo> mFragmentShaderOutputs;
        /** Program variables (samplers, and uniform blocks variables) */
        std::unordered_map<String, ParameterInfo> mVariables;
        /** Program uniform blocks info */
        std::unordered_map<String, UniformBufferInfo> mBuffers;
        /** Program descriptor with this shader's modules*/
        IRenderDevice::ProgramDesc mProgramDesc;
        /** Actual program handle */
        ID<IRenderDevice::ShaderProgram> mHandle;
        /** Uniform layout */
        ID<IRenderDevice::UniformLayout> mLayout;
        /** Render device, which is used for that shader creation */
        RefCounted<IRenderDevice> mDevice;

    };

}

#endif //IGNIMBRITE_SHADER_H