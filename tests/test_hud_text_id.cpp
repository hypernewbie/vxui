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

static vxui_ctx make_ctx( float h = 540 )
{
    vxui_ctx ctx = {};
    vxui_init( &ctx, 960, h, s_clay_mem, sizeof( s_clay_mem ) );
    Clay_SetMeasureTextFunction( measure_text, nullptr );
    return ctx;
}

static uint32_t line_id( const char* id, int line )
{
    return Clay__HashNumber( (uint32_t) line, Clay__HashNumber( 0, vxui_id( id ) ).id ).id;
}

UTEST(hud_text_id, first_line_matches_the_core_for_varied_identifiers) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    const char* names[] = { "text", "Text", "hud.text", "text##one", "two words", "", "caf\xc3\xa9", "a_long_immediate_mode_identifier_with_a_stable_suffix" };
    int n = (int) ( sizeof( names ) / sizeof( names[0] ) );
    for ( int i = 0; i < n; i++ ) vxui_hud_text( &hud, names[i], "alpha" );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, n );
    for ( int i = 0; i < n; i++ )
    {
        const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_TEXT, vxui_text_id( names[i] ) );
        ASSERT_TRUE( c );
        ASSERT_EQ( c->userdata, (const void*) &hud.items[i] );
        ASSERT_EQ( c->text_len, 5 );
        ASSERT_EQ( memcmp( c->text, "alpha", 5 ), 0 );
    }
}

UTEST(hud_text_id, newline_ids_keep_the_core_parent_and_line_index) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_text( &hud, "text", "alpha\nbeta\ngamma" );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 3 );
    for ( int i = 0; i < 3; i++ )
    {
        ASSERT_EQ( dl.cmds[i].id, line_id( "text", i ) );
        ASSERT_EQ( dl.cmds[i].userdata, (const void*) &hud.items[0] );
    }
}

UTEST(hud_text_id, blank_lines_preserve_their_line_indices) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_text( &hud, "text", "\nalpha\n\nbeta" );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 2 );
    ASSERT_EQ( dl.cmds[0].id, line_id( "text", 1 ) );
    ASSERT_EQ( dl.cmds[1].id, line_id( "text", 3 ) );
    ASSERT_NE( dl.cmds[0].id, vxui_text_id( "text" ) );
}

UTEST(hud_text_id, caller_id_overwrite_does_not_change_emitted_identity) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    char id[] = "text";
    vxui_hud_text( &hud, id, "alpha" );
    memset( id, 'x', 4 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 1 );
    ASSERT_EQ( dl.cmds[0].id, vxui_text_id( "text" ) );
    ASSERT_NE( dl.cmds[0].id, vxui_text_id( "xxxx" ) );
}

UTEST(hud_text_id, changing_display_text_does_not_change_logical_identity) {
    vxui_ctx ctx = make_ctx();
    vxui_hud hud = {};
    const char* values[] = { "alpha", "b", "", "alpha\nbeta" };
    int counts[] = { 1, 1, 0, 2 };
    for ( int frame = 0; frame < 4; frame++ )
    {
        vxui_frame( &ctx, 1.0f / 60.0f );
        vxui_hud_begin( &hud, &ctx, 960, 540 );
        vxui_hud_set_colour( &hud, { (float) frame, 0, 0, 1 } );
        vxui_hud_text( &hud, "text", values[frame] );
        vxui_draw_list dl = vxui_render( &ctx );

        ASSERT_EQ( dl.count, counts[frame] );
        ASSERT_EQ( hud.item_count, 1 );
        for ( int i = 0; i < dl.count; i++ )
        {
            ASSERT_EQ( dl.cmds[i].id, line_id( "text", i ) );
            ASSERT_EQ( dl.cmds[i].userdata, (const void*) &hud.items[0] );
            vxui_render_data out = {};
            ASSERT_TRUE( vxui_hud_resolve( &hud, &dl.cmds[i], &out ) );
            ASSERT_EQ( out.colour.r, (float) frame );
        }
    }
}

UTEST(hud_text_id, hud_and_core_text_use_the_same_line_ids) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_text( &ctx, "shared", "alpha\nbeta", 24 );
    vxui_draw_list core = vxui_render( &ctx );
    ASSERT_EQ( core.count, 2 );
    uint32_t ids[] = { core.cmds[0].id, core.cmds[1].id };
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_text( &hud, "shared", "alpha\nbeta" );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 2 );
    for ( int i = 0; i < 2; i++ ) ASSERT_EQ( dl.cmds[i].id, ids[i] );
}

UTEST(hud_text_id, declaration_order_does_not_enter_text_identity) {
    vxui_ctx ctx = make_ctx();
    vxui_hud hud = {};
    const char* names[] = { "first", "second" };
    for ( int frame = 0; frame < 2; frame++ )
    {
        vxui_frame( &ctx, 1.0f / 60.0f );
        vxui_hud_begin( &hud, &ctx, 960, 540 );
        for ( int i = 0; i < 2; i++ ) vxui_hud_text( &hud, names[frame == 0 ? i : 1 - i], "alpha" );
        vxui_draw_list dl = vxui_render( &ctx );

        ASSERT_EQ( dl.count, 2 );
        for ( int i = 0; i < 2; i++ )
        {
            int logical = frame == 0 ? i : 1 - i;
            ASSERT_EQ( dl.cmds[i].id, vxui_text_id( names[logical] ) );
            ASSERT_EQ( dl.cmds[i].userdata, (const void*) &hud.items[i] );
        }
    }
}

UTEST(hud_text_id, every_item_retains_both_line_ids_at_packet_capacity) {
    float height = (float) VXUI_HUD_MAX_ITEMS * 48.0f + 48.0f;
    vxui_ctx ctx = make_ctx( height );
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, height );
    char names[VXUI_HUD_MAX_ITEMS][16] = {};
    for ( int i = 0; i < VXUI_HUD_MAX_ITEMS; i++ )
    {
        snprintf( names[i], sizeof( names[i] ), "text%d", i );
        vxui_hud_text( &hud, names[i], "a\nb" );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, VXUI_HUD_MAX_ITEMS * 2 );
    ASSERT_EQ( hud.item_count, VXUI_HUD_MAX_ITEMS );
    for ( int i = 0; i < VXUI_HUD_MAX_ITEMS; i++ )
    {
        for ( int line = 0; line < 2; line++ )
        {
            const vxui_draw_cmd& c = dl.cmds[i * 2 + line];
            ASSERT_EQ( c.id, line_id( names[i], line ) );
            ASSERT_EQ( c.userdata, (const void*) &hud.items[i] );
        }
    }
}

UTEST_MAIN();
