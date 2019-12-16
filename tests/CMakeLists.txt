cmake_minimum_required(VERSION 3.11)
project(Tests)

set(Tests_HEADERS
    TestObjectID.h
    VulkanStartUp.h
    VulkanApplicationTest.h
)

set(Tests_SOURCES
    main.cpp
)

message(STATUS "Config Tests executable")
add_executable(Tests ${Tests_HEADERS} ${Tests_SOURCES})

target_link_libraries(Tests PUBLIC Renderer)
target_link_libraries(Tests PUBLIC VulkanDevice)

# target_compile_definitions(Tests PUBLIC VK_USE_PLATFORM_WIN32_KHR) ?
#target_include_directories(Tests PUBLIC Vulkan::Vulkan)
# target_link_libraries(Tests PUBLIC Vulkan::Vulkan) ?

variable_watch(THIRD_PARTY_LIBS)
target_link_libraries(Tests PUBLIC ${THIRD_PARTY_LIBS})
