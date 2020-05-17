/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019, 2020  Egor Orachyov                                        */
/* Copyright (c) 2019, 2020  Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#include <VulkanDescriptorAllocator.h>

namespace ignimbrite {

    VulkanDescriptorAllocator::~VulkanDescriptorAllocator() noexcept(false) {
        if (mUsedSets != 0) {
            throw VulkanException("All descriptor sets, allocated for uniform layout, must be freed");
        }

        auto& context = VulkanContext::getInstance();

        for (const auto& pool: mPools) {
            vkDestroyDescriptorPool(context.device, pool.pool, nullptr);
        }
    }

    VkDescriptorSet VulkanDescriptorAllocator::allocateSet() {
        auto& context = VulkanContext::getInstance();
        VkDescriptorSet descriptorSet;

        if (mFreeSets.empty()) {
            VkResult result;

            auto& pool = getFreePool();

            VkDescriptorSetAllocateInfo descSetAllocInfo = {};
            descSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            descSetAllocInfo.pNext = nullptr;
            descSetAllocInfo.descriptorPool = pool.pool;
            descSetAllocInfo.descriptorSetCount = 1;
            descSetAllocInfo.pSetLayouts = &mProperties.layout;

            result = vkAllocateDescriptorSets(context.device, &descSetAllocInfo, &descriptorSet);
            VK_RESULT_ASSERT(result, "Can't allocate descriptor set from descriptor pool");

            pool.allocated += 1;
            mUsedSets += 1;
        }
        else {
            descriptorSet = mFreeSets.back();
            mFreeSets.pop_back();
            mUsedSets += 1;
        }

        return descriptorSet;
    }

    void VulkanDescriptorAllocator::freeSet(VkDescriptorSet descriptorSet) {
        mUsedSets -= 1;
        mFreeSets.push_back(descriptorSet);
    }

    void VulkanDescriptorAllocator::setProperties(ignimbrite::VulkanDescriptorProperties &properties) {
        mProperties = properties;
    }

    VulkanDescriptorAllocator::VulkanPoolInfo& VulkanDescriptorAllocator::getFreePool() {
        if (mUsedSets == mMaxSetsCount) {
            return allocatePool();
        }
        else {
            return mPools.back();
        }
    }

    VulkanDescriptorAllocator::VulkanPoolInfo& VulkanDescriptorAllocator::allocatePool() {
        auto& context = VulkanContext::getInstance();

        VkResult result;
        VkDescriptorPool pool;
        uint32 descriptorsCount = mNextPoolSize;
        mNextPoolSize *= POOL_SIZE_FACTOR;
        mMaxSetsCount += descriptorsCount;

        VkDescriptorPoolSize poolSizes[2];
        uint32 poolSizesCount = 0;

        if (mProperties.uniformBuffersCount > 0) {
            poolSizes[poolSizesCount].descriptorCount = mProperties.uniformBuffersCount * descriptorsCount;
            poolSizes[poolSizesCount].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            poolSizesCount += 1;
        }

        if (mProperties.samplersCount > 0) {
            poolSizes[poolSizesCount].descriptorCount = mProperties.samplersCount * descriptorsCount;
            poolSizes[poolSizesCount].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            poolSizesCount += 1;
        }

        VkDescriptorPoolCreateInfo poolCreateInfo = {};
        poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolCreateInfo.poolSizeCount = poolSizesCount;
        poolCreateInfo.pPoolSizes = poolSizes;
        poolCreateInfo.maxSets = descriptorsCount;

        result = vkCreateDescriptorPool(context.device, &poolCreateInfo, nullptr, &pool);
        VK_RESULT_ASSERT(result, "Failed to create descriptor pool");

        VulkanPoolInfo poolInfo;
        poolInfo.pool = pool;
        poolInfo.max = descriptorsCount;

        mPools.push_back(poolInfo);
        return mPools.back();
    }

} // namespace ignimbrite
