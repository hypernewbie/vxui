#include "utest.h"
#include "../vxui.h"
#define VXUI_IMPL
#include "../vxui_impl.h"
#define VXUI_HUD_IMPL
#include "../vxui_hud.h"

static uint8_t s_clay_mem[16 * 1024 * 1024];

static vxui_ctx make_ctx()
{
    vxui_ctx ctx = {};
    vxui_init( &ctx, 960, 540, s_clay_mem, sizeof( s_clay_mem ) );
    return ctx;
}

// ===== begin / state =====================================================

UTEST(hud, begin_resets_state) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );

    vxui_hud hud = {};
    hud.x       = 99.0f;
    hud.colour  = { 0.5f, 0.5f, 0.5f, 0.5f };
    hud.item_count = 42;

    vxui_hud_begin( &hud, &ctx, 960, 540 );

    ASSERT_EQ( hud.x, 0.0f );
    ASSERT_EQ( hud.y, 0.0f );
    ASSERT_EQ( hud.w, 960.0f );
    ASSERT_EQ( hud.h, 540.0f );
    ASSERT_EQ( hud.colour.r, 1.0f );
    ASSERT_EQ( hud.colour.g, 1.0f );
    ASSERT_EQ( hud.colour.b, 1.0f );
    ASSERT_EQ( hud.colour.a, 1.0f );
    ASSERT_EQ( hud.font, (uint16_t) 0 );
    ASSERT_EQ( hud.font_px, (uint16_t) VXUI_FONT_SIZE_DEFAULT );
    ASSERT_EQ( hud.z, 0 );
    ASSERT_EQ( hud.item_count, 0 );

    vxui_render( &ctx );
}

UTEST(hud, set_pos_updates_cursor) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );

    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_set_pos( &hud, 80, 40 );
    ASSERT_EQ( hud.x, 80.0f );
    ASSERT_EQ( hud.y, 40.0f );

    vxui_render( &ctx );
}

UTEST(hud, move_pos_adds_to_cursor) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );

    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_set_pos( &hud, 80, 40 );
    vxui_hud_move_pos( &hud, 20, -10 );
    ASSERT_EQ( hud.x, 100.0f );
    ASSERT_EQ( hud.y, 30.0f );

    vxui_render( &ctx );
}

UTEST(hud, setters_update_current_state) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );

    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );

    vxui_hud_set_colour( &hud, { 0.2f, 0.4f, 0.6f, 0.8f } );
    ASSERT_EQ( hud.colour.r, 0.2f );
    ASSERT_EQ( hud.colour.a, 0.8f );

    vxui_hud_set_font( &hud, 5, 24 );
    ASSERT_EQ( hud.font, (vxui_font_id) 5 );
    ASSERT_EQ( hud.font_px, (uint16_t) 24 );

    vxui_hud_set_z( &hud, 7 );
    ASSERT_EQ( hud.z, 7 );

    vxui_render( &ctx );
}

// ===== tile helpers =======================================================

UTEST(hud, tile_returns_expected_uvs) {
    glm::vec4 uv = vxui_hud_tile( 1024, 128, 32, 32, 2 );
    ASSERT_EQ( uv.x, 64.0f / 1024.0f );
    ASSERT_EQ( uv.y, 0.0f );
    ASSERT_EQ( uv.z, 96.0f / 1024.0f );
    ASSERT_EQ( uv.w, 32.0f / 128.0f );

    // tile 32 at (1,0) in a 1024x128 texture with 32x32 tiles
    // tilesX = 1024/32 = 32, tilesY = 128/32 = 4
    // tileX = 2 % 32 = 2, tileY = 2 / 32 = 0
    // u0 = 64/1024, v0 = 0, u1 = 96/1024, v1 = 32/128
    ASSERT_EQ( uv.z, uv.x + 32.0f / 1024.0f );
    ASSERT_EQ( uv.w, uv.y + 32.0f / 128.0f );
}

UTEST(hud, tile1d_returns_expected_uvs) {
    glm::vec4 uv = vxui_hud_tile1d( 1024, 128, 5 );
    // 1D tile: square texture 1024x1024, tile 128x128
    // tilesX = 1024/128 = 8, tileX = 5 % 8 = 5, tileY = 5 / 8 = 0
    ASSERT_EQ( uv.x, (float)( 5 * 128 ) / 1024.0f );
    ASSERT_EQ( uv.y, 0.0f );
    ASSERT_EQ( uv.z, (float)( 6 * 128 ) / 1024.0f );
    ASSERT_EQ( uv.w, 128.0f / 1024.0f );
}

UTEST(hud, id_matches_vxui_id) {
    ASSERT_EQ( vxui_hud_id( "test" ), vxui_id( "test" ) );
}

// ===== meter ==============================================================

UTEST(hud, meter_emits_track_and_fill) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );

    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_set_pos( &hud, 10, 20 );
    vxui_hud_meter( &hud, "hp", 0, 0.5f, 200, 40 );

    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* track = vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "hp" ) );
    ASSERT_NE( track, nullptr );
    ASSERT_EQ( track->rect.x, 10.0f );
    ASSERT_EQ( track->rect.y, 20.0f );
    ASSERT_EQ( track->rect.z, 200.0f );
    ASSERT_EQ( track->rect.w, 40.0f );

    const vxui_draw_cmd* fill = vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "hp.fill" ) );
    ASSERT_NE( fill, nullptr );
    ASSERT_EQ( fill->rect.z, 100.0f );  // 200 * 0.5
    ASSERT_EQ( fill->rect.w, 40.0f );
}

UTEST(hud, meter_advances_cursor) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );

    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_set_pos( &hud, 10, 20 );
    vxui_hud_meter( &hud, "a", 0, 1.0f, 200, 40 );

    ASSERT_EQ( hud.y, 60.0f );  // 20 + 40
    vxui_render( &ctx );
}

UTEST(hud, meter_clamps_t) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );

    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );

    vxui_hud_meter( &hud, "lo", 0, -0.5f, 200, 40 );
    vxui_hud_meter( &hud, "hi", 0, 2.0f, 200, 40 );
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* lo = vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "lo.fill" ) );
    ASSERT_NE( lo, nullptr );
    ASSERT_EQ( lo->rect.z, 0.0f );

    const vxui_draw_cmd* hi = vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "hi.fill" ) );
    ASSERT_NE( hi, nullptr );
    ASSERT_EQ( hi->rect.z, 200.0f );
}

UTEST(hud, meter_reverse_fills_from_right) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );

    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_meter( &hud, "r", 0, 0.5f, 200, 40, false, true );

    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* track = vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "r" ) );
    ASSERT_NE( track, nullptr );
    const vxui_draw_cmd* fill  = vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "r.fill" ) );
    ASSERT_NE( fill, nullptr );
    ASSERT_EQ( fill->rect.x, track->rect.x + 100.0f );  // offset by w - w*t = 100
    ASSERT_EQ( fill->rect.z, 100.0f );
}

UTEST(hud, meter_vertical_reverse_fills_from_bottom) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );

    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_meter( &hud, "v", 0, 0.5f, 40, 200, true, true );

    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* track = vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "v" ) );
    ASSERT_NE( track, nullptr );
    const vxui_draw_cmd* fill  = vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "v.fill" ) );
    ASSERT_NE( fill, nullptr );
    ASSERT_EQ( fill->rect.y, track->rect.y + 100.0f );  // offset by h - h*t = 100
    ASSERT_EQ( fill->rect.w, 100.0f );
}

// ===== stock ==============================================================

UTEST(hud, stock_row_emits_count_slots) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );

    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_set_pos( &hud, 10, 20 );
    vxui_hud_resource_stock( &hud, "life", 1, 3, 32, 32, VXUI_HUD_STOCK_ROW, { 0, 0, 1, 1 } );

    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_NE( vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "life.0" ) ), nullptr );
    ASSERT_NE( vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "life.1" ) ), nullptr );
    ASSERT_NE( vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "life.2" ) ), nullptr );
    ASSERT_EQ( vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "life.3" ) ), nullptr );
}

UTEST(hud, stock_row_positions) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );

    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_set_pos( &hud, 10, 20 );
    vxui_hud_resource_stock( &hud, "s", 1, 3, 32, 32, VXUI_HUD_STOCK_ROW, { 0, 0, 1, 1 } );

    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* s0 = vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "s.0" ) );
    const vxui_draw_cmd* s1 = vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "s.1" ) );
    const vxui_draw_cmd* s2 = vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "s.2" ) );

    ASSERT_EQ( s0->rect.x, 10.0f );
    ASSERT_EQ( s1->rect.x, 42.0f );  // 10 + 32 + 0
    ASSERT_EQ( s2->rect.x, 74.0f );
}

UTEST(hud, stock_advances_cursor) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );

    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_set_pos( &hud, 10, 20 );
    vxui_hud_resource_stock( &hud, "s", 1, 3, 32, 32, VXUI_HUD_STOCK_COL, { 0, 0, 1, 1 } );

    ASSERT_EQ( hud.y, 20.0f + 3 * 32.0f );  // 3 slots vertical
    vxui_render( &ctx );
}

UTEST(hud, stock_hex_col_offsets) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );

    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_set_pos( &hud, 0, 0 );
    vxui_hud_resource_stock( &hud, "h", 1, 4, 32, 32, VXUI_HUD_STOCK_HEX_COL, { 0, 0, 1, 1 } );

    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* h0 = vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "h.0" ) );
    const vxui_draw_cmd* h1 = vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "h.1" ) );
    const vxui_draw_cmd* h2 = vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "h.2" ) );
    const vxui_draw_cmd* h3 = vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "h.3" ) );

    ASSERT_EQ( h0->rect.x, 0.0f );
    ASSERT_EQ( h0->rect.y, 0.0f );
    ASSERT_EQ( h1->rect.x, 32.0f );
    ASSERT_EQ( h1->rect.y, 16.0f );  // 0 + 0 + 32*0.5
    ASSERT_EQ( h2->rect.x, 0.0f );
    ASSERT_EQ( h2->rect.y, 32.0f );
    ASSERT_EQ( h3->rect.x, 32.0f );
    ASSERT_EQ( h3->rect.y, 48.0f );
}

UTEST(hud, stock_hex_row_offsets) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );

    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_set_pos( &hud, 0, 0 );
    vxui_hud_resource_stock( &hud, "h", 1, 4, 32, 32, VXUI_HUD_STOCK_HEX_ROW, { 0, 0, 1, 1 } );

    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* h0 = vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "h.0" ) );
    const vxui_draw_cmd* h1 = vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "h.1" ) );
    const vxui_draw_cmd* h2 = vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "h.2" ) );
    const vxui_draw_cmd* h3 = vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "h.3" ) );

    ASSERT_EQ( h0->rect.x, 0.0f );
    ASSERT_EQ( h0->rect.y, 0.0f );
    ASSERT_EQ( h1->rect.x, 16.0f );  // 0 + 0 + 32*0.5
    ASSERT_EQ( h1->rect.y, 32.0f );
    ASSERT_EQ( h2->rect.x, 32.0f );
    ASSERT_EQ( h2->rect.y, 0.0f );
    ASSERT_EQ( h3->rect.x, 48.0f );
    ASSERT_EQ( h3->rect.y, 32.0f );
}

UTEST(hud, stock_zero_count_emits_nothing) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );

    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_resource_stock( &hud, "z", 1, 0, 32, 32, VXUI_HUD_STOCK_ROW, { 0, 0, 1, 1 } );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 0 );
}

// ===== resolve ============================================================

UTEST(hud, resolve_sets_texture_uv_colour) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );

    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_set_pos( &hud, 10, 20 );
    vxui_hud_set_colour( &hud, { 0.1f, 0.2f, 0.3f, 1.0f } );
    vxui_hud_resource_stock( &hud, "s", 42, 1, 32, 32, VXUI_HUD_STOCK_ROW, { 0.25f, 0.5f, 0.75f, 1.0f } );
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "s.0" ) );
    ASSERT_NE( c, nullptr );

    vxui_render_data out = {};
    bool resolved = vxui_hud_resolve( &hud, c, &out );
    ASSERT_EQ( resolved, true );
    ASSERT_EQ( out.texture_id, (uint32_t) 42 );
    ASSERT_EQ( out.uv.x, 0.25f );
    ASSERT_EQ( out.uv.y, 0.5f );
    ASSERT_EQ( out.uv.z, 0.75f );
    ASSERT_EQ( out.uv.w, 1.0f );
    ASSERT_EQ( out.colour.r, 0.1f );
}

UTEST(hud, resolve_returns_false_for_unknown_id) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );

    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_draw_list dl = vxui_render( &ctx );

    vxui_draw_cmd fake = {};
    fake.id = vxui_id( "nonexistent" );
    vxui_render_data out = {};
    bool resolved = vxui_hud_resolve( &hud, &fake, &out );
    ASSERT_EQ( resolved, false );
}

UTEST(hud, meter_records_texture_for_both_rects) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );

    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_meter( &hud, "bar", 7, 0.5f, 200, 40, false, false, { 0.1f, 0.2f, 0.9f, 0.8f } );
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* track = vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "bar" ) );
    const vxui_draw_cmd* fill  = vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "bar.fill" ) );

    vxui_render_data out = {};
    ASSERT_EQ( vxui_hud_resolve( &hud, track, &out ), true );
    ASSERT_EQ( out.texture_id, (uint32_t) 7 );
    ASSERT_EQ( out.uv.x, 0.1f );

    out = {};
    ASSERT_EQ( vxui_hud_resolve( &hud, fill, &out ), true );
    ASSERT_EQ( out.texture_id, (uint32_t) 7 );
    ASSERT_EQ( out.uv.w, 0.8f );
}

// ===== image =============================================================

UTEST(hud, image_emits_rect_at_cursor) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );

    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_set_pos( &hud, 100, 200 );
    vxui_hud_image( &hud, "icon", 5, 64, 64 );

    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "icon" ) );
    ASSERT_NE( c, nullptr );
    ASSERT_EQ( c->rect.x, 100.0f );
    ASSERT_EQ( c->rect.y, 200.0f );
    ASSERT_EQ( c->rect.z, 64.0f );
    ASSERT_EQ( c->rect.w, 64.0f );
}

UTEST(hud, image_advances_y_by_height) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );

    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_set_pos( &hud, 0, 10 );
    vxui_hud_image( &hud, "a", 0, 32, 20 );

    ASSERT_EQ( hud.y, 30.0f );
    vxui_render( &ctx );
}

UTEST(hud, image_resolve_sets_metadata) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );

    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_set_colour( &hud, { 0.2f, 0.4f, 0.6f, 1.0f } );
    vxui_hud_image( &hud, "tex", 99, 32, 32, { 0.0f, 0.5f, 0.5f, 1.0f } );
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "tex" ) );
    ASSERT_NE( c, nullptr );

    vxui_render_data out = {};
    ASSERT_EQ( vxui_hud_resolve( &hud, c, &out ), true );
    ASSERT_EQ( out.texture_id, (uint32_t) 99 );
    ASSERT_EQ( out.uv.y, 0.5f );
    ASSERT_EQ( out.colour.r, 0.2f );
}

// ===== wallpaper =========================================================

UTEST(hud, wallpaper_emits_full_hud_rect) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );

    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_wallpaper( &hud, "bg", 3, { 0.0f, 0.0f, 1.0f, 1.0f } );
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "bg" ) );
    ASSERT_NE( c, nullptr );
    ASSERT_EQ( c->rect.x, 0.0f );
    ASSERT_EQ( c->rect.y, 0.0f );
    ASSERT_EQ( c->rect.z, 960.0f );
    ASSERT_EQ( c->rect.w, 540.0f );
}

UTEST(hud, wallpaper_does_not_advance_cursor) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );

    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_set_pos( &hud, 80, 40 );
    vxui_hud_wallpaper( &hud, "bg", 0 );
    ASSERT_EQ( hud.x, 80.0f );
    ASSERT_EQ( hud.y, 40.0f );

    vxui_render( &ctx );
}

UTEST(hud, wallpaper_resolve_sets_texture) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );

    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_wallpaper( &hud, "wp", 42, { 0.25f, 0.5f, 0.75f, 1.0f } );
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "wp" ) );
    ASSERT_NE( c, nullptr );

    vxui_render_data out = {};
    ASSERT_EQ( vxui_hud_resolve( &hud, c, &out ), true );
    ASSERT_EQ( out.texture_id, (uint32_t) 42 );
    ASSERT_EQ( out.uv.x, 0.25f );
    ASSERT_EQ( out.uv.z, 0.75f );
}

// ===== text ==============================================================

UTEST(hud, text_emits_text_cmd) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );

    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_text( &hud, "label", "HELLO" );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), 1 );
    const vxui_draw_cmd* t = vxui_draw_nth( dl, VXUI_DRAW_TEXT, 0 );
    ASSERT_NE( t, nullptr );
    ASSERT_EQ( t->text_len, 5 );
    ASSERT_EQ( strncmp( t->text, "HELLO", 5 ), 0 );
}

UTEST(hud, text_advances_y_by_font_px) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );

    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_set_pos( &hud, 0, 100 );
    vxui_hud_set_font( &hud, 0, 20 );
    vxui_hud_text( &hud, "a", "x" );

    ASSERT_EQ( hud.y, 120.0f );
    vxui_render( &ctx );
}

UTEST(hud, two_text_calls_stack_vertically) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );

    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_set_pos( &hud, 50, 50 );
    vxui_hud_set_font( &hud, 0, 16 );
    vxui_hud_text( &hud, "first",  "1" );
    vxui_hud_text( &hud, "second", "2" );

    ASSERT_EQ( hud.y, 50.0f + 32.0f );
    vxui_render( &ctx );
}

UTEST(hud, text_resolve_sets_colour) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );

    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_set_colour( &hud, { 0.3f, 0.6f, 0.9f, 1.0f } );
    vxui_hud_text( &hud, "msg", "TEST" );
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* t = vxui_draw_nth( dl, VXUI_DRAW_TEXT, 0 );
    ASSERT_NE( t, nullptr );

    vxui_render_data out = {};
    ASSERT_EQ( vxui_hud_resolve( &hud, t, &out ), true );
    ASSERT_EQ( out.colour.r, 0.3f );
    ASSERT_EQ( out.colour.g, 0.6f );
    ASSERT_EQ( out.colour.b, 0.9f );
}

// ===== text_box ==========================================================

UTEST(hud, text_box_emits_text_cmd) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );

    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_text_box( &hud, "box", "X", 200, 40 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), 1 );
}

UTEST(hud, text_box_advances_by_box_height) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );

    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_set_pos( &hud, 0, 100 );
    vxui_hud_text_box( &hud, "box", "X", 200, 60 );

    ASSERT_EQ( hud.y, 160.0f );
    vxui_render( &ctx );
}

UTEST(hud, text_box_centered_inside_rect) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );

    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_set_pos( &hud, 100, 200 );
    vxui_hud_text_box( &hud, "box", "X", 200, 40 );
    vxui_draw_list dl = vxui_render( &ctx );

    // With no font loaded, text measures to 0x0. Center align puts the text
    // command at the geometric center of the (w, h) box.
    const vxui_draw_cmd* t = vxui_draw_nth( dl, VXUI_DRAW_TEXT, 0 );
    ASSERT_NE( t, nullptr );
    ASSERT_EQ( t->rect.x, 200.0f );  // 100 + 200/2
    ASSERT_EQ( t->rect.y, 220.0f );  // 200 + 40/2
}

UTEST(hud, text_box_left_align) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );

    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_set_pos( &hud, 100, 200 );
    vxui_hud_text_box( &hud, "box", "X", 200, 40, 0, 0 );
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* t = vxui_draw_nth( dl, VXUI_DRAW_TEXT, 0 );
    ASSERT_NE( t, nullptr );
    ASSERT_EQ( t->rect.x, 100.0f );
    ASSERT_EQ( t->rect.y, 200.0f );
}

UTEST(hud, text_box_resolve_sets_colour) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );

    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_set_colour( &hud, { 0.7f, 0.2f, 0.4f, 1.0f } );
    vxui_hud_text_box( &hud, "box", "X", 200, 40 );
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* t = vxui_draw_nth( dl, VXUI_DRAW_TEXT, 0 );
    ASSERT_NE( t, nullptr );

    vxui_render_data out = {};
    ASSERT_EQ( vxui_hud_resolve( &hud, t, &out ), true );
    ASSERT_EQ( out.colour.r, 0.7f );
    ASSERT_EQ( out.colour.g, 0.2f );
}

UTEST_MAIN();
