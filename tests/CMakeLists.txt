cmake_minimum_required(VERSION 3.11)
project(VulkanTests)

set(SOURCES main.cpp)

variable_watch(PROJECT_LIBS)
variable_watch(THIRD_PARTY_LIBS)

add_executable(VulkanTests ${SOURCES})
target_link_libraries(VulkanTests ${PROJECT_LIBS} ${THIRD_PARTY_LIBS})