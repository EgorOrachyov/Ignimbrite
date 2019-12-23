/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                        */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov, Sultim Tsyrendashiev                  */
/**********************************************************************************/

#ifndef IGNIMBRITELIBRARY_VULKANEXTENSIONS_H
#define IGNIMBRITELIBRARY_VULKANEXTENSIONS_H

#include <ignimbrite/Options.h>
#include <ignimbrite/Types.h>
#include <VulkanRenderDevice.h>

#ifdef WITH_GLFW
#   include "GLFW/glfw3.h"
#endif

namespace ignimbrite {

    /**
     * Provides access for various platform dependent vulkan functionality,
     * needed for OS creating and managing windows, surfaces, etc.
     */
    class VulkanExtensions {
    public:
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
        static ID createSurfaceGLFW(
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

} // namespace ignimbrite

#endif //IGNIMBRITELIBRARY_VULKANEXTENSIONS_H
