cmake_minimum_required(VERSION 3.11)
project(VulkanDevice)

set(SOURCES
        VulkanDevice.cpp
        VulkanDevice.h
        VulkanQueueFamilyIndices.h
        VulkanContext.cpp
        VulkanContext.h)

add_library(VulkanDevice STATIC ${SOURCES})
