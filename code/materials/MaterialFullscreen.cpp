/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                        */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#include <MaterialFullscreen.h>
#include <VertexLayoutFactory.h>
#include <FileUtils.h>

namespace ignimbrite {

    struct Vertex {
        float32 position[2];
        float32 texcoords[2];
    };

    RefCounted<Material> MaterialFullscreen::screeMaterialSpv(const String &vertexName, const String &fragmentName, const RefCounted<RenderTarget::Format> &format, const RefCounted<IRenderDevice> &device) {
        std::vector<uint8> vertexCode;
        std::vector<uint8> fragmentCode;

        auto& supportedLanguages = device->getSupportedShaderLanguages();
        auto supportsSpirv = std::find(supportedLanguages.begin(), supportedLanguages.end(), ShaderLanguage::SPIRV) != supportedLanguages.end();

        auto shader = std::make_shared<Shader>(device);

        if (supportsSpirv) {
            FileUtils::loadBinary(vertexName, vertexCode);
            FileUtils::loadBinary(fragmentName, fragmentCode);

            shader->fromSources(ShaderLanguage::SPIRV, vertexCode, fragmentCode);
        }
        else {
            throw std::runtime_error("No shader sources for this device type");
        }

        shader->reflectData();
        shader->generateUniformLayout();

        IRenderDevice::VertexBufferLayoutDesc vertexBufferLayoutDesc = {};
        vertexBufferLayoutDesc.stride = sizeof(Vertex);
        vertexBufferLayoutDesc.usage = VertexUsage::PerVertex;
        vertexBufferLayoutDesc.attributes.resize(2);
        vertexBufferLayoutDesc.attributes[0].format = DataFormat::R32G32_SFLOAT;
        vertexBufferLayoutDesc.attributes[0].location = 0;
        vertexBufferLayoutDesc.attributes[0].offset = offsetof(Vertex, position);
        vertexBufferLayoutDesc.attributes[1].format = DataFormat::R32G32_SFLOAT;
        vertexBufferLayoutDesc.attributes[1].location = 1;
        vertexBufferLayoutDesc.attributes[1].offset = offsetof(Vertex, texcoords);

        auto pipeline = std::make_shared<GraphicsPipeline>(device);
        pipeline->setTargetFormat(format);
        pipeline->setShader(shader);
        pipeline->setVertexBuffersCount(1);
        pipeline->setVertexBufferDesc(0, vertexBufferLayoutDesc);
        pipeline->setBlendEnable(false);
        pipeline->setDepthTestEnable(true);
        pipeline->setDepthWriteEnable(true);
        pipeline->createPipeline();

        auto material = std::make_shared<Material>(device);
        material->setGraphicsPipeline(pipeline);
        material->createMaterial();

        return material;
    }

    RefCounted <Material> MaterialFullscreen::fullscreenQuad(const String &shadersFolderPath, ID <IRenderDevice::Surface> surface,
                                                             const RefCounted<IRenderDevice> &device) {
        std::vector<uint8> vertexCode;
        std::vector<uint8> fragmentCode;

        auto& supportedLanguages = device->getSupportedShaderLanguages();
        auto supportsSpirv = std::find(supportedLanguages.begin(), supportedLanguages.end(), ShaderLanguage::SPIRV) != supportedLanguages.end();

        auto shader = std::make_shared<Shader>(device);

        if (supportsSpirv) {
            auto vertexName = shadersFolderPath + "spirv/FullscreenQuad.vert.spv";
            auto fragmentName = shadersFolderPath + "spirv/FullscreenQuad.frag.spv";

            FileUtils::loadBinary(vertexName, vertexCode);
            FileUtils::loadBinary(fragmentName, fragmentCode);

            if (vertexCode.empty()) {
                throw std::runtime_error("Can't find shader: " + vertexName);
            }
            if (fragmentName.empty()) {
                throw std::runtime_error("Can't find shader: " + fragmentName);
            }

            shader->fromSources(ShaderLanguage::SPIRV, vertexCode, fragmentCode);
        }
        else {
            throw std::runtime_error("No shader sources for this device type");
        }

        shader->reflectData();
        shader->generateUniformLayout();

        IRenderDevice::VertexBufferLayoutDesc vertexBufferLayoutDesc = {};
        vertexBufferLayoutDesc.stride = sizeof(Vertex);
        vertexBufferLayoutDesc.usage = VertexUsage::PerVertex;
        vertexBufferLayoutDesc.attributes.resize(2);
        vertexBufferLayoutDesc.attributes[0].format = DataFormat::R32G32_SFLOAT;
        vertexBufferLayoutDesc.attributes[0].location = 0;
        vertexBufferLayoutDesc.attributes[0].offset = offsetof(Vertex, position);
        vertexBufferLayoutDesc.attributes[1].format = DataFormat::R32G32_SFLOAT;
        vertexBufferLayoutDesc.attributes[1].location = 1;
        vertexBufferLayoutDesc.attributes[1].offset = offsetof(Vertex, texcoords);

        auto pipeline = std::make_shared<GraphicsPipeline>(device);
        pipeline->setSurface(surface);
        pipeline->setShader(shader);
        pipeline->setVertexBuffersCount(1);
        pipeline->setVertexBufferDesc(0, vertexBufferLayoutDesc);
        pipeline->setBlendEnable(false);
        pipeline->setDepthTestEnable(true);
        pipeline->setDepthWriteEnable(true);
        pipeline->createPipeline();

        auto material = std::make_shared<Material>(device);
        material->setGraphicsPipeline(pipeline);
        material->createMaterial();

        return material;
    }

    RefCounted <Material> MaterialFullscreen::noirFilter(const String &shadersFolderPath, const RefCounted<RenderTarget::Format> &format, const RefCounted<IRenderDevice> &device) {
        auto vertexName = shadersFolderPath + "spirv/NoirFilter.vert.spv";
        auto fragmentName = shadersFolderPath + "spirv/NoirFilter.frag.spv";
        return screeMaterialSpv(vertexName, fragmentName, format, device);
    }

    RefCounted<Material> MaterialFullscreen::inverseFilter(const String &shadersFolderPath, const RefCounted<RenderTarget::Format> &format, const RefCounted<IRenderDevice> &device) {
        auto vertexName = shadersFolderPath + "spirv/InverseFilter.vert.spv";
        auto fragmentName = shadersFolderPath + "spirv/InverseFilter.frag.spv";
        return screeMaterialSpv(vertexName, fragmentName, format, device);
    }

}