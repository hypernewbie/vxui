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

static Clay_Dimensions measure_text( Clay_StringSlice s, Clay_TextElementConfig* cfg, void* )
{
    return { (float) s.length * 8.0f, (float) cfg->fontSize };
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
    ASSERT_EQ( vxui_draw_count( ctx.draw_list, VXUI_DRAW_TEXT ), 1 );
    ASSERT_EQ( ctx.text_offset, 6 );
}

UTEST(menu_slider, right_increments) {
    vxui_ctx ctx = make_ctx();
    float v = 0.5f;
    slider_frame( &ctx, &v, 0 );            // establish

    bool changed = slider_frame( &ctx, &v, VXUI_INPUT_RIGHT );
    ASSERT_TRUE( changed );
    ASSERT_NEAR( v, 0.6f, 1e-5f );
    ASSERT_EQ( vxui_draw_count( ctx.draw_list, VXUI_DRAW_TEXT ), 1 );
    ASSERT_EQ( ctx.text_offset, 6 );
}

UTEST(menu_slider, left_decrements) {
    vxui_ctx ctx = make_ctx();
    float v = 0.5f;
    slider_frame( &ctx, &v, 0 );            // establish

    bool changed = slider_frame( &ctx, &v, VXUI_INPUT_LEFT );
    ASSERT_TRUE( changed );
    ASSERT_NEAR( v, 0.4f, 1e-5f );
    ASSERT_EQ( vxui_draw_count( ctx.draw_list, VXUI_DRAW_TEXT ), 1 );
    ASSERT_EQ( ctx.text_offset, 6 );
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
    ASSERT_EQ( vxui_draw_count( ctx.draw_list, VXUI_DRAW_RECT ), 2 );
    ASSERT_EQ( vxui_draw_count( ctx.draw_list, VXUI_DRAW_TEXT ), 2 );
    const vxui_draw_cmd* t = vxui_draw_nth( ctx.draw_list, VXUI_DRAW_TEXT, 1 );
    ASSERT_TRUE( t != nullptr );
    ASSERT_EQ( t->text_len, 6 );
    ASSERT_EQ( memcmp( t->text, "Volume", 6 ), 0 );
    ASSERT_EQ( ctx.text_offset, 10 );
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

UTEST(menu_slider, only_label_text_emitted) {
    vxui_ctx ctx = make_ctx();
    float v = 0.5f;
    ASSERT_FALSE( slider_frame( &ctx, &v, 0 ) );
    vxui_draw_list dl = ctx.draw_list;

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 1 );
    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), 1 );
    const vxui_draw_cmd* r = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "test", "Volume" ) );
    const vxui_draw_cmd* t = vxui_draw_nth( dl, VXUI_DRAW_TEXT, 0 );
    ASSERT_TRUE( r && t );
    ASSERT_TRUE( ( r->state & VXUI_DRAW_FOCUSED ) != 0 );
    ASSERT_EQ( r->row_index, 0 );
    ASSERT_EQ( r->focused_row_index, 0 );
    ASSERT_EQ( t->text_len, 6 );
    ASSERT_EQ( memcmp( t->text, "Volume", 6 ), 0 );
    ASSERT_EQ( ctx.text_offset, 6 );
}

UTEST(menu_slider, label_unchanged_after_input) {
    vxui_ctx ctx = make_ctx();
    float v = 0.5f;
    slider_frame( &ctx, &v, 0 );
    uint32_t id = vxui_draw_nth( ctx.draw_list, VXUI_DRAW_RECT, 0 )->id;

    ASSERT_TRUE( slider_frame( &ctx, &v, VXUI_INPUT_RIGHT ) );
    ASSERT_NEAR( v, 0.6f, 1e-5f );
    ASSERT_EQ( vxui_draw_count( ctx.draw_list, VXUI_DRAW_RECT ), 1 );
    ASSERT_EQ( vxui_draw_count( ctx.draw_list, VXUI_DRAW_TEXT ), 1 );
    const vxui_draw_cmd* r = vxui_draw_find( ctx.draw_list, VXUI_DRAW_RECT, id );
    const vxui_draw_cmd* t = vxui_draw_nth( ctx.draw_list, VXUI_DRAW_TEXT, 0 );
    ASSERT_TRUE( r && t );
    ASSERT_TRUE( ( r->state & VXUI_DRAW_FOCUSED ) != 0 );
    ASSERT_EQ( t->text_len, 6 );
    ASSERT_EQ( memcmp( t->text, "Volume", 6 ), 0 );
    ASSERT_EQ( ctx.text_offset, 6 );
}

UTEST(menu_slider, maximum_value_emits_only_label) {
    vxui_ctx ctx = make_ctx();
    float v = 1.0f;
    ASSERT_FALSE( slider_frame( &ctx, &v, 0 ) );

    ASSERT_NEAR( v, 1.0f, 1e-6f );
    ASSERT_EQ( vxui_draw_count( ctx.draw_list, VXUI_DRAW_TEXT ), 1 );
    const vxui_draw_cmd* t = vxui_draw_nth( ctx.draw_list, VXUI_DRAW_TEXT, 0 );
    ASSERT_TRUE( t != nullptr );
    ASSERT_EQ( t->text_len, 6 );
    ASSERT_EQ( memcmp( t->text, "Volume", 6 ), 0 );
    ASSERT_EQ( ctx.text_offset, 6 );
}

UTEST(menu_slider, minimum_value_emits_only_label) {
    vxui_ctx ctx = make_ctx();
    float v = 0.0f;
    ASSERT_FALSE( slider_frame( &ctx, &v, 0 ) );

    ASSERT_NEAR( v, 0.0f, 1e-6f );
    ASSERT_EQ( vxui_draw_count( ctx.draw_list, VXUI_DRAW_TEXT ), 1 );
    const vxui_draw_cmd* t = vxui_draw_nth( ctx.draw_list, VXUI_DRAW_TEXT, 0 );
    ASSERT_TRUE( t != nullptr );
    ASSERT_EQ( t->text_len, 6 );
    ASSERT_EQ( memcmp( t->text, "Volume", 6 ), 0 );
    ASSERT_EQ( ctx.text_offset, 6 );
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

UTEST(menu_slider, row_width_depends_only_on_label) {
    vxui_ctx ctx = {};
    vxui_init( &ctx, 1280, 720, s_clay_mem, sizeof( s_clay_mem ) );
    Clay_SetMeasureTextFunction( measure_text, nullptr );
    float v = 0.0f;

    for ( int i = 0; i < 2; i++ )
    {
        v = (float) i;
        ASSERT_FALSE( slider_frame( &ctx, &v, 0 ) );
        ASSERT_EQ( vxui_draw_count( ctx.draw_list, VXUI_DRAW_RECT ), 1 );
        ASSERT_EQ( vxui_draw_count( ctx.draw_list, VXUI_DRAW_TEXT ), 1 );
        const vxui_draw_cmd* r = vxui_draw_find( ctx.draw_list, VXUI_DRAW_RECT, row_id( "test", "Volume" ) );
        const vxui_draw_cmd* t = vxui_draw_nth( ctx.draw_list, VXUI_DRAW_TEXT, 0 );
        ASSERT_TRUE( r && t );
        ASSERT_NEAR( r->rect.z, 48.0f, 0.01f );
        ASSERT_NEAR( r->rect.w, (float) VXUI_ROW_HEIGHT, 0.01f );
        ASSERT_NEAR( t->rect.z, 48.0f, 0.01f );
    }
}

UTEST(menu_slider, fill_row_still_fills_parent) {
    vxui_ctx ctx = {};
    vxui_init( &ctx, 1280, 720, s_clay_mem, sizeof( s_clay_mem ) );
    Clay_SetMeasureTextFunction( measure_text, nullptr );
    float v = 0.5f;

    vxui_frame( &ctx, 1.0f / 60.0f );
    ctx.input = VXUI_INPUT_RIGHT;
    bool changed = false;
    vxui_div( &ctx, "parent", { .width = { VXUI_FIXED, 480 }, .height = { VXUI_FIXED, 100 } } );
    if ( vxui_menu( &ctx, "test", true, 0, true ) )
    {
        changed = vxui_menu_slider( &ctx, "Volume", &v );
        vxui_menu_end( &ctx );
    }
    vxui_div_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_TRUE( changed );
    ASSERT_NEAR( v, 0.6f, 1e-5f );
    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 1 );
    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), 1 );
    const vxui_draw_cmd* r = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "test", "Volume" ) );
    const vxui_draw_cmd* t = vxui_draw_nth( dl, VXUI_DRAW_TEXT, 0 );
    ASSERT_TRUE( r && t );
    ASSERT_NEAR( r->rect.z, 480.0f, 0.01f );
    ASSERT_NEAR( r->rect.w, (float) VXUI_ROW_HEIGHT, 0.01f );
    ASSERT_NEAR( t->rect.z, 48.0f, 0.01f );
    ASSERT_EQ( t->text_len, 6 );
    ASSERT_EQ( memcmp( t->text, "Volume", 6 ), 0 );
}

UTEST(menu_slider, leading_section_promotes_focus_without_value_text) {
    vxui_ctx ctx = make_ctx();
    float v = 0.5f;
    vxui_frame( &ctx, 1.0f / 60.0f );
    ctx.input = VXUI_INPUT_RIGHT;
    bool changed = false;
    if ( vxui_menu( &ctx, "test" ) )
    {
        vxui_menu_section( &ctx, "Settings" );
        changed = vxui_menu_slider( &ctx, "Volume", &v );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_TRUE( changed );
    ASSERT_NEAR( v, 0.6f, 1e-5f );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 1 );
    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 2 );
    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), 2 );
    const vxui_draw_cmd* r = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "test", "Volume" ) );
    ASSERT_TRUE( r != nullptr );
    ASSERT_TRUE( ( r->state & VXUI_DRAW_FOCUSED ) != 0 );
    ASSERT_EQ( r->row_index, 1 );
    ASSERT_EQ( r->focused_row_index, 1 );
    ASSERT_EQ( ctx.text_offset, 14 );
}

UTEST(menu_slider, carried_click_changes_value_without_value_text) {
    vxui_ctx ctx = make_ctx();
    float v = 0.5f;
    vxui_frame( &ctx, 1.0f / 60.0f );
    ctx.input              = VXUI_INPUT_RIGHT;
    ctx.mouse_click_row_id = row_id( "test", "Volume" );
    bool changed = false;
    if ( vxui_menu( &ctx, "test" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        changed = vxui_menu_slider( &ctx, "Volume", &v );
        vxui_menu_action( &ctx, "Quit" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_TRUE( changed );
    ASSERT_NEAR( v, 0.6f, 1e-5f );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 1 );
    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 3 );
    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), 3 );
    const vxui_draw_cmd* r = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "test", "Volume" ) );
    const vxui_draw_cmd* t = vxui_draw_nth( dl, VXUI_DRAW_TEXT, 1 );
    ASSERT_TRUE( r && t );
    ASSERT_TRUE( ( r->state & VXUI_DRAW_FOCUSED ) != 0 );
    ASSERT_EQ( r->row_index, 1 );
    ASSERT_EQ( r->focused_row_index, 1 );
    ASSERT_EQ( t->text_len, 6 );
    ASSERT_EQ( memcmp( t->text, "Volume", 6 ), 0 );
    ASSERT_EQ( ctx.text_offset, 14 );
}

UTEST(menu_slider, caller_label_punctuation_is_preserved) {
    vxui_ctx ctx = make_ctx();
    const char* label = "[50%] <volume>";
    float v = 0.5f;
    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "test" ) )
    {
        vxui_menu_slider( &ctx, label, &v );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), 1 );
    const vxui_draw_cmd* t = vxui_draw_nth( dl, VXUI_DRAW_TEXT, 0 );
    ASSERT_TRUE( t != nullptr );
    ASSERT_EQ( t->text_len, (int) strlen( label ) );
    ASSERT_EQ( memcmp( t->text, label, t->text_len ), 0 );
    ASSERT_EQ( ctx.text_offset, t->text_len );
}

UTEST(menu_slider, two_sliders_emit_labels_only) {
    vxui_ctx ctx = make_ctx();
    float volume = 0.5f, gamma = 1.5f;
    vxui_frame( &ctx, 1.0f / 60.0f );
    ctx.input = VXUI_INPUT_RIGHT;
    bool volume_changed = false, gamma_changed = false;
    if ( vxui_menu( &ctx, "test" ) )
    {
        volume_changed = vxui_menu_slider( &ctx, "Volume", &volume );
        gamma_changed  = vxui_menu_slider( &ctx, "Gamma", &gamma, 0.0f, 3.0f, 0.25f );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_TRUE ( volume_changed );
    ASSERT_FALSE( gamma_changed );
    ASSERT_NEAR( volume, 0.6f, 1e-5f );
    ASSERT_NEAR( gamma, 1.5f, 1e-5f );
    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 2 );
    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), 2 );
    const vxui_draw_cmd* a = vxui_draw_nth( dl, VXUI_DRAW_TEXT, 0 );
    const vxui_draw_cmd* b = vxui_draw_nth( dl, VXUI_DRAW_TEXT, 1 );
    ASSERT_TRUE( a && b );
    ASSERT_EQ( a->text_len, 6 );
    ASSERT_EQ( b->text_len, 5 );
    ASSERT_EQ( memcmp( a->text, "Volume", 6 ), 0 );
    ASSERT_EQ( memcmp( b->text, "Gamma", 5 ), 0 );
    ASSERT_EQ( ctx.text_offset, 11 );
}

UTEST_MAIN();
