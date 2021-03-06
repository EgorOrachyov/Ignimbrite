/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019, 2020  Egor Orachyov                                        */
/* Copyright (c) 2019, 2020  Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#ifndef IGNIMBRITE_VULKANSEMAPHORE_H
#define IGNIMBRITE_VULKANSEMAPHORE_H

#include <VulkanContext.h>

namespace ignimbrite {

    /** Vulkan semaphore for GPU -> GPU synchronization */
    class VulkanSemaphore {
    public:
        VulkanSemaphore() {
            VkSemaphoreCreateInfo semaphoreCreateInfo {};
            semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

            auto &context = VulkanContext::getInstance();
            auto result = vkCreateSemaphore(context.device, &semaphoreCreateInfo, nullptr, &mSemaphore);
            VK_RESULT_ASSERT(result, "Failed to create semaphore");
        }

        ~VulkanSemaphore() {
            if (mSemaphore != VK_NULL_HANDLE) {
                auto &context = VulkanContext::getInstance();
                vkDestroySemaphore(context.device, mSemaphore, nullptr);
            }
        }

        VulkanSemaphore(VulkanSemaphore && other) noexcept {
            mSemaphore = other.mSemaphore;
            other.mSemaphore = VK_NULL_HANDLE;
        }

        /** @return Vulkan semaphore handler */
        VkSemaphore get() {
            return mSemaphore;
        }
    private:
        VkSemaphore mSemaphore = VK_NULL_HANDLE;
    };

} // namespace ignimbrite

#endif //IGNIMBRITE_VULKANSEMAPHORE_H