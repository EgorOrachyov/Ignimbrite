//
// Created by Egor Orachyov on 2019-11-02.
//

#include "include/VulkanRenderDevice.h"
#include "include/VulkanDefinitions.h"
#include <vulkan/vulkan.h>
#include <exception>

RenderDevice::ID VulkanRenderDevice::createVertexLayout(const std::vector<VertexBufferLayoutDesc> &vertexBuffersDesc) {
    VertexLayoutBatch batch;

    auto& vertBindings = batch.vertBindings;
    auto& vertAttributes = batch.vertAttributes;

    for (size_t i = 0; i < vertexBuffersDesc.size(); i++) {
        const VertexBufferLayoutDesc& desc = vertexBuffersDesc[i];

        VkVertexInputBindingDescription bindingDesc;
        bindingDesc.binding = (uint32_t)i;
        bindingDesc.inputRate = VulkanDefinitions::vertexInputRate(desc.usage);
        bindingDesc.stride = (uint32_t)desc.stride;

        vertBindings.push_back(bindingDesc);

        for (size_t j = 0; j < desc.attributes.size(); j++)
        {
            const VertexAttributeDesc &attr = desc.attributes[j];

            VkVertexInputAttributeDescription attrDesc;
            attrDesc.binding = bindingDesc.binding;
            attrDesc.format = VulkanDefinitions::dataFormat(attr.format);
            attrDesc.location = attr.location;
            attrDesc.offset = attr.offset;

            vertAttributes.push_back(attrDesc);
        }
    }

    return vertexLayoutBatches.move(batch);
}

void VulkanRenderDevice::destroyVertexLayout(RenderDevice::ID layout) {
    vertexLayoutBatches.remove(layout);
}

RenderDevice::ID VulkanRenderDevice::createVertexBuffer(BufferUsage type, uint32 size, const void *data) {
    VulkanContext::BufferObject vertexBufferObj = {};
    context.createBufferObject(type, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, size, data, vertexBufferObj);

    return vertexBuffers.move(vertexBufferObj);
}

RenderDevice::ID VulkanRenderDevice::createIndexBuffer(BufferUsage type, uint32 size, const void *data) {
    VulkanContext::BufferObject indexBufferObj = {};
    context.createBufferObject(type, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, size, data, indexBufferObj);

    return indexBuffers.move(indexBufferObj);
}

void VulkanRenderDevice::updateVertexBuffer(RenderDevice::ID bufferId, uint32 size, uint32 offset, const void *data) {
    const VkDeviceMemory &bufferMemory = vertexBuffers.get(bufferId).memory;
    context.updateBufferMemory(bufferMemory, data, size, offset);
}

void VulkanRenderDevice::updateIndexBuffer(RenderDevice::ID bufferId, uint32 size, uint32 offset, const void *data) {
    const VkDeviceMemory &bufferMemory = indexBuffers.get(bufferId).memory;
    context.updateBufferMemory(bufferMemory, data, size, offset);
}

void VulkanRenderDevice::destroyVertexBuffer(RenderDevice::ID bufferId) {
    const VkDevice &device = context.getDevice();
    VulkanContext::BufferObject *vb = vertexBuffers.getPtr(bufferId);

    vkDestroyBuffer(device, vb->buffer, nullptr);
    vkFreeMemory(device, vb->memory, nullptr);

    vertexBuffers.remove(bufferId);
}

void VulkanRenderDevice::destroyIndexBuffer(RenderDevice::ID bufferId) {
    const VkDevice &device = context.getDevice();
    VulkanContext::BufferObject *ib = indexBuffers.getPtr(bufferId);

    vkDestroyBuffer(device, ib->buffer, nullptr);
    vkFreeMemory(device, ib->memory, nullptr);

    indexBuffers.remove(bufferId);
}
