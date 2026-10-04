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

UTEST(hud_stock, all_layouts_preserve_positions_ids_and_metadata) {
    glm::vec2 positions[6][8] = {
        { { 0, 0 }, { 23, 0 }, { 46, 0 }, { 69, 0 }, { 92, 0 }, { 115, 0 }, { 138, 0 }, { 161, 0 } },
        { { 0, 0 }, { 0, 15 }, { 0, 30 }, { 0, 45 }, { 0, 60 }, { 0, 75 }, { 0, 90 }, { 0, 105 } },
        { { 0, 0 }, { 23, 0 }, { 46, 0 }, { 69, 0 }, { 0, 15 }, { 23, 15 }, { 46, 15 }, { 69, 15 } },
        { { 0, 0 }, { 23, 0 }, { 0, 15 }, { 23, 15 }, { 0, 30 }, { 23, 30 }, { 0, 45 }, { 23, 45 } },
        { { 0, 0 }, { 23, 6 }, { 0, 15 }, { 23, 21 }, { 0, 30 }, { 23, 36 }, { 0, 45 }, { 23, 51 } },
        { { 0, 0 }, { 10, 15 }, { 23, 0 }, { 33, 15 }, { 46, 0 }, { 56, 15 }, { 69, 0 }, { 79, 15 } }
    };
    float heights[] = { 12, 117, 27, 57, 63, 27 };
    glm::vec4 uv = { 0.75f, 0.25f, 0.5f, 0.875f };
    for ( uint8_t layout = 0; layout < 6; layout++ )
    {
        vxui_ctx ctx = make_ctx();
        vxui_frame( &ctx, 1.0f / 60.0f );
        vxui_hud hud = {};
        vxui_hud_begin( &hud, &ctx, 960, 540 );
        vxui_hud_set_pos( &hud, 10, 20 );
        vxui_hud_set_colour( &hud, { 0.2f, 0.4f, 0.6f, 0.8f } );
        vxui_hud_resource_stock( &hud, "stock", 42, 8, 20, 12, layout, uv, 3 );
        vxui_draw_list dl = vxui_render( &ctx );

        ASSERT_EQ( dl.count, 8 );
        ASSERT_EQ( hud.item_count, 8 );
        ASSERT_EQ( hud.x, 10.0f );
        ASSERT_EQ( hud.y, 20.0f + heights[layout] );
        ASSERT_EQ( ctx.text_offset, 0 );
        for ( int i = 0; i < 8; i++ )
        {
            char id[32];
            snprintf( id, sizeof( id ), "stock.%d", i );
            const vxui_draw_cmd& c = dl.cmds[i];
            ASSERT_EQ( c.id, vxui_id( id ) );
            ASSERT_EQ( c.rect.x, 10.0f + positions[layout][i].x );
            ASSERT_EQ( c.rect.y, 20.0f + positions[layout][i].y );
            ASSERT_EQ( c.rect.z, 20.0f );
            ASSERT_EQ( c.rect.w, 12.0f );
            ASSERT_EQ( c.userdata, (const void*) &hud.items[i] );
            vxui_render_data out = {};
            ASSERT_TRUE( vxui_hud_resolve( &hud, &c, &out ) );
            ASSERT_EQ( out.texture_id, (uint32_t) 42 );
            for ( int j = 0; j < 4; j++ ) ASSERT_EQ( out.uv[j], uv[j] );
            ASSERT_EQ( out.colour.r, 0.2f );
            ASSERT_EQ( out.colour.a, 0.8f );
        }
    }
}

UTEST(hud_stock, partial_rows_keep_the_exact_cursor_extent) {
    float heights[6][9] = {
        { 12, 12, 12, 12, 12, 12, 12, 12, 12 },
        { 12, 27, 42, 57, 72, 87, 102, 117, 132 },
        { 12, 12, 12, 12, 27, 27, 27, 27, 42 },
        { 12, 12, 27, 27, 42, 42, 57, 57, 72 },
        { 12, 18, 27, 33, 42, 48, 57, 63, 72 },
        { 12, 27, 27, 27, 27, 27, 27, 27, 27 }
    };
    for ( uint8_t layout = 0; layout < 6; layout++ )
    {
        for ( int count = 1; count <= 9; count++ )
        {
            vxui_ctx ctx = make_ctx();
            vxui_frame( &ctx, 1.0f / 60.0f );
            vxui_hud hud = {};
            vxui_hud_begin( &hud, &ctx, 960, 540 );
            vxui_hud_set_pos( &hud, 10, 20 );
            vxui_hud_resource_stock( &hud, "stock", 42, count, 20, 12, layout, { 0, 0, 1, 1 }, 3 );
            vxui_draw_list dl = vxui_render( &ctx );

            ASSERT_EQ( dl.count, count );
            ASSERT_EQ( hud.item_count, count );
            ASSERT_EQ( hud.x, 10.0f );
            ASSERT_EQ( hud.y, 20.0f + heights[layout][count - 1] );
        }
    }
}

UTEST(hud_stock, zero_count_keeps_cursor_metadata_and_packet_count_unchanged) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_image( &hud, "existing", 42, 20, 12 );
    vxui_hud_set_pos( &hud, 10, 20 );
    for ( uint8_t layout = 0; layout < 6; layout++ )
        vxui_hud_resource_stock( &hud, "empty", 99, 0, 20, 12, layout, { 0, 0, 1, 1 }, 3 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 1 );
    ASSERT_EQ( hud.item_count, 1 );
    ASSERT_EQ( hud.x, 10.0f );
    ASSERT_EQ( hud.y, 20.0f );
    ASSERT_EQ( dl.cmds[0].id, vxui_id( "existing" ) );
}

UTEST(hud_stock, fractional_dimensions_and_gap_keep_fractional_positions) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_set_pos( &hud, 10.25f, 20.5f );
    vxui_hud_resource_stock( &hud, "stock", 42, 7, 20.5f, 12.5f, VXUI_HUD_STOCK_COL, { 0, 0, 1, 1 }, 1.25f );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 7 );
    ASSERT_EQ( hud.y, 115.5f );
    for ( int i = 0; i < 7; i++ )
    {
        ASSERT_EQ( dl.cmds[i].rect.x, 10.25f );
        ASSERT_EQ( dl.cmds[i].rect.y, 20.5f + (float) i * 13.75f );
        ASSERT_EQ( dl.cmds[i].rect.z, 20.5f );
        ASSERT_EQ( dl.cmds[i].rect.w, 12.5f );
    }
}

UTEST(hud_stock, descending_column_uses_maximum_extent_not_the_last_slot) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_set_pos( &hud, 10, 20 );
    vxui_hud_resource_stock( &hud, "stock", 42, 5, 20, 12, VXUI_HUD_STOCK_COL, { 0, 0, 1, 1 }, -15 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 5 );
    ASSERT_EQ( hud.y, 32.0f );
    for ( int i = 0; i < 5; i++ ) ASSERT_EQ( dl.cmds[i].rect.y, 20.0f - (float) i * 3.0f );
}

UTEST(hud_stock, overlapping_hex_column_uses_the_highest_extent) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_set_pos( &hud, 30, 20 );
    vxui_hud_resource_stock( &hud, "stock", 42, 3, 20, 12, VXUI_HUD_STOCK_HEX_COL, { 0, 0, 1, 1 }, -15 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 3 );
    ASSERT_EQ( dl.cmds[0].rect.y, 20.0f );
    ASSERT_EQ( dl.cmds[1].rect.y, 26.0f );
    ASSERT_EQ( dl.cmds[2].rect.y, 17.0f );
    ASSERT_EQ( hud.y, 38.0f );
}

UTEST(hud_stock, following_image_uses_the_final_stock_cursor) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_set_pos( &hud, 10, 20 );
    vxui_hud_set_colour( &hud, { 1, 0, 0, 1 } );
    vxui_hud_resource_stock( &hud, "stock", 42, 7, 20, 12, VXUI_HUD_STOCK_GRID4, { 0, 0, 1, 1 }, 3 );
    vxui_hud_set_colour( &hud, { 0, 1, 0, 1 } );
    vxui_hud_image( &hud, "after", 99, 10, 8 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 8 );
    ASSERT_EQ( hud.item_count, 8 );
    ASSERT_EQ( hud.y, 55.0f );
    const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "after" ) );
    ASSERT_TRUE( c );
    ASSERT_EQ( c->rect.x, 10.0f );
    ASSERT_EQ( c->rect.y, 47.0f );
    for ( int i = 0; i < 8; i++ )
    {
        vxui_render_data out = {};
        ASSERT_TRUE( vxui_hud_resolve( &hud, &dl.cmds[i], &out ) );
        ASSERT_EQ( out.texture_id, (uint32_t) ( i < 7 ? 42 : 99 ) );
        ASSERT_EQ( out.colour.r, i < 7 ? 1.0f : 0.0f );
        ASSERT_EQ( out.colour.g, i < 7 ? 0.0f : 1.0f );
    }
}

UTEST(hud_stock, full_item_capacity_keeps_slot_order_and_records) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_resource_stock( &hud, "stock", 42, VXUI_HUD_MAX_ITEMS, 3, 2, VXUI_HUD_STOCK_GRID4, { 0, 0, 1, 1 }, 1 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, VXUI_HUD_MAX_ITEMS );
    ASSERT_EQ( hud.item_count, VXUI_HUD_MAX_ITEMS );
    ASSERT_EQ( hud.y, 191.0f );
    for ( int i = 0; i < VXUI_HUD_MAX_ITEMS; i++ )
    {
        char id[32];
        snprintf( id, sizeof( id ), "stock.%d", i );
        ASSERT_EQ( dl.cmds[i].id, vxui_id( id ) );
        ASSERT_EQ( dl.cmds[i].userdata, (const void*) &hud.items[i] );
    }
}

UTEST(hud_stock, changing_count_reuses_only_current_frame_slots) {
    vxui_ctx ctx = make_ctx();
    vxui_hud hud = {};
    int counts[] = { 5, 0, 2 };
    for ( int frame = 0; frame < 3; frame++ )
    {
        vxui_frame( &ctx, 1.0f / 60.0f );
        vxui_hud_begin( &hud, &ctx, 960, 540 );
        vxui_hud_resource_stock( &hud, "stock", (uint32_t) frame + 42, counts[frame], 20, 12, VXUI_HUD_STOCK_ROW, { 0, 0, 1, 1 } );
        vxui_draw_list dl = vxui_render( &ctx );

        ASSERT_EQ( dl.count, counts[frame] );
        ASSERT_EQ( hud.item_count, counts[frame] );
        ASSERT_EQ( hud.y, counts[frame] == 0 ? 0.0f : 12.0f );
        for ( int i = 0; i < counts[frame]; i++ )
        {
            ASSERT_EQ( dl.cmds[i].userdata, (const void*) &hud.items[i] );
            vxui_render_data out = {};
            ASSERT_TRUE( vxui_hud_resolve( &hud, &dl.cmds[i], &out ) );
            ASSERT_EQ( out.texture_id, (uint32_t) frame + 42 );
        }
    }
}

UTEST_MAIN();
