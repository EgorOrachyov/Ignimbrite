##################################################################
## Overall project setup

cmake_minimum_required(VERSION 3.11)
project(RenderingLibrary)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -g")

##################################################################
## Platform specific setup

set(THIRD_PARTY_LIBS)
set(THIRD_PARTY_LIBS_PATH)

# find_package(Vulkan REQUIRED) ???

if(APPLE)
    message(STATUS "Apple")
    list(APPEND THIRD_PARTY_LIBS vulkan glfw)
    set(THIRD_PARTY_LIBS_PATH ${PROJECT_SOURCE_DIR}/thirdparty/lib/apple)
elseif(WIN32)
    list(APPEND THIRD_PARTY_LIBS glfw)
    set(APPEND THIRD_PARTY_LIBS_PATH ${PROJECT_SOURCE_DIR}/thirdparty/lib/win32)
elseif(UNIX)
    list(APPEND THIRD_PARTY_LIBS vulkan glfw)
    set(APPEND THIRD_PARTY_LIBS_PATH ${PROJECT_SOURCE_DIR}/thirdparty/lib/unix)
endif()

link_directories(${THIRD_PARTY_LIBS_PATH})

##################################################################
## Third-party setup

option(WITH_GLFW "GLFW Window system" YES)
if (WITH_GLFW)
    if (APPLE)
        set(GLFW_INCLUDE "${PROJECT_SOURCE_DIR}/thirdparty/include")
        include_directories(${GLFW_INCLUDE})
    else()
        set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
        set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
        set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

        set(GLFW_INCLUDE "${PROJECT_SOURCE_DIR}/thirdparty/lib/unix/glfw/include")
        include_directories(${GLFW_INCLUDE})
        add_subdirectory(${PROJECT_SOURCE_DIR}/thirdparty/lib/unix/glfw)
    endif()
endif()

option(WITH_VULKAN "Vulkan rendering API" YES)
if (WITH_VULKAN)
    message(STATUS "Using Vulkan as rendering API")
    set(VULKAN_INCLUDE "${PROJECT_SOURCE_DIR}/thirdparty/include")
    include_directories(${VULKAN_INCLUDE})
endif()

option(WITH_GLM "GLM math header library" NO)
if (WITH_GLM)
    message(STATUS "Using Vulkan API")
    set(GLM_INCLUDE "${PROJECT_SOURCE_DIR}/thirdparty/include")
    include_directories(${GLM_INCLUDE})
endif()

option(WITH_SPIRV_CROSS "SPIR-V Cross" YES)
if (WITH_SPIRV_CROSS)
    message(STATUS "Using SPIR-V Cross")

    set(SPIRV_CROSS_CLI OFF CACHE BOOL "" FORCE)
    set(SPIRV_CROSS_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
    set(SPIRV_CROSS_INCLUDE "${PROJECT_SOURCE_DIR}/thirdparty/SPIRV-Cross")
    include_directories(${SPIRV_CROSS_INCLUDE})
    add_subdirectory("${PROJECT_SOURCE_DIR}/thirdparty/SPIRV-Cross")
endif()

##################################################################
## Configure project options

configure_file(
    ${PROJECT_SOURCE_DIR}/renderer/include/renderer/Options.h.in
    ${PROJECT_SOURCE_DIR}/renderer/include/renderer/Options.h
)

##################################################################
## Include actual sub directories

add_subdirectory(renderer)
add_subdirectory(device)
add_subdirectory(tests)