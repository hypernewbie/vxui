#include "utest.h"
#include "../vxui.h"
#define VXUI_IMPL
#include "../vxui_impl.h"
#include <cstring>

static uint8_t s_clay_mem[16 * 1024 * 1024];

static vxui_ctx make_ctx()
{
    vxui_ctx ctx = {};
    vxui_init( &ctx, 1280, 720, s_clay_mem, sizeof( s_clay_mem ) );
    return ctx;
}

static uint32_t row_id( const char* menu, const char* label )
{
    Clay_String cs = { false, (int32_t) strlen( label ), label };
    return Clay__HashString( cs, vxui_hash( menu ) ).id;
}

static bool any_text_contains( const vxui_draw_list& dl, const char* needle )
{
    int needle_len = (int) strlen( needle );
    int n = vxui_draw_count( dl, VXUI_DRAW_TEXT );
    for ( int i = 0; i < n; i++ )
    {
        const vxui_draw_cmd* t = vxui_draw_nth( dl, VXUI_DRAW_TEXT, i );
        for ( int j = 0; j + needle_len <= t->text_len; j++ )
            if ( memcmp( t->text + j, needle, needle_len ) == 0 ) return true;
    }
    return false;
}

static bool slider_frame( vxui_ctx* ctx, float* value, uint32_t input )
{
    vxui_frame( ctx, 1.0f / 60.0f );
    ctx->input = input;
    bool changed = false;
    if ( vxui_menu( ctx, "test" ) )
    {
        changed = vxui_menu_slider( ctx, "Volume", value, 0.0f, 1.0f, 0.1f );
        vxui_menu_end( ctx );
    }
    vxui_render( ctx );
    return changed;
}

UTEST(menu_slider, no_change_by_default) {
    vxui_ctx ctx = make_ctx();
    float v = 0.5f;
    slider_frame( &ctx, &v, 0 );            // establish

    bool changed = slider_frame( &ctx, &v, 0 );
    ASSERT_FALSE( changed );
    ASSERT_NEAR( v, 0.5f, 1e-6f );
}

UTEST(menu_slider, right_increments) {
    vxui_ctx ctx = make_ctx();
    float v = 0.5f;
    slider_frame( &ctx, &v, 0 );            // establish

    bool changed = slider_frame( &ctx, &v, VXUI_INPUT_RIGHT );
    ASSERT_TRUE( changed );
    ASSERT_NEAR( v, 0.6f, 1e-5f );
}

UTEST(menu_slider, left_decrements) {
    vxui_ctx ctx = make_ctx();
    float v = 0.5f;
    slider_frame( &ctx, &v, 0 );            // establish

    bool changed = slider_frame( &ctx, &v, VXUI_INPUT_LEFT );
    ASSERT_TRUE( changed );
    ASSERT_NEAR( v, 0.4f, 1e-5f );
}

UTEST(menu_slider, clamps_at_max) {
    vxui_ctx ctx = make_ctx();
    float v = 1.0f;
    slider_frame( &ctx, &v, 0 );            // establish

    slider_frame( &ctx, &v, VXUI_INPUT_RIGHT );
    ASSERT_NEAR( v, 1.0f, 1e-6f );
}

UTEST(menu_slider, clamps_at_min) {
    vxui_ctx ctx = make_ctx();
    float v = 0.0f;
    slider_frame( &ctx, &v, 0 );            // establish

    slider_frame( &ctx, &v, VXUI_INPUT_LEFT );
    ASSERT_NEAR( v, 0.0f, 1e-6f );
}

UTEST(menu_slider, only_fires_when_focused) {
    vxui_ctx ctx = make_ctx();
    float v = 0.5f;

    // Frame 1: establish (action + slider), focus on row 0.
    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "test" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_slider( &ctx, "Volume", &v, 0.0f, 1.0f, 0.1f );
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );

    // Frame 2: right input, focus is on row 0 (action), not the slider.
    bool changed = false;
    vxui_frame( &ctx, 1.0f / 60.0f );
    ctx.input = VXUI_INPUT_RIGHT;
    if ( vxui_menu( &ctx, "test" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        changed = vxui_menu_slider( &ctx, "Volume", &v, 0.0f, 1.0f, 0.1f );
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );
    ASSERT_FALSE( changed );
    ASSERT_NEAR( v, 0.5f, 1e-6f );
}

// Helper: full frame with custom range and step.
static bool step_frame( vxui_ctx* ctx, float* value, uint32_t input, float mn, float mx, float step )
{
    vxui_frame( ctx, 1.0f / 60.0f );
    ctx->input = input;
    bool changed = false;
    if ( vxui_menu( ctx, "test" ) )
    {
        changed = vxui_menu_slider( ctx, "V", value, mn, mx, step );
        vxui_menu_end( ctx );
    }
    vxui_render( ctx );
    return changed;
}

// Helper: simulate one user click (press + release) as two frames.
static void step_click( vxui_ctx* ctx, float* value, uint32_t input, float mn, float mx, float step )
{
    step_frame( ctx, value, input, mn, mx, step );
    step_frame( ctx, value, 0,     mn, mx, step );
}

UTEST(menu_slider, tiny_step_increments) {
    vxui_ctx ctx = make_ctx();
    float v = 0.5f;
    step_frame( &ctx, &v, 0,                0.0f, 1.0f, 0.001f );

    bool changed = step_frame( &ctx, &v, VXUI_INPUT_RIGHT, 0.0f, 1.0f, 0.001f );
    ASSERT_TRUE( changed );
    ASSERT_NEAR( v, 0.501f, 1e-5f );
}

UTEST(menu_slider, tiny_step_decrements) {
    vxui_ctx ctx = make_ctx();
    float v = 0.5f;
    step_frame( &ctx, &v, 0,                0.0f, 1.0f, 0.001f );

    bool changed = step_frame( &ctx, &v, VXUI_INPUT_LEFT, 0.0f, 1.0f, 0.001f );
    ASSERT_TRUE( changed );
    ASSERT_NEAR( v, 0.499f, 1e-5f );
}

UTEST(menu_slider, large_step_increments) {
    vxui_ctx ctx = make_ctx();
    float v = 0.0f;
    step_frame( &ctx, &v, 0,                0.0f, 1.0f, 0.5f );

    step_click( &ctx, &v, VXUI_INPUT_RIGHT, 0.0f, 1.0f, 0.5f );
    ASSERT_NEAR( v, 0.5f, 1e-5f );
    step_click( &ctx, &v, VXUI_INPUT_RIGHT, 0.0f, 1.0f, 0.5f );
    ASSERT_NEAR( v, 1.0f, 1e-5f );
    step_click( &ctx, &v, VXUI_INPUT_RIGHT, 0.0f, 1.0f, 0.5f );
    ASSERT_NEAR( v, 1.0f, 1e-5f );
}

UTEST(menu_slider, step_not_evenly_dividing_range_clamps_at_max) {
    vxui_ctx ctx = make_ctx();
    float v = 0.0f;
    step_frame( &ctx, &v, 0,                0.0f, 1.0f, 0.3f );

    for ( int i = 0; i < 10; i++ )
        step_click( &ctx, &v, VXUI_INPUT_RIGHT, 0.0f, 1.0f, 0.3f );

    ASSERT_NEAR( v, 1.0f, 1e-5f );
}

UTEST(menu_slider, step_not_evenly_dividing_range_clamps_at_min) {
    vxui_ctx ctx = make_ctx();
    float v = 1.0f;
    step_frame( &ctx, &v, 0,                0.0f, 1.0f, 0.3f );

    for ( int i = 0; i < 10; i++ )
        step_click( &ctx, &v, VXUI_INPUT_LEFT, 0.0f, 1.0f, 0.3f );

    ASSERT_NEAR( v, 0.0f, 1e-5f );
}

UTEST(menu_slider, custom_range_increments) {
    vxui_ctx ctx = make_ctx();
    float v = 15.0f;
    step_frame( &ctx, &v, 0,                10.0f, 20.0f, 1.0f );

    bool changed = step_frame( &ctx, &v, VXUI_INPUT_RIGHT, 10.0f, 20.0f, 1.0f );
    ASSERT_TRUE( changed );
    ASSERT_NEAR( v, 16.0f, 1e-5f );
}

UTEST(menu_slider, custom_range_clamps_at_max) {
    vxui_ctx ctx = make_ctx();
    float v = 20.0f;
    step_frame( &ctx, &v, 0,                10.0f, 20.0f, 1.0f );

    step_frame( &ctx, &v, VXUI_INPUT_RIGHT, 10.0f, 20.0f, 1.0f );
    ASSERT_NEAR( v, 20.0f, 1e-5f );
}

UTEST(menu_slider, custom_range_clamps_at_min) {
    vxui_ctx ctx = make_ctx();
    float v = 10.0f;
    step_frame( &ctx, &v, 0,                10.0f, 20.0f, 1.0f );

    step_frame( &ctx, &v, VXUI_INPUT_LEFT, 10.0f, 20.0f, 1.0f );
    ASSERT_NEAR( v, 10.0f, 1e-5f );
}

UTEST(menu_slider, negative_range_increments) {
    vxui_ctx ctx = make_ctx();
    float v = -1.0f;
    step_frame( &ctx, &v, 0,                -5.0f, 5.0f, 1.0f );

    bool changed = step_frame( &ctx, &v, VXUI_INPUT_RIGHT, -5.0f, 5.0f, 1.0f );
    ASSERT_TRUE( changed );
    ASSERT_NEAR( v, 0.0f, 1e-5f );
}

UTEST(menu_slider, negative_range_clamps_at_min) {
    vxui_ctx ctx = make_ctx();
    float v = -5.0f;
    step_frame( &ctx, &v, 0,                -5.0f, 5.0f, 1.0f );

    step_frame( &ctx, &v, VXUI_INPUT_LEFT, -5.0f, 5.0f, 1.0f );
    ASSERT_NEAR( v, -5.0f, 1e-5f );
}

UTEST(menu_slider, negative_range_clamps_at_max) {
    vxui_ctx ctx = make_ctx();
    float v = 5.0f;
    step_frame( &ctx, &v, 0,                -5.0f, 5.0f, 1.0f );

    step_frame( &ctx, &v, VXUI_INPUT_RIGHT, -5.0f, 5.0f, 1.0f );
    ASSERT_NEAR( v, 5.0f, 1e-5f );
}

UTEST(menu_slider, partial_step_clamps_at_max) {
    vxui_ctx ctx = make_ctx();
    float v = 0.95f;
    step_frame( &ctx, &v, 0,                0.0f, 1.0f, 0.1f );

    step_frame( &ctx, &v, VXUI_INPUT_RIGHT, 0.0f, 1.0f, 0.1f );
    ASSERT_NEAR( v, 1.0f, 1e-5f );
}

UTEST(menu_slider, partial_step_clamps_at_min) {
    vxui_ctx ctx = make_ctx();
    float v = 0.05f;
    step_frame( &ctx, &v, 0,                0.0f, 1.0f, 0.1f );

    step_frame( &ctx, &v, VXUI_INPUT_LEFT,  0.0f, 1.0f, 0.1f );
    ASSERT_NEAR( v, 0.0f, 1e-5f );
}

UTEST(menu_slider, tiny_step_long_ramp_no_drift) {
    vxui_ctx ctx = make_ctx();
    float v = 0.5f;
    step_frame( &ctx, &v, 0,                0.0f, 1.0f, 0.001f );

    for ( int i = 0; i < 100; i++ )
        step_click( &ctx, &v, VXUI_INPUT_RIGHT, 0.0f, 1.0f, 0.001f );

    ASSERT_NEAR( v, 0.6f, 1e-3f );
}

UTEST(menu_slider, large_base_small_step_increments) {
    vxui_ctx ctx = make_ctx();
    float v = 1000.0f;
    step_frame( &ctx, &v, 0,                0.0f, 10000.0f, 0.001f );

    bool changed = step_frame( &ctx, &v, VXUI_INPUT_RIGHT, 0.0f, 10000.0f, 0.001f );
    ASSERT_TRUE( changed );
    ASSERT_NEAR( v, 1000.001f, 1e-4f );
}

UTEST(menu_slider, single_step_covers_full_range) {
    vxui_ctx ctx = make_ctx();
    float v = 0.0f;
    step_frame( &ctx, &v, 0,                0.0f, 1.0f, 1.0f );

    step_frame( &ctx, &v, VXUI_INPUT_RIGHT, 0.0f, 1.0f, 1.0f );
    ASSERT_NEAR( v, 1.0f, 1e-5f );

    step_frame( &ctx, &v, VXUI_INPUT_LEFT,  0.0f, 1.0f, 1.0f );
    ASSERT_NEAR( v, 0.0f, 1e-5f );
}

/* ---- value display --------------------------------------------------- */

UTEST(menu_slider, value_text_emitted) {
    vxui_ctx ctx = make_ctx();
    float v = 0.5f;

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_slider( &ctx, "Volume", &v, 0.0f, 1.0f, 0.1f );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    // Two TEXT cmds per slider row: the label and the value display.
    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), 2 );
    ASSERT_TRUE( any_text_contains( dl, "Volume" ) );
    ASSERT_TRUE( any_text_contains( dl, "50%"    ) );
}

UTEST(menu_slider, value_text_updates_with_value) {
    vxui_ctx ctx = make_ctx();
    float v = 0.5f;

    slider_frame( &ctx, &v, 0 );
    slider_frame( &ctx, &v, VXUI_INPUT_RIGHT );

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "test" ) )
    {
        vxui_menu_slider( &ctx, "Volume", &v, 0.0f, 1.0f, 0.1f );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_NEAR( v, 0.6f, 1e-5f );
    ASSERT_TRUE ( any_text_contains( dl, "60%" ) );
    ASSERT_FALSE( any_text_contains( dl, "50%" ) );
}

UTEST(menu_slider, full_bar_at_max) {
    vxui_ctx ctx = make_ctx();
    float v = 1.0f;

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_slider( &ctx, "Volume", &v, 0.0f, 1.0f, 0.1f );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_TRUE( any_text_contains( dl, "##########" ) );
    ASSERT_TRUE( any_text_contains( dl, "100%"       ) );
}

UTEST(menu_slider, empty_bar_at_min) {
    vxui_ctx ctx = make_ctx();
    float v = 0.0f;

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_slider( &ctx, "Volume", &v, 0.0f, 1.0f, 0.1f );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_TRUE( any_text_contains( dl, "----------" ) );
    ASSERT_TRUE( any_text_contains( dl, "0%"         ) );
}

UTEST(menu_slider, row_id_stable_across_value_change) {
    vxui_ctx ctx = make_ctx();
    float v = 0.0f;

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_slider( &ctx, "Volume", &v, 0.0f, 1.0f, 0.1f );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl_0 = vxui_render( &ctx );
    uint32_t id_zero = vxui_draw_nth( dl_0, VXUI_DRAW_RECT, 0 )->id;

    v = 1.0f;
    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_slider( &ctx, "Volume", &v, 0.0f, 1.0f, 0.1f );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl_1 = vxui_render( &ctx );
    uint32_t id_full = vxui_draw_nth( dl_1, VXUI_DRAW_RECT, 0 )->id;

    ASSERT_EQ( id_zero, id_full );
    ASSERT_EQ( id_zero, row_id( "m", "Volume" ) );
}

UTEST_MAIN();
