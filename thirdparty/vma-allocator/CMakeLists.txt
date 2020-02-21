cmake_minimum_required(VERSION 3.11)
project(VMAAllocator VERSION 2.3.0)

set(VmaAlloc_HEADERS include/vk_mem_alloc.h)
set(VmaAlloc_SOURCES sources/vk_mem_alloc.cpp)

add_library(vma-alloc STATIC ${VmaAlloc_HEADERS} ${VmaAlloc_SOURCES})
target_include_directories(vma-alloc PUBLIC include)
target_link_libraries(vma-alloc PRIVATE Vulkan)