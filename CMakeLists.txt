########################## Vulkan Renderer ##########################

cmake_minimum_required(VERSION 3.11)
project(VulkanRenderer)

set(CMAKE_CXX_STANDARD 11)

set(POJECT_LIBS)
set(THIRD_PARTY_LIBS)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -Wall")

########################## Headers paths ##########################

include_directories(${PROJECT_SOURCE_DIR}/thirdparty/include)
include_directories(${PROJECT_SOURCE_DIR}/misc)
include_directories(${PROJECT_SOURCE_DIR}/device)
include_directories(${PROJECT_SOURCE_DIR}/vulkan)
include_directories(${PROJECT_SOURCE_DIR}/tests)

########################## Third-party ##########################

if(APPLE)
    set(THIRD_PARTY_LIBS ${THIRD_PARTY_LIBS} vulkan glfw)
    link_directories(${CMAKE_SOURCE_DIR}/thirdparty/lib/apple)
elseif(WIN32)
    set(THIRD_PARTY_LIBS ${THIRD_PARTY_LIBS} vulkan-1 glfw3)
    link_directories(${CMAKE_SOURCE_DIR}/thirdparty/lib/win32)
elseif(UNIX)
    set(THIRD_PARTY_LIBS ${THIRD_PARTY_LIBS} vulkan glfw)
    link_directories(${CMAKE_SOURCE_DIR}/thirdparty/lib/unix)
endif()

########################## Project h/cpp files ##########################

set(PROJECT_LIBS Misc)

add_subdirectory(thirdparty/include/glm)
add_subdirectory(misc)
add_subdirectory(device)
add_subdirectory(vulkan)
add_subdirectory(tests)
