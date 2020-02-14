/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov, Sultim Tsyrendashiev                  */
/**********************************************************************************/

#ifndef IGNIMBRITELIBRARY_VULKANFRAMEBUFFER_H
#define IGNIMBRITELIBRARY_VULKANFRAMEBUFFER_H

#include <ignimbrite/ObjectID.h>
#include <vulkan/vulkan.h>
#include <vector>

namespace ignimbrite {

    /**
     * Represent render pass structure, format of used color/depth
     * attachments and dependencies between resources.
     */
    struct VulkanFrameBufferFormat {
        VkRenderPass renderPass;
        uint32 numOfAttachments;
        bool useDepthStencil;
    };

    /**
     * Concrete framebuffer object
     * set of color and depth attachments.
     */
    struct VulkanFramebuffer {
        uint32 width = 0;
        uint32 height = 0;
        ObjectID framebufferFormatId;
        VkFramebuffer framebuffer = VK_NULL_HANDLE;
    };

} // namespace ignimbrite

#endif //IGNIMBRITELIBRARY_VULKANFRAMEBUFFER_H
