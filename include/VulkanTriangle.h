#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

#include "ValidationLayers.h"

//struct LayerProperties
//{
//	VkLayerProperties properties;
//	std::vector<VkExtensionProperties> instanceExtensions;
//	std::vector<VkExtensionProperties> deviceExtensions;
//};

struct SwapchainBuffer 
{
	VkImage			Image;
	VkImageView		View;
};

struct DepthBuffer
{
	VkImage			Image;
	VkImageView		View;
	VkDeviceMemory	Memory;
	VkFormat		Format;
};

class VulkanTriangle
{
private:
	int WindowWidth = 800;
	int WindowHeight = 600;

#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif

	GLFWwindow				*window;

	VkInstance				vkInstance;
	
	ValidationLayers		validationLayers;
	
	VkSurfaceKHR			surface;
	VkFormat				surfaceFormat;

	VkSwapchainKHR						swapchain;
	uint32_t							swapchainImageCount;
	std::vector<SwapchainBuffer>		imageBuffers;

	DepthBuffer							depthBuffer;

	std::vector<VkPhysicalDevice>		physicalDevices;

	int									choosedPhysDevice;
	VkPhysicalDeviceMemoryProperties	choosedDeviceMemProperties;
	VkPhysicalDeviceProperties			choosedDeviceProperties;
	
	uint32_t								queueFamilyCount;
	std::vector<VkQueueFamilyProperties>	queueFamilyProperties;

	uint32_t				graphicsQueueFamilyIndex;
	uint32_t				presentQueueFamilyIndex;

	VkDevice				device;

	// use 1 sample
	const VkSampleCountFlagBits SampleCount = VK_SAMPLE_COUNT_1_BIT;

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
	
	void CreateSwapchain();
	// Find queue family that supports graphics and present
	void FindQueueFamilyIndices();
	void FindSupportedFormats();
	void SetSurfaceCapabilities(VkSwapchainCreateInfoKHR &swapchainCreateInfo);
	void CreateSwapchainImages();

	void CreateDepthBuffer();
	bool FindMemoryType(uint32_t memoryTypeBits, VkFlags requirementsMask, uint32_t& result);

	void MainLoop();

	void Cleanup();
	void DestroyWindow();
	void DestroyInstance();
	void DestroyDevice();
	void DestroySwapchain();
	void DestroyDepthBuffer();

public:
	void Start();
};