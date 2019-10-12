#include "Shader.h"
#include <cassert>

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
	VkDescriptorSetLayoutBinding layoutBinding[LAYOUT_BINDING_COUNT] = {};

	layoutBinding[0] = {};
	layoutBinding[0].binding = 0;
	layoutBinding[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layoutBinding[0].descriptorCount = 1;
	layoutBinding[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	layoutBinding[0].pImmutableSamplers = NULL;

	VkDescriptorSetLayoutCreateInfo descriptorLayout = {};
	descriptorLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorLayout.pNext = NULL;
	descriptorLayout.bindingCount = LAYOUT_BINDING_COUNT;
	descriptorLayout.pBindings = layoutBinding;

	VkResult r = vkCreateDescriptorSetLayout(device, &descriptorLayout, NULL, &descriptorSetLayout);
	assert(r == VK_SUCCESS);
}

void Shader::DestroyDescriptorSetLayout(VkDevice device)
{
	vkDestroyDescriptorSetLayout(device, descriptorSetLayout, NULL);
}

const VkDescriptorSetLayout& Shader::GetDescriptorSetLayout() const
{
	return descriptorSetLayout;
}
