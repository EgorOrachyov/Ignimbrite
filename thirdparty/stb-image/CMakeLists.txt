cmake_minimum_required(VERSION 3.11)
project(STB_IMAGE VERSION 2.23)

set(StbImage_HEADERS include/stb_image.h)
set(StbImage_SOURCES sources/stb_image.cpp)

add_library(stb-image STATIC ${StbImage_HEADERS} ${StbImage_SOURCES})
target_include_directories(stb-image PUBLIC include)