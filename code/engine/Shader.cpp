/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                        */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#include <Shader.h>
#include <ShaderReflection.h>

namespace ignimbrite {

    Shader::Shader(RefCounted<IRenderDevice> device)
        : mDevice(std::move(device)) {

    }

    Shader::~Shader() {
        releaseHandle();
        releaseLayout();
    }

    void Shader::fromSources(ignimbrite::ShaderLanguage language, const std::vector<ignimbrite::uint8> &vertex,
                             const std::vector<ignimbrite::uint8> &fragment) {
        if (mHandle.isNotNull()) return;

        if (vertex.empty()) {
            throw std::runtime_error("Vertex shader source is empty");
        }
        if (fragment.empty()) {
            throw std::runtime_error("Fragment shader source is empty");
        }

        mProgramDesc.language = language;
        mProgramDesc.shaders.resize(2);
        mProgramDesc.shaders[0].type = ShaderType::Vertex;
        mProgramDesc.shaders[0].source = vertex;
        mProgramDesc.shaders[1].type = ShaderType::Fragment;
        mProgramDesc.shaders[1].source = fragment;

        mHandle = mDevice->createShaderProgram(mProgramDesc);

        if (mHandle.isNull()) {
            // todo: do something
        }
    }

    void Shader::reflectData() {
        ShaderReflection reflection(*this);
        reflection.reflect();
    }

    void Shader::generateUniformLayout() {
        if (mLayout.isNotNull()) return;

        IRenderDevice::UniformLayoutDesc uniformLayoutDesc{};

        for (const auto& pair: mVariables) {
            const auto& variable = pair.second;
            if (variable.type == DataType::Sampler2D || variable.type == DataType::SamplerCubemap) {
                IRenderDevice::UniformLayoutTextureDesc textureDesc{};
                textureDesc.binding = variable.binding;
                textureDesc.flags = variable.stageFlags;
                uniformLayoutDesc.textures.push_back(textureDesc);
            }
        }

        for (const auto& pair: mBuffers) {
            const auto& buffer = pair.second;
            IRenderDevice::UniformLayoutBufferDesc bufferDesc{};
            bufferDesc.binding = buffer.binding;
            bufferDesc.flags = buffer.stageFlags;
            uniformLayoutDesc.buffers.push_back(bufferDesc);
        }

        mLayout = mDevice->createUniformLayout(uniformLayoutDesc);

        if (mLayout.isNull()) {
            // todo: do something
        }
    }

    void Shader::releaseHandle() {
        if (mHandle.isNotNull()) {
            mDevice->destroyShaderProgram(mHandle);
            mHandle = ID<IRenderDevice::ShaderProgram>();
        }
    }

    void Shader::releaseLayout() {
        if (mLayout.isNotNull()) {
            mDevice->destroyUniformLayout(mLayout);
            mLayout = ID<IRenderDevice::UniformLayout>();
        }
    }

    ShaderLanguage Shader::getLanguage() const {
        return mProgramDesc.language;
    }

    const ID<IRenderDevice::ShaderProgram> &Shader::getHandle() const {
        return mHandle;
    }

    const ID<ignimbrite::IRenderDevice::UniformLayout> & Shader::getLayout() const {
        return mLayout;
    }

    const std::vector<IRenderDevice::ShaderDesc>& Shader::getShaders() const {
        return mProgramDesc.shaders;
    }

    const Shader::ParameterInfo& Shader::getParameterInfo(const String &name) const {
        try {
            return mVariables.at(name);
        } catch (const std::exception &e) {
            throw std::runtime_error("Can't get parameter in a shader with name:" + name);
        }
    }

    const Shader::UniformBufferInfo& Shader::getBufferInfo(const String &name) const {
        try {
            return mBuffers.at(name);
        } catch (const std::exception &e) {
            throw std::runtime_error("Can't get buffer in a shader with name: " + name);
        }
    }

    const std::unordered_map<String, Shader::UniformBufferInfo>& Shader::getBuffersInfo() const {
        return mBuffers;
    }

    const std::unordered_map<String, Shader::ParameterInfo> &Shader::getParametersInfo() const {
        return mVariables;
    }
}