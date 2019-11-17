//
// Created by Egor Orachyov on 2019-10-14.
//

#ifndef VULKANRENDERER_RENDERDEVICE_H
#define VULKANRENDERER_RENDERDEVICE_H

#include <renderer/ObjectID.h>
#include <renderer/DeviceDefinitions.h>
#include <string>
#include <vector>

/**
 * @brief Rendering device interface.
 * Wrapper for third-party drawing API, such as Vulkan, OpenGL etc.
 *
 * All the objects, created via this interface must be referenced via ID.
 * After usage you have to explicitly destroy each object.
 *
 * Some objects requires additional meta-data to be created. This structures
 * called <Some Name>Desc. Suffix 'Desc' used to mark that class of meta-structures.
 *
 * If you add your own object and meta-structures, please,
 * follow the above mentioned techniques.
 */
class RenderDevice {
public:
    typedef ObjectID ID;
    static const ID INVALID;

    virtual ~RenderDevice() = default;

    /** Single vertex shader input value description */
    struct VertexAttributeDesc {
        /** Shader in location */
        uint32 location;
        /** Offset from stride beginning */
        uint32 offset;
        /** Format of the value in the shader */
        DataFormat format;
    };

    /** Single vertex buffer descriptor */
    struct VertexBufferLayoutDesc {
        /** Size of the stride (step) fro single vertex */
        uint32 stride;
        /** Interate per instance/vertex */
        VertexUsage usage;
        /** Attributes, updated from that vertex buffer */
        std::vector<VertexAttributeDesc> attributes;
    };

    /**
     * Layout for all vertex buffers, bound to vertex shader.
     * @note Each buffer automatically will get binding num as index in that array
     */
    virtual ID createVertexLayout(const std::vector<VertexBufferLayoutDesc> &vertexBuffersDesc) = 0;
    virtual void destroyVertexLayout(ID layout) = 0;

    virtual ID createVertexBuffer(BufferUsage usage, uint32 size, const void* data) = 0;
    virtual void updateVertexBuffer(ID buffer, uint32 size, uint32 offset, const void* data) = 0;
    virtual void destroyVertexBuffer(ID buffer) = 0;

    virtual ID createIndexBuffer(BufferUsage usage, uint32 size, const void* data) = 0;
    virtual void updateIndexBuffer(ID buffer, uint32 size, uint32 offset, const void* data) = 0;
    virtual void destroyIndexBuffer(ID buffer) = 0;

    struct UniformTextureDesc {
        /** Where this texture will be used */
        ShaderStageFlags stageFlags;
        /** Binding of the texture in the shader */
        uint32 binding = -1;
        /** Actual texture with data */
        ID texture = INVALID;
        /** Specific sampler for data access in the shader */
        ID sampler = INVALID;
    };

    struct UniformBufferDesc {
        /** Binding point in target shader */
        uint32 binding = -1;
        /** Offset from the buffer where data starts */
        uint32 offset = 0;
        /** Actual data range to map into shader uniform buffer */
        uint32 range = 0;
        /** Uniform buffer with actual data */
        ID buffer = INVALID;
    };

    struct UniformSetDesc {
        std::vector<UniformTextureDesc> textures;
        std::vector<UniformBufferDesc> buffers;
    };


    virtual ID createUniformSet(const UniformSetDesc &setDesc, ID uniformLayout) = 0;
    virtual void destroyUniformSet(ID set) = 0;

    struct UniformLayoutBufferDesc {
        /** Shader stages, which uses this uniform buffer */
        ShaderStageFlags flags = 0x0;
        /** Binding point in target shader */
        uint32 binding = -1;
    };

    struct UniformLayoutTextureDesc {
        /** Shader stages, which uses this uniform buffer */
        ShaderStageFlags flags = 0x0;
        /** Binding point in target shader */
        uint32 binding = -1;
    };

    struct UniformLayoutDesc {
        std::vector<UniformLayoutTextureDesc> textures;
        std::vector<UniformLayoutBufferDesc> buffers;
    };

    virtual ID createUniformLayout(const UniformLayoutDesc& layoutDesc) = 0;
    virtual void destroyUniformLayout(ID layout) = 0;

    virtual ID createUniformBuffer(BufferUsage usage, uint32 size, const void* data) = 0;
    virtual void updateUniformBuffer(ID buffer, uint32 size, uint32 offset, const void* data) = 0;
    virtual void destroyUniformBuffer(ID buffer) = 0;

    struct SamplerDesc {
        SamplerFilter min = SamplerFilter::Nearest;
        SamplerFilter mag = SamplerFilter::Nearest;
        SamplerRepeatMode u = SamplerRepeatMode::ClampToEdge;
        SamplerRepeatMode v = SamplerRepeatMode::ClampToEdge;
        SamplerRepeatMode w =  SamplerRepeatMode::ClampToEdge;
        SamplerBorderColor color = SamplerBorderColor::Black;
        bool useAnisotropy = false;
        float32 anisotropyMax = 1.0f;
        float32 minLod = 0.0f;
        float32 maxLod = 0.0f;
        SamplerFilter mipmapMode = SamplerFilter::Linear;
        float32 mipLodBias = 0.0f;
    };

    virtual ID createSampler(const SamplerDesc& samplerDesc) = 0;
    virtual void destroySampler(ID sampler) = 0;

    struct TextureDesc {
        TextureType type = TextureType::Texture2D;
        DataFormat format = DataFormat::R8G8B8A8_UNORM;
        uint32 mipmaps = 1;
        uint32 width = 0;
        uint32 height = 0;
        uint32 depth = 1;
        uint32 usageFlags = 0;
        void* data = nullptr;
    };

    virtual ID createTexture(const TextureDesc& textureDesc) = 0;
    virtual void destroyTexture(ID texture) = 0;

    struct ShaderDataDesc {
        ShaderType type;
        ShaderLanguage language;
        std::vector<uint8> source;
    };

    virtual ID createShaderProgram(const std::vector<ShaderDataDesc> &shaders) = 0;
    virtual void destroyShaderProgram(ID program) = 0;

    struct FramebufferAttachmentDesc {
        AttachmentType type = AttachmentType::Color;
        DataFormat format = DataFormat::R8G8B8A8_UNORM;
        TextureSamples samples = TextureSamples::Samples1;
    };

    virtual ID createFramebufferFormat(const std::vector<FramebufferAttachmentDesc> &attachments) = 0;
    virtual void destroyFramebufferFormat(ID framebufferFormat) = 0;

    virtual ID createFramebuffer(const std::vector<ID> &attachments, ID framebufferFormat) = 0;
    virtual void destroyFramebuffer(ID framebuffer) = 0;

    struct PipelineRasterizationDesc {
        PolygonMode mode;
        PolygonCullMode cullMode;
        PolygonFrontFace frontFace;
        float32 lineWidth;
    };

    /**
     * Blend settings for single framebuffer attachment
     * The following equation is used to to compute blend color:
     *
     * if (blendEnable) {
     *  finalColor.rgb = (srcColorBlendFactor * newColor.rgb)
     *      <colorBlendOp> (dstColorBlendFactor * oldColor.rgb);
     *  finalColor.a = (srcAlphaBlendFactor * newColor.a) <
     *      alphaBlendOp> (dstAlphaBlendFactor * oldColor.a);
     * } else {
     *  color = newColor;
     * }
     * finalColor = finalColor & colorWriteMask;
     */
    struct BlendAttachmentDesc {
        bool blendEnable = false;
        /** Operation on color rgb components */
        BlendFactor srcColorBlendFactor = BlendFactor::One;
        BlendFactor dstColorBlendFactor = BlendFactor::Zero;
        BlendOperation colorBlendOp = BlendOperation::Add;
        /** Operation on color a component */
        BlendFactor srcAlphaBlendFactor = BlendFactor::One;
        BlendFactor dstAlphaBlendFactor = BlendFactor::Zero;
        BlendOperation alphaBlendOp = BlendOperation::Add;
        bool writeR = true;
        bool writeG = true;
        bool writeB = true;
        bool writeA = true;
    };

    struct PipelineBlendStateDesc {
        bool logicOpEnable = false;
        LogicOperation logicOp = LogicOperation::Copy;
        float blendConstants[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        std::vector<BlendAttachmentDesc> attachments;
    };

    struct StencilOpState {
        StencilOperation failOp;
        StencilOperation passOp;
        StencilOperation depthFailOp;
        StencilOperation compareOp;
        uint32 compareMask;
        uint32 writeMask;
        uint32 reference;
    };

    struct PipelineDepthStencilStateDesc {
        bool depthTestEnable;
        CompareOperation depthCompareOp;
        bool stencilTestEnable;
        /** processing rasterized fragments from points, lines and front-facing polygons */
        StencilOpState front;
        /** processing rasterized fragments from back-facing polygons */
        StencilOpState back;
    };

    virtual ID createGraphicsPipeline(PrimitiveTopology topology,
                                      ID program, ID vertexLayout, ID uniformLayout, ID framebufferFormat,
                                      const PipelineRasterizationDesc &rasterizationDesc,
                                      const PipelineBlendStateDesc &blendStateDesc,
                                      const PipelineDepthStencilStateDesc &depthStencilStateDesc) = 0;
    virtual void destroyGraphicsPipeline(ID pipeline) = 0;

    struct Color {
        float32 components[4];
    };

    struct Extent {
        uint32 x;
        uint32 y;
    };

    struct Region {
        uint32 xOffset;
        uint32 yOffset;
        Extent extent;
    };

    virtual ID drawListBegin(ID framebuffer,
                             std::vector<Color> clearColors,
                             const Region& drawArea) = 0;
    virtual ID drawListBegin(ID framebuffer,
                             std::vector<Color> clearColors,
                             float32 clearDepth,
                             uint32 clearStencil,
                             const Region& drawArea) = 0;
    virtual ID drawListBegin(ID surface,
                             Color clearColor,
                             const Region& drawArea) = 0;
    virtual ID drawListBegin(ID surface,
                             Color clearColor,
                             float32 clearDepth,
                             uint32 clearStencil,
                             const Region& drawArea) = 0;

    virtual void drawListBindPipeline(ID drawList, ID graphicsPipeline) = 0;
    virtual void drawListBindUniformSet(ID drawList, ID uniformLayout) = 0;
    virtual void drawListBindVertexBuffer(ID drawList, ID vertexBuffer, uint32 binding, uint32 offset) = 0;
    virtual void drawListBindIndexBuffer(ID drawList, ID indexBuffer, IndicesType indicesType, uint32 offset) = 0;

    virtual void drawListDraw(ID drawList, uint32 verticesCount, uint32 instancesCount) = 0;
    virtual void drawListDrawIndexed(ID drawList, uint32 indicesCount, uint32 instancesCount) = 0;

    virtual void drawListEnd(ID drawList) = 0;

    /**
     * @brief Return ID of surface with specified name
     *
     * Allows to get surface ID for specific window rendering.
     * All the application windows are created by target window
     * manager (GLFW, QT)
     *
     * @throw Exception if surface with specified name not found
     * @return ID of the surface if found
     */
    virtual ID getSurface(const std::string& surfaceName) = 0;
    virtual void getSurfaceSize(ID surface, uint32 &width, uint32 &height) = 0;

    /**
     * @brief Swap buffers to present images for all windows
     *
     * Render API primary uses double-buffering present mode.
     * This function allows to submit all the currently filled
     * command buffers for rendering and wait, until
     * previous submit session is completed.
     */
    virtual void swapBuffers() = 0;

    /** @return Readable hardware API name */
    virtual const std::string& getDeviceName() const;
    /** @return Video card vendor name */
    virtual const std::string& getVendorName() const;
};

#endif //VULKANRENDERER_RENDERDEVICE_H
