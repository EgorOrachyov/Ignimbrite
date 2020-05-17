/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019, 2020  Egor Orachyov                                        */
/* Copyright (c) 2019, 2020  Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#ifndef IGNIMBRITE_VULKANFENCE_H
#define IGNIMBRITE_VULKANFENCE_H

#include <VulkanContext.h>

namespace ignimbrite {

    /** Vulkan fence for CPU -> GPU synchronization */
    class VulkanFence {
    public:
        VulkanFence() ;

        ~VulkanFence() ;

        VulkanFence(VulkanFence && other) noexcept ;

        /** Blocks until fence is set */
        void wait() ;

        void reset() ;

        /** @return Vulkan fence handler */
        VkFence get() ;
    private:
        VkFence mFence = VK_NULL_HANDLE;
    };

} // namespace ignimbrite

#endif //IGNIMBRITE_VULKANFENCE_H