cmake_minimum_required(VERSION 3.11)
project(Tests)

add_executable(TestObjectID TestObjectID.cpp)
target_link_libraries(TestObjectID PRIVATE Ignimbrite)

if (WITH_GLFW)
    add_executable(TestGlfwWindow TestGlfwWindow.cpp)
    target_link_libraries(TestGlfwWindow PRIVATE Ignimbrite)
    target_link_libraries(TestGlfwWindow PRIVATE glfw)
endif()

if (WITH_VULKAN AND WITH_GLFW)
    add_executable(TestVulkanApplication TestVulkanApplication.cpp)
    target_link_libraries(TestVulkanApplication PRIVATE Ignimbrite)
    target_link_libraries(TestVulkanApplication PRIVATE VulkanDevice)
    target_link_libraries(TestVulkanApplication PRIVATE glfw)
endif()

