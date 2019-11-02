cmake_minimum_required(VERSION 3.11)
project(Device)

set(SOURCES
        RenderDevice.h
        RenderDevice.cpp
        DeviceDefinitions.h)

add_library(Device ${SOURCES})