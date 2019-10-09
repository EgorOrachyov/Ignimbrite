#pragma once
#include <vulkan/vulkan.h>
#include <vector>

class ValidationLayers
{
private:
	VkDebugUtilsMessengerEXT debugMessenger;

public:
	const std::vector<const char*> ValidationLayerNames = { "VK_LAYER_KHRONOS_validation" };

private:
	bool CheckValidationLayerSupport();
	void SetDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
	
	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

public:
	void Check();
	void Destroy(VkInstance& vkInstance);

	void SetupDebugMessenger(VkInstance &vkInstance);
	
	void SetValidationLayersForInstance(VkInstanceCreateInfo& instanceCreateInfo, VkDebugUtilsMessengerCreateInfoEXT& debugCreateInfo);
	void SetValidationLayersForDevice(VkDeviceCreateInfo& deviveCreateInfo);
	
	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
};