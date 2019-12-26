##################################################################
## Overall project setup

cmake_minimum_required(VERSION 3.11)
project(IgnimbriteLibrary VERSION 1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -g")
set(CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/cmake)

message(STATUS "${CMAKE_PREFIX_PATH}")

##################################################################
## Compiler specific setup

set(gcc_like_cxx "CXX" "ARMClang" "AppleClang" "Clang" "GNU")
set(msvc_cxx "CXX" "MSVC")

if ("${CMAKE_CXX_COMPILER_ID}" IN_LIST gcc_like_cxx)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wextra -Wshadow -Wformat=2 -Wunused")
elseif ("${CMAKE_CXX_COMPILER_ID}" IN_LIST msvc_cxx)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}") # -W3
endif ()

##################################################################
## Third-party setup

include_directories(thirdparty/test_include)

option(WITH_GLFW "GLFW Window system" NO)
if (WITH_GLFW)
    message(STATUS "Add GLFW as window system API")
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
    add_subdirectory(thirdparty/glfw)
endif()

option(WITH_VULKAN "Vulkan rendering API" NO)
if (WITH_VULKAN)
    message(STATUS "Add Vulkan as rendering API")
    message(STATUS "Looking for Vulkan in $ENV{VULKAN_SDK}")
    find_package(Vulkan REQUIRED)
    add_library(Vulkan INTERFACE)
    target_include_directories(Vulkan INTERFACE ${Vulkan_INCLUDE_DIRS})
    target_link_libraries(Vulkan INTERFACE ${Vulkan_LIBRARIES})
    add_subdirectory(device/vulkan)
endif()

option(WITH_SPIRVCROSS "SPIR-V Cross" YES)
if (WITH_SPIRVCROSS)
    message(STATUS "Add SPIRV-CROSS as reflection tool for spir-v files")
    set(SPIRV_CROSS_STATIC ON CACHE BOOL "" FORCE)
    set(SPIRV_CROSS_ENABLE_CPP ON CACHE BOOL "" FORCE)
    set(SPIRV_CROSS_CLI OFF CACHE BOOL "" FORCE)
    set(SPIRV_CROSS_ENABLE_REFLECT ON CACHE BOOL "" FORCE)
    set(SPIRV_CROSS_ENABLE_GLSL ON CACHE BOOL "" FORCE)
    set(SPIRV_CROSS_ENABLE_HLSL OFF CACHE BOOL "" FORCE)
    set(SPIRV_CROSS_ENABLE_MSL OFF CACHE BOOL "" FORCE)
    set(SPIRV_CROSS_ENABLE_C_API OFF CACHE BOOL "" FORCE)
    add_subdirectory(thirdparty/spirv-cross)
endif()

option(WITH_GLM "GLM math header library" YES)
if (WITH_GLM)
    message(STATUS "Add GLM as math library")
    add_subdirectory(thirdparty/glm)
endif()

option(WITH_STBIMAGE "STB Image utility to load image files" YES)
if (WITH_STBIMAGE)
    message(STATUS "Add STB Image utility to load image files")
    add_subdirectory(thirdparty/stb-image)
endif()

option(WITH_TINYOBJLOADER "Tiny OBJ loader for geometry and materials in .obj .mtl formats" YES)
if (WITH_TINYOBJLOADER)
    message(STATUS "Add Tiny OBJ loader for loading .obj/.mtl data")
    add_subdirectory(thirdparty/tinyobjloader)
endif()

option(WITH_IGNIMBRITE_QT "Use QT to create surface for rendering" NO)
if (WITH_IGNIMBRITE_QT)
    message(STATUS "Add QT for surface creation")
    find_package(Qt5 COMPONENTS Core Gui REQUIRED)
    message(STATUS "Including QT5 directories: ${Qt5Core_INCLUDE_DIRS} ${Qt5Gui_INCLUDE_DIRS}")
    include_directories(${Qt5Core_INCLUDE_DIRS} ${Qt5Gui_INCLUDE_DIRS})
endif()

##################################################################
## Configure project options

configure_file(
    ${PROJECT_SOURCE_DIR}/ignimbrite/include/ignimbrite/Options.h.in
    ${PROJECT_SOURCE_DIR}/ignimbrite/include/ignimbrite/Options.h
)

##################################################################
## Include actual sub directories

message(STATUS "Add Ignimbrite main sub directory")
add_subdirectory(ignimbrite)

option(WITH_TESTS "Compile tests and examples for library" OFF)
if (WITH_TESTS)
    message(STATUS "Add tests folder to build tests")
    add_subdirectory(tests)
endif()
