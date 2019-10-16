//
// Created by Egor Orachyov on 2019-10-15.
//

#ifndef VULKANRENDERER_VULKANAPPLICATION_H
#define VULKANRENDERER_VULKANAPPLICATION_H

#include <VulkanWindow.h>
#include <string>
#include <vector>

struct VulkanApplication {
    std::string name = "Default";
    std::vector<VulkanWindow> windows = { VulkanWindow() };
    uint32 primaryWindow = 0;
    uint32 extensionsCount = 0;
    bool enableValidation = true;
    const char* const* extensions = nullptr;

    VulkanWindow& getPrimaryWindow() {
        return windows[primaryWindow];
    }
};

#endif //VULKANRENDERER_VULKANAPPLICATION_H
