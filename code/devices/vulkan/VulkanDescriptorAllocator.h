/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019, 2020  Egor Orachyov                                        */
/* Copyright (c) 2019, 2020  Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#ifndef IGNIMBRITE_VULKANDESCRIPTORALLOCATOR_H
#define IGNIMBRITE_VULKANDESCRIPTORALLOCATOR_H

#include <VulkanContext.h>

namespace ignimbrite {

    /** Samplers and buffers to allocate in descriptor set */
    struct VulkanDescriptorProperties {
        /** VK layout */
        VkDescriptorSetLayout layout = VK_NULL_HANDLE;
        /** Combined image samplers per descriptor set */
        uint32 samplersCount = 0;
        /** Uniform buffers per descriptor set */
        uint32 uniformBuffersCount = 0;
    };

    /**
     * @brief Allocate descriptor sets for uniform layout
     *
     * Allocates descriptor sets for uniform set objects.
     * Must be created for each uniform layout object.
     *
     * Creates descriptor pool, one by one, to allocate descriptor sets.
     * First pool size = INITIAL_POOL_SIZE, next pool factor = POOL_SIZE_FACTOR.
     * Free descriptor sets could be reused.
     */
    class VulkanDescriptorAllocator {
    public:

        explicit VulkanDescriptorAllocator() = default;
        ~VulkanDescriptorAllocator() noexcept(false);
        VulkanDescriptorAllocator(VulkanDescriptorAllocator&& allocator) = default;
        VulkanDescriptorAllocator(const VulkanDescriptorAllocator& allocator) = default;

        /**
         * Allocates descriptor set, available for creating uniform layout
         * @return Free descriptor set
         */
        VkDescriptorSet allocateSet();

        /**
         * Free specified descriptor set (later this set could be reused)
         * @param descriptorSet set to free
         */
        void freeSet(VkDescriptorSet descriptorSet);

        /**
         * Set allocation properties for descriptor pools.
         * @param properties To set
         */
        void setProperties(VulkanDescriptorProperties& properties);

    private:

        /** Single VK pool info */
        struct VulkanPoolInfo {
            /** Pool handle */
            VkDescriptorPool pool = VK_NULL_HANDLE;
            /** Currently allocated descriptors count from this pool */
            uint32 allocated = 0;
            /** Max number of allocated descriptors from this pool */
            uint32 max = 0;
        };

        /** Get pool, available for set allocating */
        VulkanPoolInfo& getFreePool();

        /** Allocates next free pool */
        VulkanPoolInfo& allocatePool();

    private:

        /** First (initial) pool size */
        static const uint32 INITIAL_POOL_SIZE = 2;
        /** Factor to increase pools size */
        static const uint32 POOL_SIZE_FACTOR = 2;

        /** Next allocated pool size */
        uint32 mNextPoolSize = INITIAL_POOL_SIZE;
        /** Max number of sets, which currently could be allocated */
        uint32 mMaxSetsCount = 0;
        /** Number of sets, which currently in usage */
        uint32 mUsedSets = 0;

        /** Properties of the descriptor layout */
        VulkanDescriptorProperties mProperties;
        /** Pools, allocated for  */
        std::vector<VulkanPoolInfo> mPools;
        /** Reusable, free descriptor sets */
        std::vector<VkDescriptorSet> mFreeSets;

    };

} // namespace ignimbrite

#endif //IGNIMBRITE_VULKANDESCRIPTORALLOCATOR_H