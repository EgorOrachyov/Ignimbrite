![Project logo](https://github.com/EgorOrachyov/Ignimbrite/blob/master/documents/pictures/Logo1020x240.png)

# Ignimbrite

![MacOS build](https://github.com/EgorOrachyov/VulkanRenderer/workflows/MacOS/badge.svg)
![Linux build](https://github.com/EgorOrachyov/VulkanRenderer/workflows/Linux/badge.svg)
![Windows build](https://github.com/EgorOrachyov/VulkanRenderer/workflows/Windows/badge.svg)

**C++11 real-time rendering library** for dynamic 3D scenes visualisation with 
basic drawing interface, configurable graphics and post-process pipelines. 
Provides high-level abstractions, such as material and mesh systems, as well 
as low-level interface to access graphics back-end. Uses Vulkan API as primary 
back-end. OpenGL 4 supports is planed in far future.

**Purpose of the project**: implement rendering library with low abstractions set 
and low overhead, provide user with a toolset for visualization technical 
or another data in real-time with high level of customization. 

**Note**: project under heavy development. It is not ready for usage.

## Features

### Rendering
* Back-end graphical interface
* Material system
* Mesh system
* Post-process effects manager
* Graphical pipeline
* Shader compilation and reflection

### Back-ends support
* Vulkan 1.0 on macOS, Window and Linux

## Platforms
* Windows
* Linux
* MacOS

## Toolchain
* C++11 with standard library
* CMake 3.11 or higher

## Third-party projects
* [Vulkan SDK](https://vulkan.lunarg.com)
* [GLFW](https://github.com/glfw/glfw) 
* [SPIRV-Cross](https://github.com/KhronosGroup/SPIRV-Cross)
* [GLM](https://github.com/g-truc/glm)
* [Tiny .obj loader](https://github.com/syoyo/tinyobjloader)
* [STB Image](https://github.com/nothings/stb)

## Get and build 

This project uses git as primary VCS and CMake 3.11 as build configuration tool.
Project language is C++11 with std library 11. The following commands allow to get 
repository:

```
$ git clone https://github.com/EgorOrachyov/Ignimbrite.git
$ cd Ignimbrite
```

Create build folder, configure and run build:

```
$ mkdir build
$ cd build
$ cmake .. -DCMAKE_BUILD_MODE=Debug
$ cmake --build .
```

Build with all enabled options (see cmake options below):

```
$ cmake .. -DCMAKE_BUILD_MODE=Debug -DWITH_GLFW=ON -DWITH_VULKAN=ON -DWITH_TESTS=ON 
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

By default all the options are set in 'YES', except Vulkan and GLFW usage, because
they may require additional dependency setup process on you machine. 

### Dependencies setup

We try to keep as much dependencies managed inside project as we can. However, if you want to 
use vulkan as graphics backed, you will have to manually install Vulkan SDK from 
the official LunarG website.

Vulkan SDK [page](https://vulkan.lunarg.com/sdk/home) for Windows, Linux and MacOS. 
Follow the instructions, listed in the 'Getting started' manual page for each platform.

> Note: remember to export env variable VULKAN_SDK=/path/to/installed/vulkan/sdk, 
> because we use this variable to locate vulkan include .h files and binary libs on your
> machine in case, when cmake failed to find these files in standard system /include and /lib directories.

## License

This project is licensed under MIT license. 
Full text could be found in [license file](https://github.com/EgorOrachyov/Ignimbrite/blob/master/LICENSE.md).

## Contributors

* Egor Orachyov - [GitHub account](https://github.com/EgorOrachyov)
* Sultim Tsyrendashiev - [GitHub account](https://github.com/SultimTsyrendashiev)
