/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                        */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#include <RenderEngine.h>
#include <VulkanExtensions.h>
#include <VulkanRenderDevice.h>

using namespace ignimbrite;

struct Window {
#ifdef __APPLE__
    int32 w = 1280 / 2;
    int32 h = 720 / 2;
#else
    int32 w = 1280;
    int32 h = 720;
#endif
    GLFWwindow* handle;
    ID<IRenderDevice::Surface> surface;
    String name = "Render Engine Test";
    uint32 extensionsCount;
    const char** extensions;
};

class RenderEngineTest {
public:

    RenderEngineTest() {
        init();
    }

    ~RenderEngineTest() {
        shutdown();
    }

    void initWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        window.handle = glfwCreateWindow(window.w, window.h, window.name.c_str(), nullptr, nullptr);
        glfwGetFramebufferSize(window.handle, &window.w, &window.h);

        window.extensions = glfwGetRequiredInstanceExtensions(&window.extensionsCount);
    }

    void initDevice() {
        mDevice = std::make_shared<VulkanRenderDevice>(window.extensionsCount, window.extensions);
        window.surface = VulkanExtensions::createSurfaceGLFW((VulkanRenderDevice&)*mDevice, window.handle, window.w, window.h, window.name);
    }

    void initCamera() {
        mCamera = std::make_shared<Camera>();
        mCamera->setType(Camera::Type::Perspective);
        mCamera->setAspect((float32)window.w / (float32)window.h);
        mCamera->recalculate();
    }

    void initEngine() {
        mEngine = std::make_shared<RenderEngine>();
        mEngine->setRenderDevice(mDevice);
        mEngine->setTargetSurface(window.surface);
        mEngine->setCamera(mCamera);
    }

    void init() {
        initWindow();
        initDevice();
        initCamera();
        initEngine();
    }

    void run() {
        while (!glfwWindowShouldClose(window.handle)) {
            glfwPollEvents();
            glfwSwapBuffers(window.handle);

            mEngine->draw();
        }
    }

    void shutdown() {
        VulkanExtensions::destroySurface((VulkanRenderDevice&)*mDevice, window.surface);
    }

private:
    Window window;
    RefCounted<IRenderEngine> mEngine;
    RefCounted<IRenderDevice> mDevice;
    RefCounted<Camera>        mCamera;
};

int main() {
    RenderEngineTest test;
    test.run();
    return 0;
}