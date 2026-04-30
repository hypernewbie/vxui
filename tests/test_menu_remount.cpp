#include "utest.h"
#include "../vxui.h"
#define VXUI_IMPL
#include "../vxui_impl.h"

static uint8_t s_clay_mem[16 * 1024 * 1024];

static vxui_ctx make_ctx()
{
    vxui_ctx ctx = {};
    vxui_init( &ctx, 1280, 720, s_clay_mem, sizeof( s_clay_mem ) );
    return ctx;
}

static void emit_menu( vxui_ctx* ctx, uint32_t input )
{
    vxui_frame( ctx, 1.0f / 60.0f );
    ctx->input = input;
    if ( vxui_menu( ctx, "m" ) )
    {
        vxui_menu_action( ctx, "Play" );
        vxui_menu_action( ctx, "Quit" );
        vxui_menu_end( ctx );
    }
    vxui_render( ctx );
}

static void emit_no_menu( vxui_ctx* ctx )
{
    vxui_frame( ctx, 1.0f / 60.0f );
    vxui_render( ctx );
}

UTEST(menu_remount, active_mask_set_when_menu_called) {
    vxui_ctx ctx = make_ctx();

    emit_menu( &ctx, 0 );
    ASSERT_NE( ctx.menu_active_mask & 1u, (uint32_t) 0 );
}

UTEST(menu_remount, active_mask_cleared_when_menu_skipped) {
    vxui_ctx ctx = make_ctx();

    emit_menu   ( &ctx, 0 );
    emit_no_menu( &ctx );
    ASSERT_EQ( ctx.menu_active_mask & 1u, (uint32_t) 0 );
}

UTEST(menu_remount, prev_active_mask_reflects_previous_frame) {
    vxui_ctx ctx = make_ctx();

    emit_menu( &ctx, 0 );
    emit_menu( &ctx, 0 );
    ASSERT_NE( ctx.prev_active_mask & 1u, (uint32_t) 0 );
}

UTEST(menu_remount, spring_offset_zero_after_remount) {
    vxui_ctx ctx = make_ctx();

    emit_menu( &ctx, 0 );
    emit_menu( &ctx, VXUI_INPUT_DOWN );
    ASSERT_NE( ctx.menu_focus_spring[0].x, 0.0f );    // mid-animation, offset non-zero

    emit_no_menu( &ctx );
    emit_menu( &ctx, 0 );
    ASSERT_NEAR( ctx.menu_focus_spring[0].x, 0.0f, 1e-3f );    // snapped, offset zero
    ASSERT_NEAR( ctx.menu_focus_spring[0].y, 0.0f, 1e-3f );    // velocity zero too
}

UTEST(menu_remount, focus_row_preserved_on_remount) {
    vxui_ctx ctx = make_ctx();

    emit_menu( &ctx, 0 );
    emit_menu( &ctx, VXUI_INPUT_DOWN );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 1 );

    emit_no_menu( &ctx );

    emit_menu( &ctx, 0 );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 1 );
}

UTEST(menu_remount, no_snap_when_continuously_active) {
    vxui_ctx ctx = make_ctx();

    emit_menu( &ctx, 0 );
    emit_menu( &ctx, VXUI_INPUT_DOWN );

    // After down, prev_row gets set to current_row this frame.
    float prev_z = ctx.menu_focus_spring[0].z;
    ASSERT_GE( prev_z, 0.0f );

    emit_menu( &ctx, 0 );
    ASSERT_NEAR( ctx.menu_focus_spring[0].z, prev_z, 1e-3f );
}

UTEST(menu_remount, focus_rect_appears_at_correct_row_after_remount) {
    vxui_ctx ctx = make_ctx();

    emit_menu( &ctx, 0 );
    emit_menu( &ctx, VXUI_INPUT_DOWN );

    emit_no_menu( &ctx );

    emit_menu( &ctx, 0 );
    vxui_draw_list dl = ctx.draw_list;

    ASSERT_EQ( dl.count, 3 );
    // Focus rect should be at row 1 (Quit), spring snapped not animating.
    ASSERT_NEAR( dl.cmds[2].rect.y, dl.cmds[1].rect.y, 1e-3f );
}

UTEST(menu_remount, two_menus_independent_remount_state) {
    vxui_ctx ctx = make_ctx();

    auto emit_two = [&]( bool show_a, bool show_b, uint32_t input_a ) {
        vxui_frame( &ctx, 1.0f / 60.0f );
        ctx.input = input_a;
        if ( show_a && vxui_menu( &ctx, "a" ) )
        {
            vxui_menu_action( &ctx, "A0" );
            vxui_menu_action( &ctx, "A1" );
            vxui_menu_end( &ctx );
        }
        if ( show_b && vxui_menu( &ctx, "b" ) )
        {
            vxui_menu_action( &ctx, "B0" );
            vxui_menu_action( &ctx, "B1" );
            vxui_menu_end( &ctx );
        }
        vxui_render( &ctx );
    };

    emit_two( true, true,  0 );
    emit_two( true, true,  VXUI_INPUT_DOWN );    // a navigates, animation starts
    ASSERT_NE( ctx.menu_focus_spring[0].x, 0.0f );

    emit_two( true, false, 0 );    // b unmounts (a stays mid-animation)
    emit_two( true, true,  0 );    // b remounts

    ASSERT_NEAR( ctx.menu_focus_spring[1].x, 0.0f, 1e-3f );    // b snapped
    ASSERT_NE  ( ctx.menu_focus_spring[0].x, 0.0f );           // a still animating, not snapped
}

UTEST_MAIN();
