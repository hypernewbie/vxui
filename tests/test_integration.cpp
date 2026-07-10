#include "utest.h"
#include "../vxui.h"
#define VXUI_IMPL
#include "../vxui_impl.h"

static uint8_t s_clay_mem[16 * 1024 * 1024];

static vxui_ctx make_ctx()
{
    vxui_ctx ctx = {};
    vxui_init( &ctx, 1280, 720, s_clay_mem, sizeof( s_clay_mem ) );
    return ctx;
}

// Six-row settings menu shared by the tests below: row 0 ("DISPLAY") and
// row 3 ("AUDIO") are section headers, so navigation must skip them.
struct settings_state
{
    int   fullscreen  = 0;
    int   vsync       = 1;
    float volume      = 0.5f;
    bool  apply_fired = false;
};

static const char* s_on_off[] = { "Off", "On" };

static void settings_menu_frame( vxui_ctx* ctx, settings_state* s, uint32_t input )
{
    s->apply_fired = false;
    vxui_frame( ctx, 1.0f / 60.0f );
    ctx->input = input;
    if ( vxui_menu( ctx, "settings" ) )
    {
        vxui_menu_section( ctx, "DISPLAY" );
        vxui_menu_option ( ctx, "Fullscreen", &s->fullscreen, s_on_off, 2 );
        vxui_menu_option ( ctx, "VSync",      &s->vsync,      s_on_off, 2 );
        vxui_menu_section( ctx, "AUDIO" );
        vxui_menu_slider ( ctx, "Volume", &s->volume );
        s->apply_fired = vxui_menu_action( ctx, "Apply" );
        vxui_menu_end( ctx );
    }
    vxui_render( ctx );
    ctx->input = 0;   // release so next call registers as a fresh edge
}

UTEST(integration, page_menu_nav)
{
    vxui_ctx ctx = make_ctx();
    vxui_switch( &ctx, "title" );

    // frame 1: establish title menu [Play, Options, Quit]
    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_page( &ctx, "title" ) )
    {
        if ( vxui_menu( &ctx, "main" ) )
        {
            vxui_menu_action( &ctx, "Play" );
            vxui_menu_action( &ctx, "Options" );
            vxui_menu_action( &ctx, "Quit" );
            vxui_menu_end( &ctx );
        }
    }
    vxui_render( &ctx );
    ASSERT_TRUE( vxui_page( &ctx, "title" ) );

    vxui_frame( &ctx, 1.0f / 60.0f );
    ctx.input = VXUI_INPUT_DOWN | VXUI_INPUT_CONFIRM;
    if ( vxui_page( &ctx, "title" ) )
    {
        if ( vxui_menu( &ctx, "main" ) )
        {
            if ( vxui_menu_action( &ctx, "Play" ) )    {}
            if ( vxui_menu_action( &ctx, "Options" ) ) vxui_switch( &ctx, "options" );
            if ( vxui_menu_action( &ctx, "Quit" ) )    {}
            vxui_menu_end( &ctx );
        }
    }
    vxui_render( &ctx );
    ASSERT_TRUE( vxui_page( &ctx, "options" ) );

    // release so next confirm is a fresh edge
    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_page( &ctx, "options" ) )
        if ( vxui_menu( &ctx, "opts" ) ) vxui_menu_end( &ctx );
    vxui_render( &ctx );

    vxui_frame( &ctx, 1.0f / 60.0f );
    ctx.input = VXUI_INPUT_CONFIRM;
    if ( vxui_page( &ctx, "options" ) )
    {
        if ( vxui_menu( &ctx, "opts" ) )
        {
            if ( vxui_menu_action( &ctx, "Gameplay" ) ) vxui_switch( &ctx, "options_gameplay" );
            if ( vxui_menu_action( &ctx, "Graphics"  ) ) {}
            vxui_menu_end( &ctx );
        }
    }
    vxui_render( &ctx );
    ASSERT_TRUE( vxui_page( &ctx, "options_gameplay" ) );

    vxui_frame( &ctx, 1.0f / 60.0f );
    ctx.input = VXUI_INPUT_CANCEL;
    if ( vxui_page( &ctx, "options_gameplay" ) )
    {
        if ( vxui_menu_cancelled( &ctx ) )
            vxui_switch( &ctx, "options" );
    }
    vxui_render( &ctx );
    ASSERT_TRUE( vxui_page( &ctx, "options" ) );

    // release so next cancel is a fresh edge
    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_page( &ctx, "options" ) )
        if ( vxui_menu( &ctx, "opts" ) ) vxui_menu_end( &ctx );
    vxui_render( &ctx );

    vxui_frame( &ctx, 1.0f / 60.0f );
    ctx.input = VXUI_INPUT_CANCEL;
    if ( vxui_page( &ctx, "options" ) )
    {
        if ( vxui_menu( &ctx, "opts" ) )
        {
            if ( vxui_menu_action( &ctx, "Gameplay" ) ) {}
            if ( vxui_menu_action( &ctx, "Graphics"  ) ) {}
            vxui_menu_end( &ctx );
        }
        if ( vxui_menu_cancelled( &ctx ) )
            vxui_switch( &ctx, "title" );
    }
    vxui_render( &ctx );
    ASSERT_TRUE( vxui_page( &ctx, "title" ) );
}

// Shared input bitfield is by design; focus is keyed per menu id.
UTEST(integration, multi_menu_isolation)
{
    vxui_ctx ctx = make_ctx();

    // frame 1: establish "left" (3 rows) and "right" (3 rows)
    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "left" ) )
    {
        vxui_menu_action( &ctx, "L1" );
        vxui_menu_action( &ctx, "L2" );
        vxui_menu_action( &ctx, "L3" );
        vxui_menu_end( &ctx );
    }
    if ( vxui_menu( &ctx, "right" ) )
    {
        vxui_menu_action( &ctx, "R1" );
        vxui_menu_action( &ctx, "R2" );
        vxui_menu_action( &ctx, "R3" );
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );

    vxui_frame( &ctx, 1.0f / 60.0f );
    ctx.input = VXUI_INPUT_DOWN;
    if ( vxui_menu( &ctx, "left" ) )
    {
        vxui_menu_action( &ctx, "L1" );
        vxui_menu_action( &ctx, "L2" );
        vxui_menu_action( &ctx, "L3" );
        vxui_menu_end( &ctx );
    }
    if ( vxui_menu( &ctx, "right" ) )
    {
        vxui_menu_action( &ctx, "R1" );
        vxui_menu_action( &ctx, "R2" );
        vxui_menu_action( &ctx, "R3" );
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );

    // Both moved to row 1 — shared input is expected, not a bug.
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 1 );
    ASSERT_EQ( ctx.menu_state[1].y, (uint32_t) 1 );

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "left" ) )
    {
        vxui_menu_action( &ctx, "L1" );
        vxui_menu_action( &ctx, "L2" );
        vxui_menu_action( &ctx, "L3" );
        vxui_menu_end( &ctx );
    }
    if ( vxui_menu( &ctx, "right" ) )
    {
        vxui_menu_action( &ctx, "R1" );
        vxui_menu_action( &ctx, "R2" );
        vxui_menu_action( &ctx, "R3" );
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );

    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 1 );
    ASSERT_EQ( ctx.menu_state[1].y, (uint32_t) 1 );
}

UTEST(integration, mixed_skip_navigation)
{
    vxui_ctx ctx = make_ctx();
    settings_state s;

    settings_menu_frame( &ctx, &s, 0 );     // frame 1: establish
    settings_menu_frame( &ctx, &s, 0 );     // frame 2: initial skip → row 1
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 1 );

    settings_menu_frame( &ctx, &s, VXUI_INPUT_DOWN );   // 1 → 2
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 2 );

    settings_menu_frame( &ctx, &s, VXUI_INPUT_DOWN );   // 2 → (skip 3) → 4
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 4 );

    settings_menu_frame( &ctx, &s, VXUI_INPUT_DOWN );   // 4 → 5
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 5 );

    settings_menu_frame( &ctx, &s, VXUI_INPUT_CONFIRM ); // confirm → Apply fires
    ASSERT_TRUE( s.apply_fired );

    settings_menu_frame( &ctx, &s, VXUI_INPUT_UP );     // 5 → 4
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 4 );

    settings_menu_frame( &ctx, &s, VXUI_INPUT_UP );     // 4 → (skip 3) → 2
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 2 );

    settings_menu_frame( &ctx, &s, VXUI_INPUT_UP );     // 2 → 1
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 1 );

    settings_menu_frame( &ctx, &s, VXUI_INPUT_UP );     // 1 → (skip 0, wrap) → 5
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 5 );
}

// Focus=3 on a 4-row menu. Menu shrinks to 3 rows (Debug omitted).
// One additional frame is needed for the clamp to fire, since num_rows is
// written by vxui_menu_end and the clamp uses the previous frame's count.

UTEST(integration, focus_clamp)
{
    vxui_ctx ctx = make_ctx();

    // frame 1: establish [Play, Options, Debug, Quit] (4 rows)
    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "test" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_action( &ctx, "Options" );
        vxui_menu_action( &ctx, "Debug" );
        vxui_menu_action( &ctx, "Quit" );
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );

    // frame 2: up (wraps from 0 → 3 = Quit)
    vxui_frame( &ctx, 1.0f / 60.0f );
    ctx.input = VXUI_INPUT_UP;
    if ( vxui_menu( &ctx, "test" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_action( &ctx, "Options" );
        vxui_menu_action( &ctx, "Debug" );
        vxui_menu_action( &ctx, "Quit" );
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 3 );

    // frame 3: Debug omitted — 3-row menu writes num_rows=3.
    // Clamp has not fired yet (prev num_rows was still 4).
    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "test" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_action( &ctx, "Options" );
        vxui_menu_action( &ctx, "Quit" );
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 3 );   // still 3, clamp fires next frame

    // frame 4: confirm — clamp fires (focus 3 >= num_rows 3 → clamped to 2),
    // then confirm on clamped row 2 (Quit) fires
    int fired = -1;
    vxui_frame( &ctx, 1.0f / 60.0f );
    ctx.input = VXUI_INPUT_CONFIRM;
    if ( vxui_menu( &ctx, "test" ) )
    {
        if ( vxui_menu_action( &ctx, "Play" ) )    fired = 0;
        if ( vxui_menu_action( &ctx, "Options" ) ) fired = 1;
        if ( vxui_menu_action( &ctx, "Quit" ) )    fired = 2;
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 2 );   // clamped
    ASSERT_EQ( fired, 2 );                              // Quit fired
}

UTEST_MAIN();
