/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov, Sultim Tsyrendashiev                  */
/**********************************************************************************/

#include <VulkanRenderDevice.h>
#include <VulkanExtensions.h>
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
    ObjectID vertexLayout;
    ObjectID uniformLayout;
    ObjectID uniformSet;
    ObjectID vertexBuffer;
    ObjectID shader;
    ObjectID pipeline;
    ObjectID sampler;
};

struct OffscreenPass {
    ObjectID vertexLayout;
    ObjectID uniformLayout;
    ObjectID vertexBuffer;
    ObjectID shader;
    ObjectID pipeline;
    ObjectID frameBufferFormat;
    ObjectID frameBuffer;
    ObjectID colorTexture;
    ObjectID depthTexture;
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

        device = new VulkanRenderDevice(window.extensionsCount, window.extensions);
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
        device->destroyUniformLayout(surfacePass.uniformLayout);
        device->destroySampler(surfacePass.sampler);
        device->destroyVertexBuffer(surfacePass.vertexBuffer);
        device->destroyVertexLayout(surfacePass.vertexLayout);
        device->destroyShaderProgram(surfacePass.shader);

        device->destroyGraphicsPipeline(offscreenPass.pipeline);
        device->destroyFramebuffer(offscreenPass.frameBuffer);
        device->destroyFramebufferFormat(offscreenPass.frameBufferFormat);
        device->destroyUniformLayout(offscreenPass.uniformLayout);
        device->destroyTexture(offscreenPass.colorTexture);
        device->destroyTexture(offscreenPass.depthTexture);
        device->destroyVertexBuffer(offscreenPass.vertexBuffer);
        device->destroyVertexLayout(offscreenPass.vertexLayout);
        device->destroyShaderProgram(offscreenPass.shader);

        VulkanExtensions::destroySurface(*device, surface);
        delete device;

        glfwDestroyWindow(window.handle);
        glfwTerminate();
    }

    void createOffscreenPass() {
        loadShader(path + "gradient.vert.spv", path + "gradient.frag.spv", offscreenPass.shader);

        RenderDevice::VertexBufferLayoutDesc vertexLayoutDesc = {};
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

        RenderDevice::UniformLayoutDesc uniformLayoutDesc = {};

        offscreenPass.uniformLayout = device->createUniformLayout(uniformLayoutDesc);

        std::vector<RenderDevice::FramebufferAttachmentDesc> attachmentsDesc(2);
        attachmentsDesc[0].format = DataFormat::R8G8B8A8_UNORM;
        attachmentsDesc[0].samples = TextureSamples::Samples1;
        attachmentsDesc[0].type = AttachmentType::Color;
        attachmentsDesc[1].format = DataFormat::D32_SFLOAT_S8_UINT;
        attachmentsDesc[1].samples = TextureSamples::Samples1;
        attachmentsDesc[1].type = AttachmentType::DepthStencil;

        offscreenPass.frameBufferFormat = device->createFramebufferFormat(attachmentsDesc);

        RenderDevice::TextureDesc colorTextureDesc = {};
        colorTextureDesc.format = DataFormat::R8G8B8A8_UNORM;
        colorTextureDesc.width = window.widthFBO;
        colorTextureDesc.height = window.heightFBO;
        colorTextureDesc.type = TextureType::Texture2D;
        colorTextureDesc.usageFlags = (uint32)TextureUsageBit::ColorAttachment | (uint32)TextureUsageBit ::ShaderSampling;

        offscreenPass.colorTexture = device->createTexture(colorTextureDesc);

        RenderDevice::TextureDesc depthTextureDesc = {};
        depthTextureDesc.format = DataFormat::D32_SFLOAT_S8_UINT;
        depthTextureDesc.width = window.widthFBO;
        depthTextureDesc.height = window.heightFBO;
        depthTextureDesc.type = TextureType::Texture2D;
        depthTextureDesc.usageFlags = (uint32)TextureUsageBit::DepthAttachment | (uint32)TextureUsageBit::ShaderSampling;

        offscreenPass.depthTexture = device->createTexture(depthTextureDesc);

        std::vector<ObjectID> attachments = { offscreenPass.colorTexture, offscreenPass.depthTexture };

        offscreenPass.frameBuffer = device->createFramebuffer(attachments, offscreenPass.frameBufferFormat);
        offscreenPass.width = window.widthFBO;
        offscreenPass.height = window.heightFBO;

        RenderDevice::PipelineRasterizationDesc rasterizationDesc = {};
        rasterizationDesc.cullMode = PolygonCullMode::Back;
        rasterizationDesc.frontFace = PolygonFrontFace::FrontCounterClockwise;
        rasterizationDesc.lineWidth = 1.0f;
        rasterizationDesc.mode = PolygonMode::Fill;

        PrimitiveTopology topology = PrimitiveTopology::TriangleList;

        RenderDevice::BlendAttachmentDesc blendAttachmentDesc = {};
        blendAttachmentDesc.blendEnable = false;

        RenderDevice::PipelineBlendStateDesc blendStateDesc = {};
        blendStateDesc.attachments.push_back(blendAttachmentDesc);
        blendStateDesc.logicOpEnable = false;
        blendStateDesc.logicOp = LogicOperation::Copy;

        RenderDevice::PipelineDepthStencilStateDesc depthStencilStateDesc = {};
        depthStencilStateDesc.depthTestEnable = true;
        depthStencilStateDesc.depthCompareOp = CompareOperation::Less;
        depthStencilStateDesc.depthWriteEnable = true;
        depthStencilStateDesc.stencilTestEnable = false;

        offscreenPass.pipeline = device->createGraphicsPipeline(
                topology,
                offscreenPass.shader,
                offscreenPass.vertexLayout,
                offscreenPass.uniformLayout,
                offscreenPass.frameBufferFormat,
                rasterizationDesc,
                blendStateDesc,
                depthStencilStateDesc
        );
    }

    void createSurfacePass() {
        loadShader(path + "fullscreen.vert.spv", path + "fullscreen.frag.spv", surfacePass.shader);

        RenderDevice::VertexBufferLayoutDesc vertexLayoutDesc = {};
        vertexLayoutDesc.stride = sizeof(float) * 3;
        vertexLayoutDesc.usage = VertexUsage::PerVertex;
        vertexLayoutDesc.attributes.resize(1);

        auto &vertexAttributes = vertexLayoutDesc.attributes;
        vertexAttributes[0].format = DataFormat::R32G32B32_SFLOAT;
        vertexAttributes[0].location = 0;
        vertexAttributes[0].offset = 0;

        surfacePass.vertexLayout = device->createVertexLayout( { vertexLayoutDesc } );

        surfacePass.vertexBuffer = device->createVertexBuffer(BufferUsage::Static, sizeof(quad), quad);

        RenderDevice::UniformLayoutTextureDesc textureDesc = {};
        textureDesc.binding = 0;
        textureDesc.flags = (uint32) ShaderStageFlagBits::FragmentBit;

        RenderDevice::UniformLayoutDesc uniformLayoutDesc = {};
        uniformLayoutDesc.textures.push_back(textureDesc);

        surfacePass.uniformLayout = device->createUniformLayout(uniformLayoutDesc);

        RenderDevice::SamplerDesc samplerDesc = {};
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

        surfacePass.sampler = device->createSampler(samplerDesc);

        RenderDevice::UniformTextureDesc uniformTextureDesc = {};
        uniformTextureDesc.binding = 0;
        uniformTextureDesc.sampler = surfacePass.sampler;
        uniformTextureDesc.stageFlags = (uint32) ShaderStageFlagBits::FragmentBit;
        uniformTextureDesc.texture = offscreenPass.colorTexture;

        RenderDevice::UniformSetDesc uniformSetDesc = {};
        uniformSetDesc.textures.push_back(uniformTextureDesc);

        surfacePass.uniformSet = device->createUniformSet(uniformSetDesc, surfacePass.uniformLayout);

        RenderDevice::PipelineRasterizationDesc rasterizationDesc = {};
        rasterizationDesc.cullMode = PolygonCullMode::Back;
        rasterizationDesc.frontFace = PolygonFrontFace::FrontCounterClockwise;
        rasterizationDesc.lineWidth = 1.0f;
        rasterizationDesc.mode = PolygonMode::Fill;

        PrimitiveTopology topology = PrimitiveTopology::TriangleList;

        RenderDevice::BlendAttachmentDesc blendAttachmentDesc = {};
        blendAttachmentDesc.blendEnable = false;

        RenderDevice::PipelineSurfaceBlendStateDesc blendStateDesc = {};
        blendStateDesc.attachment = blendAttachmentDesc;
        blendStateDesc.logicOpEnable = false;
        blendStateDesc.logicOp = LogicOperation::Copy;

        RenderDevice::PipelineDepthStencilStateDesc depthStencilStateDesc = {};
        depthStencilStateDesc.depthTestEnable = false;
        depthStencilStateDesc.depthCompareOp = CompareOperation::Less;
        depthStencilStateDesc.depthWriteEnable = false;
        depthStencilStateDesc.stencilTestEnable = false;

        surfacePass.pipeline = device->createGraphicsPipeline(
                surface,
                topology,
                surfacePass.shader,
                surfacePass.vertexLayout,
                surfacePass.uniformLayout,
                rasterizationDesc,
                blendStateDesc,
                depthStencilStateDesc
        );

    }

    void loadShader(const std::string &vertexName, const std::string &fragmentName, ObjectID& id) {
        std::ifstream vertexFile(vertexName, std::ios::binary);
        std::ifstream fragmentFile(fragmentName, std::ios::binary);

        if (!vertexFile.is_open() || !fragmentFile.is_open()) {
            throw std::runtime_error("Failed to open files: " + vertexName + ", " + fragmentName);
        }

        std::vector<uint8> vertexSpv(std::istreambuf_iterator<char>(vertexFile), {});
        std::vector<uint8> fragmentSpv(std::istreambuf_iterator<char>(fragmentFile), {});

        RenderDevice::ProgramDesc programDesc;
        programDesc.language = ShaderLanguage::SPIRV;
        programDesc.shaders.resize(2);

        programDesc.shaders[0].type = ShaderType::Vertex;
        programDesc.shaders[0].source = std::move(vertexSpv);

        programDesc.shaders[1].type = ShaderType::Fragment;
        programDesc.shaders[1].source = std::move(fragmentSpv);

        id = device->createShaderProgram(programDesc);
    }


    void loop() {
        while (!glfwWindowShouldClose(window.handle)) {
            glfwPollEvents();
            glfwSwapBuffers(window.handle);
            glfwGetFramebufferSize(window.handle, &window.widthFBO, &window.heightFBO);

            RenderDevice::Color color = {  { 0.1, 0.2, 0.3, 0.0 } };
            RenderDevice::Region region = { 0, 0, { (uint32) window.widthFBO, (uint32) window.heightFBO } };
            RenderDevice::Region regionOffscreen = { 0, 0, { offscreenPass.width, offscreenPass.height } };
            std::vector<RenderDevice::Color> colors = { { { 0.0, 0.0, 0.0, 0.0 }} };

            if (region.extent.x == 0|| region.extent.y == 0)
            {
                continue;
            }

            {
                device->drawListBegin();
                device->drawListBindFramebuffer(offscreenPass.frameBuffer, colors, regionOffscreen);
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

    VulkanRenderDevice* device;
    Window window;
    ObjectID surface;
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