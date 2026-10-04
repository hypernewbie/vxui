#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
    #define NOMINMAX
#endif
#ifdef _WIN32
    #include <windows.h>
#endif

#include "utest.h"
#include "../vxui.h"
#define VXUI_IMPL
#include "../vxui_impl.h"
#define VXUI_HUD_IMPL
#include "../vxui_hud.h"
#ifdef _MSC_VER
#include <crtdbg.h>
#endif

static uint8_t s_clay_mem[16 * 1024 * 1024];
static const char* s_executable = nullptr;

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

static Clay_ElementId clay_id( const char* name )
{
    return Clay_GetElementId( { false, (int32_t) strlen( name ), name } );
}

static uint32_t line_id( const char* name, int line )
{
    return Clay__HashNumber( (uint32_t) line, Clay__HashNumber( 0, clay_id( name ).id ).id ).id;
}

UTEST(text_id, varied_identifiers_keep_wrapper_and_first_line_ids) {
    vxui_ctx ctx = make_ctx();
    const char* names[] = { "text", "Text", "layout.text", "text##one", "two words", "", "caf\xc3\xa9", "a_long_immediate_mode_identifier_with_a_stable_suffix" };
    int n = (int) ( sizeof( names ) / sizeof( names[0] ) );
    vxui_frame( &ctx, 1.0f / 60.0f );
    for ( int i = 0; i < n; i++ ) vxui_text( &ctx, names[i], "A", 24 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, n );
    for ( int i = 0; i < n; i++ )
    {
        ASSERT_TRUE( Clay_GetElementData( clay_id( names[i] ) ).found );
        ASSERT_EQ( dl.cmds[i].id, line_id( names[i], 0 ) );
        ASSERT_EQ( dl.cmds[i].id, vxui_text_id( names[i] ) );
        ASSERT_EQ( dl.cmds[i].font_px, (uint16_t) 24 );
        ASSERT_TRUE( dl.cmds[i].userdata == nullptr );
    }
}

UTEST(text_id, blank_newlines_keep_original_line_indices) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_text( &ctx, "text", "\nalpha\n\nbeta", 24 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 2 );
    ASSERT_EQ( dl.cmds[0].id, line_id( "text", 1 ) );
    ASSERT_EQ( dl.cmds[1].id, line_id( "text", 3 ) );
    ASSERT_EQ( dl.cmds[0].text, ctx.text_buf + 1 );
    ASSERT_EQ( dl.cmds[1].text, ctx.text_buf + 8 );
    ASSERT_EQ( dl.cmds[0].rect.y, 24.0f );
    ASSERT_EQ( dl.cmds[1].rect.y, 72.0f );
}

UTEST(text_id, wrapping_changes_line_count_not_the_wrapper_hash) {
    vxui_ctx ctx = make_ctx();
    float widths[] = { 160, 48, 160 };
    int counts[] = { 1, 3, 1 };
    for ( int frame = 0; frame < 3; frame++ )
    {
        vxui_frame( &ctx, 1.0f / 60.0f );
        vxui_div( &ctx, "container", { .width = { VXUI_FIXED, widths[frame] }, .height = { VXUI_FIXED, 96 } } );
        vxui_text( &ctx, "text", "alpha beta gamma", 24 );
        vxui_div_end( &ctx );
        vxui_draw_list dl = vxui_render( &ctx );

        ASSERT_TRUE( Clay_GetElementData( clay_id( "text" ) ).found );
        ASSERT_EQ( dl.count, counts[frame] );
        for ( int i = 0; i < dl.count; i++ ) ASSERT_EQ( dl.cmds[i].id, line_id( "text", i ) );
    }
}

UTEST(text_id, display_text_and_font_size_do_not_change_identity) {
    vxui_ctx ctx = make_ctx();
    const char* values[] = { "a", "beta", "gamma" };
    uint16_t sizes[] = { 24, 31, 0 };
    for ( int frame = 0; frame < 3; frame++ )
    {
        vxui_frame( &ctx, 1.0f / 60.0f );
        vxui_text( &ctx, "stable", values[frame], sizes[frame] );
        vxui_draw_list dl = vxui_render( &ctx );

        ASSERT_EQ( dl.count, 1 );
        ASSERT_EQ( dl.cmds[0].id, line_id( "stable", 0 ) );
        ASSERT_EQ( dl.cmds[0].font_px, sizes[frame] ? sizes[frame] : (uint16_t) VXUI_FONT_SIZE_DEFAULT );
        ASSERT_EQ( dl.cmds[0].text_len, (int) strlen( values[frame] ) );
        ASSERT_EQ( memcmp( dl.cmds[0].text, values[frame], dl.cmds[0].text_len ), 0 );
    }
}

UTEST(text_id, caller_id_and_display_overwrites_do_not_change_the_packet) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    char id[] = "mutable";
    char label[] = "alpha";
    vxui_text( &ctx, id, label, 24 );
    memset( id, 'x', 7 );
    memset( label, 'x', 5 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 1 );
    ASSERT_EQ( dl.cmds[0].id, line_id( "mutable", 0 ) );
    ASSERT_NE( dl.cmds[0].id, line_id( "xxxxxxx", 0 ) );
    ASSERT_EQ( dl.cmds[0].text, ctx.text_buf );
    ASSERT_EQ( memcmp( dl.cmds[0].text, "alpha", 5 ), 0 );
}

UTEST(text_id, core_and_both_hud_text_forms_share_line_identity) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_text( &ctx, "shared", "alpha\nbeta", 24 );
    vxui_draw_list core = vxui_render( &ctx );
    ASSERT_EQ( core.count, 2 );
    uint32_t ids[] = { core.cmds[0].id, core.cmds[1].id };
    for ( int boxed = 0; boxed < 2; boxed++ )
    {
        vxui_frame( &ctx, 1.0f / 60.0f );
        vxui_hud hud = {};
        vxui_hud_begin( &hud, &ctx, 960, 540 );
        if ( boxed ) vxui_hud_text_box( &hud, "shared", "alpha\nbeta", 120, 96 );
        else         vxui_hud_text( &hud, "shared", "alpha\nbeta" );
        vxui_draw_list dl = vxui_render( &ctx );

        ASSERT_EQ( dl.count, 2 );
        for ( int i = 0; i < 2; i++ )
        {
            ASSERT_EQ( dl.cmds[i].id, ids[i] );
            ASSERT_EQ( dl.cmds[i].userdata, (const void*) &hud.items[0] );
        }
    }
}

UTEST(text_id, empty_text_does_not_take_the_following_text_id) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_text( &ctx, "empty", "", 24 );
    vxui_text( &ctx, "next", "alpha", 24 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 1 );
    ASSERT_EQ( dl.cmds[0].id, line_id( "next", 0 ) );
    ASSERT_NE( dl.cmds[0].id, line_id( "empty", 0 ) );
    ASSERT_EQ( ctx.text_offset, 5 );
}

UTEST(text_id, every_text_packet_keeps_its_line_id_at_capacity) {
    vxui_ctx ctx = make_ctx();
    char names[VXUI_MAX_DRAW_CMDS][16] = {};
    char roots[VXUI_MAX_DRAW_CMDS][16] = {};
    vxui_frame( &ctx, 1.0f / 60.0f );
    for ( int i = 0; i < VXUI_MAX_DRAW_CMDS; i++ )
    {
        snprintf( names[i], sizeof( names[i] ), "text%d", i );
        snprintf( roots[i], sizeof( roots[i] ), "root%d", i );
        vxui_root( &ctx, roots[i], (float) ( i % 64 ) * 8, (float) ( i / 64 ) * 24 );
        vxui_text( &ctx, names[i], "A", 24 );
        vxui_root_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, VXUI_MAX_DRAW_CMDS );
    ASSERT_EQ( ctx.text_offset, VXUI_MAX_DRAW_CMDS );
    for ( int i = 0; i < VXUI_MAX_DRAW_CMDS; i++ )
    {
        ASSERT_EQ( dl.cmds[i].id, line_id( names[i], 0 ) );
        ASSERT_TRUE( dl.cmds[i].userdata == nullptr );
    }
}

#ifndef NDEBUG
static bool null_id_asserts()
{
    char command[4096];
#ifdef _MSC_VER
    int len = snprintf( command, sizeof( command ), "\"\"%s\" --null-id 2>&1\"", s_executable );
#else
    int len = snprintf( command, sizeof( command ), "ulimit -c 0; \"%s\" --null-id 2>&1", s_executable );
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
        if ( strstr( line, "Assertion failed: id," ) || strstr( line, "Expression: id" ) || strstr( line, "'id'" ) || strstr( line, "`id'" ) ) seen = true;
#ifdef _MSC_VER
    int status = _pclose( pipe );
#else
    int status = pclose( pipe );
#endif
    return status != 0 && seen;
}

UTEST(text_id, null_identifier_reaches_the_shared_id_assertion) {
    ASSERT_TRUE( null_id_asserts() );
}
#endif

UTEST_STATE();

int main( int argc, const char* const argv[] )
{
#ifdef _WIN32
    SetErrorMode( SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX );
#endif
#ifdef _MSC_VER
    _set_error_mode( _OUT_TO_STDERR );
    _set_abort_behavior( 0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT );
    _CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_FILE );
    _CrtSetReportFile( _CRT_ASSERT, _CRTDBG_FILE_STDERR );
#endif
    s_executable = argv[0];
    if ( argc == 2 && strcmp( argv[1], "--null-id" ) == 0 )
    {
        vxui_ctx ctx = make_ctx();
        vxui_frame( &ctx, 1.0f / 60.0f );
        vxui_text( &ctx, nullptr, "A", 24 );
        return 0;
    }
    return utest_main( argc, argv );
}
