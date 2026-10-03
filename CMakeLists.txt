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
if(POLICY CMP0141)
    cmake_policy(SET CMP0141 NEW)
endif()

set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT Embedded CACHE STRING "" FORCE)

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
    foreach(t
        test_util test_pages test_menu test_menu_option test_menu_slider
        test_menu_noninteractive test_frame test_integration test_menu_draw
        test_render_data test_game_menu test_menu_in_div test_root
        test_menu_scroll test_input_just_pressed test_menu_remount
        test_menu_collision test_input_repeated test_menu_input_edge
        test_menu_text test_mouse test_hud test_hud_z test_hud_text_lines test_hud_text_box test_hud_metadata test_menu_stack test_row_index
        test_text test_draw_capacity test_vefc_backend)
        add_executable(${t} tests/${t}.cpp)
        target_link_libraries(${t} PRIVATE vxui)
        add_test(NAME ${t} COMMAND ${t})
    endforeach()

    if(VXUI_BUILD_DEMO)
        foreach(t test_vefc_backend_gl test_demo_operations test_demo_stage_select test_demo_hud_image test_demo_quad test_demo_panel test_demo_material_capacity test_demo_target_state)
            add_executable(${t} tests/${t}.cpp)
            target_include_directories(${t} PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}/vefc/demo/include
                ${CMAKE_CURRENT_SOURCE_DIR}/demo
            )
            target_link_libraries(${t} PRIVATE vxui glfw)
            if(WIN32)
                target_link_libraries(${t} PRIVATE opengl32)
            elseif(APPLE)
                target_link_libraries(${t} PRIVATE OpenGL::GL)
            elseif(UNIX)
                target_link_libraries(${t} PRIVATE OpenGL::GL)
            endif()
            add_test(NAME ${t} COMMAND ${t})
        endforeach()
    endif()

    file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tmp) # test_plot dumps .layout files here; gitignored, absent on a fresh checkout
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
