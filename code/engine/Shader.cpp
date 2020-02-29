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
    }

    void Shader::fromSources(ignimbrite::ShaderLanguage language, const std::vector<ignimbrite::uint8> &vertex,
                             const std::vector<ignimbrite::uint8> &fragment) {
        if (mHandle.isNotNull()) return;

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

    void Shader::releaseHandle() {
        if (mHandle.isNotNull()) {
            mDevice->destroyShaderProgram(mHandle);
            mHandle = ID<IRenderDevice::ShaderProgram>();
        }
    }

    ShaderLanguage Shader::getLanguage() const {
        return mProgramDesc.language;
    }

    const ID<IRenderDevice::ShaderProgram> &Shader::getHandle() const {
        return mHandle;
    }

    const std::vector<IRenderDevice::ShaderDesc>& Shader::getShaders() const {
        return mProgramDesc.shaders;
    }

    const Shader::ParameterInfo& Shader::getParameterInfo(const String &name) const {
        return mVariables.at(name);
    }

    const Shader::UniformBufferInfo& Shader::getBufferInfo(const String &name) const {
        return mBuffers.at(name);
    }


}