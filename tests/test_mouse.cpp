#include "utest.h"
#include "../vxui.h"
#define VXUI_IMPL
#include "../vxui_impl.h"
#include <cfloat>
#include <cstdio>
#include <filesystem>
#include <vector>

static uint8_t s_clay_mem[16 * 1024 * 1024];

static const std::vector< uint8_t >& roboto_bytes()
{
    static std::vector< uint8_t > bytes = []() {
        std::filesystem::path p = std::filesystem::path( __FILE__ ).parent_path().parent_path() / "fonts" / "Roboto-Regular.ttf";
        FILE* f = fopen( p.string().c_str(), "rb" );
        assert( f && "roboto_bytes: failed to open font file" );
        fseek( f, 0, SEEK_END );
        long sz = ftell( f );
        fseek( f, 0, SEEK_SET );
        std::vector< uint8_t > buf( (size_t) sz );
        size_t got = fread( buf.data(), 1, (size_t) sz, f );
        (void) got;
        fclose( f );
        return buf;
    }();
    return bytes;
}

// Init in place (don't return ctx by value): Clay's measure callback userData is
// the address passed to vxui_init, so the ctx must outlive that registration.
static void init_ctx( vxui_ctx* ctx )
{
    *ctx = {};
    vxui_init( ctx, 1280, 720, s_clay_mem, sizeof( s_clay_mem ) );
}

static void init_ctx_with_font( vxui_ctx* ctx )
{
    init_ctx( ctx );
    const std::vector< uint8_t >& b = roboto_bytes();
    vxui_load_font( ctx, b.data(), b.size(), (float) VXUI_FONT_SIZE_DEFAULT );
}

static uint32_t elem_id( const char* name )
{
    Clay_String cs = { false, (int32_t) strlen( name ), name };
    return Clay__HashString( cs, 0 ).id;
}

static uint32_t row_id( const char* menu, const char* label )
{
    Clay_String cs = { false, (int32_t) strlen( label ), label };
    return Clay__HashString( cs, vxui_hash( menu ) ).id;
}

// Two-action test menu, captures fire results. Mouse pos+buttons are explicit
// helper params so callers can see exactly what state they pushed.
struct menu_fire { bool play; bool quit; };
static menu_fire menu_frame( vxui_ctx* ctx, float mx, float my, uint32_t mb )
{
    vxui_frame( ctx, 1.0f / 60.0f );
    vxui_mouse( ctx, mx, my, mb );
    menu_fire r = { false, false };
    if ( vxui_menu( ctx, "test" ) )
    {
        if ( vxui_menu_action( ctx, "Play" ) ) r.play = true;
        if ( vxui_menu_action( ctx, "Quit" ) ) r.quit = true;
        vxui_menu_end( ctx );
    }
    vxui_render( ctx );
    return r;
}

UTEST(mouse, default_sentinel_no_hover) {
    vxui_ctx ctx;
    init_ctx( &ctx );

    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_rect( &ctx, "panel", { .width = { VXUI_FIXED, 200 }, .height = { VXUI_FIXED, 100 } } );
    vxui_div_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, elem_id( "panel" ) );
    ASSERT_TRUE( c != nullptr );
    ASSERT_EQ( c->state & VXUI_DRAW_HOVERED, (uint8_t) 0 );
}

UTEST(mouse, inside_rect_hovered) {
    vxui_ctx ctx;
    init_ctx( &ctx );

    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_mouse( &ctx, 50.0f, 50.0f, 0 );
    vxui_rect( &ctx, "panel", { .width = { VXUI_FIXED, 200 }, .height = { VXUI_FIXED, 100 } } );
    vxui_div_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, elem_id( "panel" ) );
    ASSERT_TRUE( c != nullptr );
    ASSERT_TRUE( ( c->state & VXUI_DRAW_HOVERED ) != 0 );
}

UTEST(mouse, outside_rect_no_hover) {
    vxui_ctx ctx;
    init_ctx( &ctx );

    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_mouse( &ctx, 500.0f, 500.0f, 0 );
    vxui_rect( &ctx, "panel", { .width = { VXUI_FIXED, 200 }, .height = { VXUI_FIXED, 100 } } );
    vxui_div_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, elem_id( "panel" ) );
    ASSERT_TRUE( c != nullptr );
    ASSERT_EQ( c->state & VXUI_DRAW_HOVERED, (uint8_t) 0 );
}

UTEST(mouse, nested_rects_all_get_hovered) {
    vxui_ctx ctx;
    init_ctx( &ctx );

    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_mouse( &ctx, 30.0f, 30.0f, 0 );
    vxui_rect( &ctx, "outer", { .width = { VXUI_FIXED, 200 }, .height = { VXUI_FIXED, 200 }, .padding = { 10, 10, 10, 10 } } );
    vxui_rect( &ctx, "inner", { .width = { VXUI_FIXED, 100 }, .height = { VXUI_FIXED, 100 } } );
    vxui_div_end( &ctx );
    vxui_div_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* outer = vxui_draw_find( dl, VXUI_DRAW_RECT, elem_id( "outer" ) );
    const vxui_draw_cmd* inner = vxui_draw_find( dl, VXUI_DRAW_RECT, elem_id( "inner" ) );
    ASSERT_TRUE( outer != nullptr );
    ASSERT_TRUE( inner != nullptr );
    ASSERT_TRUE( ( outer->state & VXUI_DRAW_HOVERED ) != 0 );
    ASSERT_TRUE( ( inner->state & VXUI_DRAW_HOVERED ) != 0 );
}

UTEST(mouse, text_cmd_no_hover) {
    vxui_ctx ctx;
    init_ctx_with_font( &ctx );

    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_mouse( &ctx, 5.0f, 5.0f, 0 );
    if ( vxui_menu( &ctx, "test" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    int n = vxui_draw_count( dl, VXUI_DRAW_TEXT );
    ASSERT_GT( n, 0 );
    for ( int i = 0; i < n; i++ )
        ASSERT_EQ( vxui_draw_nth( dl, VXUI_DRAW_TEXT, i )->state & VXUI_DRAW_HOVERED, (uint8_t) 0 );
}

UTEST(mouse, buttons_cleared_each_frame) {
    vxui_ctx ctx;
    init_ctx( &ctx );

    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_mouse( &ctx, 0, 0, VXUI_MOUSE_LEFT );
    vxui_render( &ctx );
    ASSERT_EQ( ctx.mouse_buttons, (uint32_t) VXUI_MOUSE_LEFT );

    vxui_frame( &ctx, 1.0f / 60.0f );
    ASSERT_EQ( ctx.mouse_buttons, (uint32_t) 0 );
}

UTEST(mouse, prev_buttons_carries_one_frame) {
    vxui_ctx ctx;
    init_ctx( &ctx );

    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_mouse( &ctx, 0, 0, VXUI_MOUSE_LEFT );
    vxui_render( &ctx );

    vxui_frame( &ctx, 1.0f / 60.0f );
    ASSERT_EQ( ctx.prev_mouse_buttons, (uint32_t) VXUI_MOUSE_LEFT );
    ASSERT_EQ( ctx.mouse_buttons,      (uint32_t) 0 );
}

UTEST(mouse, pos_persists_across_frames) {
    vxui_ctx ctx;
    init_ctx( &ctx );

    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_mouse( &ctx, 42.0f, 13.0f, 0 );
    vxui_render( &ctx );

    vxui_frame( &ctx, 1.0f / 60.0f );
    ASSERT_EQ( ctx.mouse_pos.x, 42.0f );
    ASSERT_EQ( ctx.mouse_pos.y, 13.0f );
}

UTEST(mouse, down_on_rect_stamps_pressed) {
    vxui_ctx ctx;
    init_ctx( &ctx );

    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_mouse( &ctx, 50.0f, 50.0f, VXUI_MOUSE_LEFT );
    vxui_rect( &ctx, "panel", { .width = { VXUI_FIXED, 200 }, .height = { VXUI_FIXED, 100 } } );
    vxui_div_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, elem_id( "panel" ) );
    ASSERT_TRUE( c != nullptr );
    ASSERT_TRUE( ( c->state & VXUI_DRAW_PRESSED ) != 0 );
    ASSERT_EQ  ( ctx.mouse_press_row_id, c->id );
}

UTEST(mouse, down_off_no_pressed_anywhere) {
    vxui_ctx ctx;
    init_ctx( &ctx );

    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_mouse( &ctx, 500.0f, 500.0f, VXUI_MOUSE_LEFT );
    vxui_rect( &ctx, "panel", { .width = { VXUI_FIXED, 200 }, .height = { VXUI_FIXED, 100 } } );
    vxui_div_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, elem_id( "panel" ) );
    ASSERT_TRUE( c != nullptr );
    ASSERT_EQ( c->state & VXUI_DRAW_PRESSED, (uint8_t) 0 );
    ASSERT_EQ( ctx.mouse_press_row_id, (uint32_t) 0 );
}

UTEST(mouse, drag_off_during_press_drops_pressed_visual) {
    vxui_ctx ctx;
    init_ctx( &ctx );

    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_mouse( &ctx, 50.0f, 50.0f, VXUI_MOUSE_LEFT );
    vxui_rect( &ctx, "panel", { .width = { VXUI_FIXED, 200 }, .height = { VXUI_FIXED, 100 } } );
    vxui_div_end( &ctx );
    vxui_render( &ctx );

    // Cursor leaves rect with button still held: PRESSED gone, but press_row_id retained.
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_mouse( &ctx, 500.0f, 500.0f, VXUI_MOUSE_LEFT );
    vxui_rect( &ctx, "panel", { .width = { VXUI_FIXED, 200 }, .height = { VXUI_FIXED, 100 } } );
    vxui_div_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, elem_id( "panel" ) );
    ASSERT_TRUE( c != nullptr );
    ASSERT_EQ  ( c->state & VXUI_DRAW_PRESSED, (uint8_t) 0 );
    ASSERT_NE  ( ctx.mouse_press_row_id, (uint32_t) 0 );
}

UTEST(mouse, click_on_focused_action_fires) {
    vxui_ctx ctx;
    init_ctx_with_font( &ctx );

    // Frame 0: settle layout, capture rects.
    menu_frame( &ctx, FLT_MAX, FLT_MAX, 0 );
    vxui_draw_list dl0 = ctx.draw_list;
    const vxui_draw_cmd* play = vxui_draw_find( dl0, VXUI_DRAW_RECT, row_id( "test", "Play" ) );
    ASSERT_TRUE( play != nullptr );
    float px = play->rect.x + 5.0f;
    float py = play->rect.y + play->rect.w * 0.5f;

    // Press over Play.
    ASSERT_FALSE( menu_frame( &ctx, px, py, VXUI_MOUSE_LEFT ).play );
    // Release over Play. Sets click_row_id at end of render.
    ASSERT_FALSE( menu_frame( &ctx, px, py, 0 ).play );
    // Next declarations consume click_row_id and fire.
    menu_fire f3 = menu_frame( &ctx, px, py, 0 );
    ASSERT_TRUE ( f3.play );
    ASSERT_FALSE( f3.quit );
}

UTEST(mouse, hold_fires_only_once) {
    vxui_ctx ctx;
    init_ctx_with_font( &ctx );

    menu_frame( &ctx, FLT_MAX, FLT_MAX, 0 );
    vxui_draw_list dl0 = ctx.draw_list;
    const vxui_draw_cmd* play = vxui_draw_find( dl0, VXUI_DRAW_RECT, row_id( "test", "Play" ) );
    ASSERT_TRUE( play != nullptr );
    float px = play->rect.x + 5.0f;
    float py = play->rect.y + play->rect.w * 0.5f;

    menu_frame( &ctx, px, py, VXUI_MOUSE_LEFT );        // press
    for ( int i = 0; i < 30; i++ )                       // hold for many frames
        ASSERT_FALSE( menu_frame( &ctx, px, py, VXUI_MOUSE_LEFT ).play );
    menu_frame( &ctx, px, py, 0 );                       // release
    ASSERT_TRUE ( menu_frame( &ctx, px, py, 0 ).play );  // fires once
    for ( int i = 0; i < 5; i++ )
        ASSERT_FALSE( menu_frame( &ctx, px, py, 0 ).play );
}

UTEST(mouse, drag_off_then_release_no_fire) {
    vxui_ctx ctx;
    init_ctx_with_font( &ctx );

    menu_frame( &ctx, FLT_MAX, FLT_MAX, 0 );
    vxui_draw_list dl0 = ctx.draw_list;
    const vxui_draw_cmd* play = vxui_draw_find( dl0, VXUI_DRAW_RECT, row_id( "test", "Play" ) );
    ASSERT_TRUE( play != nullptr );
    float px = play->rect.x + 5.0f;
    float py = play->rect.y + play->rect.w * 0.5f;

    menu_frame( &ctx, px,    py,    VXUI_MOUSE_LEFT );  // press over Play
    menu_frame( &ctx, 800.0f, 800.0f, VXUI_MOUSE_LEFT );  // drag far off
    menu_frame( &ctx, 800.0f, 800.0f, 0 );                // release off
    menu_fire fire = menu_frame( &ctx, 800.0f, 800.0f, 0 );

    ASSERT_FALSE( fire.play );
    ASSERT_FALSE( fire.quit );
    ASSERT_EQ( ctx.mouse_click_row_id, (uint32_t) 0 );
}

UTEST(mouse, click_on_unfocused_row_moves_focus_and_fires) {
    vxui_ctx ctx;
    init_ctx_with_font( &ctx );

    menu_frame( &ctx, FLT_MAX, FLT_MAX, 0 );
    vxui_draw_list dl0 = ctx.draw_list;
    const vxui_draw_cmd* quit = vxui_draw_find( dl0, VXUI_DRAW_RECT, row_id( "test", "Quit" ) );
    ASSERT_TRUE( quit != nullptr );
    float qx = quit->rect.x + 5.0f;
    float qy = quit->rect.y + quit->rect.w * 0.5f;

    // Initial focus is on Play (row 0). Click on Quit.
    menu_frame( &ctx, qx, qy, VXUI_MOUSE_LEFT );
    menu_frame( &ctx, qx, qy, 0 );
    menu_fire fire = menu_frame( &ctx, qx, qy, 0 );

    ASSERT_TRUE ( fire.quit );
    ASSERT_FALSE( fire.play );

    vxui_draw_list dl_post = ctx.draw_list;
    const vxui_draw_cmd* qrect = vxui_draw_find( dl_post, VXUI_DRAW_RECT, row_id( "test", "Quit" ) );
    ASSERT_TRUE( qrect != nullptr );
    ASSERT_TRUE( ( qrect->state & VXUI_DRAW_FOCUSED ) != 0 );
}

UTEST(mouse, hover_does_not_move_focus) {
    vxui_ctx ctx;
    init_ctx_with_font( &ctx );

    menu_frame( &ctx, FLT_MAX, FLT_MAX, 0 );
    vxui_draw_list dl0 = ctx.draw_list;
    const vxui_draw_cmd* quit = vxui_draw_find( dl0, VXUI_DRAW_RECT, row_id( "test", "Quit" ) );
    ASSERT_TRUE( quit != nullptr );
    float qx = quit->rect.x + 5.0f;
    float qy = quit->rect.y + quit->rect.w * 0.5f;

    // Hover Quit without clicking for many frames.
    for ( int i = 0; i < 10; i++ )
        menu_frame( &ctx, qx, qy, 0 );

    vxui_draw_list dl_post = ctx.draw_list;
    const vxui_draw_cmd* play = vxui_draw_find( dl_post, VXUI_DRAW_RECT, row_id( "test", "Play" ) );
    ASSERT_TRUE( play != nullptr );
    ASSERT_TRUE( ( play->state & VXUI_DRAW_FOCUSED ) != 0 );
}

UTEST(mouse, press_then_move_back_then_release_fires) {
    vxui_ctx ctx;
    init_ctx_with_font( &ctx );

    menu_frame( &ctx, FLT_MAX, FLT_MAX, 0 );
    vxui_draw_list dl0 = ctx.draw_list;
    const vxui_draw_cmd* play = vxui_draw_find( dl0, VXUI_DRAW_RECT, row_id( "test", "Play" ) );
    ASSERT_TRUE( play != nullptr );
    float px = play->rect.x + 5.0f;
    float py = play->rect.y + play->rect.w * 0.5f;

    menu_frame( &ctx, px,    py,    VXUI_MOUSE_LEFT );  // press
    menu_frame( &ctx, 800.0f, 800.0f, VXUI_MOUSE_LEFT );  // drag off
    menu_frame( &ctx, px,    py,    VXUI_MOUSE_LEFT );  // drag back
    menu_frame( &ctx, px,    py,    0 );                // release on row
    menu_fire fire = menu_frame( &ctx, px, py, 0 );

    ASSERT_TRUE( fire.play );
}

UTEST_MAIN();
