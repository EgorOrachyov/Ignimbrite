cmake_minimum_required(VERSION 3.11)
project(Tests)

add_executable(Tests "")

set_property(TARGET Tests PROPERTY CXX_STANDARD 11)
set_property(TARGET Tests PROPERTY CXX_STANDARD_REQUIRED ON)

include_directories(${Renderer_SOURCE_DIR}/include)
target_link_libraries(Tests Renderer)

target_sources(Tests
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/TestObjectID.h
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/main.cpp
)