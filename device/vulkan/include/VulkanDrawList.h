/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov, Sultim Tsyrendashiev                  */
/**********************************************************************************/

#ifndef IGNIMBRITELIBRARY_VULKANCOMMANDSQUEUE_H
#define IGNIMBRITELIBRARY_VULKANCOMMANDSQUEUE_H

#include <VulkanContext.h>
#include <vector>

namespace ignimbrite {

    /** Single time submit drawing list */
    struct VulkanDrawList {
        VkCommandBuffer commandBuffer;
        VkPipelineLayout pipelineLayout;
        bool frameBufferAttached;
    };

} // namespace ignimbrite

#endif //IGNIMBRITELIBRARY_VULKANCOMMANDSQUEUE_H