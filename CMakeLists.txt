########################## Vulkan Renderer ##########################

cmake_minimum_required(VERSION 3.11)
project(VulkanRenderer)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_GENERATOR_PLATFORM x64)

set(THIRD_PARTY_LIBS)
set(SOURCES)

########################## Headers paths ##########################

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/thirdparty)
include_directories(${PROJECT_SOURCE_DIR}/thirdparty/Glfw/include)
include_directories(${PROJECT_SOURCE_DIR}/thirdparty/Vulkan/include)

########################## Vulkan GLFW lib ##########################

find_library(GLFW_LIB glfw PATHS "${PROJECT_SOURCE_DIR}/thirdparty/Glfw/lib")
set(THIRD_PARTY_LIBS ${THIRD_PARTY_LIBS} ${GLFW_LIB})

########################## Vulkan SDK lib ##########################

find_library(VULKAN_LIB vulkan PATHS "${PROJECT_SOURCE_DIR}/thirdparty/Vulkan/lib")
set(THIRD_PARTY_LIBS ${THIRD_PARTY_LIBS} ${VULKAN_LIB})

########################## Project h/cpp files ##########################

set(SOURCES ${SOURCES} source/main.cpp)

add_executable(VulkanRenderer ${SOURCES})
target_link_libraries(VulkanRenderer ${THIRD_PARTY_LIBS})
