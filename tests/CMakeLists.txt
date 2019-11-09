cmake_minimum_required(VERSION 3.11)
project(Tests)

add_executable(Tests "" VulkanStartUp.h)
message(STATUS "Build Tests executable")

set_property(TARGET Tests PROPERTY CXX_STANDARD 11)
set_property(TARGET Tests PROPERTY CXX_STANDARD_REQUIRED ON)

target_include_directories(Tests PRIVATE ${Renderer_SOURCE_DIR}/include)
target_include_directories(Tests PRIVATE ${VulkanDevice_SOURCE_DIR}/include)

target_link_libraries(Tests PRIVATE Renderer)
target_link_libraries(Tests PRIVATE VulkanDevice)

variable_watch(THIRD_PARTY_LIBS)
target_link_libraries(Tests PRIVATE ${THIRD_PARTY_LIBS})

target_sources(Tests
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/TestObjectID.h
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/main.cpp
)

add_test(NAME Tests COMMAND Tests)