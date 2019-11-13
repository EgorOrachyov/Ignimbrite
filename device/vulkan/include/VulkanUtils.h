//
// Created by Egor Orachyov on 2019-11-07.
//

#ifndef RENDERINGLIBRARY_VULKANUTILS_H
#define RENDERINGLIBRARY_VULKANUTILS_H

#include <VulkanContext.h>
#include <VulkanDefinitions.h>

class VulkanUtils {
public:

    static VkFormatProperties getDeviceFormatProperties(
            VulkanContext& context,
            VkFormat format
    );

    static uint32_t getMemoryTypeIndex(
            VulkanContext& context,
            uint32_t memoryTypeBits,
            VkFlags requirementsMask
    );

    static void createBuffer(
            VulkanContext& context,
            VkDeviceSize size,
            VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
            VkBuffer &outBuffer, VkDeviceMemory &outBufferMemory
    );

    static void createBufferLocal(
            VulkanContext& context,
            const void *data,
            VkDeviceSize size, VkBufferUsageFlags usage,
            VkBuffer &outBuffer, VkDeviceMemory &outBufferMemory
    );

    static void copyBuffer(
            VulkanContext& context,
            VkCommandPool commandPool,
            VkQueue queue,
            VkBuffer srcBuffer, VkBuffer dstBuffer,
            VkDeviceSize size
    );

    static void updateBufferMemory(
            VulkanContext& context,
            VkDeviceMemory bufferMemory,
            VkDeviceSize offset, VkDeviceSize size,
            const void *data
    );

    static void createTextureImage(
            VulkanContext& context,
            const void *imageData,
            uint32_t width, uint32_t height, 
            uint32_t depth, uint32_t mipLevels,
            VkImageType imageType, VkFormat format, VkImageTiling tiling,
            VkImage &outTextureImage, VkDeviceMemory &outTextureMemory,
            VkImageLayout textureLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    );

    static void createImage(
            VulkanContext& context,
            uint32_t width, uint32_t height, 
            uint32_t depth, uint32_t mipLevels,
            VkImageType imageType, VkFormat format,
            VkImageTiling tiling, VkImageUsageFlags usage,
            VkMemoryPropertyFlags properties,
            VkImage &outImage, VkDeviceMemory &outImageMemory
    );

    static void copyBufferToImage(
            VulkanContext& context,
            VkBuffer buffer,
            VkImage image,
            uint32_t width, uint32_t height, uint32_t depth
    );

    static void transitionImageLayout(
            VulkanContext& context,
            VkImage image,
            VkImageLayout oldLayout,
            VkImageLayout newLayout,
            uint32_t mipLevels
    );

    static void createImageView(
            VulkanContext& context,
            VkImageView &outImageView,
            VkImage image,
            VkImageViewType viewType, VkFormat format,
            const VkImageSubresourceRange &subResourceRange,
            VkComponentMapping components = {}
    );

    static void generateMipmaps(
        VulkanContext& context,
        VkImage image, VkFormat format,
        uint32_t width, uint32_t height, 
        uint32_t depth, uint32_t mipLevels
    );

    static void createDepthStencilBuffer(VulkanContext& context, 
        uint32_t width, uint32_t height, uint32_t depth,
        VkImageType imageType, VkFormat format, VkImageViewType viewType,
        VkImage& outImage, VkDeviceMemory& outImageMemory, 
        VkImageView& outImageView
    );

    static void getSurfaceProperties(
            VkPhysicalDevice physicalDevice, VkSurfaceKHR surfaceKHR,
            std::vector<VkSurfaceFormatKHR> &outSurfaceFormats,
            std::vector<VkPresentModeKHR> &outPresentModes
    );
    
    static VkExtent2D getSwapChainExtent(
            uint32_t preferredWidth, uint32_t preferredHeight,
            const VkSurfaceCapabilitiesKHR &surfaceCapabilities
    );
    
    static VkCompositeAlphaFlagBitsKHR getAvailableCompositeAlpha(
            const VkSurfaceCapabilitiesKHR &surfaceCapabilities
    );
    
};

#endif //RENDERINGLIBRARY_VULKANUTILS_H
