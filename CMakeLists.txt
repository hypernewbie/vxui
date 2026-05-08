cmake_minimum_required(VERSION 3.10)
project(vxui LANGUAGES C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

option(VXUI_BUILD_DEMO  "Build the VXUI demo"  ON)
option(VXUI_BUILD_TESTS "Build the VXUI tests" ON)

if(POLICY CMP0077)
    cmake_policy(SET CMP0077 NEW)
endif()

FetchContent_Declare(
    harfbuzz
    GIT_REPOSITORY https://github.com/harfbuzz/harfbuzz.git
    GIT_TAG 11.2.1
)
set(HB_BUILD_SUBSET   OFF CACHE BOOL "" FORCE)
set(HB_BUILD_UTILS    OFF CACHE BOOL "" FORCE)
set(HB_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(HB_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(HB_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(harfbuzz)

add_library(vxui INTERFACE)
target_include_directories(vxui INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/vefc
    ${harfbuzz_SOURCE_DIR}/src
)
target_link_libraries(vxui INTERFACE harfbuzz)
target_compile_definitions(vxui INTERFACE VE_FONTCACHE_HARFBUZZ)

if(MSVC)
    target_compile_options(vxui INTERFACE /utf-8)
endif()

if(VXUI_BUILD_TESTS)
    enable_testing()
    add_executable(test_util tests/test_util.cpp)
    target_link_libraries(test_util PRIVATE vxui)
    add_test(NAME test_util COMMAND test_util)

    add_executable(test_pages tests/test_pages.cpp)
    target_link_libraries(test_pages PRIVATE vxui)
    add_test(NAME test_pages COMMAND test_pages)

    add_executable(test_menu tests/test_menu.cpp)
    target_link_libraries(test_menu PRIVATE vxui)
    add_test(NAME test_menu COMMAND test_menu)

    add_executable(test_menu_option tests/test_menu_option.cpp)
    target_link_libraries(test_menu_option PRIVATE vxui)
    add_test(NAME test_menu_option COMMAND test_menu_option)

    add_executable(test_menu_slider tests/test_menu_slider.cpp)
    target_link_libraries(test_menu_slider PRIVATE vxui)
    add_test(NAME test_menu_slider COMMAND test_menu_slider)

    add_executable(test_menu_noninteractive tests/test_menu_noninteractive.cpp)
    target_link_libraries(test_menu_noninteractive PRIVATE vxui)
    add_test(NAME test_menu_noninteractive COMMAND test_menu_noninteractive)

    add_executable(test_frame tests/test_frame.cpp)
    target_link_libraries(test_frame PRIVATE vxui)
    add_test(NAME test_frame COMMAND test_frame)



    add_executable(test_integration tests/test_integration.cpp)
    target_link_libraries(test_integration PRIVATE vxui)
    add_test(NAME test_integration COMMAND test_integration)

    add_executable(test_menu_draw tests/test_menu_draw.cpp)
    target_link_libraries(test_menu_draw PRIVATE vxui)
    add_test(NAME test_menu_draw COMMAND test_menu_draw)

    add_executable(test_game_menu tests/test_game_menu.cpp)
    target_link_libraries(test_game_menu PRIVATE vxui)
    add_test(NAME test_game_menu COMMAND test_game_menu)

    add_executable(test_menu_in_div tests/test_menu_in_div.cpp)
    target_link_libraries(test_menu_in_div PRIVATE vxui)
    add_test(NAME test_menu_in_div COMMAND test_menu_in_div)

    add_executable(test_root tests/test_root.cpp)
    target_link_libraries(test_root PRIVATE vxui)
    add_test(NAME test_root COMMAND test_root)

    add_executable(test_menu_scroll tests/test_menu_scroll.cpp)
    target_link_libraries(test_menu_scroll PRIVATE vxui)
    add_test(NAME test_menu_scroll COMMAND test_menu_scroll)

    add_executable(test_input_just_pressed tests/test_input_just_pressed.cpp)
    target_link_libraries(test_input_just_pressed PRIVATE vxui)
    add_test(NAME test_input_just_pressed COMMAND test_input_just_pressed)

    add_executable(test_menu_remount tests/test_menu_remount.cpp)
    target_link_libraries(test_menu_remount PRIVATE vxui)
    add_test(NAME test_menu_remount COMMAND test_menu_remount)

    add_executable(test_menu_collision tests/test_menu_collision.cpp)
    target_link_libraries(test_menu_collision PRIVATE vxui)
    add_test(NAME test_menu_collision COMMAND test_menu_collision)

    add_executable(test_input_repeated tests/test_input_repeated.cpp)
    target_link_libraries(test_input_repeated PRIVATE vxui)
    add_test(NAME test_input_repeated COMMAND test_input_repeated)

    add_executable(test_menu_input_edge tests/test_menu_input_edge.cpp)
    target_link_libraries(test_menu_input_edge PRIVATE vxui)
    add_test(NAME test_menu_input_edge COMMAND test_menu_input_edge)

    add_executable(test_menu_text tests/test_menu_text.cpp)
    target_link_libraries(test_menu_text PRIVATE vxui)
    add_test(NAME test_menu_text COMMAND test_menu_text)

    add_executable(test_vefc_backend tests/test_vefc_backend.cpp)
    target_link_libraries(test_vefc_backend PRIVATE vxui)
    add_test(NAME test_vefc_backend COMMAND test_vefc_backend)

    if(VXUI_BUILD_DEMO)
        add_executable(test_vefc_backend_gl tests/test_vefc_backend_gl.cpp)
        target_include_directories(test_vefc_backend_gl PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/vefc/demo/include
            ${CMAKE_CURRENT_SOURCE_DIR}/demo
        )
        target_link_libraries(test_vefc_backend_gl PRIVATE vxui glfw)
        if(WIN32)
            target_link_libraries(test_vefc_backend_gl PRIVATE opengl32)
        elseif(APPLE)
            target_link_libraries(test_vefc_backend_gl PRIVATE OpenGL::GL)
        elseif(UNIX)
            target_link_libraries(test_vefc_backend_gl PRIVATE OpenGL::GL)
        endif()
        add_test(NAME test_vefc_backend_gl COMMAND test_vefc_backend_gl)
    endif()

    add_executable(test_plot tests/test_plot.cpp)
    target_link_libraries(test_plot PRIVATE vxui)
    add_test(NAME test_plot COMMAND test_plot WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()

if(VXUI_BUILD_DEMO)
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.4
    )
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(glfw)

    add_executable(vxui_demo
        demo/main.cpp
    )
    target_include_directories(vxui_demo PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/vefc/demo/include
    )
    target_link_libraries(vxui_demo PRIVATE vxui glfw)
    target_compile_definitions(vxui_demo PRIVATE
        VXUI_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
        $<$<CONFIG:Debug>:VXUI_DEBUG>
    )
    if(WIN32)
        target_link_libraries(vxui_demo PRIVATE opengl32)
    elseif(APPLE)
        find_package(OpenGL REQUIRED)
        target_link_libraries(vxui_demo PRIVATE OpenGL::GL)
    elseif(UNIX)
        find_package(OpenGL REQUIRED)
        target_link_libraries(vxui_demo PRIVATE OpenGL::GL)
    endif()
endif()
