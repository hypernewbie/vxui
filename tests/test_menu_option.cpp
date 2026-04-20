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

static const char* s_diff[] = { "Easy", "Normal", "Hard" };
static const int   s_diff_n = 3;

// Helper: full frame with a single option row.
static bool option_frame( vxui_ctx* ctx, int* index, uint32_t input )
{
    vxui_frame( ctx, 1.0f / 60.0f );
    ctx->input = input;
    bool changed = false;
    if ( vxui_menu( ctx, "test" ) )
    {
        changed = vxui_menu_option( ctx, "Difficulty", index, s_diff, s_diff_n );
        vxui_menu_end( ctx );
    }
    vxui_render( ctx );
    return changed;
}

/* ---- basic ----------------------------------------------------------- */

UTEST(menu_option, no_change_by_default) {
    vxui_ctx ctx = make_ctx();
    int idx = 1;
    option_frame( &ctx, &idx, 0 );          // establish

    bool changed = option_frame( &ctx, &idx, 0 );
    ASSERT_FALSE( changed );
    ASSERT_EQ( idx, 1 );
}

/* ---- left/right ------------------------------------------------------ */

UTEST(menu_option, right_increments) {
    vxui_ctx ctx = make_ctx();
    int idx = 0;
    option_frame( &ctx, &idx, 0 );          // establish

    bool changed = option_frame( &ctx, &idx, VXUI_INPUT_RIGHT );
    ASSERT_TRUE( changed );
    ASSERT_EQ( idx, 1 );
}

UTEST(menu_option, left_decrements) {
    vxui_ctx ctx = make_ctx();
    int idx = 2;
    option_frame( &ctx, &idx, 0 );          // establish

    bool changed = option_frame( &ctx, &idx, VXUI_INPUT_LEFT );
    ASSERT_TRUE( changed );
    ASSERT_EQ( idx, 1 );
}

UTEST(menu_option, right_wraps) {
    vxui_ctx ctx = make_ctx();
    int idx = 2;
    option_frame( &ctx, &idx, 0 );          // establish

    option_frame( &ctx, &idx, VXUI_INPUT_RIGHT );
    ASSERT_EQ( idx, 0 );
}

UTEST(menu_option, left_wraps) {
    vxui_ctx ctx = make_ctx();
    int idx = 0;
    option_frame( &ctx, &idx, 0 );          // establish

    option_frame( &ctx, &idx, VXUI_INPUT_LEFT );
    ASSERT_EQ( idx, s_diff_n - 1 );
}

/* ---- focus ----------------------------------------------------------- */

UTEST(menu_option, only_fires_when_focused) {
    vxui_ctx ctx = make_ctx();
    int idx = 0;

    // Frame 1: establish a two-row menu (action + option), focus on row 0.
    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "test" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_option( &ctx, "Difficulty", &idx, s_diff, s_diff_n );
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );

    // Frame 2: right input, focus is on row 0 (action), not the option.
    bool changed = false;
    vxui_frame( &ctx, 1.0f / 60.0f );
    ctx.input = VXUI_INPUT_RIGHT;
    if ( vxui_menu( &ctx, "test" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        changed = vxui_menu_option( &ctx, "Difficulty", &idx, s_diff, s_diff_n );
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );
    ASSERT_FALSE( changed );
    ASSERT_EQ( idx, 0 );
}

UTEST_MAIN();
