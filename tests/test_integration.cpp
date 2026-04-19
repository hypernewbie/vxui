#include "utest.h"
#include "../vxui.h"
#define VXUI_IMPL
#include "../vxui_impl.h"

// ---- 3.3 helper --------------------------------------------------------

struct settings_state
{
    int   fullscreen  = 0;
    int   vsync       = 1;
    float volume      = 0.5f;
    bool  apply_fired = false;
};

static const char* s_on_off[] = { "Off", "On" };

static void settings_menu_frame( vxui_ctx* ctx, settings_state* s )
{
    s->apply_fired = false;
    if ( vxui_menu( ctx, "settings" ) )
    {
        vxui_menu_section( ctx, "DISPLAY" );                               // row 0  skip
        vxui_menu_option ( ctx, "Fullscreen", &s->fullscreen, s_on_off, 2 ); // row 1
        vxui_menu_option ( ctx, "VSync",      &s->vsync,      s_on_off, 2 ); // row 2
        vxui_menu_section( ctx, "AUDIO" );                                 // row 3  skip
        vxui_menu_slider ( ctx, "Volume", &s->volume );                    // row 4
        s->apply_fired = vxui_menu_action( ctx, "Apply" );                 // row 5
        vxui_menu_end( ctx );
    }
}

// ---- 3.1 Page + menu navigation (title → options → back) ---------------

UTEST(integration, page_menu_nav)
{
    vxui_ctx ctx = {};
    vxui_switch( &ctx, "title" );

    // ---- Frame 1: establish title menu [Play, Options, Quit] ----
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
    ASSERT_TRUE( vxui_page( &ctx, "title" ) );
    ctx.input = 0;

    // ---- Frame 2: navigate down to Options (row 1), confirm → "options" ----
    vxui_input( &ctx, "down" );
    vxui_input( &ctx, "confirm" );
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
    ASSERT_TRUE( vxui_page( &ctx, "options" ) );
    ctx.input = 0;

    // ---- Frame 3: on "options", confirm Gameplay (row 0) → "options_gameplay" ----
    // Fresh menu: num_rows=0 so no navigation skip. Focus=0. Confirm fires row 0.
    vxui_input( &ctx, "confirm" );
    if ( vxui_page( &ctx, "options" ) )
    {
        if ( vxui_menu( &ctx, "opts" ) )
        {
            if ( vxui_menu_action( &ctx, "Gameplay" ) ) vxui_switch( &ctx, "options_gameplay" );
            if ( vxui_menu_action( &ctx, "Graphics"  ) ) {}
            vxui_menu_end( &ctx );
        }
    }
    ASSERT_TRUE( vxui_page( &ctx, "options_gameplay" ) );
    ctx.input = 0;

    // ---- Frame 4: on "options_gameplay", cancel → back to "options" ----
    vxui_input( &ctx, "cancel" );
    if ( vxui_page( &ctx, "options_gameplay" ) )
    {
        if ( vxui_menu_cancelled( &ctx ) )
            vxui_switch( &ctx, "options" );
    }
    ASSERT_TRUE( vxui_page( &ctx, "options" ) );
    ctx.input = 0;

    // ---- Frame 5: on "options", cancel → back to "title" ----
    vxui_input( &ctx, "cancel" );
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
    ASSERT_TRUE( vxui_page( &ctx, "title" ) );
}

// ---- 3.2 Multi-menu isolation ------------------------------------------
//
// Both menus share the same input bitfield — this is expected behavior.
// Focus state is keyed by ID so it does not bleed between menus.

UTEST(integration, multi_menu_isolation)
{
    vxui_ctx ctx = {};

    // ---- Frame 1: establish "left" (3 rows) and "right" (3 rows) ----
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
    ctx.input = 0;

    // ---- Frame 2: input=down — both menus navigate (expected shared-input behavior) ----
    vxui_input( &ctx, "down" );
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

    // Both moved to row 1 — shared input is expected, not a bug.
    // Focus is keyed by ID: menu_state[0] = left, menu_state[1] = right.
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 1 );
    ASSERT_EQ( ctx.menu_state[1].y, (uint32_t) 1 );
    ctx.input = 0;

    // ---- Frame 3: no input — focus stable ----
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

    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 1 );
    ASSERT_EQ( ctx.menu_state[1].y, (uint32_t) 1 );
}

// ---- 3.3 Mixed row navigation with skip --------------------------------
//
// Menu layout:
//   row 0  section "DISPLAY"   — skip
//   row 1  option  "Fullscreen"
//   row 2  option  "VSync"
//   row 3  section "AUDIO"     — skip
//   row 4  slider  "Volume"
//   row 5  action  "Apply"
//
// Navigation must never land on rows 0 or 3.

UTEST(integration, mixed_skip_navigation)
{
    vxui_ctx ctx = {};
    settings_state s;

    // ---- Frame 1: establish ----
    settings_menu_frame( &ctx, &s );
    ctx.input = 0;

    // ---- Frame 2: initial skip advances focus 0 (section) → 1 (Fullscreen) ----
    settings_menu_frame( &ctx, &s );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 1 );
    ctx.input = 0;

    // ---- down: 1 → 2 ----
    vxui_input( &ctx, "down" );
    settings_menu_frame( &ctx, &s );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 2 );
    ctx.input = 0;

    // ---- down: 2 → (skip 3) → 4 ----
    vxui_input( &ctx, "down" );
    settings_menu_frame( &ctx, &s );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 4 );
    ctx.input = 0;

    // ---- down: 4 → 5 ----
    vxui_input( &ctx, "down" );
    settings_menu_frame( &ctx, &s );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 5 );
    ctx.input = 0;

    // ---- confirm on row 5 → Apply fires ----
    vxui_input( &ctx, "confirm" );
    settings_menu_frame( &ctx, &s );
    ASSERT_TRUE( s.apply_fired );
    ctx.input = 0;

    // ---- up: 5 → 4 ----
    vxui_input( &ctx, "up" );
    settings_menu_frame( &ctx, &s );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 4 );
    ctx.input = 0;

    // ---- up: 4 → (skip 3) → 2 ----
    vxui_input( &ctx, "up" );
    settings_menu_frame( &ctx, &s );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 2 );
    ctx.input = 0;

    // ---- up: 2 → 1 ----
    vxui_input( &ctx, "up" );
    settings_menu_frame( &ctx, &s );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 1 );
    ctx.input = 0;

    // ---- up: 1 → (skip 0, wrap) → 5 ----
    vxui_input( &ctx, "up" );
    settings_menu_frame( &ctx, &s );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 5 );
}

// ---- 3.4 Focus clamping integration ------------------------------------
//
// Focus=3 on a 4-row menu. Menu shrinks to 3 rows (Debug omitted).
// One additional frame is needed for the clamp to fire, since num_rows is
// written by vxui_menu_end and the clamp uses the previous frame's count.

UTEST(integration, focus_clamp)
{
    vxui_ctx ctx = {};

    // ---- Frame 1: establish [Play, Options, Debug, Quit] (4 rows) ----
    if ( vxui_menu( &ctx, "test" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_action( &ctx, "Options" );
        vxui_menu_action( &ctx, "Debug" );
        vxui_menu_action( &ctx, "Quit" );
        vxui_menu_end( &ctx );
    }
    ctx.input = 0;

    // ---- Frame 2: up (wraps from 0 → 3 = Quit) ----
    vxui_input( &ctx, "up" );
    if ( vxui_menu( &ctx, "test" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_action( &ctx, "Options" );
        vxui_menu_action( &ctx, "Debug" );
        vxui_menu_action( &ctx, "Quit" );
        vxui_menu_end( &ctx );
    }
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 3 );
    ctx.input = 0;

    // ---- Frame 3: Debug omitted — 3-row menu writes num_rows=3.
    //              Clamp has not fired yet (prev num_rows was still 4). ----
    if ( vxui_menu( &ctx, "test" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_action( &ctx, "Options" );
        vxui_menu_action( &ctx, "Quit" );
        vxui_menu_end( &ctx );
    }
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 3 );   // still 3, clamp fires next frame
    ctx.input = 0;

    // ---- Frame 4: confirm — clamp fires (focus 3 >= num_rows 3 → clamped to 2),
    //              then confirm on clamped row 2 (Quit) fires ----
    int fired = -1;
    vxui_input( &ctx, "confirm" );
    if ( vxui_menu( &ctx, "test" ) )
    {
        if ( vxui_menu_action( &ctx, "Play" ) )    fired = 0;
        if ( vxui_menu_action( &ctx, "Options" ) ) fired = 1;
        if ( vxui_menu_action( &ctx, "Quit" ) )    fired = 2;
        vxui_menu_end( &ctx );
    }
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 2 );   // clamped
    ASSERT_EQ( fired, 2 );                              // Quit fired
}

UTEST_MAIN();
