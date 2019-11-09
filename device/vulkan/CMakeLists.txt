cmake_minimum_required(VERSION 3.11)
project(VulkanDevice)

add_library(VulkanDevice STATIC ..  )
message(STATUS "Build VulkanDevice library")

target_include_directories(VulkanDevice PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_include_directories(VulkanDevice PRIVATE ${Renderer_SOURCE_DIR}/include)
target_link_libraries(VulkanDevice PRIVATE Renderer)

target_sources(VulkanDevice
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include/VulkanRenderDevice.h
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/sources/VulkanRenderDevice.cpp
        ${CMAKE_CURRENT_LIST_DIR}/include/VulkanDefinitions.h
        ${CMAKE_CURRENT_LIST_DIR}/include/VulkanContext.h
        ${CMAKE_CURRENT_LIST_DIR}/sources/VulkanContext.cpp
        ${CMAKE_CURRENT_LIST_DIR}/include/VulkanErrors.h
        ${CMAKE_CURRENT_LIST_DIR}/include/VulkanStructures.h
)

