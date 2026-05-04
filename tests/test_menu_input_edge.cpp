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

// Single-action menu: returns true on the frame the action fires.
static bool action_frame( vxui_ctx* ctx, uint32_t input, float dt )
{
    vxui_frame( ctx, dt );
    ctx->input = input;
    bool fired = false;
    if ( vxui_menu( ctx, "test" ) )
    {
        if ( vxui_menu_action( ctx, "Play" ) ) fired = true;
        vxui_menu_end( ctx );
    }
    vxui_render( ctx );
    return fired;
}

UTEST(menu_input_edge, confirm_held_fires_only_once) {
    vxui_ctx ctx = make_ctx();
    action_frame( &ctx, 0, 0.05f );

    ASSERT_TRUE ( action_frame( &ctx, VXUI_INPUT_CONFIRM, 0.05f ) );
    ASSERT_FALSE( action_frame( &ctx, VXUI_INPUT_CONFIRM, 0.05f ) );
    ASSERT_FALSE( action_frame( &ctx, VXUI_INPUT_CONFIRM, 0.05f ) );
    ASSERT_FALSE( action_frame( &ctx, VXUI_INPUT_CONFIRM, 0.05f ) );
}

UTEST(menu_input_edge, confirm_re_press_fires_again) {
    vxui_ctx ctx = make_ctx();
    action_frame( &ctx, 0, 0.05f );

    ASSERT_TRUE ( action_frame( &ctx, VXUI_INPUT_CONFIRM, 0.05f ) );
    ASSERT_FALSE( action_frame( &ctx, VXUI_INPUT_CONFIRM, 0.05f ) );
    ASSERT_FALSE( action_frame( &ctx, 0,                  0.05f ) );
    ASSERT_TRUE ( action_frame( &ctx, VXUI_INPUT_CONFIRM, 0.05f ) );
}

UTEST(menu_input_edge, confirm_held_across_das_does_not_repeat) {
    vxui_ctx ctx = make_ctx();
    action_frame( &ctx, 0, 0.05f );

    ASSERT_TRUE( action_frame( &ctx, VXUI_INPUT_CONFIRM, 0.05f ) );
    for ( int i = 0; i < 30; i++ )
        ASSERT_FALSE( action_frame( &ctx, VXUI_INPUT_CONFIRM, 0.05f ) );
}

// Cancel uses a query function rather than returning from the row.
static bool cancel_frame( vxui_ctx* ctx, uint32_t input, float dt )
{
    vxui_frame( ctx, dt );
    ctx->input = input;
    bool cancelled = false;
    if ( vxui_menu( ctx, "test" ) )
    {
        vxui_menu_action( ctx, "Play" );
        cancelled = vxui_menu_cancelled( ctx );
        vxui_menu_end( ctx );
    }
    vxui_render( ctx );
    return cancelled;
}

UTEST(menu_input_edge, cancel_held_fires_only_once) {
    vxui_ctx ctx = make_ctx();
    cancel_frame( &ctx, 0, 0.05f );

    ASSERT_TRUE ( cancel_frame( &ctx, VXUI_INPUT_CANCEL, 0.05f ) );
    ASSERT_FALSE( cancel_frame( &ctx, VXUI_INPUT_CANCEL, 0.05f ) );
    ASSERT_FALSE( cancel_frame( &ctx, VXUI_INPUT_CANCEL, 0.05f ) );
}

UTEST(menu_input_edge, cancel_re_press_fires_again) {
    vxui_ctx ctx = make_ctx();
    cancel_frame( &ctx, 0, 0.05f );

    ASSERT_TRUE ( cancel_frame( &ctx, VXUI_INPUT_CANCEL, 0.05f ) );
    ASSERT_FALSE( cancel_frame( &ctx, 0,                 0.05f ) );
    ASSERT_TRUE ( cancel_frame( &ctx, VXUI_INPUT_CANCEL, 0.05f ) );
}

static const char* s_diff[] = { "Easy", "Normal", "Hard" };
static const int   s_diff_n = 3;

static int option_frame( vxui_ctx* ctx, int* idx, uint32_t input, float dt )
{
    vxui_frame( ctx, dt );
    ctx->input = input;
    if ( vxui_menu( ctx, "test" ) )
    {
        vxui_menu_option( ctx, "Difficulty", idx, s_diff, s_diff_n );
        vxui_menu_end( ctx );
    }
    vxui_render( ctx );
    return *idx;
}

UTEST(menu_input_edge, option_right_held_no_repeat_within_das) {
    vxui_ctx ctx = make_ctx();
    int idx = 0;
    option_frame( &ctx, &idx, 0, 0.05f );

    ASSERT_EQ( option_frame( &ctx, &idx, VXUI_INPUT_RIGHT, 0.05f ), 1 );
    for ( int i = 0; i < 7; i++ )
        ASSERT_EQ( option_frame( &ctx, &idx, VXUI_INPUT_RIGHT, 0.05f ), 1 );
}

UTEST(menu_input_edge, option_right_held_repeats_after_das) {
    vxui_ctx ctx = make_ctx();
    int idx = 0;
    option_frame( &ctx, &idx, 0, 0.05f );

    ASSERT_EQ( option_frame( &ctx, &idx, VXUI_INPUT_RIGHT, 0.05f ), 1 );
    for ( int i = 0; i < 7; i++ )
        option_frame( &ctx, &idx, VXUI_INPUT_RIGHT, 0.05f );
    ASSERT_EQ( option_frame( &ctx, &idx, VXUI_INPUT_RIGHT, 0.05f ), 2 );
}

UTEST(menu_input_edge, option_left_held_no_repeat_within_das) {
    vxui_ctx ctx = make_ctx();
    int idx = 2;
    option_frame( &ctx, &idx, 0, 0.05f );

    ASSERT_EQ( option_frame( &ctx, &idx, VXUI_INPUT_LEFT, 0.05f ), 1 );
    for ( int i = 0; i < 7; i++ )
        ASSERT_EQ( option_frame( &ctx, &idx, VXUI_INPUT_LEFT, 0.05f ), 1 );
}

UTEST(menu_input_edge, option_re_press_fires_again) {
    vxui_ctx ctx = make_ctx();
    int idx = 0;
    option_frame( &ctx, &idx, 0, 0.05f );

    ASSERT_EQ( option_frame( &ctx, &idx, VXUI_INPUT_RIGHT, 0.05f ), 1 );
    ASSERT_EQ( option_frame( &ctx, &idx, 0,                0.05f ), 1 );
    ASSERT_EQ( option_frame( &ctx, &idx, VXUI_INPUT_RIGHT, 0.05f ), 2 );
}

static float slider_frame( vxui_ctx* ctx, float* v, uint32_t input, float dt )
{
    vxui_frame( ctx, dt );
    ctx->input = input;
    if ( vxui_menu( ctx, "test" ) )
    {
        vxui_menu_slider( ctx, "Volume", v, 0.0f, 1.0f, 0.1f );
        vxui_menu_end( ctx );
    }
    vxui_render( ctx );
    return *v;
}

UTEST(menu_input_edge, slider_right_held_no_repeat_within_das) {
    vxui_ctx ctx = make_ctx();
    float v = 0.0f;
    slider_frame( &ctx, &v, 0, 0.05f );

    ASSERT_NEAR( slider_frame( &ctx, &v, VXUI_INPUT_RIGHT, 0.05f ), 0.1f, 1e-5f );
    for ( int i = 0; i < 7; i++ )
        ASSERT_NEAR( slider_frame( &ctx, &v, VXUI_INPUT_RIGHT, 0.05f ), 0.1f, 1e-5f );
}

UTEST(menu_input_edge, slider_right_held_repeats_after_das) {
    vxui_ctx ctx = make_ctx();
    float v = 0.0f;
    slider_frame( &ctx, &v, 0, 0.05f );

    ASSERT_NEAR( slider_frame( &ctx, &v, VXUI_INPUT_RIGHT, 0.05f ), 0.1f, 1e-5f );
    for ( int i = 0; i < 7; i++ )
        slider_frame( &ctx, &v, VXUI_INPUT_RIGHT, 0.05f );
    ASSERT_NEAR( slider_frame( &ctx, &v, VXUI_INPUT_RIGHT, 0.05f ), 0.2f, 1e-5f );
}

UTEST(menu_input_edge, option_release_resets_das_delay) {
    // After a press + partial hold + release, the next press starts a fresh
    // DAS delay — i.e. held_time is reset on release. Without that reset,
    // a quick re-press during the prior DAS window would auto-repeat instantly.
    vxui_ctx ctx = make_ctx();
    int idx = 0;
    option_frame( &ctx, &idx, 0, 0.05f );

    ASSERT_EQ( option_frame( &ctx, &idx, VXUI_INPUT_RIGHT, 0.05f ), 1 );
    for ( int i = 0; i < 6; i++ )
        option_frame( &ctx, &idx, VXUI_INPUT_RIGHT, 0.05f );
    ASSERT_EQ( idx, 1 );                                    // 0.30s held, no DAS yet

    option_frame( &ctx, &idx, 0, 0.05f );                   // release

    ASSERT_EQ( option_frame( &ctx, &idx, VXUI_INPUT_RIGHT, 0.05f ), 2 );
    for ( int i = 0; i < 6; i++ )
        option_frame( &ctx, &idx, VXUI_INPUT_RIGHT, 0.05f );
    ASSERT_EQ( idx, 2 );                                    // fresh 0.30s, no DAS yet
}

UTEST(menu_input_edge, slider_re_press_fires_again) {
    vxui_ctx ctx = make_ctx();
    float v = 0.0f;
    slider_frame( &ctx, &v, 0, 0.05f );

    ASSERT_NEAR( slider_frame( &ctx, &v, VXUI_INPUT_RIGHT, 0.05f ), 0.1f, 1e-5f );
    ASSERT_NEAR( slider_frame( &ctx, &v, 0,                0.05f ), 0.1f, 1e-5f );
    ASSERT_NEAR( slider_frame( &ctx, &v, VXUI_INPUT_RIGHT, 0.05f ), 0.2f, 1e-5f );
}

UTEST_MAIN();
