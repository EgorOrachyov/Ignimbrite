//
// Created by Egor Orachyov on 2019-12-18.
//

#include <ignimbrite/Types.h>
#include <GLFW/glfw3.h>

using namespace ignimbrite;

int32 main(int32 argc, char** argv) {
    int32 width = 640;
    int32 height = 480;
    const char caption[] = "Test Window";

    glfwInit();
    auto handle = glfwCreateWindow(width, height, caption, nullptr, nullptr);

    while (!glfwWindowShouldClose(handle)) {
        glfwPollEvents();
        glfwSwapBuffers(handle);
    }

    glfwDestroyWindow(handle);
    glfwTerminate();
}
