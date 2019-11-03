//
// Created by Egor Orachyov on 2019-11-03.
//

#ifndef RENDERINGLIBRARY_VULKANDEFINITIONS_H
#define RENDERINGLIBRARY_VULKANDEFINITIONS_H

#include <vulkan/vulkan.h>
#include <renderer/DeviceDefinitions.h>
#include <exception>
#include <string>

class InvalidEnum : public std::exception {
public:
    ~InvalidEnum() override = default;
    const char *what() const noexcept override {
        return "VulkanDefinitions: invalid input enum";
    }
};

class VulkanDefinitions {
public:

    static VkFormat dataFormat(DataFormat format) {
        switch (format) {
            case DataFormat::D24_UNORM_S8_UINT:
                return VkFormat::VK_FORMAT_D24_UNORM_S8_UINT;
            default:
                throw InvalidEnum();
        }
    }

    static VkVertexInputRate vertexInputRate(VertexUsage vertexUsage) {
        switch (vertexUsage) {
            case VertexUsage::PerVertex:
                return VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX;
            case VertexUsage::PerInstance:
                return VkVertexInputRate::VK_VERTEX_INPUT_RATE_INSTANCE;
            default:
                throw InvalidEnum();
        }
    }
};

#endif //RENDERINGLIBRARY_VULKANDEFINITIONS_H
