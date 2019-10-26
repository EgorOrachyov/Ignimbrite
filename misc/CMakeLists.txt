cmake_minimum_required(VERSION 3.11)
project(Misc)

set(SOURCES
        Compilation.h
        FileUtils.cpp
        FileUtils.h
        ObjectID.h
        Optional.h
        Platform.h
        Types.h
        ObjectIDBuffer.h)

add_library(Misc STATIC ${SOURCES})