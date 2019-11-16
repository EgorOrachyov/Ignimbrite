cmake_minimum_required(VERSION 3.11)
project(VulkanDevice)

set(VulkanDevice_HEADERS
    include/VulkanRenderDevice.h
    include/VulkanDefinitions.h
    include/VulkanContext.h
    include/VulkanErrors.h
    include/VulkanUtils.h
    include/VulkanExtensions.h
    include/VulkanObjects.h
)

set(VulkanDevice_SOURCES
    sources/VulkanRenderDevice.cpp
    sources/VulkanContext.cpp
    sources/VulkanExtensions.cpp
    sources/VulkanUtils.cpp
)

message(STATUS "Config VulkanDevice library")
add_library(VulkanDevice STATIC ${VulkanDevice_HEADERS} ${VulkanDevice_SOURCES})

target_include_directories(VulkanDevice PUBLIC include)
target_link_libraries(VulkanDevice PUBLIC Renderer)