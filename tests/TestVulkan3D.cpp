#ifndef IGNIMBRITE_VULKAN3DTEST_H
#define IGNIMBRITE_VULKAN3DTEST_H

#include <VulkanRenderDevice.h>
#include <VulkanExtensions.h>
#include <Shader.h>
#include <UniformBuffer.h>
#include <MeshLoader.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <stb_image.h>

#include <algorithm>
#include <fstream>
#include <string>

using namespace ignimbrite;

struct Vertex {
    float32 Position[3];
    float32 Normal[3];
    float32 UV[2];
};

struct RenderableMesh {
    ID<RenderDevice::VertexLayout> vertexLayout;
    ID<RenderDevice::VertexBuffer> vertexBuffer;
    ID<RenderDevice::IndexBuffer> indexBuffer;
    uint32 indexCount = 0;
};

struct ShaderUniformBuffer {
    float mvp[16] = {};
    float model[16] = {};
    float lightDir[3] = {};
    float ambient[3] = {};
};

struct Material {
    std::shared_ptr<Shader> shader;
    ID<RenderDevice::UniformLayout> uniformLayout;
    ID<RenderDevice::GraphicsPipeline> graphicsPipeline;
    ID<RenderDevice::UniformSet> uniformSet;
    std::shared_ptr<UniformBuffer> uniformBuffer;
    ShaderUniformBuffer data;
    ID<RenderDevice::Texture> texture;
    ID<RenderDevice::Sampler> textureSampler;
};

struct Window {
    GLFWwindow* glfwWindow = nullptr;
    std::string name;
    int32 width = 0, height = 0;
    int32 widthFrameBuffer = 0, heightFrameBuffer = 0;
    uint32 extensionsCount = 0;
    const char* const* extensions = nullptr;
};

class Vulkan3DTest {
public:
    Vulkan3DTest(const char *objMeshPath, const char *texturePath) {
        this->objMeshPath = objMeshPath;
        this->texturePath = texturePath;

        init();
    }

    ~Vulkan3DTest() {
        destroy();
    }

    void loop() {
        RenderDevice::Color clearColor = { { 1.0f, 1.0f, 1.0f, 0.0f} };

        while (!glfwWindowShouldClose(window.glfwWindow)) {
            glfwPollEvents();
            glfwSwapBuffers(window.glfwWindow);
            glfwGetFramebufferSize(window.glfwWindow, &window.widthFrameBuffer, &window.height);

            RenderDevice::Region area = { 0, 0, { (uint32)window.widthFrameBuffer, (uint32)window.heightFrameBuffer} };

            if (area.extent.x == 0|| area.extent.y == 0)
            {
                continue;
            }

            updateScene();

            pDevice->drawListBegin();
            {
                pDevice->drawListBindSurface(surface, clearColor, area);
                pDevice->drawListBindPipeline(material.graphicsPipeline);

                pDevice->drawListBindUniformSet(material.uniformSet);
                pDevice->drawListBindVertexBuffer(rmesh.vertexBuffer, 0, 0);
                pDevice->drawListBindIndexBuffer(rmesh.indexBuffer, ignimbrite::IndicesType::Uint32, 0);

                pDevice->drawListDrawIndexed(rmesh.indexCount, 1);
            }
            pDevice->drawListEnd();

            pDevice->flush();
            pDevice->synchronize();
            pDevice->swapBuffers(surface);
        }
    }

private:
    void init() {
        // create window to render in
        initWindow();

        // create render device
        pDevice = std::make_shared<VulkanRenderDevice>(window.extensionsCount, window.extensions);

        // create surface
        surface = VulkanExtensions::createSurfaceGLFW(
                *pDevice, window.glfwWindow, window.widthFrameBuffer, window.heightFrameBuffer, name);

        // init vertex layout and load model
        initModel();

        // init material and its resources
        initMaterial();

        // create graphics pipeline
        initGraphicsPipeline();
    }

    void initWindow() {
        window.width = 1024;
        window.height = 720;

        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        window.glfwWindow = glfwCreateWindow(window.width, window.height, name.c_str(), nullptr, nullptr);

        glfwGetFramebufferSize(window.glfwWindow,
                (int*) &window.widthFrameBuffer, (int*) &window.heightFrameBuffer);

        // get required extensions for a surface
        window.extensions = glfwGetRequiredInstanceExtensions(&window.extensionsCount);

        glfwSetCursorPosCallback(window.glfwWindow, mouseCallback);
        glfwSetScrollCallback(window.glfwWindow, scrollCallback);
    }

    void initModel() {
        // declare vertex layout
        initVertexLayout();
        // init mesh vertex and index buffers
        loadObjModel(objMeshPath.c_str());
    }

    void initVertexLayout() {
        RenderDevice::VertexBufferLayoutDesc vertexBufferLayoutDesc = {};

        std::vector<RenderDevice::VertexAttributeDesc> &attrs = vertexBufferLayoutDesc.attributes;
        attrs.resize(3);

        attrs[0].location = 0;
        attrs[0].format = DataFormat::R32G32B32_SFLOAT;
        attrs[0].offset = offsetof(Vertex, Position);

        attrs[1].location = 1;
        attrs[1].format = DataFormat::R32G32B32_SFLOAT;
        attrs[1].offset = offsetof(Vertex, Normal);

        attrs[2].location = 2;
        attrs[2].format = DataFormat::R32G32_SFLOAT;
        attrs[2].offset = offsetof(Vertex, UV);

        vertexBufferLayoutDesc.stride = sizeof(Vertex);
        vertexBufferLayoutDesc.usage = VertexUsage::PerVertex;

        rmesh.vertexLayout = pDevice->createVertexLayout({ vertexBufferLayoutDesc });
    }

    void loadObjModel(const char *path) {
        MeshLoader meshLoader(path);
        cmesh = meshLoader.importMesh(Mesh::VertexFormat::PNT);

        rmesh.vertexBuffer = pDevice->createVertexBuffer(BufferUsage::Dynamic,
                cmesh->getVertexCount() * sizeof(Vertex), cmesh->getVertexData());
        rmesh.indexCount = cmesh->getIndexCount();
        rmesh.indexBuffer = pDevice->createIndexBuffer(BufferUsage::Static,
                rmesh.indexCount * sizeof(uint32), cmesh->getIndexData());
    }

    void initMaterial() {
        // load shader, init its uniform layout
        initShader();
        initUniformLayout();

        // prepare buffers and textures for material
        initUniformBuffers();
        initTextures();

        // bind created resources to uniform set
        initUniformSet();
    }

    void initShader() {
        std::ifstream vertFile(MODEL3D_SHADER_PATH_VERT.c_str(), std::ios::binary);
        std::ifstream fragFile(MODEL3D_SHADER_PATH_FRAG.c_str(), std::ios::binary);

        std::vector<uint8> vertSpv(std::istreambuf_iterator<char>(vertFile), {});
        std::vector<uint8> fragSpv(std::istreambuf_iterator<char>(fragFile), {});

        material.shader = std::make_shared<Shader>(pDevice);
        material.shader->fromSources(ShaderLanguage::SPIRV, vertSpv, fragSpv);
        material.shader->reflectData();

    }

    void initUniformLayout() {
        RenderDevice::UniformLayoutBufferDesc uniformLayoutBufferDesc = {};
        uniformLayoutBufferDesc.binding = 0;
        uniformLayoutBufferDesc.flags = (uint32) ShaderStageFlagBits::VertexBit;
        RenderDevice::UniformLayoutTextureDesc uniformLayoutTextureDesc = {};
        uniformLayoutTextureDesc.binding = 1;
        uniformLayoutTextureDesc.flags = (ShaderStageFlags) ShaderStageFlagBits::FragmentBit;

        RenderDevice::UniformLayoutDesc uniformLayoutDesc = {};
        uniformLayoutDesc.buffers.push_back(uniformLayoutBufferDesc);
        uniformLayoutDesc.textures.push_back(uniformLayoutTextureDesc);

        material.uniformLayout = pDevice->createUniformLayout(uniformLayoutDesc);
    }

    void initUniformBuffers() {
        material.uniformBuffer = std::make_shared<UniformBuffer>(pDevice);
        material.uniformBuffer->createBuffer(sizeof(ShaderUniformBuffer));
    }

    void initTextures() {
        loadTexture(texturePath.c_str());
    }

    void loadTexture(const char *path) {
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(path, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

        if (pixels == nullptr) {
            throw std::runtime_error(std::string("Can't load texture at: ") + path);
        }

        uint32 mipmapCount = (uint32)std::floor(std::log2(std::max(texWidth, texHeight))) + 1;

        RenderDevice::TextureDesc textureDesc = {};
        textureDesc.height = texHeight;
        textureDesc.width = texWidth;
        textureDesc.depth = 1;
        textureDesc.size = texHeight * texWidth * 4;
        textureDesc.type = TextureType::Texture2D;
        textureDesc.usageFlags = (uint32)TextureUsageBit::ShaderSampling;
        textureDesc.format = DataFormat::R8G8B8A8_UNORM;
        textureDesc.data = pixels;
        textureDesc.size = texWidth * texHeight * texChannels;
        textureDesc.mipmaps = mipmapCount;

        material.texture = pDevice->createTexture(textureDesc);

        stbi_image_free(pixels);

        RenderDevice::SamplerDesc samplerDesc = {};
        samplerDesc.mag = SamplerFilter::Linear;
        samplerDesc.min = SamplerFilter::Linear;
        samplerDesc.u = samplerDesc.v = samplerDesc.w = SamplerRepeatMode::Repeat;
        samplerDesc.useAnisotropy = true;
        samplerDesc.anisotropyMax = 16;
        samplerDesc.color = SamplerBorderColor::Black;
        samplerDesc.minLod = 0;
        samplerDesc.maxLod = mipmapCount;
        samplerDesc.mipmapMode = SamplerFilter::Linear;
        samplerDesc.mipLodBias = 0;

        material.textureSampler = pDevice->createSampler(samplerDesc);
    }

    void initUniformSet() {
        RenderDevice::UniformBufferDesc uniformBufferDesc = {};
        uniformBufferDesc.binding = 0;
        uniformBufferDesc.offset = 0;
        uniformBufferDesc.range = sizeof(ShaderUniformBuffer);
        uniformBufferDesc.buffer = material.uniformBuffer->getHandle();
        RenderDevice::UniformTextureDesc uniformTextureDesc = {};
        uniformTextureDesc.binding = 1;
        uniformTextureDesc.texture = material.texture;
        uniformTextureDesc.sampler = material.textureSampler;
        uniformTextureDesc.stageFlags = (ShaderStageFlags)ShaderStageFlagBits::FragmentBit;

        RenderDevice::UniformSetDesc uniformSetDesc = {};
        uniformSetDesc.buffers.push_back(uniformBufferDesc);
        uniformSetDesc.textures.push_back(uniformTextureDesc);

        material.uniformSet = pDevice->createUniformSet(uniformSetDesc, material.uniformLayout);
    }

    void initGraphicsPipeline() {
        RenderDevice::PipelineRasterizationDesc rasterizationDesc = {};
        rasterizationDesc.cullMode = PolygonCullMode::Back;
        rasterizationDesc.frontFace = PolygonFrontFace::FrontCounterClockwise;
        rasterizationDesc.lineWidth = 1.0f;
        rasterizationDesc.mode = PolygonMode::Fill;

        RenderDevice::BlendAttachmentDesc blendAttachmentDesc = {};
        blendAttachmentDesc.blendEnable = false;
        RenderDevice::PipelineSurfaceBlendStateDesc blendStateDesc = {};
        blendStateDesc.attachment = blendAttachmentDesc;
        blendStateDesc.logicOpEnable = false;
        blendStateDesc.logicOp = LogicOperation::NoOp;

        RenderDevice::PipelineDepthStencilStateDesc depthStencilStateDesc = {};
        depthStencilStateDesc.depthCompareOp = CompareOperation::Less;
        depthStencilStateDesc.depthWriteEnable = true;
        depthStencilStateDesc.depthTestEnable = true;
        depthStencilStateDesc.stencilTestEnable = false;

        material.graphicsPipeline = pDevice->createGraphicsPipeline(
                surface, PrimitiveTopology::TriangleList,
                material.shader->getHandle(), rmesh.vertexLayout,  material.uniformLayout,
                rasterizationDesc, blendStateDesc, depthStencilStateDesc
        );
    }

    void destroy() {
        pDevice->destroyVertexBuffer(rmesh.vertexBuffer);
        pDevice->destroyVertexLayout(rmesh.vertexLayout);
        pDevice->destroyIndexBuffer(rmesh.indexBuffer);

        pDevice->destroyUniformSet(material.uniformSet);
        pDevice->destroyUniformLayout(material.uniformLayout);

        pDevice->destroyTexture(material.texture);
        pDevice->destroySampler(material.textureSampler);

        pDevice->destroyGraphicsPipeline(material.graphicsPipeline);
        material.shader = nullptr; // or material.shader->releaseHandle();
        material.uniformBuffer = nullptr;

        ignimbrite::VulkanExtensions::destroySurface(*pDevice, surface);
        pDevice = nullptr;
        cmesh = nullptr;

        glfwDestroyWindow(window.glfwWindow);
        glfwTerminate();
    }


    void updateScene() {
        calculateMvp(window.widthFrameBuffer, window.heightFrameBuffer, fov, pitch, yaw, z,
                material.data.mvp, material.data.model);

        material.data.lightDir[0] = -1;
        material.data.lightDir[1] = 1;
        material.data.lightDir[2] = -0.5f;
        material.data.ambient[0] = 0.1f;
        material.data.ambient[1] = 0.1f;
        material.data.ambient[2] = 0.1f;

        material.uniformBuffer->updateData(sizeof(ShaderUniformBuffer), 0, (uint8*)&material.data);
    }

    static void calculateMvp(float viewWidth, float viewHeight,
                             float fovDegrees, float apitch, float ayaw, float cz,
                             float *outMat4, float *outModelMat4) {
        auto projection = glm::perspective(fovDegrees, viewWidth / viewHeight, 0.1f, 100.0f);

        auto view = glm::lookAt(
                glm::vec3(0, 0, cz),
                glm::vec3(0, 0, 0),
                glm::vec3(0, 1, 0)
        );

        auto model = glm::mat4(1.0f);
        model = glm::rotate(model, apitch, glm::vec3(1, 0, 0));
        model = glm::rotate(model, ayaw, glm::vec3(0, 1, 0));

        auto clip = glm::mat4(1.0f, 0.0f, 0.0f, 0.0f,
                              0.0f, -1.0f, 0.0f, 0.0f,
                              0.0f, 0.0f, 0.5f, 0.0f,
                              0.0f, 0.0f, 0.5f, 1.0f);

        auto mvp = clip * projection * view * model;

        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                outMat4[i * 4 + j] = mvp[i][j];
                outModelMat4[i * 4 + j] = model[i][j];
            }
        }
    }

    static void mouseCallback(GLFWwindow *window, double x, double y) {
        const float sensitivity = 0.01f;

        if (glfwGetMouseButton(window, 0) == GLFW_PRESS) {
            yaw += (float)x * sensitivity - prevx;
            pitch -= (float)y * sensitivity - prevy;
        }

        prevx = (float)x * sensitivity;
        prevy = (float)y * sensitivity;
    }

    static void scrollCallback(GLFWwindow *, double, double y) {
        z += (float)y;
    }

private:

    std::shared_ptr<VulkanRenderDevice> pDevice;
    ID<RenderDevice::Surface> surface;
    Window           window;
    RefCounted<Mesh> cmesh;
    RenderableMesh   rmesh;
    Material         material;

    String name = "Textured 3D model";
    String objMeshPath;
    String texturePath;

    String MODEL3D_SHADER_PATH_VERT = "shaders/spirv/vert3d.spv";
    String MODEL3D_SHADER_PATH_FRAG = "shaders/spirv/frag3d.spv";

    static float pitch;
    static float yaw;
    static float fov;
    static float z;
    static float prevx;
    static float prevy;
};

float Vulkan3DTest::pitch = 0;
float Vulkan3DTest::yaw = 0;
float Vulkan3DTest::fov = 70;
float Vulkan3DTest::z = -80;
float Vulkan3DTest::prevx = 0;
float Vulkan3DTest::prevy = 0;

int main(int argc, char **argv) {
    const char *mesh = "assets/models/sphere.obj";
    const char *texture = "assets/textures/double.png";

    if (argc >= 3) {
        mesh = argv[1];
        texture = argv[2];
    }
    
    Vulkan3DTest test(mesh, texture);
    test.loop();
}

#endif //IGNIMBRITE_VULKAN3DTEST_H
