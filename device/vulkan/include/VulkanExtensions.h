//
// Created by Egor Orachyov on 2019-11-11.
//

#ifndef RENDERINGLIBRARY_VULKANEXTENSIONS_H
#define RENDERINGLIBRARY_VULKANEXTENSIONS_H

#include <renderer/Options.h>
#include <renderer/Types.h>
#include <VulkanRenderDevice.h>

#ifdef WITH_GLFW
#   include "GLFW/glfw3.h"
#endif

/**
 * Provides access for various platform dependent vulkan functionality,
 * needed for OS creating and managing windows, surfaces, etc.
 */
class VulkanExtensions {
    typedef ObjectID ID;
#ifdef WITH_GLFW
    /**
     * Creates surface for specified GLFW window instance
     * @throw VulkanException if failed to create vulkan surface
     *
     * @param device
     * @param handle
     * @param width
     * @param height
     * @param widthFramebuffer
     * @param heightFramebuffer
     * @param name Required param to reference created window
     */
    static void createSurfaceGLFW(
            VulkanRenderDevice &device,
            GLFWwindow *handle,
            uint32 width,
            uint32 height,
            uint32 widthFramebuffer,
            uint32 heightFramebuffer,
            const std::string &name
    );
#endif

    /** Idle device and destroy surface with all its relative data */
    static void destroySurface(
            VulkanRenderDevice &device,
            ID surface
    );

};

#endif //RENDERINGLIBRARY_VULKANEXTENSIONS_H
