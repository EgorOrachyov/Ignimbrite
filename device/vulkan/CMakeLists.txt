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
    include/VulkanDescriptorAllocator.h
    include/VulkanSurface.h
    include/VulkanFramebuffer.h
        include/VulkanDrawList.h
    include/VulkanFence.h
    include/VulkanSemaphore.h
)

set(VulkanDevice_SOURCES
    sources/VulkanRenderDevice.cpp
    sources/VulkanContext.cpp
    sources/VulkanExtensions.cpp
    sources/VulkanUtils.cpp
    sources/VulkanDescriptorAllocator.cpp
    sources/VulkanSurface.cpp
)

add_library(
    VulkanDevice
    STATIC
    ${VulkanDevice_HEADERS}
    ${VulkanDevice_SOURCES}
)

target_include_directories(VulkanDevice PUBLIC include)

target_link_libraries(VulkanDevice PUBLIC Ignimbrite)
target_link_libraries(VulkanDevice PUBLIC Vulkan)

if (WITH_GLFW)
    target_link_libraries(VulkanDevice PUBLIC glfw)
endif()

if (WITH_IGNIMBRITE_QT)
    target_link_libraries(VulkanDevice PRIVATE Qt5::Core Qt5::Gui)
endif()
