#include "utest.h"
#include "../vxui.h"
#define VXUI_IMPL
#include "../vxui_impl.h"

static uint8_t s_clay_mem[16 * 1024 * 1024];

static Clay_Dimensions measure_text( Clay_StringSlice s, Clay_TextElementConfig* cfg, void* )
{
    return { (float) s.length * 8.0f, (float) cfg->fontSize };
}

static vxui_ctx make_ctx()
{
    vxui_ctx ctx = {};
    vxui_init( &ctx, 4096, 2048, s_clay_mem, sizeof( s_clay_mem ) );
    Clay_SetMeasureTextFunction( measure_text, nullptr );
    return ctx;
}

static uint32_t row_id( const char* menu, const char* label )
{
    return Clay__HashString( { false, (int32_t) strlen( label ), label }, vxui_hash( menu ) ).id;
}

static vxui_draw_list menu_frame( vxui_ctx* ctx, uint32_t input, float dt = 1.0f / 60.0f, int visible = 0 )
{
    vxui_frame( ctx, dt );
    ctx->input = input;
    vxui_menu( ctx, "menu", true, visible );
    vxui_menu_action( ctx, "A" );
    vxui_menu_action( ctx, "B" );
    vxui_menu_action( ctx, "C" );
    vxui_menu_end( ctx );
    return vxui_render( ctx );
}

static void capture_focus( const vxui_draw_cmd* c, vxui_render_data* out, void* )
{
    out->params[0] = ( c->state & VXUI_DRAW_FOCUSED ) != 0 ? 1.0f : 0.0f;
    out->params[1] = c->focus_offset_y;
    out->params[2] = (float) c->focused_row_index;
}

UTEST(menu_focus_owner, focus_and_offset_follow_the_same_menu_spring_during_navigation) {
    vxui_ctx ctx = make_ctx();
    uint32_t inputs[] = { 0, VXUI_INPUT_DOWN, 0, VXUI_INPUT_DOWN, 0, VXUI_INPUT_UP };
    for ( int frame = 0; frame < 6; frame++ )
    {
        vxui_draw_list dl = menu_frame( &ctx, inputs[frame] );
        ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 3 );
        for ( int row = 0; row < 3; row++ )
        {
            const vxui_draw_cmd* c = vxui_draw_nth( dl, VXUI_DRAW_RECT, row );
            bool focused = row == (int) ctx.menu_state[0].y;
            ASSERT_EQ( ( c->state & VXUI_DRAW_FOCUSED ) != 0, focused );
            ASSERT_EQ( c->focus_offset_y, focused ? ctx.menu_focus_spring[0].x : 0.0f );
            ASSERT_EQ( c->focused_row_index, (int) ctx.menu_state[0].y );
        }
    }
}

UTEST(menu_focus_owner, option_and_slider_share_focus_truth_but_skip_rows_do_not) {
    vxui_ctx ctx = make_ctx();
    const char* options[] = { "Off", "On" };
    int index = 0;
    float value = 0.5f;
    for ( int frame = 0; frame < 2; frame++ )
    {
        vxui_frame( &ctx, 1.0f / 60.0f );
        ctx.input = frame == 0 ? 0 : VXUI_INPUT_DOWN;
        vxui_menu( &ctx, "mixed" );
        vxui_menu_section( &ctx, "section" );
        vxui_menu_option( &ctx, "option", &index, options, 2 );
        vxui_menu_label( &ctx, "label" );
        vxui_menu_slider( &ctx, "slider", &value );
        vxui_menu_end( &ctx );
        vxui_draw_list dl = vxui_render( &ctx );
        int focus = frame == 0 ? 1 : 3;
        for ( int row = 0; row < 4; row++ )
        {
            const vxui_draw_cmd* c = vxui_draw_nth( dl, VXUI_DRAW_RECT, row );
            ASSERT_EQ( ( c->state & VXUI_DRAW_FOCUSED ) != 0, row == focus );
            ASSERT_EQ( c->focused_row_index, focus );
        }
    }
}

UTEST(menu_focus_owner, all_skipped_rows_have_no_focus_or_offset_even_with_old_spring_history) {
    vxui_ctx ctx = make_ctx();
    menu_frame( &ctx, 0 );
    menu_frame( &ctx, VXUI_INPUT_DOWN );
    ASSERT_NE( ctx.menu_focus_spring[0].x, 0.0f );
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_menu( &ctx, "menu" );
    vxui_menu_section( &ctx, "A" );
    vxui_menu_label( &ctx, "B" );
    vxui_menu_label( &ctx, "C" );
    vxui_menu_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );
    for ( int i = 0; i < dl.count; i++ )
    {
        ASSERT_FALSE( dl.cmds[i].state & VXUI_DRAW_FOCUSED );
        ASSERT_EQ( dl.cmds[i].focus_offset_y, 0.0f );
    }
}

UTEST(menu_focus_owner, row_count_shrink_does_not_focus_a_row_that_was_not_declared) {
    vxui_ctx ctx = make_ctx();
    menu_frame( &ctx, 0 );
    ctx.menu_state[0].y = 2;
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_menu( &ctx, "menu" );
    vxui_menu_action( &ctx, "A" );
    vxui_menu_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 2 );
    ASSERT_FALSE( vxui_draw_nth( dl, VXUI_DRAW_RECT, 0 )->state & VXUI_DRAW_FOCUSED );
    dl = menu_frame( &ctx, 0 );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 0 );
    ASSERT_TRUE( vxui_draw_nth( dl, VXUI_DRAW_RECT, 0 )->state & VXUI_DRAW_FOCUSED );
}

UTEST(menu_focus_owner, remounted_focus_keeps_its_row_and_resets_its_displacement) {
    vxui_ctx ctx = make_ctx();
    menu_frame( &ctx, 0 );
    menu_frame( &ctx, VXUI_INPUT_DOWN );
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_render( &ctx );
    vxui_draw_list dl = menu_frame( &ctx, 0 );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 1 );
    const vxui_draw_cmd* c = vxui_draw_nth( dl, VXUI_DRAW_RECT, 1 );
    ASSERT_TRUE( c->state & VXUI_DRAW_FOCUSED );
    ASSERT_EQ( c->focus_offset_y, 0.0f );
    ASSERT_EQ( ctx.menu_focus_spring[0].y, 0.0f );
}

UTEST(menu_focus_owner, focus_can_be_scrolled_or_clipped_without_becoming_unfocused) {
    vxui_ctx ctx = make_ctx();
    menu_frame( &ctx, 0, 1.0f / 60.0f, 1 );
    vxui_draw_list dl = menu_frame( &ctx, VXUI_INPUT_DOWN, 1.0f / 60.0f, 1 );
    const vxui_draw_cmd* c = vxui_draw_nth( dl, VXUI_DRAW_RECT, 1 );
    ASSERT_TRUE( c->clipped );
    ASSERT_TRUE( c->state & VXUI_DRAW_FOCUSED );
    ASSERT_EQ( c->row_index, 1 );
    ASSERT_EQ( c->focused_row_index, 1 );
    ASSERT_EQ( c->focus_offset_y, ctx.menu_focus_spring[0].x );
}

UTEST(menu_focus_owner, non_menu_rectangles_and_text_do_not_inherit_focus_or_offsets) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_menu( &ctx, "menu" );
    vxui_menu_action( &ctx, "A" );
    vxui_menu_end( &ctx );
    vxui_rect( &ctx, "ordinary", { .width = { VXUI_FIXED, 20 }, .height = { VXUI_FIXED, 20 } } );
    vxui_div_end( &ctx );
    vxui_text( &ctx, "text", "A", 24 );
    vxui_draw_list dl = vxui_render( &ctx );
    int focused = 0;
    for ( int i = 0; i < dl.count; i++ )
    {
        const vxui_draw_cmd& c = dl.cmds[i];
        if ( c.type == VXUI_DRAW_RECT && c.row_index >= 0 )
        {
            ASSERT_TRUE( c.state & VXUI_DRAW_FOCUSED );
            focused++;
            continue;
        }
        ASSERT_FALSE( c.state & VXUI_DRAW_FOCUSED );
        ASSERT_EQ( c.focus_offset_y, 0.0f );
    }
    ASSERT_EQ( focused, 1 );
}

UTEST(menu_focus_owner, all_menu_slots_use_their_own_final_row_and_spring) {
    vxui_ctx ctx = make_ctx();
    char names[VXUI_MAX_MENUS][40] = {};
    for ( int frame = 0; frame < 2; frame++ )
    {
        vxui_frame( &ctx, 1.0f / 60.0f );
        for ( int i = 0; i < VXUI_MAX_MENUS; i++ )
        {
            snprintf( names[i], sizeof( names[i] ), "focus.owner.%d.panel", i );
            if ( frame == 1 ) ctx.menu_state[i].y = (uint32_t) ( i % 2 );
            vxui_menu( &ctx, names[i] );
            vxui_menu_action( &ctx, "A" );
            vxui_menu_action( &ctx, "B" );
            vxui_menu_end( &ctx );
        }
        vxui_draw_list dl = vxui_render( &ctx );
        for ( int i = 0; i < VXUI_MAX_MENUS; i++ )
        {
            for ( int row = 0; row < 2; row++ )
            {
                const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( names[i], row == 0 ? "A" : "B" ) );
                ASSERT_TRUE( c );
                bool focused = row == (int) ctx.menu_state[i].y;
                ASSERT_EQ( ( c->state & VXUI_DRAW_FOCUSED ) != 0, focused );
                ASSERT_EQ( c->focus_offset_y, focused ? ctx.menu_focus_spring[i].x : 0.0f );
            }
        }
    }
}

UTEST(menu_focus_owner, final_state_changes_after_menu_end_reach_the_resolver_consistently) {
    vxui_ctx ctx = make_ctx();
    vxui_set_render_data_fn( &ctx, capture_focus, nullptr );
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_menu( &ctx, "menu" );
    vxui_menu_action( &ctx, "A" );
    vxui_menu_action( &ctx, "B" );
    vxui_menu_end( &ctx );
    ctx.menu_state[0].y = 1;
    vxui_draw_list dl = vxui_render( &ctx );
    for ( int i = 0; i < 2; i++ )
    {
        const vxui_draw_cmd* c = vxui_draw_nth( dl, VXUI_DRAW_RECT, i );
        ASSERT_EQ( c->focused_row_index, 1 );
        ASSERT_EQ( ( c->state & VXUI_DRAW_FOCUSED ) != 0, i == 1 );
        ASSERT_EQ( c->render.params[0], i == 1 ? 1.0f : 0.0f );
        ASSERT_EQ( c->render.params[2], 1.0f );
    }
}

UTEST(menu_focus_owner, final_spring_value_is_not_an_earlier_offset_copy) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_menu( &ctx, "menu" );
    vxui_menu_action( &ctx, "A" );
    vxui_menu_end( &ctx );
    ctx.menu_focus_spring[0].x = 7.25f;
    vxui_draw_list dl = vxui_render( &ctx );
    const vxui_draw_cmd* c = vxui_draw_nth( dl, VXUI_DRAW_RECT, 0 );
    ASSERT_TRUE( c->state & VXUI_DRAW_FOCUSED );
    ASSERT_EQ( c->focus_offset_y, 7.25f );
}

UTEST_MAIN();
