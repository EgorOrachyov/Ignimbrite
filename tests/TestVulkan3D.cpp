#ifndef IGNIMBRITE_VULKAN3DTEST_H
#define IGNIMBRITE_VULKAN3DTEST_H

#include <VulkanRenderDevice.h>
#include <VulkanExtensions.h>
#include <Shader.h>
#include <UniformBuffer.h>
#include <MeshLoader.h>
#include <GraphicsPipeline.h>
#include <PipelineContext.h>
#include <Material.h>

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
    ID<IRenderDevice::VertexBuffer> vertexBuffer;
    ID<IRenderDevice::IndexBuffer> indexBuffer;
    uint32 indexCount = 0;
};

struct ShaderUniformBuffer {
    Mat4f model = Mat4f();
    Mat4f MVP = Mat4f();
};

struct MatData {
    RefCounted<Shader> shader;
    RefCounted<GraphicsPipeline> graphicsPipeline;
    RefCounted<Material> material;
    RefCounted<Texture> texture;
    RefCounted<Sampler> sampler;
    ShaderUniformBuffer data;
    RefCounted<Material> instanced;
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
        IRenderDevice::Color clearColor = { { 1.0f, 1.0f, 1.0f, 0.0f} };

        while (!glfwWindowShouldClose(window.glfwWindow)) {
            glfwPollEvents();
            glfwSwapBuffers(window.glfwWindow);
            glfwGetFramebufferSize(window.glfwWindow, &window.widthFrameBuffer, &window.heightFrameBuffer);

            IRenderDevice::Region area = { 0, 0, { (uint32)window.widthFrameBuffer, (uint32)window.heightFrameBuffer} };

            if (area.extent.x == 0|| area.extent.y == 0) {
                continue;
            }

            updateScene();

            pDevice->drawListBegin(); {
                pDevice->drawListBindSurface(surface, clearColor, area);
                PipelineContext::cacheSurfaceBinding(surface);

                matData.material->bindGraphicsPipeline();
                matData.material->bindUniformData();

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

        // supported texture data formats (for debug)
        for (auto format: pDevice->getSupportedTextureFormats()) {
            std::cout << (uint32) format;
        }

        // create surface
        surface = VulkanExtensions::createSurfaceGLFW(*pDevice, window.glfwWindow, window.widthFrameBuffer, window.heightFrameBuffer, name);
        // init vertex layout and load model
        initModel();
        // init material and its resources
        initMaterial();
        // first upadte
        updateScene();
        // instancing test
        matData.instanced = matData.material->clone();
        matData.material = matData.instanced;
    }

    void initWindow() {
        window.width = 1024;
        window.height = 720;

        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        window.glfwWindow = glfwCreateWindow(window.width, window.height, name.c_str(), nullptr, nullptr);
        glfwGetFramebufferSize(window.glfwWindow,(int*) &window.widthFrameBuffer, (int*) &window.heightFrameBuffer);

        // get required extensions for a surface
        window.extensions = glfwGetRequiredInstanceExtensions(&window.extensionsCount);

        glfwSetCursorPosCallback(window.glfwWindow, mouseCallback);
        glfwSetScrollCallback(window.glfwWindow, scrollCallback);
    }

    void initModel() {
        // init mesh vertex and index buffers
        loadObjModel(objMeshPath.c_str());
    }

    void loadObjModel(const char *path) {
        MeshLoader meshLoader(path);
        cmesh = meshLoader.importMesh(Mesh::VertexFormat::PNT);

        rmesh.vertexBuffer = pDevice->createVertexBuffer(BufferUsage::Dynamic,cmesh->getVertexCount() * sizeof(Vertex), cmesh->getVertexData());
        rmesh.indexCount = cmesh->getIndicesCount();
        rmesh.indexBuffer = pDevice->createIndexBuffer(BufferUsage::Static,rmesh.indexCount * sizeof(uint32), cmesh->getIndexData());
    }

    void initMaterial() {
        initShader();
        initGraphicsPipeline();
        initTextures();

        matData.material = std::make_shared<Material>(pDevice);
        matData.material->setGraphicsPipeline(matData.graphicsPipeline);
        matData.material->createMaterial();
        matData.material->setTexture2D("texSampler", matData.texture);
        matData.material->updateUniformData();
    }

    void initShader() {
        std::ifstream vertFile(MODEL3D_SHADER_PATH_VERT.c_str(), std::ios::binary);
        std::ifstream fragFile(MODEL3D_SHADER_PATH_FRAG.c_str(), std::ios::binary);

        std::vector<uint8> vertSpv(std::istreambuf_iterator<char>(vertFile), {});
        std::vector<uint8> fragSpv(std::istreambuf_iterator<char>(fragFile), {});

        matData.shader = std::make_shared<Shader>(pDevice);
        matData.shader->fromSources(ShaderLanguage::SPIRV, vertSpv, fragSpv);
        matData.shader->reflectData();
        matData.shader->generateUniformLayout();
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

        matData.sampler = std::make_shared<Sampler>(pDevice);
        matData.sampler->setHighQualityFiltering();

        matData.texture = std::make_shared<Texture>(pDevice);
        matData.texture->setDataAsRGBA8(texWidth, texHeight, pixels, true);
        matData.texture->setSampler(matData.sampler);

        stbi_image_free(pixels);
    }

    void initGraphicsPipeline() {
        IRenderDevice::VertexBufferLayoutDesc vertexBufferLayoutDesc = {};

        std::vector<IRenderDevice::VertexAttributeDesc> &attrs = vertexBufferLayoutDesc.attributes;
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

        std::shared_ptr<GraphicsPipeline> &pipeline = matData.graphicsPipeline;
        pipeline = std::make_shared<GraphicsPipeline>(pDevice);
        pipeline->setSurface(surface);
        pipeline->setShader(matData.shader);
        pipeline->setVertexBuffersCount(1);
        pipeline->setVertexBufferDesc(0, vertexBufferLayoutDesc);
        pipeline->setBlendEnable(false);
        pipeline->setDepthTestEnable(true);
        pipeline->setDepthWriteEnable(true);
        pipeline->createPipeline();
    }

    void destroy() {
        pDevice->destroyVertexBuffer(rmesh.vertexBuffer);
        pDevice->destroyIndexBuffer(rmesh.indexBuffer);

        ignimbrite::VulkanExtensions::destroySurface(*pDevice, surface);

        glfwDestroyWindow(window.glfwWindow);
        glfwTerminate();
    }


    void updateScene() {
        calculateMvp(window.widthFrameBuffer, window.heightFrameBuffer, fov, pitch, yaw, z, matData.data.model, matData.data.MVP);

        static String mvpName = "bufferVals.mvp";
        matData.material->setMat4(mvpName, matData.data.MVP);
        matData.material->updateUniformData();
    }

    static void calculateMvp(float32 viewWidth, float32 viewHeight,
                             float32 fovDegrees, float32 apitch, float32 ayaw, float32 cz,
                             Mat4f& outModel, Mat4f& outMvp) {
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

        outModel = model;
        outMvp = clip * projection * view * model;
    }

    static void mouseCallback(GLFWwindow *window, float64 x, float64 y) {
        const float32 sensitivity = 0.01f;

        if (glfwGetMouseButton(window, 0) == GLFW_PRESS) {
            yaw += (float32)x * sensitivity - prevx;
            pitch -= (float32)y * sensitivity - prevy;
        }

        prevx = (float32)x * sensitivity;
        prevy = (float32)y * sensitivity;
    }

    static void scrollCallback(GLFWwindow *, float64 x, float64 y) {
        z += (float32)y;

        // Clamp to prevent reverse scroll
        if (z < 5.0f)
            z = 5.0f;
    }

private:

    std::shared_ptr<VulkanRenderDevice> pDevice;
    ID<IRenderDevice::Surface> surface;
    Window           window;
    RefCounted<Mesh> cmesh;
    RenderableMesh   rmesh;
    MatData          matData;

    String name = "Textured 3D model";
    String objMeshPath;
    String texturePath;

    String MODEL3D_SHADER_PATH_VERT = "shaders/spirv/vert3d.spv";
    String MODEL3D_SHADER_PATH_FRAG = "shaders/spirv/frag3d.spv";

    static float32 pitch;
    static float32 yaw;
    static float32 fov;
    static float32 z;
    static float32 prevx;
    static float32 prevy;
};

float32 Vulkan3DTest::pitch = 0;
float32 Vulkan3DTest::yaw = 0;
float32 Vulkan3DTest::fov = 70;
float32 Vulkan3DTest::z = 40;
float32 Vulkan3DTest::prevx = 0;
float32 Vulkan3DTest::prevy = 0;

int main(int argc, char **argv) {
    const char *mesh = "assets/modspvels/sphere.obj";
    const char *texture = "assets/textures/double.png";

    if (argc >= 3) {
        mesh = argv[1];
        texture = argv[2];
    }
    
    Vulkan3DTest test(mesh, texture);
    test.loop();
}

#endif //IGNIMBRITE_VULKAN3DTEST_H
