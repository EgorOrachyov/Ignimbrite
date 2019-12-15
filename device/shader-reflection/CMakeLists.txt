cmake_minimum_required(VERSION 3.11)
project(VShaderReflection)

set(VShaderReflection_HEADERS
        )

set(VShaderReflection_SOURCES
        )

message(STATUS "Config VShaderReflection library")
add_library(VShaderReflection STATIC ${VShaderReflection_HEADERS} ${VShaderReflection_SOURCES})

target_include_directories(VShaderReflection PUBLIC include)

target_link_libraries(VShaderReflection PUBLIC
        Renderer)

target_link_libraries(VShaderReflection PUBLIC
        spirv-cross-glsl
        spirv-cross-hlsl
        spirv-cross-cpp
        spirv-cross-reflect
        spirv-cross-msl
        spirv-cross-util
        spirv-cross-core)