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

UTEST_MAIN();
