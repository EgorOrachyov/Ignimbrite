//
// Created by Egor Orachyov on 2019-10-14.
//

#ifndef VULKANRENDERER_RENDERDEVICE_H
#define VULKANRENDERER_RENDERDEVICE_H

#include <ObjectID.h>
#include <DeviceDefinitions.h>
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

    virtual ID createVertexBuffer(BufferUsage usage, uint32 size, const void* data) = 0;
    virtual void updateVertexBuffer(ID buffer, uint32 size, uint32 offset, const void* data) = 0;
    virtual void destroyVertexBuffer(ID buffer) = 0;

    virtual ID createIndexBuffer(BufferUsage usage, uint32 size, const void* data) = 0;
    virtual void updateIndexBuffer(ID buffer, uint32 size, uint32 offset, const void* data) = 0;
    virtual void destroyIndexBuffer(ID buffer) = 0;

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

    struct AttachmentDesc {
        ID texture = INVALID;
        DataFormat format;
        TextureSamples samples;
    };

    virtual ID createFramebuffer(const std::vector<AttachmentDesc> &attachments) = 0;
    virtual void destroyFramebuffer(ID framebuffer) = 0;

    /** @return Readable hardware API name */
    virtual const std::string& getDeviceName() const;
    /** @return Video card vendor name */
    virtual const std::string& getVendor() const;

};

#endif //VULKANRENDERER_RENDERDEVICE_H
