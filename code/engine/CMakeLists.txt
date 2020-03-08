cmake_minimum_required(VERSION 3.11)
project(Ignimbrite)

# Library sources
set(Ignimbrite_SOURCES
    IncludeStd.h
    IncludeMath.h
    Compilation.h
    IRenderDevice.cpp
    IRenderDevice.h
    IRenderDeviceDefinitions.h
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
    Mesh.cpp
    Mesh.h
    MeshLoader.h
    MeshLoader.cpp
    RenderTarget.cpp
    RenderTarget.h
    Frustum.h)

add_library(Ignimbrite STATIC ${Ignimbrite_SOURCES})

target_include_directories(Ignimbrite PUBLIC .)
target_link_libraries(Ignimbrite PRIVATE spirv-cross-core)
target_link_libraries(Ignimbrite PUBLIC glm)
target_link_libraries(Ignimbrite PUBLIC tinyobjloader)