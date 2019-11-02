cmake_minimum_required(VERSION 3.11)
project(RenderingLibrary)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -O0 -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -g")

set(THIRD_PARTY_LIBS)

# option(WITH_GLFW "GLFW Window system" YES)
# if (WITH_GLFW)
#     message(STATUS "Use GLFW as window system")
#     set(GLFW_INCLUDE "${CMAKE_CURRENT_LIST_DIR}/thirdparty/include")
# endif()
#
# if(APPLE)
#     set(THIRD_PARTY_LIBS ${THIRD_PARTY_LIBS} vulkan glfw)
#     link_directories(${CMAKE_SOURCE_DIR}/thirdparty/lib/apple)
# elseif(WIN32)
#     set(THIRD_PARTY_LIBS ${THIRD_PARTY_LIBS} vulkan-1 glfw3)
#     link_directories(${CMAKE_SOURCE_DIR}/thirdparty/lib/win32)
# elseif(UNIX)
#     set(THIRD_PARTY_LIBS ${THIRD_PARTY_LIBS} vulkan glfw)
#     link_directories(${CMAKE_SOURCE_DIR}/thirdparty/lib/unix)
# endif()

#################################################################
# Include actual sub directories
# (project modules)
#################################################################

add_subdirectory(renderer)
add_subdirectory(tests)