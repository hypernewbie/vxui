#include "utest.h"
#include "../vxui.h"
#define VXUI_IMPL
#include "../vxui_impl.h"

static uint8_t s_clay_mem[16 * 1024 * 1024];
static int s_snapshot_count = 0;

static Clay_Dimensions measure_text( Clay_StringSlice s, Clay_TextElementConfig* cfg, void* )
{
    return { (float) s.length * 8.0f, (float) cfg->fontSize };
}

static vxui_ctx make_ctx()
{
    vxui_ctx ctx = {};
    vxui_init( &ctx, 960, 540, s_clay_mem, sizeof( s_clay_mem ) );
    Clay_SetMeasureTextFunction( measure_text, nullptr );
    return ctx;
}

static uint32_t row_id( const char* menu, const char* label )
{
    return Clay__HashString( { false, (int32_t) strlen( label ), label }, vxui_hash( menu ) ).id;
}

static void snapshot_state( const vxui_draw_cmd* cmd, vxui_render_data*, void* userdata )
{
    vxui_draw_cmd* snapshots = (vxui_draw_cmd*) userdata;
    snapshots[s_snapshot_count++] = *cmd;
}

static vxui_draw_list menu_frame( vxui_ctx* ctx, uint32_t input, glm::vec2 mouse, uint32_t buttons, int visible = 0 )
{
    vxui_frame( ctx, 1.0f / 60.0f );
    ctx->input = input;
    vxui_mouse( ctx, mouse.x, mouse.y, buttons );
    vxui_menu( ctx, "menu", true, visible );
    vxui_menu_action( ctx, "A" );
    vxui_menu_action( ctx, "B" );
    vxui_menu_action( ctx, "C" );
    vxui_menu_end( ctx );
    return vxui_render( ctx );
}

UTEST(draw_state, skip_rows_and_text_keep_their_distinct_state_contracts) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    ctx.input = VXUI_INPUT_CONFIRM;
    vxui_menu( &ctx, "menu" );
    vxui_menu_section( &ctx, "section" );
    vxui_menu_action( &ctx, "A" );
    vxui_menu_action( &ctx, "B" );
    vxui_menu_label( &ctx, "label" );
    vxui_menu_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 4 );
    for ( int i = 0; i < 4; i++ )
    {
        const vxui_draw_cmd* c = vxui_draw_nth( dl, VXUI_DRAW_RECT, i );
        ASSERT_EQ( c->row_index, i );
        ASSERT_EQ( c->focused_row_index, 1 );
        ASSERT_EQ( c->state, (uint8_t) ( i == 1 ? VXUI_DRAW_FOCUSED | VXUI_DRAW_PRESSED : 0 ) );
    }
    for ( int i = 0; i < dl.count; i++ )
    {
        if ( dl.cmds[i].type != VXUI_DRAW_TEXT ) continue;
        ASSERT_EQ( dl.cmds[i].state, (uint8_t) 0 );
        ASSERT_EQ( dl.cmds[i].row_index, -1 );
        ASSERT_EQ( dl.cmds[i].focused_row_index, -1 );
        ASSERT_EQ( dl.cmds[i].focus_offset_y, 0.0f );
    }
}

UTEST(draw_state, independent_menus_resolve_focus_press_and_offsets_together) {
    vxui_ctx ctx = make_ctx();
    const char* menus[] = { "left", "right" };
    for ( int frame = 0; frame < 2; frame++ )
    {
        vxui_frame( &ctx, 1.0f / 60.0f );
        ctx.input = frame == 0 ? 0 : VXUI_INPUT_CONFIRM;
        if ( frame == 1 ) ctx.menu_state[1].y = 1;
        for ( int m = 0; m < 2; m++ )
        {
            vxui_menu( &ctx, menus[m] );
            vxui_menu_action( &ctx, "A" );
            vxui_menu_action( &ctx, "B" );
            vxui_menu_end( &ctx );
        }
        vxui_draw_list dl = vxui_render( &ctx );

        for ( int m = 0; m < 2; m++ )
        {
            int focused = frame == 1 && m == 1 ? 1 : 0;
            for ( int row = 0; row < 2; row++ )
            {
                const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( menus[m], row == 0 ? "A" : "B" ) );
                ASSERT_TRUE( c );
                ASSERT_EQ( c->row_index, row );
                ASSERT_EQ( c->focused_row_index, focused );
                ASSERT_EQ( ( c->state & VXUI_DRAW_FOCUSED ) != 0, row == focused );
                ASSERT_EQ( ( c->state & VXUI_DRAW_PRESSED ) != 0, frame == 1 && row == focused );
                ASSERT_EQ( c->focus_offset_y, row == focused ? ctx.menu_focus_spring[m].x : 0.0f );
            }
        }
    }
}

UTEST(draw_state, last_overlapping_rectangle_wins_press_but_both_are_hovered) {
    vxui_ctx ctx = make_ctx();
    for ( int frame = 0; frame < 2; frame++ )
    {
        vxui_frame( &ctx, 1.0f / 60.0f );
        vxui_mouse( &ctx, 4, 4, frame == 0 ? VXUI_MOUSE_LEFT : 0 );
        vxui_root( &ctx, "menu_root", 0, 0 );
        vxui_menu( &ctx, "menu" );
        vxui_menu_action( &ctx, "A" );
        vxui_menu_end( &ctx );
        vxui_root_end( &ctx );
        vxui_root( &ctx, "overlay_root", 0, 0 );
        vxui_rect( &ctx, "overlay", { .width = { VXUI_FIXED, 8 }, .height = { VXUI_FIXED, 32 } } );
        vxui_div_end( &ctx );
        vxui_root_end( &ctx );
        vxui_draw_list dl = vxui_render( &ctx );

        const vxui_draw_cmd* row = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "menu", "A" ) );
        const vxui_draw_cmd* overlay = vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "overlay" ) );
        ASSERT_TRUE( row && overlay );
        ASSERT_TRUE( row->state & VXUI_DRAW_FOCUSED );
        ASSERT_TRUE( row->state & VXUI_DRAW_HOVERED );
        ASSERT_FALSE( row->state & VXUI_DRAW_PRESSED );
        ASSERT_TRUE( overlay->state & VXUI_DRAW_HOVERED );
        ASSERT_EQ( ( overlay->state & VXUI_DRAW_PRESSED ) != 0, frame == 0 );
        ASSERT_EQ( overlay->row_index, -1 );
        ASSERT_EQ( overlay->focused_row_index, -1 );
        ASSERT_EQ( ctx.mouse_press_row_id, frame == 0 ? vxui_id( "overlay" ) : 0u );
        ASSERT_EQ( ctx.mouse_click_row_id, frame == 1 ? vxui_id( "overlay" ) : 0u );
    }
}

UTEST(draw_state, leaving_a_pressed_row_and_releasing_outside_does_not_click) {
    vxui_ctx ctx = make_ctx();
    glm::vec2 positions[] = { { 4, 4 }, { 200, 200 }, { 200, 200 } };
    uint32_t buttons[] = { VXUI_MOUSE_LEFT, VXUI_MOUSE_LEFT, 0 };
    for ( int frame = 0; frame < 3; frame++ )
    {
        vxui_draw_list dl = menu_frame( &ctx, 0, positions[frame], buttons[frame] );
        const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "menu", "A" ) );
        ASSERT_TRUE( c );
        ASSERT_EQ( ( c->state & VXUI_DRAW_PRESSED ) != 0, frame == 0 );
        ASSERT_EQ( ( c->state & VXUI_DRAW_HOVERED ) != 0, frame == 0 );
        ASSERT_EQ( ctx.mouse_press_row_id, frame < 2 ? row_id( "menu", "A" ) : 0u );
        ASSERT_EQ( ctx.mouse_click_row_id, 0u );
    }
}

UTEST(draw_state, clip_rejection_and_scrolled_focus_share_the_same_packet) {
    vxui_ctx ctx = make_ctx();
    menu_frame( &ctx, 0, { 4, 40 }, 0, 1 );
    ASSERT_EQ( ctx.mouse_press_row_id, 0u );
    vxui_draw_list dl = menu_frame( &ctx, VXUI_INPUT_DOWN, { 4, 16 }, VXUI_MOUSE_LEFT, 1 );

    const vxui_draw_cmd* b = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "menu", "B" ) );
    const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "menu", "C" ) );
    ASSERT_TRUE( b && c );
    ASSERT_TRUE( b->clipped );
    ASSERT_EQ( b->row_index, 1 );
    ASSERT_EQ( b->focused_row_index, 1 );
    ASSERT_TRUE( b->state & VXUI_DRAW_FOCUSED );
    ASSERT_TRUE( b->state & VXUI_DRAW_HOVERED );
    ASSERT_TRUE( b->state & VXUI_DRAW_PRESSED );
    ASSERT_EQ( ctx.mouse_press_row_id, b->id );
    ASSERT_FALSE( c->state & VXUI_DRAW_HOVERED );
    ASSERT_FALSE( c->state & VXUI_DRAW_PRESSED );
}

UTEST(draw_state, resolver_observes_final_focus_after_a_later_row_click) {
    vxui_ctx ctx = make_ctx();
    menu_frame( &ctx, 0, { FLT_MAX, FLT_MAX }, 0 );
    vxui_draw_cmd snapshots[6] = {};
    s_snapshot_count = 0;
    vxui_set_render_data_fn( &ctx, snapshot_state, snapshots );
    vxui_frame( &ctx, 1.0f / 60.0f );
    ctx.mouse_click_row_id = row_id( "menu", "C" );
    vxui_menu( &ctx, "menu" );
    vxui_menu_action( &ctx, "A" );
    vxui_menu_action( &ctx, "B" );
    vxui_menu_action( &ctx, "C" );
    vxui_menu_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( s_snapshot_count, dl.count );
    for ( int i = 0; i < dl.count; i++ )
    {
        ASSERT_EQ( snapshots[i].id, dl.cmds[i].id );
        ASSERT_EQ( snapshots[i].state, dl.cmds[i].state );
        if ( snapshots[i].type != VXUI_DRAW_RECT ) continue;
        ASSERT_EQ( snapshots[i].focused_row_index, 2 );
        ASSERT_EQ( ( snapshots[i].state & VXUI_DRAW_FOCUSED ) != 0, snapshots[i].row_index == 2 );
    }
}

UTEST(draw_state, empty_frame_and_empty_menu_do_not_latch_stale_mouse_targets) {
    vxui_ctx ctx = make_ctx();
    menu_frame( &ctx, 0, { 4, 4 }, VXUI_MOUSE_LEFT );
    ASSERT_NE( ctx.mouse_press_row_id, 0u );
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_mouse( &ctx, 4, 4, 0 );
    vxui_menu( &ctx, "empty" );
    vxui_menu_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 0 );
    ASSERT_EQ( ctx.mouse_press_row_id, 0u );
    ASSERT_EQ( ctx.mouse_click_row_id, 0u );
}

UTEST(draw_state, every_menu_resolves_both_rows_at_menu_capacity) {
    vxui_ctx ctx = make_ctx();
    char names[VXUI_MAX_MENUS][32] = {};
    char roots[VXUI_MAX_MENUS][16] = {};
    for ( int frame = 0; frame < 2; frame++ )
    {
        vxui_frame( &ctx, 1.0f / 60.0f );
        ctx.input = VXUI_INPUT_CONFIRM | ( frame == 1 ? VXUI_INPUT_DOWN : 0 );
        for ( int m = 0; m < VXUI_MAX_MENUS; m++ )
        {
            snprintf( names[m], sizeof( names[m] ), "state.menu.%d.panel", m );
            snprintf( roots[m], sizeof( roots[m] ), "root%d", m );
            vxui_root( &ctx, roots[m], (float) ( m % 8 ) * 80, (float) ( m / 8 ) * 64 );
            vxui_menu( &ctx, names[m] );
            vxui_menu_action( &ctx, "A" );
            vxui_menu_action( &ctx, "B" );
            vxui_menu_end( &ctx );
            vxui_root_end( &ctx );
        }
        vxui_draw_list dl = vxui_render( &ctx );

        ASSERT_EQ( ctx.focused_row_count, VXUI_MAX_MENUS );
        ASSERT_EQ( ctx.pressed_row_count, VXUI_MAX_MENUS );
        for ( int m = 0; m < VXUI_MAX_MENUS; m++ )
        {
            for ( int row = 0; row < 2; row++ )
            {
                const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( names[m], row == 0 ? "A" : "B" ) );
                ASSERT_TRUE( c );
                ASSERT_EQ( c->row_index, row );
                ASSERT_EQ( c->focused_row_index, frame );
                ASSERT_EQ( ( c->state & VXUI_DRAW_FOCUSED ) != 0, row == frame );
                ASSERT_EQ( ( c->state & VXUI_DRAW_PRESSED ) != 0, row == frame );
                ASSERT_EQ( c->focus_offset_y, row == frame ? ctx.menu_focus_spring[m].x : 0.0f );
            }
        }
    }
}

UTEST_MAIN();
