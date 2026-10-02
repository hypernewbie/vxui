#include "utest.h"
#include "../vxui.h"
#define VXUI_IMPL
#include "../vxui_impl.h"
#define VXUI_HUD_IMPL
#include "../vxui_hud.h"

static uint8_t s_clay_mem[16 * 1024 * 1024];
static uint8_t s_clay_mem2[16 * 1024 * 1024];

static Clay_Dimensions measure_text( Clay_StringSlice s, Clay_TextElementConfig* cfg, void* )
{
    return { (float) s.length * 8.0f, (float) cfg->fontSize };
}

static vxui_ctx make_ctx( void* memory = s_clay_mem, float h = 540 )
{
    vxui_ctx ctx = {};
    vxui_init( &ctx, 960, h, memory, sizeof( s_clay_mem ) );
    Clay_SetMeasureTextFunction( measure_text, nullptr );
    return ctx;
}

static void resolve_hud( const vxui_draw_cmd* cmd, vxui_render_data* out, void* userdata )
{
    vxui_hud* hud = (vxui_hud*) userdata;
    if ( !vxui_hud_resolve( hud, cmd, out ) ) out->colour = { -1, -1, -1, -1 };
}

UTEST(hud_text_box, every_newline_resolves) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_set_colour( &hud, { 0.2f, 0.4f, 0.6f, 0.8f } );
    vxui_hud_text_box( &hud, "box", "alpha\nbeta\ngamma", 200, 100 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), 3 );
    ASSERT_EQ( hud.item_count, 1 );
    for ( int i = 0; i < 3; i++ )
    {
        vxui_render_data out = {};
        ASSERT_TRUE( vxui_hud_resolve( &hud, vxui_draw_nth( dl, VXUI_DRAW_TEXT, i ), &out ) );
        ASSERT_EQ( out.colour.r, 0.2f );
        ASSERT_EQ( out.colour.g, 0.4f );
        ASSERT_EQ( out.colour.b, 0.6f );
        ASSERT_EQ( out.colour.a, 0.8f );
        ASSERT_EQ( out.texture_id, (uint32_t) 0 );
        ASSERT_EQ( out.uv.x, 0.0f );
        ASSERT_EQ( out.uv.z, 1.0f );
    }
}

UTEST(hud_text_box, wrapped_lines_resolve_with_their_source_base) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_set_colour( &hud, { 1, 0, 0, 1 } );
    vxui_hud_text_box( &hud, "box", "alpha beta gamma", 48, 96, 0, 0 );
    vxui_hud_set_colour( &hud, { 0, 1, 0, 1 } );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), 3 );
    ASSERT_EQ( hud.items[0].text_base, ctx.text_buf );
    const char* words[] = { "alpha", "beta", "gamma" };
    int offsets[] = { 0, 6, 11 };
    for ( int i = 0; i < 3; i++ )
    {
        const vxui_draw_cmd* c = vxui_draw_nth( dl, VXUI_DRAW_TEXT, i );
        ASSERT_EQ( c->text_base, ctx.text_buf );
        ASSERT_EQ( c->text, ctx.text_buf + offsets[i] );
        ASSERT_EQ( c->text_len, (int) strlen( words[i] ) );
        ASSERT_EQ( strncmp( c->text, words[i], c->text_len ), 0 );
        vxui_render_data out = {};
        ASSERT_TRUE( vxui_hud_resolve( &hud, c, &out ) );
        ASSERT_EQ( out.colour.r, 1.0f );
        ASSERT_EQ( out.colour.g, 0.0f );
    }
}

UTEST(hud_text_box, newlines_and_wrapping_share_one_record) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_text_box( &hud, "box", "alpha beta\ngamma delta", 48, 120, 0, 0 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), 4 );
    ASSERT_EQ( hud.item_count, 1 );
    const char* words[] = { "alpha", "beta", "gamma", "delta" };
    for ( int i = 0; i < 4; i++ )
    {
        const vxui_draw_cmd* c = vxui_draw_nth( dl, VXUI_DRAW_TEXT, i );
        ASSERT_EQ( c->text_len, (int) strlen( words[i] ) );
        ASSERT_EQ( strncmp( c->text, words[i], c->text_len ), 0 );
        vxui_render_data out = {};
        ASSERT_TRUE( vxui_hud_resolve( &hud, c, &out ) );
    }
}

UTEST(hud_text_box, blank_lines_do_not_break_source_matching) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_text_box( &hud, "box", "\nalpha\n\nbeta\n", 200, 180, 0, 0 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), 2 );
    for ( int i = 0; i < 2; i++ )
    {
        const vxui_draw_cmd* c = vxui_draw_nth( dl, VXUI_DRAW_TEXT, i );
        ASSERT_NE( c->id, vxui_text_id( "box" ) );
        vxui_render_data out = {};
        ASSERT_TRUE( vxui_hud_resolve( &hud, c, &out ) );
    }
}

UTEST(hud_text_box, all_alignments_resolve_wrapped_lines) {
    vxui_ctx ctx = make_ctx();
    vxui_hud hud = {};
    for ( uint8_t x = 0; x < 3; x++ )
    {
        for ( uint8_t y = 0; y < 3; y++ )
        {
            vxui_frame( &ctx, 1.0f / 60.0f );
            vxui_hud_begin( &hud, &ctx, 960, 540 );
            vxui_hud_text_box( &hud, "box", "alpha beta gamma", 48, 96, x, y );
            vxui_draw_list dl = vxui_render( &ctx );

            ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), 3 );
            for ( int i = 0; i < 3; i++ )
            {
                vxui_render_data out = {};
                ASSERT_TRUE( vxui_hud_resolve( &hud, vxui_draw_nth( dl, VXUI_DRAW_TEXT, i ), &out ) );
            }
        }
    }
}

UTEST(hud_text_box, identical_boxes_keep_separate_colours) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_set_colour( &hud, { 1, 0, 0, 1 } );
    vxui_hud_text_box( &hud, "first", "alpha\nbeta", 200, 100 );
    vxui_hud_set_colour( &hud, { 0, 1, 0, 1 } );
    vxui_hud_text_box( &hud, "second", "alpha\nbeta", 200, 100 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), 4 );
    ASSERT_NE( hud.items[0].text_base, hud.items[1].text_base );
    for ( int i = 0; i < 4; i++ )
    {
        vxui_render_data out = {};
        ASSERT_TRUE( vxui_hud_resolve( &hud, vxui_draw_nth( dl, VXUI_DRAW_TEXT, i ), &out ) );
        ASSERT_EQ( out.colour.r, i < 2 ? 1.0f : 0.0f );
        ASSERT_EQ( out.colour.g, i < 2 ? 0.0f : 1.0f );
    }
}

UTEST(hud_text_box, mixed_plain_and_boxed_text_keep_separate_colours) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_set_colour( &hud, { 1, 0, 0, 1 } );
    vxui_hud_text( &hud, "plain", "alpha\nbeta" );
    vxui_hud_set_colour( &hud, { 0, 1, 0, 1 } );
    vxui_hud_text_box( &hud, "box", "alpha\nbeta", 200, 100 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), 4 );
    for ( int i = 0; i < 4; i++ )
    {
        vxui_render_data out = {};
        ASSERT_TRUE( vxui_hud_resolve( &hud, vxui_draw_nth( dl, VXUI_DRAW_TEXT, i ), &out ) );
        ASSERT_EQ( out.colour.r, i < 2 ? 1.0f : 0.0f );
        ASSERT_EQ( out.colour.g, i < 2 ? 0.0f : 1.0f );
    }
}

UTEST(hud_text_box, identical_ids_in_separate_contexts_do_not_match) {
    vxui_ctx first_ctx = make_ctx();
    vxui_ctx second_ctx = make_ctx( s_clay_mem2 );
    vxui_hud first = {}, second = {};

    vxui_frame( &first_ctx, 1.0f / 60.0f );
    vxui_hud_begin( &first, &first_ctx, 960, 540 );
    vxui_hud_text_box( &first, "box", "alpha\nbeta", 200, 100 );
    vxui_draw_list first_dl = vxui_render( &first_ctx );

    vxui_frame( &second_ctx, 1.0f / 60.0f );
    vxui_hud_begin( &second, &second_ctx, 960, 540 );
    vxui_hud_text_box( &second, "box", "alpha\nbeta", 200, 100 );
    vxui_draw_list second_dl = vxui_render( &second_ctx );

    ASSERT_EQ( vxui_draw_count( first_dl, VXUI_DRAW_TEXT ), 2 );
    ASSERT_EQ( vxui_draw_count( second_dl, VXUI_DRAW_TEXT ), 2 );
    for ( int i = 0; i < 2; i++ )
    {
        vxui_render_data out = {};
        ASSERT_FALSE( vxui_hud_resolve( &second, vxui_draw_nth( first_dl, VXUI_DRAW_TEXT, i ), &out ) );
        ASSERT_FALSE( vxui_hud_resolve( &first, vxui_draw_nth( second_dl, VXUI_DRAW_TEXT, i ), &out ) );
        ASSERT_TRUE( vxui_hud_resolve( &first, vxui_draw_nth( first_dl, VXUI_DRAW_TEXT, i ), &out ) );
        ASSERT_TRUE( vxui_hud_resolve( &second, vxui_draw_nth( second_dl, VXUI_DRAW_TEXT, i ), &out ) );
    }
}

UTEST(hud_text_box, copied_text_survives_caller_overwrite) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    char text[] = "alpha beta gamma";
    vxui_hud_text_box( &hud, "box", text, 48, 96, 0, 0 );
    memset( text, 'X', sizeof( text ) - 1 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), 3 );
    ASSERT_EQ( strncmp( ctx.text_buf, "alpha beta gamma", 16 ), 0 );
    for ( int i = 0; i < 3; i++ )
    {
        vxui_render_data out = {};
        ASSERT_TRUE( vxui_hud_resolve( &hud, vxui_draw_nth( dl, VXUI_DRAW_TEXT, i ), &out ) );
    }
}

UTEST(hud_text_box, empty_text_does_not_claim_the_next_source) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_set_colour( &hud, { 1, 0, 0, 1 } );
    vxui_hud_text_box( &hud, "empty", "", 200, 100 );
    vxui_hud_set_colour( &hud, { 0, 1, 0, 1 } );
    vxui_hud_text_box( &hud, "box", "alpha\nbeta", 200, 100 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), 2 );
    ASSERT_EQ( hud.item_count, 2 );
    ASSERT_TRUE( hud.items[0].text_base == nullptr );
    for ( int i = 0; i < 2; i++ )
    {
        vxui_render_data out = {};
        ASSERT_TRUE( vxui_hud_resolve( &hud, vxui_draw_nth( dl, VXUI_DRAW_TEXT, i ), &out ) );
        ASSERT_EQ( out.colour.r, 0.0f );
        ASSERT_EQ( out.colour.g, 1.0f );
    }
}

UTEST(hud_text_box, unrelated_text_leaves_render_data_untouched) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_text_box( &hud, "box", "alpha\nbeta", 200, 100 );
    vxui_text( &ctx, "ordinary", "alpha\nbeta", 24 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), 4 );
    int unclaimed = 0;
    for ( int i = 0; i < dl.count; i++ )
    {
        vxui_render_data out = {};
        out.texture_id = 99;
        out.colour     = { 0.25f, 0.5f, 0.75f, 1.0f };
        if ( vxui_hud_resolve( &hud, &dl.cmds[i], &out ) ) continue;
        ASSERT_EQ( out.texture_id, (uint32_t) 99 );
        ASSERT_EQ( out.colour.b, 0.75f );
        unclaimed++;
    }
    ASSERT_EQ( unclaimed, 2 );
}

UTEST(hud_text_box, same_id_rectangle_is_not_text) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_text_box( &hud, "box", "alpha\nbeta", 200, 100 );
    vxui_render( &ctx );

    vxui_draw_cmd c = {};
    c.id   = hud.items[0].id;
    c.type = VXUI_DRAW_RECT;
    vxui_render_data out = {};
    out.texture_id = 99;
    ASSERT_FALSE( vxui_hud_resolve( &hud, &c, &out ) );
    ASSERT_EQ( out.texture_id, (uint32_t) 99 );
}

UTEST(hud_text_box, registered_resolver_sees_every_wrapped_line) {
    vxui_ctx ctx = make_ctx();
    vxui_hud hud = {};
    vxui_set_render_data_fn( &ctx, resolve_hud, &hud );
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_set_colour( &hud, { 0.2f, 0.4f, 0.6f, 0.8f } );
    vxui_hud_text_box( &hud, "box", "alpha beta gamma", 48, 96, 0, 0 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), 3 );
    for ( int i = 0; i < 3; i++ )
    {
        const vxui_draw_cmd* c = vxui_draw_nth( dl, VXUI_DRAW_TEXT, i );
        ASSERT_EQ( c->render.colour.r, 0.2f );
        ASSERT_EQ( c->render.colour.a, 0.8f );
    }
}

UTEST(hud_text_box, reused_rectangle_slot_clears_both_source_bases) {
    vxui_ctx ctx = make_ctx();
    vxui_hud hud = {};
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_text_box( &hud, "box", "alpha\nbeta", 200, 100 );
    vxui_draw_list dl = vxui_render( &ctx );
    ASSERT_EQ( hud.items[0].text_base, ctx.text_buf );

    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_image( &hud, "image", 42, 40, 20 );
    dl = vxui_render( &ctx );
    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 1 );
    ASSERT_TRUE( hud.items[0].text_base == nullptr );
    const vxui_draw_cmd* c = vxui_draw_nth( dl, VXUI_DRAW_RECT, 0 );
    ASSERT_TRUE( c->text_base == nullptr );
    vxui_render_data out = {};
    ASSERT_TRUE( vxui_hud_resolve( &hud, c, &out ) );
    ASSERT_EQ( out.texture_id, (uint32_t) 42 );
}

UTEST(hud_text_box, line_ids_stay_distinct) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_text_box( &hud, "box", "alpha beta gamma", 48, 96, 0, 0 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), 3 );
    const vxui_draw_cmd* first = vxui_draw_nth( dl, VXUI_DRAW_TEXT, 0 );
    const vxui_draw_cmd* second = vxui_draw_nth( dl, VXUI_DRAW_TEXT, 1 );
    const vxui_draw_cmd* third = vxui_draw_nth( dl, VXUI_DRAW_TEXT, 2 );
    ASSERT_EQ( first->id, vxui_text_id( "box" ) );
    ASSERT_NE( first->id, second->id );
    ASSERT_NE( first->id, third->id );
    ASSERT_NE( second->id, third->id );
}

UTEST(hud_text_box, all_item_slots_resolve_without_per_line_records) {
    float height = (float) VXUI_HUD_MAX_ITEMS * 64.0f + 64.0f;
    vxui_ctx ctx = make_ctx( s_clay_mem, height );
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, height );
    char labels[VXUI_HUD_MAX_ITEMS][16] = {};
    for ( int i = 0; i < VXUI_HUD_MAX_ITEMS; i++ )
    {
        snprintf( labels[i], sizeof( labels[i] ), "box%d", i );
        vxui_hud_text_box( &hud, labels[i], "alpha\nbeta", 100, 64 );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( hud.item_count, VXUI_HUD_MAX_ITEMS );
    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), VXUI_HUD_MAX_ITEMS * 2 );
    for ( int i = 0; i < dl.count; i++ )
    {
        vxui_render_data out = {};
        ASSERT_TRUE( vxui_hud_resolve( &hud, &dl.cmds[i], &out ) );
    }
}

UTEST_MAIN();
