cmake_minimum_required(VERSION 3.11)
project(Ignimbrite)

# Library sources
set(Ignimbrite_SOURCES
    Options.h
    IncludeStd.h
    IncludeMath.h
    Compilation.h
    IRenderable.cpp
    IRenderable.h
    IRenderEngine.cpp
    IRenderEngine.h
    IRenderContext.h
    IRenderDevice.cpp
    IRenderDevice.h
    IRenderDeviceDefinitions.h
    FileUtils.cpp
    FileUtils.h
    ObjectID.h
    ObjectIDBuffer.h
    Optional.h
    Platform.h
    AABB.h
    Types.h
    Shader.cpp
    Shader.h
    ShaderReflection.cpp
    ShaderReflection.h
    Texture.cpp
    Texture.h
    Sampler.cpp
    Sampler.h
    UniformBuffer.cpp
    UniformBuffer.h
    Cache.cpp
    Cache.h
    CacheItem.cpp
    CacheItem.h
    GraphicsPipeline.cpp
    GraphicsPipeline.h
    PipelineContext.cpp
    PipelineContext.h
    Material.cpp
    Material.h
    Mesh.cpp
    Mesh.h
    MeshLoader.cpp
    MeshLoader.h
    RenderTarget.cpp
    RenderTarget.h
    Frustum.h
    Light.cpp 
    Light.h
    Camera.cpp
    Camera.h
    RenderableMesh.cpp
    RenderableMesh.h
    RenderEngine.cpp
    RenderEngine.h
    RenderQueueElement.h
    IPostEffect.h
    IPostEffect.h 
    PresentationPass.cpp 
    PresentationPass.h 
    IPresentationPass.h
    Geometry.cpp
    Geometry.h
    Canvas.cpp
    Canvas.h)

add_library(Ignimbrite STATIC ${Ignimbrite_SOURCES})

target_include_directories(Ignimbrite PUBLIC .)
target_link_libraries(Ignimbrite PRIVATE spirv-cross-core)
target_link_libraries(Ignimbrite PRIVATE tinyobjloader)
target_link_libraries(Ignimbrite PUBLIC glm)