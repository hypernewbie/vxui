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

static uint32_t row_id( const char* menu, const char* label )
{
    Clay_String cs = { false, (int32_t) strlen( label ), label };
    return Clay__HashString( cs, vxui_hash( menu ) ).id;
}

UTEST(menu_label_arena, caller_overwrite_does_not_change_label_or_row_id) {
    vxui_ctx ctx = make_ctx();
    char label[] = "Play";
    uint32_t id = row_id( "menu", label );
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_menu( &ctx, "menu" );
    vxui_menu_action( &ctx, label );
    memset( label, 'x', 4 );
    vxui_menu_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* r = vxui_draw_find( dl, VXUI_DRAW_RECT, id );
    const vxui_draw_cmd* t = vxui_draw_nth( dl, VXUI_DRAW_TEXT, 0 );
    ASSERT_TRUE( r && t );
    ASSERT_EQ( t->text, ctx.text_buf );
    ASSERT_EQ( t->text_len, 4 );
    ASSERT_EQ( memcmp( t->text, "Play", 4 ), 0 );
    ASSERT_TRUE( t->userdata == nullptr );
    ASSERT_TRUE( r->state & VXUI_DRAW_FOCUSED );
    ASSERT_EQ( ctx.text_offset, 4 );
}

UTEST(menu_label_arena, every_row_kind_copies_its_label) {
    vxui_ctx ctx = make_ctx();
    char labels[][8] = { "Section", "Action", "Option", "Slider", "Label" };
    const char* options[] = { "" };
    int index = 0;
    float value = 0.5f;
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_menu( &ctx, "menu" );
    vxui_menu_section( &ctx, labels[0] );
    vxui_menu_action( &ctx, labels[1] );
    vxui_menu_option( &ctx, labels[2], &index, options, 1 );
    vxui_menu_slider( &ctx, labels[3], &value );
    vxui_menu_label( &ctx, labels[4] );
    memset( labels, 'x', sizeof( labels ) );
    vxui_menu_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    const char* expected[] = { "Section", "Action", "Option", "Slider", "Label" };
    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 5 );
    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), 5 );
    int offset = 0;
    for ( int i = 0; i < 5; i++ )
    {
        const vxui_draw_cmd* t = vxui_draw_nth( dl, VXUI_DRAW_TEXT, i );
        int len = (int) strlen( expected[i] );
        ASSERT_EQ( t->text, ctx.text_buf + offset );
        ASSERT_EQ( t->text_len, len );
        ASSERT_EQ( memcmp( t->text, expected[i], len ), 0 );
        offset += len;
    }
    ASSERT_EQ( ctx.text_offset, offset );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 1 );
    ASSERT_EQ( ctx.menu_state[0].w, (uint32_t) ( ( 1u << 0 ) | ( 1u << 4 ) ) );
}

UTEST(menu_label_arena, empty_label_keeps_its_interactive_rectangle) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_menu( &ctx, "menu" );
    vxui_menu_action( &ctx, "" );
    vxui_menu_action( &ctx, "Next" );
    vxui_menu_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 2 );
    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), 1 );
    const vxui_draw_cmd* r = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "menu", "" ) );
    const vxui_draw_cmd* t = vxui_draw_nth( dl, VXUI_DRAW_TEXT, 0 );
    ASSERT_TRUE( r && t );
    ASSERT_TRUE( r->state & VXUI_DRAW_FOCUSED );
    ASSERT_EQ( r->row_index, 0 );
    ASSERT_EQ( t->text, ctx.text_buf );
    ASSERT_EQ( memcmp( t->text, "Next", 4 ), 0 );
    ASSERT_EQ( ctx.text_offset, 4 );
}

UTEST(menu_label_arena, exact_capacity_label_is_not_dropped) {
    vxui_ctx ctx = make_ctx();
    char label[VXUI_MAX_TEXT_BYTES + 1];
    memset( label, 'a', VXUI_MAX_TEXT_BYTES );
    label[VXUI_MAX_TEXT_BYTES] = 0;
    uint32_t id = row_id( "menu", label );
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_menu( &ctx, "menu" );
    vxui_menu_action( &ctx, label );
    vxui_menu_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( ctx.text_offset, VXUI_MAX_TEXT_BYTES );
    ASSERT_EQ( dl.count, 2 );
    const vxui_draw_cmd* t = vxui_draw_nth( dl, VXUI_DRAW_TEXT, 0 );
    ASSERT_TRUE( t );
    ASSERT_EQ( t->text, ctx.text_buf );
    ASSERT_EQ( t->text_len, VXUI_MAX_TEXT_BYTES );
    ASSERT_EQ( memcmp( t->text, label, VXUI_MAX_TEXT_BYTES ), 0 );
    ASSERT_TRUE( vxui_draw_find( dl, VXUI_DRAW_RECT, id ) );
}

UTEST(menu_label_arena, multiple_labels_share_the_last_arena_bytes) {
    vxui_ctx ctx = make_ctx();
    char prefix[VXUI_MAX_TEXT_BYTES - 3];
    memset( prefix, 'a', sizeof( prefix ) - 1 );
    prefix[sizeof( prefix ) - 1] = 0;
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_menu( &ctx, "menu" );
    vxui_menu_action( &ctx, prefix );
    vxui_menu_action( &ctx, "Tail" );
    vxui_menu_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( ctx.text_offset, VXUI_MAX_TEXT_BYTES );
    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 2 );
    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), 2 );
    const vxui_draw_cmd* t = vxui_draw_nth( dl, VXUI_DRAW_TEXT, 1 );
    ASSERT_TRUE( t );
    ASSERT_EQ( t->text, ctx.text_buf + VXUI_MAX_TEXT_BYTES - 4 );
    ASSERT_EQ( t->text_len, 4 );
    ASSERT_EQ( memcmp( t->text, "Tail", 4 ), 0 );
}

UTEST(menu_label_arena, empty_label_is_valid_at_full_capacity) {
    vxui_ctx ctx = make_ctx();
    char prefix[VXUI_MAX_TEXT_BYTES + 1];
    memset( prefix, 'a', VXUI_MAX_TEXT_BYTES );
    prefix[VXUI_MAX_TEXT_BYTES] = 0;
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_menu( &ctx, "menu" );
    vxui_menu_action( &ctx, prefix );
    vxui_menu_action( &ctx, "" );
    vxui_menu_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( ctx.text_offset, VXUI_MAX_TEXT_BYTES );
    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 2 );
    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), 1 );
    const vxui_draw_cmd* r = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "menu", "" ) );
    ASSERT_TRUE( r );
    ASSERT_EQ( r->row_index, 1 );
}

UTEST(menu_label_arena, reused_buffer_does_not_retain_the_previous_label) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_menu( &ctx, "menu" );
    vxui_menu_action( &ctx, "Long label" );
    vxui_menu_end( &ctx );
    vxui_render( &ctx );
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_menu( &ctx, "menu" );
    vxui_menu_action( &ctx, "b" );
    vxui_menu_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* t = vxui_draw_nth( dl, VXUI_DRAW_TEXT, 0 );
    ASSERT_TRUE( t );
    ASSERT_EQ( ctx.text_offset, 1 );
    ASSERT_EQ( t->text, ctx.text_buf );
    ASSERT_EQ( t->text_len, 1 );
    ASSERT_EQ( t->text[0], 'b' );
    ASSERT_TRUE( vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "menu", "b" ) ) );
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

UTEST(menu_label_arena, label_overflow_hits_the_checked_arena_assertion) {
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
        char label[VXUI_MAX_TEXT_BYTES + 2];
        memset( label, 'a', VXUI_MAX_TEXT_BYTES + 1 );
        label[VXUI_MAX_TEXT_BYTES + 1] = 0;
        vxui_frame( &ctx, 1.0f / 60.0f );
        vxui_menu( &ctx, "menu" );
        vxui_menu_action( &ctx, label );
        return 0;
    }
    return utest_main( argc, argv );
}
