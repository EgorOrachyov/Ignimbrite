########################## Vulkan Renderer ##########################

cmake_minimum_required(VERSION 3.11)
project(VulkanRenderer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_GENERATOR_PLATFORM x64)

set(THIRD_PARTY_LIBS)
set(SOURCES)

# Include

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/thirdparty/include)

# Lib

if (WIN32)
    if (${CMAKE_GENERATOR_PLATFORM} MATCHES "x64")
        link_directories(${CMAKE_SOURCE_DIR}/thirdparty/lib/win64)
    else()
        link_directories(${CMAKE_SOURCE_DIR}/thirdparty/lib/win32)
    endif()
endif (WIN32)

if (UNIX)
    link_directories(${CMAKE_SOURCE_DIR}/thirdparty/lib/linux64)
endif (UNIX)

set(THIRD_PARTY_LIBS ${THIRD_PARTY_LIBS} glfw3 vulkan-1)

# Source

set(SOURCES ${SOURCES} source/main.cpp)
set(SOURCES ${SOURCES} source/VulkanTriangle.cpp)
set(SOURCES ${SOURCES} source/ValidationLayers.cpp)
set(SOURCES ${SOURCES} source/UniformBuffer.cpp)
set(SOURCES ${SOURCES} source/Shader.cpp)

add_executable(VulkanRenderer ${SOURCES})
target_link_libraries(VulkanRenderer ${THIRD_PARTY_LIBS})
