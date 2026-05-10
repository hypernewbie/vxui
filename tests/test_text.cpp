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

UTEST(text, emits_one_text_cmd) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_text( &ctx, "title", "VXUI", 64 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), 1 );
}

UTEST(text, emits_no_rect_cmd) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_text( &ctx, "title", "VXUI", 64 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 0 );
}

UTEST(text, label_contents_match) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_text( &ctx, "title", "VXUI DEMO", 64 );
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* t = vxui_draw_nth( dl, VXUI_DRAW_TEXT, 0 );
    ASSERT_TRUE( t != nullptr );
    ASSERT_EQ( t->text_len, 9 );
    ASSERT_EQ( strncmp( t->text, "VXUI DEMO", 9 ), 0 );
}

UTEST(text, font_px_matches_requested) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_text( &ctx, "title", "VXUI", 64 );
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* t = vxui_draw_nth( dl, VXUI_DRAW_TEXT, 0 );
    ASSERT_TRUE( t != nullptr );
    ASSERT_EQ( t->font_px, (uint16_t) 64 );
}

UTEST(text, font_px_zero_defaults) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_text( &ctx, "title", "VXUI" );
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* t = vxui_draw_nth( dl, VXUI_DRAW_TEXT, 0 );
    ASSERT_TRUE( t != nullptr );
    ASSERT_EQ( t->font_px, (uint16_t) VXUI_FONT_SIZE_DEFAULT );
}

UTEST(text, two_texts_emit_two_cmds) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_text( &ctx, "title",  "VXUI",          64 );
    vxui_text( &ctx, "prompt", "Press Enter",   24 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), 2 );
    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 0 );
}

UTEST_MAIN();
