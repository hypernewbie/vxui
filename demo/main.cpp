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

#define DEMO_MATERIAL_CRT            1
#define DEMO_MATERIAL_FLAG_SCANLINES 1
#define DEMO_MATERIAL_FLAG_CURVE     2

struct demo_resolver_state
{
    uint32_t panel_id           = 0;
    uint32_t frame_id           = 0;
    uint32_t focus_icon_texture = 0;
    float    time_seconds       = 0;
};

static void demo_render_data( const vxui_draw_cmd* c, vxui_render_data* out, void* ud )
{
    demo_resolver_state* st = (demo_resolver_state*) ud;
    if ( c->type != VXUI_DRAW_RECT ) return;

    if ( c->id == st->panel_id )
    {
        out->material_id = DEMO_MATERIAL_CRT;
        out->flags       = DEMO_MATERIAL_FLAG_SCANLINES | DEMO_MATERIAL_FLAG_CURVE;
        out->params[0]   = st->time_seconds;
        out->params[1]   = 0.35f;
        out->params[2]   = 0.04f;
        out->params[3]   = 0.005f;
        out->colour      = { 0.05f, 0.07f, 0.12f, 1.0f };
        return;
    }

    if ( c->id == st->frame_id )
    {
        out->colour            = { 0.03f, 0.03f, 0.05f, 1.0f };
        out->outline_colour    = { 0.30f, 0.70f, 0.95f, 1.0f };
        out->outline_thickness = 2.0f;
        return;
    }

    if ( c->state & VXUI_DRAW_PRESSED ) { out->colour = { 0.45f, 0.70f, 1.00f, 1.0f }; return; }
    if ( c->state & VXUI_DRAW_FOCUSED )
    {
        out->colour     = { 0.30f, 0.55f, 0.85f, 1.0f };
        out->texture_id = st->focus_icon_texture;
        out->uv         = { 0.0f, 0.0f, 1.0f, 1.0f };
        return;
    }
    if ( c->state & VXUI_DRAW_HOVERED ) { out->colour = { 0.22f, 0.22f, 0.27f, 1.0f }; return; }
    out->colour = { 0.15f, 0.15f, 0.18f, 1.0f };
}

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

    demo_resolver_state resolver_state;
    {
        Clay_String cs_panel = { false, (int32_t) strlen( "demo_panel" ), "demo_panel" };
        Clay_String cs_frame = { false, (int32_t) strlen( "demo_frame" ), "demo_frame" };
        resolver_state.panel_id = Clay__HashString( cs_panel, 0 ).id;
        resolver_state.frame_id = Clay__HashString( cs_frame, 0 ).id;
    }
    resolver_state.focus_icon_texture = vxui_gl_create_chevron_texture();
    vxui_set_render_data_fn( &ctx, demo_render_data, &resolver_state );

    static const struct { int key; const char* action; } s_keymap[] = {
        { GLFW_KEY_UP,     "up"      },
        { GLFW_KEY_W,      "up"      },
        { GLFW_KEY_DOWN,   "down"    },
        { GLFW_KEY_S,      "down"    },
        { GLFW_KEY_LEFT,   "left"    },
        { GLFW_KEY_A,      "left"    },
        { GLFW_KEY_RIGHT,  "right"   },
        { GLFW_KEY_D,      "right"   },
        { GLFW_KEY_ENTER,  "confirm" },
        { GLFW_KEY_SPACE,  "confirm" },
        { GLFW_KEY_ESCAPE, "cancel"  },
    };

    int frame = 0;
    while ( !glfwWindowShouldClose( window ) )
    {
        glfwPollEvents();
        glfwGetFramebufferSize( window, &fb_w, &fb_h );

        glViewport  ( 0, 0, fb_w, fb_h );
        glClearColor( 0.08f, 0.08f, 0.10f, 1.0f );
        glClear     ( GL_COLOR_BUFFER_BIT );

        vxui_frame( &ctx, 1.0f / 60.0f, (float) fb_w, (float) fb_h );

        for ( auto& m : s_keymap )
            if ( glfwGetKey( window, m.key ) == GLFW_PRESS )
                vxui_input( &ctx, m.action );

        double mx = 0, my = 0;
        glfwGetCursorPos( window, &mx, &my );
        uint32_t mb = 0;
        if ( glfwGetMouseButton( window, GLFW_MOUSE_BUTTON_LEFT  ) == GLFW_PRESS ) mb |= VXUI_MOUSE_LEFT;
        if ( glfwGetMouseButton( window, GLFW_MOUSE_BUTTON_RIGHT ) == GLFW_PRESS ) mb |= VXUI_MOUSE_RIGHT;
        vxui_mouse( &ctx, (float) mx, (float) my, mb );

        resolver_state.time_seconds += 1.0f / 60.0f;

        vxui_rect( &ctx, "demo_frame", { .width = { VXUI_FIXED, 256 }, .col = true, .padding = { 4, 4, 4, 4 } } );
        vxui_rect( &ctx, "demo_panel", { .width = { VXUI_GROW, 0 }, .col = true, .padding = { 8, 8, 8, 8 } } );
        if ( vxui_menu( &ctx, "main" ) )
        {
            if ( vxui_menu_action( &ctx, "Play"    ) ) printf( "Play fired\n" );
            if ( vxui_menu_action( &ctx, "Options" ) ) printf( "Options fired\n" );
            if ( vxui_menu_action( &ctx, "Quit"    ) ) glfwSetWindowShouldClose( window, GLFW_TRUE );
            vxui_menu_end( &ctx );
        }
        vxui_div_end( &ctx );
        vxui_div_end( &ctx );
        vxui_draw_list dl = vxui_render( &ctx );
        vxui_gl_render( &ctx, dl, (float) fb_w, (float) fb_h );

        if ( ( frame % 60 ) == 0 )
        {
            printf( "frame %d: %d rects, %d texts\n",
                    frame,
                    vxui_draw_count( dl, VXUI_DRAW_RECT ),
                    vxui_draw_count( dl, VXUI_DRAW_TEXT ) );
            fflush( stdout );
        }
        frame++;

        glfwSwapBuffers( window );
    }

    vxui_gl_shutdown( &ctx );
    vxui_shutdown   ( &ctx );
    glfwDestroyWindow( window );
    glfwTerminate();
    return 0;
}
