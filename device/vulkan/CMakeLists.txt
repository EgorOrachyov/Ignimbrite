cmake_minimum_required(VERSION 3.11)
project(VulkanDevice)

add_library(VulkanDevice STATIC)
message(STATUS "Build VulkanDevice library")

target_sources(VulkanDevice
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include/VulkanRenderDevice.h
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/sources/VulkanRenderDevice.cpp
)

variable_watch(VULKAN_INCLUDE)

target_include_directories(VulkanDevice PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(VulkanDevice PUBLIC ${VULKAN_INCLUDE})