//
// Created by Egor Orachyov on 2019-11-02.
//

#ifndef VULKANRENDERER_VULKANRENDERDEVICE_H
#define VULKANRENDERER_VULKANRENDERDEVICE_H

#include <include/renderer/RenderDevice.h>
#include <include/renderer/ObjectIDBuffer.h>
#include "VulkanContext.h"

/** Vulkan implementation for Render Device interface */
class VulkanRenderDevice : public RenderDevice {
public:

    VulkanRenderDevice(uint32 extensionsCount, const char* const* extensions);
    ~VulkanRenderDevice() override = default;

    ID createVertexLayout(const std::vector<VertexBufferLayoutDesc> &vertexBuffersDesc) override;
    void destroyVertexLayout(ID layout) override;

    ID createVertexBuffer(BufferUsage usage, uint32 size, const void *data) override;
    void updateVertexBuffer(ID buffer, uint32 size, uint32 offset, const void *data) override;
    void destroyVertexBuffer(ID buffer) override;

    ID createIndexBuffer(BufferUsage usage, uint32 size, const void *data) override;
    void updateIndexBuffer(ID buffer, uint32 size, uint32 offset, const void *data) override;
    void destroyIndexBuffer(ID buffer) override;

    ID createSampler(const SamplerDesc &samplerDesc) override;
    void destroySampler(ID sampler) override;

    ID createTexture(const TextureDesc &textureDesc) override;
    void destroyTexture(ID texture) override;

    ID getSurface(const std::string &surfaceName) override;
    void getSurfaceSize(ID surface, uint32 &width, uint32 &height) override;

private:

    friend class VulkanExtensions;

    struct VertexLayout {
        std::vector<VkVertexInputBindingDescription> vkBindings;
        std::vector<VkVertexInputAttributeDescription> vkAttributes;
    };

    struct VertexBuffer {
        BufferUsage usage;
        uint32 size;
        VkBuffer vkBuffer;
        VkDeviceMemory vkDeviceMemory;
    };

    struct IndexBuffer {
        BufferUsage usage;
        uint32 size;
        VkBuffer vkBuffer;
        VkDeviceMemory vkDeviceMemory;
    };

    struct ImageObject {
        VkImage image;
        VkDeviceMemory imageMemory;
        VkImageView imageView;
    };

    struct Window {
        uint32 width;
        uint32 height;
        uint32 widthFramebuffer;
        uint32 heightFramebuffer;
        std::string name;
        VkSurfaceKHR surface;
    };

    VulkanContext context;

    ObjectIDBuffer<Window> mWindows;
    ObjectIDBuffer<VertexLayout> mVertexLayouts;
    ObjectIDBuffer<VertexBuffer> mVertexBuffers;
    ObjectIDBuffer<IndexBuffer> mIndexBuffers;
    ObjectIDBuffer<VkSampler> mSamplers;
    ObjectIDBuffer<ImageObject> mImageObjects;
};

#endif //VULKANRENDERER_VULKANRENDERDEVICE_H