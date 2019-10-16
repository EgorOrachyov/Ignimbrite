#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <VulkanContext.h>

static void callback(int a, const char* message) {
    printf("[GLFW]: %s\n", message);
}

int main() {

    VulkanApplication application;
    VulkanWindow& window = application.getPrimaryWindow();

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window.handle = glfwCreateWindow(600, 400, "Vulkan", nullptr, nullptr);
    glfwSetErrorCallback(callback);
    application.extensions = glfwGetRequiredInstanceExtensions(&application.extensionsCount);

    printf("[GLFW]: Vulkan %s\n", (glfwVulkanSupported() ? "supported" : "is not supported"));

    auto device = new VulkanContext(application);

    while (!glfwWindowShouldClose(window.handle)) {
        glfwPollEvents();
    }

    delete device;

    glfwDestroyWindow(window.handle);
    glfwTerminate();

    return 0;
}