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

static bool option_frame( vxui_ctx* ctx, int* index, const char** options, int count, uint32_t input )
{
    vxui_frame( ctx, 1.0f / 60.0f );
    ctx->input = input;
    vxui_menu( ctx, "menu" );
    bool changed = vxui_menu_option( ctx, "Mode", index, options, count );
    vxui_menu_end( ctx );
    vxui_render( ctx );
    return changed;
}

UTEST(menu_value_arena, value_copy_survives_caller_overwrite) {
    vxui_ctx ctx = make_ctx();
    char value[] = "High";
    const char* options[] = { value };
    int index = 0;
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_menu( &ctx, "menu" );
    ASSERT_FALSE( vxui_menu_option( &ctx, "Mode", &index, options, 1 ) );
    memset( value, 'x', 4 );
    vxui_menu_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 3 );
    const vxui_draw_cmd* t = vxui_draw_nth( dl, VXUI_DRAW_TEXT, 1 );
    ASSERT_TRUE( t );
    ASSERT_EQ( t->text, ctx.text_buf + 4 );
    ASSERT_EQ( t->text_len, 4 );
    ASSERT_EQ( memcmp( t->text, "High", 4 ), 0 );
    ASSERT_TRUE( t->userdata == nullptr );
    ASSERT_EQ( ctx.text_offset, 8 );
}

UTEST(menu_value_arena, empty_value_emits_only_the_label) {
    vxui_ctx ctx = make_ctx();
    const char* options[] = { "" };
    int index = 0;
    ASSERT_FALSE( option_frame( &ctx, &index, options, 1, 0 ) );

    ASSERT_EQ( ctx.text_offset, 4 );
    ASSERT_EQ( vxui_draw_count( ctx.draw_list, VXUI_DRAW_RECT ), 1 );
    ASSERT_EQ( vxui_draw_count( ctx.draw_list, VXUI_DRAW_TEXT ), 1 );
    const vxui_draw_cmd* t = vxui_draw_nth( ctx.draw_list, VXUI_DRAW_TEXT, 0 );
    ASSERT_EQ( t->text_len, 4 );
    ASSERT_EQ( memcmp( t->text, "Mode", 4 ), 0 );
}

UTEST(menu_value_arena, null_value_stays_optional) {
    vxui_ctx ctx = make_ctx();
    const char* options[] = { nullptr };
    int index = 0;
    ASSERT_FALSE( option_frame( &ctx, &index, options, 1, 0 ) );

    ASSERT_EQ( ctx.text_offset, 4 );
    ASSERT_EQ( vxui_draw_count( ctx.draw_list, VXUI_DRAW_RECT ), 1 );
    ASSERT_EQ( vxui_draw_count( ctx.draw_list, VXUI_DRAW_TEXT ), 1 );
    ASSERT_EQ( index, 0 );
}

UTEST(menu_value_arena, label_and_value_can_fill_the_arena_exactly) {
    vxui_ctx ctx = make_ctx();
    char value[VXUI_MAX_TEXT_BYTES - 3];
    memset( value, 'a', sizeof( value ) - 1 );
    value[sizeof( value ) - 1] = 0;
    const char* options[] = { value };
    int index = 0;
    option_frame( &ctx, &index, options, 1, 0 );

    ASSERT_EQ( ctx.text_offset, VXUI_MAX_TEXT_BYTES );
    ASSERT_EQ( ctx.draw_list.count, 3 );
    const vxui_draw_cmd* t = vxui_draw_nth( ctx.draw_list, VXUI_DRAW_TEXT, 1 );
    ASSERT_TRUE( t );
    ASSERT_EQ( t->text, ctx.text_buf + 4 );
    ASSERT_EQ( t->text_len, VXUI_MAX_TEXT_BYTES - 4 );
    ASSERT_EQ( memcmp( t->text, value, t->text_len ), 0 );
}

UTEST(menu_value_arena, multiple_values_share_the_final_arena_bytes) {
    vxui_ctx ctx = make_ctx();
    char value[VXUI_MAX_TEXT_BYTES - 11];
    memset( value, 'a', sizeof( value ) - 1 );
    value[sizeof( value ) - 1] = 0;
    const char* first[] = { value };
    const char* last[] = { "end" };
    int index = 0;
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_menu( &ctx, "menu" );
    vxui_menu_option( &ctx, "First", &index, first, 1 );
    vxui_menu_option( &ctx, "Last", &index, last, 1 );
    vxui_menu_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( ctx.text_offset, VXUI_MAX_TEXT_BYTES );
    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 2 );
    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), 4 );
    const vxui_draw_cmd* t = vxui_draw_nth( dl, VXUI_DRAW_TEXT, 3 );
    ASSERT_TRUE( t );
    ASSERT_EQ( t->text, ctx.text_buf + VXUI_MAX_TEXT_BYTES - 3 );
    ASSERT_EQ( t->text_len, 3 );
    ASSERT_EQ( memcmp( t->text, "end", 3 ), 0 );
}

UTEST(menu_value_arena, optional_values_do_not_push_past_full_capacity) {
    const char* values[] = { "", nullptr };
    for ( int i = 0; i < 2; i++ )
    {
        vxui_ctx ctx = make_ctx();
        char prefix[VXUI_MAX_TEXT_BYTES - 3];
        memset( prefix, 'a', sizeof( prefix ) - 1 );
        prefix[sizeof( prefix ) - 1] = 0;
        int index = 0;
        vxui_frame( &ctx, 1.0f / 60.0f );
        vxui_text( &ctx, "prefix", prefix, 24 );
        vxui_menu( &ctx, "menu" );
        vxui_menu_option( &ctx, "Mode", &index, &values[i], 1 );
        vxui_menu_end( &ctx );
        vxui_draw_list dl = vxui_render( &ctx );

        ASSERT_EQ( ctx.text_offset, VXUI_MAX_TEXT_BYTES );
        ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 1 );
        ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), 2 );
        const vxui_draw_cmd* t = vxui_draw_nth( dl, VXUI_DRAW_TEXT, 1 );
        ASSERT_EQ( t->text, ctx.text_buf + VXUI_MAX_TEXT_BYTES - 4 );
        ASSERT_EQ( memcmp( t->text, "Mode", 4 ), 0 );
    }
}

UTEST(menu_value_arena, value_presence_changes_without_remounting_the_row) {
    vxui_ctx ctx = make_ctx();
    const char* options[] = { "", "High", "b" };
    int index = 0;
    option_frame( &ctx, &index, options, 3, 0 );
    const vxui_draw_cmd* r = vxui_draw_nth( ctx.draw_list, VXUI_DRAW_RECT, 0 );
    ASSERT_TRUE( r );
    uint32_t id = r->id;

    ASSERT_TRUE( option_frame( &ctx, &index, options, 3, VXUI_INPUT_RIGHT ) );
    ASSERT_EQ( index, 1 );
    ASSERT_EQ( ctx.text_offset, 8 );
    ASSERT_EQ( vxui_draw_count( ctx.draw_list, VXUI_DRAW_TEXT ), 2 );
    option_frame( &ctx, &index, options, 3, 0 );
    ASSERT_TRUE( option_frame( &ctx, &index, options, 3, VXUI_INPUT_RIGHT ) );
    ASSERT_EQ( index, 2 );
    ASSERT_EQ( ctx.text_offset, 5 );
    const vxui_draw_cmd* t = vxui_draw_nth( ctx.draw_list, VXUI_DRAW_TEXT, 1 );
    ASSERT_EQ( t->text, ctx.text_buf + 4 );
    ASSERT_EQ( t->text_len, 1 );
    ASSERT_EQ( t->text[0], 'b' );
    option_frame( &ctx, &index, options, 3, 0 );
    ASSERT_TRUE( option_frame( &ctx, &index, options, 3, VXUI_INPUT_RIGHT ) );
    ASSERT_EQ( index, 0 );
    ASSERT_EQ( ctx.text_offset, 4 );
    ASSERT_EQ( vxui_draw_count( ctx.draw_list, VXUI_DRAW_TEXT ), 1 );
    r = vxui_draw_nth( ctx.draw_list, VXUI_DRAW_RECT, 0 );
    ASSERT_EQ( r->id, id );
    ASSERT_TRUE( r->state & VXUI_DRAW_FOCUSED );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 0 );
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

UTEST(menu_value_arena, value_overflow_hits_the_checked_arena_assertion) {
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
        char value[VXUI_MAX_TEXT_BYTES - 2];
        memset( value, 'a', sizeof( value ) - 1 );
        value[sizeof( value ) - 1] = 0;
        const char* options[] = { value };
        int index = 0;
        option_frame( &ctx, &index, options, 1, 0 );
        return 0;
    }
    return utest_main( argc, argv );
}
