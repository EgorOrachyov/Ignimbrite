//
// Created by Egor Orachyov on 2019-11-07.
//

#ifndef RENDERINGLIBRARY_VULKANSTRUCTURES_H
#define RENDERINGLIBRARY_VULKANSTRUCTURES_H

#include <vulkan/vulkan.h>

struct BufferObject {
    VkBuffer buffer;
    VkDeviceMemory memory;
};

#endif //RENDERINGLIBRARY_VULKANSTRUCTURES_H
