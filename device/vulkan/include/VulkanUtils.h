//
// Created by Egor Orachyov on 2019-11-07.
//

#ifndef RENDERINGLIBRARY_VULKANUTILS_H
#define RENDERINGLIBRARY_VULKANUTILS_H

#include <renderer/Optional.h>
#include <vulkan/vulkan.h>
#include <vector>

struct QueueFamilyIndices {
    Optional<uint32> graphicsFamily;
    Optional<uint32> transferFamily;

    bool isComplete() {
        return graphicsFamily.hasValue() &&
               transferFamily.hasValue();
    }
};

#endif //RENDERINGLIBRARY_VULKANUTILS_H
