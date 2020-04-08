cmake_minimum_required(VERSION 3.11)
project(IgnimbriteMaterials)

set(IgnimbriteMaterials_SOURCES
    VertexLayoutFactory.cpp
    VertexLayoutFactory.h
    MaterialFullscreen.cpp
    MaterialFullscreen.h)

add_library(IgnimbriteMaterials STATIC ${IgnimbriteMaterials_SOURCES})
target_include_directories(IgnimbriteMaterials PUBLIC .)
target_link_libraries(IgnimbriteMaterials PUBLIC Ignimbrite)
