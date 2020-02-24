/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov, Sultim Tsyrendashiev                  */
/**********************************************************************************/

#include <ignimbrite/Shader.h>
#include <ignimbrite/ShaderReflection.h>

using namespace ignimbrite;

void Shader::addModule(ShaderType moduleType, const std::vector<uint8> &moduleSourceCode)
{
#ifndef NDEBUG
    for (const auto &m : mProgramDesc.shaders)
    {
        if (m.type == moduleType)
        {
            throw std::runtime_error("Attempt to add module with type which is already present");
        }
    }
#endif

    mProgramDesc.shaders.push_back({});
    mProgramDesc.shaders.back().type = moduleType;
    mProgramDesc.shaders.back().source = moduleSourceCode;
}

Shader::Shader(const std::shared_ptr<RenderDevice> &renderDevice, ignimbrite::ShaderLanguage language) {
    mRenderDevice = renderDevice;
    mProgramDesc.language = language;
}

Shader::Shader(const std::shared_ptr<RenderDevice> &renderDevice,
               ignimbrite::ShaderLanguage language,
               const std::vector<uint8> &vertSourceCode,
               const std::vector<uint8> &fragSourceCode) : Shader(renderDevice, language) {

    addModule(ShaderType::Vertex, vertSourceCode);
    addModule(ShaderType::Fragment, fragSourceCode);
}

void Shader::create() {
    // get shader info
    ShaderReflection::reflect(*this);

    // create shader in render device
    mHandle = mRenderDevice->createShaderProgram(mProgramDesc);
}

ID<RenderDevice::ShaderProgram> Shader::getHandle() const
{
    return mHandle;
}

Shader::~Shader() {
    mRenderDevice->destroyShaderProgram(mHandle);
}
