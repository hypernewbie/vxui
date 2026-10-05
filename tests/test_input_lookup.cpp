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

static const char* s_actions[] = { "up", "down", "left", "right", "confirm", "cancel" };
static const char* s_executable = nullptr;
static uint8_t s_clay_mem[16 * 1024 * 1024];

UTEST(input_lookup, every_action_maps_to_its_original_bit) {
    for ( int i = 0; i < 6; i++ )
    {
        vxui_ctx ctx = {};
        vxui_input( &ctx, s_actions[i] );
        ASSERT_EQ( ctx.input, 1u << i );
        ASSERT_TRUE( vxui_input_pressed( &ctx, s_actions[i] ) );
        ASSERT_TRUE( vxui_input_just_pressed( &ctx, s_actions[i] ) );
        ASSERT_TRUE( vxui_input_repeated( &ctx, s_actions[i] ) );
    }
}

UTEST(input_lookup, set_and_clear_preserve_all_other_bits) {
    for ( uint32_t mask = 0; mask < 64; mask++ )
        for ( int i = 0; i < 6; i++ )
        {
            vxui_ctx ctx = {};
            ctx.input = mask | 0x80000000u;
            vxui_input( &ctx, s_actions[i], true );
            ASSERT_EQ( ctx.input, mask | 0x80000000u | ( 1u << i ) );
            vxui_input( &ctx, s_actions[i], false );
            ASSERT_EQ( ctx.input, ( mask | 0x80000000u ) & ~( 1u << i ) );
        }
}

UTEST(input_lookup, pressed_and_edge_queries_cover_every_previous_and_current_mask) {
    for ( uint32_t prev = 0; prev < 64; prev++ )
        for ( uint32_t now = 0; now < 64; now++ )
        {
            vxui_ctx ctx = {};
            ctx.input = now;
            ctx.prev_input = prev;
            for ( int i = 0; i < 6; i++ )
            {
                ASSERT_EQ( vxui_input_pressed( &ctx, s_actions[i] ), ( now & ( 1u << i ) ) != 0 );
                ASSERT_EQ( vxui_input_just_pressed( &ctx, s_actions[i] ), ( now & ~prev & ( 1u << i ) ) != 0 );
            }
            ASSERT_FALSE( ctx.inputs_committed );
        }
}

UTEST(input_lookup, unknown_setter_names_keep_the_existing_no_op_contract) {
    vxui_ctx ctx = {};
    ctx.input = 0xa5a5a5a5u;
    const char* names[] = { "", "UP", "confirm ", "jump", "downward" };
    for ( const char* name : names )
    {
        vxui_input( &ctx, name );
        ASSERT_EQ( ctx.input, 0xa5a5a5a5u );
        vxui_input( &ctx, name, false );
        ASSERT_EQ( ctx.input, 0xa5a5a5a5u );
    }
}

UTEST(input_lookup, equal_action_bytes_in_different_storage_map_identically) {
    vxui_ctx ctx = {};
    char action[16] = "confirm";
    vxui_input( &ctx, action );
    ASSERT_TRUE( vxui_input_pressed( &ctx, "confirm" ) );
    ASSERT_TRUE( vxui_input_just_pressed( &ctx, action ) );
    strcpy( action, "cancel" );
    vxui_input( &ctx, action );
    ASSERT_EQ( ctx.input, (uint32_t) ( VXUI_INPUT_CONFIRM | VXUI_INPUT_CANCEL ) );
}

UTEST(input_lookup, repeated_queries_share_one_lazy_commit_for_all_six_actions) {
    vxui_ctx ctx = {};
    ctx.input = 63;
    ctx.prev_input = 63;
    ctx.dt = 0.1f;
    for ( int i = 0; i < 6; i++ ) ctx.input_next_fire[i] = 0.1f;
    for ( int pass = 0; pass < 4; pass++ )
        for ( int i = 0; i < 6; i++ ) ASSERT_TRUE( vxui_input_repeated( &ctx, s_actions[i] ) );
    for ( int i = 0; i < 6; i++ ) ASSERT_EQ( ctx.input_held_time[i], 0.1f );
}

UTEST(input_lookup, query_order_does_not_change_held_or_edge_results) {
    vxui_ctx ctx = {};
    ctx.input = VXUI_INPUT_UP | VXUI_INPUT_CANCEL;
    ctx.prev_input = VXUI_INPUT_UP;
    ctx.input_next_fire[0] = VXUI_INPUT_DELAY;
    ASSERT_TRUE( vxui_input_repeated( &ctx, "cancel" ) );
    ASSERT_TRUE( vxui_input_pressed( &ctx, "up" ) );
    ASSERT_FALSE( vxui_input_just_pressed( &ctx, "up" ) );
    ASSERT_TRUE( vxui_input_just_pressed( &ctx, "cancel" ) );
    ASSERT_EQ( ctx.input_repeated, (uint32_t) VXUI_INPUT_CANCEL );
}

UTEST(input_lookup, frame_lifecycle_still_clears_input_and_carries_previous_bits) {
    vxui_ctx ctx = {};
    vxui_init( &ctx, 640, 480, s_clay_mem, sizeof( s_clay_mem ) );
    vxui_frame( &ctx, 0.01f );
    for ( const char* action : s_actions ) vxui_input( &ctx, action );
    vxui_render( &ctx );
    vxui_frame( &ctx, 0.01f );
    ASSERT_EQ( ctx.input, 0u );
    ASSERT_EQ( ctx.prev_input, 63u );
    for ( const char* action : s_actions ) ASSERT_FALSE( vxui_input_just_pressed( &ctx, action ) );
    vxui_render( &ctx );
}

#ifndef NDEBUG
static bool input_asserts( const char* mode, const char* argument, const char* message )
{
    char command[4096];
#ifdef _MSC_VER
    snprintf( command, sizeof( command ), "\"\"%s\" --bad-input %s %s 2>&1\"", s_executable, mode, argument );
    FILE* pipe = _popen( command, "r" );
#else
    snprintf( command, sizeof( command ), "ulimit -c 0; \"%s\" --bad-input %s %s 2>&1", s_executable, mode, argument );
    FILE* pipe = popen( command, "r" );
#endif
    if ( !pipe ) return false;
    bool seen = false;
    char line[512];
    while ( fgets( line, sizeof( line ), pipe ) )
        if ( strstr( line, message ) && ( strstr( line, "Assertion" ) || strstr( line, "assertion" ) ) ) seen = true;
#ifdef _MSC_VER
    int status = _pclose( pipe );
#else
    int status = pclose( pipe );
#endif
    return status != 0 && seen;
}

UTEST(input_lookup, null_action_reaches_a_precondition_assertion_in_every_entry_point) {
    for ( const char* mode : { "set", "pressed", "edge", "repeat" } )
        ASSERT_TRUE( input_asserts( mode, "null", "action" ) );
}

UTEST(input_lookup, null_context_reaches_a_precondition_assertion_in_every_entry_point) {
    for ( const char* mode : { "set", "pressed", "edge", "repeat" } )
        ASSERT_TRUE( input_asserts( mode, "ctx", "ctx" ) );
}

UTEST(input_lookup, unknown_query_names_keep_their_existing_assertion) {
    for ( const char* mode : { "pressed", "edge", "repeat" } )
        ASSERT_TRUE( input_asserts( mode, "unknown", "invalid input action" ) );
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
    if ( argc == 4 && strcmp( argv[1], "--bad-input" ) == 0 )
    {
        vxui_ctx ctx = {};
        vxui_ctx* p = strcmp( argv[3], "ctx" ) == 0 ? nullptr : &ctx;
        const char* action = strcmp( argv[3], "null" ) == 0 ? nullptr : strcmp( argv[3], "unknown" ) == 0 ? "jump" : "up";
        if ( strcmp( argv[2], "set" ) == 0 ) vxui_input( p, action );
        else if ( strcmp( argv[2], "pressed" ) == 0 ) vxui_input_pressed( p, action );
        else if ( strcmp( argv[2], "edge" ) == 0 ) vxui_input_just_pressed( p, action );
        else vxui_input_repeated( p, action );
        return 0;
    }
    return utest_main( argc, argv );
}
