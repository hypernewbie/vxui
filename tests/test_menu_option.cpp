#include "utest.h"
#include "../vxui.h"
#define VXUI_IMPL
#include "../vxui_impl.h"

static const char* s_diff[] = { "Easy", "Normal", "Hard" };
static const int   s_diff_n = 3;

// Helper: one frame with a single option row.
static bool option_frame( vxui_ctx* ctx, int* index )
{
    bool changed = false;
    if ( vxui_menu( ctx, "test" ) )
    {
        changed = vxui_menu_option( ctx, "Difficulty", index, s_diff, s_diff_n );
        vxui_menu_end( ctx );
    }
    return changed;
}

/* ---- basic ----------------------------------------------------------- */

UTEST(menu_option, no_change_by_default) {
    vxui_ctx ctx = {};
    int idx = 1;
    option_frame( &ctx, &idx );     // establish
    ctx.input = 0;

    bool changed = option_frame( &ctx, &idx );
    ASSERT_FALSE( changed );
    ASSERT_EQ( idx, 1 );
}

/* ---- left/right ------------------------------------------------------ */

UTEST(menu_option, right_increments) {
    vxui_ctx ctx = {};
    int idx = 0;
    option_frame( &ctx, &idx );     // establish
    ctx.input = 0;

    vxui_input( &ctx, "right" );
    bool changed = option_frame( &ctx, &idx );
    ASSERT_TRUE( changed );
    ASSERT_EQ( idx, 1 );
}

UTEST(menu_option, left_decrements) {
    vxui_ctx ctx = {};
    int idx = 2;
    option_frame( &ctx, &idx );     // establish
    ctx.input = 0;

    vxui_input( &ctx, "left" );
    bool changed = option_frame( &ctx, &idx );
    ASSERT_TRUE( changed );
    ASSERT_EQ( idx, 1 );
}

UTEST(menu_option, right_wraps) {
    vxui_ctx ctx = {};
    int idx = 2;
    option_frame( &ctx, &idx );     // establish
    ctx.input = 0;

    vxui_input( &ctx, "right" );
    option_frame( &ctx, &idx );
    ASSERT_EQ( idx, 0 );
}

UTEST(menu_option, left_wraps) {
    vxui_ctx ctx = {};
    int idx = 0;
    option_frame( &ctx, &idx );     // establish
    ctx.input = 0;

    vxui_input( &ctx, "left" );
    option_frame( &ctx, &idx );
    ASSERT_EQ( idx, s_diff_n - 1 );
}

/* ---- focus ----------------------------------------------------------- */

UTEST(menu_option, only_fires_when_focused) {
    vxui_ctx ctx = {};
    int idx = 0;

    // Frame 1: establish a two-row menu (action + option).
    if ( vxui_menu( &ctx, "test" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_option( &ctx, "Difficulty", &idx, s_diff, s_diff_n );
        vxui_menu_end( &ctx );
    }
    ctx.input = 0;

    // Frame 2: right input, focus is on row 0 (action), not the option.
    vxui_input( &ctx, "right" );
    bool changed = false;
    if ( vxui_menu( &ctx, "test" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        changed = vxui_menu_option( &ctx, "Difficulty", &idx, s_diff, s_diff_n );
        vxui_menu_end( &ctx );
    }
    ASSERT_FALSE( changed );
    ASSERT_EQ( idx, 0 );
}

UTEST_MAIN();
