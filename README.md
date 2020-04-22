![Project logo](https://github.com/EgorOrachyov/Ignimbrite/blob/master/docs/pictures/Logo1020x240.png)

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

## Features

### Rendering
* Generic back-end graphical interface (compatible with OpenGL and Vulkan as well)
* Generic basic material for safe CPU -> GPU communication
* Mesh importing
* Post-process effects pipeline
* Graphical pipeline for *renderable* objects of any kind
* Dynamic SPIR-V binaries reflection for Vulkan shaders

### Back-ends support
* Vulkan 1.0 on macOS, Window and Linux

## Platforms
* Windows 10 (**Tested**)
* Linux Ubuntu (**Tested**)
* macOS Mojave (**Tested**)

## Toolchain
* C++11 with standard library
* CMake 3.14 or higher

## Third-party projects
* [Vulkan SDK](https://vulkan.lunarg.com) - Vulkan API SDK
* [Vulkan Memory Allocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator) - GPU Memory allocator for Vulkan
* [GLFW](https://github.com/glfw/glfw) - Cross-platform window management system
* [SPIRV-Cross](https://github.com/KhronosGroup/SPIRV-Cross) - Reflection tools SPIR-V binaries
* [GLM](https://github.com/g-truc/glm) - Math utility for OpenGL-style applications 
* [Tiny .OBJ Loader](https://github.com/syoyo/tinyobjloader) - Tiny .OBJ geometry importer
* [STB Image](https://github.com/nothings/stb) - Image import library

## Directory structure

* assets - various geometry and image data for testing
* code - actual library sources with depenencies
  * devices - backends for various graphics API (currently vulkan only)
  * engine - library core 
  * materials - material library and material utility
  * posteffects - predefined post effects for rendering engine
  * modules - optional library modules
  * thirdparty - project code dependencies 
* docs - documents, text files and data for development/info
* shaders - shaders GLSL source files  and compiled SPIR-V shaders
* test - contains test applications for various lib code testing
* CMakeLists.txt - root cmake file, add it as sub directory to your project for this project usage

## Tests

**Test folder** consist of test examples and executables. Io order to test some
graphics features, we created several files, which are intended to test vulkan 
device, rendering engine, offscreen rendering, shadow mapping and frustum culling.
Tests' structure is quite similar and compact, its content is self-descriptive. 

**In order to run tests** copy *shader* and *assets* folder inside out build directory
containing tests executables (after build it something like *build/tests/*).

* build
  * tests
    * **assets**
    * **shaders**
    * test1.exe
    * ...
    * testN.exe

## Get and build 

This project uses git as primary VCS and CMake 3.11 as build configuration tool.
Project language is C++11 with std library 11. The following commands allow to get repository:

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
$ cmake .. -DCMAKE_BUILD_MODE=Debug -DIGNIMBRITE_WITH_GLFW=ON -DIGNIMBRITE_WITH_VULKAN=ON -DIGNIMBRITE_WITH_TESTS=ON 
$ cmake --build .
```

### CMake optional flags

We export some optional flags to configure cmake build and final
library image. You can set the following options to get desired functionality:

* IGNIMBRITE_WITH_GLFW=(YES/NO) - build glfw library to be able to create glfw based window applications.
* IGNIMBRITE_WITH_VULKAN=(YES/NO) - build vulkan device to be able to use vulkan graphics backend.
* IGNIMBRITE_WITH_TESTS=(YES/NO) - build tests executables with samples and test examples.
* IGNIMBRITE_WITH_QT=(YES/NO) - build Vulkan with support for Qt surface.

By default all the options are set in 'NO', including Vulkan and GLFW, because
they may require additional dependency setup process on you machine. 

### Dependencies setup

**We try to keep as much dependencies** managed inside project as we can. 
Almost all third-party libraries sources are added to the project source tree.
This decision was made instead of using *git submodules*, as it seemed to us much 
simpler and more understandable. 

**Configuration** of such libraries as *GLM* or *STB Image* will be done automatically. 
But *GLFW* uses some native platform components, therefore, it might require additional 
setup step for you. Follow their platform setup guide for more info.

**If you want to use Vulkan** as graphics backed, you will have to manually install
Vulkan SDK from the official LunarG website.

Vulkan SDK [page](https://vulkan.lunarg.com/sdk/home) for Windows, Linux and MacOS. 
Follow the instructions, listed in the 'Getting started' manual page for each platform.

> Note: remember to export env variable VULKAN_SDK=/path/to/installed/vulkan/sdk, 
> because we use this variable to locate vulkan include .h files and binary libs on your
> machine in case, when cmake failed to find these files in standard system /include and /lib directories.

> Note: remember to export env variables for Vulkan library and loader for if your
> target platform is macOS since Vulkan implementation for this platform is 
> based on Metal API and requires additional setup.

## License

This project is licensed under MIT license. 
Full text could be found in [license file](https://github.com/EgorOrachyov/Ignimbrite/blob/master/LICENSE.md).

## Contributors

* Egor Orachyov - [GitHub account](https://github.com/EgorOrachyov)
* Sultim Tsyrendashiev - [GitHub account](https://github.com/SultimTsyrendashiev)
