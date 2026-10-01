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

static void init_ctx( vxui_ctx* ctx )
{
    *ctx = {};
    vxui_init( ctx, 1280, 720, s_clay_mem, sizeof( s_clay_mem ) );
}

static Clay_Dimensions measure_text( Clay_StringSlice s, Clay_TextElementConfig*, void* )
{
    return { (float) s.length, 1.0f };
}

static vxui_draw_list draw_frame( vxui_ctx* ctx, int rects, int texts, bool trailing_clip = false )
{
    assert( rects >= 0 && texts >= 0 && rects + texts <= VXUI_MAX_DRAW_CMDS + 2 );
    char ids[VXUI_MAX_DRAW_CMDS + 2][16] = {};
    vxui_frame( ctx, 1.0f / 60.0f );
    for ( int i = 0; i < rects; i++ )
    {
        snprintf( ids[i], sizeof( ids[i] ), "rect%d", i );
        vxui_rect( ctx, ids[i], { .width = { VXUI_FIXED, 1 }, .height = { VXUI_FIXED, 1 } } );
        vxui_div_end( ctx );
    }
    for ( int i = 0; i < texts; i++ )
    {
        snprintf( ids[rects + i], sizeof( ids[rects + i] ), "text%d", i );
        vxui_text( ctx, ids[rects + i], "x" );
    }
    if ( trailing_clip && vxui_menu( ctx, "clip", true, 1 ) )
        vxui_menu_end( ctx );
    return vxui_render( ctx );
}

#ifndef NDEBUG
static bool overflow_asserts( const char* mode )
{
    char command[4096];
#ifdef _MSC_VER
    int len = snprintf( command, sizeof( command ), "\"\"%s\" %s 2>&1\"", s_executable, mode );
#else
    int len = snprintf( command, sizeof( command ), "ulimit -c 0; \"%s\" %s 2>&1", s_executable, mode );
#endif
    assert( len > 0 && len < (int) sizeof( command ) );
#ifdef _MSC_VER
    FILE* pipe = _popen( command, "r" );
#else
    FILE* pipe = popen( command, "r" );
#endif
    if ( !pipe ) return false;
    char line[512];
    bool seen = false;
    while ( fgets( line, sizeof( line ), pipe ) )
        if ( strstr( line, "draw command capacity exceeded" ) ) seen = true;
#ifdef _MSC_VER
    int status = _pclose( pipe );
#else
    int status = pclose( pipe );
#endif
    return status != 0 && seen;
}
#endif

UTEST(draw_capacity, empty_frame) {
    vxui_ctx ctx; init_ctx( &ctx );
    ASSERT_EQ( draw_frame( &ctx, 0, 0 ).count, 0 );
}

UTEST(draw_capacity, exact_rectangle_capacity) {
    vxui_ctx ctx; init_ctx( &ctx );
    vxui_draw_list dl = draw_frame( &ctx, VXUI_MAX_DRAW_CMDS, 0 );

    ASSERT_EQ( dl.count, VXUI_MAX_DRAW_CMDS );
    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), VXUI_MAX_DRAW_CMDS );
    ASSERT_EQ( Clay_GetCurrentContext()->renderCommands.length, VXUI_MAX_DRAW_CMDS );
    char id[16];
    for ( int i = 0; i < dl.count; i++ )
    {
        snprintf( id, sizeof( id ), "rect%d", i );
        ASSERT_EQ( dl.cmds[i].id, vxui_id( id ) );
        ASSERT_NEAR( dl.cmds[i].rect.z, 1.0f, 0.01f );
        ASSERT_EQ( dl.cmds[i].row_index, -1 );
    }
}

UTEST(draw_capacity, exact_text_capacity) {
    vxui_ctx ctx; init_ctx( &ctx );
    Clay_SetMeasureTextFunction( measure_text, nullptr );
    vxui_draw_list dl = draw_frame( &ctx, 0, VXUI_MAX_DRAW_CMDS );

    ASSERT_EQ( dl.count, VXUI_MAX_DRAW_CMDS );
    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), VXUI_MAX_DRAW_CMDS );
    ASSERT_EQ( Clay_GetCurrentContext()->renderCommands.length, VXUI_MAX_DRAW_CMDS );
    char id[16];
    for ( int i = 0; i < dl.count; i++ )
    {
        snprintf( id, sizeof( id ), "text%d", i );
        ASSERT_EQ( dl.cmds[i].id, vxui_text_id( id ) );
        ASSERT_EQ( dl.cmds[i].text_len, 1 );
        ASSERT_EQ( dl.cmds[i].text[0], 'x' );
    }
}

UTEST(draw_capacity, rectangle_and_text_share_capacity) {
    vxui_ctx ctx; init_ctx( &ctx );
    Clay_SetMeasureTextFunction( measure_text, nullptr );
    vxui_draw_list dl = draw_frame( &ctx, VXUI_MAX_DRAW_CMDS / 2, VXUI_MAX_DRAW_CMDS / 2 );

    ASSERT_EQ( dl.count, VXUI_MAX_DRAW_CMDS );
    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), VXUI_MAX_DRAW_CMDS / 2 );
    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), VXUI_MAX_DRAW_CMDS / 2 );
}

UTEST(draw_capacity, trailing_scissors_do_not_exceed_capacity) {
    vxui_ctx ctx; init_ctx( &ctx );
    vxui_draw_list dl = draw_frame( &ctx, VXUI_MAX_DRAW_CMDS, 0, true );
    Clay_RenderCommandArray& raw = Clay_GetCurrentContext()->renderCommands;

    ASSERT_EQ( dl.count, VXUI_MAX_DRAW_CMDS );
    ASSERT_EQ( raw.length, VXUI_MAX_DRAW_CMDS + 2 );
    ASSERT_EQ( raw.internalArray[VXUI_MAX_DRAW_CMDS].commandType, CLAY_RENDER_COMMAND_TYPE_SCISSOR_START );
    ASSERT_EQ( raw.internalArray[VXUI_MAX_DRAW_CMDS + 1].commandType, CLAY_RENDER_COMMAND_TYPE_SCISSOR_END );
}

UTEST(draw_capacity, next_frame_has_its_own_capacity) {
    vxui_ctx ctx; init_ctx( &ctx );
    ASSERT_EQ( draw_frame( &ctx, VXUI_MAX_DRAW_CMDS, 0 ).count, VXUI_MAX_DRAW_CMDS );
    vxui_draw_list dl = draw_frame( &ctx, 1, 0 );

    ASSERT_EQ( dl.count, 1 );
    ASSERT_EQ( dl.cmds[0].id, vxui_id( "rect0" ) );
}

#ifdef NDEBUG
UTEST(draw_capacity, rectangle_overflow_truncates) {
    vxui_ctx ctx; init_ctx( &ctx );
    vxui_draw_list dl = draw_frame( &ctx, VXUI_MAX_DRAW_CMDS + 1, 0 );

    ASSERT_EQ( Clay_GetCurrentContext()->renderCommands.length, VXUI_MAX_DRAW_CMDS + 1 );
    ASSERT_EQ( dl.count, VXUI_MAX_DRAW_CMDS );
    ASSERT_TRUE( dl.cmds == ctx.draw_buf );
    char kept[16], dropped[16];
    snprintf( kept, sizeof( kept ), "rect%d", VXUI_MAX_DRAW_CMDS - 1 );
    snprintf( dropped, sizeof( dropped ), "rect%d", VXUI_MAX_DRAW_CMDS );
    ASSERT_TRUE( vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( kept ) ) != nullptr );
    ASSERT_TRUE( vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( dropped ) ) == nullptr );
}

UTEST(draw_capacity, text_overflow_truncates) {
    vxui_ctx ctx; init_ctx( &ctx );
    Clay_SetMeasureTextFunction( measure_text, nullptr );
    vxui_draw_list dl = draw_frame( &ctx, 0, VXUI_MAX_DRAW_CMDS + 1 );

    ASSERT_EQ( Clay_GetCurrentContext()->renderCommands.length, VXUI_MAX_DRAW_CMDS + 1 );
    ASSERT_EQ( dl.count, VXUI_MAX_DRAW_CMDS );
    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), VXUI_MAX_DRAW_CMDS );
    char dropped[16];
    snprintf( dropped, sizeof( dropped ), "text%d", VXUI_MAX_DRAW_CMDS );
    ASSERT_TRUE( vxui_draw_find( dl, VXUI_DRAW_TEXT, vxui_text_id( dropped ) ) == nullptr );
}

UTEST(draw_capacity, mixed_overflow_truncates) {
    vxui_ctx ctx; init_ctx( &ctx );
    Clay_SetMeasureTextFunction( measure_text, nullptr );
    vxui_draw_list dl = draw_frame( &ctx, VXUI_MAX_DRAW_CMDS / 2, VXUI_MAX_DRAW_CMDS / 2 + 1 );

    ASSERT_EQ( Clay_GetCurrentContext()->renderCommands.length, VXUI_MAX_DRAW_CMDS + 1 );
    ASSERT_EQ( dl.count, VXUI_MAX_DRAW_CMDS );
    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), VXUI_MAX_DRAW_CMDS / 2 );
    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), VXUI_MAX_DRAW_CMDS / 2 );
}
#else
UTEST(draw_capacity, rectangle_overflow_asserts) {
    ASSERT_TRUE( overflow_asserts( "--overflow-rect" ) );
}

UTEST(draw_capacity, text_overflow_asserts) {
    ASSERT_TRUE( overflow_asserts( "--overflow-text" ) );
}

UTEST(draw_capacity, mixed_overflow_asserts) {
    ASSERT_TRUE( overflow_asserts( "--overflow-mixed" ) );
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
    if ( argc == 2 && strncmp( argv[1], "--overflow-", 11 ) == 0 )
    {
        vxui_ctx ctx; init_ctx( &ctx );
        Clay_SetMeasureTextFunction( measure_text, nullptr );
        if ( strcmp( argv[1], "--overflow-rect" ) == 0 )
            draw_frame( &ctx, VXUI_MAX_DRAW_CMDS + 1, 0 );
        else if ( strcmp( argv[1], "--overflow-text" ) == 0 )
            draw_frame( &ctx, 0, VXUI_MAX_DRAW_CMDS + 1 );
        else if ( strcmp( argv[1], "--overflow-mixed" ) == 0 )
            draw_frame( &ctx, VXUI_MAX_DRAW_CMDS / 2, VXUI_MAX_DRAW_CMDS / 2 + 1 );
        else
            return 1;
        return 0;
    }
    return utest_main( argc, argv );
}
