//
// Created by Egor Orachyov on 2019-10-15.
//

#ifndef VULKANRENDERER_VULKANQUEUEFAMILYINDICES_H
#define VULKANRENDERER_VULKANQUEUEFAMILYINDICES_H

#include <Types.h>

struct VulkanQueueFamilyIndices {
    uint32 graphicsFamily = 0xffffffff;
    bool found = false;

    void setGraphicsFamilyIndex(uint32 i) {
        graphicsFamily = i;
        found = true;
    }

    bool isComplete() {
        return found;
    }
};

#endif //VULKANRENDERER_VULKANQUEUEFAMILYINDICES_H
