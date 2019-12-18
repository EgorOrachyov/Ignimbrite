#ifndef IGNIMBRITELIBRARY_VULKAN3DTEST_H
#define IGNIMBRITELIBRARY_VULKAN3DTEST_H

#include <VulkanRenderDevice.h>
#include <VulkanExtensions.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

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
    void init();
    void loop();
    ~Vulkan3DTest();

private:
    void initDevice();

    void loadModel(const char *objPath);
    void loadShader(const char *vertSpirvPath, const char *fragSpirvPath);

    static void calculateMvp(float viewWidth, float viewHeight, float fov, float angle, float *outMat4);

private:
    typedef ignimbrite::ObjectID ID;

    ID surface;
    GLFWwindow* window;
    std::string name = "Test";
    uint32_t width = 960, height = 720;
    uint32_t widthFrameBuffer, heightFrameBuffer;
    uint32_t extensionsCount;
    const char* const* extensions;

    ignimbrite::VulkanRenderDevice* pDevice;

    ID vertexLayout;
    ID vertexBuffer;
    ID indexBuffer;
    uint32_t indexCount;
    ID uniformMvpBuffer;
    ID uniformLayout;
    ID uniformSet;
    ID shaderProgram;
    ID graphicsPipeline;

    struct Transform {
        float values[16];
    } transform;
};

void Vulkan3DTest::init() {
    using namespace ignimbrite;

    initDevice();

    auto& device = *pDevice;

    RenderDevice::VertexBufferLayoutDesc vertexBufferLayoutDesc = {};
    Vertex::getAttributeDescriptions(vertexBufferLayoutDesc.attributes);
    vertexBufferLayoutDesc.stride = sizeof(Vertex);
    vertexBufferLayoutDesc.usage = VertexUsage::PerVertex;

    vertexLayout = device.createVertexLayout({ vertexBufferLayoutDesc });

    loadModel("double.obj");

    uniformMvpBuffer = device.createUniformBuffer(BufferUsage::Dynamic, sizeof(Transform), transform.values);
    calculateMvp(width, height, 50, 0, transform.values);
    device.updateUniformBuffer(uniformMvpBuffer, sizeof(Transform), 0, transform.values);

    loadShader("resources/shaders/spirv/vert3d.spv", "resources/shaders/spirv/frag3d.spv");

    RenderDevice::UniformLayoutBufferDesc uniformLayoutBufferDesc = {};
    uniformLayoutBufferDesc.binding = 0;
    uniformLayoutBufferDesc.flags = (uint32) ShaderStageFlagBits::VertexBit;

    RenderDevice::UniformLayoutDesc uniformLayoutDesc = {};
    uniformLayoutDesc.buffers.push_back(uniformLayoutBufferDesc);

    uniformLayout = device.createUniformLayout(uniformLayoutDesc);

    RenderDevice::UniformBufferDesc uniformBufferDesc = {};
    uniformBufferDesc.binding = 0;
    uniformBufferDesc.offset = 0;
    uniformBufferDesc.range = sizeof(Transform);
    uniformBufferDesc.buffer = uniformMvpBuffer;

    RenderDevice::UniformSetDesc uniformSetDesc = {};
    uniformSetDesc.buffers.push_back(uniformBufferDesc);

    uniformSet = device.createUniformSet(uniformSetDesc, uniformLayout);

    RenderDevice::PipelineRasterizationDesc rasterizationDesc = {};
    rasterizationDesc.cullMode = PolygonCullMode::Disabled;
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

    graphicsPipeline = device.createGraphicsPipeline(
            surface,
            topology,
            shaderProgram, vertexLayout, uniformLayout, rasterizationDesc, blendStateDesc
    );
}

void Vulkan3DTest::initDevice() {
    using namespace ignimbrite;

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
    glfwGetFramebufferSize(window, (int*) &widthFrameBuffer, (int*) &heightFrameBuffer);
    extensions = glfwGetRequiredInstanceExtensions(&extensionsCount);

    pDevice = new VulkanRenderDevice(extensionsCount, extensions);
    auto& device = *pDevice;

    surface = VulkanExtensions::createSurfaceGLFW(device, window, width, height, widthFrameBuffer, heightFrameBuffer, name);
}

Vulkan3DTest::~Vulkan3DTest() {

    pDevice->destroyVertexBuffer(vertexBuffer);
    pDevice->destroyVertexLayout(vertexLayout);
    pDevice->destroyIndexBuffer(indexBuffer);

    pDevice->destroyUniformSet(uniformSet);
    pDevice->destroyUniformBuffer(uniformMvpBuffer);
    pDevice->destroyUniformLayout(uniformLayout);

    pDevice->destroyGraphicsPipeline(graphicsPipeline);
    pDevice->destroyShaderProgram(shaderProgram);

    ignimbrite::VulkanExtensions::destroySurface(*pDevice, surface);
    glfwDestroyWindow(window);
    glfwTerminate();

    delete pDevice;
}

void Vulkan3DTest::loop() {
    using namespace ignimbrite;

    auto& device = *pDevice;

    RenderDevice::Color clearColor = {};
    clearColor.components[0] = clearColor.components[1] = clearColor.components[2] = 0.5f;

    RenderDevice::Region area = {};
    area.extent.x = width;
    area.extent.y = height;

    // TODO: DELETE THIS
    device.swapBuffers(surface);

    float b = 0.0f, delta = 0.005f;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        device.drawListBegin();
        // TODO: default render area, which is same as surface size
        device.drawListBindSurface(surface, clearColor, area);

        device.drawListBindPipeline(graphicsPipeline);

        calculateMvp(width, height, 50, b, transform.values);
        device.updateUniformBuffer(uniformMvpBuffer, sizeof(Transform), 0, transform.values);
        device.drawListBindUniformSet(uniformSet);
        device.drawListBindVertexBuffer(vertexBuffer, 0, 0);
        device.drawListBindIndexBuffer(indexBuffer, IndicesType::Uint32, 0);
        device.drawListDrawIndexed(indexCount, 1);

        device.drawListEnd();

        device.swapBuffers(surface);
        //glfwSwapBuffers(window);

        b += delta;
    }
}

void Vulkan3DTest::calculateMvp(float viewWidth, float viewHeight, float fov, float angle, float *outMat4) {
    if (viewWidth > viewHeight)
    {
        fov *= viewHeight / viewWidth;
    }

    auto projection = glm::perspective(fov, viewWidth / viewHeight, 0.1f, 100.0f);

    auto view = glm::lookAt(
            glm::vec3(-5, 0, -5),
            glm::vec3(0, 0, 0),
            glm::vec3(0, 1, 0)
    );

    auto model = glm::mat4(1.0f);
    model = glm::rotate(model, angle, glm::vec3(0, 1, 0));

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
        }
    }
}

void Vulkan3DTest::loadModel(const char *path) {

    using namespace ignimbrite;

    std::vector<Vertex> outVertices;
    std::vector<uint32> outIndices;

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    bool loaded = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path);
    assert(loaded);

    uint32_t i = 0;
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

            vertex.UV[0] = attrib.texcoords[2 * index.texcoord_index + 0];
            vertex.UV[1] = 1.0f - attrib.texcoords[2 * index.texcoord_index + 1];

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

void Vulkan3DTest::loadShader(const char *vertSpirvPath, const char *fragSpirvPath) {

    using namespace ignimbrite;


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

    shaderProgram = pDevice->createShaderProgram(shaderDescs);
}

void Vertex::getAttributeDescriptions(std::vector<ignimbrite::RenderDevice::VertexAttributeDesc> &outAttrs) {
    using namespace ignimbrite;

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

int main() {
    Vulkan3DTest test;
    test.init();
    test.loop();
}

#endif //IGNIMBRITELIBRARY_VULKAN3DTEST_H
