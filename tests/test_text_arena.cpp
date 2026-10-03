#include "utest.h"
#include "../vxui.h"
#define VXUI_IMPL
#include "../vxui_impl.h"
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

UTEST(text_arena, declaration_copies_caller_text) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    char text[] = "alpha";
    vxui_text( &ctx, "text", text, 24 );
    memset( text, 'x', 5 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 1 );
    ASSERT_EQ( dl.cmds[0].text_len, 5 );
    ASSERT_EQ( dl.cmds[0].text, ctx.text_buf );
    ASSERT_EQ( memcmp( dl.cmds[0].text, "alpha", 5 ), 0 );
    ASSERT_EQ( ctx.text_offset, 5 );
    ASSERT_TRUE( dl.cmds[0].userdata == nullptr );
}

UTEST(text_arena, empty_text_does_not_consume_bytes_or_emit_a_packet) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_text( &ctx, "empty", "", 24 );
    vxui_text( &ctx, "next", "beta", 24 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( ctx.text_offset, 4 );
    ASSERT_EQ( dl.count, 1 );
    ASSERT_EQ( dl.cmds[0].id, vxui_text_id( "next" ) );
    ASSERT_EQ( dl.cmds[0].text, ctx.text_buf );
    ASSERT_EQ( memcmp( dl.cmds[0].text, "beta", 4 ), 0 );
}

UTEST(text_arena, exact_capacity_text_is_not_dropped) {
    vxui_ctx ctx = make_ctx();
    char text[VXUI_MAX_TEXT_BYTES + 1];
    memset( text, 'a', VXUI_MAX_TEXT_BYTES );
    text[VXUI_MAX_TEXT_BYTES] = 0;
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_text( &ctx, "full", text, 24 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( ctx.text_offset, VXUI_MAX_TEXT_BYTES );
    ASSERT_EQ( dl.count, 1 );
    ASSERT_EQ( dl.cmds[0].id, vxui_text_id( "full" ) );
    ASSERT_EQ( dl.cmds[0].text_len, VXUI_MAX_TEXT_BYTES );
    ASSERT_EQ( dl.cmds[0].text, ctx.text_buf );
    ASSERT_EQ( memcmp( dl.cmds[0].text, text, VXUI_MAX_TEXT_BYTES ), 0 );
}

UTEST(text_arena, last_bytes_are_shared_by_multiple_declarations) {
    vxui_ctx ctx = make_ctx();
    char prefix[VXUI_MAX_TEXT_BYTES - 2];
    memset( prefix, 'a', sizeof( prefix ) - 1 );
    prefix[sizeof( prefix ) - 1] = 0;
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_text( &ctx, "prefix", prefix, 24 );
    vxui_text( &ctx, "tail", "end", 24 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( ctx.text_offset, VXUI_MAX_TEXT_BYTES );
    ASSERT_EQ( dl.count, 2 );
    const vxui_draw_cmd* tail = vxui_draw_find( dl, VXUI_DRAW_TEXT, vxui_text_id( "tail" ) );
    ASSERT_TRUE( tail );
    ASSERT_EQ( tail->text, ctx.text_buf + VXUI_MAX_TEXT_BYTES - 3 );
    ASSERT_EQ( tail->text_len, 3 );
    ASSERT_EQ( memcmp( tail->text, "end", 3 ), 0 );
}

UTEST(text_arena, empty_text_is_valid_at_full_capacity) {
    vxui_ctx ctx = make_ctx();
    char text[VXUI_MAX_TEXT_BYTES + 1];
    memset( text, 'a', VXUI_MAX_TEXT_BYTES );
    text[VXUI_MAX_TEXT_BYTES] = 0;
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_text( &ctx, "full", text, 24 );
    vxui_text( &ctx, "empty", "", 24 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( ctx.text_offset, VXUI_MAX_TEXT_BYTES );
    ASSERT_EQ( dl.count, 1 );
    ASSERT_EQ( dl.cmds[0].id, vxui_text_id( "full" ) );
}

UTEST(text_arena, next_frame_reuses_the_buffer_without_retaining_text) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_text( &ctx, "text", "alpha", 24 );
    vxui_render( &ctx );
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_text( &ctx, "text", "b", 24 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( ctx.text_offset, 1 );
    ASSERT_EQ( dl.count, 1 );
    ASSERT_EQ( dl.cmds[0].text, ctx.text_buf );
    ASSERT_EQ( dl.cmds[0].text_len, 1 );
    ASSERT_EQ( dl.cmds[0].text[0], 'b' );
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

UTEST(text_arena, exceeding_capacity_asserts_instead_of_returning_null) {
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
        vxui_text( &ctx, "overflow", text, 24 );
        return 0;
    }
    return utest_main( argc, argv );
}
