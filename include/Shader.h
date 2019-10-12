#pragma once

#include <vulkan/vulkan.h>
#include "UniformBuffer.h"

// Hardcoded shader for rendering a cube
class Shader
{
private:
	VkDescriptorSetLayout	descriptorSetLayout;
	VkDescriptorPoolSize	poolSize;

public:
	// uniform buffer for MVP matrix (should be in a child of this class)
	UniformBuffer			mvpUniform;

private:
	// Create descriptor set layout for this shader (should be abstract)
	void CreateDescriptorSetLayout(VkDevice device);
	// Define pool size for this shader (should be abstract)
	void CreatePoolSize();

	// Destroy descriptor set layout for this shader
	void DestroyDescriptorSetLayout(VkDevice device);

public:
	void Init(VkDevice device);
	void Destroy(VkDevice device);

	const VkDescriptorSetLayout		&GetDescriptorSetLayout() const;
	const VkDescriptorPoolSize		&GetPoolSize() const;
	VkWriteDescriptorSet			GetWriteDescriptorSet(const VkDescriptorSet &descSet) const;
};