#include "utest.h"
#include "../vxui.h"
#define VXUI_IMPL
#include "../vxui_impl.h"

UTEST(page, default_is_nothing) {
    vxui_ctx ctx = {};
    ASSERT_FALSE(vxui_page(&ctx, "title"));
    ASSERT_FALSE(vxui_page(&ctx, "options"));
}

UTEST(page, switch_and_check) {
    vxui_ctx ctx = {};
    vxui_switch(&ctx, "title");
    ASSERT_TRUE(vxui_page(&ctx, "title"));
    ASSERT_FALSE(vxui_page(&ctx, "options"));
}

UTEST(page, switch_replaces) {
    vxui_ctx ctx = {};
    vxui_switch(&ctx, "title");
    vxui_switch(&ctx, "options");
    ASSERT_FALSE(vxui_page(&ctx, "title"));
    ASSERT_TRUE(vxui_page(&ctx, "options"));
}

/* ---- input ----------------------------------------------------------- */

UTEST(input, nothing_by_default) {
    vxui_ctx ctx = {};
    ASSERT_EQ(ctx.input, (uint32_t)0);
}

UTEST(input, cancel) {
    vxui_ctx ctx = {};
    vxui_input(&ctx, "cancel");
    ASSERT_TRUE(ctx.input & VXUI_INPUT_CANCEL);
}

UTEST(input, directions) {
    vxui_ctx ctx = {};
    vxui_input(&ctx, "up");
    vxui_input(&ctx, "left");
    ASSERT_TRUE(ctx.input & VXUI_INPUT_UP);
    ASSERT_TRUE(ctx.input & VXUI_INPUT_LEFT);
    ASSERT_FALSE(ctx.input & VXUI_INPUT_DOWN);
}

UTEST(input, confirm) {
    vxui_ctx ctx = {};
    vxui_input(&ctx, "confirm");
    ASSERT_TRUE(ctx.input & VXUI_INPUT_CONFIRM);
}

UTEST(input, release) {
    vxui_ctx ctx = {};
    vxui_input(&ctx, "up");
    ASSERT_TRUE(ctx.input & VXUI_INPUT_UP);
    vxui_input(&ctx, "up", false);
    ASSERT_FALSE(ctx.input & VXUI_INPUT_UP);
}

/* ---- input_pressed --------------------------------------------------- */

UTEST(input_pressed, reads_back_true) {
    vxui_ctx ctx = {};
    vxui_input( &ctx, "confirm" );
    ASSERT_TRUE( vxui_input_pressed( &ctx, "confirm" ) );
}

UTEST(input_pressed, reads_back_false) {
    vxui_ctx ctx = {};
    ASSERT_FALSE( vxui_input_pressed( &ctx, "up" ) );
    ASSERT_FALSE( vxui_input_pressed( &ctx, "cancel" ) );
}

UTEST(input_pressed, tracks_release) {
    vxui_ctx ctx = {};
    vxui_input( &ctx, "left" );
    ASSERT_TRUE( vxui_input_pressed( &ctx, "left" ) );
    vxui_input( &ctx, "left", false );
    ASSERT_FALSE( vxui_input_pressed( &ctx, "left" ) );
}

UTEST(input_pressed, independent_actions) {
    vxui_ctx ctx = {};
    vxui_input( &ctx, "up" );
    vxui_input( &ctx, "confirm" );
    ASSERT_TRUE( vxui_input_pressed( &ctx, "up" ) );
    ASSERT_TRUE( vxui_input_pressed( &ctx, "confirm" ) );
    ASSERT_FALSE( vxui_input_pressed( &ctx, "down" ) );
    ASSERT_FALSE( vxui_input_pressed( &ctx, "cancel" ) );
}

UTEST_MAIN();
