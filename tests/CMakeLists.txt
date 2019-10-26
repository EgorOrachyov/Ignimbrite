cmake_minimum_required(VERSION 3.11)
project(Tests)

set(SOURCES
        main.cpp
        TestObjectID.h)

variable_watch(PROJECT_LIBS)
variable_watch(THIRD_PARTY_LIBS)

add_executable(Tests ${SOURCES})
target_link_libraries(Tests ${PROJECT_LIBS} ${THIRD_PARTY_LIBS})