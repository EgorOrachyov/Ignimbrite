/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019, 2020  Egor Orachyov                                        */
/* Copyright (c) 2019, 2020  Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#include <VulkanFence.h>

using namespace ignimbrite ;

        VulkanFence::VulkanFence() {
            VkFenceCreateInfo fenceCreateInfo {};
            fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fenceCreateInfo.flags = VkFenceCreateFlagBits::VK_FENCE_CREATE_SIGNALED_BIT;

            auto &context = VulkanContext::getInstance();
            auto result = vkCreateFence(context.device, &fenceCreateInfo, nullptr, &mFence);
            VK_RESULT_ASSERT(result, "Failed to create fence");
        }

        VulkanFence::~VulkanFence() {
            if (mFence != VK_NULL_HANDLE) {
                auto &context = VulkanContext::getInstance();
                vkDestroyFence(context.device, mFence, nullptr);
            }
        }

        VulkanFence::VulkanFence(VulkanFence && other) noexcept {
            mFence = other.mFence;
            other.mFence = VK_NULL_HANDLE;
        }

        /** Blocks until fence is set */
        void VulkanFence::wait() {
            auto &context = VulkanContext::getInstance();
            auto result = vkWaitForFences(context.device, 1, &mFence, true, UINT64_MAX);
            VK_RESULT_ASSERT(result, "Failed to wait for fence");
        }

        void VulkanFence::reset() {
            auto &context = VulkanContext::getInstance();
            auto result = vkResetFences(context.device, 1, &mFence);
            VK_RESULT_ASSERT(result, "Failed to reset fence");
        }

        /** @return Vulkan fence handler */
        VkFence VulkanFence::get() {
            return mFence;
        }
