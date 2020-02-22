cmake_minimum_required(VERSION 3.11)
project(VulkanDevice)

set(VulkanDevice_HEADERS
    VulkanRenderDevice.h
    VulkanDefinitions.h
    VulkanContext.h
    VulkanErrors.h
    VulkanUtils.h
    VulkanExtensions.h
    VulkanObjects.h
    VulkanDescriptorAllocator.h
    VulkanSurface.h
    VulkanFramebuffer.h
    VulkanDrawList.h
    VulkanFence.h
    VulkanSemaphore.h
)

set(VulkanDevice_SOURCES
    VulkanRenderDevice.cpp
    VulkanContext.cpp
    VulkanExtensions.cpp
    VulkanUtils.cpp
    VulkanDescriptorAllocator.cpp
    VulkanSurface.cpp
)

add_library(
    VulkanDevice
    STATIC
    ${VulkanDevice_HEADERS}
    ${VulkanDevice_SOURCES}
)

target_include_directories(VulkanDevice PUBLIC .)

target_link_libraries(VulkanDevice PRIVATE Ignimbrite)
target_link_libraries(VulkanDevice PUBLIC Vulkan)
target_link_libraries(VulkanDevice PUBLIC vma-alloc)

if (IGNIMBRITE_WITH_GLFW)
    target_link_libraries(VulkanDevice PRIVATE glfw)
endif()

if (IGNIMBRITE_WITH_QT)
    target_link_libraries(VulkanDevice PRIVATE Qt5::Core Qt5::Gui)
endif()
