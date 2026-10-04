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

static vxui_ctx make_ctx()
{
    vxui_ctx ctx = {};
    vxui_init( &ctx, 960, 540, s_clay_mem, sizeof( s_clay_mem ) );
    return ctx;
}

static Clay_ElementId clay_id( const char* name )
{
    return Clay_GetElementId( { false, (int32_t) strlen( name ), name } );
}

static void record_id( const vxui_draw_cmd* cmd, vxui_render_data* out, void* userdata )
{
    (*(int*) userdata)++;
    out->flags     = cmd->id;
    out->params[0] = cmd->rect.z;
}

UTEST(rect_id, varied_identifiers_match_clays_public_ids) {
    vxui_ctx ctx = make_ctx();
    const char* names[] = { "rect", "Rect", "layout.rect", "rect##one", "two words", "", "caf\xc3\xa9", "a_long_immediate_mode_identifier_with_a_stable_suffix" };
    int n = (int) ( sizeof( names ) / sizeof( names[0] ) );
    vxui_frame( &ctx, 1.0f / 60.0f );
    for ( int i = 0; i < n; i++ )
    {
        vxui_rect( &ctx, names[i], { .width = { VXUI_FIXED, 32 }, .height = { VXUI_FIXED, 16 } } );
        vxui_div_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, n );
    for ( int i = 0; i < n; i++ )
    {
        ASSERT_EQ( dl.cmds[i].id, clay_id( names[i] ).id );
        ASSERT_EQ( dl.cmds[i].rect.x, (float) i * 32 );
        ASSERT_EQ( dl.cmds[i].rect.z, 32.0f );
        ASSERT_EQ( dl.cmds[i].rect.w, 16.0f );
    }
}

UTEST(rect_id, geometry_changes_do_not_change_identity_or_add_metadata) {
    vxui_ctx ctx = make_ctx();
    for ( int frame = 0; frame < 2; frame++ )
    {
        float w = frame == 0 ? 64.0f : 80.0f;
        float h = frame == 0 ? 32.0f : 40.0f;
        vxui_frame( &ctx, 1.0f / 60.0f );
        vxui_rect( &ctx, "panel", { .width = { VXUI_FIXED, w }, .height = { VXUI_FIXED, h } } );
        vxui_div_end( &ctx );
        vxui_draw_list dl = vxui_render( &ctx );

        ASSERT_EQ( dl.count, 1 );
        ASSERT_EQ( dl.cmds[0].id, clay_id( "panel" ).id );
        ASSERT_EQ( dl.cmds[0].rect.z, w );
        ASSERT_EQ( dl.cmds[0].rect.w, h );
        ASSERT_EQ( dl.cmds[0].state, (uint8_t) 0 );
        ASSERT_EQ( dl.cmds[0].row_index, -1 );
        ASSERT_EQ( dl.cmds[0].focused_row_index, -1 );
        ASSERT_TRUE( dl.cmds[0].text == nullptr );
        ASSERT_TRUE( dl.cmds[0].userdata == nullptr );
    }
}

UTEST(rect_id, caller_id_overwrite_does_not_change_the_packet_id) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    char id[] = "mutable";
    vxui_rect( &ctx, id, { .width = { VXUI_FIXED, 64 }, .height = { VXUI_FIXED, 32 } } );
    vxui_div_end( &ctx );
    memset( id, 'x', 7 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 1 );
    ASSERT_EQ( dl.cmds[0].id, clay_id( "mutable" ).id );
    ASSERT_NE( dl.cmds[0].id, clay_id( "xxxxxxx" ).id );
}

UTEST(rect_id, nested_rects_keep_global_ids_and_producer_order) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_rect( &ctx, "outer", { .width = { VXUI_FIXED, 100 }, .height = { VXUI_FIXED, 60 }, .padding = { 4, 0, 6, 0 } } );
    vxui_rect( &ctx, "inner", { .width = { VXUI_FIXED, 20 }, .height = { VXUI_FIXED, 12 } } );
    vxui_div_end( &ctx );
    vxui_div_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 2 );
    ASSERT_EQ( dl.cmds[0].id, clay_id( "outer" ).id );
    ASSERT_EQ( dl.cmds[1].id, clay_id( "inner" ).id );
    ASSERT_EQ( dl.cmds[1].rect.x, 4.0f );
    ASSERT_EQ( dl.cmds[1].rect.y, 6.0f );
}

UTEST(rect_id, core_and_hud_rectangles_share_id_conversion) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_rect( &ctx, "shared", { .width = { VXUI_FIXED, 32 }, .height = { VXUI_FIXED, 16 } } );
    vxui_div_end( &ctx );
    vxui_draw_list core = vxui_render( &ctx );
    ASSERT_EQ( core.count, 1 );
    uint32_t id = core.cmds[0].id;
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_image( &hud, "shared", 42, 32, 16 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 1 );
    ASSERT_EQ( dl.cmds[0].id, id );
    ASSERT_EQ( id, clay_id( "shared" ).id );
    ASSERT_EQ( dl.cmds[0].userdata, (const void*) &hud.items[0] );
}

UTEST(rect_id, resolver_sees_the_same_id_and_geometry) {
    vxui_ctx ctx = make_ctx();
    int calls = 0;
    vxui_set_render_data_fn( &ctx, record_id, &calls );
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_rect( &ctx, "panel", { .width = { VXUI_FIXED, 64 }, .height = { VXUI_FIXED, 32 } } );
    vxui_div_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 1 );
    ASSERT_EQ( calls, 1 );
    ASSERT_EQ( dl.cmds[0].render.flags, clay_id( "panel" ).id );
    ASSERT_EQ( dl.cmds[0].render.params[0], 64.0f );
}

UTEST(rect_id, declaration_order_changes_packets_not_identity) {
    vxui_ctx ctx = make_ctx();
    const char* names[] = { "first", "second" };
    for ( int frame = 0; frame < 2; frame++ )
    {
        vxui_frame( &ctx, 1.0f / 60.0f );
        for ( int i = 0; i < 2; i++ )
        {
            vxui_rect( &ctx, names[frame == 0 ? i : 1 - i], { .width = { VXUI_FIXED, 32 }, .height = { VXUI_FIXED, 16 } } );
            vxui_div_end( &ctx );
        }
        vxui_draw_list dl = vxui_render( &ctx );

        ASSERT_EQ( dl.count, 2 );
        for ( int i = 0; i < 2; i++ ) ASSERT_EQ( dl.cmds[i].id, clay_id( names[frame == 0 ? i : 1 - i] ).id );
    }
}

UTEST(rect_id, every_rectangle_keeps_its_id_at_packet_capacity) {
    vxui_ctx ctx = make_ctx();
    char names[VXUI_MAX_DRAW_CMDS][16] = {};
    vxui_frame( &ctx, 1.0f / 60.0f );
    for ( int i = 0; i < VXUI_MAX_DRAW_CMDS; i++ )
    {
        snprintf( names[i], sizeof( names[i] ), "rect%d", i );
        vxui_rect( &ctx, names[i], { .width = { VXUI_FIXED, 1 }, .height = { VXUI_FIXED, 1 } } );
        vxui_div_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, VXUI_MAX_DRAW_CMDS );
    for ( int i = 0; i < VXUI_MAX_DRAW_CMDS; i++ ) ASSERT_EQ( dl.cmds[i].id, clay_id( names[i] ).id );
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

UTEST(rect_id, null_identifier_reaches_the_shared_id_assertion) {
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
        vxui_rect( &ctx, nullptr );
        return 0;
    }
    return utest_main( argc, argv );
}
