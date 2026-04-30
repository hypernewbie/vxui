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

UTEST(input_just_pressed, transition_off_to_on_returns_true) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_input( &ctx, "down" );
    ASSERT_TRUE( vxui_input_just_pressed( &ctx, "down" ) );
    vxui_render( &ctx );
}

UTEST(input_just_pressed, held_returns_false) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_input( &ctx, "down" );
    vxui_render( &ctx );

    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_input( &ctx, "down" );
    ASSERT_TRUE ( vxui_input_pressed     ( &ctx, "down" ) );
    ASSERT_FALSE( vxui_input_just_pressed( &ctx, "down" ) );
    vxui_render( &ctx );
}

UTEST(input_just_pressed, released_returns_false) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_input( &ctx, "down" );
    vxui_render( &ctx );

    vxui_frame( &ctx, 1.0f / 60.0f );
    ASSERT_FALSE( vxui_input_pressed     ( &ctx, "down" ) );
    ASSERT_FALSE( vxui_input_just_pressed( &ctx, "down" ) );
    vxui_render( &ctx );
}

UTEST(input_just_pressed, re_press_after_release_returns_true) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_input( &ctx, "down" );
    vxui_render( &ctx );

    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_render( &ctx );

    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_input( &ctx, "down" );
    ASSERT_TRUE( vxui_input_just_pressed( &ctx, "down" ) );
    vxui_render( &ctx );
}

UTEST(input_just_pressed, multiple_inputs_independent) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_input( &ctx, "down" );
    vxui_render( &ctx );

    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_input( &ctx, "down" );
    vxui_input( &ctx, "confirm" );
    ASSERT_FALSE( vxui_input_just_pressed( &ctx, "down"    ) );
    ASSERT_TRUE ( vxui_input_just_pressed( &ctx, "confirm" ) );
    vxui_render( &ctx );
}

UTEST(input_just_pressed, prev_input_saved_at_frame_start) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    ctx.input = VXUI_INPUT_DOWN | VXUI_INPUT_CONFIRM;
    vxui_render( &ctx );

    vxui_frame( &ctx, 1.0f / 60.0f );
    ASSERT_EQ( ctx.prev_input, (uint32_t) ( VXUI_INPUT_DOWN | VXUI_INPUT_CONFIRM ) );
    ASSERT_EQ( ctx.input,      (uint32_t) 0 );
}

UTEST_MAIN();
