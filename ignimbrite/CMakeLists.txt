cmake_minimum_required(VERSION 3.11)
project(Ignimbrite)

message(STATUS "Configure Ignimbrite library")

set(Ignimbrite_HEADERS
    include/ignimbrite/Compilation.h
    include/ignimbrite/DeviceDefinitions.h
    include/ignimbrite/RenderDevice.h
    include/ignimbrite/FileUtils.h
    include/ignimbrite/ObjectID.h
    include/ignimbrite/ObjectIDBuffer.h
    include/ignimbrite/Optional.h
    include/ignimbrite/Platform.h
    include/ignimbrite/Types.h
    include/ignimbrite/ShaderReflection.h
)

set(Ignimbrite_SOURCES
    sources/RenderDevice.cpp
    sources/FileUtils.cpp
)

add_library(Ignimbrite
    STATIC
    ${Ignimbrite_HEADERS}
    ${Ignimbrite_SOURCES}
)

target_include_directories(Ignimbrite PUBLIC include)