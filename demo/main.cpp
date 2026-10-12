// VXUI demo: layout, materials, and movement.

#include <cstdio>
#include <cmath>
#include <filesystem>
#include <vector>
#include <glad/glad.h>
#include <glad/glad.c>
#include <GLFW/glfw3.h>

#include "vxui.h"
#define VXUI_IMPL
#include "vxui_impl.h"
#define VXUI_HUD_IMPL
#include "vxui_hud.h"
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

#define DEMO_PORTRAIT_W 480
#define DEMO_PORTRAIT_H 640
#define DEMO_WINDOW_W   1280
#define DEMO_WINDOW_H   720

#define DEMO_SHOWCASE_CONTROL_COUNT 4

static const char* s_demo_materials[] = { "Solid", "Rounded", "Scanlines" };
static const char* s_demo_motion[]    = { "Off", "On" };
static const char* s_demo_items[] = {
    "Item 01", "Item 02", "Item 03", "Item 04", "Item 05", "Item 06", "Item 07",
    "Item 08", "Item 09", "Item 10", "Item 11", "Item 12", "Item 13", "Item 14",
};
static const glm::vec4 s_demo_colours[] = {
    { 1.00f, 0.55f, 0.35f, 1.0f },
    { 0.40f, 0.85f, 1.00f, 1.0f },
    { 0.55f, 1.00f, 0.60f, 1.0f },
    { 1.00f, 0.85f, 0.30f, 1.0f },
    { 0.75f, 0.40f, 1.00f, 1.0f },
    { 1.00f, 0.30f, 0.35f, 1.0f },
};
static constexpr int s_demo_item_count   = (int) ( sizeof( s_demo_items ) / sizeof( s_demo_items[0] ) );
static constexpr int s_demo_colour_count = (int) ( sizeof( s_demo_colours ) / sizeof( s_demo_colours[0] ) );
static_assert( DEMO_SHOWCASE_CONTROL_COUNT + s_demo_item_count <= VXUI_MAX_MENU_ROWS, "showcase rows exceed VXUI menu capacity" );

struct demo_resolver_state
{
    uint32_t  panel_id       = 0;
    uint32_t  frame_id       = 0;
    uint32_t  bg_id          = 0;
    uint32_t  sample_id      = 0;
    int       material_index = 1;
    float     radius         = 4.0f;
    float     intensity      = 0.35f;
    int       motion_index   = 1;
    int       selected       = 0;
    glm::vec4 sample_colour  = { 1.00f, 0.55f, 0.35f, 1.0f };
    float     time_seconds   = 0.0f;
    vxui_hud* hud            = nullptr;
};

static void demo_render_data( const vxui_draw_cmd* c, vxui_render_data* out, void* ud )
{
    demo_resolver_state* st = (demo_resolver_state*) ud;

    if ( st->hud && vxui_hud_resolve( st->hud, c, out ) )
    {
        if ( c->type == VXUI_DRAW_RECT && out->texture_id != 0 ) out->material_id = DEMO_MATERIAL_IMAGE;
        return;
    }

    if ( c->type == VXUI_DRAW_TEXT )
    {
        out->colour = { 0.85f, 0.92f, 1.00f, 1.0f };
        return;
    }

    if ( c->id == st->bg_id )
    {
        out->colour = { 0.02f, 0.03f, 0.07f, 1.0f };
        return;
    }

    if ( c->id == st->sample_id )
    {
        out->colour = st->sample_colour;
        if ( st->material_index == 1 )
        {
            out->material_id = DEMO_MATERIAL_ROUND;
            out->params[0]   = st->radius;
            out->params[1]   = 1.0f;
        }
        else if ( st->material_index == 2 )
        {
            out->material_id = DEMO_MATERIAL_CRT;
            out->flags       = DEMO_MATERIAL_FLAG_SCANLINES;
            out->params[0]   = st->time_seconds;
            out->params[1]   = st->intensity;
        }
        else
            assert( st->material_index == 0 );
        return;
    }

    if ( c->id == st->panel_id ) return;

    if ( c->id == st->frame_id )
    {
        out->colour            = { 0.0f, 0.0f, 0.0f, 0.0f };
        out->outline_colour    = { 0.30f, 0.70f, 0.95f, 1.0f };
        out->outline_thickness = 2.0f;
        return;
    }

    out->material_id = DEMO_MATERIAL_ROUND;
    out->params[0]   = 4.0f;
    out->params[1]   = 1.0f;

    if ( c->state & VXUI_DRAW_PRESSED ) { out->colour = { 0.55f, 0.95f, 1.00f, 0.95f }; return; }
    if ( c->state & VXUI_DRAW_FOCUSED )
    {
        out->colour            = { 0.10f, 0.40f, 0.85f, 0.85f };
        out->outline_colour    = { 0.55f, 0.95f, 1.00f, 1.00f };
        out->outline_thickness = 2.0f;
        return;
    }
    if ( c->state & VXUI_DRAW_HOVERED ) { out->colour = { 0.16f, 0.24f, 0.36f, 0.80f }; return; }

    if ( c->row_index >= 0 && c->focused_row_index >= 0 )
    {
        int d = c->row_index - c->focused_row_index;
        if ( d < 0 ) d = -d;
        static const glm::vec4 s_ramp[4] = {
            { 0.10f, 0.40f, 0.85f, 0.85f },
            { 0.06f, 0.18f, 0.34f, 0.70f },
            { 0.04f, 0.11f, 0.22f, 0.55f },
            { 0.03f, 0.07f, 0.14f, 0.40f },
        };
        out->colour = s_ramp[d > 3 ? 3 : d];
        return;
    }

    out->colour = { 0.10f, 0.12f, 0.16f, 0.0f };
}

static int demo_menu_focused( vxui_ctx* ctx, const char* name )
{
    uint32_t h = vxui_hash( name );
    for ( int i = 0; i < VXUI_MAX_MENUS; i++ )
        if ( ctx->menu_state[i].x == h ) return (int) ctx->menu_state[i].y;
    return 0;
}

static bool demo_showcase( vxui_ctx* ctx, demo_resolver_state* st, float dt )
{
    bool close = false;

    vxui_div( ctx, "showcase_body", { .width = { VXUI_GROW, 0 }, .height = { VXUI_GROW, 0 }, .gap = 6 } );
        vxui_div( ctx, "showcase_menu_column", { .width = { VXUI_FIXED, 240 }, .height = { VXUI_GROW, 0 }, .col = true } );
            vxui_text( ctx, "showcase_menu_heading", "LAYOUT / UI", 18 );
            if ( vxui_menu( ctx, "showcase", true, 9, true ) )
            {
                vxui_menu_option( ctx, "Material", &st->material_index, s_demo_materials, 3 );
                vxui_menu_slider( ctx, "Radius", &st->radius, 1.0f, 32.0f, 1.0f );
                vxui_menu_slider( ctx, "Intensity", &st->intensity, 0.0f, 1.0f, 0.05f );
                vxui_menu_option( ctx, "Motion", &st->motion_index, s_demo_motion, 2 );
                for ( int i = 0; i < s_demo_item_count; i++ ) vxui_menu_action( ctx, s_demo_items[i] );
                close = vxui_menu_cancelled( ctx );
                vxui_menu_end( ctx );
            }
        vxui_div_end( ctx );

        if ( st->motion_index ) st->time_seconds += dt;

        int focused = demo_menu_focused( ctx, "showcase" );
        st->selected = focused >= DEMO_SHOWCASE_CONTROL_COUNT ? focused - DEMO_SHOWCASE_CONTROL_COUNT : 0;
        st->sample_colour = s_demo_colours[st->selected % s_demo_colour_count];

        vxui_div( ctx, "showcase_readout", { .width = { VXUI_GROW, 0 }, .height = { VXUI_GROW, 0 }, .col = true, .gap = 6 } );
            vxui_text( ctx, "showcase_material_heading", "MATERIAL SAMPLE", 18 );
            vxui_rect( ctx, "showcase_sample", { .width = { VXUI_GROW, 0 }, .height = { VXUI_FIXED, 200 } } );
            vxui_div_end( ctx );
            vxui_text( ctx, "showcase_selected", s_demo_items[st->selected], 16 );
            vxui_text( ctx, "showcase_layout", "240px FIXED + GROW", 13 );
        vxui_div_end( ctx );
    vxui_div_end( ctx );

    vxui_div( ctx, "showcase_footer", { .width = { VXUI_GROW, 0 }, .height = { VXUI_FIXED, 80 }, .col = true } );
    vxui_div_end( ctx );

    vxui_hud_set_font( st->hud, 0, 14 );
    vxui_hud_set_pos( st->hud, 20, 538 );
    vxui_hud_set_colour( st->hud, { 0.85f, 0.92f, 1.00f, 1.0f } );
    vxui_hud_text_box( st->hud, "showcase_fill_label", "GEOMETRIC MOTION", 440, 18, 0, 1 );

    vxui_hud_set_pos( st->hud, 20, 560 );
    vxui_hud_set_colour( st->hud, { 0.10f, 0.18f, 0.26f, 1.0f } );
    vxui_hud_resource_bar( st->hud, "showcase_fill_track", 0, 1.0f, 440, 14, {} );

    vxui_hud_set_pos( st->hud, 20, 560 );
    vxui_hud_set_colour( st->hud, { 0.25f, 0.80f, 1.00f, 1.0f } );
    vxui_hud_resource_bar( st->hud, "showcase_fill", 0, 0.5f + 0.5f * sinf( st->time_seconds ), 440, 14, {} );

    vxui_hud_set_pos( st->hud, 20, 585 );
    vxui_hud_set_colour( st->hud, { 0.85f, 0.92f, 1.00f, 1.0f } );
    vxui_hud_text_box( st->hud, "showcase_hint", "ESC EXIT  UP/DOWN SELECT  LEFT/RIGHT EDIT", 440, 20, 0, 1 );

    return close;
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

    GLFWwindow* window = glfwCreateWindow( DEMO_WINDOW_W, DEMO_WINDOW_H, "VXUI Layout Showcase", nullptr, nullptr );
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

    std::filesystem::path src_dir   = VXUI_SOURCE_DIR;
    std::filesystem::path font_path = src_dir / "fonts" / "Roboto-Regular.ttf";
    std::vector< uint8_t > font_bytes = load_file( font_path.string().c_str() );
    if ( font_bytes.empty() )
    {
        fprintf( stderr, "demo: failed to load %s\n", font_path.string().c_str() );
        return 1;
    }
    vxui_load_font( &ctx, font_bytes.data(), font_bytes.size(), (float) VXUI_FONT_SIZE_DEFAULT );

    vxui_gl_init( &ctx );

    demo_resolver_state state;
    vxui_hud hud = {};
    state.panel_id  = vxui_id( "demo_panel" );
    state.frame_id  = vxui_id( "demo_frame" );
    state.bg_id     = vxui_id( "demo_bg" );
    state.sample_id = vxui_id( "showcase_sample" );
    state.hud       = &hud;
    vxui_set_render_data_fn( &ctx, demo_render_data, &state );

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

    double previous_time = glfwGetTime();
    while ( !glfwWindowShouldClose( window ) )
    {
        glfwPollEvents();
        double now        = glfwGetTime();
        float  dt         = (float) ( now - previous_time );
        previous_time     = now;
        glfwGetFramebufferSize( window, &fb_w, &fb_h );

        glViewport  ( 0, 0, fb_w, fb_h );
        glClearColor( 0.02f, 0.02f, 0.03f, 1.0f );
        glClear     ( GL_COLOR_BUFFER_BIT );

        vxui_frame( &ctx, dt, (float) DEMO_PORTRAIT_W, (float) DEMO_PORTRAIT_H );
        vxui_hud_begin( &hud, &ctx, (float) DEMO_PORTRAIT_W, (float) DEMO_PORTRAIT_H );

        for ( auto& m : s_keymap )
            if ( glfwGetKey( window, m.key ) == GLFW_PRESS )
                vxui_input( &ctx, m.action );

        float scale    = glm::min( (float) fb_w / (float) DEMO_PORTRAIT_W, (float) fb_h / (float) DEMO_PORTRAIT_H );
        float origin_x = ( (float) fb_w - (float) DEMO_PORTRAIT_W * scale ) * 0.5f;
        float origin_y = ( (float) fb_h - (float) DEMO_PORTRAIT_H * scale ) * 0.5f;

        vxui_gl_xform xf;
        xf.offset = { origin_x, origin_y };
        xf.scale  = scale;

        double mx = 0, my = 0;
        glfwGetCursorPos( window, &mx, &my );
        uint32_t mb = 0;
        if ( glfwGetMouseButton( window, GLFW_MOUSE_BUTTON_LEFT  ) == GLFW_PRESS ) mb |= VXUI_MOUSE_LEFT;
        if ( glfwGetMouseButton( window, GLFW_MOUSE_BUTTON_RIGHT ) == GLFW_PRESS ) mb |= VXUI_MOUSE_RIGHT;
        bool inside = mx >= origin_x && mx < origin_x + (float) DEMO_PORTRAIT_W * scale
                   && my >= origin_y && my < origin_y + (float) DEMO_PORTRAIT_H * scale;
        if ( inside )
            vxui_mouse( &ctx, ( (float) mx - origin_x ) / scale, ( (float) my - origin_y ) / scale, mb );
        else
            vxui_mouse( &ctx, FLT_MAX, FLT_MAX, 0 );

        vxui_root( &ctx, "demo_root", 0.0f, 0.0f );
            vxui_rect( &ctx, "demo_bg", { .width = { VXUI_FIXED, DEMO_PORTRAIT_W }, .height = { VXUI_FIXED, DEMO_PORTRAIT_H } } );
            vxui_div_end( &ctx );
        vxui_root_end( &ctx );

        vxui_root( &ctx, "demo_root2", 0.0f, 0.0f );
            vxui_rect( &ctx, "demo_frame", { .width = { VXUI_FIXED, DEMO_PORTRAIT_W }, .height = { VXUI_FIXED, DEMO_PORTRAIT_H }, .col = true, .padding = { 18, 18, 18, 18 }, .gap = 8 } );
                vxui_rect( &ctx, "demo_panel", { .width = { VXUI_GROW, 0 }, .height = { VXUI_GROW, 0 }, .col = true, .padding = { 14, 14, 14, 14 }, .gap = 6 } );
                    if ( demo_showcase( &ctx, &state, dt ) ) glfwSetWindowShouldClose( window, GLFW_TRUE );
                vxui_div_end( &ctx );
            vxui_div_end( &ctx );
        vxui_root_end( &ctx );

        vxui_draw_list dl = vxui_render( &ctx );
        vxui_gl_render( &ctx, dl, (float) fb_w, (float) fb_h, xf );
        glfwSwapBuffers( window );
    }

    vxui_gl_shutdown( &ctx );
    vxui_shutdown( &ctx );
    glfwDestroyWindow( window );
    glfwTerminate();
    return 0;
}
