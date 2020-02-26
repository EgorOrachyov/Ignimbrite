cmake_minimum_required(VERSION 3.11)
project(Ignimbrite)

# Library sources
set(Ignimbrite_SOURCES
    IncludeStd.h
    IncludeMath.h
    Compilation.h
    RenderDeviceDefinitions.h
    RenderDevice.h
    FileUtils.h
    ObjectID.h
    ObjectIDBuffer.h
    Optional.h
    Platform.h
    AABB.h
    Types.h
    ShaderReflection.h
    Shader.h
    ShaderReflection.h
    Texture.h
    Sampler.h
    UniformBuffer.h
    RenderDevice.cpp
    FileUtils.cpp
    Cache.cpp
    Cache.h
    CacheItem.cpp
    CacheItem.h
    Shader.cpp
    ShaderReflection.cpp
    Texture.cpp
    Sampler.cpp
    UniformBuffer.cpp
    Material.cpp
    Material.h
    IRenderable.cpp
    IRenderable.h
        )

add_library(Ignimbrite STATIC ${Ignimbrite_SOURCES})

target_include_directories(Ignimbrite PUBLIC .)
target_link_libraries(Ignimbrite PRIVATE spirv-cross-core)
target_link_libraries(Ignimbrite PUBLIC glm)