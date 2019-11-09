//
// Created by Egor Orachyov on 2019-11-09.
//

#ifndef RENDERINGLIBRARY_VULKANSTARTUP_H
#define RENDERINGLIBRARY_VULKANSTARTUP_H

#include "VulkanContext.h"
#include "VulkanRenderDevice.h"

struct VulkanStartUp {

    static void run() {

        VulkanContext context(0, nullptr);

    }

};

#endif //RENDERINGLIBRARY_VULKANSTARTUP_H
