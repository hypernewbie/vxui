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

UTEST(hud_text_box_id, first_line_matches_the_core_for_varied_identifiers) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    const char* names[] = { "box", "Box", "hud.box", "box##one", "two words", "", "caf\xc3\xa9", "a_long_immediate_mode_identifier_with_a_stable_suffix" };
    int n = (int) ( sizeof( names ) / sizeof( names[0] ) );
    for ( int i = 0; i < n; i++ ) vxui_hud_text_box( &hud, names[i], "alpha", 120, 48 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, n );
    for ( int i = 0; i < n; i++ )
    {
        const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_TEXT, vxui_text_id( names[i] ) );
        ASSERT_TRUE( c );
        ASSERT_EQ( c->userdata, (const void*) &hud.items[i] );
        ASSERT_EQ( c->text_len, 5 );
    }
}

UTEST(hud_text_box_id, all_alignments_keep_wrapped_line_ids) {
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

            ASSERT_EQ( dl.count, 3 );
            for ( int i = 0; i < 3; i++ )
            {
                ASSERT_EQ( dl.cmds[i].id, line_id( "box", i ) );
                ASSERT_EQ( dl.cmds[i].userdata, (const void*) &hud.items[0] );
            }
        }
    }
}

UTEST(hud_text_box_id, blank_lines_keep_their_original_indices) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_text_box( &hud, "box", "\nalpha\n\nbeta", 120, 144, 0, 0 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 2 );
    ASSERT_EQ( dl.cmds[0].id, line_id( "box", 1 ) );
    ASSERT_EQ( dl.cmds[1].id, line_id( "box", 3 ) );
}

UTEST(hud_text_box_id, caller_id_overwrite_does_not_change_identity) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    char id[] = "box";
    vxui_hud_text_box( &hud, id, "alpha", 120, 48 );
    memset( id, 'x', 3 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 1 );
    ASSERT_EQ( dl.cmds[0].id, vxui_text_id( "box" ) );
    ASSERT_NE( dl.cmds[0].id, vxui_text_id( "xxx" ) );
}

UTEST(hud_text_box_id, changing_box_width_only_changes_wrap_count) {
    vxui_ctx ctx = make_ctx();
    vxui_hud hud = {};
    float widths[] = { 160, 48, 160 };
    int counts[] = { 1, 3, 1 };
    for ( int frame = 0; frame < 3; frame++ )
    {
        vxui_frame( &ctx, 1.0f / 60.0f );
        vxui_hud_begin( &hud, &ctx, 960, 540 );
        vxui_hud_text_box( &hud, "box", "alpha beta gamma", widths[frame], 96, 0, 0 );
        vxui_draw_list dl = vxui_render( &ctx );

        ASSERT_EQ( dl.count, counts[frame] );
        ASSERT_EQ( hud.item_count, 1 );
        for ( int i = 0; i < dl.count; i++ ) ASSERT_EQ( dl.cmds[i].id, line_id( "box", i ) );
    }
}

UTEST(hud_text_box_id, boxed_plain_and_core_text_share_line_identity) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_text( &ctx, "shared", "alpha\nbeta", 24 );
    vxui_draw_list core = vxui_render( &ctx );
    ASSERT_EQ( core.count, 2 );
    uint32_t ids[] = { core.cmds[0].id, core.cmds[1].id };
    vxui_hud hud = {};
    for ( int boxed = 0; boxed < 2; boxed++ )
    {
        vxui_frame( &ctx, 1.0f / 60.0f );
        vxui_hud_begin( &hud, &ctx, 960, 540 );
        if ( boxed ) vxui_hud_text_box( &hud, "shared", "alpha\nbeta", 120, 96 );
        else         vxui_hud_text( &hud, "shared", "alpha\nbeta" );
        vxui_draw_list dl = vxui_render( &ctx );

        ASSERT_EQ( dl.count, 2 );
        for ( int i = 0; i < 2; i++ ) ASSERT_EQ( dl.cmds[i].id, ids[i] );
    }
}

UTEST(hud_text_box_id, declaration_order_keeps_ids_attached_to_current_records) {
    vxui_ctx ctx = make_ctx();
    vxui_hud hud = {};
    const char* names[] = { "first", "second" };
    for ( int frame = 0; frame < 2; frame++ )
    {
        vxui_frame( &ctx, 1.0f / 60.0f );
        vxui_hud_begin( &hud, &ctx, 960, 540 );
        for ( int i = 0; i < 2; i++ ) vxui_hud_text_box( &hud, names[frame == 0 ? i : 1 - i], "alpha", 120, 48 );
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

UTEST(hud_text_box_id, empty_box_does_not_take_the_following_text_id) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_text_box( &hud, "empty", "", 120, 48 );
    vxui_hud_text_box( &hud, "next", "alpha", 120, 48 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 1 );
    ASSERT_EQ( hud.item_count, 2 );
    ASSERT_EQ( dl.cmds[0].id, vxui_text_id( "next" ) );
    ASSERT_NE( dl.cmds[0].id, vxui_text_id( "empty" ) );
    ASSERT_EQ( dl.cmds[0].userdata, (const void*) &hud.items[1] );
}

UTEST(hud_text_box_id, every_box_retains_both_line_ids_at_packet_capacity) {
    float height = (float) VXUI_HUD_MAX_ITEMS * 48.0f + 48.0f;
    vxui_ctx ctx = make_ctx( height );
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, height );
    char names[VXUI_HUD_MAX_ITEMS][16] = {};
    for ( int i = 0; i < VXUI_HUD_MAX_ITEMS; i++ )
    {
        snprintf( names[i], sizeof( names[i] ), "box%d", i );
        vxui_hud_text_box( &hud, names[i], "a\nb", 120, 48 );
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
