#################################################################
# Overall project setup

cmake_minimum_required(VERSION 3.11)
project(RenderingLibrary)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -O0 -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -g")

#################################################################
# Platform specific setup

set(THIRD_PARTY_LIBS)
set(THIRD_PARTY_LIBS_PATH)

if(APPLE)
    list(APPEND THIRD_PARTY_LIBS vulkan glfw)
    list(APPEND THIRD_PARTY_LIBS_PATH ${CMAKE_SOURCE_DIR}/thirdparty/lib/apple)
elseif(WIN32)
    list(APPEND THIRD_PARTY_LIBS ${THIRD_PARTY_LIBS} vulkan-1 glfw3)
    list(APPEND THIRD_PARTY_LIBS_PATH ${CMAKE_SOURCE_DIR}/thirdparty/lib/win32)
elseif(UNIX)
    list(APPEND THIRD_PARTY_LIBS ${THIRD_PARTY_LIBS} vulkan glfw)
    list(APPEND THIRD_PARTY_LIBS_PATH ${CMAKE_SOURCE_DIR}/thirdparty/lib/unix)
endif()

#################################################################
# Third-party setup

option(WITH_GLFW "GLFW Window system" YES)
if (WITH_GLFW)
    message(STATUS "Using GLFW as window system")
    set(GLFW_INCLUDE "${CMAKE_CURRENT_LIST_DIR}/thirdparty/include")
endif()

option(WITH_VULKAN "Vulkan rendering API" YES)
if (WITH_VULKAN)
    message(STATUS "Using Vulkan as rendering API")
    set(VULKAN_INCLUDE "${CMAKE_CURRENT_LIST_DIR}/thirdparty/include")
endif()

option(WITH_GLM "GLM math header library" NO)
if (WITH_GLM)
    message(STATUS "Using Vulkan API")
    set(GLM_INCLUDE "${CMAKE_CURRENT_LIST_DIR}/thirdparty/include")
endif()

#################################################################
# Include actual sub directories

add_subdirectory(renderer)
add_subdirectory(device)

enable_testing()
add_subdirectory(tests)