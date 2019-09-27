#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <iostream>
#include <stdexcept>
#include <functional>
#include <cstdlib>

int main() {

    VkInstance instance;
    VkResult result;
    VkInstanceCreateInfo info = {};

    result = vkCreateInstance(&info, NULL, &instance);
    std::cout << "vkCreateInstance result: " << result  << "\n";

    vkDestroyInstance(instance, nullptr);

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(600, 400, "Vulkan", nullptr, nullptr);

    while (!glfwWindowShouldClose(window)) {

        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}