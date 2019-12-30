/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov, Sultim Tsyrendashiev                  */
/**********************************************************************************/

#ifndef IGNIMBRITELIBRARY_VULKANDRAWLIST_H
#define IGNIMBRITELIBRARY_VULKANDRAWLIST_H

#include <VulkanContext.h>
#include <vector>

namespace ignimbrite {

    /** Control draw list state, while it is update by user */
    struct VulkanDrawListStateControl {
        VulkanDrawListStateControl() {
            frameBufferAttached = false;
            pipelineAttached = false;
            indexBufferAttached = false;
            vertexBufferAttached = false;

            commandBuffer = VK_NULL_HANDLE;
            pipelineLayout = VK_NULL_HANDLE;
        }

        void resetFlags() {
            frameBufferAttached = false;
            pipelineAttached = false;
            indexBufferAttached = false;
            vertexBufferAttached = false;
        }

        bool frameBufferAttached : 1;
        bool pipelineAttached : 1;
        bool indexBufferAttached : 1;
        bool vertexBufferAttached : 1;

        /** Draw list to be filled */
        VkCommandBuffer commandBuffer;
        /** Currently attached layout, needed for uniform set binding */
        VkPipelineLayout pipelineLayout;
    };

} // namespace ignimbrite

#endif //IGNIMBRITELIBRARY_VULKANDRAWLIST_H