cmake_minimum_required(VERSION 3.21)
project(vxui LANGUAGES C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(VXUI_BUILD_DEMO "Build the VXUI demo" ON)
option(VXUI_BUILD_TESTS "Build VXUI tests" ON)

add_library(vxui INTERFACE)
target_include_directories(vxui INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(vxui INTERFACE /utf-8)
endif()

if(VXUI_BUILD_DEMO)
    add_executable(vxui_demo main.cpp)
    target_link_libraries(vxui_demo PRIVATE vxui)
endif()

if(VXUI_BUILD_TESTS)
    enable_testing()
    add_executable(vxui_tests
        tests/test_main.cpp
    )
    target_link_libraries(vxui_tests PRIVATE vxui)
    add_test(NAME vxui_tests COMMAND vxui_tests)
endif()
