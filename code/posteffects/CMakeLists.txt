cmake_minimum_required(VERSION 3.11)
project(IgnimbritePostEffects)

set(IgnimbritePostEffects_SOURCES
    InverseFilter.cpp
    InverseFilter.h
    NoirFilter.cpp
    NoirFilter.h
        )

add_library(IgnimbritePostEffects STATIC ${IgnimbritePostEffects_SOURCES})
target_include_directories(IgnimbritePostEffects PUBLIC .)
target_link_libraries(IgnimbritePostEffects PUBLIC Ignimbrite)
target_link_libraries(IgnimbritePostEffects PUBLIC IgnimbriteMaterials)
