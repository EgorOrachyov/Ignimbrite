/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov, Sultim Tsyrendashiev                  */
/**********************************************************************************/

#ifndef IGNIMBRITELIBRARY_VULKANFENCE_H
#define IGNIMBRITELIBRARY_VULKANFENCE_H

#include <VulkanContext.h>

namespace ignimbrite {

    /** Vulkan fence for CPU -> GPU synchronization */
    class VulkanFence {
    public:
        VulkanFence() {
            VkFenceCreateInfo fenceCreateInfo {};
            fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fenceCreateInfo.flags = VkFenceCreateFlagBits::VK_FENCE_CREATE_SIGNALED_BIT;

            auto &context = VulkanContext::getInstance();
            auto result = vkCreateFence(context.device, &fenceCreateInfo, nullptr, &mFence);
            VK_RESULT_ASSERT(result, "Failed to create fence");
        }

        ~VulkanFence() {
            if (mFence != VK_NULL_HANDLE) {
                auto &context = VulkanContext::getInstance();
                vkDestroyFence(context.device, mFence, nullptr);
            }
        }

        VulkanFence(VulkanFence && other) noexcept {
            mFence = other.mFence;
            other.mFence = VK_NULL_HANDLE;
        }

        /** Blocks until fence is set */
        void wait() {
            auto &context = VulkanContext::getInstance();
            auto result = vkWaitForFences(context.device, 1, &mFence, true, UINT64_MAX);
            VK_RESULT_ASSERT(result, "Failed to wait for fence");
        }

        void reset() {
            auto &context = VulkanContext::getInstance();
            auto result = vkResetFences(context.device, 1, &mFence);
            VK_RESULT_ASSERT(result, "Failed to reset fence");
        }

        /** @return Vulkan fence handler */
        VkFence get() {
            return mFence;
        }
    private:
        VkFence mFence = VK_NULL_HANDLE;
    };

} // namespace ignimbrite

#endif //IGNIMBRITELIBRARY_VULKANFENCE_H