/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                        */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#ifndef IGNIMBRITE_VULKANUTILS_H
#define IGNIMBRITE_VULKANUTILS_H

#include <VulkanObjects.h>
#include <VulkanDefinitions.h>

namespace ignimbrite {

    class VulkanUtils {
    public:

        static VkFormatProperties getDeviceFormatProperties(
                VkFormat format
        );

        static VkFormat findSupportedFormat(
                const VkFormat *candidates,
                uint32 candidatesCount,
                VkImageTiling tiling,
                VkFormatFeatureFlags features
        );

        static uint32 getMemoryTypeIndex(
                uint32 memoryTypeBits,
                VkFlags requirementsMask
        );

        static void createBuffer(
                VkDeviceSize size,
                VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                VkBuffer &outBuffer, VulkanAllocation &outAllocation
        );

        static void createBufferLocal(
                const void *data,
                VkDeviceSize size, VkBufferUsageFlags usage,
                VkBuffer &outBuffer, VulkanAllocation &outAllocation
        );

        static void copyBuffer(
                VkBuffer srcBuffer,
                VkBuffer dstBuffer,
                const VkBufferCopy *copyRegion
        );

        static void updateBufferMemory(
                const VulkanAllocation &allocation,
                VkDeviceSize offset, VkDeviceSize size,
                const void *data
        );

        static void createTextureImage(
                const void *imageData, uint32 imageDataSize,
                uint32 width, uint32 height,
                uint32 depth, uint32 mipLevels,
                VkImageType imageType, VkFormat format, VkImageTiling tiling,
                VkImage &outTextureImage, VulkanAllocation &outAllocation,
                VkImageLayout textureLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );

        static void createImage(
                uint32 width, uint32 height,
                uint32 depth, uint32 mipLevels,
                VkImageType imageType, VkFormat format,
                VkImageTiling tiling, VkImageUsageFlags usage,
                VkMemoryPropertyFlags properties,
                VkImage &outImage, VulkanAllocation &outAllocation
        );

        static void copyBufferToImage(
                VkBuffer buffer,
                VkImage image,
                uint32 width, uint32 height, uint32 depth
        );

        static void transitionImageLayout(
                VkImage image,
                VkImageLayout oldLayout,
                VkImageLayout newLayout,
                uint32 mipLevels
        );

        static void createImageView(
                VkImageView &outImageView,
                VkImage image,
                VkImageViewType viewType, VkFormat format,
                const VkImageSubresourceRange &subResourceRange,
                VkComponentMapping components = {}
        );

        static void generateMipmaps(
                VkImage image, VkFormat format,
                uint32 width, uint32 height,
                uint32 mipLevels,
                VkImageLayout newLayout
        );

        static void createDepthStencilBuffer(
                uint32 width, uint32 height, uint32 depth,
                VkImageType imageType, VkFormat format, VkImage &outImage,
                VulkanAllocation &outAllocation, VkImageUsageFlags usageFlags
        );

        static void createVertexInputState(
                const VulkanVertexLayout &layout,
                VkPipelineVertexInputStateCreateInfo &state
        );

        static void createInputAssembly(
                PrimitiveTopology topology,
                VkPipelineInputAssemblyStateCreateInfo &inputAssembly
        );

        static void createViewportState(
                VkViewport &viewport,
                VkRect2D &scissor,
                VkPipelineViewportStateCreateInfo &state
        );

        static void createRasterizationState(
                const IRenderDevice::PipelineRasterizationDesc &rasterizationDesc,
                VkPipelineRasterizationStateCreateInfo &rasterizer
        );

        static void createPipelineLayout(

                const VulkanUniformLayout &uniformLayout,
                VkPipelineLayout &pipelineLayout
        );

        static void createMultisampleState(
                VkPipelineMultisampleStateCreateInfo &state
        );

        static void createColorBlendAttachmentState(
                const IRenderDevice::BlendAttachmentDesc &attachmentDesc,
                VkPipelineColorBlendAttachmentState &state
        );

        static void createColorBlendState(
                const IRenderDevice::PipelineBlendStateDesc &stateDesc,
                uint32 attachmentsCount,
                const VkPipelineColorBlendAttachmentState *attachments,
                VkPipelineColorBlendStateCreateInfo &stateCreateInfo
        );

        static void createSurfaceColorBlendState(
                const IRenderDevice::PipelineSurfaceBlendStateDesc &stateDesc,
                const VkPipelineColorBlendAttachmentState *attachment,
                VkPipelineColorBlendStateCreateInfo &stateCreateInfo
        );

        static void createDepthStencilState(
                const IRenderDevice::PipelineDepthStencilStateDesc &desc,
                VkPipelineDepthStencilStateCreateInfo &stateCreateInfo
        );

        static VkStencilOpState createStencilOperationState(
                const IRenderDevice::StencilOpStateDesc &desc
        );

        static VkCommandPool createCommandPool(
                VkCommandPoolCreateFlags flags,
                uint32 queueFamilyIndex
        );

        static VkCommandBuffer beginTmpCommandBuffer(
                VkCommandPool commandPool
        );

        static void endTmpCommandBuffer(
                VkCommandBuffer commandBuffer,
                VkQueue queue,
                VkCommandPool commandPool
        );

        static void destroyTmpComandBuffer(
                VkCommandBuffer commandBuffer,
                VkCommandPool commandPool
        );

        static void destroyBuffer(
                VkBuffer buffer,
                VulkanAllocation &allocation
        );

        static void destroyImage(
                VkImage image,
                VulkanAllocation &allocation
        );
    };

} // namespace ignimbrite

#endif //IGNIMBRITE_VULKANUTILS_H
