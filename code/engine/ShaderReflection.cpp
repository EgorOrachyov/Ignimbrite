/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019, 2020  Egor Orachyov                                        */
/* Copyright (c) 2019, 2020  Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#include <ShaderReflection.h>
#include <spirv_cross.hpp>
#include <vector>
#include <string>

namespace ignimbrite {

    Shader::DataType getDimType(Shader::DataType baseType, uint32 rows, uint32 columns) {
        if (rows == 0 || columns == 0 || rows > 4 || columns > 4) {
            throw std::runtime_error("Wrong row or column count");
        }

        if (rows == 1 && columns == 1) {
            return baseType;
        }

        switch (baseType) {
            case Shader::DataType::Bool:
                if (columns == 1) {
                    switch (rows) {
                        case 2:
                            return Shader::DataType::Bool2;
                        case 3:
                            return Shader::DataType::Bool3;
                        case 4:
                            return Shader::DataType::Bool4;
                        default:;
                    }
                } else {
                    throw std::runtime_error("Matrices of bool are not supported");
                }
                break;
            case Shader::DataType::Int:
                if (columns == 1) {
                    switch (rows) {
                        case 2:
                            return Shader::DataType::Int2;
                        case 3:
                            return Shader::DataType::Int3;
                        case 4:
                            return Shader::DataType::Int4;
                        default:;
                    }
                } else {
                    throw std::runtime_error("Matrices of int are not supported");
                }
                break;
            case Shader::DataType::UInt:
                if (columns == 1) {
                    switch (rows) {
                        case 2:
                            return Shader::DataType::UInt2;
                        case 3:
                            return Shader::DataType::UInt3;
                        case 4:
                            return Shader::DataType::UInt4;
                        default:;
                    }
                } else {
                    throw std::runtime_error("Matrices of uint are not supported");
                }
                break;
            case Shader::DataType::Float:
                if (columns == 1) {
                    switch (rows) {
                        case 2:
                            return Shader::DataType::Float2;
                        case 3:
                            return Shader::DataType::Float3;
                        case 4:
                            return Shader::DataType::Float4;
                        default:;
                    }
                } else {
                    if (columns == rows) {
                        switch (columns) {
                            case 2:
                                return Shader::DataType::Mat2;
                            case 3:
                                return Shader::DataType::Mat3;
                            case 4:
                                return Shader::DataType::Mat4;
                            default:;
                        }
                    } else {
                        throw std::runtime_error("Matrices must be square");
                    }
                }
                break;
            case Shader::DataType::Bool2:
            case Shader::DataType::Bool3:
            case Shader::DataType::Bool4:
            case Shader::DataType::Int2:
            case Shader::DataType::Int3:
            case Shader::DataType::Int4:
            case Shader::DataType::UInt2:
            case Shader::DataType::UInt3:
            case Shader::DataType::UInt4:
            case Shader::DataType::Float2:
            case Shader::DataType::Float3:
            case Shader::DataType::Float4:
            case Shader::DataType::Mat2:
            case Shader::DataType::Mat3:
            case Shader::DataType::Mat4:
                throw std::runtime_error("Base type must be bool, int, uint or float");
            case Shader::DataType::Sampler2D:
            case Shader::DataType::SamplerCubemap:
                throw std::runtime_error("Base type must be primitive");
        }

        throw std::runtime_error("Unexpected error in getDimType");
    }

    Shader::DataType getSamplerType(const spirv_cross::SPIRType &type, const String &samplerName) {
        using namespace spirv_cross;

        if (type.basetype == SPIRType::SampledImage) {

            switch (type.image.dim) {
                case spv::Dim2D:
                    return Shader::DataType::Sampler2D;

                case spv::DimCube:
                    return Shader::DataType::SamplerCubemap;

                default:
                    throw std::runtime_error(String("Expected sampled image 2D or Cube: ") + samplerName);
            }

        } else {
            throw std::runtime_error(String("Expected sampled image type: ") + samplerName);
        }
    }

    Shader::DataType getType(const spirv_cross::Compiler &comp,
                             const spirv_cross::SPIRType &type, const String &memberName) {
        using namespace spirv_cross;

        Shader::DataType baseType;

        switch (type.basetype) {
            case SPIRType::Boolean:
                baseType = Shader::DataType::Bool;
                break;
            case SPIRType::Int:
                baseType = Shader::DataType::Int;
                break;
            case SPIRType::UInt:
                baseType = Shader::DataType::UInt;
                break;
            case SPIRType::Float:
                baseType = Shader::DataType::Float;
                break;
                /*case SPIRType::Int64:
                    break;
                case SPIRType::UInt64:
                    break;
                case SPIRType::Double:
                    break;*/
            case SPIRType::Struct:
                throw std::runtime_error("Structs in uniform/inputs are not supported: " + memberName);
            default:
                throw std::runtime_error(
                        "Base type of this var is not supported: " + comp.get_name(type.self) + " " + memberName);
        }

        return getDimType(baseType, type.vecsize, type.columns);

        /*} else {
            throw std::runtime_error("Base type with size " +
            std::to_string(type.vecsize) + "x" + std::to_string(type.columns) + " is not supported ");
        }*/
    }

    void parseSpirvStruct(std::unordered_map<String, Shader::ParameterInfo> &params,
                          std::vector<String> &membersList,
                          ShaderStageFlags flags, const spirv_cross::Compiler &comp,
                          const spirv_cross::SPIRType &spirType, const String &baseName,
                          uint32 baseBinding, ignimbrite::uint32 baseOffset) {

        using namespace spirv_cross;


        uint32 memberCount = spirType.member_types.size();

        for (uint32 i = 0; i < memberCount; i++) {

            const SPIRType &memberType = comp.get_type(spirType.member_types[i]);
            String memberName = comp.get_member_name(spirType.self, i);

            Shader::ParameterInfo info = {};
            info.stageFlags = flags;
            info.blockSize = 0;

            membersList.push_back(memberName);

            // members have same binding, but different offsets
            info.binding = baseBinding;
            // offset in uniform buffer
            info.offset = baseOffset + comp.type_struct_member_offset(spirType, i);

            info.type = getType(comp, memberType, memberName);

            String paramName = baseName;
            paramName += '.';
            paramName += memberName;

            params[paramName] = info;
        }
    }

    void getSpirvParams(
            const spirv_cross::Compiler &comp, const spirv_cross::ShaderResources &resources,
            std::unordered_map<String, Shader::ParameterInfo> &params,
            std::unordered_map<String, Shader::UniformBufferInfo> &uniforms,
            ShaderStageFlags stageFlags) {
        using namespace spirv_cross;

        for (const auto &resource : resources.uniform_buffers) {

            if (uniforms.count(resource.name) > 0) {
                // if resource already exists with the same name, check its binding
                if (uniforms[resource.name].binding != comp.get_decoration(resource.id, spv::DecorationBinding)) {
                    throw std::runtime_error("If resources have same name they must have same bindings too");
                }

                // if it's the same resource but in another stage, update flags
                uniforms[resource.name].stageFlags |= stageFlags;
                continue;
            }

            uniforms[resource.name] = {};
            Shader::UniformBufferInfo &uniformInfo = uniforms[resource.name];

            uniformInfo.stageFlags = stageFlags;
            uniformInfo.binding = comp.get_decoration(resource.id, spv::DecorationBinding);

            // get size of this uniform buffer
            const SPIRType &spirType = comp.get_type(resource.base_type_id);
            uniformInfo.size = comp.get_declared_struct_size(spirType);

            // parse uniform buffer members
            parseSpirvStruct(params, uniformInfo.members, stageFlags, comp, spirType, resource.name,
                             uniformInfo.binding, 0);
        }

        for (const auto &resource : resources.sampled_images) {

            if (params.count(resource.name) > 0) {
                // if resource already exists with the same name, check its binding
                if (params[resource.name].binding != comp.get_decoration(resource.id, spv::DecorationBinding)) {
                    throw std::runtime_error("If resources have same name they must have same bindings too");
                }

                // if it's the same resource but in another stage, update flags
                params[resource.name].stageFlags |= stageFlags;
                continue;
            }

            params[resource.name] = {};
            Shader::ParameterInfo &info = params[resource.name];

            info.stageFlags = stageFlags;
            info.binding = comp.get_decoration(resource.id, spv::DecorationBinding);
            // sampled images don't have offset
            info.offset = 0;
            // sampled images are not buffers
            info.blockSize = 0;

            info.type = getSamplerType(comp.get_type(resource.base_type_id), resource.name);
        }
    }

    void getSpirvModuleInputs(
            const spirv_cross::Compiler &comp, const spirv_cross::ShaderResources &resources,
            std::vector<Shader::AttributeInfo> &moduleInputs) {

        for (const auto &resource : resources.stage_inputs) {

            Shader::AttributeInfo info = {};
            info.name = resource.name;
            info.location = comp.get_decoration(resource.id, spv::DecorationLocation);
            info.type = getType(comp, comp.get_type(resource.type_id), resource.name);

            moduleInputs.push_back(info);
        }
    }

    void getSpirvModuleOutputs(
            const spirv_cross::Compiler &comp, const spirv_cross::ShaderResources &resources,
            std::vector<Shader::AttributeInfo> &moduleOutputs) {

        for (const auto &resource : resources.stage_outputs) {

            Shader::AttributeInfo info = {};
            info.name = resource.name;
            info.location = comp.get_decoration(resource.id, spv::DecorationLocation);
            info.type = getType(comp, comp.get_type(resource.type_id), resource.name);

            moduleOutputs.push_back(info);
        }
    }

    ShaderReflection::ShaderReflection(Shader &shader)
        : mShader(shader) {

    }

    void ShaderReflection::reflect() {
        auto& shader = mShader;

        for (const auto &desc : shader.mProgramDesc.shaders) {
            ShaderStageFlags stageFlags = 0;

            if (desc.type == ShaderType::Vertex) {
                stageFlags = (uint32) ShaderStageFlagBits::VertexBit;
            } else if (desc.type == ShaderType::Fragment) {
                stageFlags = (uint32) ShaderStageFlagBits::FragmentBit;
            } else {
                throw std::runtime_error("Unknown shader stage");
            }

            if (shader.mProgramDesc.language == ShaderLanguage::SPIRV) {

                const uint32 wordSize = sizeof(uint32);
                uint32 wordCount = desc.source.size() / wordSize;

                spirv_cross::Compiler compiler((const uint32_t *) desc.source.data(), wordCount);
                const spirv_cross::ShaderResources &resources = compiler.get_shader_resources();

                getSpirvParams(compiler, resources, shader.mVariables, shader.mBuffers, stageFlags);

                if (desc.type == ShaderType::Vertex) {
                    getSpirvModuleInputs(compiler, resources, shader.mVertexShaderInputs);
                } else if (desc.type == ShaderType::Fragment) {
                    getSpirvModuleOutputs(compiler, resources, shader.mFragmentShaderOutputs);
                }

            } else {
                throw std::runtime_error("Unsupported language");
            }
        }
    }

}
