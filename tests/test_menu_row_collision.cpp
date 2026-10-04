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
static uint8_t s_other_mem[16 * 1024 * 1024];
static const char* s_executable = nullptr;

static vxui_ctx make_ctx()
{
    vxui_ctx ctx = {};
    vxui_init( &ctx, 1280, 720, s_clay_mem, sizeof( s_clay_mem ) );
    return ctx;
}

static uint32_t row_id( const char* menu, const char* label )
{
    return Clay__HashString( { false, (int32_t) strlen( label ), label }, vxui_hash( menu ) ).id;
}

UTEST(menu_row_collision, known_cross_menu_collision_keeps_its_existing_numeric_identity) {
    ASSERT_EQ( row_id( "menu0", "B" ), (uint32_t) 2726600868u );
    ASSERT_EQ( row_id( "menu0", "B" ), row_id( "menu1", "A" ) );
}

UTEST(menu_row_collision, same_label_in_distinct_menus_keeps_distinct_seeded_ids) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_menu( &ctx, "menu0" );
    vxui_menu_action( &ctx, "Action" );
    vxui_menu_end( &ctx );
    vxui_menu( &ctx, "menu1" );
    vxui_menu_action( &ctx, "Action" );
    vxui_menu_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( ctx.frame_row_count, 2 );
    ASSERT_NE( row_id( "menu0", "Action" ), row_id( "menu1", "Action" ) );
    for ( int i = 0; i < 2; i++ )
    {
        const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( i == 0 ? "menu0" : "menu1", "Action" ) );
        ASSERT_TRUE( c );
        ASSERT_TRUE( c->state & VXUI_DRAW_FOCUSED );
        ASSERT_EQ( c->row_index, 0 );
        ASSERT_EQ( c->focused_row_index, 0 );
    }
}

UTEST(menu_row_collision, distinct_row_kinds_keep_their_seeded_identity_and_skip_mask) {
    vxui_ctx ctx = make_ctx();
    const char* labels[] = { "action", "option", "slider", "section", "label" };
    const char* options[] = { "Off", "On" };
    int index = 0;
    float value = 0.5f;
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_menu( &ctx, "mixed" );
    vxui_menu_action( &ctx, labels[0] );
    vxui_menu_option( &ctx, labels[1], &index, options, 2 );
    vxui_menu_slider( &ctx, labels[2], &value );
    vxui_menu_section( &ctx, labels[3] );
    vxui_menu_label( &ctx, labels[4] );
    vxui_menu_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( ctx.frame_row_count, 5 );
    ASSERT_EQ( ctx.menu_state[0].w, (uint32_t) ( ( 1u << 3 ) | ( 1u << 4 ) ) );
    for ( int i = 0; i < 5; i++ )
    {
        ASSERT_EQ( ctx.frame_row_ids[i], row_id( "mixed", labels[i] ) );
        const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, ctx.frame_row_ids[i] );
        ASSERT_TRUE( c );
        ASSERT_EQ( c->row_index, i );
    }
}

UTEST(menu_row_collision, colliding_rows_in_different_frames_do_not_share_a_frame_registry) {
    vxui_ctx ctx = make_ctx();
    for ( int frame = 0; frame < 4; frame++ )
    {
        vxui_frame( &ctx, 1.0f / 60.0f );
        vxui_menu( &ctx, frame % 2 == 0 ? "menu0" : "menu1" );
        vxui_menu_action( &ctx, frame % 2 == 0 ? "B" : "A" );
        vxui_menu_end( &ctx );
        vxui_draw_list dl = vxui_render( &ctx );
        ASSERT_EQ( ctx.frame_row_count, 1 );
        ASSERT_EQ( ctx.frame_row_ids[0], (uint32_t) 2726600868u );
        const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, ctx.frame_row_ids[0] );
        ASSERT_TRUE( c );
        ASSERT_TRUE( c->state & VXUI_DRAW_FOCUSED );
    }
}

UTEST(menu_row_collision, colliding_rows_in_different_contexts_have_independent_registries) {
    vxui_ctx a = make_ctx();
    vxui_ctx b = {};
    vxui_init( &b, 1280, 720, s_other_mem, sizeof( s_other_mem ) );
    vxui_ctx* contexts[] = { &a, &b };
    for ( int i = 0; i < 2; i++ )
    {
        vxui_frame( contexts[i], 1.0f / 60.0f );
        vxui_menu( contexts[i], i == 0 ? "menu0" : "menu1" );
        vxui_menu_action( contexts[i], i == 0 ? "B" : "A" );
        vxui_menu_end( contexts[i] );
        vxui_render( contexts[i] );
    }
    ASSERT_EQ( a.frame_row_count, 1 );
    ASSERT_EQ( b.frame_row_count, 1 );
    ASSERT_EQ( a.frame_row_ids[0], b.frame_row_ids[0] );
    ASSERT_NE( a.menu_state[0].x, b.menu_state[0].x );
}

UTEST(menu_row_collision, declaration_order_changes_do_not_remount_menu_identity) {
    vxui_ctx ctx = make_ctx();
    const char* names[] = { "left", "right" };
    for ( int frame = 0; frame < 2; frame++ )
    {
        vxui_frame( &ctx, 1.0f / 60.0f );
        if ( frame == 1 ) ctx.menu_state[0].y = 1;
        for ( int j = 0; j < 2; j++ )
        {
            int menu = frame == 0 ? j : 1 - j;
            vxui_menu( &ctx, names[menu] );
            vxui_menu_action( &ctx, "A" );
            vxui_menu_action( &ctx, "B" );
            vxui_menu_end( &ctx );
        }
        vxui_draw_list dl = vxui_render( &ctx );
        ASSERT_EQ( ctx.frame_row_count, 4 );
        for ( int menu = 0; menu < 2; menu++ )
        {
            const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( names[menu], "B" ) );
            ASSERT_TRUE( c );
            ASSERT_EQ( c->focused_row_index, frame == 1 && menu == 0 ? 1 : 0 );
        }
    }
    ASSERT_EQ( ctx.menu_count, 2 );
}

UTEST(menu_row_collision, every_menu_slot_checks_against_rows_of_earlier_menus) {
    vxui_ctx ctx = make_ctx();
    char names[VXUI_MAX_MENUS][32] = {};
    const char* labels[] = { "row.a", "row.b", "row.c", "row.d" };
    vxui_frame( &ctx, 1.0f / 60.0f );
    for ( int i = 0; i < VXUI_MAX_MENUS; i++ )
    {
        snprintf( names[i], sizeof( names[i] ), "row.guard.%d.panel", i );
        vxui_menu( &ctx, names[i] );
        for ( int j = 0; j < 4; j++ ) vxui_menu_action( &ctx, labels[j] );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );
    ASSERT_EQ( ctx.frame_row_count, VXUI_MAX_MENUS * 4 );
    for ( int i = 0; i < ctx.frame_row_count; i++ )
    {
        ASSERT_EQ( ctx.frame_row_ids[i], row_id( names[i / 4], labels[i % 4] ) );
        ASSERT_TRUE( vxui_draw_find( dl, VXUI_DRAW_RECT, ctx.frame_row_ids[i] ) );
    }
}

UTEST(menu_row_collision, every_row_slot_survives_then_a_second_menu_reuses_the_label) {
    vxui_ctx ctx = make_ctx();
    char labels[VXUI_MAX_MENU_ROWS][16] = {};
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_menu( &ctx, "full" );
    for ( int i = 0; i < VXUI_MAX_MENU_ROWS; i++ )
    {
        snprintf( labels[i], sizeof( labels[i] ), "row%d", i );
        vxui_menu_action( &ctx, labels[i] );
    }
    vxui_menu_end( &ctx );
    vxui_menu( &ctx, "other" );
    vxui_menu_action( &ctx, labels[0] );
    vxui_menu_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );
    ASSERT_EQ( ctx.frame_row_count, VXUI_MAX_MENU_ROWS + 1 );
    for ( int i = 0; i < VXUI_MAX_MENU_ROWS; i++ )
        ASSERT_TRUE( vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "full", labels[i] ) ) );
    ASSERT_TRUE( vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "other", labels[0] ) ) );
}

#ifndef NDEBUG
static bool row_asserts( const char* mode, const char* message )
{
    char command[4096];
#ifdef _MSC_VER
    int len = snprintf( command, sizeof( command ), "\"\"%s\" --collision %s 2>&1\"", s_executable, mode );
#else
    int len = snprintf( command, sizeof( command ), "ulimit -c 0; \"%s\" --collision %s 2>&1", s_executable, mode );
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
        if ( strstr( line, message ) ) seen = true;
#ifdef _MSC_VER
    int status = _pclose( pipe );
#else
    int status = pclose( pipe );
#endif
    return status != 0 && seen;
}

UTEST(menu_row_collision, cross_menu_actions_assert_before_clay_reports_duplicate_ids) {
    ASSERT_TRUE( row_asserts( "action", "menu row hash collision" ) );
}

UTEST(menu_row_collision, cross_menu_options_assert_before_clay_reports_duplicate_ids) {
    ASSERT_TRUE( row_asserts( "option", "menu row hash collision" ) );
}

UTEST(menu_row_collision, cross_menu_sliders_assert_before_clay_reports_duplicate_ids) {
    ASSERT_TRUE( row_asserts( "slider", "menu row hash collision" ) );
}

UTEST(menu_row_collision, noninteractive_rows_use_the_same_cross_menu_guard) {
    ASSERT_TRUE( row_asserts( "section", "menu row hash collision" ) );
    ASSERT_TRUE( row_asserts( "label", "menu row hash collision" ) );
}

UTEST(menu_row_collision, same_menu_duplicate_labels_keep_their_existing_assertion) {
    ASSERT_TRUE( row_asserts( "duplicate", "duplicate label in same menu" ) );
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
    if ( argc == 3 && strcmp( argv[1], "--collision" ) == 0 )
    {
        vxui_ctx ctx = make_ctx();
        vxui_frame( &ctx, 1.0f / 60.0f );
        int index = 0;
        float value = 0.5f;
        const char* options[] = { "Off", "On" };
        for ( int i = 0; i < 2; i++ )
        {
            bool duplicate = strcmp( argv[2], "duplicate" ) == 0;
            if ( !duplicate || i == 0 ) vxui_menu( &ctx, i == 0 ? "menu0" : "menu1" );
            const char* label = duplicate || i == 0 ? "B" : "A";
            if ( strcmp( argv[2], "option" ) == 0 ) vxui_menu_option( &ctx, label, &index, options, 2 );
            else if ( strcmp( argv[2], "slider" ) == 0 ) vxui_menu_slider( &ctx, label, &value );
            else if ( strcmp( argv[2], "section" ) == 0 ) vxui_menu_section( &ctx, label );
            else if ( strcmp( argv[2], "label" ) == 0 ) vxui_menu_label( &ctx, label );
            else vxui_menu_action( &ctx, label );
            if ( !duplicate || i == 1 ) vxui_menu_end( &ctx );
        }
        vxui_render( &ctx );
        return 0;
    }
    return utest_main( argc, argv );
}
