#include "utest.h"
#include "../vxui.h"
#define VXUI_IMPL
#include "../vxui_impl.h"
#define VXUI_HUD_IMPL
#include "../vxui_hud.h"

static uint8_t s_clay_mem[16 * 1024 * 1024];

static Clay_Dimensions measure( Clay_StringSlice s, Clay_TextElementConfig* cfg, void* )
{
    return { (float) s.length * 8, (float) cfg->fontSize };
}

static vxui_ctx make_ctx()
{
    vxui_ctx ctx = {};
    vxui_init( &ctx, 640, 480, s_clay_mem, sizeof( s_clay_mem ) );
    Clay_SetMeasureTextFunction( measure, nullptr );
    return ctx;
}

UTEST(hud_health_composition, health_fill_matches_the_existing_resource_bar_geometry_and_uvs) {
    float hp[] = { -25, 0, 12.5f, 50, 100, 150 };
    for ( float value : hp )
    {
        vxui_ctx ctx = make_ctx();
        vxui_hud hud = {};
        vxui_frame( &ctx, 0.01f );
        vxui_hud_begin( &hud, &ctx, 640, 480 );
        vxui_hud_set_pos( &hud, 12.5f, 20.25f );
        glm::vec4 uv = { 0.9f, 0.8f, 0.1f, 0.2f };
        vxui_hud_health_bar( &hud, "health", 42, value, 100, 125.5f, 20.5f, uv, false );
        vxui_hud_set_pos( &hud, 12.5f, 20.25f );
        vxui_hud_resource_bar( &hud, "resource", 42, value / 100, 125.5f, 20.5f, uv );
        vxui_draw_list dl = vxui_render( &ctx );
        const vxui_draw_cmd* a = vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "health.fill" ) );
        const vxui_draw_cmd* b = vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "resource" ) );
        ASSERT_TRUE( a && b );
        for ( int i = 0; i < 4; i++ ) ASSERT_EQ( a->rect[i], b->rect[i] );
        vxui_render_data ra = {}, rb = {};
        ASSERT_TRUE( vxui_hud_resolve( &hud, a, &ra ) );
        ASSERT_TRUE( vxui_hud_resolve( &hud, b, &rb ) );
        for ( int i = 0; i < 4; i++ ) ASSERT_EQ( ra.uv[i], rb.uv[i] );
    }
}

UTEST(hud_health_composition, zero_and_negative_maximum_keep_the_existing_empty_fill) {
    for ( float maximum : { 0.0f, -100.0f } )
    {
        vxui_ctx ctx = make_ctx();
        vxui_hud hud = {};
        vxui_frame( &ctx, 0.01f );
        vxui_hud_begin( &hud, &ctx, 640, 480 );
        vxui_hud_health_bar( &hud, "health", 42, 50, maximum, 100, 20, { 0.2f, 0.3f, 0.8f, 0.9f }, false );
        vxui_draw_list dl = vxui_render( &ctx );
        ASSERT_EQ( dl.count, 1 );
        ASSERT_EQ( dl.cmds[0].rect.z, 0.0f );
        ASSERT_EQ( hud.items[0].uv.z, 0.2f );
        ASSERT_EQ( hud.y, 20.0f );
    }
}

UTEST(hud_health_composition, optional_text_stays_centered_and_formats_the_original_integer_hp) {
    vxui_ctx ctx = make_ctx();
    vxui_hud hud = {};
    vxui_frame( &ctx, 0.01f );
    vxui_hud_begin( &hud, &ctx, 640, 480 );
    vxui_hud_set_pos( &hud, 10, 20 );
    vxui_hud_set_font( &hud, 7, 16 );
    vxui_hud_health_bar( &hud, "health", 42, 142.75f, 100, 100, 40, { 0, 0, 1, 1 } );
    vxui_draw_list dl = vxui_render( &ctx );
    const vxui_draw_cmd* text = vxui_draw_find( dl, VXUI_DRAW_TEXT, vxui_text_id( "health.text" ) );
    ASSERT_TRUE( text );
    ASSERT_EQ( text->text_len, 3 );
    ASSERT_EQ( memcmp( text->text, "142", 3 ), 0 );
    ASSERT_EQ( text->rect.x, 48.0f );
    ASSERT_EQ( text->rect.y, 32.0f );
    ASSERT_EQ( text->font, (uint16_t) 7 );
    ASSERT_EQ( text->font_px, (uint16_t) 16 );
}

UTEST(hud_health_composition, both_optional_text_states_advance_once_and_preserve_x) {
    for ( bool text : { false, true } )
    {
        vxui_ctx ctx = make_ctx();
        vxui_hud hud = {};
        vxui_frame( &ctx, 0.01f );
        vxui_hud_begin( &hud, &ctx, 640, 480 );
        vxui_hud_set_pos( &hud, 12.5f, -8.25f );
        vxui_hud_health_bar( &hud, "health", 42, 25, 100, 100, 32.5f, { 0, 0, 1, 1 }, text );
        ASSERT_EQ( hud.x, 12.5f );
        ASSERT_EQ( hud.y, 24.25f );
        vxui_render( &ctx );
    }
}

UTEST(hud_health_composition, fill_and_text_receive_separate_current_metadata_attachments) {
    vxui_ctx ctx = make_ctx();
    vxui_hud hud = {};
    vxui_frame( &ctx, 0.01f );
    vxui_hud_begin( &hud, &ctx, 640, 480 );
    vxui_hud_set_colour( &hud, { 0.2f, 0.4f, 0.6f, 0.8f } );
    vxui_hud_health_bar( &hud, "health", 42, 25, 100, 100, 32, { 0, 0, 1, 1 } );
    vxui_draw_list dl = vxui_render( &ctx );
    ASSERT_EQ( hud.item_count, 2 );
    ASSERT_EQ( dl.count, 2 );
    ASSERT_EQ( dl.cmds[0].userdata, (const void*) &hud.items[0] );
    ASSERT_EQ( dl.cmds[1].userdata, (const void*) &hud.items[1] );
    ASSERT_EQ( hud.items[0].texture_id, 42u );
    ASSERT_EQ( hud.items[1].texture_id, 0u );
    for ( int i = 0; i < 2; i++ ) ASSERT_EQ( hud.items[i].colour.a, 0.8f );
}

UTEST(hud_health_composition, later_image_starts_at_the_health_bar_bottom_not_the_text_bottom) {
    vxui_ctx ctx = make_ctx();
    vxui_hud hud = {};
    vxui_frame( &ctx, 0.01f );
    vxui_hud_begin( &hud, &ctx, 640, 480 );
    vxui_hud_health_bar( &hud, "health", 42, 25, 100, 100, 40, { 0, 0, 1, 1 } );
    vxui_hud_image( &hud, "next", 43, 20, 10 );
    vxui_draw_list dl = vxui_render( &ctx );
    const vxui_draw_cmd* next = vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "next" ) );
    ASSERT_TRUE( next );
    ASSERT_EQ( next->rect.y, 40.0f );
    ASSERT_EQ( hud.y, 50.0f );
}

UTEST(hud_health_composition, declaration_z_and_generated_ids_remain_clay_owned) {
    vxui_ctx ctx = make_ctx();
    vxui_hud hud = {};
    vxui_frame( &ctx, 0.01f );
    vxui_hud_begin( &hud, &ctx, 640, 480 );
    vxui_hud_set_z( &hud, 12 );
    vxui_hud_health_bar( &hud, "front", 42, 25, 100, 100, 32, { 0, 0, 1, 1 }, false );
    vxui_hud_set_z( &hud, -12 );
    vxui_hud_health_bar( &hud, "back", 43, 50, 100, 100, 32, { 0, 0, 1, 1 }, false );
    vxui_draw_list dl = vxui_render( &ctx );
    ASSERT_EQ( dl.count, 2 );
    ASSERT_EQ( dl.cmds[0].id, vxui_id( "back.fill" ) );
    ASSERT_EQ( dl.cmds[1].id, vxui_id( "front.fill" ) );
}

UTEST(hud_health_composition, full_metadata_capacity_keeps_both_items_of_every_health_bar) {
    vxui_ctx ctx = make_ctx();
    vxui_hud hud = {};
    char names[VXUI_HUD_MAX_ITEMS / 2][16] = {};
    vxui_frame( &ctx, 0.01f );
    vxui_hud_begin( &hud, &ctx, 640, 480 );
    for ( int i = 0; i < VXUI_HUD_MAX_ITEMS / 2; i++ )
    {
        snprintf( names[i], sizeof( names[i] ), "health.%d", i );
        vxui_hud_health_bar( &hud, names[i], 42, (float) i, 128, 100, 1, { 0, 0, 1, 1 } );
    }
    vxui_draw_list dl = vxui_render( &ctx );
    ASSERT_EQ( hud.item_count, VXUI_HUD_MAX_ITEMS );
    ASSERT_EQ( dl.count, VXUI_HUD_MAX_ITEMS );
    ASSERT_EQ( hud.y, (float) VXUI_HUD_MAX_ITEMS / 2 );
    for ( int i = 0; i < dl.count; i++ )
    {
        vxui_render_data out = {};
        ASSERT_TRUE( vxui_hud_resolve( &hud, &dl.cmds[i], &out ) );
        ASSERT_EQ( out.texture_id, dl.cmds[i].type == VXUI_DRAW_RECT ? 42u : 0u );
    }
}

UTEST_MAIN();
