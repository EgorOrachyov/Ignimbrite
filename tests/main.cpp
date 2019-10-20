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
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    window.handle = glfwCreateWindow(window.width, window.height, application.name.c_str(), nullptr, nullptr);
    glfwSetWindowSizeLimits(window.handle, window.width, window.height, window.width * 2, window.height * 2);
    glfwSetErrorCallback(callback);
    glfwGetFramebufferSize(window.handle, &window.frameBufferWidth, &window.frameBufferHeight);
    glfwGetFramebufferSize(window.handle, &window.frameBufferWidthOld, &window.frameBufferHeightOld);

    application.extensions = glfwGetRequiredInstanceExtensions(&application.extensionsCount);

    printf("[GLFW]: Vulkan %s\n", (glfwVulkanSupported() ? "supported" : "is not supported"));

    auto device = new VulkanContext(application);

    while (!glfwWindowShouldClose(window.handle)) {
        /** Check window system changes */
        glfwPollEvents();

        /** Must recreate swap chain if window size was changed */
        {
            glfwGetFramebufferSize(window.handle, &window.frameBufferWidth, &window.frameBufferHeight);

            if (window.frameBufferHeight != window.frameBufferHeightOld ||
                window.frameBufferWidth != window.frameBufferWidthOld) {
                device->windowResized();
            }

            window.frameBufferWidthOld = window.frameBufferWidth;
            window.frameBufferHeightOld = window.frameBufferHeight;
        }

        /** Must be after swap chain recreated for window sizes */
        device->drawFrame();
    }

    delete device;

    glfwDestroyWindow(window.handle);
    glfwTerminate();

    return 0;
}