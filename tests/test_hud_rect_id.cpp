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

UTEST(hud_rect_id, image_ids_match_the_core_for_varied_names) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    const char* names[] = { "image", "Image", "hud.image", "image##one", "two words", "", "caf\xc3\xa9", "a_long_immediate_mode_identifier_with_a_stable_suffix" };
    int n = (int) ( sizeof( names ) / sizeof( names[0] ) );
    for ( int i = 0; i < n; i++ ) vxui_hud_image( &hud, names[i], 42, 32, 16 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, n );
    ASSERT_EQ( hud.item_count, n );
    for ( int i = 0; i < n; i++ )
    {
        const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( names[i] ) );
        ASSERT_TRUE( c );
        ASSERT_EQ( c->userdata, (const void*) &hud.items[i] );
        ASSERT_EQ( c->rect.z, 32.0f );
        ASSERT_EQ( c->rect.w, 16.0f );
    }
}

UTEST(hud_rect_id, identity_does_not_depend_on_caller_storage_after_declaration) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    char id[] = "image";
    vxui_hud_image( &hud, id, 42, 32, 16 );
    memset( id, 'x', 5 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 1 );
    ASSERT_EQ( dl.cmds[0].id, vxui_id( "image" ) );
    ASSERT_NE( dl.cmds[0].id, vxui_id( "xxxxx" ) );
    ASSERT_EQ( dl.cmds[0].userdata, (const void*) &hud.items[0] );
}

UTEST(hud_rect_id, wallpaper_keeps_root_geometry_and_core_id) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_set_pos( &hud, 80, 40 );
    vxui_hud_wallpaper( &hud, "wallpaper", 42 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 1 );
    ASSERT_EQ( dl.cmds[0].id, vxui_id( "wallpaper" ) );
    ASSERT_EQ( dl.cmds[0].rect.x, 0.0f );
    ASSERT_EQ( dl.cmds[0].rect.y, 0.0f );
    ASSERT_EQ( dl.cmds[0].rect.z, 960.0f );
    ASSERT_EQ( dl.cmds[0].rect.w, 540.0f );
    ASSERT_EQ( hud.x, 80.0f );
    ASSERT_EQ( hud.y, 40.0f );
}

UTEST(hud_rect_id, meter_track_and_generated_fill_keep_distinct_core_ids) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_meter( &hud, "meter", 42, 0.25f, 160, 20 );
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* track = vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "meter" ) );
    const vxui_draw_cmd* fill = vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "meter.fill" ) );
    ASSERT_TRUE( track && fill );
    ASSERT_EQ( dl.count, 2 );
    ASSERT_NE( track->id, fill->id );
    ASSERT_EQ( track->userdata, (const void*) &hud.items[0] );
    ASSERT_EQ( fill->userdata, (const void*) &hud.items[1] );
    ASSERT_EQ( track->rect.z, 160.0f );
    ASSERT_EQ( fill->rect.z, 40.0f );
}

UTEST(hud_rect_id, resource_bar_and_health_fill_keep_their_core_ids) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_resource_bar( &hud, "resource", 42, 0.25f, 160, 20, { 0, 0, 1, 1 } );
    vxui_hud_health_bar( &hud, "health", 42, 25, 100, 160, 20, { 0, 0, 1, 1 }, false );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 2 );
    ASSERT_EQ( dl.cmds[0].id, vxui_id( "resource" ) );
    ASSERT_EQ( dl.cmds[1].id, vxui_id( "health.fill" ) );
    for ( int i = 0; i < 2; i++ )
    {
        ASSERT_EQ( dl.cmds[i].rect.z, 40.0f );
        ASSERT_EQ( dl.cmds[i].userdata, (const void*) &hud.items[i] );
        vxui_render_data out = {};
        ASSERT_TRUE( vxui_hud_resolve( &hud, &dl.cmds[i], &out ) );
        ASSERT_EQ( out.texture_id, (uint32_t) 42 );
        ASSERT_EQ( out.uv.z, 0.25f );
    }
}

UTEST(hud_rect_id, every_stock_layout_keeps_generated_slot_ids) {
    for ( uint8_t layout = 0; layout < 6; layout++ )
    {
        vxui_ctx ctx = make_ctx();
        vxui_frame( &ctx, 1.0f / 60.0f );
        vxui_hud hud = {};
        vxui_hud_begin( &hud, &ctx, 960, 540 );
        vxui_hud_resource_stock( &hud, "stock", 42, 8, 16, 12, layout, { 0, 0, 1, 1 }, 2 );
        vxui_draw_list dl = vxui_render( &ctx );

        ASSERT_EQ( dl.count, 8 );
        ASSERT_EQ( hud.item_count, 8 );
        for ( int i = 0; i < 8; i++ )
        {
            char id[32];
            snprintf( id, sizeof( id ), "stock.%d", i );
            ASSERT_EQ( dl.cmds[i].id, vxui_id( id ) );
            ASSERT_EQ( dl.cmds[i].userdata, (const void*) &hud.items[i] );
        }
    }
}

UTEST(hud_rect_id, changing_visual_metadata_does_not_change_identity) {
    vxui_ctx ctx = make_ctx();
    vxui_hud hud = {};
    for ( int frame = 0; frame < 2; frame++ )
    {
        vxui_frame( &ctx, 1.0f / 60.0f );
        vxui_hud_begin( &hud, &ctx, 960, 540 );
        vxui_hud_set_pos( &hud, (float) frame * 20, (float) frame * 10 );
        vxui_hud_set_colour( &hud, { (float) frame, 0, 1, 1 } );
        vxui_hud_set_z( &hud, frame == 0 ? -3 : 3 );
        vxui_hud_image( &hud, "image", (uint32_t) frame + 42, 32, 16, { 0, 0, 0.5f, 1 } );
        vxui_draw_list dl = vxui_render( &ctx );

        ASSERT_EQ( dl.count, 1 );
        ASSERT_EQ( dl.cmds[0].id, vxui_id( "image" ) );
        ASSERT_EQ( dl.cmds[0].rect.x, (float) frame * 20 );
        ASSERT_EQ( dl.cmds[0].rect.y, (float) frame * 10 );
        vxui_render_data out = {};
        ASSERT_TRUE( vxui_hud_resolve( &hud, &dl.cmds[0], &out ) );
        ASSERT_EQ( out.texture_id, (uint32_t) frame + 42 );
        ASSERT_EQ( out.colour.r, (float) frame );
    }
}

UTEST(hud_rect_id, changed_declaration_order_keeps_ids_with_their_records) {
    vxui_ctx ctx = make_ctx();
    vxui_hud hud = {};
    const char* names[] = { "first", "second" };
    for ( int frame = 0; frame < 2; frame++ )
    {
        vxui_frame( &ctx, 1.0f / 60.0f );
        vxui_hud_begin( &hud, &ctx, 960, 540 );
        for ( int i = 0; i < 2; i++ )
        {
            int logical = frame == 0 ? i : 1 - i;
            vxui_hud_image( &hud, names[logical], (uint32_t) logical + 42, 32, 16 );
        }
        vxui_draw_list dl = vxui_render( &ctx );

        ASSERT_EQ( dl.count, 2 );
        for ( int i = 0; i < 2; i++ )
        {
            int logical = frame == 0 ? i : 1 - i;
            ASSERT_EQ( dl.cmds[i].id, vxui_id( names[logical] ) );
            ASSERT_EQ( dl.cmds[i].userdata, (const void*) &hud.items[i] );
            vxui_render_data out = {};
            ASSERT_TRUE( vxui_hud_resolve( &hud, &dl.cmds[i], &out ) );
            ASSERT_EQ( out.texture_id, (uint32_t) logical + 42 );
        }
    }
}

UTEST_MAIN();
