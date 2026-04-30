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

// Helper: declare two-action menu with wrap configurable.
static void wrap_body( vxui_ctx* ctx, bool wrap )
{
    if ( vxui_menu( ctx, "test", wrap ) )
    {
        vxui_menu_action( ctx, "A" );
        vxui_menu_action( ctx, "B" );
        vxui_menu_end( ctx );
    }
}

static void wrap_frame( vxui_ctx* ctx, uint32_t input, bool wrap )
{
    vxui_frame( ctx, 1.0f / 60.0f );
    ctx->input = input;
    wrap_body( ctx, wrap );
    vxui_render( ctx );
}

UTEST(menu, no_wrap_clamps_down) {
    vxui_ctx ctx = make_ctx();
    wrap_frame( &ctx, 0,                false );
    wrap_frame( &ctx, VXUI_INPUT_DOWN,  false );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 1 );
    wrap_frame( &ctx, VXUI_INPUT_DOWN,  false );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 1 );
}

UTEST(menu, no_wrap_down_then_up_clamps_at_top) {
    vxui_ctx ctx = make_ctx();
    wrap_frame( &ctx, 0,                false );
    wrap_frame( &ctx, VXUI_INPUT_DOWN,  false );
    wrap_frame( &ctx, VXUI_INPUT_UP,    false );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 0 );
    wrap_frame( &ctx, VXUI_INPUT_UP,    false );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 0 );
}

// Helper: section + 2 actions, with wrap configurable.
static void no_wrap_section_body( vxui_ctx* ctx, bool wrap )
{
    if ( vxui_menu( ctx, "test", wrap ) )
    {
        vxui_menu_section( ctx, "Header" );
        vxui_menu_action ( ctx, "Play" );
        vxui_menu_action ( ctx, "Quit" );
        vxui_menu_end( ctx );
    }
}

static void no_wrap_section_frame( vxui_ctx* ctx, uint32_t input, bool wrap )
{
    vxui_frame( ctx, 1.0f / 60.0f );
    ctx->input = input;
    no_wrap_section_body( ctx, wrap );
    vxui_render( ctx );
}

UTEST(menu, no_wrap_with_leading_skip_up_clamps_at_first_action) {
    vxui_ctx ctx = make_ctx();
    no_wrap_section_frame( &ctx, 0,             false );
    no_wrap_section_frame( &ctx, 0,             false );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 1 );

    no_wrap_section_frame( &ctx, VXUI_INPUT_UP, false );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 1 );
}

UTEST(menu, no_wrap_with_leading_skip_down_then_up_clamps_at_first_action) {
    vxui_ctx ctx = make_ctx();
    no_wrap_section_frame( &ctx, 0,                false );
    no_wrap_section_frame( &ctx, VXUI_INPUT_DOWN,  false );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 2 );
    no_wrap_section_frame( &ctx, VXUI_INPUT_UP,    false );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 1 );
    no_wrap_section_frame( &ctx, VXUI_INPUT_UP,    false );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 1 );
}

// Helper: 2 actions then a trailing label (skip row), wrap configurable.
static void no_wrap_trailing_skip_body( vxui_ctx* ctx, bool wrap )
{
    if ( vxui_menu( ctx, "test", wrap ) )
    {
        vxui_menu_action ( ctx, "A" );
        vxui_menu_action ( ctx, "B" );
        vxui_menu_label  ( ctx, "Tail" );
        vxui_menu_end( ctx );
    }
}

static void no_wrap_trailing_skip_frame( vxui_ctx* ctx, uint32_t input, bool wrap )
{
    vxui_frame( ctx, 1.0f / 60.0f );
    ctx->input = input;
    no_wrap_trailing_skip_body( ctx, wrap );
    vxui_render( ctx );
}

UTEST(menu, no_wrap_with_trailing_skip_down_clamps_at_last_action) {
    vxui_ctx ctx = make_ctx();
    no_wrap_trailing_skip_frame( &ctx, 0,                false );
    no_wrap_trailing_skip_frame( &ctx, VXUI_INPUT_DOWN,  false );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 1 );
    no_wrap_trailing_skip_frame( &ctx, VXUI_INPUT_DOWN,  false );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 1 );
}

// Helper: action, label (skip row), action, wrap configurable.
static void no_wrap_middle_skip_body( vxui_ctx* ctx, bool wrap )
{
    if ( vxui_menu( ctx, "test", wrap ) )
    {
        vxui_menu_action ( ctx, "A" );
        vxui_menu_label  ( ctx, "Mid" );
        vxui_menu_action ( ctx, "B" );
        vxui_menu_end( ctx );
    }
}

static void no_wrap_middle_skip_frame( vxui_ctx* ctx, uint32_t input, bool wrap )
{
    vxui_frame( ctx, 1.0f / 60.0f );
    ctx->input = input;
    no_wrap_middle_skip_body( ctx, wrap );
    vxui_render( ctx );
}

UTEST(menu, no_wrap_with_middle_skip_jumps_over) {
    vxui_ctx ctx = make_ctx();
    no_wrap_middle_skip_frame( &ctx, 0,                false );
    no_wrap_middle_skip_frame( &ctx, VXUI_INPUT_DOWN,  false );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 2 );
    no_wrap_middle_skip_frame( &ctx, VXUI_INPUT_UP,    false );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 0 );
}

// Helper: action, two consecutive label skips, action, with wrap configurable.
static void no_wrap_double_middle_skip_body( vxui_ctx* ctx, bool wrap )
{
    if ( vxui_menu( ctx, "test", wrap ) )
    {
        vxui_menu_action ( ctx, "A" );
        vxui_menu_label  ( ctx, "L1" );
        vxui_menu_label  ( ctx, "L2" );
        vxui_menu_action ( ctx, "B" );
        vxui_menu_end( ctx );
    }
}

static void no_wrap_double_middle_skip_frame( vxui_ctx* ctx, uint32_t input, bool wrap )
{
    vxui_frame( ctx, 1.0f / 60.0f );
    ctx->input = input;
    no_wrap_double_middle_skip_body( ctx, wrap );
    vxui_render( ctx );
}

UTEST(menu, no_wrap_jumps_over_two_consecutive_skip_rows) {
    vxui_ctx ctx = make_ctx();
    no_wrap_double_middle_skip_frame( &ctx, 0,                false );
    no_wrap_double_middle_skip_frame( &ctx, VXUI_INPUT_DOWN,  false );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 3 );
    no_wrap_double_middle_skip_frame( &ctx, VXUI_INPUT_UP,    false );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 0 );
}

// Helper: section, action, label, action, label, with wrap configurable.
static void no_wrap_combined_skip_body( vxui_ctx* ctx, bool wrap )
{
    if ( vxui_menu( ctx, "test", wrap ) )
    {
        vxui_menu_section( ctx, "Top" );
        vxui_menu_action ( ctx, "A" );
        vxui_menu_label  ( ctx, "Mid" );
        vxui_menu_action ( ctx, "B" );
        vxui_menu_label  ( ctx, "End" );
        vxui_menu_end( ctx );
    }
}

static void no_wrap_combined_skip_frame( vxui_ctx* ctx, uint32_t input, bool wrap )
{
    vxui_frame( ctx, 1.0f / 60.0f );
    ctx->input = input;
    no_wrap_combined_skip_body( ctx, wrap );
    vxui_render( ctx );
}

UTEST(menu, no_wrap_with_combined_leading_middle_trailing_skip) {
    vxui_ctx ctx = make_ctx();
    no_wrap_combined_skip_frame( &ctx, 0,                false );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 1 );

    no_wrap_combined_skip_frame( &ctx, VXUI_INPUT_DOWN,  false );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 3 );

    no_wrap_combined_skip_frame( &ctx, VXUI_INPUT_DOWN,  false );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 3 );

    no_wrap_combined_skip_frame( &ctx, VXUI_INPUT_UP,    false );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 1 );

    no_wrap_combined_skip_frame( &ctx, VXUI_INPUT_UP,    false );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 1 );
}

// Helper: all-skip menu (section + label only), wrap configurable.
static void all_skip_body( vxui_ctx* ctx, bool wrap )
{
    if ( vxui_menu( ctx, "test", wrap ) )
    {
        vxui_menu_section( ctx, "S" );
        vxui_menu_label  ( ctx, "L" );
        vxui_menu_end( ctx );
    }
}

static void all_skip_frame( vxui_ctx* ctx, uint32_t input, bool wrap )
{
    vxui_frame( ctx, 1.0f / 60.0f );
    ctx->input = input;
    all_skip_body( ctx, wrap );
    vxui_render( ctx );
}

UTEST(menu, all_skip_menu_no_wrap_input_does_nothing) {
    vxui_ctx ctx = make_ctx();
    all_skip_frame( &ctx, 0,                false );
    uint32_t row_after_init = ctx.menu_state[0].y;

    all_skip_frame( &ctx, VXUI_INPUT_DOWN,  false );
    ASSERT_EQ( ctx.menu_state[0].y, row_after_init );

    all_skip_frame( &ctx, VXUI_INPUT_UP,    false );
    ASSERT_EQ( ctx.menu_state[0].y, row_after_init );
}

UTEST(menu, all_skip_menu_wrap_input_does_nothing) {
    vxui_ctx ctx = make_ctx();
    all_skip_frame( &ctx, 0,                true );
    uint32_t row_after_init = ctx.menu_state[0].y;

    all_skip_frame( &ctx, VXUI_INPUT_DOWN,  true );
    ASSERT_EQ( ctx.menu_state[0].y, row_after_init );

    all_skip_frame( &ctx, VXUI_INPUT_UP,    true );
    ASSERT_EQ( ctx.menu_state[0].y, row_after_init );
}

UTEST(menu, all_skip_menu_emits_no_focus_rect) {
    vxui_ctx ctx = make_ctx();
    all_skip_frame( &ctx, 0, true );
    vxui_draw_list dl = ctx.draw_list;

    ASSERT_EQ( dl.count, 2 );
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
