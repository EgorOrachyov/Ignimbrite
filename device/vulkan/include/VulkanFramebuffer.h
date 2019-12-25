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

namespace ignimbrite {

    struct VulkanFrameBufferFormat {
        VkRenderPass renderPass;
        uint32 numOfAttachments;
        bool useDepthStencil;
    };

    struct VulkanFrameBuffer {
        VkFramebuffer framebuffer = VK_NULL_HANDLE;
        ObjectID framebufferFormatId;
        uint32 width = 0;
        uint32 height = 0;
    };

} // namespace ignimbrite

#endif //IGNIMBRITELIBRARY_VULKANFRAMEBUFFER_H
