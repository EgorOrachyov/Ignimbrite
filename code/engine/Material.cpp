/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                        */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#include <Material.h>

namespace ignimbrite {

    Material::SharedData::~SharedData() {
        release();
    }

    void Material::SharedData::release() {
        if (pipeline.isNotNull()) {
            device->destroyGraphicsPipeline(pipeline);
            pipeline = ID<IRenderDevice::GraphicsPipeline>();
        }
        if (uniformLayout.isNotNull()) {
            device->destroyUniformLayout(uniformLayout);
            uniformLayout = ID<IRenderDevice::UniformLayout>();
        }
        if (vertexLayout.isNotNull()) {
            device->destroyVertexLayout(vertexLayout);
            vertexLayout = ID<IRenderDevice::VertexLayout>();
        }
        shader = nullptr;
    }

    Material::Material(RefCounted<IRenderDevice> device) {
        mData = std::make_shared<SharedData>();
        mData->device = std::move(device);
    }

    Material::~Material() {

    }

    void Material::setInt(const String &name, int32 value) {
        const auto& info = mData->shader->getParameterInfo(name);
        auto& uniformBlock = mUniformBuffers.at(info.binding);
        uniformBlock.updateDataOnCPU(sizeof(value), info.offset, (uint8*)&value);
        mUniformBuffersWereModified = true;
    }

    void Material::setFloat(const String &name, float32 value) {
        const auto& info = mData->shader->getParameterInfo(name);
        auto& uniformBlock = mUniformBuffers.at(info.binding);
        uniformBlock.updateDataOnCPU(sizeof(value), info.offset, (uint8*)&value);
        mUniformBuffersWereModified = true;
    }

    void Material::setVec2(const String &name, const Vec2f &vec) {
        const auto& info = mData->shader->getParameterInfo(name);
        auto& uniformBlock = mUniformBuffers.at(info.binding);
        uniformBlock.updateDataOnCPU(sizeof(vec), info.offset, (uint8*)&vec);
        mUniformBuffersWereModified = true;
    }

    void Material::setVec3(const String &name, const Vec3f &vec) {
        const auto& info = mData->shader->getParameterInfo(name);
        auto& uniformBlock = mUniformBuffers.at(info.binding);
        uniformBlock.updateDataOnCPU(sizeof(vec), info.offset, (uint8*)&vec);
        mUniformBuffersWereModified = true;
    }

    void Material::setVec4(const String &name, const Vec4f &vec) {
        const auto& info = mData->shader->getParameterInfo(name);
        auto& uniformBlock = mUniformBuffers.at(info.binding);
        uniformBlock.updateDataOnCPU(sizeof(vec), info.offset, (uint8*)&vec);
        mUniformBuffersWereModified = true;
    }

    void Material::setTexture2D(const String &name, RefCounted<Texture> texture) {
        const auto& info = mData->shader->getParameterInfo(name);
        mTextures[info.binding] = std::move(texture);
        mUniformTexturesWereModified = true;
    }

    void Material::updateUniformData() {
        // Firstly, update uniform buffers on GPU if needed
        if (mUniformBuffersWereModified) {
            for (auto& buffer: mUniformBuffers) {
                buffer.second.updateDataOnGPU();
            }
        }
        // If textures were modified, therefore we need to recreate uniform set
        if (mUniformTexturesWereModified) {

        }

        mUniformBuffersWereModified = false;
        mUniformTexturesWereModified = false;
    }

}