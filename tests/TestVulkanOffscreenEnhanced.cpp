/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov, Sultim Tsyrendashiev                  */
/**********************************************************************************/

#include <VulkanRenderDevice.h>
#include <VulkanExtensions.h>

#include <Shader.h>
#include <Sampler.h>
#include <Texture.h>
#include <RenderTarget.h>

#include <fstream>

using namespace ignimbrite;

struct VertPCf {
    float position[3];
    float color[3];
};

struct VertPf {
    float position[3];
};

struct Window {
    GLFWwindow* handle = nullptr;
    std::string name = "Offscreen Rendering";
    int32 width = 640;
    int32 height = 480;
    int32 widthFBO = 0;
    int32 heightFBO = 0;
    uint32 extensionsCount = 0;
    const char* const* extensions = nullptr;
};

struct SurfacePass {
    ID<IRenderDevice::VertexLayout> vertexLayout;
    ID<IRenderDevice::UniformSet> uniformSet;
    ID<IRenderDevice::VertexBuffer> vertexBuffer;
    ID<IRenderDevice::GraphicsPipeline> pipeline;

    RefCounted<Sampler> sampler;
    RefCounted<Shader> shader;
};

struct OffscreenPass {
    ID<IRenderDevice::VertexLayout> vertexLayout;
    ID<IRenderDevice::VertexBuffer> vertexBuffer;
    ID<IRenderDevice::GraphicsPipeline> pipeline;

    RefCounted<Shader> shader;
    RefCounted<Texture> colorTexture;
    RefCounted<Texture> depthTexture;
    RefCounted<RenderTarget> renderTarget;

    uint32 width = 0, height = 0;
};

class OffscreenRendering {
public:

    OffscreenRendering() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        window.handle = glfwCreateWindow(window.width, window.height, window.name.c_str(), nullptr, nullptr);
        glfwGetFramebufferSize(window.handle, (int32 *) &window.widthFBO, (int32 *) &window.heightFBO);
        window.extensions = glfwGetRequiredInstanceExtensions(&window.extensionsCount);

        device = std::make_shared<VulkanRenderDevice>(window.extensionsCount, window.extensions);

        surface = VulkanExtensions::createSurfaceGLFW(
                *device,
                window.handle,
                window.widthFBO, window.heightFBO,
                window.name
        );

        createOffscreenPass();
        createSurfacePass();
    }

    ~OffscreenRendering() {
        device->destroyGraphicsPipeline(surfacePass.pipeline);
        device->destroyUniformSet(surfacePass.uniformSet);
        device->destroyVertexBuffer(surfacePass.vertexBuffer);
        device->destroyVertexLayout(surfacePass.vertexLayout);

        device->destroyGraphicsPipeline(offscreenPass.pipeline);
        device->destroyVertexBuffer(offscreenPass.vertexBuffer);
        device->destroyVertexLayout(offscreenPass.vertexLayout);

        VulkanExtensions::destroySurface(*device, surface);

        glfwDestroyWindow(window.handle);
        glfwTerminate();
    }

    void createOffscreenPass() {
        loadShader(path + "gradient.vert.spv", path + "gradient.frag.spv", offscreenPass.shader);

        IRenderDevice::VertexBufferLayoutDesc vertexLayoutDesc = {};
        vertexLayoutDesc.stride = sizeof(float) * 6;
        vertexLayoutDesc.usage = VertexUsage::PerVertex;
        vertexLayoutDesc.attributes.resize(2);

        auto &vertexAttributes = vertexLayoutDesc.attributes;
        vertexAttributes[0].format = DataFormat::R32G32B32_SFLOAT;
        vertexAttributes[0].location = 0;
        vertexAttributes[0].offset = 0;
        vertexAttributes[1].format = DataFormat::R32G32B32_SFLOAT;
        vertexAttributes[1].location = 1;
        vertexAttributes[1].offset = sizeof(float) * 3;

        offscreenPass.vertexLayout = device->createVertexLayout( { vertexLayoutDesc } );

        offscreenPass.vertexBuffer = device->createVertexBuffer(BufferUsage::Static, sizeof(geometry), geometry);

        offscreenPass.width = window.widthFBO;
        offscreenPass.height = window.heightFBO;
        offscreenPass.renderTarget = std::make_shared<RenderTarget>(device);
        offscreenPass.renderTarget->createTargetFromFormat(offscreenPass.width, offscreenPass.height, RenderTarget::DefaultFormat::Color0AndDepthStencil);
        offscreenPass.colorTexture = offscreenPass.renderTarget->getAttachment(0);
        offscreenPass.depthTexture = offscreenPass.renderTarget->getDepthStencilAttachment();

        IRenderDevice::PipelineRasterizationDesc rasterizationDesc = {};
        rasterizationDesc.cullMode = PolygonCullMode::Back;
        rasterizationDesc.frontFace = PolygonFrontFace::FrontCounterClockwise;
        rasterizationDesc.lineWidth = 1.0f;
        rasterizationDesc.mode = PolygonMode::Fill;

        PrimitiveTopology topology = PrimitiveTopology::TriangleList;

        IRenderDevice::BlendAttachmentDesc blendAttachmentDesc = {};
        blendAttachmentDesc.blendEnable = false;

        IRenderDevice::PipelineBlendStateDesc blendStateDesc = {};
        blendStateDesc.attachments.push_back(blendAttachmentDesc);
        blendStateDesc.logicOpEnable = false;
        blendStateDesc.logicOp = LogicOperation::Copy;

        IRenderDevice::PipelineDepthStencilStateDesc depthStencilStateDesc = {};
        depthStencilStateDesc.depthTestEnable = true;
        depthStencilStateDesc.depthCompareOp = CompareOperation::Less;
        depthStencilStateDesc.depthWriteEnable = true;
        depthStencilStateDesc.stencilTestEnable = false;

        offscreenPass.pipeline = device->createGraphicsPipeline(
                topology,
                offscreenPass.shader->getHandle(),
                offscreenPass.vertexLayout,
                offscreenPass.shader->getLayout(),
                offscreenPass.renderTarget->getFramebufferFormat()->handle,
                rasterizationDesc,
                blendStateDesc,
                depthStencilStateDesc
        );
    }

    void createSurfacePass() {
        loadShader(path + "fullscreen.vert.spv", path + "fullscreen.frag.spv", surfacePass.shader);

        IRenderDevice::VertexBufferLayoutDesc vertexLayoutDesc = {};
        vertexLayoutDesc.stride = sizeof(float) * 3;
        vertexLayoutDesc.usage = VertexUsage::PerVertex;
        vertexLayoutDesc.attributes.resize(1);

        auto &vertexAttributes = vertexLayoutDesc.attributes;
        vertexAttributes[0].format = DataFormat::R32G32B32_SFLOAT;
        vertexAttributes[0].location = 0;
        vertexAttributes[0].offset = 0;

        surfacePass.vertexLayout = device->createVertexLayout( { vertexLayoutDesc } );

        surfacePass.vertexBuffer = device->createVertexBuffer(BufferUsage::Static, sizeof(quad), quad);


        IRenderDevice::SamplerDesc samplerDesc = {};
        samplerDesc.min = SamplerFilter::Linear;
        samplerDesc.mag = SamplerFilter::Linear;
        samplerDesc.minLod = 0.0f;
        samplerDesc.maxLod = 1.0f;
        samplerDesc.useAnisotropy = true;
        samplerDesc.anisotropyMax = 1.0f;
        samplerDesc.color = SamplerBorderColor::Black;
        samplerDesc.u = SamplerRepeatMode::Repeat;
        samplerDesc.v = SamplerRepeatMode::Repeat;
        samplerDesc.anisotropyMax = 16;
        samplerDesc.mipmapMode = SamplerFilter::Nearest;
        samplerDesc.mipLodBias = 0;

        surfacePass.sampler = std::make_shared<Sampler>(device);
        surfacePass.sampler->setHighQualityFiltering();

        IRenderDevice::UniformTextureDesc uniformTextureDesc = {};
        uniformTextureDesc.binding = 0;
        uniformTextureDesc.sampler = surfacePass.sampler->getHandle();
        uniformTextureDesc.stageFlags = (uint32) ShaderStageFlagBits::FragmentBit;
        uniformTextureDesc.texture = offscreenPass.colorTexture->getHandle();

        IRenderDevice::UniformSetDesc uniformSetDesc = {};
        uniformSetDesc.textures.push_back(uniformTextureDesc);

        surfacePass.uniformSet = device->createUniformSet(uniformSetDesc, surfacePass.shader->getLayout());

        IRenderDevice::PipelineRasterizationDesc rasterizationDesc = {};
        rasterizationDesc.cullMode = PolygonCullMode::Back;
        rasterizationDesc.frontFace = PolygonFrontFace::FrontCounterClockwise;
        rasterizationDesc.lineWidth = 1.0f;
        rasterizationDesc.mode = PolygonMode::Fill;

        PrimitiveTopology topology = PrimitiveTopology::TriangleList;

        IRenderDevice::BlendAttachmentDesc blendAttachmentDesc = {};
        blendAttachmentDesc.blendEnable = false;

        IRenderDevice::PipelineSurfaceBlendStateDesc blendStateDesc = {};
        blendStateDesc.attachment = blendAttachmentDesc;
        blendStateDesc.logicOpEnable = false;
        blendStateDesc.logicOp = LogicOperation::Copy;

        IRenderDevice::PipelineDepthStencilStateDesc depthStencilStateDesc = {};
        depthStencilStateDesc.depthTestEnable = false;
        depthStencilStateDesc.depthCompareOp = CompareOperation::Less;
        depthStencilStateDesc.depthWriteEnable = false;
        depthStencilStateDesc.stencilTestEnable = false;

        surfacePass.pipeline = device->createGraphicsPipeline(
                surface,
                topology,
                surfacePass.shader->getHandle(),
                surfacePass.vertexLayout,
                surfacePass.shader->getLayout(),
                rasterizationDesc,
                blendStateDesc,
                depthStencilStateDesc
        );

    }

    void loadShader(const std::string &vertexName, const std::string &fragmentName, RefCounted<Shader> &shader) {
        std::ifstream vertexFile(vertexName, std::ios::binary);
        std::ifstream fragmentFile(fragmentName, std::ios::binary);

        if (!vertexFile.is_open() || !fragmentFile.is_open()) {
            throw std::runtime_error("Failed to open files: " + vertexName + ", " + fragmentName);
        }

        std::vector<uint8> vertexSpv(std::istreambuf_iterator<char>(vertexFile), {});
        std::vector<uint8> fragmentSpv(std::istreambuf_iterator<char>(fragmentFile), {});

        shader = std::make_shared<Shader>(device);
        shader->fromSources(ShaderLanguage::SPIRV, vertexSpv, fragmentSpv);
        shader->reflectData();
        shader->generateUniformLayout();
    }


    void loop() {
        while (!glfwWindowShouldClose(window.handle)) {
            glfwPollEvents();
            glfwSwapBuffers(window.handle);
            glfwGetFramebufferSize(window.handle, &window.widthFBO, &window.heightFBO);

            IRenderDevice::Color color = {  { 0.1, 0.2, 0.3, 0.0 } };
            IRenderDevice::Region region = { 0, 0, { (uint32) window.widthFBO, (uint32) window.heightFBO } };
            IRenderDevice::Region regionOffscreen = { 0, 0, { offscreenPass.width, offscreenPass.height } };
            std::vector<IRenderDevice::Color> colors = { { { 0.0, 0.0, 0.0, 0.0 }} };

            if (region.extent.x == 0|| region.extent.y == 0)
                continue;

            {
                device->drawListBegin();
                device->drawListBindFramebuffer(offscreenPass.renderTarget->getHandle(), colors, regionOffscreen);
                device->drawListBindPipeline(offscreenPass.pipeline);
                device->drawListBindVertexBuffer(offscreenPass.vertexBuffer, 0, 0);
                device->drawListDraw(3, 1);
                device->drawListBindSurface(surface, color, region);
                device->drawListBindPipeline(surfacePass.pipeline);
                device->drawListBindUniformSet(surfacePass.uniformSet);
                device->drawListBindVertexBuffer(surfacePass.vertexBuffer, 0, 0);
                device->drawListDraw(6, 1);
                device->drawListEnd();

                device->flush();
                device->synchronize();
                device->swapBuffers(surface);
            }
        }
    }

private:

    const std::string path = "shaders/spirv/";

    RefCounted<VulkanRenderDevice> device;
    Window window;
    ID<IRenderDevice::Surface> surface;
    OffscreenPass offscreenPass;
    SurfacePass surfacePass;

    VertPCf geometry[3] = {
            { { -1.0f, 1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
            { { 1.0f, 1.0f, 0.0f },  { 0.0f, 1.0f, 0.0f } },
            { { 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } }
    };

    VertPf quad[6] = {
            { { -1.0f, -1.0f, 0.0f }, },
            { { -1.0f, 1.0f, 0.0f },  },
            { { 1.0f,  1.0f, 0.0f }, },
            { { 1.0f,  1.0f, 0.0f }, },
            { { 1.0f, -1.0f, 0.0f }, },
            { { -1.0f, -1.0f, 0.0f }, }
    };

};

int main() {
    OffscreenRendering offscreenRendering;
    offscreenRendering.loop();
    return 0;
}