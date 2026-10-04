#include "utest.h"
#include "../vxui.h"
#define VXUI_IMPL
#include "../vxui_impl.h"
#define VXUI_HUD_IMPL
#include "../vxui_hud.h"
#include <cstdlib>
#ifdef _MSC_VER
#include <crtdbg.h>
#endif

static uint8_t s_clay_mem[16 * 1024 * 1024];
static const char* s_executable = nullptr;

static vxui_ctx make_ctx()
{
    vxui_ctx ctx = {};
    vxui_init( &ctx, 1280, 720, s_clay_mem, sizeof( s_clay_mem ) );
    return ctx;
}

UTEST(hud_text_box_arena, caller_overwrite_keeps_text_font_and_metadata) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 1280, 720 );
    vxui_hud_set_pos( &hud, 12, 10 );
    vxui_hud_set_font( &hud, 7, 27 );
    vxui_hud_set_colour( &hud, { 1, 0, 0, 1 } );
    char text[] = "alpha";
    vxui_hud_text_box( &hud, "box", text, 160, 60 );
    memset( text, 'x', 5 );
    vxui_hud_set_font( &hud, 3, 48 );
    vxui_hud_set_colour( &hud, { 0, 1, 0, 1 } );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 1 );
    ASSERT_EQ( ctx.text_offset, 5 );
    ASSERT_EQ( hud.item_count, 1 );
    ASSERT_EQ( hud.x, 12.0f );
    ASSERT_EQ( hud.y, 70.0f );
    const vxui_draw_cmd& c = dl.cmds[0];
    ASSERT_EQ( c.text, ctx.text_buf );
    ASSERT_EQ( c.text_len, 5 );
    ASSERT_EQ( memcmp( c.text, "alpha", 5 ), 0 );
    ASSERT_EQ( c.font, (uint16_t) 7 );
    ASSERT_EQ( c.font_px, (uint16_t) 27 );
    ASSERT_EQ( c.userdata, (const void*) &hud.items[0] );
    vxui_render_data out = {};
    ASSERT_TRUE( vxui_hud_resolve( &hud, &c, &out ) );
    ASSERT_EQ( out.colour.r, 1.0f );
    ASSERT_EQ( out.colour.g, 0.0f );
}

UTEST(hud_text_box_arena, empty_box_advances_by_box_height_and_keeps_one_record) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 1280, 720 );
    vxui_hud_set_pos( &hud, 12, 10 );
    vxui_hud_text_box( &hud, "empty", "", 160, 60 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 0 );
    ASSERT_EQ( ctx.text_offset, 0 );
    ASSERT_EQ( hud.item_count, 1 );
    ASSERT_EQ( hud.x, 12.0f );
    ASSERT_EQ( hud.y, 70.0f );
}

UTEST(hud_text_box_arena, every_alignment_accepts_exact_capacity_text) {
    char text[VXUI_MAX_TEXT_BYTES + 1];
    memset( text, 'a', VXUI_MAX_TEXT_BYTES );
    text[VXUI_MAX_TEXT_BYTES] = 0;
    for ( uint8_t x = 0; x < 3; x++ )
    {
        for ( uint8_t y = 0; y < 3; y++ )
        {
            vxui_ctx ctx = make_ctx();
            vxui_frame( &ctx, 1.0f / 60.0f );
            vxui_hud hud = {};
            vxui_hud_begin( &hud, &ctx, 1280, 720 );
            vxui_hud_text_box( &hud, "full", text, 160, 60, x, y );
            vxui_draw_list dl = vxui_render( &ctx );

            ASSERT_EQ( ctx.text_offset, VXUI_MAX_TEXT_BYTES );
            ASSERT_EQ( dl.count, 1 );
            ASSERT_EQ( hud.item_count, 1 );
            ASSERT_EQ( hud.y, 60.0f );
            ASSERT_EQ( dl.cmds[0].text, ctx.text_buf );
            ASSERT_EQ( dl.cmds[0].text_len, VXUI_MAX_TEXT_BYTES );
            ASSERT_EQ( memcmp( dl.cmds[0].text, text, VXUI_MAX_TEXT_BYTES ), 0 );
            ASSERT_EQ( dl.cmds[0].userdata, (const void*) &hud.items[0] );
        }
    }
}

UTEST(hud_text_box_arena, multiple_boxes_share_final_bytes_but_keep_distinct_records) {
    vxui_ctx ctx = make_ctx();
    char prefix[VXUI_MAX_TEXT_BYTES - 2];
    memset( prefix, 'a', sizeof( prefix ) - 1 );
    prefix[sizeof( prefix ) - 1] = 0;
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 1280, 720 );
    vxui_hud_set_colour( &hud, { 1, 0, 0, 1 } );
    vxui_hud_text_box( &hud, "first", prefix, 160, 60 );
    vxui_hud_set_colour( &hud, { 0, 1, 0, 1 } );
    vxui_hud_text_box( &hud, "last", "end", 160, 60 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( ctx.text_offset, VXUI_MAX_TEXT_BYTES );
    ASSERT_EQ( dl.count, 2 );
    ASSERT_EQ( hud.item_count, 2 );
    ASSERT_EQ( hud.y, 120.0f );
    ASSERT_EQ( dl.cmds[0].userdata, (const void*) &hud.items[0] );
    ASSERT_EQ( dl.cmds[1].userdata, (const void*) &hud.items[1] );
    ASSERT_EQ( dl.cmds[1].text, ctx.text_buf + VXUI_MAX_TEXT_BYTES - 3 );
    ASSERT_EQ( dl.cmds[1].text_len, 3 );
    ASSERT_EQ( memcmp( dl.cmds[1].text, "end", 3 ), 0 );
    for ( int i = 0; i < 2; i++ )
    {
        vxui_render_data out = {};
        ASSERT_TRUE( vxui_hud_resolve( &hud, &dl.cmds[i], &out ) );
        ASSERT_EQ( out.colour.r, i == 0 ? 1.0f : 0.0f );
        ASSERT_EQ( out.colour.g, i == 0 ? 0.0f : 1.0f );
    }
}

UTEST(hud_text_box_arena, empty_box_is_valid_at_full_capacity) {
    vxui_ctx ctx = make_ctx();
    char text[VXUI_MAX_TEXT_BYTES + 1];
    memset( text, 'a', VXUI_MAX_TEXT_BYTES );
    text[VXUI_MAX_TEXT_BYTES] = 0;
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 1280, 720 );
    vxui_hud_text_box( &hud, "full", text, 160, 60 );
    vxui_hud_text_box( &hud, "empty", "", 160, 30 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( ctx.text_offset, VXUI_MAX_TEXT_BYTES );
    ASSERT_EQ( dl.count, 1 );
    ASSERT_EQ( hud.item_count, 2 );
    ASSERT_EQ( hud.y, 90.0f );
    ASSERT_EQ( dl.cmds[0].userdata, (const void*) &hud.items[0] );
}

UTEST(hud_text_box_arena, core_and_box_text_share_the_same_arena_capacity) {
    vxui_ctx ctx = make_ctx();
    char prefix[VXUI_MAX_TEXT_BYTES - 3];
    memset( prefix, 'a', sizeof( prefix ) - 1 );
    prefix[sizeof( prefix ) - 1] = 0;
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_text( &ctx, "ordinary", prefix, 24 );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 1280, 720 );
    vxui_hud_text_box( &hud, "box", "tail", 160, 60 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( ctx.text_offset, VXUI_MAX_TEXT_BYTES );
    ASSERT_EQ( dl.count, 2 );
    const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_TEXT, vxui_text_id( "box" ) );
    ASSERT_TRUE( c );
    ASSERT_EQ( c->text, ctx.text_buf + VXUI_MAX_TEXT_BYTES - 4 );
    ASSERT_EQ( c->text_len, 4 );
    ASSERT_EQ( memcmp( c->text, "tail", 4 ), 0 );
    ASSERT_EQ( c->userdata, (const void*) &hud.items[0] );
}

UTEST(hud_text_box_arena, empty_record_does_not_claim_unattached_text) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 1280, 720 );
    vxui_hud_text_box( &hud, "empty", "", 160, 60 );
    vxui_text( &ctx, "ordinary", "alpha", 24 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 1 );
    ASSERT_EQ( dl.cmds[0].text, ctx.text_buf );
    ASSERT_TRUE( dl.cmds[0].userdata == nullptr );
    vxui_render_data out = {};
    ASSERT_FALSE( vxui_hud_resolve( &hud, &dl.cmds[0], &out ) );
}

UTEST(hud_text_box_arena, next_frame_reuses_text_bytes_and_metadata_slots) {
    vxui_ctx ctx = make_ctx();
    vxui_hud hud = {};
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud_begin( &hud, &ctx, 1280, 720 );
    vxui_hud_set_colour( &hud, { 1, 0, 0, 1 } );
    vxui_hud_text_box( &hud, "box", "alpha", 160, 60 );
    vxui_render( &ctx );
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud_begin( &hud, &ctx, 1280, 720 );
    vxui_hud_set_colour( &hud, { 0, 1, 0, 1 } );
    vxui_hud_text_box( &hud, "box", "b", 160, 30 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( ctx.text_offset, 1 );
    ASSERT_EQ( dl.count, 1 );
    ASSERT_EQ( hud.item_count, 1 );
    ASSERT_EQ( hud.y, 30.0f );
    ASSERT_EQ( dl.cmds[0].text, ctx.text_buf );
    ASSERT_EQ( dl.cmds[0].text_len, 1 );
    ASSERT_EQ( dl.cmds[0].text[0], 'b' );
    ASSERT_EQ( dl.cmds[0].userdata, (const void*) &hud.items[0] );
    vxui_render_data out = {};
    ASSERT_TRUE( vxui_hud_resolve( &hud, &dl.cmds[0], &out ) );
    ASSERT_EQ( out.colour.r, 0.0f );
    ASSERT_EQ( out.colour.g, 1.0f );
}

#ifndef NDEBUG
static bool overflow_asserts()
{
    char command[4096];
#ifdef _MSC_VER
    int len = snprintf( command, sizeof( command ), "\"\"%s\" --overflow 2>&1\"", s_executable );
#else
    int len = snprintf( command, sizeof( command ), "ulimit -c 0; \"%s\" --overflow 2>&1", s_executable );
#endif
    assert( len > 0 && len < (int) sizeof( command ) );
#ifdef _MSC_VER
    FILE* pipe = _popen( command, "r" );
#else
    FILE* pipe = popen( command, "r" );
#endif
    if ( !pipe ) return false;
    bool seen = false;
    char line[512];
    while ( fgets( line, sizeof( line ), pipe ) )
        if ( strstr( line, "ctx->text_offset + len <= VXUI_MAX_TEXT_BYTES" ) ) seen = true;
#ifdef _MSC_VER
    int status = _pclose( pipe );
#else
    int status = pclose( pipe );
#endif
    return status != 0 && seen;
}

UTEST(hud_text_box_arena, overflow_hits_the_checked_arena_assertion) {
    ASSERT_TRUE( overflow_asserts() );
}
#endif

UTEST_STATE();

int main( int argc, const char* const argv[] )
{
#ifdef _MSC_VER
    _set_error_mode( _OUT_TO_STDERR );
    _set_abort_behavior( 0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT );
    _CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_FILE );
    _CrtSetReportFile( _CRT_ASSERT, _CRTDBG_FILE_STDERR );
#endif
    s_executable = argv[0];
    if ( argc == 2 && strcmp( argv[1], "--overflow" ) == 0 )
    {
        vxui_ctx ctx = make_ctx();
        char text[VXUI_MAX_TEXT_BYTES + 2];
        memset( text, 'a', VXUI_MAX_TEXT_BYTES + 1 );
        text[VXUI_MAX_TEXT_BYTES + 1] = 0;
        vxui_frame( &ctx, 1.0f / 60.0f );
        vxui_hud hud = {};
        vxui_hud_begin( &hud, &ctx, 1280, 720 );
        vxui_hud_text_box( &hud, "overflow", text, 160, 60 );
        return 0;
    }
    return utest_main( argc, argv );
}
