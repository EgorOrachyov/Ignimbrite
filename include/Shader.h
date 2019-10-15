#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "UniformBuffer.h"

// Hardcoded shader for rendering a cube
class Shader
{
private:
	VkDevice				device;

	VkDescriptorSetLayout	descriptorSetLayout;
	VkDescriptorPoolSize	poolSize;

	VkPipelineShaderStageCreateInfo vertStage;
	VkPipelineShaderStageCreateInfo fragStage;

public:
	// uniform buffer for MVP matrix (should be in a child of this class)
	UniformBuffer			mvpUniform;

private:
	// Create descriptor set layout for this shader (should be abstract)
	void CreateDescriptorSetLayout();
	// Define pool size for this shader (should be abstract)
	void CreatePoolSize();

	void CreateStage(uint32_t* spvCode, size_t codeSize, VkShaderStageFlagBits stageType, VkPipelineShaderStageCreateInfo* result);
	
	// Destroy descriptor set layout for this shader
	void DestroyDescriptorSetLayout();
	void DestroyShaderStages();


public:
	void Init(VkDevice device);
	void Load(const char *vertSpvPath, const char *fragSpvPath);
	void Destroy();

	const VkDescriptorSetLayout		&GetDescriptorSetLayout() const;
	const VkDescriptorPoolSize		&GetPoolSize() const;
	VkWriteDescriptorSet			GetWriteDescriptorSet(const VkDescriptorSet &descSet) const;
};