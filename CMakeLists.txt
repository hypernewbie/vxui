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

add_library(tomlc99 STATIC
    third_party/tomlc99/toml.c
)
target_include_directories(tomlc99 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if(VXUI_BUILD_DEMO)
    add_executable(vxui_demo main.cpp)
    target_link_libraries(vxui_demo PRIVATE vxui tomlc99)
    target_compile_definitions(vxui_demo PRIVATE $<$<CONFIG:Debug>:VXUI_DEBUG>)
endif()

if(VXUI_BUILD_TESTS)
    enable_testing()
    add_executable(vxui_tests
        tests/test_main.cpp
        tests/test_core.cpp
        tests/test_clay_bridge.cpp
        tests/test_primitives_basic.cpp
        tests/test_primitives_advanced.cpp
        tests/test_motion.cpp
        tests/test_navigation.cpp
        tests/test_locale.cpp
        tests/test_screen_stack.cpp
        tests/test_traits.cpp
        tests/test_sequences_tooling.cpp
    )
    target_link_libraries(vxui_tests PRIVATE vxui tomlc99)
    target_compile_definitions(vxui_tests PRIVATE
        $<$<CONFIG:Debug>:VXUI_DEBUG>
        VXUI_TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests/data"
        VXUI_TEST_TEMP_DIR="${CMAKE_CURRENT_BINARY_DIR}"
    )
    add_test(NAME vxui_tests COMMAND vxui_tests)
endif()

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/vxui_release_probe.cpp
"#include <string>\n#define CLAY_IMPLEMENTATION\n#include \"clay/clay.h\"\n#define VE_FONTCACHE_IMPL\n#include \"vefc/ve_fontcache.h\"\n#define VXUI_IMPL\n#include \"vxui.h\"\nint main(){return (int)vxui_min_memory_size();}\n")
add_executable(vxui_release_probe ${CMAKE_CURRENT_BINARY_DIR}/vxui_release_probe.cpp)
target_include_directories(vxui_release_probe PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(vxui_release_probe PRIVATE tomlc99)
if(MSVC)
    target_compile_options(vxui_release_probe PRIVATE /utf-8)
endif()
