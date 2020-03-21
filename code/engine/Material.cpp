/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                        */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#include <Material.h>
#include <PipelineContext.h>

namespace ignimbrite {

    Material::Material(RefCounted<IRenderDevice> device)
        : mDevice(std::move(device)) {

    }

    Material::~Material() {
        releaseMaterial();
    }

    void Material::setGraphicsPipeline(ignimbrite::RefCounted<ignimbrite::GraphicsPipeline> pipeline) {
        mPipeline = std::move(pipeline);
    }

    void Material::createMaterial() {
        auto& shader = mPipeline->getShader();

        for (const auto& p: shader->getBuffersInfo()) {
            auto binding = p.second.binding;
            auto size = p.second.size;

            mUniformBuffers.emplace(binding, mDevice);
            mUniformBuffers.at(binding).createBuffer(size);
        }
    }

    void Material::releaseMaterial() {
        if (mUniformSet.isNotNull()) {
            mDevice->destroyUniformSet(mUniformSet);
            mUniformSet = ID<IRenderDevice::UniformSet>();
        }
        mTextures.clear();
        mUniformBuffers.clear();
    }

    void Material::setInt(const String &name, int32 value) {
        const auto& info = mPipeline->getShader()->getParameterInfo(name);
        auto& uniformBlock = mUniformBuffers.at(info.binding);
        uniformBlock.updateDataOnCPU(sizeof(value), info.offset, (uint8*)&value);
        mUniformBuffersWereModified = true;
    }

    void Material::setFloat(const String &name, float32 value) {
        const auto& info = mPipeline->getShader()->getParameterInfo(name);
        auto& uniformBlock = mUniformBuffers.at(info.binding);
        uniformBlock.updateDataOnCPU(sizeof(value), info.offset, (uint8*)&value);
        mUniformBuffersWereModified = true;
    }

    void Material::setVec2(const String &name, const Vec2f &vec) {
        const auto& info = mPipeline->getShader()->getParameterInfo(name);
        auto& uniformBlock = mUniformBuffers.at(info.binding);
        uniformBlock.updateDataOnCPU(sizeof(vec), info.offset, (uint8*)&vec);
        mUniformBuffersWereModified = true;
    }

    void Material::setVec3(const String &name, const Vec3f &vec) {
        const auto& info = mPipeline->getShader()->getParameterInfo(name);
        auto& uniformBlock = mUniformBuffers.at(info.binding);
        uniformBlock.updateDataOnCPU(sizeof(vec), info.offset, (uint8*)&vec);
        mUniformBuffersWereModified = true;
    }

    void Material::setVec4(const String &name, const Vec4f &vec) {
        const auto& info = mPipeline->getShader()->getParameterInfo(name);
        auto& uniformBlock = mUniformBuffers.at(info.binding);
        uniformBlock.updateDataOnCPU(sizeof(vec), info.offset, (uint8*)&vec);
        mUniformBuffersWereModified = true;
    }

    void Material::setMat4(const ignimbrite::String &name, const ignimbrite::Mat4f &mat) {
        const auto& info = mPipeline->getShader()->getParameterInfo(name);
        auto& uniformBlock = mUniformBuffers.at(info.binding);
        uniformBlock.updateDataOnCPU(sizeof(mat), info.offset, (uint8*)&mat);
        mUniformBuffersWereModified = true;
    }

    void Material::setTexture2D(const String &name, RefCounted<Texture> texture) {
        const auto& info = mPipeline->getShader()->getParameterInfo(name);
        mTextures[info.binding] = std::move(texture);
        mUniformTexturesWereModified = true;
    }

    void Material::bindGraphicsPipeline() {
        auto pipeline = mPipeline->getHandle();
        if (!PipelineContext::isPipelineCached(pipeline)) {
            mDevice->drawListBindPipeline(pipeline);
            PipelineContext::cachePipelineBinding(pipeline);
        }
    }

    void Material::bindUniformData() {
        mDevice->drawListBindUniformSet(mUniformSet);
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
            IRenderDevice::UniformSetDesc setDesc;
            setDesc.textures.reserve(mTextures.size());
            setDesc.buffers.reserve(mUniformBuffers.size());

            for (const auto& p: mTextures) {
                IRenderDevice::UniformTextureDesc textureDesc;
                textureDesc.binding = p.first;
                textureDesc.texture = p.second->getHandle();
                textureDesc.sampler = p.second->getSampler()->getHandle();

                setDesc.textures.push_back(textureDesc);
            }

            for (const auto& p: mUniformBuffers) {
                IRenderDevice::UniformBufferDesc bufferDesc;
                bufferDesc.binding = p.first;
                bufferDesc.offset = 0;
                bufferDesc.range = p.second.getBufferSize();
                bufferDesc.buffer = p.second.getHandle();

                setDesc.buffers.push_back(bufferDesc);
            }

            if (mUniformSet.isNotNull()) {
                mDevice->destroyUniformSet(mUniformSet);
            }

            mUniformSet = mDevice->createUniformSet(setDesc, mPipeline->getShader()->getLayout());

            if (mUniformSet.isNull()) {
                throw std::runtime_error("Failed to create uniform set for material");
            }
        }

        mUniformBuffersWereModified = false;
        mUniformTexturesWereModified = false;
    }

    RefCounted<Material> Material::clone() const {
        RefCounted<Material> mat = std::make_shared<Material>(mDevice);
        mat->setGraphicsPipeline(mPipeline);
        mat->createMaterial();

        for (const auto& p: mTextures) {
            mat->mTextures.emplace(p.first, p.second);
        }

        for (const auto& p: mUniformBuffers) {
            mat->mUniformBuffers.at(p.first).updateDataOnCPU(p.second.getBufferSize(), 0, p.second.getData().data());
        }

        mat->mUniformBuffersWereModified = true;
        mat->mUniformTexturesWereModified = true;
        mat->updateUniformData();

        return mat;
    }

    const RefCounted<GraphicsPipeline>& Material::getGraphicsPipeline() const {
        return mPipeline;
    }

}