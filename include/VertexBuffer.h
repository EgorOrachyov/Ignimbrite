#pragma once

#include <vulkan/vulkan.h>

struct Vertex
{
	float posX, posY, posZ, posW;  // Position data
	float r, g, b, a;              // Color
};
#define XYZ1(_x_, _y_, _z_) (_x_), (_y_), (_z_), 1.f

static const Vertex g_vb_solid_face_colors_Data[] = {
	// red face
	{ XYZ1(-1, -1, 1), XYZ1(1.f, 0.f, 0.f) },
	{ XYZ1(-1, 1, 1), XYZ1(1.f, 0.f, 0.f) },
	{ XYZ1(1, -1, 1), XYZ1(1.f, 0.f, 0.f) },
	{ XYZ1(1, -1, 1), XYZ1(1.f, 0.f, 0.f) },
	{ XYZ1(-1, 1, 1), XYZ1(1.f, 0.f, 0.f) },
	{ XYZ1(1, 1, 1), XYZ1(1.f, 0.f, 0.f) },
	// green face
	{ XYZ1(-1, -1, -1), XYZ1(0.f, 1.f, 0.f) },
	{ XYZ1(1, -1, -1), XYZ1(0.f, 1.f, 0.f) },
	{ XYZ1(-1, 1, -1), XYZ1(0.f, 1.f, 0.f) },
	{ XYZ1(-1, 1, -1), XYZ1(0.f, 1.f, 0.f) },
	{ XYZ1(1, -1, -1), XYZ1(0.f, 1.f, 0.f) },
	{ XYZ1(1, 1, -1), XYZ1(0.f, 1.f, 0.f) },
	// blue face
	{ XYZ1(-1, 1, 1), XYZ1(0.f, 0.f, 1.f) },
	{ XYZ1(-1, -1, 1), XYZ1(0.f, 0.f, 1.f) },
	{ XYZ1(-1, 1, -1), XYZ1(0.f, 0.f, 1.f) },
	{ XYZ1(-1, 1, -1), XYZ1(0.f, 0.f, 1.f) },
	{ XYZ1(-1, -1, 1), XYZ1(0.f, 0.f, 1.f) },
	{ XYZ1(-1, -1, -1), XYZ1(0.f, 0.f, 1.f) },
	// yellow face
	{ XYZ1(1, 1, 1), XYZ1(1.f, 1.f, 0.f) },
	{ XYZ1(1, 1, -1), XYZ1(1.f, 1.f, 0.f) },
	{ XYZ1(1, -1, 1), XYZ1(1.f, 1.f, 0.f) },
	{ XYZ1(1, -1, 1), XYZ1(1.f, 1.f, 0.f) },
	{ XYZ1(1, 1, -1), XYZ1(1.f, 1.f, 0.f) },
	{ XYZ1(1, -1, -1), XYZ1(1.f, 1.f, 0.f) },
	// magenta face
	{ XYZ1(1, 1, 1), XYZ1(1.f, 0.f, 1.f) },
	{ XYZ1(-1, 1, 1), XYZ1(1.f, 0.f, 1.f) },
	{ XYZ1(1, 1, -1), XYZ1(1.f, 0.f, 1.f) },
	{ XYZ1(1, 1, -1), XYZ1(1.f, 0.f, 1.f) },
	{ XYZ1(-1, 1, 1), XYZ1(1.f, 0.f, 1.f) },
	{ XYZ1(-1, 1, -1), XYZ1(1.f, 0.f, 1.f) },
	// cyan face
	{ XYZ1(1, -1, 1), XYZ1(0.f, 1.f, 1.f) },
	{ XYZ1(1, -1, -1), XYZ1(0.f, 1.f, 1.f) },
	{ XYZ1(-1, -1, 1), XYZ1(0.f, 1.f, 1.f) },
	{ XYZ1(-1, -1, 1), XYZ1(0.f, 1.f, 1.f) },
	{ XYZ1(1, -1, -1), XYZ1(0.f, 1.f, 1.f) },
	{ XYZ1(-1, -1, -1), XYZ1(0.f, 1.f, 1.f) },
};

class VertexBuffer
{
private:
	void CreateBufferObject(VkDeviceSize bufferSize);
	void AllocateDeviceMemory(const VkPhysicalDeviceMemoryProperties& physDeviceMemProperties);
	void BindBufferMemory(VkDeviceSize bufferSize);

public:
	VkBuffer				Buffer;

	VkDeviceMemory			Memory;
	VkDeviceSize			RequiredMemorySize;

	VkDescriptorBufferInfo	BufferInfo;

	VkDevice				Device;

public:
	void Init(const VkPhysicalDeviceMemoryProperties& physDeviceMemProperties, VkDevice device);
	void Destroy();

	// Map memory object into application address space
	// and copy data to this memory. This memory will be visible to device.
	void MapAndCopy() const;

	// Unmap memory that was mapped
	void Unmap() const;


// vertex buffer specific
private:
	VkVertexInputBindingDescription vertInputBinding;
	VkVertexInputAttributeDescription vertInputAttributes[2];

	void InitVertexInputDescription();
};