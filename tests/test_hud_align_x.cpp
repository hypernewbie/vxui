#include "utest.h"
#include "../vxui.h"
#define VXUI_IMPL
#include "../vxui_impl.h"
#define VXUI_HUD_IMPL
#include "../vxui_hud.h"

static uint8_t s_clay_mem[16 * 1024 * 1024];

static Clay_Dimensions measure_text( Clay_StringSlice s, Clay_TextElementConfig* cfg, void* )
{
    return { (float) s.length * 8.0f, (float) cfg->fontSize };
}

static vxui_ctx make_ctx()
{
    vxui_ctx ctx = {};
    vxui_init( &ctx, 960, 540, s_clay_mem, sizeof( s_clay_mem ) );
    Clay_SetMeasureTextFunction( measure_text, nullptr );
    return ctx;
}

UTEST(hud_align_x, single_line_positions_match_all_nine_alignments) {
    vxui_ctx ctx = make_ctx();
    float xs[] = { 10.5f, 50.5f, 90.5f };
    float ys[] = { 20.25f, 56.25f, 92.25f };
    for ( uint8_t x = 0; x < 3; x++ )
    {
        for ( uint8_t y = 0; y < 3; y++ )
        {
            vxui_frame( &ctx, 1.0f / 60.0f );
            vxui_hud hud = {};
            vxui_hud_begin( &hud, &ctx, 960, 540 );
            vxui_hud_set_pos( &hud, 10.5f, 20.25f );
            vxui_hud_text_box( &hud, "box", "alpha", 120, 96, x, y );
            vxui_draw_list dl = vxui_render( &ctx );

            ASSERT_EQ( dl.count, 1 );
            ASSERT_EQ( dl.cmds[0].rect.x, xs[x] );
            ASSERT_EQ( dl.cmds[0].rect.y, ys[y] );
            ASSERT_EQ( dl.cmds[0].rect.z, 40.0f );
            ASSERT_EQ( dl.cmds[0].rect.w, 24.0f );
        }
    }
}

UTEST(hud_align_x, default_box_alignment_is_centered) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_set_pos( &hud, 10.5f, 20.25f );
    vxui_hud_text_box( &hud, "box", "alpha", 120, 96 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 1 );
    ASSERT_EQ( dl.cmds[0].rect.x, 50.5f );
}

UTEST(hud_align_x, multiline_element_uses_the_widest_line_for_alignment) {
    vxui_ctx ctx = make_ctx();
    float xs[] = { 10.5f, 50.5f, 90.5f };
    for ( uint8_t x = 0; x < 3; x++ )
    {
        vxui_frame( &ctx, 1.0f / 60.0f );
        vxui_hud hud = {};
        vxui_hud_begin( &hud, &ctx, 960, 540 );
        vxui_hud_set_pos( &hud, 10.5f, 20.25f );
        vxui_hud_text_box( &hud, "box", "alpha\nbeta", 120, 96, x, 0 );
        vxui_draw_list dl = vxui_render( &ctx );

        ASSERT_EQ( dl.count, 2 );
        for ( int i = 0; i < 2; i++ ) ASSERT_EQ( dl.cmds[i].rect.x, xs[x] );
    }
}

UTEST(hud_align_x, hud_alignment_agrees_with_a_core_div) {
    vxui_ctx ctx = make_ctx();
    for ( uint8_t x = 0; x < 3; x++ )
    {
        vxui_frame( &ctx, 1.0f / 60.0f );
        vxui_root( &ctx, "root", 10.5f, 20.25f );
        vxui_div( &ctx, "div", { .width = { VXUI_FIXED, 120 }, .height = { VXUI_FIXED, 96 }, .align_x = x } );
        vxui_text( &ctx, "core", "alpha", 24 );
        vxui_div_end( &ctx );
        vxui_root_end( &ctx );
        vxui_hud hud = {};
        vxui_hud_begin( &hud, &ctx, 960, 540 );
        vxui_hud_set_pos( &hud, 10.5f, 20.25f );
        vxui_hud_text_box( &hud, "box", "alpha", 120, 96, x, 0 );
        vxui_draw_list dl = vxui_render( &ctx );

        const vxui_draw_cmd* core = vxui_draw_find( dl, VXUI_DRAW_TEXT, vxui_text_id( "core" ) );
        const vxui_draw_cmd* box = vxui_draw_find( dl, VXUI_DRAW_TEXT, vxui_text_id( "box" ) );
        ASSERT_TRUE( core && box );
        ASSERT_EQ( box->rect.x, core->rect.x );
        ASSERT_EQ( box->rect.z, core->rect.z );
    }
}

UTEST(hud_align_x, fractional_center_offset_is_not_rounded) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_set_pos( &hud, 10.25f, 20.5f );
    vxui_hud_text_box( &hud, "box", "abc", 121, 80, 1, 0 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 1 );
    ASSERT_EQ( dl.cmds[0].rect.x, 58.75f );
    ASSERT_EQ( dl.cmds[0].rect.z, 24.0f );
}

UTEST(hud_align_x, alignment_change_preserves_id_metadata_and_cursor) {
    vxui_ctx ctx = make_ctx();
    for ( uint8_t x = 0; x < 3; x++ )
    {
        vxui_frame( &ctx, 1.0f / 60.0f );
        vxui_hud hud = {};
        vxui_hud_begin( &hud, &ctx, 960, 540 );
        vxui_hud_set_pos( &hud, 10, 20 );
        vxui_hud_set_colour( &hud, { 0.2f, 0.4f, 0.6f, 0.8f } );
        vxui_hud_text_box( &hud, "box", "alpha", 120, 96, x, 0 );
        vxui_draw_list dl = vxui_render( &ctx );

        ASSERT_EQ( dl.count, 1 );
        ASSERT_EQ( dl.cmds[0].id, vxui_text_id( "box" ) );
        ASSERT_EQ( dl.cmds[0].userdata, (const void*) &hud.items[0] );
        ASSERT_EQ( hud.y, 116.0f );
        vxui_render_data out = {};
        ASSERT_TRUE( vxui_hud_resolve( &hud, &dl.cmds[0], &out ) );
        ASSERT_EQ( out.colour.r, 0.2f );
        ASSERT_EQ( out.colour.a, 0.8f );
    }
}

UTEST(hud_align_x, empty_box_emits_no_alignment_packet) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_text_box( &hud, "empty", "", 120, 96, 2, 0 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 0 );
    ASSERT_EQ( hud.item_count, 1 );
    ASSERT_EQ( hud.y, 96.0f );
}

UTEST(hud_align_x, existing_default_mapping_is_preserved_for_every_byte) {
    vxui_ctx ctx = make_ctx();
    for ( int x = 0; x < 256; x++ )
    {
        vxui_frame( &ctx, 1.0f / 60.0f );
        vxui_hud hud = {};
        vxui_hud_begin( &hud, &ctx, 960, 540 );
        vxui_hud_text_box( &hud, "box", "alpha", 120, 96, (uint8_t) x, 0 );
        vxui_draw_list dl = vxui_render( &ctx );

        ASSERT_EQ( dl.count, 1 );
        ASSERT_EQ( dl.cmds[0].rect.x, x == 1 ? 40.0f : x == 2 ? 80.0f : 0.0f );
    }
}

UTEST_MAIN();
