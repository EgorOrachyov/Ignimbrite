cmake_minimum_required(VERSION 3.11)
project(Renderer)

set(Renderer_HEADERS
    include/renderer/Compilation.h
    include/renderer/DeviceDefinitions.h
    include/renderer/RenderDevice.h
    include/renderer/FileUtils.h
    include/renderer/ObjectID.h
    include/renderer/ObjectIDBuffer.h
    include/renderer/Optional.h
    include/renderer/Platform.h
    include/renderer/Types.h
        include/renderer/ShaderReflection.h)

set(Renderer_SOURCES
    sources/RenderDevice.cpp
    sources/FileUtils.cpp
)

message(STATUS "Config Renderer library")
add_library(Renderer STATIC ${Renderer_HEADERS} ${Renderer_SOURCES})
target_include_directories(Renderer PUBLIC include)