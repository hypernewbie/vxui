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

UTEST(frame, clears_input) {
    vxui_ctx ctx = make_ctx();
    vxui_input( &ctx, "confirm" );
    vxui_input( &ctx, "up" );
    ASSERT_NE( ctx.input, (uint32_t) 0 );

    vxui_frame( &ctx, 1.0f / 60.0f );
    ASSERT_EQ( ctx.input, (uint32_t) 0 );
    vxui_render( &ctx );
}

UTEST(frame, stores_dt) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 30.0f );
    ASSERT_NEAR( ctx.dt, 1.0f / 30.0f, 1e-6f );
    vxui_render( &ctx );
}

UTEST(frame, render_returns_empty) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_draw_list dl = vxui_render( &ctx );
    ASSERT_EQ( dl.count, 0 );
}

UTEST(frame, input_after_frame_works) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "A" );
        vxui_menu_action( &ctx, "B" );
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );

    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_input( &ctx, "down" );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "A" );
        vxui_menu_action( &ctx, "B" );
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );

    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 1 );
}

UTEST(frame, dimensions_default_to_init) {
    vxui_ctx ctx = make_ctx();   // init at 1280x720
    vxui_frame( &ctx, 1.0f / 60.0f );
    Clay_Dimensions d = ( (Clay_Context*) ctx.clay )->layoutDimensions;
    ASSERT_NEAR( d.width,  1280.0f, 1e-3f );
    ASSERT_NEAR( d.height,  720.0f, 1e-3f );
    vxui_render( &ctx );
}

UTEST(frame, dimensions_set_per_frame) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f, 800.0f, 600.0f );
    Clay_Dimensions d = ( (Clay_Context*) ctx.clay )->layoutDimensions;
    ASSERT_NEAR( d.width,  800.0f, 1e-3f );
    ASSERT_NEAR( d.height, 600.0f, 1e-3f );
    vxui_render( &ctx );
}

UTEST(frame, dimensions_change_between_frames) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f, 800.0f, 600.0f );
    vxui_render( &ctx );

    vxui_frame( &ctx, 1.0f / 60.0f, 1920.0f, 1080.0f );
    Clay_Dimensions d = ( (Clay_Context*) ctx.clay )->layoutDimensions;
    ASSERT_NEAR( d.width,  1920.0f, 1e-3f );
    ASSERT_NEAR( d.height, 1080.0f, 1e-3f );
    vxui_render( &ctx );
}

UTEST(frame, dimensions_zero_preserves_previous) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f, 800.0f, 600.0f );
    vxui_render( &ctx );

    vxui_frame( &ctx, 1.0f / 60.0f );   // 0,0 = no change
    Clay_Dimensions d = ( (Clay_Context*) ctx.clay )->layoutDimensions;
    ASSERT_NEAR( d.width,  800.0f, 1e-3f );
    ASSERT_NEAR( d.height, 600.0f, 1e-3f );
    vxui_render( &ctx );
}

UTEST_MAIN();
