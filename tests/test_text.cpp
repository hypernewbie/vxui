#include "utest.h"
#include "../vxui.h"
#define VXUI_IMPL
#include "../vxui_impl.h"

static uint8_t s_clay_mem[16 * 1024 * 1024];
static uint8_t s_clay_mem2[16 * 1024 * 1024];

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

UTEST(text, cmd_text_points_into_ctx_buffer) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_text( &ctx, "title", "VXUI", 64 );
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* t = vxui_draw_nth( dl, VXUI_DRAW_TEXT, 0 );
    ASSERT_TRUE( t != nullptr );
    ASSERT_TRUE( t->text >= ctx.text_buf && t->text < ctx.text_buf + VXUI_MAX_TEXT_BYTES );
}

UTEST(text, two_ctx_do_not_share_text) {
    vxui_ctx ctx_a = {};
    vxui_ctx ctx_b = {};
    vxui_init( &ctx_a, 1280, 720, s_clay_mem,  sizeof( s_clay_mem ) );
    vxui_init( &ctx_b, 1280, 720, s_clay_mem2, sizeof( s_clay_mem2 ) );

    vxui_frame( &ctx_a, 1.0f / 60.0f );
    vxui_text( &ctx_a, "title", "Alpha", 32 );
    vxui_draw_list dl_a = vxui_render( &ctx_a );

    vxui_frame( &ctx_b, 1.0f / 60.0f );
    vxui_text( &ctx_b, "title", "Beta", 32 );
    vxui_draw_list dl_b = vxui_render( &ctx_b );

    const vxui_draw_cmd* ta = vxui_draw_nth( dl_a, VXUI_DRAW_TEXT, 0 );
    const vxui_draw_cmd* tb = vxui_draw_nth( dl_b, VXUI_DRAW_TEXT, 0 );
    ASSERT_TRUE( ta != nullptr && tb != nullptr );
    ASSERT_EQ( strncmp( ta->text, "Alpha", 5 ), 0 );
    ASSERT_EQ( strncmp( tb->text, "Beta",  4 ), 0 );
    ASSERT_TRUE( ta->text >= ctx_a.text_buf && ta->text < ctx_a.text_buf + VXUI_MAX_TEXT_BYTES );
    ASSERT_TRUE( tb->text >= ctx_b.text_buf && tb->text < ctx_b.text_buf + VXUI_MAX_TEXT_BYTES );
}

UTEST(text, cmd_id_matches_vxui_text_id) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_text( &ctx, "title", "VXUI", 64 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_TRUE( vxui_draw_find( dl, VXUI_DRAW_TEXT, vxui_text_id( "title" ) ) != nullptr );
}

UTEST(text, offset_resets_each_frame) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_text( &ctx, "title", "VXUI", 64 );
    vxui_render( &ctx );

    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_text( &ctx, "title", "VXUI", 64 );
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* t = vxui_draw_nth( dl, VXUI_DRAW_TEXT, 0 );
    ASSERT_TRUE( t != nullptr );
    ASSERT_EQ( t->text, ctx.text_buf );
}

UTEST_MAIN();
