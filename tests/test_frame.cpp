#include "utest.h"
#include "../vxui.h"
#define VXUI_IMPL
#include "../vxui_impl.h"

UTEST(frame, clears_input) {
    vxui_ctx ctx = {};
    vxui_input( &ctx, "confirm" );
    vxui_input( &ctx, "up" );
    ASSERT_NE( ctx.input, (uint32_t) 0 );

    vxui_frame( &ctx, 1.0f / 60.0f );
    ASSERT_EQ( ctx.input, (uint32_t) 0 );
}

UTEST(frame, stores_dt) {
    vxui_ctx ctx = {};
    vxui_frame( &ctx, 1.0f / 30.0f );
    ASSERT_NEAR( ctx.dt, 1.0f / 30.0f, 1e-6f );
}

UTEST(frame, render_returns_empty) {
    vxui_ctx ctx = {};
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_draw_list dl = vxui_render( &ctx );
    ASSERT_EQ( dl.count, 0 );
}

UTEST(frame, input_after_frame_works) {
    // Input set AFTER vxui_frame should still be visible to the menu system.
    vxui_ctx ctx = {};

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "A" );
        vxui_menu_action( &ctx, "B" );
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );

    // Frame 2: input after frame, before declarations.
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

UTEST_MAIN();
