cmake_minimum_required(VERSION 3.10)
project(vxui LANGUAGES C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

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

if(VXUI_BUILD_DEMO OR VXUI_BUILD_TESTS)
    if(POLICY CMP0077)
        cmake_policy(SET CMP0077 NEW)
    endif()

    FetchContent_Declare(
        freetype
        GIT_REPOSITORY https://github.com/freetype/freetype.git
        GIT_TAG VER-2-13-3
    )
    set(FT_DISABLE_ZLIB ON CACHE BOOL "" FORCE)
    set(FT_DISABLE_BZIP2 ON CACHE BOOL "" FORCE)
    set(FT_DISABLE_PNG ON CACHE BOOL "" FORCE)
    set(FT_DISABLE_BROTLI ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(freetype)

    FetchContent_Declare(
        harfbuzz
        GIT_REPOSITORY https://github.com/harfbuzz/harfbuzz.git
        GIT_TAG 11.2.1
    )
    set(HB_BUILD_SUBSET OFF CACHE BOOL "" FORCE)
    set(HB_BUILD_UTILS OFF CACHE BOOL "" FORCE)
    set(HB_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(HB_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(HB_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(harfbuzz)
endif()

if(VXUI_BUILD_DEMO)
    add_executable(vxui_demo
        main.cpp
        vefc/demo/include/glad/glad.c
    )
    target_include_directories(vxui_demo PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/vefc/demo/include
        ${freetype_SOURCE_DIR}/include
        ${harfbuzz_SOURCE_DIR}/src
    )
    target_link_libraries(vxui_demo PRIVATE vxui tomlc99 freetype harfbuzz glu32)
    target_compile_definitions(vxui_demo PRIVATE
        VXUI_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
        VE_FONTCACHE_FREETYPE_RASTERISATION
        VE_FONTCACHE_HARFBUZZ
        $<$<CONFIG:Debug>:VXUI_DEBUG>
    )
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
        tests/test_soak.cpp
        tests/test_vefc_integration.cpp
    )
    target_include_directories(vxui_tests PRIVATE
        ${freetype_SOURCE_DIR}/include
        ${harfbuzz_SOURCE_DIR}/src
    )
    target_link_libraries(vxui_tests PRIVATE vxui tomlc99 freetype harfbuzz)
    target_compile_definitions(vxui_tests PRIVATE
        VE_FONTCACHE_FREETYPE_RASTERISATION
        VE_FONTCACHE_HARFBUZZ
        $<$<CONFIG:Debug>:VXUI_DEBUG>
        VXUI_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
        VXUI_TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests/data"
    )
    add_test(NAME vxui_tests COMMAND vxui_tests)
endif()

add_executable(vxui_release_probe tests/release_probe.cpp)
target_link_libraries(vxui_release_probe PRIVATE vxui tomlc99)
if(MSVC)
    target_compile_options(vxui_release_probe PRIVATE /utf-8)
endif()
