cmake_minimum_required(VERSION 3.11)
project(VulkanDevice)

add_library(VulkanDevice STATIC ..)
message(STATUS "Build VulkanDevice library")

set_property(TARGET Renderer PROPERTY CXX_STANDARD 11)
set_property(TARGET Renderer PROPERTY CXX_STANDARD_REQUIRED ON)

target_include_directories(VulkanDevice PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include)

target_include_directories(VulkanDevice PRIVATE ${Renderer_SOURCE_DIRS}/include/)
target_link_libraries(VulkanDevice PRIVATE Renderer)

target_sources(VulkanDevice
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include/VulkanRenderDevice.h
        ${CMAKE_CURRENT_LIST_DIR}/include/VulkanDefinitions.h
        ${CMAKE_CURRENT_LIST_DIR}/include/VulkanContext.h
        ${CMAKE_CURRENT_LIST_DIR}/include/VulkanErrors.h
        ${CMAKE_CURRENT_LIST_DIR}/include/VulkanUtils.h
        ${CMAKE_CURRENT_LIST_DIR}/include/VulkanExtensions.h
        ${CMAKE_CURRENT_LIST_DIR}/include/VulkanObjects.h
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/sources/VulkanRenderDevice.cpp
        ${CMAKE_CURRENT_LIST_DIR}/sources/VulkanContext.cpp
        ${CMAKE_CURRENT_LIST_DIR}/sources/VulkanExtensions.cpp
        ${CMAKE_CURRENT_LIST_DIR}/sources/VulkanUtils.cpp
)

