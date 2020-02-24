cmake_minimum_required(VERSION 3.11)
project(Ignimbrite)

# Public interface headers for the library
# Anyone who want to use this library, may only access this public header
set(Ignimbrite_HEADERS
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
        )

# Library sources and private files
# which are not exported to library users
set(Ignimbrite_SOURCES
    RenderDevice.cpp
    FileUtils.cpp
    Shader.cpp
    ShaderReflection.cpp
        )

add_library(Ignimbrite
    STATIC
    ${Ignimbrite_HEADERS}
    ${Ignimbrite_SOURCES}
)

# Public interface
target_include_directories(Ignimbrite PUBLIC include)
# For private includes
target_include_directories(Ignimbrite PRIVATE .)