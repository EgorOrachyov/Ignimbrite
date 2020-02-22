/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov, Sultim Tsyrendashiev                  */
/**********************************************************************************/

#ifndef IGNIMBRITE_VULKANEXTENSIONS_H
#define IGNIMBRITE_VULKANEXTENSIONS_H

#include <ignimbrite/Options.h>
#include <ignimbrite/Types.h>
#include <VulkanRenderDevice.h>

#ifdef IGNIMBRITE_WITH_GLFW
#   include <GLFW/glfw3.h>
#endif

#ifdef IGNIMBRITE_WITH_QT
#   include <QVulkanWindow>
#   include <QVulkanInstance>
#endif

namespace ignimbrite {

    /**
     * Provides access for various platform dependent vulkan functionality,
     * needed for OS creating and managing windows, surfaces, etc.
     */
    class VulkanExtensions {
    public:

#ifdef IGNIMBRITE_WITH_GLFW
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
        static RenderDevice::ID createSurfaceGLFW(
                VulkanRenderDevice &device,
                GLFWwindow *handle,
                uint32 widthFramebuffer,
                uint32 heightFramebuffer,
                const std::string &name
        );
#endif

#ifdef IGNIMBRITE_WITH_QT
        /**
         * Create Ignimbrite surface for specified QWindow
         * @throw VulkanException if failed to create vulkan surface
         *
         * @param device Ignimbrite vulkan render device
         * @param qvkInstance device's VkInstance will be set to this QVulkanInstance
         * @param qwindow QWindow to get surface from
         * @return Ignimbrite surface ID
         */
        static ID createSurfaceQtWindow(
                VulkanRenderDevice &device,
                QVulkanInstance *qvkInstance,
                QWindow *qwindow
        );

        /**
         * Set render device VkInstance to QVulkanInstance
         *
         * @param device Ignimbrite vulkan render device
         * @param qvkInstance device's VkInstance will be set to this QVulkanInstance
         */
        static void setVulkanInstance(
                VulkanRenderDevice &device,
                QVulkanInstance *qvkInstance
        );

        /**
         * Create Ignimbrite surface for specified QWindow.
         * Use this function if QWindow will be used as a widget
         * and rendering surface is not a whole window.
         * @param device Ignimbrite vulkan render device
         * @param qwindow QWindow to get vulkan surface
         * @return Ignimbrite surface ID
         */
        static ID createSurfaceQtWidget(
                VulkanRenderDevice &device,
                QWindow *qwindow
        );
#endif

        /** Idle device and destroy surface with all its relative data */
        static void destroySurface(
                VulkanRenderDevice &device,
                RenderDevice::ID surface
        );

    private:
        static RenderDevice::ID createSurfaceFromKHR(
                VulkanRenderDevice &device,
                VkSurfaceKHR surfaceKhr,
                uint32 widthFramebuffer,
                uint32 heightFramebuffer,
                const std::string &name
        );
    };

} // namespace ignimbrite

#endif //IGNIMBRITE_VULKANEXTENSIONS_H
