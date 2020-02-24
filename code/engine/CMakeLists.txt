cmake_minimum_required(VERSION 3.11)
project(Ignimbrite)

# Library sources
set(Ignimbrite_SOURCES
    include/ignimbrite/Compilation.h
    include/ignimbrite/RenderDeviceDefinitions.h
    include/ignimbrite/RenderDevice.h
    include/ignimbrite/FileUtils.h
    include/ignimbrite/ObjectID.h
    include/ignimbrite/ObjectIDBuffer.h
    include/ignimbrite/Optional.h
    include/ignimbrite/Platform.h
    include/ignimbrite/Types.h
    include/ignimbrite/ShaderReflection.h
    include/ignimbrite/CacheItem.h
    include/ignimbrite/Shader.h
    include/ignimbrite/ShaderReflection.h
    include/ignimbrite/Texture.h
    include/ignimbrite/Sampler.h

    RenderDevice.cpp
    FileUtils.cpp
    Shader.cpp
    ShaderReflection.cpp
    Texture.cpp
    Sampler.cpp
        )

add_library(Ignimbrite STATIC ${Ignimbrite_SOURCES})

# Public interface
target_include_directories(Ignimbrite PUBLIC include)
# For private includes
target_include_directories(Ignimbrite PRIVATE .)

target_link_libraries(Ignimbrite PUBLIC spirv-cross-core)