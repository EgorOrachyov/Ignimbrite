//
// Created by Egor Orachyov on 2019-10-15.
//

#ifndef VULKANRENDERER_VULKANWINDOW_H
#define VULKANRENDERER_VULKANWINDOW_H

#include <Platform.h>
#include <Types.h>
#include <VulkanGlfw.h>

struct VulkanWindow {
    GLFWwindow* handle = nullptr;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
};

#endif //VULKANRENDERER_VULKANWINDOW_H
