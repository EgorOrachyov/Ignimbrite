#include "UniformBuffer.h"
#include "VulkanTriangle.h"
#include <cassert>
#include <string>

void UniformBuffer::Init(const VkPhysicalDeviceMemoryProperties &physDeviceMemProperties, VkDevice device, VkDeviceSize uniformBufferSize)
{
	this->Device = device;

	CreateBufferObject(uniformBufferSize);
	AllocateDeviceMemory(physDeviceMemProperties);
	BindBufferMemory(uniformBufferSize);
}

void UniformBuffer::Destroy()
{
	vkDestroyBuffer(this->Device, this->Buffer, NULL);
	vkFreeMemory(this->Device, this->Memory, NULL);
}

void UniformBuffer::CreateBufferObject(VkDeviceSize uniformBufferSize)
{
	VkBufferCreateInfo bufferCreateInfo = {};

	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.pNext = NULL;
	// will be used as uniform buffer
	bufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	bufferCreateInfo.size = uniformBufferSize;
	bufferCreateInfo.queueFamilyIndexCount = 0;
	bufferCreateInfo.pQueueFamilyIndices = NULL;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferCreateInfo.flags = 0;

	VkResult r = vkCreateBuffer(this->Device, &bufferCreateInfo, TR_VK_ALLOCATION_CALLBACKS_MARK, &this->Buffer);
	assert(r == VK_SUCCESS);
}

void UniformBuffer::AllocateDeviceMemory(const VkPhysicalDeviceMemoryProperties &physDeviceMemProperties)
{
	VkMemoryRequirements memReqs;
	vkGetBufferMemoryRequirements(this->Device, this->Buffer, &memReqs);

	VkMemoryAllocateInfo memAllocInfo = {};
	memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAllocInfo.pNext = NULL;
	memAllocInfo.allocationSize = memReqs.size;

	this->RequiredMemorySize = memReqs.size;

	// VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT: 
	//   to be visible for the host
	// VK_MEMORY_PROPERTY_HOST_COHERENT_BIT: 
	//   writes by the host will be visible to device
	//   and vice-versa without flush
	bool found = Utils::GetMemoryType(physDeviceMemProperties, memReqs.memoryTypeBits,

		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,

		memAllocInfo.memoryTypeIndex);

	assert(found);

	VkResult r = vkAllocateMemory(this->Device, &memAllocInfo, TR_VK_ALLOCATION_CALLBACKS_MARK, &this->Memory);
	assert(r == VK_SUCCESS);
}

void UniformBuffer::BindBufferMemory(VkDeviceSize uniformBufferSize)
{
	VkResult r = vkBindBufferMemory(this->Device, this->Buffer, this->Memory, 0);
	assert(r == VK_SUCCESS);

	this->BufferInfo.buffer = this->Buffer;
	this->BufferInfo.offset = 0;
	this->BufferInfo.range = uniformBufferSize;
}

void UniformBuffer::MapAndCopy(void *data, uint32_t dataSize) const
{
	void* mapped;

	// map memory
	VkResult r = vkMapMemory(this->Device, this->Memory, 0, this->RequiredMemorySize, 0, &mapped);
	assert(r == VK_SUCCESS);

	// copy data to this memory
	memcpy(mapped, &data, dataSize);
}

void UniformBuffer::Unmap() const
{
	vkUnmapMemory(this->Device, this->Memory);
}