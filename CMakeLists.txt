########################## Vulkan Renderer ##########################

cmake_minimum_required(VERSION 3.11)
project(VulkanRenderer)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_GENERATOR_PLATFORM x64)

set(THIRD_PARTY_LIBS)
set(SOURCES)

########################## Headers paths ##########################

include_directories(${PROJECT_SOURCE_DIR}/thirdparty/include)
include_directories(${PROJECT_SOURCE_DIR}/renderDevice)
include_directories(${PROJECT_SOURCE_DIR}/vulkanDevice)
include_directories(${PROJECT_SOURCE_DIR}/tests)

########################## GLFW lib ##########################

link_directories(${CMAKE_SOURCE_DIR}/thirdparty/lib)
set(THIRD_PARTY_LIBS ${THIRD_PARTY_LIBS} vulkan glfw)

########################## Project h/cpp files ##########################

set(SOURCES
        ${SOURCES}
        tests/main.cpp
        renderDevice/renderDevice.h
        vulkanDevice/vulkanRenderDevice.h
        renderDevice/objectID.h
        renderDevice/objectIDBuffer.h
)

add_executable(VulkanRenderer ${SOURCES})
target_link_libraries(VulkanRenderer ${THIRD_PARTY_LIBS})
