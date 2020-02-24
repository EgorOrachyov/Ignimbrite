/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov, Sultim Tsyrendashiev                  */
/**********************************************************************************/

#ifndef IGNIMBRITE_RENDERDEVICE_H
#define IGNIMBRITE_RENDERDEVICE_H

#include <ignimbrite/ObjectID.h>
#include <ignimbrite/RenderDeviceDefinitions.h>
#include <string>
#include <vector>

namespace ignimbrite {

    /**
     * @brief Rendering device interface
     *
     * Wrapper for third-party drawing API, such as Vulkan, OpenGL, DirecX.
     *
     * All the objects, created via this interface must be referenced via ID.
     * After usage you have to explicitly destroy each object in the correct (reverse) order.
     *
     * Some objects requires additional meta-data to be created. This structures
     * called <Some Name>Desc. Suffix 'Desc' used to mark that class of meta-structures.
     *
     * If you add your own object and meta-structures, please,
     * follow the above mentioned notation.
     */
    class RenderDevice {
    public:

        class VertexLayout;
        class VertexBuffer;
        class IndexBuffer;
        class UniformBuffer;
        class UniformLayout;
        class UniformSet;
        class ShaderProgram;
        class GraphicsPipeline;
        class FramebufferFormat;
        class Framebuffer;
        class Surface;
        class Texture;
        class Sampler;

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
        virtual ID<VertexLayout> createVertexLayout(const std::vector<VertexBufferLayoutDesc> &vertexBuffersDesc) = 0;

        virtual void destroyVertexLayout(ID<VertexLayout> layout) = 0;

        virtual ID<VertexBuffer> createVertexBuffer(BufferUsage usage, uint32 size, const void *data) = 0;

        virtual void updateVertexBuffer(ID<VertexBuffer> buffer, uint32 size, uint32 offset, const void *data) = 0;

        virtual void destroyVertexBuffer(ID<VertexBuffer> buffer) = 0;

        virtual ID<IndexBuffer> createIndexBuffer(BufferUsage usage, uint32 size, const void *data) = 0;

        virtual void updateIndexBuffer(ID<IndexBuffer> buffer, uint32 size, uint32 offset, const void *data) = 0;

        virtual void destroyIndexBuffer(ID<IndexBuffer> buffer) = 0;

        struct UniformTextureDesc {
            /** Where this texture will be used */
            ShaderStageFlags stageFlags = 0;
            /** Binding of the texture in the shader */
            uint32 binding = -1;
            /** Actual texture with data */
            ID<Texture> texture;
            /** Specific sampler for data access in the shader */
            ID<Sampler> sampler;
        };

        struct UniformBufferDesc {
            /** Binding point in target shader */
            uint32 binding = -1;
            /** Offset from the buffer where data starts */
            uint32 offset = 0;
            /** Actual data range to map into shader uniform buffer */
            uint32 range = 0;
            /** Uniform buffer with actual data */
            ID<UniformBuffer> buffer;
        };

        struct UniformSetDesc {
            std::vector<UniformTextureDesc> textures;
            std::vector<UniformBufferDesc> buffers;
        };


        virtual ID<UniformSet> createUniformSet(const UniformSetDesc &setDesc, ID<UniformLayout> uniformLayout) = 0;

        virtual void destroyUniformSet(ID<UniformSet> set) = 0;

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

        virtual ID<UniformLayout> createUniformLayout(const UniformLayoutDesc &layoutDesc) = 0;

        virtual void destroyUniformLayout(ID<UniformLayout> layout) = 0;

        virtual ID<UniformBuffer> createUniformBuffer(BufferUsage usage, uint32 size, const void *data) = 0;

        virtual void updateUniformBuffer(ID<UniformBuffer> buffer, uint32 size, uint32 offset, const void *data) = 0;

        virtual void destroyUniformBuffer(ID<UniformBuffer> buffer) = 0;

        struct SamplerDesc {
            SamplerFilter min = SamplerFilter::Nearest;
            SamplerFilter mag = SamplerFilter::Nearest;
            SamplerRepeatMode u = SamplerRepeatMode::ClampToEdge;
            SamplerRepeatMode v = SamplerRepeatMode::ClampToEdge;
            SamplerRepeatMode w = SamplerRepeatMode::ClampToEdge;
            SamplerBorderColor color = SamplerBorderColor::Black;
            bool useAnisotropy = false;
            float32 anisotropyMax = 1.0f;
            float32 minLod = 0.0f;
            float32 maxLod = 0.0f;
            SamplerFilter mipmapMode = SamplerFilter::Linear;
            float32 mipLodBias = 0.0f;
        };

        virtual ID<Sampler> createSampler(const SamplerDesc &samplerDesc) = 0;

        virtual void destroySampler(ID<Sampler> sampler) = 0;

        struct TextureDesc {
            TextureType type = TextureType::Texture2D;
            DataFormat format = DataFormat::R8G8B8A8_UNORM;
            TextureUsageFlags usageFlags = 0;
            uint32 mipmaps = 1;
            uint32 width = 0;
            uint32 height = 0;
            uint32 depth = 1;
            uint32 size = 0;
            void *data = nullptr;
            uint32 dataSize = 0;
        };

        virtual ID<Texture> createTexture(const TextureDesc &textureDesc) = 0;

        virtual void destroyTexture(ID<Texture> texture) = 0;

        struct ShaderDesc {
            ShaderType type;
            std::vector<uint8> source;
        };

        struct ProgramDesc {
            ShaderLanguage language;
            std::vector<ShaderDesc> shaders;
        };

        virtual ID<ShaderProgram> createShaderProgram(const ProgramDesc &programDesc) = 0;

        virtual void destroyShaderProgram(ID<ShaderProgram> program) = 0;

        struct FramebufferAttachmentDesc {
            AttachmentType type = AttachmentType::Color;
            DataFormat format = DataFormat::R8G8B8A8_UNORM;
            TextureSamples samples = TextureSamples::Samples1;
        };

        virtual ID<FramebufferFormat> createFramebufferFormat(const std::vector<FramebufferAttachmentDesc> &attachments) = 0;

        virtual void destroyFramebufferFormat(ID<FramebufferFormat> framebufferFormat) = 0;

        virtual ID<Framebuffer> createFramebuffer(const std::vector<ID<Texture>> &attachments, ID<FramebufferFormat> framebufferFormat) = 0;

        virtual void destroyFramebuffer(ID<Framebuffer> framebuffer) = 0;

        struct PipelineRasterizationDesc {
            PolygonMode mode = PolygonMode::Fill;
            PolygonCullMode cullMode = PolygonCullMode::Back;
            PolygonFrontFace frontFace = PolygonFrontFace::FrontCounterClockwise;
            float32 lineWidth = 1.0f;
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
            float blendConstants[4] = {0.0f, 0.0f, 0.0f, 0.0f};
            std::vector<BlendAttachmentDesc> attachments;
        };

        struct PipelineSurfaceBlendStateDesc {
            bool logicOpEnable = false;
            LogicOperation logicOp = LogicOperation::Copy;
            float blendConstants[4] = {0.0f, 0.0f, 0.0f, 0.0f};
            BlendAttachmentDesc attachment;
        };

        struct StencilOpStateDesc {
            StencilOperation failOp = StencilOperation::Keep;
            StencilOperation passOp = StencilOperation::Keep;
            StencilOperation depthFailOp = StencilOperation::Keep;
            CompareOperation compareOp = CompareOperation::Always;
            uint32 compareMask = 0x0;
            uint32 writeMask = 0x0;
            uint32 reference = 0x0;
        };

        struct PipelineDepthStencilStateDesc {
            bool depthTestEnable = false;
            bool depthWriteEnable = false;
            CompareOperation depthCompareOp = CompareOperation::Less;
            bool stencilTestEnable = false;
            /** processing rasterized fragments from points, lines and front-facing polygons */
            StencilOpStateDesc front;
            /** processing rasterized fragments from back-facing polygons */
            StencilOpStateDesc back;
        };

        virtual ID<GraphicsPipeline> createGraphicsPipeline(PrimitiveTopology topology,
                                          ID<ShaderProgram> program,
                                          ID<VertexLayout> vertexLayout,
                                          ID<UniformLayout> uniformLayout,
                                          ID<FramebufferFormat> framebufferFormat,
                                          const PipelineRasterizationDesc &rasterizationDesc,
                                          const PipelineBlendStateDesc &blendStateDesc,
                                          const PipelineDepthStencilStateDesc &depthStencilStateDesc) = 0;

        /**
         * @brief Creates graphics pipeline
         *
         * Creates graphics pipeline for specified surface with
         * predefined internal frame buffer format.
         *
         * Supports only single color attachment, therefore fragment shader must write result
         * color value only to single out variable with location 0.
         *
         * Supports depth and stencil buffering.
         *
         * @param surface ID of the target surface for rendering via this pipeline
         * @param topology Rendered primitives topology
         * @param program ID of shader program to be executed in this pipeline
         * @param vertexLayout ID of vertex layout, which describes how vertices data will be passed into shader
         * @param uniformLayout ID of uniform layout, which describes CPU -> shader communication format
         * @param rasterizationDesc Primitives rasterization state descriptor
         * @param blendStateDesc Blending descriptor for single surface color attachment
         * @param depthStateDesc State describes stencil and depth operations
         *
         * @return ID of the created graphics pipeline
         */
        virtual ID<GraphicsPipeline> createGraphicsPipeline(ID<Surface> surface,
                                          PrimitiveTopology topology,
                                          ID<ShaderProgram> program,
                                          ID<VertexLayout> vertexLayout,
                                          ID<UniformLayout> uniformLayout,
                                          const PipelineRasterizationDesc &rasterizationDesc,
                                          const PipelineSurfaceBlendStateDesc &blendStateDesc,
                                          const PipelineDepthStencilStateDesc &depthStateDesc) = 0;

        /**
         * @brief Destroys graphics pipeline
         * @error Does not allows to destroy object, if other objects depend on that or have some references to that
         * @param pipeline ID of the pipeline to be destroyed
         */
        virtual void destroyGraphicsPipeline(ID<GraphicsPipeline> pipeline) = 0;

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

        /**
         * @brief Begin draw list
         *
         * Single time submit draw list. Requires drawListEnd() to be called.
         * Between this functions' calls allowed only commands with drawList prefix.
         *
         * @note Order of commands execution inside single draw list IS SYNCHRONIZED.
         * @note Order of different draw list execution IS NOT SYNCHRONIZED.
         */
        virtual void drawListBegin() = 0;

        /**
         * @brief End draw list
         *
         * Finish draw list commands setup. Submits draw list on GPU for
         * rendering and waits, until draw list is executed.
         *
         * @note Ended draw list won't be executed until flush() called,
         *       followed by explicit synchronize() call.
         */
        virtual void drawListEnd() = 0;

        virtual void drawListBindSurface(ID<Surface> surface, const Color &color, const Region &area) = 0;

        virtual void drawListBindFramebuffer(ID<Framebuffer> framebuffer,
                                             const std::vector<Color> &colors,
                                             const Region &area) = 0;

        virtual void drawListBindFramebuffer(ID<Framebuffer> framebuffer,
                                             const std::vector<Color> &colors,
                                             float32 depth, uint32 stencil,
                                             const Region &area) = 0;

        virtual void drawListBindPipeline(ID<GraphicsPipeline> graphicsPipeline) = 0;

        virtual void drawListBindUniformSet(ID<UniformSet> uniformSet) = 0;

        virtual void drawListBindVertexBuffer(ID<VertexBuffer> vertexBuffer, uint32 binding, uint32 offset) = 0;

        virtual void drawListBindIndexBuffer(ID<IndexBuffer> indexBuffer, IndicesType indicesType, uint32 offset) = 0;

        virtual void drawListDraw(uint32 verticesCount, uint32 instancesCount) = 0;

        virtual void drawListDrawIndexed(uint32 indicesCount, uint32 instancesCount) = 0;

        /**
         * @brief Get surface id
         *
         * Allows to get surface ID via name for specific window rendering.
         * All the application windows are created by target window
         * manager (GLFW, QT)
         *
         * @throw Exception if surface with specified name not found
         * @return ID of the surface if found
         */
        virtual ID<Surface> getSurface(const std::string &surfaceName) = 0;

        /**
         * @brief Get surface size
         *
         * @param surface ID of surface to get size
         * @param width[out] Surface framebuffer width
         * @param height[out] Surface framebuffer height
         */
        virtual void getSurfaceSize(ID<Surface> surface, uint32 &width, uint32 &height) = 0;

        /**
         * @brief Swap buffers
         *
         * Swap buffers for specified surface to present final image on the screen.
         *
         * Render API primary uses double-buffering present mode. This function allows to submit
         * all the currently filled command buffers for rendering in specified surface
         * and wait, until previous submit session is completed.
         *
         * @note Before swap buffer all the draw lists must be executed.
         *       To ensure, that all the draw lists executed call synchronize() method.
         *
         * @param surface ID of the surface to swap buffers to present final image
         */
        virtual void swapBuffers(ID<Surface> surface) = 0;

        /**
         * @brief Flushes draw lists
         *
         * Flush all the created draw lists from previous drawListFlush() call.
         * Immediately submits all the draw lists to be executed on GPU.
         *
         * @note Order of the execution of draw lists is not specified.
         *
         * @note After this stage host CPU and GPU are no synchronized.
         *       To change state of render device object you must call hostSynchronize() function.
         */
        virtual void flush() = 0;

        /**
         * @brief CPU and GPU synchronization
         *
         * Synchronize CPU host program with the GPU. Wait for all the
         * flush requests to be finished. After than function call all
         * the render device objects could be safely modified.
         *
         * @note Before swap buffer all the draw lists must be executed.
         *       To ensure, that all the draw lists executed call synchronize() method.
         */
        virtual void synchronize() = 0;

        /** @return Readable hardware API name */
        virtual const std::string &getDeviceName() const;

        /** @return Video card vendor name */
        virtual const std::string &getVendorName() const;
    };

} // namespace ignimbrite

#endif //IGNIMBRITE_RENDERDEVICE_H
