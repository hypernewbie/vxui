#include "utest.h"
#include "../vxui.h"
#define VXUI_IMPL
#include "../vxui_impl.h"

static bool slider_frame( vxui_ctx* ctx, float* value )
{
    bool changed = false;
    if ( vxui_menu( ctx, "test" ) )
    {
        changed = vxui_menu_slider( ctx, "Volume", value, 0.0f, 1.0f, 0.1f );  // mn=0, mx=1
        vxui_menu_end( ctx );
    }
    return changed;
}

UTEST(menu_slider, no_change_by_default) {
    vxui_ctx ctx = {};
    float v = 0.5f;
    slider_frame( &ctx, &v );
    ctx.input = 0;

    bool changed = slider_frame( &ctx, &v );
    ASSERT_FALSE( changed );
    ASSERT_NEAR( v, 0.5f, 1e-6f );
}

UTEST(menu_slider, right_increments) {
    vxui_ctx ctx = {};
    float v = 0.5f;
    slider_frame( &ctx, &v );
    ctx.input = 0;

    vxui_input( &ctx, "right" );
    bool changed = slider_frame( &ctx, &v );
    ASSERT_TRUE( changed );
    ASSERT_NEAR( v, 0.6f, 1e-5f );
}

UTEST(menu_slider, left_decrements) {
    vxui_ctx ctx = {};
    float v = 0.5f;
    slider_frame( &ctx, &v );
    ctx.input = 0;

    vxui_input( &ctx, "left" );
    bool changed = slider_frame( &ctx, &v );
    ASSERT_TRUE( changed );
    ASSERT_NEAR( v, 0.4f, 1e-5f );
}

UTEST(menu_slider, clamps_at_max) {
    vxui_ctx ctx = {};
    float v = 1.0f;
    slider_frame( &ctx, &v );
    ctx.input = 0;

    vxui_input( &ctx, "right" );
    slider_frame( &ctx, &v );
    ASSERT_NEAR( v, 1.0f, 1e-6f );
}

UTEST(menu_slider, clamps_at_min) {
    vxui_ctx ctx = {};
    float v = 0.0f;
    slider_frame( &ctx, &v );
    ctx.input = 0;

    vxui_input( &ctx, "left" );
    slider_frame( &ctx, &v );
    ASSERT_NEAR( v, 0.0f, 1e-6f );
}

UTEST(menu_slider, only_fires_when_focused) {
    vxui_ctx ctx = {};
    float v = 0.5f;

    if ( vxui_menu( &ctx, "test" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_slider( &ctx, "Volume", &v, 0.0f, 1.0f, 0.1f );
        vxui_menu_end( &ctx );
    }
    ctx.input = 0;

    vxui_input( &ctx, "right" );
    bool changed = false;
    if ( vxui_menu( &ctx, "test" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        changed = vxui_menu_slider( &ctx, "Volume", &v, 0.0f, 1.0f, 0.1f );
        vxui_menu_end( &ctx );
    }
    ASSERT_FALSE( changed );
    ASSERT_NEAR( v, 0.5f, 1e-6f );
}

UTEST_MAIN();
