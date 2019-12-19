cmake_minimum_required(VERSION 3.11)
project(TinyObjLoader VERSION 1.0)

set(tinyobjloader_HEADERS include/tiny_obj_loader.h)
set(tinyobjloader_SOURCES sources/tiny_obj_loader.cpp)

add_library(tinyobjloader STATIC ${tinyobjloader_HEADERS} ${tinyobjloader_SOURCES})
target_include_directories(tinyobjloader PUBLIC include)