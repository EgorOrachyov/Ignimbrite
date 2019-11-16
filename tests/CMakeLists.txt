cmake_minimum_required(VERSION 3.11)
project(Tests)

set(Tests_HEADERS
    TestObjectID.h
    VulkanStartUp.h
)

set(Tests_SOURCES
    main.cpp
)

message(STATUS "Config Tests executable")
add_executable(Tests ${Test_HEADERS} ${Tests_SOURCES})

target_link_libraries(Tests PUBLIC Renderer)
target_link_libraries(Tests PUBLIC VulkanDevice)

variable_watch(THIRD_PARTY_LIBS)
target_link_libraries(Tests PUBLIC ${THIRD_PARTY_LIBS})