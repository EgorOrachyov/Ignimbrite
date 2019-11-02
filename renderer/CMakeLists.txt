cmake_minimum_required(VERSION 3.11)
project(Renderer)

add_library(Renderer STATIC "")

set_property(TARGET Renderer PROPERTY CXX_STANDARD 11)
set_property(TARGET Renderer PROPERTY CXX_STANDARD_REQUIRED ON)

target_include_directories(Renderer PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_sources(Renderer
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include/renderer/Compilation.h
        ${CMAKE_CURRENT_LIST_DIR}/include/renderer/DeviceDefinitions.h
        ${CMAKE_CURRENT_LIST_DIR}/include/renderer/RenderDevice.h
        ${CMAKE_CURRENT_LIST_DIR}/include/renderer/FileUtils.h
        ${CMAKE_CURRENT_LIST_DIR}/include/renderer/ObjectID.h
        ${CMAKE_CURRENT_LIST_DIR}/include/renderer/ObjectIDBuffer.h
        ${CMAKE_CURRENT_LIST_DIR}/include/renderer/Optional.h
        ${CMAKE_CURRENT_LIST_DIR}/include/renderer/Platform.h
        ${CMAKE_CURRENT_LIST_DIR}/include/renderer/Types.h
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/sources/RenderDevice.cpp
        ${CMAKE_CURRENT_LIST_DIR}/sources/FileUtils.cpp
)

