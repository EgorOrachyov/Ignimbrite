# Ignimbrite

![MacOS build](https://github.com/EgorOrachyov/VulkanRenderer/workflows/MacOS/badge.svg)
![Linux build](https://github.com/EgorOrachyov/VulkanRenderer/workflows/Linux/badge.svg)
![Windows build](https://github.com/EgorOrachyov/VulkanRenderer/workflows/Windows/badge.svg)

C/C++ library for visualization dynamic 3D scene with GPU hardware acceleration via Vulkan API

## Features
* To be done

## Platforms
* Windows
* Linux
* MacOS

## Dependencies
* Vulkan SDK
* GLFW
* GLM
* SPIRV-CROSS

## Get and build 

This project uses git as primary CVS and CMake 3.11 as build configuration tool.
Project language is C++11 with std library 11. The following commands allow to get 
repository and build the minimal library image in /build folder.

```
$ git clone https://github.com/EgorOrachyov/Ignimbrite.git
$ cd Ignimbrite
$ mkdir build
$ cd build
$ cmake .. 
$ cmake --build .
```

### CMake optional flags

We export some optional flags to configure cmake build and final
library image. You can set the following options to get desired functionality:

* WITH_GLFW=(YES/NO) - build glfw library to be able to create glfw based window applications.
* WITH_SPIRVCROSS=(YES/NO) - build spirv-cross tools to automate shaders reflection.
* WITH_VULKAN=(YES/NO) - build vulkan device to be able to use vulkan graphics backend.
* WITH_STBIMAGE=(YES/NO) - use stb image utility to load image data for tests. 
* WITH_GLM=(YES/NO) - use glm as math library for tests
* WITH_TINYOBJLOADER=(YES/NO) - use tiny obj loader to import geometry for tests.
* WITH_TESTS=(YES/NO) - build tests executables with samples and test examples.

By default all the options are set in 'NO', therefore will be built only minimal library image. All the
flags can be configured when cmake configuration build is called:

```
$ cmake .. -DWITH_GLFW=ON -DWITH_VULKAN=ON -DWITH_TESTS=ON -DWITH_SPIRVCROSS=ON
```

### Dependencies setup

We try to keep as much dependencies managed inside project as we can. However, if you want to 
use vulkan as graphics backed, you will have to manually install Vulkan SDK from 
the official LunarG [website](https://www.lunarg.com).

Vulkan SDK [page](https://vulkan.lunarg.com/sdk/home) for Windows, Linux and MacOS. 
Follow the instructions, listed in the 'Getting started' manual page for each platform.

> Note: remember to export env variable VULKAN_SDK=/path/to/installed/vulkan/sdk, 
> because we use this variable to locate vulkan include .h files and binary libs on your
> machine in case, when cmake failed to find these files in standard system /include and /lib directories.

