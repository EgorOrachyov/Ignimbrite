#pragma once
#include <vulkan/vulkan.h>

class UniformBuffer
{
private:
	void CreateBufferObject(VkDeviceSize uniformBufferSize);
	void AllocateDeviceMemory(const VkPhysicalDeviceMemoryProperties& physDeviceMemProperties);
	void BindBufferMemory(VkDeviceSize uniformBufferSize);

public:
	VkBuffer				Buffer;

	VkDeviceMemory			Memory;
	VkDeviceSize			RequiredMemorySize;

	VkDescriptorBufferInfo	BufferInfo;

	VkDevice				Device;

public:
	void Init(const VkPhysicalDeviceMemoryProperties& physDeviceMemProperties, const VkDevice& device, VkDeviceSize uniformBufferSize);
	void Destroy();

	// Map memory object into application address space
	// and copy data to this memory. This memory will be visible to device.
	void MapAndCopy(void *data, uint32_t dataSize);

	// Unmap memory that was mapped
	void Unmap();
};
