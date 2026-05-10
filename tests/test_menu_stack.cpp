#include "utest.h"
#include "../vxui.h"
#define VXUI_IMPL
#include "../vxui_impl.h"
#include <cstring>

static uint8_t s_clay_mem[16 * 1024 * 1024];

// User-owned screen stack. VXUI does not model screens; the demo and tests
// drive vxui_menu by name and decide push/pop policy here.
struct stack_state
{
    const char* stack[8]    = { "main" };
    int         depth       = 1;
    float       volume      = 0.5f;
    int         resolution  = 1;
    int         vsync       = 1;
};

static void stack_push( stack_state* s, const char* name )
{
    assert( s && s->depth < 8 );
    s->stack[s->depth++] = name;
}

static void stack_pop( stack_state* s )
{
    assert( s && s->depth >= 1 );
    if ( s->depth > 1 ) s->depth--;
}

static void init_ctx( vxui_ctx* ctx )
{
    *ctx = {};
    vxui_init( ctx, 1280, 720, s_clay_mem, sizeof( s_clay_mem ) );
}

static int find_menu( vxui_ctx* ctx, const char* name )
{
    uint32_t h = vxui_hash( name );
    for ( int i = 0; i < ctx->menu_count; i++ )
        if ( ctx->menu_state[i].x == h ) return i;
    return -1;
}

static const char* s_resolutions[] = { "1280x720", "1920x1080", "2560x1440" };
static const char* s_on_off[]      = { "Off", "On" };

// Same shape as the demo: dispatch on stack top, declare the matching menu.
// Input is held semantics (caller releases by passing 0), matching the
// cancel_frame pattern in test_menu_input_edge.cpp.
static void stack_frame( vxui_ctx* ctx, stack_state* s, uint32_t input )
{
    vxui_frame( ctx, 1.0f / 60.0f );
    ctx->input = input;
    const char* active = s->stack[s->depth - 1];
    if ( strcmp( active, "main" ) == 0 )
    {
        if ( vxui_menu( ctx, "main" ) )
        {
            vxui_menu_action( ctx, "Play" );
            if ( vxui_menu_action( ctx, "Options" ) ) stack_push( s, "options" );
            vxui_menu_action( ctx, "Quit" );
            vxui_menu_end( ctx );
        }
    }
    else if ( strcmp( active, "options" ) == 0 )
    {
        if ( vxui_menu( ctx, "options" ) )
        {
            vxui_menu_section( ctx, "AUDIO" );
            vxui_menu_slider ( ctx, "Volume", &s->volume );
            vxui_menu_section( ctx, "DISPLAY" );
            vxui_menu_option ( ctx, "Resolution", &s->resolution, s_resolutions, 3 );
            vxui_menu_option ( ctx, "Vsync",      &s->vsync,      s_on_off,      2 );
            if ( vxui_menu_action( ctx, "Back" ) ) stack_pop( s );
            if ( vxui_menu_cancelled( ctx ) )      stack_pop( s );
            vxui_menu_end( ctx );
        }
    }
    vxui_render( ctx );
}

UTEST(menu_stack, starts_at_main) {
    stack_state s;
    ASSERT_EQ   ( s.depth, 1 );
    ASSERT_STREQ( s.stack[0], "main" );
}

UTEST(menu_stack, confirm_options_pushes_options) {
    vxui_ctx ctx;
    init_ctx( &ctx );
    stack_state s;

    stack_frame( &ctx, &s, 0 );                      // establish main
    stack_frame( &ctx, &s, VXUI_INPUT_DOWN );        // focus -> Options
    stack_frame( &ctx, &s, 0 );                      // release down
    stack_frame( &ctx, &s, VXUI_INPUT_CONFIRM );     // confirm fires push

    ASSERT_EQ   ( s.depth, 2 );
    ASSERT_STREQ( s.stack[1], "options" );
}

UTEST(menu_stack, cancel_in_options_pops) {
    vxui_ctx ctx;
    init_ctx( &ctx );
    stack_state s;
    stack_push( &s, "options" );

    stack_frame( &ctx, &s, 0 );                      // establish options
    stack_frame( &ctx, &s, VXUI_INPUT_CANCEL );      // pops

    ASSERT_EQ   ( s.depth, 1 );
    ASSERT_STREQ( s.stack[0], "main" );
}

UTEST(menu_stack, back_action_in_options_pops) {
    vxui_ctx ctx;
    init_ctx( &ctx );
    stack_state s;
    stack_push( &s, "options" );

    // Options interactive rows: Volume(1), Resolution(3), Vsync(4), Back(5).
    // Initial focus auto-promotes to row 1; navigate down 3 times to reach Back.
    stack_frame( &ctx, &s, 0 );                      // establish, focus -> Volume
    stack_frame( &ctx, &s, VXUI_INPUT_DOWN );        // -> Resolution
    stack_frame( &ctx, &s, 0 );
    stack_frame( &ctx, &s, VXUI_INPUT_DOWN );        // -> Vsync
    stack_frame( &ctx, &s, 0 );
    stack_frame( &ctx, &s, VXUI_INPUT_DOWN );        // -> Back
    stack_frame( &ctx, &s, 0 );
    stack_frame( &ctx, &s, VXUI_INPUT_CONFIRM );     // confirm fires Back

    ASSERT_EQ( s.depth, 1 );
}

UTEST(menu_stack, cancel_held_pop_then_root_no_op) {
    vxui_ctx ctx;
    init_ctx( &ctx );
    stack_state s;
    stack_push( &s, "options" );

    stack_frame( &ctx, &s, 0 );                      // establish options
    stack_frame( &ctx, &s, VXUI_INPUT_CANCEL );      // pops once
    ASSERT_EQ( s.depth, 1 );

    // Hold cancel through 30 frames. main is root and has no cancel handler.
    for ( int i = 0; i < 30; i++ )
        stack_frame( &ctx, &s, VXUI_INPUT_CANCEL );
    ASSERT_EQ( s.depth, 1 );
}

UTEST(menu_stack, cancel_re_press_pops_again) {
    vxui_ctx ctx;
    init_ctx( &ctx );
    stack_state s;
    stack_push( &s, "options" );

    stack_frame( &ctx, &s, 0 );
    stack_frame( &ctx, &s, VXUI_INPUT_CANCEL );      // pops to main
    ASSERT_EQ( s.depth, 1 );

    stack_frame( &ctx, &s, 0 );                      // release in main
    stack_push( &s, "options" );
    stack_frame( &ctx, &s, 0 );                      // re-establish options
    stack_frame( &ctx, &s, VXUI_INPUT_CANCEL );      // pops again

    ASSERT_EQ( s.depth, 1 );
}

UTEST(menu_stack, cancel_at_root_no_op) {
    vxui_ctx ctx;
    init_ctx( &ctx );
    stack_state s;

    stack_frame( &ctx, &s, 0 );
    stack_frame( &ctx, &s, VXUI_INPUT_CANCEL );

    ASSERT_EQ   ( s.depth, 1 );
    ASSERT_STREQ( s.stack[0], "main" );
}

UTEST(menu_stack, main_focus_preserved_across_options_visit) {
    vxui_ctx ctx;
    init_ctx( &ctx );
    stack_state s;

    stack_frame( &ctx, &s, 0 );
    stack_frame( &ctx, &s, VXUI_INPUT_DOWN );        // main focus -> Options (row 1)

    stack_push( &s, "options" );
    stack_frame( &ctx, &s, 0 );                      // options visible, main unmounted
    stack_frame( &ctx, &s, 0 );

    stack_pop( &s );
    stack_frame( &ctx, &s, 0 );                      // main remounts

    int main_idx = find_menu( &ctx, "main" );
    ASSERT_GE( main_idx, 0 );
    ASSERT_EQ( ctx.menu_state[main_idx].y, (uint32_t) 1 );
}

UTEST(menu_stack, options_focus_starts_first_interactive_row) {
    vxui_ctx ctx;
    init_ctx( &ctx );
    stack_state s;
    stack_push( &s, "options" );

    stack_frame( &ctx, &s, 0 );                      // establish; focus auto-promotes past skip

    int opts_idx = find_menu( &ctx, "options" );
    ASSERT_GE( opts_idx, 0 );
    ASSERT_EQ( ctx.menu_state[opts_idx].y, (uint32_t) 1 );
}

UTEST(menu_stack, options_focus_preserved_across_main_visit_and_back) {
    vxui_ctx ctx;
    init_ctx( &ctx );
    stack_state s;

    stack_frame( &ctx, &s, 0 );                      // main establish
    stack_push( &s, "options" );
    stack_frame( &ctx, &s, 0 );                      // options establish, focus -> Volume(1)
    stack_frame( &ctx, &s, VXUI_INPUT_DOWN );        // -> Resolution(3)

    stack_pop( &s );
    stack_frame( &ctx, &s, 0 );                      // back to main

    stack_push( &s, "options" );
    stack_frame( &ctx, &s, 0 );                      // re-enter options

    int opts_idx = find_menu( &ctx, "options" );
    ASSERT_GE( opts_idx, 0 );
    ASSERT_EQ( ctx.menu_state[opts_idx].y, (uint32_t) 3 );
}

UTEST_MAIN();
