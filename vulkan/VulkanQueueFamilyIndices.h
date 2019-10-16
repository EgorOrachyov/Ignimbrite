//
// Created by Egor Orachyov on 2019-10-15.
//

#ifndef VULKANRENDERER_VULKANQUEUEFAMILYINDICES_H
#define VULKANRENDERER_VULKANQUEUEFAMILYINDICES_H

#include <Optional.h>

struct VulkanQueueFamilyIndices {
    Optional<uint32> graphicsFamily;
    Optional<uint32> presentFamily;

    bool isComplete() {
        return graphicsFamily.hasValue() &&
            presentFamily.hasValue();
    }
};

#endif //VULKANRENDERER_VULKANQUEUEFAMILYINDICES_H
