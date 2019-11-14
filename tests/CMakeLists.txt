cmake_minimum_required(VERSION 3.1)
project(Tests)

add_executable(Tests "")
message(STATUS "Build Tests executable")

target_include_directories(Tests PRIVATE ${Renderer_SOURCE_DIR}/include)
target_include_directories(Tests PRIVATE ${VulkanDevice_SOURCE_DIR}/include)

target_link_libraries(Tests PRIVATE Renderer)
target_link_libraries(Tests PRIVATE VulkanDevice)

variable_watch(THIRD_PARTY_LIBS)
target_link_libraries(Tests PRIVATE ${THIRD_PARTY_LIBS})

target_sources(Tests
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/TestObjectID.h
        ${CMAKE_CURRENT_LIST_DIR}/VulkanStartUp.h
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/main.cpp
)