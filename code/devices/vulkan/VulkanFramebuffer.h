/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                        */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#ifndef IGNIMBRITE_VULKANFRAMEBUFFER_H
#define IGNIMBRITE_VULKANFRAMEBUFFER_H

#include <IRenderDevice.h>
#include <vulkan/vulkan.h>

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
        ID<IRenderDevice::FramebufferFormat> framebufferFormatId;
        VkFramebuffer framebuffer = VK_NULL_HANDLE;
    };

} // namespace ignimbrite

#endif //IGNIMBRITE_VULKANFRAMEBUFFER_H
