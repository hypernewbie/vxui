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

static void emit_menu( vxui_ctx* ctx, int n_rows, int max_visible, uint32_t input )
{
    vxui_frame( ctx, 1.0f / 60.0f );
    ctx->input = input;
    if ( vxui_menu( ctx, "m", true, max_visible ) )
    {
        char buf[16];
        for ( int i = 0; i < n_rows; i++ )
        {
            snprintf( buf, sizeof( buf ), "row%d", i );
            vxui_menu_action( ctx, buf );
        }
        vxui_menu_end( ctx );
    }
    vxui_render( ctx );
}

UTEST(menu_scroll, no_scroll_when_max_visible_zero) {
    vxui_ctx ctx = make_ctx();
    emit_menu( &ctx, 8, 0, 0 );

    ASSERT_EQ( ctx.menu_scroll_top[0], 0 );
}

UTEST(menu_scroll, no_scroll_when_rows_fit) {
    vxui_ctx ctx = make_ctx();
    emit_menu( &ctx, 3, 5, 0 );

    ASSERT_EQ( ctx.menu_scroll_top[0], 0 );
}

UTEST(menu_scroll, scroll_top_zero_at_start) {
    vxui_ctx ctx = make_ctx();
    emit_menu( &ctx, 8, 3, 0 );

    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 0 );
    ASSERT_EQ( ctx.menu_scroll_top[0], 0 );
}

UTEST(menu_scroll, scroll_advances_when_focus_leaves_bottom) {
    vxui_ctx ctx = make_ctx();
    emit_menu( &ctx, 8, 3, 0 );

    emit_menu( &ctx, 8, 3, VXUI_INPUT_DOWN );    // focus 1
    ASSERT_EQ( ctx.menu_scroll_top[0], 0 );

    emit_menu( &ctx, 8, 3, VXUI_INPUT_DOWN );    // focus 2
    ASSERT_EQ( ctx.menu_scroll_top[0], 0 );

    emit_menu( &ctx, 8, 3, VXUI_INPUT_DOWN );    // focus 3, scroll to 1
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 3 );
    ASSERT_EQ( ctx.menu_scroll_top[0], 1 );

    emit_menu( &ctx, 8, 3, VXUI_INPUT_DOWN );    // focus 4, scroll to 2
    ASSERT_EQ( ctx.menu_scroll_top[0], 2 );
}

UTEST(menu_scroll, scroll_recedes_when_focus_leaves_top) {
    vxui_ctx ctx = make_ctx();
    emit_menu( &ctx, 8, 3, 0 );

    for ( int i = 0; i < 5; i++ )
        emit_menu( &ctx, 8, 3, VXUI_INPUT_DOWN );

    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 5 );
    ASSERT_EQ( ctx.menu_scroll_top[0], 3 );

    emit_menu( &ctx, 8, 3, VXUI_INPUT_UP );    // focus 4, still in [3,5]
    ASSERT_EQ( ctx.menu_scroll_top[0], 3 );

    emit_menu( &ctx, 8, 3, VXUI_INPUT_UP );    // focus 3, still in [3,5]
    ASSERT_EQ( ctx.menu_scroll_top[0], 3 );

    emit_menu( &ctx, 8, 3, VXUI_INPUT_UP );    // focus 2, scroll to 2
    ASSERT_EQ( ctx.menu_scroll_top[0], 2 );

    emit_menu( &ctx, 8, 3, VXUI_INPUT_UP );    // focus 1, scroll to 1
    ASSERT_EQ( ctx.menu_scroll_top[0], 1 );

    emit_menu( &ctx, 8, 3, VXUI_INPUT_UP );    // focus 0, scroll to 0
    ASSERT_EQ( ctx.menu_scroll_top[0], 0 );
}

UTEST(menu_scroll, wrap_up_jumps_scroll_to_bottom) {
    vxui_ctx ctx = make_ctx();
    emit_menu( &ctx, 8, 3, 0 );

    emit_menu( &ctx, 8, 3, VXUI_INPUT_UP );    // wraps to row 7
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 7 );
    ASSERT_EQ( ctx.menu_scroll_top[0], 5 );    // 7 - 3 + 1 = 5
}

UTEST(menu_scroll, wrap_down_resets_scroll_to_top) {
    vxui_ctx ctx = make_ctx();
    emit_menu( &ctx, 8, 3, 0 );

    for ( int i = 0; i < 7; i++ )
        emit_menu( &ctx, 8, 3, VXUI_INPUT_DOWN );

    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 7 );

    emit_menu( &ctx, 8, 3, VXUI_INPUT_DOWN );    // wraps to 0
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 0 );
    ASSERT_EQ( ctx.menu_scroll_top[0], 0 );
}

UTEST(menu_scroll, draw_list_shows_scrolled_positions) {
    vxui_ctx ctx = make_ctx();
    emit_menu( &ctx, 8, 3, 0 );

    for ( int i = 0; i < 4; i++ )
        emit_menu( &ctx, 8, 3, VXUI_INPUT_DOWN );

    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 4 );
    ASSERT_EQ( ctx.menu_scroll_top[0], 2 );

    vxui_draw_list dl = ctx.draw_list;
    Clay_String row2_cs = { false, 4, "row2" };
    uint32_t row2_id = Clay__HashString( row2_cs, vxui_hash( "m" ) ).id;

    const vxui_draw_cmd* row2 = vxui_draw_find( dl, VXUI_DRAW_RECT, row2_id );
    ASSERT_TRUE( row2 != nullptr );
    ASSERT_NEAR( row2->rect.y, 0.0f, 1e-3f );
}

UTEST(menu_scroll, scroll_with_skip_rows) {
    vxui_ctx ctx = make_ctx();

    auto emit = [&]( uint32_t input ) {
        vxui_frame( &ctx, 1.0f / 60.0f );
        ctx.input = input;
        if ( vxui_menu( &ctx, "m", true, 3 ) )
        {
            vxui_menu_section( &ctx, "Top" );
            vxui_menu_action ( &ctx, "A0" );
            vxui_menu_action ( &ctx, "A1" );
            vxui_menu_action ( &ctx, "A2" );
            vxui_menu_action ( &ctx, "A3" );
            vxui_menu_action ( &ctx, "A4" );
            vxui_menu_end( &ctx );
        }
        vxui_render( &ctx );
    };

    emit( 0 );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 1 );    // promoted past section

    emit( VXUI_INPUT_DOWN );    // focus 2
    emit( VXUI_INPUT_DOWN );    // focus 3
    emit( VXUI_INPUT_DOWN );    // focus 4, scroll_top should advance
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 4 );
    ASSERT_EQ( ctx.menu_scroll_top[0], 2 );
}

UTEST(menu_scroll, two_menus_independent_scroll) {
    vxui_ctx ctx = make_ctx();

    auto emit_two = [&]( uint32_t input_a, uint32_t input_b ) {
        vxui_frame( &ctx, 1.0f / 60.0f );
        ctx.input = input_a;
        if ( vxui_menu( &ctx, "ma", true, 3 ) )
        {
            for ( int i = 0; i < 8; i++ )
            {
                char buf[16];
                snprintf( buf, sizeof( buf ), "a%d", i );
                vxui_menu_action( &ctx, buf );
            }
            vxui_menu_end( &ctx );
        }
        ctx.input = input_b;
        if ( vxui_menu( &ctx, "mb", true, 3 ) )
        {
            for ( int i = 0; i < 8; i++ )
            {
                char buf[16];
                snprintf( buf, sizeof( buf ), "b%d", i );
                vxui_menu_action( &ctx, buf );
            }
            vxui_menu_end( &ctx );
        }
        vxui_render( &ctx );
    };

    emit_two( 0, 0 );

    for ( int i = 0; i < 5; i++ )
        emit_two( VXUI_INPUT_DOWN, 0 );

    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 5 );
    ASSERT_EQ( ctx.menu_state[1].y, (uint32_t) 0 );
    ASSERT_EQ( ctx.menu_scroll_top[0], 3 );
    ASSERT_EQ( ctx.menu_scroll_top[1], 0 );
}

UTEST_MAIN();
