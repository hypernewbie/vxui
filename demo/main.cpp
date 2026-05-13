// VXUI demo — portrait sci-fi mock menu (BR-shaped, not BR-labelled).

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>
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

// Portrait viewport. The window is landscape; we letterbox a portrait UI.
#define DEMO_PORTRAIT_W   480
#define DEMO_PORTRAIT_H   640
#define DEMO_WINDOW_W     1280
#define DEMO_WINDOW_H     720

#define DEMO_MAX_STACK    8
#define DEMO_MISSION_COUNT 18

// Mission unlock states. Drives the row text colour in operations screen.
#define DEMO_MISSION_LOCKED   0
#define DEMO_MISSION_OPEN     1
#define DEMO_MISSION_CLEARED  2
#define DEMO_MISSION_APLUS    3

struct demo_mission
{
    const char* id;
    const char* name;
    int         sector;        // 0..4 stage index, 5 = boss-rush, etc.
    int         status;
    const char* description;
};

static const demo_mission s_demo_missions[DEMO_MISSION_COUNT] = {
    { "op01", "Debris Baptism",   0, DEMO_MISSION_APLUS,   "Opening sortie through the outer debris cloud. Light contact." },
    { "op02", "Signal Bloom",     1, DEMO_MISSION_CLEARED, "Decode the relay before the carrier wave collapses." },
    { "op03", "Glass Reactor",    2, DEMO_MISSION_CLEARED, "Penetrate the resonance grid and disable core lockout." },
    { "op04", "Iron Weather",     3, DEMO_MISSION_OPEN,    "Hostile shipyard. Heavy turrets, dense flak corridor." },
    { "op05", "Helix Foundry",    3, DEMO_MISSION_OPEN,    "Reactor coil ignition run, expect rotating beams." },
    { "op06", "Static Choir",     4, DEMO_MISSION_LOCKED,  "Saturation field. Clear three drone choirs to advance." },
    { "op07", "Blackbox Dive",    4, DEMO_MISSION_LOCKED,  "Recover encrypted blackbox before scuttling deadline." },
    { "op08", "Pale Vector",      4, DEMO_MISSION_LOCKED,  "Solo intercept against the pale vector squadron." },
    { "op09", "Moonbreaker",      5, DEMO_MISSION_LOCKED,  "Splinter the lunar harvester before it reseats." },
    { "op10", "Null Cathedral",   5, DEMO_MISSION_LOCKED,  "Final mission. Survive the cathedral's last hymn." },
    { "op11", "Neon Undertow",    1, DEMO_MISSION_LOCKED,  "Stealth gauntlet through the neon undertow current." },
    { "op12", "Starfall Engine",  2, DEMO_MISSION_LOCKED,  "Defend the starfall engine against a saturation strike." },
    { "op13", "Ghost Relay",      3, DEMO_MISSION_LOCKED,  "Trace the ghost relay broadcasting in dead frequencies." },
    { "op14", "Dead Orbit",       4, DEMO_MISSION_LOCKED,  "Recover survivors from the dead-orbit derelict ring." },
    { "op15", "Terminal Bloom",   5, DEMO_MISSION_LOCKED,  "Race against the terminal bloom expansion event." },
    { "br01", "Boss Rush",        5, DEMO_MISSION_LOCKED,  "All five sector commanders, back to back, no rest." },
    { "sv01", "Survival Protocol",5, DEMO_MISSION_LOCKED,  "Endless waves. How long can you stay aboard?" },
    { "ol01", "One-Life Trial",   5, DEMO_MISSION_LOCKED,  "One ship. One life. No continues. No mercy." },
};

struct demo_sector
{
    const char* name;
    const char* tagline;
    int         status;        // reuse mission status enum
};

static const demo_sector s_demo_sectors[] = {
    { "Sector 1: Debris Belt",   "Asteroid scatter and salvage convoys",      DEMO_MISSION_APLUS   },
    { "Sector 2: Signal Array",  "Long-range comms graveyard",                DEMO_MISSION_CLEARED },
    { "Sector 3: Glass Reactor", "Resonant core, mirrored coils",             DEMO_MISSION_CLEARED },
    { "Sector 4: Helix Foundry", "Industrial spiral, hot machinery",          DEMO_MISSION_OPEN    },
    { "Sector 5: Null Cathedral","Gothic deepspace cathedral",                DEMO_MISSION_LOCKED  },
    { "Boss Rush",               "Five sector commanders. No rest.",          DEMO_MISSION_LOCKED  },
};
#define DEMO_SECTOR_COUNT ( (int) ( sizeof( s_demo_sectors ) / sizeof( s_demo_sectors[0] ) ) )

struct demo_assets
{
    uint32_t bg         = 0;
    uint32_t logo       = 0;
    uint32_t border     = 0;
    uint32_t chevron    = 0;
    uint32_t lock       = 0;
    uint32_t warning    = 0;
    uint32_t ship[3]    = {};
    uint32_t mission[8] = {};
};

static uint32_t demo_mission_thumb( const demo_assets* a, int sector )
{
    // 0..4 = stages; 5..7 = rush/survival/one-life share thumbs.
    static const int s_thumb_idx[] = { 0, 1, 2, 3, 4, 5, 6, 7 };
    if ( sector < 0 || sector >= 8 ) sector = 0;
    return a->mission[s_thumb_idx[sector]];
}

struct demo_screen_state
{
    bool         splash                  = true;
    const char*  stack[DEMO_MAX_STACK]   = { "main" };
    int          depth                   = 1;

    float        master_volume = 0.8f;
    float        music_volume  = 0.6f;
    float        sfx_volume    = 0.7f;
    float        voice_volume  = 0.5f;
    int          ruleset       = 1;
    int          ship_speed    = 1;
    int          score_tips    = 1;
    int          window_mode   = 0;
    int          resolution    = 0;
    int          vsync         = 1;
    int          crt_curve     = 1;
    int          filter        = 0;
    int          rotation      = 0;
    int          scaling       = 0;
    int          show_fps      = 0;
    int          controller    = 0;
    int          input_icons   = 1;
    int          freeplay      = 0;
    int          infinite_ammo = 0;
    int          reduce_bullet = 0;
    int          slow_bullet   = 0;
    int          auto_shield   = 0;
};

static void demo_push( demo_screen_state* s, const char* name )
{
    assert( s && s->depth < DEMO_MAX_STACK );
    s->stack[s->depth++] = name;
}

static void demo_pop( demo_screen_state* s )
{
    assert( s && s->depth >= 1 );
    if ( s->depth > 1 ) s->depth--;
}

static int demo_menu_focused( vxui_ctx* ctx, const char* name )
{
    uint32_t h = vxui_hash( name );
    for ( int i = 0; i < VXUI_MAX_MENUS; i++ )
        if ( ctx->menu_state[i].x == h ) return (int) ctx->menu_state[i].y;
    return 0;
}

struct demo_resolver_state
{
    uint32_t        panel_id            = 0;
    uint32_t        frame_id            = 0;
    uint32_t        bg_id               = 0;
    uint32_t        border_id           = 0;
    uint32_t        logo_id             = 0;
    uint32_t        splash_prompt_id    = 0;
    uint32_t        version_id          = 0;
    uint32_t        focus_icon_texture  = 0;
    uint32_t        ops_icon[DEMO_MISSION_COUNT] = {};  // texture per ops row; 0 = none
    float           time_seconds        = 0;
    const demo_assets* assets           = nullptr;
    const demo_screen_state* screen     = nullptr;
};

static uint32_t demo_id( const char* s )
{
    Clay_String cs = { false, (int32_t) strlen( s ), s };
    return Clay__HashString( cs, 0 ).id;
}

// Status -> text colour. Mirrors the BR mission list scheme:
// locked = grey, cleared = green, A+ = yellow, default = white.
static glm::vec4 demo_status_colour( int status )
{
    switch ( status )
    {
        case DEMO_MISSION_LOCKED:  return { 0.40f, 0.40f, 0.45f, 1.0f };
        case DEMO_MISSION_CLEARED: return { 0.55f, 0.90f, 0.50f, 1.0f };
        case DEMO_MISSION_APLUS:   return { 1.00f, 0.85f, 0.30f, 1.0f };
        default:                   return { 0.95f, 0.95f, 0.95f, 1.0f };
    }
}

static void demo_render_data( const vxui_draw_cmd* c, vxui_render_data* out, void* ud )
{
    demo_resolver_state* st = (demo_resolver_state*) ud;

    if ( c->type == VXUI_DRAW_TEXT )
    {
        if ( c->id == st->splash_prompt_id )
        {
            float pulse = 0.55f + 0.35f * sinf( st->time_seconds * 3.0f );
            out->colour = { pulse, pulse * 0.95f, pulse * 0.8f, 1.0f };
            return;
        }
        // Pale blue-white reads as "panel readout" against the cyan accents.
        out->colour = { 0.85f, 0.92f, 1.00f, 1.0f };
        return;
    }

    if ( c->id == st->bg_id )
    {
        out->material_id = DEMO_MATERIAL_IMAGE;
        out->texture_id  = st->assets->bg;
        out->colour      = { 1.0f, 1.0f, 1.0f, 1.0f };
        return;
    }

    if ( c->id == st->logo_id )
    {
        out->material_id = DEMO_MATERIAL_IMAGE;
        out->texture_id  = st->assets->logo;
        out->uv          = { 0.05f, 0.30f, 0.95f, 0.70f };   // crop the logo band out of the square source
        out->colour      = { 1.0f, 1.0f, 1.0f, 1.0f };
        return;
    }

    if ( c->id == st->border_id )
    {
        out->material_id = DEMO_MATERIAL_IMAGE;
        out->texture_id  = st->assets->border;
        out->colour      = { 0.55f, 0.95f, 1.00f, 1.0f };
        return;
    }

    if ( c->id == st->panel_id )
    {
        // No fill — let the BG image + content render through. Scanlines
        // ride the existing pixels via the CRT material at low strength.
        out->material_id = DEMO_MATERIAL_CRT;
        out->flags       = DEMO_MATERIAL_FLAG_SCANLINES;
        out->params[0]   = st->time_seconds;
        out->params[1]   = 0.08f;
        out->colour      = { 0.0f, 0.0f, 0.0f, 0.0f };
        return;
    }

    if ( c->id == st->frame_id )
    {
        // Outline only — the BG image is the background, no dark fill.
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
        out->texture_id        = st->focus_icon_texture;
        out->uv                = { 0.0f, 0.0f, 1.0f, 1.0f };
        return;
    }
    if ( c->state & VXUI_DRAW_HOVERED ) { out->colour = { 0.16f, 0.24f, 0.36f, 0.80f }; return; }

    if ( c->row_index >= 0 && c->focused_row_index >= 0 )
    {
        // Distance-from-focus ramp: closer rows brighter, further rows fade
        // toward the BG image so the column reads as a stack of glowing strips.
        int d = c->row_index - c->focused_row_index;
        if ( d < 0 ) d = -d;
        static const glm::vec4 s_ramp[4] = {
            { 0.10f, 0.40f, 0.85f, 0.85f },
            { 0.06f, 0.18f, 0.34f, 0.70f },
            { 0.04f, 0.11f, 0.22f, 0.55f },
            { 0.03f, 0.07f, 0.14f, 0.40f },
        };
        int idx = d > 3 ? 3 : d;
        out->colour = s_ramp[idx];

        const demo_screen_state* s = st->screen;
        bool on_ops = s && s->depth > 0 && strcmp( s->stack[s->depth - 1], "operations" ) == 0;
        if ( on_ops && c->row_index < DEMO_MISSION_COUNT && st->ops_icon[c->row_index] != 0 )
            out->texture_id = st->ops_icon[c->row_index];

        return;
    }

    out->colour = { 0.10f, 0.12f, 0.16f, 0.0f };
}

static const char* s_demo_off_on[]      = { "Off", "On" };
static const char* s_demo_rulesets[]    = { "Standard", "Arcade", "Practice" };
static const char* s_demo_ship_speed[]  = { "Slow", "Normal", "Fast" };
static const char* s_demo_window_mode[] = { "Windowed", "Borderless", "Fullscreen" };
static const char* s_demo_resolution[]  = { "1280x720", "1920x1080", "2560x1440" };
static const char* s_demo_vsync[]       = { "Off", "On", "Adaptive" };
static const char* s_demo_curve[]       = { "Off", "Low", "High" };
static const char* s_demo_filter[]      = { "None", "Soft CRT", "LCD Grid", "Neon" };
static const char* s_demo_rotation[]    = { "None", "Clockwise", "Counter" };
static const char* s_demo_scaling[]     = { "Pixel Perfect", "Fit", "Stretch" };
static const char* s_demo_controller[]  = { "Keyboard", "Gamepad 1", "All" };
static const char* s_demo_icons[]       = { "Keyboard", "Xbox", "PlayStation", "Arcade" };

static void demo_splash( vxui_ctx* ctx, demo_screen_state* s, GLFWwindow* window )
{
    if ( ( ctx->input & ~ctx->prev_input & VXUI_INPUT_CONFIRM ) != 0 ) s->splash = false;
    if ( ( ctx->input & ~ctx->prev_input & VXUI_INPUT_CANCEL  ) != 0 ) glfwSetWindowShouldClose( window, GLFW_TRUE );

    vxui_div ( ctx, "splash_col", { .width = { VXUI_GROW, 0 }, .height = { VXUI_GROW, 0 }, .col = true, .align_x = 1 } );

        vxui_div ( ctx, "splash_pad_top", { .height = { VXUI_FIXED, 64 } } );
        vxui_div_end( ctx );

        vxui_rect( ctx, "demo_logo", { .width = { VXUI_FIXED, 400 }, .height = { VXUI_FIXED, 180 } } );
        vxui_div_end( ctx );

        vxui_div ( ctx, "splash_pad_mid", { .height = { VXUI_GROW, 0 } } );
        vxui_div_end( ctx );

        vxui_div ( ctx, "splash_prompt_row", { .width = { VXUI_GROW, 0 }, .align_x = 1 } );
        vxui_text( ctx, "splash_prompt", "PRESS ENTER TO START", 22 );
        vxui_div_end( ctx );

        vxui_div ( ctx, "splash_pad_bot", { .height = { VXUI_FIXED, 64 } } );
        vxui_div_end( ctx );

    vxui_div_end( ctx );
}

static void demo_main_menu( vxui_ctx* ctx, demo_screen_state* s, GLFWwindow* window )
{
    if ( !vxui_menu( ctx, "main", true, 0, true ) ) return;
    if ( vxui_menu_action( ctx, "Start"        ) ) printf( "Start fired\n" );
    if ( vxui_menu_action( ctx, "Operations"   ) ) demo_push( s, "operations" );
    if ( vxui_menu_action( ctx, "Stage Select" ) ) demo_push( s, "stageselect" );
    if ( vxui_menu_action( ctx, "Settings"     ) ) demo_push( s, "settings" );
    if ( vxui_menu_action( ctx, "Archives"     ) ) demo_push( s, "archives" );
    if ( vxui_menu_action( ctx, "Extras"       ) ) demo_push( s, "extras" );
    if ( vxui_menu_action( ctx, "Exit"         ) ) glfwSetWindowShouldClose( window, GLFW_TRUE );
    if ( vxui_menu_cancelled( ctx )              ) s->splash = true;
    vxui_menu_end( ctx );
}

static void demo_operations_menu( vxui_ctx* ctx, demo_screen_state* s )
{
    int focused = demo_menu_focused( ctx, "operations" );
    if ( focused < 0 || focused >= DEMO_MISSION_COUNT ) focused = 0;
    const demo_mission& sel = s_demo_missions[focused];

    vxui_div ( ctx, "ops_row", { .width = { VXUI_GROW, 0 }, .height = { VXUI_GROW, 0 }, .gap = 6 } );

        vxui_div ( ctx, "ops_left", { .width = { VXUI_FIXED, 240 }, .height = { VXUI_GROW, 0 }, .col = true } );
        if ( vxui_menu( ctx, "operations", true, 9, true ) )
        {
            for ( int i = 0; i < DEMO_MISSION_COUNT; i++ )
            {
                const demo_mission& m = s_demo_missions[i];
                if ( vxui_menu_action( ctx, m.name ) )
                {
                    if ( m.status == DEMO_MISSION_LOCKED ) printf( "locked: %s\n", m.name );
                    else                                   printf( "launch: %s\n", m.name );
                }
            }
            if ( vxui_menu_cancelled( ctx ) ) demo_pop( s );
            vxui_menu_end( ctx );
        }
        vxui_div_end( ctx );

        vxui_rect( ctx, "ops_preview", { .width = { VXUI_GROW, 0 }, .height = { VXUI_GROW, 0 }, .col = true, .padding = { 6, 6, 6, 6 }, .gap = 4 } );

            vxui_rect( ctx, "ops_thumb", { .width = { VXUI_GROW, 0 }, .height = { VXUI_FIXED, 140 } } );
            vxui_div_end( ctx );

            vxui_text( ctx, "ops_sector",   sel.status == DEMO_MISSION_LOCKED ? "?????" : sel.name, 22 );

            char meta[64];
            const char* tag = sel.status == DEMO_MISSION_LOCKED  ? "LOCKED"
                            : sel.status == DEMO_MISSION_APLUS   ? "PERFECT"
                            : sel.status == DEMO_MISSION_CLEARED ? "CLEARED"
                                                                 : "OPEN";
            snprintf( meta, sizeof( meta ), "STATUS  %s", tag );
            vxui_text( ctx, "ops_status", meta, 16 );

            vxui_div ( ctx, "ops_desc_pad", { .height = { VXUI_FIXED, 6 } } );
            vxui_div_end( ctx );

            const char* desc = sel.status == DEMO_MISSION_LOCKED ? "Locked. Clear earlier sorties to reveal this objective."
                                                                 : sel.description;
            vxui_text( ctx, "ops_desc", desc, 14 );

        vxui_div_end( ctx );

    vxui_div_end( ctx );
}

static void demo_stage_select( vxui_ctx* ctx, demo_screen_state* s )
{
    int focused = demo_menu_focused( ctx, "stageselect" );
    if ( focused < 0 || focused >= DEMO_SECTOR_COUNT ) focused = 0;
    const demo_sector& sel = s_demo_sectors[focused];

    vxui_div ( ctx, "stg_row", { .width = { VXUI_GROW, 0 }, .height = { VXUI_GROW, 0 }, .gap = 6 } );

        vxui_div ( ctx, "stg_left", { .width = { VXUI_FIXED, 240 }, .height = { VXUI_GROW, 0 }, .col = true } );
        if ( vxui_menu( ctx, "stageselect", true, 0, true ) )
        {
            for ( int i = 0; i < DEMO_SECTOR_COUNT; i++ )
                if ( vxui_menu_action( ctx, s_demo_sectors[i].name ) ) printf( "select: %s\n", s_demo_sectors[i].name );
            if ( vxui_menu_cancelled( ctx ) ) demo_pop( s );
            vxui_menu_end( ctx );
        }
        vxui_div_end( ctx );

        vxui_rect( ctx, "stg_preview", { .width = { VXUI_GROW, 0 }, .height = { VXUI_GROW, 0 }, .col = true, .padding = { 6, 6, 6, 6 }, .gap = 4 } );
            vxui_rect( ctx, "stg_thumb", { .width = { VXUI_GROW, 0 }, .height = { VXUI_FIXED, 140 } } );
            vxui_div_end( ctx );
            vxui_text( ctx, "stg_tag", sel.tagline, 16 );
        vxui_div_end( ctx );

    vxui_div_end( ctx );
}

static void demo_settings_hub( vxui_ctx* ctx, demo_screen_state* s )
{
    if ( !vxui_menu( ctx, "settings", true, 0, true ) ) return;
    if ( vxui_menu_action( ctx, "Gameplay"     ) ) demo_push( s, "settings_gameplay" );
    if ( vxui_menu_action( ctx, "Display"      ) ) demo_push( s, "settings_display"  );
    if ( vxui_menu_action( ctx, "Video Filter" ) ) demo_push( s, "settings_video"    );
    if ( vxui_menu_action( ctx, "Audio"        ) ) demo_push( s, "settings_audio"    );
    if ( vxui_menu_action( ctx, "Controls"     ) ) demo_push( s, "settings_controls" );
    if ( vxui_menu_action( ctx, "Assist"       ) ) demo_push( s, "settings_assist"   );
    if ( vxui_menu_action( ctx, "Back"         ) ) demo_pop ( s );
    if ( vxui_menu_cancelled( ctx )              ) demo_pop ( s );
    vxui_menu_end( ctx );
}

static void demo_settings_gameplay( vxui_ctx* ctx, demo_screen_state* s )
{
    if ( !vxui_menu( ctx, "settings_gameplay", true, 0, true ) ) return;
    vxui_menu_option( ctx, "Ruleset",         &s->ruleset,    s_demo_rulesets,   3 );
    vxui_menu_option( ctx, "Ship Speed",      &s->ship_speed, s_demo_ship_speed, 3 );
    vxui_menu_option( ctx, "Score Tips",      &s->score_tips, s_demo_off_on,     2 );
    if ( vxui_menu_action( ctx, "Back" ) ) demo_pop( s );
    if ( vxui_menu_cancelled( ctx )      ) demo_pop( s );
    vxui_menu_end( ctx );
}

static void demo_settings_display( vxui_ctx* ctx, demo_screen_state* s )
{
    if ( !vxui_menu( ctx, "settings_display", true, 0, true ) ) return;
    vxui_menu_option( ctx, "Window Mode", &s->window_mode, s_demo_window_mode, 3 );
    vxui_menu_option( ctx, "Resolution",  &s->resolution,  s_demo_resolution,  3 );
    vxui_menu_option( ctx, "VSync",       &s->vsync,       s_demo_vsync,       3 );
    vxui_menu_option( ctx, "CRT Curve",   &s->crt_curve,   s_demo_curve,       3 );
    if ( vxui_menu_action( ctx, "Apply Display" ) ) printf( "Apply display\n" );
    if ( vxui_menu_action( ctx, "Back"          ) ) demo_pop( s );
    if ( vxui_menu_cancelled( ctx )               ) demo_pop( s );
    vxui_menu_end( ctx );
}

static void demo_settings_video( vxui_ctx* ctx, demo_screen_state* s )
{
    if ( !vxui_menu( ctx, "settings_video", true, 0, true ) ) return;
    vxui_menu_option( ctx, "Rotation", &s->rotation, s_demo_rotation, 3 );
    vxui_menu_option( ctx, "Scaling",  &s->scaling,  s_demo_scaling,  3 );
    vxui_menu_option( ctx, "Filter",   &s->filter,   s_demo_filter,   4 );
    vxui_menu_option( ctx, "Show FPS", &s->show_fps, s_demo_off_on,   2 );
    if ( vxui_menu_action( ctx, "Back" ) ) demo_pop( s );
    if ( vxui_menu_cancelled( ctx )      ) demo_pop( s );
    vxui_menu_end( ctx );
}

static void demo_settings_audio( vxui_ctx* ctx, demo_screen_state* s )
{
    if ( !vxui_menu( ctx, "settings_audio", true, 0, true ) ) return;
    vxui_menu_slider( ctx, "Master Volume", &s->master_volume );
    vxui_menu_slider( ctx, "Music Volume",  &s->music_volume  );
    vxui_menu_slider( ctx, "SFX Volume",    &s->sfx_volume    );
    vxui_menu_slider( ctx, "Voice Volume",  &s->voice_volume  );
    if ( vxui_menu_action( ctx, "Back" ) ) demo_pop( s );
    if ( vxui_menu_cancelled( ctx )      ) demo_pop( s );
    vxui_menu_end( ctx );
}

static void demo_settings_controls( vxui_ctx* ctx, demo_screen_state* s )
{
    if ( !vxui_menu( ctx, "settings_controls", true, 0, true ) ) return;
    vxui_menu_option( ctx, "Controller",   &s->controller,  s_demo_controller, 3 );
    if ( vxui_menu_action( ctx, "Rebind Controls" ) ) printf( "Rebind\n" );
    if ( vxui_menu_action( ctx, "Reset Controls"  ) ) printf( "Reset\n"  );
    vxui_menu_option( ctx, "Input Icons",  &s->input_icons, s_demo_icons,      4 );
    if ( vxui_menu_action( ctx, "Back" ) ) demo_pop( s );
    if ( vxui_menu_cancelled( ctx )      ) demo_pop( s );
    vxui_menu_end( ctx );
}

static void demo_settings_assist( vxui_ctx* ctx, demo_screen_state* s )
{
    if ( !vxui_menu( ctx, "settings_assist", true, 0, true ) ) return;
    vxui_menu_option( ctx, "Freeplay",       &s->freeplay,      s_demo_off_on, 2 );
    vxui_menu_option( ctx, "Infinite Ammo",  &s->infinite_ammo, s_demo_off_on, 2 );
    vxui_menu_option( ctx, "Reduce Bullets", &s->reduce_bullet, s_demo_off_on, 2 );
    vxui_menu_option( ctx, "Slow Bullets",   &s->slow_bullet,   s_demo_off_on, 2 );
    vxui_menu_option( ctx, "Auto Shield",    &s->auto_shield,   s_demo_off_on, 2 );
    if ( vxui_menu_action( ctx, "Back" ) ) demo_pop( s );
    if ( vxui_menu_cancelled( ctx )      ) demo_pop( s );
    vxui_menu_end( ctx );
}

static void demo_archives_menu( vxui_ctx* ctx, demo_screen_state* s )
{
    if ( !vxui_menu( ctx, "archives", true, 0, true ) ) return;
    if ( vxui_menu_action( ctx, "Combat Records"    ) ) printf( "Records\n"   );
    if ( vxui_menu_action( ctx, "Ship Database"     ) ) printf( "Ships\n"     );
    if ( vxui_menu_action( ctx, "Enemy Index"       ) ) printf( "Enemies\n"   );
    if ( vxui_menu_action( ctx, "Music Test"        ) ) printf( "Music\n"     );
    if ( vxui_menu_action( ctx, "Development Notes" ) ) printf( "Devnotes\n"  );
    if ( vxui_menu_action( ctx, "Back" ) ) demo_pop( s );
    if ( vxui_menu_cancelled( ctx )      ) demo_pop( s );
    vxui_menu_end( ctx );
}

static void demo_extras_menu( vxui_ctx* ctx, demo_screen_state* s )
{
    if ( !vxui_menu( ctx, "extras", true, 0, true ) ) return;
    if ( vxui_menu_action( ctx, "Credits"         ) ) printf( "Credits\n"     );
    if ( vxui_menu_action( ctx, "Attract Loop"    ) ) printf( "Attract\n"     );
    if ( vxui_menu_action( ctx, "Diagnostics"     ) ) printf( "Diagnostics\n" );
    if ( vxui_menu_action( ctx, "Benchmark Scene" ) ) printf( "Benchmark\n"   );
    if ( vxui_menu_action( ctx, "Back" ) ) demo_pop( s );
    if ( vxui_menu_cancelled( ctx )      ) demo_pop( s );
    vxui_menu_end( ctx );
}

static void demo_dispatch_screen( vxui_ctx* ctx, demo_screen_state* s, GLFWwindow* window )
{
    const char* active = s->stack[s->depth - 1];
    if      ( strcmp( active, "main"              ) == 0 ) demo_main_menu        ( ctx, s, window );
    else if ( strcmp( active, "operations"        ) == 0 ) demo_operations_menu  ( ctx, s );
    else if ( strcmp( active, "stageselect"       ) == 0 ) demo_stage_select     ( ctx, s );
    else if ( strcmp( active, "settings"          ) == 0 ) demo_settings_hub     ( ctx, s );
    else if ( strcmp( active, "settings_gameplay" ) == 0 ) demo_settings_gameplay( ctx, s );
    else if ( strcmp( active, "settings_display"  ) == 0 ) demo_settings_display ( ctx, s );
    else if ( strcmp( active, "settings_video"    ) == 0 ) demo_settings_video   ( ctx, s );
    else if ( strcmp( active, "settings_audio"    ) == 0 ) demo_settings_audio   ( ctx, s );
    else if ( strcmp( active, "settings_controls" ) == 0 ) demo_settings_controls( ctx, s );
    else if ( strcmp( active, "settings_assist"   ) == 0 ) demo_settings_assist  ( ctx, s );
    else if ( strcmp( active, "archives"          ) == 0 ) demo_archives_menu    ( ctx, s );
    else if ( strcmp( active, "extras"            ) == 0 ) demo_extras_menu      ( ctx, s );
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

    GLFWwindow* window = glfwCreateWindow( DEMO_WINDOW_W, DEMO_WINDOW_H, "VXUI Demo", nullptr, nullptr );
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

    demo_assets assets = {};
    {
        std::filesystem::path a = src_dir / "assets";
        assets.bg         = vxui_gl_load_image( ( a / "ui" / "vxui_bg.png"                ).string().c_str() );
        assets.logo       = vxui_gl_load_image( ( a / "ui" / "vxui_logo_clean_alpha.png"  ).string().c_str() );
        assets.border     = vxui_gl_load_image( ( a / "ui" / "ui_border_alpha.png"       ).string().c_str() );
        assets.chevron    = vxui_gl_load_image( ( a / "ui" / "ui_chevron_alpha.png"      ).string().c_str() );
        assets.lock       = vxui_gl_load_image( ( a / "ui" / "ui_lock_alpha.png"         ).string().c_str() );
        assets.warning    = vxui_gl_load_image( ( a / "ui" / "ui_warning.png"            ).string().c_str() );
        assets.ship[0]    = vxui_gl_load_image( ( a / "ships" / "ship_interceptor_alpha.png"  ).string().c_str() );
        assets.ship[1]    = vxui_gl_load_image( ( a / "ships" / "ship_strike_craft_alpha.png" ).string().c_str() );
        assets.ship[2]    = vxui_gl_load_image( ( a / "ships" / "ship_experimental_alpha.png" ).string().c_str() );
        assets.mission[0] = vxui_gl_load_image( ( a / "missions" / "debris_belt.png"      ).string().c_str() );
        assets.mission[1] = vxui_gl_load_image( ( a / "missions" / "signal_array.png"     ).string().c_str() );
        assets.mission[2] = vxui_gl_load_image( ( a / "missions" / "glass_reactor.png"    ).string().c_str() );
        assets.mission[3] = vxui_gl_load_image( ( a / "missions" / "iron_weather.png"     ).string().c_str() );
        assets.mission[4] = vxui_gl_load_image( ( a / "missions" / "helix_foundry.png"    ).string().c_str() );
        assets.mission[5] = vxui_gl_load_image( ( a / "missions" / "static_choir.png"     ).string().c_str() );
        assets.mission[6] = vxui_gl_load_image( ( a / "missions" / "moonbreaker.png"      ).string().c_str() );
        assets.mission[7] = vxui_gl_load_image( ( a / "missions" / "null_cathedral.png"   ).string().c_str() );
    }

    demo_screen_state   screen_state;
    demo_resolver_state resolver_state;
    resolver_state.panel_id           = demo_id( "demo_panel"    );
    resolver_state.frame_id           = demo_id( "demo_frame"    );
    resolver_state.bg_id              = demo_id( "demo_bg"       );
    resolver_state.border_id          = demo_id( "demo_border"   );
    resolver_state.logo_id            = demo_id( "demo_logo"     );
    resolver_state.splash_prompt_id   = Clay__HashNumber( 0, demo_id( "splash_prompt" ) ).id;
    resolver_state.focus_icon_texture = vxui_gl_create_chevron_texture();
    resolver_state.assets             = &assets;
    resolver_state.screen             = &screen_state;
    for ( int i = 0; i < DEMO_MISSION_COUNT; i++ )
    {
        if      ( s_demo_missions[i].status == DEMO_MISSION_LOCKED ) resolver_state.ops_icon[i] = assets.lock;
        else if ( s_demo_missions[i].status == DEMO_MISSION_OPEN   ) resolver_state.ops_icon[i] = assets.warning;
        else                                                          resolver_state.ops_icon[i] = 0;
    }
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

    while ( !glfwWindowShouldClose( window ) )
    {
        glfwPollEvents();
        glfwGetFramebufferSize( window, &fb_w, &fb_h );

        glViewport  ( 0, 0, fb_w, fb_h );
        glClearColor( 0.02f, 0.02f, 0.03f, 1.0f );
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

        // Centre the portrait UI inside the landscape window.
        float origin_x = ( (float) fb_w - (float) DEMO_PORTRAIT_W ) * 0.5f;
        float origin_y = ( (float) fb_h - (float) DEMO_PORTRAIT_H ) * 0.5f;

        vxui_root( &ctx, "demo_root", origin_x, origin_y );

            vxui_rect( &ctx, "demo_bg", { .width = { VXUI_FIXED, DEMO_PORTRAIT_W }, .height = { VXUI_FIXED, DEMO_PORTRAIT_H } } );
            vxui_div_end( &ctx );

        vxui_root_end( &ctx );

        vxui_root( &ctx, "demo_root2", origin_x, origin_y );

            vxui_rect( &ctx, "demo_frame", { .width = { VXUI_FIXED, DEMO_PORTRAIT_W }, .height = { VXUI_FIXED, DEMO_PORTRAIT_H }, .col = true, .padding = { 18, 18, 18, 18 }, .gap = 8 } );

                if ( screen_state.splash )
                {
                    demo_splash( &ctx, &screen_state, window );
                }
                else
                {
                    vxui_rect( &ctx, "demo_panel", { .width = { VXUI_GROW, 0 }, .height = { VXUI_GROW, 0 }, .col = true, .padding = { 14, 14, 14, 14 }, .gap = 6 } );
                        demo_dispatch_screen( &ctx, &screen_state, window );
                    vxui_div_end( &ctx );
                }

            vxui_div_end( &ctx );

        vxui_root_end( &ctx );

        vxui_draw_list dl = vxui_render( &ctx );
        vxui_gl_render( &ctx, dl, (float) fb_w, (float) fb_h );

        glfwSwapBuffers( window );
    }

    vxui_gl_shutdown( &ctx );
    vxui_shutdown   ( &ctx );
    glfwDestroyWindow( window );
    glfwTerminate();
    return 0;
}
