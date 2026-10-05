#include "utest.h"
#include "../vxui.h"
#define VXUI_IMPL
#include "../vxui_impl.h"

static uint8_t s_clay_mem[16 * 1024 * 1024];

static vxui_ctx make_ctx()
{
    vxui_ctx ctx = {};
    vxui_init( &ctx, 640, 480, s_clay_mem, sizeof( s_clay_mem ) );
    return ctx;
}

static vxui_draw_list overlap_frame( vxui_ctx* ctx, glm::vec2 mouse, uint32_t buttons )
{
    vxui_frame( ctx, 0.01f );
    vxui_mouse( ctx, mouse.x, mouse.y, buttons );
    vxui_root( ctx, "a.root", 0, 0 );
    vxui_rect( ctx, "a", { .width = { VXUI_FIXED, 100 }, .height = { VXUI_FIXED, 80 } } );
    vxui_div_end( ctx );
    vxui_root_end( ctx );
    vxui_root( ctx, "b.root", 20, 20 );
    vxui_rect( ctx, "b", { .width = { VXUI_FIXED, 100 }, .height = { VXUI_FIXED, 80 } } );
    vxui_div_end( ctx );
    vxui_root_end( ctx );
    return vxui_render( ctx );
}

UTEST(mouse_resolution, overlapping_rectangles_hover_together_but_only_the_topmost_is_pressed) {
    vxui_ctx ctx = make_ctx();
    vxui_draw_list dl = overlap_frame( &ctx, { 40, 40 }, VXUI_MOUSE_LEFT );
    ASSERT_EQ( dl.count, 2 );
    ASSERT_EQ( dl.cmds[0].state, (uint8_t) VXUI_DRAW_HOVERED );
    ASSERT_EQ( dl.cmds[1].state, (uint8_t) ( VXUI_DRAW_HOVERED | VXUI_DRAW_PRESSED ) );
    ASSERT_EQ( ctx.mouse_press_row_id, vxui_id( "b" ) );
}

UTEST(mouse_resolution, dragged_press_reappears_only_when_its_original_rectangle_contains_the_cursor) {
    vxui_ctx ctx = make_ctx();
    overlap_frame( &ctx, { 40, 40 }, VXUI_MOUSE_LEFT );
    vxui_draw_list dl = overlap_frame( &ctx, { 4, 4 }, VXUI_MOUSE_LEFT );
    ASSERT_EQ( dl.cmds[0].state, (uint8_t) VXUI_DRAW_HOVERED );
    ASSERT_EQ( dl.cmds[1].state, (uint8_t) 0 );
    ASSERT_EQ( ctx.mouse_press_row_id, vxui_id( "b" ) );
    dl = overlap_frame( &ctx, { 40, 40 }, VXUI_MOUSE_LEFT );
    ASSERT_TRUE( dl.cmds[1].state & VXUI_DRAW_PRESSED );
}

UTEST(mouse_resolution, release_uses_the_settled_topmost_target_before_pressed_stamping) {
    vxui_ctx ctx = make_ctx();
    overlap_frame( &ctx, { 40, 40 }, VXUI_MOUSE_LEFT );
    vxui_draw_list dl = overlap_frame( &ctx, { 40, 40 }, 0 );
    ASSERT_EQ( ctx.mouse_click_row_id, vxui_id( "b" ) );
    ASSERT_EQ( ctx.mouse_press_row_id, 0u );
    for ( int i = 0; i < dl.count; i++ ) ASSERT_EQ( dl.cmds[i].state, (uint8_t) VXUI_DRAW_HOVERED );
}

UTEST(mouse_resolution, release_over_a_different_rectangle_does_not_click) {
    vxui_ctx ctx = make_ctx();
    overlap_frame( &ctx, { 40, 40 }, VXUI_MOUSE_LEFT );
    overlap_frame( &ctx, { 4, 4 }, 0 );
    ASSERT_EQ( ctx.mouse_click_row_id, 0u );
    ASSERT_EQ( ctx.mouse_press_row_id, 0u );
}

UTEST(mouse_resolution, all_inclusive_edges_share_the_same_hover_and_press_result) {
    glm::vec2 points[] = { { 20, 20 }, { 120, 20 }, { 20, 100 }, { 120, 100 } };
    for ( glm::vec2 p : points )
    {
        vxui_ctx ctx = make_ctx();
        vxui_draw_list dl = overlap_frame( &ctx, p, VXUI_MOUSE_LEFT );
        ASSERT_TRUE( dl.cmds[1].state & VXUI_DRAW_HOVERED );
        ASSERT_TRUE( dl.cmds[1].state & VXUI_DRAW_PRESSED );
    }
}

UTEST(mouse_resolution, frame_reuse_overwrites_old_hover_bits) {
    vxui_ctx ctx = make_ctx();
    overlap_frame( &ctx, { 40, 40 }, VXUI_MOUSE_LEFT );
    vxui_draw_list dl = overlap_frame( &ctx, { FLT_MAX, FLT_MAX }, 0 );
    for ( int i = 0; i < dl.count; i++ ) ASSERT_EQ( dl.cmds[i].state, (uint8_t) 0 );
    ASSERT_EQ( ctx.mouse_click_row_id, 0u );
}

static void inspect_other_packets( const vxui_draw_cmd*, vxui_render_data* out, void* userdata )
{
    vxui_ctx* ctx = (vxui_ctx*) userdata;
    out->flags = ctx->draw_buf[1].state;
}

UTEST(mouse_resolution, first_resolver_sees_later_packets_final_press_bits) {
    vxui_ctx ctx = make_ctx();
    vxui_set_render_data_fn( &ctx, inspect_other_packets, &ctx );
    vxui_draw_list dl = overlap_frame( &ctx, { 40, 40 }, VXUI_MOUSE_LEFT );
    ASSERT_EQ( dl.cmds[0].render.flags, (uint32_t) ( VXUI_DRAW_HOVERED | VXUI_DRAW_PRESSED ) );
    ASSERT_EQ( dl.cmds[0].render.flags, (uint32_t) dl.cmds[1].state );
}

UTEST(mouse_resolution, right_button_only_never_sets_a_left_press_target) {
    vxui_ctx ctx = make_ctx();
    vxui_draw_list dl = overlap_frame( &ctx, { 40, 40 }, VXUI_MOUSE_RIGHT );
    for ( int i = 0; i < dl.count; i++ ) ASSERT_EQ( dl.cmds[i].state, (uint8_t) VXUI_DRAW_HOVERED );
    ASSERT_EQ( ctx.mouse_press_row_id, 0u );
}

UTEST_MAIN();
