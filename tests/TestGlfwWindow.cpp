/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov, Sultim Tsyrendashiev                  */
/**********************************************************************************/

#include <Types.h>
#include <GLFW/glfw3.h>

using namespace ignimbrite;

int32 main() {
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
