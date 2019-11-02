cmake_minimum_required(VERSION 3.11)
project(VulkanDevice)

target_sources(VulkanDevice
    PUBLIC
        include/VulkanRenderDevice.h
    PRIVATE
        sources/VulkanRenderDevice.cpp
)

add_library(VulkanDevice STATIC)