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

UTEST(hud_align_y, single_line_positions_use_remaining_box_height) {
    vxui_ctx ctx = make_ctx();
    float ys[] = { 20.25f, 56.25f, 92.25f };
    for ( uint8_t y = 0; y < 3; y++ )
    {
        vxui_frame( &ctx, 1.0f / 60.0f );
        vxui_hud hud = {};
        vxui_hud_begin( &hud, &ctx, 960, 540 );
        vxui_hud_set_pos( &hud, 10.5f, 20.25f );
        vxui_hud_text_box( &hud, "box", "alpha", 120, 96, 0, y );
        vxui_draw_list dl = vxui_render( &ctx );

        ASSERT_EQ( dl.count, 1 );
        ASSERT_EQ( dl.cmds[0].rect.y, ys[y] );
        ASSERT_EQ( dl.cmds[0].rect.w, 24.0f );
    }
}

UTEST(hud_align_y, multiline_height_moves_the_whole_text_element) {
    vxui_ctx ctx = make_ctx();
    float ys[] = { 20.25f, 44.25f, 68.25f };
    for ( uint8_t y = 0; y < 3; y++ )
    {
        vxui_frame( &ctx, 1.0f / 60.0f );
        vxui_hud hud = {};
        vxui_hud_begin( &hud, &ctx, 960, 540 );
        vxui_hud_set_pos( &hud, 10.5f, 20.25f );
        vxui_hud_text_box( &hud, "box", "alpha\nbeta", 120, 96, 0, y );
        vxui_draw_list dl = vxui_render( &ctx );

        ASSERT_EQ( dl.count, 2 );
        for ( int i = 0; i < 2; i++ ) ASSERT_EQ( dl.cmds[i].rect.y, ys[y] + (float) i * 24.0f );
    }
}

UTEST(hud_align_y, wrapped_height_is_used_for_vertical_alignment) {
    vxui_ctx ctx = make_ctx();
    float ys[] = { 20.25f, 32.25f, 44.25f };
    for ( uint8_t y = 0; y < 3; y++ )
    {
        vxui_frame( &ctx, 1.0f / 60.0f );
        vxui_hud hud = {};
        vxui_hud_begin( &hud, &ctx, 960, 540 );
        vxui_hud_set_pos( &hud, 10.5f, 20.25f );
        vxui_hud_text_box( &hud, "box", "alpha beta gamma", 48, 96, 0, y );
        vxui_draw_list dl = vxui_render( &ctx );

        ASSERT_EQ( dl.count, 3 );
        for ( int i = 0; i < 3; i++ ) ASSERT_EQ( dl.cmds[i].rect.y, ys[y] + (float) i * 24.0f );
    }
}

UTEST(hud_align_y, hud_alignment_agrees_with_a_core_div) {
    vxui_ctx ctx = make_ctx();
    for ( uint8_t y = 0; y < 3; y++ )
    {
        vxui_frame( &ctx, 1.0f / 60.0f );
        vxui_root( &ctx, "root", 10.5f, 20.25f );
        vxui_div( &ctx, "div", { .width = { VXUI_FIXED, 120 }, .height = { VXUI_FIXED, 96 }, .align_y = y } );
        vxui_text( &ctx, "core", "alpha", 24 );
        vxui_div_end( &ctx );
        vxui_root_end( &ctx );
        vxui_hud hud = {};
        vxui_hud_begin( &hud, &ctx, 960, 540 );
        vxui_hud_set_pos( &hud, 10.5f, 20.25f );
        vxui_hud_text_box( &hud, "box", "alpha", 120, 96, 0, y );
        vxui_draw_list dl = vxui_render( &ctx );

        const vxui_draw_cmd* core = vxui_draw_find( dl, VXUI_DRAW_TEXT, vxui_text_id( "core" ) );
        const vxui_draw_cmd* box = vxui_draw_find( dl, VXUI_DRAW_TEXT, vxui_text_id( "box" ) );
        ASSERT_TRUE( core && box );
        ASSERT_EQ( box->rect.y, core->rect.y );
        ASSERT_EQ( box->rect.w, core->rect.w );
    }
}

UTEST(hud_align_y, odd_font_height_keeps_fractional_center_offset) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_set_pos( &hud, 10.5f, 20.25f );
    vxui_hud_set_font( &hud, 0, 27 );
    vxui_hud_text_box( &hud, "box", "alpha", 120, 96, 0, 1 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 1 );
    ASSERT_EQ( dl.cmds[0].rect.y, 54.75f );
    ASSERT_EQ( dl.cmds[0].rect.w, 27.0f );
}

UTEST(hud_align_y, alignment_preserves_line_ids_metadata_and_cursor) {
    vxui_ctx ctx = make_ctx();
    for ( uint8_t y = 0; y < 3; y++ )
    {
        vxui_frame( &ctx, 1.0f / 60.0f );
        vxui_hud hud = {};
        vxui_hud_begin( &hud, &ctx, 960, 540 );
        vxui_hud_set_pos( &hud, 10, 20 );
        vxui_hud_text_box( &hud, "box", "alpha beta gamma", 48, 96, 0, y );
        vxui_draw_list dl = vxui_render( &ctx );

        ASSERT_EQ( dl.count, 3 );
        ASSERT_EQ( dl.cmds[0].id, vxui_text_id( "box" ) );
        ASSERT_EQ( hud.y, 116.0f );
        ASSERT_EQ( hud.item_count, 1 );
        for ( int i = 0; i < 3; i++ ) ASSERT_EQ( dl.cmds[i].userdata, (const void*) &hud.items[0] );
    }
}

UTEST(hud_align_y, empty_box_advance_is_independent_of_alignment) {
    vxui_ctx ctx = make_ctx();
    for ( uint8_t y = 0; y < 3; y++ )
    {
        vxui_frame( &ctx, 1.0f / 60.0f );
        vxui_hud hud = {};
        vxui_hud_begin( &hud, &ctx, 960, 540 );
        vxui_hud_text_box( &hud, "empty", "", 120, 96, 0, y );
        vxui_draw_list dl = vxui_render( &ctx );

        ASSERT_EQ( dl.count, 0 );
        ASSERT_EQ( hud.item_count, 1 );
        ASSERT_EQ( hud.y, 96.0f );
    }
}

UTEST(hud_align_y, existing_default_mapping_is_preserved_for_every_byte) {
    vxui_ctx ctx = make_ctx();
    for ( int y = 0; y < 256; y++ )
    {
        vxui_frame( &ctx, 1.0f / 60.0f );
        vxui_hud hud = {};
        vxui_hud_begin( &hud, &ctx, 960, 540 );
        vxui_hud_text_box( &hud, "box", "alpha", 120, 96, 0, (uint8_t) y );
        vxui_draw_list dl = vxui_render( &ctx );

        ASSERT_EQ( dl.count, 1 );
        ASSERT_EQ( dl.cmds[0].rect.y, y == 1 ? 36.0f : y == 2 ? 72.0f : 0.0f );
    }
}

UTEST_MAIN();
