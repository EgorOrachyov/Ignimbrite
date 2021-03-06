cmake_minimum_required(VERSION 3.11)
project(Tests)

add_executable(TestObjectID TestObjectID.cpp)
target_link_libraries(TestObjectID PRIVATE Ignimbrite)

if (IGNIMBRITE_WITH_GLFW)
    add_executable(TestGlfwWindow TestGlfwWindow.cpp)
    target_link_libraries(TestGlfwWindow PRIVATE Ignimbrite)
    target_link_libraries(TestGlfwWindow PRIVATE glfw)
endif()

if (IGNIMBRITE_WITH_VULKAN AND IGNIMBRITE_WITH_GLFW)
    add_executable(TestVulkanApplication TestVulkanApplication.cpp)
    target_link_libraries(TestVulkanApplication PRIVATE Ignimbrite)
    target_link_libraries(TestVulkanApplication PRIVATE VulkanDevice)
    target_link_libraries(TestVulkanApplication PRIVATE glfw)
endif()

if (IGNIMBRITE_WITH_VULKAN AND IGNIMBRITE_WITH_GLFW)
    add_executable(TestVulkan3D TestVulkan3D.cpp)
    target_link_libraries(TestVulkan3D PRIVATE Ignimbrite)
    target_link_libraries(TestVulkan3D PRIVATE VulkanDevice)
    target_link_libraries(TestVulkan3D PRIVATE glfw)
    target_link_libraries(TestVulkan3D PRIVATE stb-image)
    target_link_libraries(TestVulkan3D PRIVATE glm)
    target_link_libraries(TestVulkan3D PRIVATE tinyobjloader)
endif()

if (IGNIMBRITE_WITH_VULKAN AND IGNIMBRITE_WITH_GLFW)
    add_executable(TestVulkanOffscreen TestVulkanOffscreen.cpp)
    target_link_libraries(TestVulkanOffscreen PRIVATE Ignimbrite)
    target_link_libraries(TestVulkanOffscreen PRIVATE VulkanDevice)
    target_link_libraries(TestVulkanOffscreen PRIVATE glfw)
endif()

if (IGNIMBRITE_WITH_VULKAN AND IGNIMBRITE_WITH_GLFW)
    add_executable(TestVulkanOffscreenEnhanced TestVulkanOffscreenEnhanced.cpp)
    target_link_libraries(TestVulkanOffscreenEnhanced PRIVATE Ignimbrite)
    target_link_libraries(TestVulkanOffscreenEnhanced PRIVATE VulkanDevice)
    target_link_libraries(TestVulkanOffscreenEnhanced PRIVATE glfw)
endif()

if (IGNIMBRITE_WITH_VULKAN AND IGNIMBRITE_WITH_GLFW)
    add_executable(TestFrustum TestFrustum.cpp)
    target_link_libraries(TestFrustum PRIVATE Ignimbrite)
    target_link_libraries(TestFrustum PRIVATE VulkanDevice)
    target_link_libraries(TestFrustum PRIVATE glfw)
endif()

if (IGNIMBRITE_WITH_VULKAN AND IGNIMBRITE_WITH_GLFW)
    add_executable(TestLightFrustum TestLightFrustum.cpp)
    target_link_libraries(TestLightFrustum PRIVATE Ignimbrite)
    target_link_libraries(TestLightFrustum PRIVATE VulkanDevice)
    target_link_libraries(TestLightFrustum PRIVATE glfw)
endif()

if (IGNIMBRITE_WITH_VULKAN AND IGNIMBRITE_WITH_GLFW)
    add_executable(TestRenderEngine TestRenderEngine.cpp)
    target_link_libraries(TestRenderEngine PRIVATE Ignimbrite)
    target_link_libraries(TestRenderEngine PRIVATE IgnimbriteMaterials)
    target_link_libraries(TestRenderEngine PRIVATE IgnimbritePostEffects)
    target_link_libraries(TestRenderEngine PRIVATE VulkanDevice)
    target_link_libraries(TestRenderEngine PRIVATE glfw)
    target_link_libraries(TestRenderEngine PRIVATE stb-image)
endif()

if (IGNIMBRITE_WITH_VULKAN AND IGNIMBRITE_WITH_GLFW)
    add_executable(TestShadows TestShadows.cpp)
    target_link_libraries(TestShadows PRIVATE Ignimbrite)
    target_link_libraries(TestShadows PRIVATE VulkanDevice)
    target_link_libraries(TestShadows PRIVATE glfw)
endif()

if (IGNIMBRITE_WITH_VULKAN AND IGNIMBRITE_WITH_GLFW)
    add_executable(TestHelmetPBR TestHelmetPBR.cpp)
    target_link_libraries(TestHelmetPBR PRIVATE Ignimbrite)
    target_link_libraries(TestHelmetPBR PRIVATE IgnimbriteMaterials)
    target_link_libraries(TestHelmetPBR PRIVATE IgnimbritePostEffects)
    target_link_libraries(TestHelmetPBR PRIVATE VulkanDevice)
    target_link_libraries(TestHelmetPBR PRIVATE glfw)
    target_link_libraries(TestHelmetPBR PRIVATE stb-image)
endif()
