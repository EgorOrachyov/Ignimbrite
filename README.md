# VulkanRenderer

C/C++ library for visualization dynamic 3D scene with GPU hardware acceleration via Vulkan API

## Getting up and running

1. This project uses git as VCS. Clone full repo via git clone operation with
this repo URL address as a param.

```
$ git clone https://github.com/EgorOrachyov/VulkanRenderer.git
```

2. The project has the following structure:

```
VulkanRenderer
+- device
+- misc
+- vulkan
+- tests
+- thirdparty
 +- include
 +- lib
+- CMakeLists.txt
```

Project requires as dependency GLFW3 library for multi-platform window system
support and Vulkan SDK as API for high performance GPU rendering computations.
You have to explicitly download, compile listed above SDKs and place include/lib
files in third-party folder.

### Linux

CMake must be installed to build this project. In some cases you have to explicitly
specify path to the cmake to use it in the terminal.

To build project open terminal in the folder with the project and execute the
following command:

```
$ mkdir build
$ cd build
$ cmake ../
$ make 
```

To run the example executable file enter the following command (from the folder 'build'):

```
$ ./VulkanRenderer
```