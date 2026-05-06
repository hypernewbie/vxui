// VXUI demo — Phase 0: lifecycle + clear screen.

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <vector>
#include <glad/glad.h>
#include <glad/glad.c>
#include <GLFW/glfw3.h>

#include "vxui.h"
#define VXUI_IMPL
#include "vxui_impl.h"
#define VXUI_DEMO_RENDER_GL_IMPL
#include "demo_render_gl.h"

#ifndef VXUI_SOURCE_DIR
    #define VXUI_SOURCE_DIR "."
#endif

static std::vector< uint8_t > load_file( const char* path )
{
    std::vector< uint8_t > buf;
    FILE* f = fopen( path, "rb" );
    if ( !f ) return buf;
    fseek( f, 0, SEEK_END );
    long sz = ftell( f );
    fseek( f, 0, SEEK_SET );
    buf.resize( (size_t) sz );
    fread( buf.data(), 1, (size_t) sz, f );
    fclose( f );
    return buf;
}

static uint8_t s_clay_mem[16 * 1024 * 1024];

int main( int /*argc*/, char** /*argv*/ )
{
    if ( !glfwInit() )
    {
        fprintf( stderr, "demo: glfwInit failed\n" );
        return 1;
    }

    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
    glfwWindowHint( GLFW_OPENGL_PROFILE,        GLFW_OPENGL_CORE_PROFILE );
    glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE );

    GLFWwindow* window = glfwCreateWindow( 1280, 720, "VXUI Demo", nullptr, nullptr );
    if ( !window )
    {
        fprintf( stderr, "demo: glfwCreateWindow failed\n" );
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent( window );

    if ( !gladLoadGLLoader( (GLADloadproc) glfwGetProcAddress ) )
    {
        fprintf( stderr, "demo: gladLoadGL failed\n" );
        glfwDestroyWindow( window );
        glfwTerminate();
        return 1;
    }

    int fb_w = 0, fb_h = 0;
    glfwGetFramebufferSize( window, &fb_w, &fb_h );

    vxui_ctx ctx = {};
    vxui_init( &ctx, (float) fb_w, (float) fb_h, s_clay_mem, sizeof( s_clay_mem ) );

    std::filesystem::path font_path = std::filesystem::path( VXUI_SOURCE_DIR ) / "fonts" / "Roboto-Regular.ttf";
    std::vector< uint8_t > font_bytes = load_file( font_path.string().c_str() );
    if ( font_bytes.empty() )
    {
        fprintf( stderr, "demo: failed to load %s\n", font_path.string().c_str() );
        return 1;
    }
    vxui_load_font( &ctx, font_bytes.data(), font_bytes.size(), (float) VXUI_FONT_SIZE_DEFAULT );

    vxui_gl_init( &ctx );

    int frame = 0;
    while ( !glfwWindowShouldClose( window ) )
    {
        glfwPollEvents();
        glfwGetFramebufferSize( window, &fb_w, &fb_h );

        glViewport  ( 0, 0, fb_w, fb_h );
        glClearColor( 0.08f, 0.08f, 0.10f, 1.0f );
        glClear     ( GL_COLOR_BUFFER_BIT );

        vxui_frame( &ctx, 1.0f / 60.0f, (float) fb_w, (float) fb_h );
        if ( vxui_menu( &ctx, "main" ) )
        {
            vxui_menu_action( &ctx, "Play" );
            vxui_menu_action( &ctx, "Options" );
            vxui_menu_action( &ctx, "Quit" );
            vxui_menu_end( &ctx );
        }
        vxui_draw_list dl = vxui_render( &ctx );
        vxui_gl_render( &ctx, dl, (float) fb_w, (float) fb_h );

        if ( ( frame % 60 ) == 0 )
            printf( "frame %d: %d rects, %d texts\n",
                    frame,
                    vxui_draw_count( dl, VXUI_DRAW_RECT ),
                    vxui_draw_count( dl, VXUI_DRAW_TEXT ) );
        frame++;

        glfwSwapBuffers( window );
    }

    vxui_gl_shutdown( &ctx );
    vxui_shutdown   ( &ctx );
    glfwDestroyWindow( window );
    glfwTerminate();
    return 0;
}
