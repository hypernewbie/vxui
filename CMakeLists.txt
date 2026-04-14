cmake_minimum_required(VERSION 3.10)
project(vxui LANGUAGES C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

option(VXUI_BUILD_DEMO "Build the VXUI demo" ON)

add_library(vxui INTERFACE)
target_include_directories(vxui INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(vxui INTERFACE /utf-8)
endif()

if(VXUI_BUILD_DEMO)
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

    add_executable(vxui_demo
        demo/main.cpp
        demo/windows/vxui_demo_manifest.rc
        vefc/demo/include/glad/glad.c
    )
    target_include_directories(vxui_demo PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/vefc/demo/include
        ${freetype_SOURCE_DIR}/include
        ${harfbuzz_SOURCE_DIR}/src
    )
    target_link_libraries(vxui_demo PRIVATE vxui freetype harfbuzz glu32 ole32 windowscodecs)
    target_compile_definitions(vxui_demo PRIVATE
        VXUI_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
        VE_FONTCACHE_FREETYPE_RASTERISATION
        VE_FONTCACHE_HARFBUZZ
        $<$<CONFIG:Debug>:VXUI_DEBUG>
    )
    if(MSVC)
        target_link_options(vxui_demo PRIVATE "/MANIFEST:NO")
    endif()
endif()
