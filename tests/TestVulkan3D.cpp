#ifndef IGNIMBRITELIBRARY_VULKAN3DTEST_H
#define IGNIMBRITELIBRARY_VULKAN3DTEST_H

#include <VulkanRenderDevice.h>
#include <VulkanExtensions.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <tiny_obj_loader.h>
#include <stb_image.h>

#include <fstream>

using namespace ignimbrite;
typedef ignimbrite::ObjectID ID;

struct Vertex
{
    float Position[4];
    float Color[4];
    float Normal[3];
    float UV[2];

    static void getAttributeDescriptions(std::vector<ignimbrite::RenderDevice::VertexAttributeDesc> &outAttrs);
};

class Vulkan3DTest {

public:
    Vulkan3DTest(const char *objMeshPath, const char *texturePath);
    ~Vulkan3DTest();

    void loop();

private:
    void initDevice();
    void updateScene();
    void createUniform();

    void loadModel(const char *objPath);
    void loadTexture(const char *path);
    ID loadShader(const char *vertSpirvPath, const char *fragSpirvPath);

    static void calculateMvp(
            float viewWidth, float viewHeight,
            float fovDegrees, float pitch, float yaw, float z,
            float *outMat4, float *outModelMat4
    );

    static void mouseCallback(GLFWwindow *window, double x, double y);
    static void scrollCallback(GLFWwindow *window, double x, double y);

private:
    ID surface;
    GLFWwindow* window = nullptr;
    std::string name = "Test";
    uint32 width = 640, height = 480;
    uint32 widthFrameBuffer = 0, heightFrameBuffer = 0;
    uint32 extensionsCount = 0;
    const char* const* extensions = nullptr;

    VulkanRenderDevice* pDevice = nullptr;

    ID vertexLayout;
    ID vertexBuffer;
    ID indexBuffer;
    uint32 indexCount = 0;
    ID textureId;
    ID textureSamplerId;

    ID uniformBuffer;
    ID uniformLayout;
    ID uniformSet;
    ID shaderProgram;
    ID graphicsPipeline;

    static float pitch;
    static float yaw;
    static float fov;
    static float z;
    static float prevx;
    static float prevy;

    struct ShaderUniformBuffer {
        float mvp[16] = {};
        float model[16] = {};
        float lightDir[3] = {};
        float ambient[3] = {};
    } transform;
};

float Vulkan3DTest::pitch = 0;
float Vulkan3DTest::yaw = 0;
float Vulkan3DTest::fov = 70;
float Vulkan3DTest::z = -80;
float Vulkan3DTest::prevx = 0;
float Vulkan3DTest::prevy = 0;

Vulkan3DTest::Vulkan3DTest(const char *objMeshPath, const char *texturePath) {
    initDevice();

    RenderDevice::VertexBufferLayoutDesc vertexBufferLayoutDesc = {};
    Vertex::getAttributeDescriptions(vertexBufferLayoutDesc.attributes);
    vertexBufferLayoutDesc.stride = sizeof(Vertex);
    vertexBufferLayoutDesc.usage = VertexUsage::PerVertex;

    vertexLayout = pDevice->createVertexLayout({ vertexBufferLayoutDesc });

    loadModel(objMeshPath);
    loadTexture(texturePath);

    uniformBuffer = pDevice->createUniformBuffer(BufferUsage::Dynamic, sizeof(ShaderUniformBuffer), nullptr);

    shaderProgram = loadShader(
            "resources/shaders/spirv/vert3d.spv",
            "resources/shaders/spirv/frag3d.spv"
    );

    createUniform();

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

    graphicsPipeline = pDevice->createGraphicsPipeline(
            surface, PrimitiveTopology::TriangleList,
            shaderProgram, vertexLayout, uniformLayout,
            rasterizationDesc, blendStateDesc, depthStencilStateDesc
    );
}

void Vulkan3DTest::initDevice() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
    glfwGetFramebufferSize(window, (int*) &widthFrameBuffer, (int*) &heightFrameBuffer);
    extensions = glfwGetRequiredInstanceExtensions(&extensionsCount);

    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetScrollCallback(window, scrollCallback);

    pDevice = new VulkanRenderDevice(extensionsCount, extensions);
    surface = VulkanExtensions::createSurfaceGLFW(*pDevice, window, width, height, widthFrameBuffer, heightFrameBuffer, name);
}

Vulkan3DTest::~Vulkan3DTest() {
    pDevice->destroyVertexBuffer(vertexBuffer);
    pDevice->destroyVertexLayout(vertexLayout);
    pDevice->destroyIndexBuffer(indexBuffer);

    pDevice->destroyUniformSet(uniformSet);
    pDevice->destroyUniformBuffer(uniformBuffer);
    pDevice->destroyUniformLayout(uniformLayout);

    pDevice->destroyTexture(textureId);
    pDevice->destroySampler(textureSamplerId);

    pDevice->destroyGraphicsPipeline(graphicsPipeline);
    pDevice->destroyShaderProgram(shaderProgram);

    ignimbrite::VulkanExtensions::destroySurface(*pDevice, surface);
    glfwDestroyWindow(window);
    glfwTerminate();

    delete pDevice;
}

void Vulkan3DTest::loop() {
    auto& device = *pDevice;

    RenderDevice::Region area = { 0, 0, { widthFrameBuffer, heightFrameBuffer} };
    RenderDevice::Color clearColor = { { 0.8, 0.8, 0.8, 0.0} };
    std::vector<RenderDevice::Color> clearColors = { clearColor };

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glfwSwapBuffers(window);

        updateScene();

        device.swapBuffers(surface);
        device.drawListBegin();
        {
            device.drawListBindSurface(surface, clearColor, area);
            device.drawListBindPipeline(graphicsPipeline);

            device.drawListBindUniformSet(uniformSet);
            device.drawListBindVertexBuffer(vertexBuffer, 0, 0);
            device.drawListBindIndexBuffer(indexBuffer, ignimbrite::IndicesType::Uint32, 0);

            device.drawListDrawIndexed(indexCount, 1);
        }
        device.drawListEnd();
    }
}

void Vulkan3DTest::updateScene() {
    calculateMvp(width, height, fov, pitch, yaw, z, transform.mvp, transform.model);
    transform.lightDir[0] = -1;
    transform.lightDir[1] = 1;
    transform.lightDir[2] = -0.5f;
    transform.ambient[0] = 0.1f;
    transform.ambient[1] = 0.1f;
    transform.ambient[2] = 0.1f;
    pDevice->updateUniformBuffer(uniformBuffer, sizeof(ShaderUniformBuffer), 0, &transform);
}

void Vulkan3DTest::calculateMvp(float viewWidth, float viewHeight,
        float fovDegrees, float apitch, float ayaw, float cz,
        float *outMat4, float *outModelMat4) {
    auto projection = glm::perspective(fovDegrees, viewWidth / viewHeight, 0.1f, 100.0f);

    auto view = glm::lookAt(
            glm::vec3(0, 3, cz),
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

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            outMat4[i * 4 + j] = mvp[i][j];
            outModelMat4[i * 4 + j] = model[i][j];
        }
    }
}

void Vulkan3DTest::loadModel(const char *path) {
    std::vector<Vertex> outVertices;
    std::vector<uint32> outIndices;

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    bool loaded = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path);
    assert(loaded);

    bool useUv = attrib.texcoords.size() != 0;

    uint32 i = 0;
    for (const auto& shape : shapes)
    {
        for (const auto& index : shape.mesh.indices)
        {
            Vertex vertex = {};

            vertex.Position[0] = attrib.vertices[3 * index.vertex_index + 0];
            vertex.Position[1] = attrib.vertices[3 * index.vertex_index + 1];
            vertex.Position[2] = attrib.vertices[3 * index.vertex_index + 2];
            vertex.Position[3] = 1.0f;

            if (attrib.normals.size() > 0)
            {
                vertex.Normal[0] = attrib.normals[3 * index.normal_index + 0];
                vertex.Normal[1] = attrib.normals[3 * index.normal_index + 1];
                vertex.Normal[2] = attrib.normals[3 * index.normal_index + 2];
            }
            else
            {
                vertex.Normal[0] = 0.0f;
                vertex.Normal[1] = 1.0f;
                vertex.Normal[2] = 0.0f;
            }

            if (useUv) {
                vertex.UV[0] = attrib.texcoords[2 * index.texcoord_index + 0];
                vertex.UV[1] = 1.0f - attrib.texcoords[2 * index.texcoord_index + 1];
            }

            vertex.Color[0] = 1.0f;
            vertex.Color[1] = 1.0f;
            vertex.Color[2] = 1.0f;
            vertex.Color[3] = 1.0f;

            outVertices.push_back(vertex);
            outIndices.push_back(i++);
        }
    }

    vertexBuffer = pDevice->createVertexBuffer(BufferUsage::Dynamic,
                                             outVertices.size() * sizeof(Vertex), outVertices.data());

    indexCount = outIndices.size();
    indexBuffer = pDevice->createIndexBuffer(BufferUsage::Static,
                                             indexCount * sizeof(uint32), outIndices.data());
}

Vulkan3DTest::ID Vulkan3DTest::loadShader(const char *vertSpirvPath, const char *fragSpirvPath) {


    std::vector<RenderDevice::ShaderDataDesc> shaderDescs(2);

    shaderDescs[0].language = ShaderLanguage::SPIRV;
    shaderDescs[1].language = ShaderLanguage::SPIRV;
    shaderDescs[0].type = ShaderType::Vertex;
    shaderDescs[1].type = ShaderType::Fragment;

    std::ifstream vertFile(vertSpirvPath, std::ios::binary);
    std::ifstream fragFile(fragSpirvPath, std::ios::binary);

    std::vector<uint8> vertSpv(std::istreambuf_iterator<char>(vertFile), {});
    std::vector<uint8> fragSpv(std::istreambuf_iterator<char>(fragFile), {});

    shaderDescs[0].source = std::move(vertSpv);
    shaderDescs[1].source = std::move(fragSpv);

    return pDevice->createShaderProgram(shaderDescs);
}

void Vulkan3DTest::loadTexture(const char *path) {
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(path, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    uint32 mipmapCount = (uint32)std::floor(std::log2(std::max(texWidth, texHeight))) + 1;

    RenderDevice::TextureDesc textureDesc = {};
    textureDesc.height = texHeight;
    textureDesc.width = texWidth;
    textureDesc.depth = 1;
    textureDesc.type = TextureType::Texture2D;
    textureDesc.usageFlags = (uint32)TextureUsageBit::ShaderSampling;
    textureDesc.format = DataFormat::R8G8B8A8_UNORM;
    textureDesc.data = pixels;
    textureDesc.dataSize = texWidth * texHeight * texChannels;
    textureDesc.mipmaps = mipmapCount;

    textureId = pDevice->createTexture(textureDesc);

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

    textureSamplerId = pDevice->createSampler(samplerDesc);
}

void Vulkan3DTest::createUniform() {


    RenderDevice::UniformLayoutBufferDesc uniformLayoutBufferDesc = {};
    uniformLayoutBufferDesc.binding = 0;
    uniformLayoutBufferDesc.flags = (uint32) ShaderStageFlagBits::VertexBit;
    RenderDevice::UniformLayoutTextureDesc uniformLayoutTextureDesc = {};
    uniformLayoutTextureDesc.binding = 1;
    uniformLayoutTextureDesc.flags = (ShaderStageFlags)ShaderStageFlagBits::FragmentBit;

    RenderDevice::UniformLayoutDesc uniformLayoutDesc = {};
    uniformLayoutDesc.buffers.push_back(uniformLayoutBufferDesc);
    uniformLayoutDesc.textures.push_back(uniformLayoutTextureDesc);

    uniformLayout = pDevice->createUniformLayout(uniformLayoutDesc);


    RenderDevice::UniformBufferDesc uniformBufferDesc = {};
    uniformBufferDesc.binding = 0;
    uniformBufferDesc.offset = 0;
    uniformBufferDesc.range = sizeof(ShaderUniformBuffer);
    uniformBufferDesc.buffer = uniformBuffer;
    RenderDevice::UniformTextureDesc uniformTextureDesc = {};
    uniformTextureDesc.binding = 1;
    uniformTextureDesc.texture = textureId;
    uniformTextureDesc.sampler = textureSamplerId;
    uniformTextureDesc.stageFlags = (ShaderStageFlags)ShaderStageFlagBits::FragmentBit;

    RenderDevice::UniformSetDesc uniformSetDesc = {};
    uniformSetDesc.buffers.push_back(uniformBufferDesc);
    uniformSetDesc.textures.push_back(uniformTextureDesc);

    uniformSet = pDevice->createUniformSet(uniformSetDesc, uniformLayout);
}

void Vulkan3DTest::scrollCallback(GLFWwindow*, double, double y) {
    z += (float)y;
}

void Vulkan3DTest::mouseCallback(GLFWwindow *window, double x, double y) {
    const float sensitivity = 0.01f;

    if (glfwGetMouseButton(window, 0) == GLFW_PRESS) {
        yaw += (float)x * sensitivity - prevx;
        pitch -= (float)y * sensitivity - prevy;
    }

    prevx = (float)x * sensitivity;
    prevy = (float)y * sensitivity;
}

void Vertex::getAttributeDescriptions(std::vector<ignimbrite::RenderDevice::VertexAttributeDesc> &outAttrs) {
    outAttrs.resize(4);

    outAttrs[0].location = 0;
    outAttrs[0].format = DataFormat::R32G32B32A32_SFLOAT;
    outAttrs[0].offset = offsetof(Vertex, Position);

    outAttrs[1].location = 1;
    outAttrs[1].format = DataFormat::R32G32B32A32_SFLOAT;
    outAttrs[1].offset = offsetof(Vertex, Color);

    outAttrs[2].location = 2;
    outAttrs[2].format = DataFormat::R32G32B32_SFLOAT;
    outAttrs[2].offset = offsetof(Vertex, Normal);

    outAttrs[3].location = 3;
    outAttrs[3].format = DataFormat::R32G32_SFLOAT;
    outAttrs[3].offset = offsetof(Vertex, UV);
}

int main(int argc, char **argv) {
    if (argc < 3 || (argc >= 2 && std::strcmp(argv[1], "--help") == 0)) {
        printf("Arguments should be: <path to .obj mesh> <path to texture>\n");
        return 0;
    }
    
    Vulkan3DTest test;
    test.init(argv[1], argv[2]);
    test.loop();
}

#endif //IGNIMBRITELIBRARY_VULKAN3DTEST_H
