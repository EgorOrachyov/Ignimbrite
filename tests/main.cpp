#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <VulkanContext.h>

int main() {

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(600, 400, "Vulkan", nullptr, nullptr);

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    VulkanContext* device = new VulkanContext("Vulkan", true, glfwExtensionCount, glfwExtensions);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    delete device;

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}