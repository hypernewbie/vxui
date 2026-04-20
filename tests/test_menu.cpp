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

// Helper: declare the menu body only. Caller owns vxui_frame / vxui_render.
static int menu_body( vxui_ctx* ctx )
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

// Helper: full frame — begin, set input, declare menu, render.
static int menu_frame( vxui_ctx* ctx, uint32_t input )
{
    vxui_frame( ctx, 1.0f / 60.0f );
    ctx->input = input;
    int fired = menu_body( ctx );
    vxui_render( ctx );
    return fired;
}

/* ---- basic ----------------------------------------------------------- */

UTEST(menu, opens_and_closes) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    ASSERT_TRUE( vxui_menu( &ctx, "m" ) );
    vxui_menu_end( &ctx );
    vxui_render( &ctx );
}

UTEST(menu, focus_starts_at_zero) {
    vxui_ctx ctx = make_ctx();
    menu_frame( &ctx, 0 );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 0 );
}

/* ---- confirm --------------------------------------------------------- */

UTEST(menu, confirm_fires_focused) {
    vxui_ctx ctx = make_ctx();
    menu_frame( &ctx, 0 );                              // frame 1: establish (count=3)

    int fired = menu_frame( &ctx, VXUI_INPUT_CONFIRM ); // frame 2: confirm on row 0
    ASSERT_EQ( fired, 0 );
}

UTEST(menu, confirm_only_fires_focused) {
    vxui_ctx ctx = make_ctx();
    menu_frame( &ctx, 0 );                              // frame 1: establish
    menu_frame( &ctx, VXUI_INPUT_DOWN );                // frame 2: move to row 1

    int fired = menu_frame( &ctx, VXUI_INPUT_CONFIRM ); // frame 3: confirm on row 1
    ASSERT_EQ( fired, 1 );
}

/* ---- navigation ------------------------------------------------------ */

UTEST(menu, down_moves_focus) {
    vxui_ctx ctx = make_ctx();
    menu_frame( &ctx, 0 );                  // frame 1: establish (count=3)
    menu_frame( &ctx, VXUI_INPUT_DOWN );    // frame 2: focus -> 1
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 1 );
}

UTEST(menu, up_wraps_to_last) {
    vxui_ctx ctx = make_ctx();
    menu_frame( &ctx, 0 );                  // frame 1: establish (count=3, focus=0)
    menu_frame( &ctx, VXUI_INPUT_UP );      // frame 2: wrap to row 2
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 2 );
}

UTEST(menu, down_wraps_to_first) {
    vxui_ctx ctx = make_ctx();
    menu_frame( &ctx, 0 );
    menu_frame( &ctx, VXUI_INPUT_UP );      // move to last row (2)
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 2 );
    menu_frame( &ctx, VXUI_INPUT_DOWN );    // down from last wraps to 0
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 0 );
}

UTEST(menu, no_wrap_clamps_up) {
    vxui_ctx ctx = make_ctx();

    // Frame 1: establish with wrap=false.
    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "test", false ) )
    {
        vxui_menu_action( &ctx, "A" );
        vxui_menu_action( &ctx, "B" );
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );

    // Frame 2: up from 0 stays at 0.
    vxui_frame( &ctx, 1.0f / 60.0f );
    ctx.input = VXUI_INPUT_UP;
    if ( vxui_menu( &ctx, "test", false ) )
    {
        vxui_menu_action( &ctx, "A" );
        vxui_menu_action( &ctx, "B" );
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 0 );
}

/* ---- cancel ---------------------------------------------------------- */

UTEST(menu, cancelled_detects_cancel) {
    vxui_ctx ctx = make_ctx();
    menu_frame( &ctx, VXUI_INPUT_CANCEL );
    ASSERT_TRUE( vxui_menu_cancelled( &ctx ) );
}

UTEST(menu, cancelled_false_without_input) {
    vxui_ctx ctx = make_ctx();
    menu_frame( &ctx, 0 );
    ASSERT_FALSE( vxui_menu_cancelled( &ctx ) );
}

/* ---- persistence ----------------------------------------------------- */

UTEST(menu, focus_persists_across_frames) {
    vxui_ctx ctx = make_ctx();
    menu_frame( &ctx, 0 );                  // frame 1: establish
    menu_frame( &ctx, VXUI_INPUT_DOWN );    // frame 2: focus -> 1
    menu_frame( &ctx, 0 );                  // frame 3: no input, focus still 1
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 1 );
}

UTEST_MAIN();
