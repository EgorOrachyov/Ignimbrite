#include "Shader.h"
#include <cassert>
#include <fstream>
#include <iterator>

// for allocation mark define: TR_VK_ALLOCATION_CALLBACKS_MARK
#include "VulkanTriangle.h"

// specific for this shader
#define LAYOUT_BINDING_COUNT 1

void Shader::CreateDescriptorSetLayout()
{
	// current shader uses only 1 uniform buffer
	VkDescriptorSetLayoutBinding layoutBinding[LAYOUT_BINDING_COUNT] = {};

	layoutBinding[0] = {};
	
	// binding index, used in shader for uniform buffer
	layoutBinding[0].binding = 0;
	layoutBinding[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layoutBinding[0].descriptorCount = 1;
	// this binding is for vertex shader
	layoutBinding[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	layoutBinding[0].pImmutableSamplers = NULL;

	VkDescriptorSetLayoutCreateInfo descriptorLayout = {};
	descriptorLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorLayout.pNext = NULL;
	descriptorLayout.bindingCount = LAYOUT_BINDING_COUNT;
	descriptorLayout.pBindings = layoutBinding;

	VkResult r = vkCreateDescriptorSetLayout(device, &descriptorLayout, TR_VK_ALLOCATION_CALLBACKS_MARK, &descriptorSetLayout);
	assert(r == VK_SUCCESS);
}

void Shader::CreatePoolSize()
{
	// current shader uses only 1 uniform buffer
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount = 1;
}

void Shader::DestroyDescriptorSetLayout()
{
	vkDestroyDescriptorSetLayout(device, descriptorSetLayout, NULL);
}

void Shader::DestroyShaderStages()
{
	vkDestroyShaderModule(device, vertStage.module, NULL);
	vkDestroyShaderModule(device, fragStage.module, NULL);
}

void Shader::Init(VkDevice device)
{
	this->device = device;

	CreateDescriptorSetLayout();
	CreatePoolSize();
}

void Shader::Load(const char* vertSpvPath, const char* fragSpvPath)
{
	std::ifstream vertFile(vertSpvPath, std::ios::binary);
	std::ifstream fragFile(fragSpvPath, std::ios::binary);

	std::vector<char> vertSpv(std::istreambuf_iterator<char>(vertFile), {});
	std::vector<char> fragSpv(std::istreambuf_iterator<char>(fragFile), {});

	size_t vertSpvSize = vertSpv.size() * sizeof(char);
	size_t fragSpvSize = fragSpv.size() * sizeof(char);

	CreateStage((uint32_t*)vertSpv.data(), vertSpvSize, VK_SHADER_STAGE_VERTEX_BIT, &vertStage);
	CreateStage((uint32_t*)fragSpv.data(), fragSpvSize, VK_SHADER_STAGE_FRAGMENT_BIT, &fragStage);
}

void Shader::CreateStage(uint32_t *spvCode, size_t codeSize, VkShaderStageFlagBits stageType, VkPipelineShaderStageCreateInfo* result)
{
	result->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	result->pNext = NULL;
	result->pSpecializationInfo = NULL;
	result->flags = 0;
	// is vertex stage
	result->stage = stageType;
	// entry point name
	result->pName = "main";

	VkShaderModuleCreateInfo moduleCreateInfo;
	moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	moduleCreateInfo.pNext = NULL;
	moduleCreateInfo.flags = 0;
	moduleCreateInfo.codeSize = codeSize;
	moduleCreateInfo.pCode = spvCode;

	VkResult r = vkCreateShaderModule(device, &moduleCreateInfo, TR_VK_ALLOCATION_CALLBACKS_MARK, &result->module);
	assert(r == VK_SUCCESS);
}

void Shader::Destroy()
{
	DestroyDescriptorSetLayout();
	DestroyShaderStages();
}

const VkDescriptorSetLayout &Shader::GetDescriptorSetLayout() const
{
	return descriptorSetLayout;
}

const VkDescriptorPoolSize &Shader::GetPoolSize() const
{
	return poolSize;
}

VkWriteDescriptorSet Shader::GetWriteDescriptorSet(const VkDescriptorSet &descSet) const
{
	VkWriteDescriptorSet writeDescSet = {};

	writeDescSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescSet.pNext = NULL;
	writeDescSet.dstSet = descSet;
	writeDescSet.descriptorCount = 1;

	// uniform buffer
	writeDescSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

	// specify buffer info
	writeDescSet.pBufferInfo = &mvpUniform.BufferInfo;

	writeDescSet.dstArrayElement = 0;
	writeDescSet.dstBinding = 0;

	return writeDescSet;
}
