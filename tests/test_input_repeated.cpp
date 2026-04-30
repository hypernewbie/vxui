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

UTEST(input_repeated, single_tap_fires_once) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 0.05f );
    vxui_input( &ctx, "down" );
    ASSERT_TRUE( vxui_input_repeated( &ctx, "down" ) );
    vxui_render( &ctx );

    vxui_frame( &ctx, 0.05f );
    ASSERT_FALSE( vxui_input_repeated( &ctx, "down" ) );
    vxui_render( &ctx );
}

UTEST(input_repeated, hold_no_fire_until_delay) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 0.05f );
    vxui_input( &ctx, "down" );
    ASSERT_TRUE( vxui_input_repeated( &ctx, "down" ) );
    vxui_render( &ctx );

    for ( int i = 0; i < 7; i++ )
    {
        vxui_frame( &ctx, 0.05f );
        vxui_input( &ctx, "down" );
        ASSERT_FALSE( vxui_input_repeated( &ctx, "down" ) );
        vxui_render( &ctx );
    }

    vxui_frame( &ctx, 0.05f );
    vxui_input( &ctx, "down" );
    ASSERT_TRUE( vxui_input_repeated( &ctx, "down" ) );
    vxui_render( &ctx );
}

UTEST(input_repeated, hold_fires_at_repeat_interval) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 0.05f );
    vxui_input( &ctx, "down" );
    vxui_render( &ctx );

    for ( int i = 0; i < 8; i++ )
    {
        vxui_frame( &ctx, 0.05f );
        vxui_input( &ctx, "down" );
        bool expect = ( i == 7 );
        ASSERT_TRUE( vxui_input_repeated( &ctx, "down" ) == expect );
        vxui_render( &ctx );
    }

    vxui_frame( &ctx, 0.05f );
    vxui_input( &ctx, "down" );
    ASSERT_FALSE( vxui_input_repeated( &ctx, "down" ) );
    vxui_render( &ctx );

    vxui_frame( &ctx, 0.05f );
    vxui_input( &ctx, "down" );
    ASSERT_TRUE( vxui_input_repeated( &ctx, "down" ) );
    vxui_render( &ctx );
}

UTEST(input_repeated, release_resets_held_time) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 0.05f );
    vxui_input( &ctx, "down" );
    ASSERT_TRUE( vxui_input_repeated( &ctx, "down" ) );
    vxui_render( &ctx );

    for ( int i = 0; i < 4; i++ )
    {
        vxui_frame( &ctx, 0.05f );
        vxui_input( &ctx, "down" );
        ASSERT_FALSE( vxui_input_repeated( &ctx, "down" ) );
        vxui_render( &ctx );
    }

    vxui_frame( &ctx, 0.05f );
    vxui_render( &ctx );

    vxui_frame( &ctx, 0.05f );
    vxui_input( &ctx, "down" );
    ASSERT_TRUE( vxui_input_repeated( &ctx, "down" ) );
    vxui_render( &ctx );

    ASSERT_NEAR( ctx.input_held_time[1], 0.0f, 1e-5f );    // bit 1 = down
}

UTEST(input_repeated, multiple_inputs_independent) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 0.05f );
    vxui_input( &ctx, "down" );
    ASSERT_TRUE ( vxui_input_repeated( &ctx, "down"  ) );
    ASSERT_FALSE( vxui_input_repeated( &ctx, "right" ) );
    vxui_render( &ctx );

    vxui_frame( &ctx, 0.05f );
    vxui_input( &ctx, "down" );
    vxui_input( &ctx, "right" );
    ASSERT_FALSE( vxui_input_repeated( &ctx, "down"  ) );
    ASSERT_TRUE ( vxui_input_repeated( &ctx, "right" ) );
    vxui_render( &ctx );
}

UTEST(input_repeated, commit_is_idempotent_within_frame) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 0.05f );
    vxui_input( &ctx, "down" );
    vxui_render( &ctx );

    vxui_frame( &ctx, 0.05f );
    vxui_input( &ctx, "down" );

    for ( int i = 0; i < 5; i++ )
        vxui_input_repeated( &ctx, "down" );

    ASSERT_NEAR( ctx.input_held_time[1], 0.05f, 1e-5f );
    vxui_render( &ctx );
}

static int menu_auto_repeat_frame( vxui_ctx* ctx, uint32_t input, bool auto_repeat, float dt )
{
    vxui_frame( ctx, dt );
    ctx->input = input;
    if ( vxui_menu( ctx, "test", true, 0, auto_repeat ) )
    {
        vxui_menu_action( ctx, "A" );
        vxui_menu_action( ctx, "B" );
        vxui_menu_action( ctx, "C" );
        vxui_menu_action( ctx, "D" );
        vxui_menu_action( ctx, "E" );
        vxui_menu_end( ctx );
    }
    vxui_render( ctx );
    return (int) ctx->menu_state[0].y;
}

UTEST(input_repeated, menu_auto_repeat_true_holds_position) {
    vxui_ctx ctx = make_ctx();
    menu_auto_repeat_frame( &ctx, 0, true, 0.05f );

    int row = menu_auto_repeat_frame( &ctx, VXUI_INPUT_DOWN, true, 0.05f );
    ASSERT_EQ( row, 1 );

    for ( int i = 0; i < 7; i++ )
    {
        row = menu_auto_repeat_frame( &ctx, VXUI_INPUT_DOWN, true, 0.05f );
        ASSERT_EQ( row, 1 );
    }

    row = menu_auto_repeat_frame( &ctx, VXUI_INPUT_DOWN, true, 0.05f );
    ASSERT_EQ( row, 2 );
}

UTEST(input_repeated, menu_auto_repeat_false_rolls_on_hold) {
    vxui_ctx ctx = make_ctx();
    menu_auto_repeat_frame( &ctx, 0, false, 0.05f );

    int row = menu_auto_repeat_frame( &ctx, VXUI_INPUT_DOWN, false, 0.05f );
    ASSERT_EQ( row, 1 );

    row = menu_auto_repeat_frame( &ctx, VXUI_INPUT_DOWN, false, 0.05f );
    ASSERT_EQ( row, 2 );

    row = menu_auto_repeat_frame( &ctx, VXUI_INPUT_DOWN, false, 0.05f );
    ASSERT_EQ( row, 3 );
}

UTEST(input_repeated, menu_auto_repeat_true_release_resets) {
    vxui_ctx ctx = make_ctx();
    menu_auto_repeat_frame( &ctx, 0, true, 0.05f );
    menu_auto_repeat_frame( &ctx, VXUI_INPUT_DOWN, true, 0.05f );

    for ( int i = 0; i < 5; i++ )
        menu_auto_repeat_frame( &ctx, VXUI_INPUT_DOWN, true, 0.05f );
    int row = (int) ctx.menu_state[0].y;
    ASSERT_EQ( row, 1 );

    menu_auto_repeat_frame( &ctx, 0, true, 0.05f );

    row = menu_auto_repeat_frame( &ctx, VXUI_INPUT_DOWN, true, 0.05f );
    ASSERT_EQ( row, 2 );
}

UTEST_MAIN();
