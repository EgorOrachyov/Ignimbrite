# VulkanRenderer

C/C++ library for visualization dynamic 3D scene with GPU hardware acceleration via Vulkan API

## Getting up and running

1. This project uses git as VCS. Clone full repo via git clone operation with
this repo URL address as a param.

```
$ git clone <repo URL>
```

2. The project has the following structure:

```
VulkanRenderer
|
+- include
+- source
+- thirdparty
| |
| +- include
| +- lib
|   |
|   +- win32
|   +- win64
|   +- mac64
|   +- linux64
|  
+- CMakeLists.txt

...
```
Project uses GLFW3 library for window system
and Vulkan SDK as API for graphics rendering.
You have to explicitly download, compile listed above SDKs and place it in the 
folder 'win32', 'win64' or other.

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


### Windows

As the project uses CMake, building on Windows is almost the same as on Linux.
