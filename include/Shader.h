#pragma once

#include <vulkan/vulkan.h>

// Hardcoded shader for rendering a cube
class Shader
{
private:
	VkDescriptorSetLayout descriptorSetLayout;

public:
	// Create descriptor set layout for this shader (should be virtual)
	void CreateDescriptorSetLayout(VkDevice device);
	// Destroy descriptor set layout for this shader (should be virtual)
	void DestroyDescriptorSetLayout(VkDevice device);

	const VkDescriptorSetLayout &GetDescriptorSetLayout() const;
};