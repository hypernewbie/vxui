#include "utest.h"
#include "../vxui.h"
#define VXUI_IMPL
#include "../vxui_impl.h"

// Helper: run one full menu frame with 3 actions. Returns which action fired (-1 = none).
static int menu_frame( vxui_ctx* ctx )
{
    int fired = -1;
    if ( vxui_menu( ctx, "test" ) )
    {
        if ( vxui_menu_action( ctx, "Play" ) )    fired = 0;
        if ( vxui_menu_action( ctx, "Options" ) ) fired = 1;
        if ( vxui_menu_action( ctx, "Quit" ) )    fired = 2;
        vxui_menu_end( ctx );
    }
    return fired;
}

/* ---- basic ----------------------------------------------------------- */

UTEST(menu, opens_and_closes) {
    vxui_ctx ctx = {};
    ASSERT_TRUE( vxui_menu( &ctx, "m" ) );
    vxui_menu_end( &ctx );
}

UTEST(menu, focus_starts_at_zero) {
    vxui_ctx ctx = {};
    menu_frame( &ctx );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 0 );
}

/* ---- confirm --------------------------------------------------------- */

UTEST(menu, confirm_fires_focused) {
    vxui_ctx ctx = {};
    menu_frame( &ctx );                     // frame 1: establish menu (count=3)
    ctx.input = 0;

    vxui_input( &ctx, "confirm" );
    int fired = menu_frame( &ctx );         // frame 2: confirm on row 0
    ASSERT_EQ( fired, 0 );
}

UTEST(menu, confirm_only_fires_focused) {
    vxui_ctx ctx = {};
    menu_frame( &ctx );                     // frame 1: establish
    ctx.input = 0;

    vxui_input( &ctx, "down" );
    menu_frame( &ctx );                     // frame 2: move to row 1
    ctx.input = 0;

    vxui_input( &ctx, "confirm" );
    int fired = menu_frame( &ctx );         // frame 3: confirm on row 1
    ASSERT_EQ( fired, 1 );
}

/* ---- navigation ------------------------------------------------------ */

UTEST(menu, down_moves_focus) {
    vxui_ctx ctx = {};
    menu_frame( &ctx );                     // frame 1: establish (count=3)
    ctx.input = 0;

    vxui_input( &ctx, "down" );
    menu_frame( &ctx );                     // frame 2: focus -> 1
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 1 );
}

UTEST(menu, up_wraps_to_last) {
    vxui_ctx ctx = {};
    menu_frame( &ctx );                     // frame 1: establish (count=3, focus=0)
    ctx.input = 0;

    vxui_input( &ctx, "up" );
    menu_frame( &ctx );                     // frame 2: wrap to row 2
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 2 );
}

UTEST(menu, down_wraps_to_first) {
    vxui_ctx ctx = {};
    menu_frame( &ctx );
    ctx.input = 0;

    // Move to last row.
    vxui_input( &ctx, "up" );
    menu_frame( &ctx );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 2 );
    ctx.input = 0;

    // Down from last wraps to 0.
    vxui_input( &ctx, "down" );
    menu_frame( &ctx );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 0 );
}

UTEST(menu, no_wrap_clamps_up) {
    vxui_ctx ctx = {};

    // Frame 1: establish with wrap=false.
    if ( vxui_menu( &ctx, "test", false ) )
    {
        vxui_menu_action( &ctx, "A" );
        vxui_menu_action( &ctx, "B" );
        vxui_menu_end( &ctx );
    }
    ctx.input = 0;

    // Frame 2: up from 0 stays at 0.
    vxui_input( &ctx, "up" );
    if ( vxui_menu( &ctx, "test", false ) )
    {
        vxui_menu_action( &ctx, "A" );
        vxui_menu_action( &ctx, "B" );
        vxui_menu_end( &ctx );
    }
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 0 );
}

/* ---- cancel ---------------------------------------------------------- */

UTEST(menu, cancelled_detects_cancel) {
    vxui_ctx ctx = {};
    vxui_input( &ctx, "cancel" );
    menu_frame( &ctx );
    ASSERT_TRUE( vxui_menu_cancelled( &ctx ) );
}

UTEST(menu, cancelled_false_without_input) {
    vxui_ctx ctx = {};
    menu_frame( &ctx );
    ASSERT_FALSE( vxui_menu_cancelled( &ctx ) );
}

/* ---- persistence ----------------------------------------------------- */

UTEST(menu, focus_persists_across_frames) {
    vxui_ctx ctx = {};
    menu_frame( &ctx );                     // frame 1: establish
    ctx.input = 0;

    vxui_input( &ctx, "down" );
    menu_frame( &ctx );                     // frame 2: focus -> 1
    ctx.input = 0;

    menu_frame( &ctx );                     // frame 3: no input, focus still 1
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 1 );
}

UTEST_MAIN();
