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

UTEST(root_id, varied_identifiers_keep_public_clay_ids_and_offsets) {
    vxui_ctx ctx = make_ctx();
    const char* names[] = { "root", "Root", "layout.root", "root##one", "two words", "", "caf\xc3\xa9", "a_long_immediate_mode_identifier_with_a_stable_suffix" };
    int n = (int) ( sizeof( names ) / sizeof( names[0] ) );
    char leaves[8][16] = {};
    vxui_frame( &ctx, 1.0f / 60.0f );
    for ( int i = 0; i < n; i++ )
    {
        snprintf( leaves[i], sizeof( leaves[i] ), "leaf%d", i );
        vxui_root( &ctx, names[i], (float) i * 32 + 0.5f, 20.25f );
        vxui_rect( &ctx, leaves[i], { .width = { VXUI_FIXED, 16 }, .height = { VXUI_FIXED, 8 } } );
        vxui_div_end( &ctx );
        vxui_root_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, n );
    for ( int i = 0; i < n; i++ )
    {
        Clay_ElementData e = Clay_GetElementData( clay_id( names[i] ) );
        ASSERT_TRUE( e.found );
        ASSERT_EQ( e.boundingBox.x, (float) i * 32 + 0.5f );
        ASSERT_EQ( e.boundingBox.y, 20.25f );
        ASSERT_EQ( e.boundingBox.width, 16.0f );
        ASSERT_EQ( e.boundingBox.height, 8.0f );
        ASSERT_EQ( dl.cmds[i].id, clay_id( leaves[i] ).id );
        ASSERT_EQ( dl.cmds[i].rect.x, e.boundingBox.x );
    }
}

UTEST(root_id, empty_root_emits_no_packet_or_text_bytes) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_root( &ctx, "empty", 10, 20 );
    vxui_root_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 0 );
    ASSERT_EQ( ctx.text_offset, 0 );
    ASSERT_TRUE( Clay_GetElementData( clay_id( "empty" ) ).found );
}

UTEST(root_id, parent_padding_does_not_enter_root_coordinates_or_identity) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_div( &ctx, "outer", { .width = { VXUI_FIXED, 400 }, .height = { VXUI_FIXED, 200 }, .padding = { 40, 0, 30, 0 } } );
    vxui_root( &ctx, "floating", 200.5f, 100.25f );
    vxui_rect( &ctx, "leaf", { .width = { VXUI_FIXED, 16 }, .height = { VXUI_FIXED, 8 } } );
    vxui_div_end( &ctx );
    vxui_root_end( &ctx );
    vxui_div_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    Clay_ElementData e = Clay_GetElementData( clay_id( "floating" ) );
    ASSERT_TRUE( e.found );
    ASSERT_EQ( dl.count, 1 );
    ASSERT_EQ( dl.cmds[0].id, clay_id( "leaf" ).id );
    ASSERT_EQ( dl.cmds[0].rect.x, 200.5f );
    ASSERT_EQ( dl.cmds[0].rect.y, 100.25f );
    ASSERT_EQ( e.boundingBox.x, 200.5f );
    ASSERT_EQ( e.boundingBox.y, 100.25f );
}

UTEST(root_id, nested_roots_still_attach_to_the_layout_root) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_root( &ctx, "outer", 100, 50 );
    vxui_root( &ctx, "inner", 20.5f, 10.25f );
    vxui_rect( &ctx, "leaf", { .width = { VXUI_FIXED, 16 }, .height = { VXUI_FIXED, 8 } } );
    vxui_div_end( &ctx );
    vxui_root_end( &ctx );
    vxui_root_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_TRUE( Clay_GetElementData( clay_id( "inner" ) ).found );
    ASSERT_EQ( dl.count, 1 );
    ASSERT_EQ( dl.cmds[0].id, clay_id( "leaf" ).id );
    ASSERT_EQ( dl.cmds[0].rect.x, 20.5f );
    ASSERT_EQ( dl.cmds[0].rect.y, 10.25f );
}

UTEST(root_id, movement_changes_geometry_not_ids_or_metadata) {
    vxui_ctx ctx = make_ctx();
    float xs[] = { 10.5f, 41.5f };
    float ys[] = { 20.25f, 7.75f };
    for ( int frame = 0; frame < 2; frame++ )
    {
        vxui_frame( &ctx, 1.0f / 60.0f );
        vxui_root( &ctx, "moving", xs[frame], ys[frame] );
        vxui_rect( &ctx, "leaf", { .width = { VXUI_FIXED, 16 }, .height = { VXUI_FIXED, 8 } } );
        vxui_div_end( &ctx );
        vxui_root_end( &ctx );
        vxui_draw_list dl = vxui_render( &ctx );

        ASSERT_TRUE( Clay_GetElementData( clay_id( "moving" ) ).found );
        ASSERT_EQ( dl.count, 1 );
        ASSERT_EQ( dl.cmds[0].id, clay_id( "leaf" ).id );
        ASSERT_EQ( dl.cmds[0].rect.x, xs[frame] );
        ASSERT_EQ( dl.cmds[0].rect.y, ys[frame] );
        ASSERT_FALSE( dl.cmds[0].clipped );
        ASSERT_TRUE( dl.cmds[0].userdata == nullptr );
        ASSERT_EQ( dl.cmds[0].row_index, -1 );
    }
}

UTEST(root_id, caller_id_overwrite_does_not_change_the_registered_root) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    char id[] = "mutable";
    vxui_root( &ctx, id, 10, 20 );
    memset( id, 'x', 7 );
    vxui_rect( &ctx, "leaf", { .width = { VXUI_FIXED, 16 }, .height = { VXUI_FIXED, 8 } } );
    vxui_div_end( &ctx );
    vxui_root_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_TRUE( Clay_GetElementData( clay_id( "mutable" ) ).found );
    ASSERT_FALSE( Clay_GetElementData( clay_id( "xxxxxxx" ) ).found );
    ASSERT_EQ( dl.count, 1 );
    ASSERT_EQ( dl.cmds[0].rect.x, 10.0f );
    ASSERT_EQ( dl.cmds[0].rect.y, 20.0f );
}

UTEST(root_id, declaration_order_keeps_each_root_with_its_child) {
    vxui_ctx ctx = make_ctx();
    const char* roots[] = { "first_root", "second_root" };
    const char* leaves[] = { "first", "second" };
    for ( int frame = 0; frame < 2; frame++ )
    {
        vxui_frame( &ctx, 1.0f / 60.0f );
        for ( int i = 0; i < 2; i++ )
        {
            int logical = frame == 0 ? i : 1 - i;
            vxui_root( &ctx, roots[logical], (float) ( logical + 1 ) * 100, 20 );
            vxui_rect( &ctx, leaves[logical], { .width = { VXUI_FIXED, 16 }, .height = { VXUI_FIXED, 8 } } );
            vxui_div_end( &ctx );
            vxui_root_end( &ctx );
        }
        vxui_draw_list dl = vxui_render( &ctx );

        ASSERT_EQ( dl.count, 2 );
        for ( int i = 0; i < 2; i++ )
        {
            int logical = frame == 0 ? i : 1 - i;
            ASSERT_TRUE( Clay_GetElementData( clay_id( roots[logical] ) ).found );
            ASSERT_EQ( dl.cmds[i].id, clay_id( leaves[logical] ).id );
            ASSERT_EQ( dl.cmds[i].rect.x, (float) ( logical + 1 ) * 100 );
        }
    }
}

UTEST(root_id, roots_keep_global_ids_at_packet_capacity) {
    vxui_ctx ctx = make_ctx();
    char roots[VXUI_MAX_DRAW_CMDS][16] = {};
    char leaves[VXUI_MAX_DRAW_CMDS][16] = {};
    vxui_frame( &ctx, 1.0f / 60.0f );
    for ( int i = 0; i < VXUI_MAX_DRAW_CMDS; i++ )
    {
        snprintf( roots[i], sizeof( roots[i] ), "root%d", i );
        snprintf( leaves[i], sizeof( leaves[i] ), "leaf%d", i );
        vxui_root( &ctx, roots[i], (float) i + 0.5f, 0 );
        vxui_rect( &ctx, leaves[i], { .width = { VXUI_FIXED, 1 }, .height = { VXUI_FIXED, 1 } } );
        vxui_div_end( &ctx );
        vxui_root_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, VXUI_MAX_DRAW_CMDS );
    for ( int i = 0; i < VXUI_MAX_DRAW_CMDS; i++ )
    {
        ASSERT_TRUE( Clay_GetElementData( clay_id( roots[i] ) ).found );
        ASSERT_EQ( dl.cmds[i].id, clay_id( leaves[i] ).id );
        ASSERT_EQ( dl.cmds[i].rect.x, (float) i + 0.5f );
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

UTEST(root_id, null_identifier_reaches_the_shared_id_assertion) {
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
        vxui_root( &ctx, nullptr, 0, 0 );
        return 0;
    }
    return utest_main( argc, argv );
}
