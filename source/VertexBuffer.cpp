#include "VertexBuffer.h"
#include "VulkanTriangle.h"
#include <cassert>
#include <string>

void VertexBuffer::CreateBufferObject(VkDeviceSize bufferSize)
{
	VkBufferCreateInfo bufferCreateInfo = {};

	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.pNext = NULL;
	// will be used as vertex buffer
	bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	bufferCreateInfo.size = bufferSize;
	bufferCreateInfo.queueFamilyIndexCount = 0;
	bufferCreateInfo.pQueueFamilyIndices = NULL;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferCreateInfo.flags = 0;

	VkResult r = vkCreateBuffer(this->Device, &bufferCreateInfo, TR_VK_ALLOCATION_CALLBACKS_MARK, &this->Buffer);
	assert(r == VK_SUCCESS);
}

void VertexBuffer::AllocateDeviceMemory(const VkPhysicalDeviceMemoryProperties & physDeviceMemProperties)
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

void VertexBuffer::BindBufferMemory(VkDeviceSize uniformBufferSize)
{
	VkResult r = vkBindBufferMemory(this->Device, this->Buffer, this->Memory, 0);
	assert(r == VK_SUCCESS);

	this->BufferInfo.buffer = this->Buffer;
	this->BufferInfo.offset = 0;
	this->BufferInfo.range = uniformBufferSize;
}

void VertexBuffer::Init(const VkPhysicalDeviceMemoryProperties & physDeviceMemProperties, VkDevice device)
{
	uint32_t bufferSize = sizeof(g_vb_solid_face_colors_Data);

	this->Device = device;

	CreateBufferObject(bufferSize);
	AllocateDeviceMemory(physDeviceMemProperties);
	BindBufferMemory(bufferSize);

	// vertex
	InitVertexInputDescription();
}

void VertexBuffer::Destroy()
{
	vkDestroyBuffer(this->Device, this->Buffer, NULL);
	vkFreeMemory(this->Device, this->Memory, NULL);
}

void VertexBuffer::MapAndCopy() const
{
	void *data = (void*)g_vb_solid_face_colors_Data;
	uint32_t dataSize = sizeof(g_vb_solid_face_colors_Data);

	void *mapped;

	// map memory
	VkResult r = vkMapMemory(this->Device, this->Memory, 0, this->RequiredMemorySize, 0, &mapped);
	assert(r == VK_SUCCESS);

	// copy data to this memory
	memcpy(mapped, &data, dataSize);
}

void VertexBuffer::Unmap() const
{
	vkUnmapMemory(this->Device, this->Memory);
}

void VertexBuffer::InitVertexInputDescription()
{
	vertInputBinding.binding = 0;
	vertInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	vertInputBinding.stride = sizeof(g_vb_solid_face_colors_Data[0]);

	vertInputAttributes[0].binding = 0;
	vertInputAttributes[0].location = 0;
	vertInputAttributes[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	vertInputAttributes[0].offset = 0;

	vertInputAttributes[1].binding = 0;
	vertInputAttributes[1].location = 1;
	vertInputAttributes[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	vertInputAttributes[1].offset = 16;
}
