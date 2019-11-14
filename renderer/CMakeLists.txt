cmake_minimum_required(VERSION 3.1)
project(Renderer)

add_library(Renderer STATIC "")
message(STATUS "Build Renderer library")

target_include_directories(Renderer PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include/)

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

