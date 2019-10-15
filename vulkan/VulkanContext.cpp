//
// Created by Egor Orachyov on 2019-10-15.
//

#include "VulkanContext.h"
#include <Platform.h>

VulkanContext::VulkanContext(std::string appName, bool enableValidation, uint32 extensionsCount,
                             const char *const *extensions)
        : mApplicationName(std::move(appName)),
          mEnableValidationLayers(enableValidation) {
    _fillRequiredExt(extensionsCount, extensions);
    _createInstance();
    _setupDebugMessenger();
}

VulkanContext::~VulkanContext() {
    _destroyDebugMessenger();
    _destroyInstance();
}

void VulkanContext::_createInstance() {
    /** General application info */
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = mApplicationName.c_str();
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = (uint32) mRequiredExtensions.size();
    createInfo.ppEnabledExtensionNames = mRequiredExtensions.data();

    /** Validation layers check*/
    if (mEnableValidationLayers) {
        if (_checkValidationLayers()) {
            createInfo.enabledLayerCount = (uint32) mValidationLayers.size();
            createInfo.ppEnabledLayerNames = mValidationLayers.data();
        } else {
            throw std::runtime_error("Required validation layer is not available");
        }
    } else {
        createInfo.enabledLayerCount = 0;
        createInfo.ppEnabledLayerNames = nullptr;
    }

    /** Debug extensions info check and output */
    _checkSupportedExtensions();

    VkResult result = vkCreateInstance(&createInfo, nullptr, &mInstance);

    if (result != VK_SUCCESS) {
        throw std::runtime_error("Cannot create VK instance");
    }
}

void VulkanContext::_destroyInstance() {
    vkDestroyInstance(mInstance, nullptr);
}

void VulkanContext::_checkSupportedExtensions() {
    uint32 extensionsCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, nullptr);

    std::vector<VkExtensionProperties> extensions(extensionsCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, extensions.data());

#ifdef MODE_DEBUG
    printf("Required (count: %u) extensions for vulkan:\n", (uint32) mRequiredExtensions.size());
    for (const auto &e: mRequiredExtensions) {
        printf("%s\n", e);
    }

    printf("Supported (count: %u) extensions by vulkan:\n", (uint32) extensions.size());
    for (const auto &e: extensions) {
        printf("%s\n", e.extensionName);
    }
#endif
}

bool VulkanContext::_checkValidationLayers() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

#ifdef MODE_DEBUG
    printf("Required (count: %u) validation layers for vulkan:\n", (uint32) mValidationLayers.size());
    for (const auto &required: mValidationLayers) {
        printf("%s\n", required);
    }

    printf("Available (count: %u) validation layers by vulkan:\n", layerCount);
    for (const auto &available: availableLayers) {
        printf("%s\n", available.layerName);
    }
#endif

    for (const auto &required: mValidationLayers) {
        bool found = false;
        for (const auto &available: availableLayers) {
            if (std::strcmp(required, available.layerName) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            return false;
        }
    }

    return true;
}

void VulkanContext::_fillRequiredExt(uint32 count, const char *const *ext) {
    if (count > 0) {
        mRequiredExtensions.reserve(count + 1);
        for (uint32 i = 0; i < count; i++) {
            mRequiredExtensions.push_back(ext[i]);
        }
    }

    if (mEnableValidationLayers) {
        mRequiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
}

void VulkanContext::_setupDebugMessenger() {
    if (!mEnableValidationLayers) {
        return;
    }

    VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = _debugCallback;
    createInfo.pUserData = this; // Optional

    if (_createDebugUtilsMessengerEXT(mInstance, &createInfo, nullptr, &mDebugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create debug utils messenger");
    }
}

void VulkanContext::_destroyDebugMessenger() {
    if (!mEnableValidationLayers) {
        return;
    }

    _destroyDebugUtilsMessengerEXT(mInstance, mDebugMessenger, nullptr);
}

VkResult VulkanContext::_createDebugUtilsMessengerEXT(VkInstance instance,
                                                      const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                                      const VkAllocationCallbacks *pAllocator,
                                                      VkDebugUtilsMessengerEXT *pDebugMessenger) {
    auto pFunction = (PFN_vkCreateDebugUtilsMessengerEXT)
            vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

    if (pFunction != nullptr) {
        return pFunction(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void VulkanContext::_destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                                   const VkAllocationCallbacks *pAllocator) {
    auto pFunction = (PFN_vkDestroyDebugUtilsMessengerEXT)
            vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (pFunction != nullptr) {
        pFunction(instance, debugMessenger, pAllocator);
    } else {
        throw std::runtime_error("Cannot load \"vkDestroyDebugUtilsMessengerEXT\" function");
    }
}

VkBool32 VulkanContext::_debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                       VkDebugUtilsMessageTypeFlagsEXT messageType,
                                       const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                       void *pUserData) {
    printf("[Vk Validation layer]: %s\n", pCallbackData->pMessage);
    return VK_FALSE;
}