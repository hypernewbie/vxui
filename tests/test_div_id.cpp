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

UTEST(div_id, varied_identifiers_match_clays_public_id_lookup) {
    vxui_ctx ctx = make_ctx();
    const char* names[] = { "div", "Div", "layout.div", "div##one", "two words", "", "caf\xc3\xa9", "a_long_immediate_mode_identifier_with_a_stable_suffix" };
    int n = (int) ( sizeof( names ) / sizeof( names[0] ) );
    vxui_frame( &ctx, 1.0f / 60.0f );
    for ( int i = 0; i < n; i++ )
    {
        vxui_div( &ctx, names[i], { .width = { VXUI_FIXED, 64 }, .height = { VXUI_FIXED, 32 } } );
        vxui_div_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 0 );
    for ( int i = 0; i < n; i++ )
    {
        Clay_ElementData e = Clay_GetElementData( clay_id( names[i] ) );
        ASSERT_TRUE( e.found );
        ASSERT_EQ( e.boundingBox.x, (float) i * 64 );
        ASSERT_EQ( e.boundingBox.width, 64.0f );
        ASSERT_EQ( e.boundingBox.height, 32.0f );
        ASSERT_EQ( vxui_id( names[i] ), clay_id( names[i] ).id );
    }
}

UTEST(div_id, container_emits_neither_packet_nor_text_bytes) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_div( &ctx, "invisible", { .width = { VXUI_FIXED, 64 }, .height = { VXUI_FIXED, 32 } } );
    vxui_div_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 0 );
    ASSERT_EQ( ctx.text_offset, 0 );
    ASSERT_TRUE( Clay_GetElementData( clay_id( "invisible" ) ).found );
}

UTEST(div_id, nested_containers_keep_global_ids_and_child_geometry) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_div( &ctx, "outer", { .width = { VXUI_FIXED, 200 }, .height = { VXUI_FIXED, 100 }, .col = true, .padding = { 3, 5, 7, 11 } } );
    vxui_div( &ctx, "inner", { .width = { VXUI_FIXED, 80 }, .height = { VXUI_FIXED, 40 } } );
    vxui_rect( &ctx, "leaf", { .width = { VXUI_FIXED, 20 }, .height = { VXUI_FIXED, 12 } } );
    vxui_div_end( &ctx );
    vxui_div_end( &ctx );
    vxui_div_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    Clay_ElementData inner = Clay_GetElementData( clay_id( "inner" ) );
    ASSERT_TRUE( inner.found );
    ASSERT_EQ( inner.boundingBox.x, 3.0f );
    ASSERT_EQ( inner.boundingBox.y, 7.0f );
    ASSERT_EQ( dl.count, 1 );
    ASSERT_EQ( dl.cmds[0].id, clay_id( "leaf" ).id );
    ASSERT_EQ( dl.cmds[0].rect.x, 3.0f );
    ASSERT_EQ( dl.cmds[0].rect.y, 7.0f );
}

UTEST(div_id, row_and_column_configuration_do_not_change_identity) {
    vxui_ctx ctx = make_ctx();
    for ( int frame = 0; frame < 2; frame++ )
    {
        vxui_frame( &ctx, 1.0f / 60.0f );
        vxui_div( &ctx, "layout", { .width = { VXUI_FIXED, 120 }, .height = { VXUI_FIXED, 64 }, .col = frame != 0, .padding = { 5, 7, 3, 9 }, .gap = 6 } );
        vxui_rect( &ctx, "a", { .width = { VXUI_FIXED, 20 }, .height = { VXUI_FIXED, 10 } } );
        vxui_div_end( &ctx );
        vxui_rect( &ctx, "b", { .width = { VXUI_FIXED, 20 }, .height = { VXUI_FIXED, 10 } } );
        vxui_div_end( &ctx );
        vxui_div_end( &ctx );
        vxui_draw_list dl = vxui_render( &ctx );

        ASSERT_TRUE( Clay_GetElementData( clay_id( "layout" ) ).found );
        ASSERT_EQ( dl.count, 2 );
        ASSERT_EQ( dl.cmds[1].id, clay_id( "b" ).id );
        ASSERT_EQ( dl.cmds[1].rect.x, frame == 0 ? 31.0f : 5.0f );
        ASSERT_EQ( dl.cmds[1].rect.y, frame == 0 ? 3.0f : 19.0f );
    }
}

UTEST(div_id, caller_id_overwrite_does_not_change_the_declared_hash) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    char id[] = "mutable";
    vxui_div( &ctx, id, { .width = { VXUI_FIXED, 64 }, .height = { VXUI_FIXED, 32 } } );
    vxui_div_end( &ctx );
    memset( id, 'x', 7 );
    vxui_render( &ctx );

    ASSERT_TRUE( Clay_GetElementData( clay_id( "mutable" ) ).found );
    ASSERT_FALSE( Clay_GetElementData( clay_id( "xxxxxxx" ) ).found );
}

UTEST(div_id, declaration_order_changes_positions_not_ids) {
    vxui_ctx ctx = make_ctx();
    const char* names[] = { "first", "second" };
    for ( int frame = 0; frame < 2; frame++ )
    {
        vxui_frame( &ctx, 1.0f / 60.0f );
        for ( int i = 0; i < 2; i++ )
        {
            vxui_div( &ctx, names[frame == 0 ? i : 1 - i], { .width = { VXUI_FIXED, 64 }, .height = { VXUI_FIXED, 32 } } );
            vxui_div_end( &ctx );
        }
        vxui_render( &ctx );

        for ( int i = 0; i < 2; i++ )
        {
            Clay_ElementData e = Clay_GetElementData( clay_id( names[i] ) );
            ASSERT_TRUE( e.found );
            ASSERT_EQ( e.boundingBox.x, (float) ( frame == 0 ? i : 1 - i ) * 64 );
        }
    }
}

UTEST(div_id, menu_composite_ids_and_navigation_are_unchanged) {
    vxui_ctx ctx = make_ctx();
    for ( int frame = 0; frame < 2; frame++ )
    {
        vxui_frame( &ctx, 1.0f / 60.0f );
        ctx.input = frame == 0 ? 0 : VXUI_INPUT_DOWN;
        vxui_div( &ctx, "container", { .col = true } );
        vxui_menu( &ctx, "menu" );
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_action( &ctx, "Quit" );
        vxui_menu_end( &ctx );
        vxui_div_end( &ctx );
        vxui_draw_list dl = vxui_render( &ctx );

        ASSERT_TRUE( Clay_GetElementData( clay_id( "container" ) ).found );
        Clay_ElementId row = Clay__HashString( CLAY_STRING( "Quit" ), vxui_hash( "menu" ) );
        const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, row.id );
        ASSERT_TRUE( c );
        ASSERT_EQ( c->row_index, 1 );
        ASSERT_EQ( c->focused_row_index, frame );
        ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) frame );
    }
}

UTEST(div_id, containers_keep_child_ids_at_packet_capacity) {
    vxui_ctx ctx = make_ctx();
    char divs[VXUI_MAX_DRAW_CMDS][16] = {};
    char leaves[VXUI_MAX_DRAW_CMDS][16] = {};
    vxui_frame( &ctx, 1.0f / 60.0f );
    for ( int i = 0; i < VXUI_MAX_DRAW_CMDS; i++ )
    {
        snprintf( divs[i], sizeof( divs[i] ), "div%d", i );
        snprintf( leaves[i], sizeof( leaves[i] ), "leaf%d", i );
        vxui_div( &ctx, divs[i] );
        vxui_rect( &ctx, leaves[i], { .width = { VXUI_FIXED, 1 }, .height = { VXUI_FIXED, 1 } } );
        vxui_div_end( &ctx );
        vxui_div_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, VXUI_MAX_DRAW_CMDS );
    for ( int i = 0; i < VXUI_MAX_DRAW_CMDS; i++ )
    {
        ASSERT_TRUE( Clay_GetElementData( clay_id( divs[i] ) ).found );
        ASSERT_EQ( dl.cmds[i].id, clay_id( leaves[i] ).id );
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

UTEST(div_id, null_identifier_reaches_the_shared_id_assertion) {
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
        vxui_div( &ctx, nullptr );
        return 0;
    }
    return utest_main( argc, argv );
}
