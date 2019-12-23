/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Berserk                                        */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov, Sultim Tsyrendashiev                  */
/**********************************************************************************/

#ifndef IGNIMBRITELIBRARY_VULKANUTILS_H
#define IGNIMBRITELIBRARY_VULKANUTILS_H

#include <VulkanContext.h>
#include <VulkanDefinitions.h>

namespace ignimbrite {

    class VulkanUtils {
    public:

        static VkFormatProperties getDeviceFormatProperties(
                VulkanContext &context,
                VkFormat format
        );

        static VkFormat findSupportedFormat(
                VulkanContext &context,
                const VkFormat *candidates, uint32 candidatesCount,
                VkImageTiling tiling,
                VkFormatFeatureFlags features
        );

        static uint32 getMemoryTypeIndex(
                VulkanContext &context,
                uint32 memoryTypeBits,
                VkFlags requirementsMask
        );

        static void createBuffer(
                VulkanContext &context,
                VkDeviceSize size,
                VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                VkBuffer &outBuffer, VkDeviceMemory &outBufferMemory
        );

        static void createBufferLocal(
                VulkanContext &context,
                const void *data,
                VkDeviceSize size, VkBufferUsageFlags usage,
                VkBuffer &outBuffer, VkDeviceMemory &outBufferMemory
        );

        static void copyBuffer(
                VulkanContext &context,
                VkBuffer srcBuffer,
                VkBuffer dstBuffer,
                VkDeviceSize size
        );

        static void updateBufferMemory(
                VulkanContext &context,
                VkDeviceMemory bufferMemory,
                VkDeviceSize offset, VkDeviceSize size,
                const void *data
        );

        static void createTextureImage(
                VulkanContext &context,
                const void *imageData,
                uint32 width, uint32 height,
                uint32 depth, uint32 mipLevels,
                VkImageType imageType, VkFormat format, VkImageTiling tiling,
                VkImage &outTextureImage, VkDeviceMemory &outTextureMemory,
                VkImageLayout textureLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );

        static void createImage(
                VulkanContext &context,
                uint32 width, uint32 height,
                uint32 depth, uint32 mipLevels,
                VkImageType imageType, VkFormat format,
                VkImageTiling tiling, VkImageUsageFlags usage,
                VkMemoryPropertyFlags properties,
                VkImage &outImage, VkDeviceMemory &outImageMemory
        );

        static void copyBufferToImage(
                VulkanContext &context,
                VkBuffer buffer,
                VkImage image,
                uint32 width, uint32 height, uint32 depth
        );

        static void transitionImageLayout(
                VulkanContext &context,
                VkImage image,
                VkImageLayout oldLayout,
                VkImageLayout newLayout,
                uint32 mipLevels
        );

        static void createImageView(
                VulkanContext &context,
                VkImageView &outImageView,
                VkImage image,
                VkImageViewType viewType, VkFormat format,
                const VkImageSubresourceRange &subResourceRange,
                VkComponentMapping components = {}
        );

        static void generateMipmaps(VulkanContext &context, VkImage image, VkFormat format, uint32 width, uint32 height,
                                    uint32 mipLevels, VkImageLayout newLayout);

        static void createDepthStencilBuffer(
                VulkanContext &context, uint32 width, uint32 height, uint32 depth,
                VkImageType imageType, VkFormat format, VkImage &outImage,
                VkDeviceMemory &outImageMemory, VkImageUsageFlags usageFlags
        );

        static void getSurfaceProperties(
                VkPhysicalDevice physicalDevice, VkSurfaceKHR surfaceKHR,
                std::vector<VkSurfaceFormatKHR> &outSurfaceFormats,
                std::vector<VkPresentModeKHR> &outPresentModes
        );

        static VkExtent2D getSwapChainExtent(
                uint32 preferredWidth, uint32 preferredHeight,
                const VkSurfaceCapabilitiesKHR &surfaceCapabilities
        );

        static VkCompositeAlphaFlagBitsKHR getAvailableCompositeAlpha(
                const VkSurfaceCapabilitiesKHR &surfaceCapabilities
        );

        static void allocateDescriptorPool(
                VulkanContext &context,
                VulkanUniformLayout &layout
        );

        static VulkanDescriptorPool &getAvailableDescriptorPool(
                VulkanContext &context,
                VulkanUniformLayout &layout
        );

        static VkDescriptorSet getAvailableDescriptorSet(
                VulkanContext& context,
                VulkanUniformLayout &layout
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
                const RenderDevice::PipelineRasterizationDesc &rasterizationDesc,
                VkPipelineRasterizationStateCreateInfo &rasterizer
        );

        static void createPipelineLayout(
                VulkanContext &context,
                const VulkanUniformLayout &uniformLayout,
                VkPipelineLayout &pipelineLayout
        );

        static void createMultisampleState(
                VkPipelineMultisampleStateCreateInfo &state
        );

        static void createColorBlendAttachmentState(
                const RenderDevice::BlendAttachmentDesc &attachmentDesc,
                VkPipelineColorBlendAttachmentState &state
        );

        static void createColorBlendState(
                const RenderDevice::PipelineBlendStateDesc &stateDesc,
                uint32 attachmentsCount,
                const VkPipelineColorBlendAttachmentState *attachments,
                VkPipelineColorBlendStateCreateInfo &stateCreateInfo
        );

        static void createSurfaceColorBlendState(
                const RenderDevice::PipelineSurfaceBlendStateDesc &stateDesc,
                const VkPipelineColorBlendAttachmentState *attachment,
                VkPipelineColorBlendStateCreateInfo &stateCreateInfo
        );

        static void createDepthStencilState(
                const RenderDevice::PipelineDepthStencilStateDesc &desc,
                VkPipelineDepthStencilStateCreateInfo &stateCreateInfo
        );

        static VkStencilOpState createStencilOperationState(
                const RenderDevice::StencilOpStateDesc &desc
        );

        static VkCommandPool createCommandPool(
                VulkanContext &context,
                VkCommandPoolCreateFlags flags, uint32_t queueFamilyIndex
        );

        static VkCommandBuffer beginTempCommandBuffer(
                VulkanContext &context, VkCommandPool commandPool
        );

        static void endTempCommandBuffer(
                VulkanContext &context, VkCommandBuffer commandBuffer,
                VkQueue queue, VkCommandPool commandPool
        );
    };

} // namespace ignimbrite

#endif //IGNIMBRITELIBRARY_VULKANUTILS_H
