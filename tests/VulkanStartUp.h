//
// Created by Egor Orachyov on 2019-11-09.
//

#ifndef RENDERINGLIBRARY_VULKANSTARTUP_H
#define RENDERINGLIBRARY_VULKANSTARTUP_H

#include "VulkanContext.h"
#include "VulkanRenderDevice.h"
#include "VulkanExtensions.h"

struct VulkanStartUp {

    static void run() {
        VulkanContext context;
        context.fillRequiredExt(0, nullptr);
        context.createInstance();
        context.setupDebugMessenger();
        context.pickPhysicalDevice();

        context.destroyDebugMessenger();
        context.destroyInstance();
    }

};

#endif //RENDERINGLIBRARY_VULKANSTARTUP_H
