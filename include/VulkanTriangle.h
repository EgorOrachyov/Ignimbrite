#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

#include "ValidationLayers.h"
#include "UniformBuffer.h"
#include "VertexBuffer.h"

// special mark that shows vulkan allocation callbacks
#define TR_VK_ALLOCATION_CALLBACKS_MARK NULL

class VulkanTriangle;

struct SwapchainBuffer 
{
	VkImage					Image;
	VkImageView				View;
};

struct DepthBuffer
{
	VkImage					Image;
	VkImageView				View;
	VkDeviceMemory			Memory;
	VkFormat				Format;
};

// Scene that contains cube and a camera
class Scene
{
public:
	// Model view projection matrix
	float				MVP[16];

	void				CalculateMVP(float width, float height, float angle);
	void				Destroy();
};

class Utils
{
public:
	static bool GetMemoryType(const VkPhysicalDeviceMemoryProperties& deviceMemProperties, uint32_t memoryTypeBits, VkFlags requirementsMask, uint32_t& result);
};

class VulkanTriangle
{
private:
	int WindowWidth = 800;
	int WindowHeight = 600;

	float time = 0;

#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif

	GLFWwindow						*window;

	VkInstance						vkInstance;
	
	ValidationLayers				validationLayers;
	
	VkSurfaceKHR					surface;
	VkFormat						surfaceFormat;

	VkCommandPool					commandPool;
	std::vector<VkCommandBuffer>	commandBuffers;

	VkSwapchainKHR					swapchain;
	uint32_t						swapchainImageCount;
	std::vector<SwapchainBuffer>	imageBuffers;

	VkSemaphore						swapSemaphore;

	VkSampleCountFlagBits			SampleCount;

	VkPipelineLayout				pipelineLayout;

	VkDescriptorPool				descriptorPool;
	std::vector<VkDescriptorSet>	descriptorSets;

	VkRenderPass					renderPass;

	std::vector<VkFramebuffer>		framebuffers;

	std::vector<VkPipeline>			pipelines;

	DepthBuffer								depthBuffer;

	std::vector<VkPhysicalDevice>			physicalDevices;

	int										choosedPhysDevice;
	VkPhysicalDeviceMemoryProperties		choosedDeviceMemProperties;
	VkPhysicalDeviceProperties				choosedDeviceProperties;
	
	uint32_t								queueFamilyCount;
	std::vector<VkQueueFamilyProperties>	queueFamilyProperties;

	uint32_t				graphicsQueueFamilyIndex;
	uint32_t				presentQueueFamilyIndex;

	VkQueue					graphicsQueue;
	VkQueue					presentQueue;

	VkDevice				device;


	VertexBuffer vertexBuffer;

private:
	std::vector<const char*> GetRequiredInstanceExtensions();
	std::vector<const char*> GetRequiredDeviceExtensions();

	// Create GLFW window
	void CreateWindow();

	void InitVulkan();
	void CreateInstance();
	void CreateSurface();
	void SetupDebugMessenger();
	// Find physical devices, setup queue families, 
	// create logical device
	void EnumerateDevices();

	void CreateCommandPool();
	void CreateCommandBuffers();

	void BeginCommandBuffer(VkCommandBuffer cmdBuffer);
	void EndCommandBuffer(VkCommandBuffer cmdBuffer);

	void CreateSwapchain();
	// Find queue family that supports graphics and present
	void FindQueueFamilyIndices();
	// Find surface format
	void FindSupportedFormats();
	void SetSurfaceCapabilities(VkSwapchainCreateInfoKHR &swapchainCreateInfo);

	void CreateSwapchainImages();
	void CreateDepthBuffer();

	void CreatePipelineLayout(const VkDescriptorSetLayout* pSetLayouts, uint32_t setLayoutCount);
	void CreateDescriptorPool(const VkDescriptorPoolSize* poolSizes, uint32_t poolSizeCount);
	void AllocateDescriptorSets(const VkDescriptorSetLayout* descSetLayouts, uint32_t descriptorSetCount);

	void CreateRenderPass();
	void CreateFramebuffers();

	void CreateGraphicsPipeline(const VkVertexInputBindingDescription* pVertexBindingDescriptions, uint32_t vertexBindingDescriptionCount,
		const VkVertexInputAttributeDescription* pVertexAttributeDescriptions, uint32_t vertexAttributeDescriptionCount,
		const VkPipelineShaderStageCreateInfo* pStages, uint32_t stageCount);

	void CreateSemaphore();


	void MainLoop();


	void DestroyVulkan();
	void DestroyWindow();
	void DestroyInstance();
	void DestroyDevice();
	
	void DestroyCommandPool();
	void DestroyCommandBuffers();

	void DestroySwapchain();
	void DestroyDepthBuffer();

	void DestroyPipelineLayout();
	void DestroyDescriptorPool();

	void DestroyRenderPass();
	void DestroyFramebuffers();

	void DestroyGraphicsPipeline();

	void DestroySemaphore();

public:
	void Start();

	const VkPhysicalDeviceMemoryProperties &GetChoosedDeviceMemProperties() const;
	const VkDevice GetDevice() const;
};