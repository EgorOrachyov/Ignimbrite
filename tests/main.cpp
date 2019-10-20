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
    glfwGetFramebufferSize(window.handle, &window.frameBufferWidth, &window.frameBufferHeight);
    glfwSetErrorCallback(callback);

    application.extensions = glfwGetRequiredInstanceExtensions(&application.extensionsCount);

    printf("[GLFW]: Vulkan %s\n", (glfwVulkanSupported() ? "supported" : "is not supported"));

    auto device = new VulkanContext(application);

    std::vector<VulkanVertex> vertices {
        { {0.0f, -0.5f}, {1.0f, 0.0f, 0.0f} },
        { {0.5f, 0.5f},  {0.0f, 1.0f, 0.0f} },
        { {-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f} }
    };

    while (!glfwWindowShouldClose(window.handle)) {
        /** Check window system changes */
        glfwPollEvents();

        /** Must recreate swap chain if window size was changed */
        {
            auto frameBufferWidthOld = window.frameBufferWidth;
            auto frameBufferHeightOld = window.frameBufferHeight;
            window.resized = false;

            glfwGetFramebufferSize(window.handle, &window.frameBufferWidth, &window.frameBufferHeight);

            if (window.frameBufferHeight != frameBufferHeightOld ||
                window.frameBufferWidth != frameBufferWidthOld) {
                window.resized = true;
            }
        }

        /** Must be after swap chain recreated for window sizes */
        device->drawFrame();
    }

    delete device;

    glfwDestroyWindow(window.handle);
    glfwTerminate();

    return 0;
}