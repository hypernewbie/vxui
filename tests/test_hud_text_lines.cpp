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

UTEST(hud_text_lines, every_newline_resolves) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_set_colour( &hud, { 0.2f, 0.4f, 0.6f, 0.8f } );
    vxui_hud_text( &hud, "label", "alpha\nbeta\ngamma" );
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

UTEST(hud_text_lines, leading_empty_line_still_resolves) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_text( &hud, "label", "\nalpha" );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), 1 );
    const vxui_draw_cmd* c = vxui_draw_nth( dl, VXUI_DRAW_TEXT, 0 );
    ASSERT_NE( c->id, vxui_text_id( "label" ) );
    ASSERT_EQ( c->text_len, 5 );
    ASSERT_EQ( strncmp( c->text, "alpha", 5 ), 0 );
    vxui_render_data out = {};
    ASSERT_TRUE( vxui_hud_resolve( &hud, c, &out ) );
}

UTEST(hud_text_lines, blank_lines_do_not_break_source_matching) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_text( &hud, "label", "\nalpha\n\nbeta\n" );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), 2 );
    for ( int i = 0; i < 2; i++ )
    {
        vxui_render_data out = {};
        ASSERT_TRUE( vxui_hud_resolve( &hud, vxui_draw_nth( dl, VXUI_DRAW_TEXT, i ), &out ) );
    }
}

UTEST(hud_text_lines, identical_text_keeps_separate_colours) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_set_colour( &hud, { 1, 0, 0, 1 } );
    vxui_hud_text( &hud, "first", "alpha\nbeta" );
    vxui_hud_set_colour( &hud, { 0, 1, 0, 1 } );
    vxui_hud_text( &hud, "second", "alpha\nbeta" );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), 4 );
    ASSERT_EQ( hud.item_count, 2 );
    for ( int i = 0; i < 4; i++ )
    {
        vxui_render_data out = {};
        ASSERT_TRUE( vxui_hud_resolve( &hud, vxui_draw_nth( dl, VXUI_DRAW_TEXT, i ), &out ) );
        ASSERT_EQ( out.colour.r, i < 2 ? 1.0f : 0.0f );
        ASSERT_EQ( out.colour.g, i < 2 ? 0.0f : 1.0f );
    }
}

UTEST(hud_text_lines, separate_huds_do_not_claim_each_others_lines) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud first = {}, second = {};
    vxui_hud_begin( &first, &ctx, 960, 540 );
    vxui_hud_begin( &second, &ctx, 960, 540 );
    vxui_hud_text( &first, "first", "alpha\nbeta" );
    vxui_hud_text( &second, "second", "alpha\nbeta" );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), 4 );
    for ( int i = 0; i < 4; i++ )
    {
        vxui_render_data out = {};
        const vxui_draw_cmd* c = vxui_draw_nth( dl, VXUI_DRAW_TEXT, i );
        ASSERT_EQ( vxui_hud_resolve( &first, c, &out ), i < 2 );
        ASSERT_EQ( vxui_hud_resolve( &second, c, &out ), i >= 2 );
    }
}

UTEST(hud_text_lines, identical_ids_in_separate_contexts_do_not_match) {
    vxui_ctx first_ctx = make_ctx();
    vxui_ctx second_ctx = make_ctx( s_clay_mem2 );
    vxui_hud first = {}, second = {};

    vxui_frame( &first_ctx, 1.0f / 60.0f );
    vxui_hud_begin( &first, &first_ctx, 960, 540 );
    vxui_hud_text( &first, "label", "alpha\nbeta" );
    vxui_draw_list first_dl = vxui_render( &first_ctx );

    vxui_frame( &second_ctx, 1.0f / 60.0f );
    vxui_hud_begin( &second, &second_ctx, 960, 540 );
    vxui_hud_text( &second, "label", "alpha\nbeta" );
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

UTEST(hud_text_lines, copied_text_survives_caller_overwrite) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    char text[] = "alpha\nbeta";
    vxui_hud_text( &hud, "label", text );
    memset( text, 'X', sizeof( text ) - 1 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), 2 );
    ASSERT_EQ( strncmp( vxui_draw_nth( dl, VXUI_DRAW_TEXT, 0 )->text, "alpha", 5 ), 0 );
    ASSERT_EQ( strncmp( vxui_draw_nth( dl, VXUI_DRAW_TEXT, 1 )->text, "beta", 4 ), 0 );
    for ( int i = 0; i < 2; i++ )
    {
        vxui_render_data out = {};
        ASSERT_TRUE( vxui_hud_resolve( &hud, vxui_draw_nth( dl, VXUI_DRAW_TEXT, i ), &out ) );
    }
}

UTEST(hud_text_lines, registered_resolver_sees_every_line) {
    vxui_ctx ctx = make_ctx();
    vxui_hud hud = {};
    vxui_set_render_data_fn( &ctx, resolve_hud, &hud );
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_set_colour( &hud, { 0.2f, 0.4f, 0.6f, 0.8f } );
    vxui_hud_text( &hud, "label", "alpha\nbeta\ngamma" );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), 3 );
    for ( int i = 0; i < 3; i++ )
    {
        const vxui_draw_cmd* c = vxui_draw_nth( dl, VXUI_DRAW_TEXT, i );
        ASSERT_EQ( c->render.colour.r, 0.2f );
        ASSERT_EQ( c->render.colour.a, 0.8f );
    }
}

UTEST(hud_text_lines, non_hud_text_with_same_content_stays_unclaimed) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_text( &hud, "hud", "alpha\nbeta" );
    vxui_text( &ctx, "ordinary", "alpha\nbeta", 24 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), 4 );
    int claimed = 0, unclaimed = 0;
    for ( int i = 0; i < dl.count; i++ )
    {
        if ( dl.cmds[i].type != VXUI_DRAW_TEXT ) continue;
        vxui_render_data out = {};
        if ( vxui_hud_resolve( &hud, &dl.cmds[i], &out ) ) claimed++;
        else unclaimed++;
    }
    ASSERT_EQ( claimed, 2 );
    ASSERT_EQ( unclaimed, 2 );
}

UTEST(hud_text_lines, every_line_keeps_its_clay_id) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_text( &hud, "label", "alpha\nbeta\ngamma" );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), 3 );
    const vxui_draw_cmd* first = vxui_draw_nth( dl, VXUI_DRAW_TEXT, 0 );
    const vxui_draw_cmd* second = vxui_draw_nth( dl, VXUI_DRAW_TEXT, 1 );
    const vxui_draw_cmd* third = vxui_draw_nth( dl, VXUI_DRAW_TEXT, 2 );
    ASSERT_EQ( first->id, vxui_text_id( "label" ) );
    ASSERT_NE( first->id, second->id );
    ASSERT_NE( first->id, third->id );
    ASSERT_NE( second->id, third->id );
}

UTEST(hud_text_lines, all_item_slots_resolve_without_per_line_records) {
    float height = (float) VXUI_HUD_MAX_ITEMS * 48.0f + 48.0f;
    vxui_ctx ctx = make_ctx( s_clay_mem, height );
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, height );
    char labels[VXUI_HUD_MAX_ITEMS][16] = {};
    for ( int i = 0; i < VXUI_HUD_MAX_ITEMS; i++ )
    {
        snprintf( labels[i], sizeof( labels[i] ), "text%d", i );
        vxui_hud_text( &hud, labels[i], "alpha\nbeta" );
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

UTEST(hud_text_lines, text_box_newlines_resolve) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_text_box( &hud, "box", "alpha\nbeta", 200, 100 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), 2 );
    vxui_render_data out = {};
    ASSERT_TRUE( vxui_hud_resolve( &hud, vxui_draw_nth( dl, VXUI_DRAW_TEXT, 0 ), &out ) );
    ASSERT_TRUE( vxui_hud_resolve( &hud, vxui_draw_nth( dl, VXUI_DRAW_TEXT, 1 ), &out ) );
}

UTEST(hud_text_lines, every_line_carries_the_declaration_record) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_text( &hud, "label", "alpha\nbeta\ngamma" );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), 3 );
    for ( int i = 0; i < 3; i++ )
        ASSERT_EQ( vxui_draw_nth( dl, VXUI_DRAW_TEXT, i )->userdata, (const void*) &hud.items[0] );
    ASSERT_EQ( vxui_draw_nth( dl, VXUI_DRAW_TEXT, 0 )->text, ctx.text_buf );
    ASSERT_EQ( vxui_draw_nth( dl, VXUI_DRAW_TEXT, 1 )->text, ctx.text_buf + 6 );
    ASSERT_EQ( vxui_draw_nth( dl, VXUI_DRAW_TEXT, 2 )->text, ctx.text_buf + 11 );
}

UTEST(hud_text_lines, wrapped_core_text_has_no_hud_attachment) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_div( &ctx, "wrap", { .width = { VXUI_FIXED, 48 }, .col = true } );
    vxui_text( &ctx, "label", "alpha beta gamma", 24 );
    vxui_div_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), 3 );
    for ( int i = 0; i < 3; i++ )
        ASSERT_TRUE( vxui_draw_nth( dl, VXUI_DRAW_TEXT, i )->userdata == nullptr );
}

UTEST(hud_text_lines, identical_content_has_distinct_records) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_text( &hud, "first", "alpha\nbeta" );
    vxui_hud_text( &hud, "second", "alpha\nbeta" );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), 4 );
    for ( int i = 0; i < 4; i++ )
        ASSERT_EQ( vxui_draw_nth( dl, VXUI_DRAW_TEXT, i )->userdata, (const void*) &hud.items[i < 2 ? 0 : 1] );
}

UTEST(hud_text_lines, empty_text_does_not_claim_the_next_source) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_set_colour( &hud, { 1, 0, 0, 1 } );
    vxui_hud_text( &hud, "empty", "" );
    vxui_hud_set_colour( &hud, { 0, 1, 0, 1 } );
    vxui_hud_text( &hud, "label", "alpha\nbeta" );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), 2 );
    ASSERT_EQ( hud.item_count, 2 );
    for ( int i = 0; i < 2; i++ )
    {
        ASSERT_EQ( vxui_draw_nth( dl, VXUI_DRAW_TEXT, i )->userdata, (const void*) &hud.items[1] );
        vxui_render_data out = {};
        ASSERT_TRUE( vxui_hud_resolve( &hud, vxui_draw_nth( dl, VXUI_DRAW_TEXT, i ), &out ) );
        ASSERT_EQ( out.colour.r, 0.0f );
        ASSERT_EQ( out.colour.g, 1.0f );
    }
}

UTEST(hud_text_lines, reused_rectangle_slot_has_current_metadata) {
    vxui_ctx ctx = make_ctx();
    vxui_hud hud = {};
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_text( &hud, "label", "alpha\nbeta" );
    vxui_draw_list dl = vxui_render( &ctx );
    ASSERT_EQ( vxui_draw_nth( dl, VXUI_DRAW_TEXT, 0 )->userdata, (const void*) &hud.items[0] );

    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_image( &hud, "image", 42, 40, 20 );
    dl = vxui_render( &ctx );
    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 1 );
    const vxui_draw_cmd* c = vxui_draw_nth( dl, VXUI_DRAW_RECT, 0 );
    ASSERT_EQ( c->userdata, (const void*) &hud.items[0] );
    vxui_render_data out = {};
    ASSERT_TRUE( vxui_hud_resolve( &hud, c, &out ) );
    ASSERT_EQ( out.texture_id, (uint32_t) 42 );
}

UTEST_MAIN();
