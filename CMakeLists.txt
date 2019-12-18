##################################################################
## Overall project setup

cmake_minimum_required(VERSION 3.11)
project(IgnimbriteLibrary VERSION 1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -g")

##################################################################
## Compiler specific setup

set(gcc_like_cxx "CXX" "ARMClang" "AppleClang" "Clang" "GNU")
set(msvc_cxx "CXX" "MSVC")

if ("${CMAKE_CXX_COMPILER_ID}" IN_LIST gcc_like_cxx)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wextra -Wshadow -Wformat=2 -Wunused")
elseif ("${CMAKE_CXX_COMPILER_ID}" IN_LIST msvc_cxx)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -W3")
endif ()

##################################################################
## Third-party setup

option(WITH_GLFW "GLFW Window system" NO)
if (WITH_GLFW)
    message(STATUS "Using GLFW as window system API")
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
    add_subdirectory(thirdparty/glfw)
endif()

option(WITH_VULKAN "Vulkan rendering API" NO)
if (WITH_VULKAN)
    message(STATUS "Using Vulkan as rendering API")
    add_subdirectory(thirdparty/vulkan)
    add_subdirectory(device/vulkan)
endif()

option(WITH_GLM "GLM math header library" NO)
option(WITH_SPIRVCROSS "SPIR-V Cross" NO)

##################################################################
## Include actual sub directories

add_subdirectory(ignimbrite)


option(WITH_TESTS "Compile tests and examples for library" OFF)
if (WITH_TESTS)
    add_subdirectory(tests)
endif()

##################################################################
## Configure project options

configure_file(
    ${PROJECT_SOURCE_DIR}/ignimbrite/include/ignimbrite/Options.h.in
    ${PROJECT_SOURCE_DIR}/ignimbrite/include/ignimbrite/Options.h
)