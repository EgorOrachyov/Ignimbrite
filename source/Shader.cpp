#include "Shader.h"
#include <cassert>

// for allocation mark define: TR_VK_ALLOCATION_CALLBACKS_MARK
#include "VulkanTriangle.h"

// specific for this shader
#define LAYOUT_BINDING_COUNT 1

const char* vertShaderText =
"#version 400\n"
"#extension GL_ARB_separate_shader_objects : enable\n"
"#extension GL_ARB_shading_language_420pack : enable\n"
"layout (std140, binding = 0) uniform buf {\n"
"        mat4 mvp;\n"
"} ubuf;\n"
"layout (location = 0) in vec4 pos;\n"
"layout (location = 1) in vec2 inTexCoords;\n"
"layout (location = 0) out vec2 texcoord;\n"
"void main() {\n"
"   texcoord = inTexCoords;\n"
"   gl_Position = ubuf.mvp * pos;\n"
"}\n";

const char* fragShaderText =
"#version 400\n"
"#extension GL_ARB_separate_shader_objects : enable\n"
"#extension GL_ARB_shading_language_420pack : enable\n"
"layout (binding = 1) uniform sampler2D tex;\n"
"layout (location = 0) in vec2 texcoord;\n"
"layout (location = 0) out vec4 outColor;\n"
"void main() {\n"
"   outColor = textureLod(tex, texcoord, 0.0);\n"
"}\n";

void Shader::CreateDescriptorSetLayout(VkDevice device)
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

void Shader::DestroyDescriptorSetLayout(VkDevice device)
{
	vkDestroyDescriptorSetLayout(device, descriptorSetLayout, NULL);
}

void Shader::Init(VkDevice device)
{
	CreateDescriptorSetLayout(device);
	CreatePoolSize();
}

void Shader::Destroy(VkDevice device)
{
	DestroyDescriptorSetLayout(device);
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
