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

// Replicate composite id: menu hash as seed, label as key. Matches vxui_menu_open_row.
static uint32_t row_id( const char* menu, const char* label )
{
    Clay_String cs = { false, (int32_t) strlen( label ), label };
    return Clay__HashString( cs, vxui_hash( menu ) ).id;
}

UTEST(render_data, rect_default_state_zero) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_label( &ctx, "Hello" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 1 );
    ASSERT_EQ( vxui_draw_nth( dl, VXUI_DRAW_RECT, 0 )->state, (uint8_t) 0 );
}

UTEST(render_data, focused_row_has_focused_bit) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "m", "Play" ) );
    ASSERT_TRUE( c != nullptr );
    ASSERT_TRUE( ( c->state & VXUI_DRAW_FOCUSED ) != 0 );
}

UTEST(render_data, unfocused_rows_no_focused_bit) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_action( &ctx, "Quit" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "m", "Quit" ) );
    ASSERT_TRUE( c != nullptr );
    ASSERT_EQ( c->state & VXUI_DRAW_FOCUSED, (uint8_t) 0 );
}

UTEST(render_data, section_row_no_focused_bit) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_section( &ctx, "Header" );
        vxui_menu_action ( &ctx, "Play" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "m", "Header" ) );
    ASSERT_TRUE( c != nullptr );
    ASSERT_EQ( c->state & VXUI_DRAW_FOCUSED, (uint8_t) 0 );
}

UTEST(render_data, label_row_no_focused_bit) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_label ( &ctx, "Note" );
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "m", "Note" ) );
    ASSERT_TRUE( c != nullptr );
    ASSERT_EQ( c->state & VXUI_DRAW_FOCUSED, (uint8_t) 0 );
}

UTEST(render_data, text_cmd_state_zero) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    int n = vxui_draw_count( dl, VXUI_DRAW_TEXT );
    for ( int i = 0; i < n; i++ )
        ASSERT_EQ( vxui_draw_nth( dl, VXUI_DRAW_TEXT, i )->state, (uint8_t) 0 );
}

UTEST(render_data, confirm_held_focused_action_has_pressed) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    ctx.input = VXUI_INPUT_CONFIRM;
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "m", "Play" ) );
    ASSERT_TRUE( c != nullptr );
    ASSERT_TRUE( ( c->state & VXUI_DRAW_PRESSED ) != 0 );
    ASSERT_TRUE( ( c->state & VXUI_DRAW_FOCUSED ) != 0 );
}

UTEST(render_data, confirm_released_no_pressed) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "m", "Play" ) );
    ASSERT_TRUE( c != nullptr );
    ASSERT_EQ( c->state & VXUI_DRAW_PRESSED, (uint8_t) 0 );
}

UTEST(render_data, confirm_held_unfocused_actions_not_pressed) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    ctx.input = VXUI_INPUT_CONFIRM;
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_action( &ctx, "Quit" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "m", "Quit" ) );
    ASSERT_TRUE( c != nullptr );
    ASSERT_EQ( c->state & VXUI_DRAW_PRESSED, (uint8_t) 0 );
}

UTEST(render_data, two_menus_each_focused_action_has_pressed) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    ctx.input = VXUI_INPUT_CONFIRM;
    if ( vxui_menu( &ctx, "left" ) )
    {
        vxui_menu_action( &ctx, "Back" );
        vxui_menu_end( &ctx );
    }
    if ( vxui_menu( &ctx, "right" ) )
    {
        vxui_menu_action( &ctx, "Next" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* l = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "left",  "Back" ) );
    const vxui_draw_cmd* r = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "right", "Next" ) );
    ASSERT_TRUE( l != nullptr );
    ASSERT_TRUE( r != nullptr );
    ASSERT_TRUE( ( l->state & VXUI_DRAW_PRESSED ) != 0 );
    ASSERT_TRUE( ( r->state & VXUI_DRAW_PRESSED ) != 0 );
}

UTEST(render_data, two_menus_each_focused_row_has_focused_bit) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "left" ) )
    {
        vxui_menu_action( &ctx, "Back" );
        vxui_menu_end( &ctx );
    }
    if ( vxui_menu( &ctx, "right" ) )
    {
        vxui_menu_action( &ctx, "Next" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* l = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "left",  "Back" ) );
    const vxui_draw_cmd* r = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "right", "Next" ) );
    ASSERT_TRUE( l != nullptr );
    ASSERT_TRUE( r != nullptr );
    ASSERT_TRUE( ( l->state & VXUI_DRAW_FOCUSED ) != 0 );
    ASSERT_TRUE( ( r->state & VXUI_DRAW_FOCUSED ) != 0 );
}

UTEST(render_data, focused_row_carries_focus_offset_zero_on_snap) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "m", "Play" ) );
    ASSERT_TRUE( c != nullptr );
    ASSERT_TRUE( ( c->state & VXUI_DRAW_FOCUSED ) != 0 );
    ASSERT_NEAR( c->focus_offset_y, 0.0f, 1e-3f );
}

UTEST(render_data, unfocused_rows_focus_offset_zero) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_action( &ctx, "Quit" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "m", "Quit" ) );
    ASSERT_TRUE( c != nullptr );
    ASSERT_EQ  ( c->state & VXUI_DRAW_FOCUSED, (uint8_t) 0 );
    ASSERT_NEAR( c->focus_offset_y, 0.0f, 1e-3f );
}

UTEST(render_data, text_cmd_focus_offset_zero) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    int n = vxui_draw_count( dl, VXUI_DRAW_TEXT );
    for ( int i = 0; i < n; i++ )
        ASSERT_NEAR( vxui_draw_nth( dl, VXUI_DRAW_TEXT, i )->focus_offset_y, 0.0f, 1e-3f );
}

UTEST(render_data, focus_offset_nonzero_after_focus_move) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_action( &ctx, "Quit" );
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );

    vxui_frame( &ctx, 1.0f / 60.0f );
    ctx.input = VXUI_INPUT_DOWN;
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_action( &ctx, "Quit" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "m", "Quit" ) );
    ASSERT_TRUE( c != nullptr );
    ASSERT_TRUE( ( c->state & VXUI_DRAW_FOCUSED ) != 0 );
    ASSERT_LT( c->focus_offset_y, -(float) VXUI_ROW_HEIGHT * 0.1f );
}

UTEST(render_data, focus_offset_settles_toward_zero) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_action( &ctx, "Quit" );
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );

    vxui_frame( &ctx, 1.0f / 60.0f );
    ctx.input = VXUI_INPUT_DOWN;
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_action( &ctx, "Quit" );
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );

    float prev_abs = (float) VXUI_ROW_HEIGHT;
    for ( int i = 0; i < 10; i++ )
    {
        vxui_frame( &ctx, 1.0f / 60.0f );
        if ( vxui_menu( &ctx, "m" ) )
        {
            vxui_menu_action( &ctx, "Play" );
            vxui_menu_action( &ctx, "Quit" );
            vxui_menu_end( &ctx );
        }
        vxui_draw_list dl = vxui_render( &ctx );

        const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "m", "Quit" ) );
        ASSERT_TRUE( c != nullptr );
        float abs_offset = c->focus_offset_y < 0.0f ? -c->focus_offset_y : c->focus_offset_y;
        ASSERT_LT( abs_offset, prev_abs + 1e-3f );
        prev_abs = abs_offset;
    }
}

UTEST(render_data, same_label_two_menus_only_focused_one_has_bit) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "left" ) )
    {
        vxui_menu_action( &ctx, "Same" );
        vxui_menu_action( &ctx, "Tail" );
        vxui_menu_end( &ctx );
    }
    if ( vxui_menu( &ctx, "right" ) )
    {
        vxui_menu_action( &ctx, "Head" );
        vxui_menu_action( &ctx, "Same" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* l_same = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "left",  "Same" ) );
    const vxui_draw_cmd* r_same = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "right", "Same" ) );
    ASSERT_TRUE( l_same != nullptr );
    ASSERT_TRUE( r_same != nullptr );
    ASSERT_TRUE( ( l_same->state & VXUI_DRAW_FOCUSED ) != 0 );
    ASSERT_EQ  ( r_same->state & VXUI_DRAW_FOCUSED, (uint8_t) 0 );
}

UTEST_MAIN();
