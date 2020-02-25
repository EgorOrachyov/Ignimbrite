cmake_minimum_required(VERSION 3.11)
project(Ignimbrite)

# Library sources
set(Ignimbrite_SOURCES
    Compilation.h
    RenderDeviceDefinitions.h
    RenderDevice.h
    FileUtils.h
    ObjectID.h
    ObjectIDBuffer.h
    Optional.h
    Platform.h
    Types.h
    ShaderReflection.h
    CacheItem.h
    Shader.h
    ShaderReflection.h
    Texture.h
    Sampler.h
    UniformBuffer.h
    RenderDevice.cpp
    FileUtils.cpp
    Shader.cpp
    ShaderReflection.cpp
    Texture.cpp
    Sampler.cpp
    UniformBuffer.cpp
        )

add_library(Ignimbrite STATIC ${Ignimbrite_SOURCES})

target_include_directories(Ignimbrite PUBLIC .)
target_link_libraries(Ignimbrite PRIVATE spirv-cross-core)