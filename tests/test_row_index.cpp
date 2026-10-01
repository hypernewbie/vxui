#include "utest.h"
#include "../vxui.h"
#define VXUI_IMPL
#include "../vxui_impl.h"

static uint8_t s_clay_mem[16 * 1024 * 1024];

static uint32_t row_id( const char* menu, const char* label )
{
    Clay_String cs = { false, (int32_t) strlen( label ), label };
    return Clay__HashString( cs, vxui_hash( menu ) ).id;
}

static void init_ctx( vxui_ctx* ctx )
{
    *ctx = {};
    vxui_init( ctx, 1280, 720, s_clay_mem, sizeof( s_clay_mem ) );
}

static vxui_draw_list action_frame( vxui_ctx* ctx, uint32_t input, uint32_t click_row_id = 0 )
{
    vxui_frame( ctx, 1.0f / 60.0f );
    ctx->input              = input;
    ctx->mouse_click_row_id = click_row_id;
    if ( vxui_menu( ctx, "m" ) )
    {
        vxui_menu_action( ctx, "A" );
        vxui_menu_action( ctx, "B" );
        vxui_menu_action( ctx, "C" );
        vxui_menu_end( ctx );
    }
    return vxui_render( ctx );
}

static void resolve_row_indices( const vxui_draw_cmd* c, vxui_render_data* out, void* )
{
    out->params[0] = (float) c->row_index;
    out->params[1] = (float) c->focused_row_index;
}

UTEST(row_index, three_rows_get_indices_zero_one_two) {
    vxui_ctx ctx; init_ctx( &ctx );
    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "A" );
        vxui_menu_action( &ctx, "B" );
        vxui_menu_action( &ctx, "C" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* a = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "m", "A" ) );
    const vxui_draw_cmd* b = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "m", "B" ) );
    const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "m", "C" ) );
    ASSERT_TRUE( a && b && c );
    ASSERT_EQ( a->row_index, 0 );
    ASSERT_EQ( b->row_index, 1 );
    ASSERT_EQ( c->row_index, 2 );
}

UTEST(row_index, focused_row_index_matches_menu_state) {
    vxui_ctx ctx; init_ctx( &ctx );

    auto frame = []( vxui_ctx* ctx, uint32_t input ) {
        vxui_frame( ctx, 1.0f / 60.0f );
        ctx->input = input;
        if ( vxui_menu( ctx, "m" ) )
        {
            vxui_menu_action( ctx, "A" );
            vxui_menu_action( ctx, "B" );
            vxui_menu_action( ctx, "C" );
            vxui_menu_end( ctx );
        }
        vxui_render( ctx );
    };

    frame( &ctx, 0 );
    frame( &ctx, VXUI_INPUT_DOWN );
    vxui_draw_list dl = ctx.draw_list;

    const vxui_draw_cmd* a = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "m", "A" ) );
    const vxui_draw_cmd* b = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "m", "B" ) );
    ASSERT_TRUE( a && b );
    ASSERT_EQ( a->focused_row_index, 1 );
    ASSERT_EQ( b->focused_row_index, 1 );
}

UTEST(row_index, section_and_label_count_as_rows) {
    vxui_ctx ctx; init_ctx( &ctx );
    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_section( &ctx, "S" );
        vxui_menu_action ( &ctx, "A" );
        vxui_menu_label  ( &ctx, "L" );
        vxui_menu_action ( &ctx, "B" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* s = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "m", "S" ) );
    const vxui_draw_cmd* a = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "m", "A" ) );
    const vxui_draw_cmd* l = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "m", "L" ) );
    const vxui_draw_cmd* b = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "m", "B" ) );
    ASSERT_TRUE( s && a && l && b );
    ASSERT_EQ( s->row_index, 0 );
    ASSERT_EQ( a->row_index, 1 );
    ASSERT_EQ( l->row_index, 2 );
    ASSERT_EQ( b->row_index, 3 );
}

UTEST(row_index, panel_rect_has_minus_one) {
    vxui_ctx ctx; init_ctx( &ctx );
    action_frame( &ctx, 0 );
    action_frame( &ctx, 0, row_id( "m", "C" ) );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 2 );
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_rect( &ctx, "panel", { .width = { VXUI_FIXED, 100 }, .height = { VXUI_FIXED, 100 } } );
    vxui_div_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* p = vxui_draw_nth( dl, VXUI_DRAW_RECT, 0 );
    ASSERT_TRUE( p != nullptr );
    ASSERT_EQ( p->row_index,         -1 );
    ASSERT_EQ( p->focused_row_index, -1 );
    ASSERT_EQ( ctx.frame_row_count, 0 );
}

UTEST(row_index, two_menus_have_independent_indices) {
    vxui_ctx ctx; init_ctx( &ctx );
    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "left"  ) ) { vxui_menu_action( &ctx, "L0" ); vxui_menu_action( &ctx, "L1" ); vxui_menu_end( &ctx ); }
    if ( vxui_menu( &ctx, "right" ) ) { vxui_menu_action( &ctx, "R0" ); vxui_menu_action( &ctx, "R1" ); vxui_menu_end( &ctx ); }
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* l0 = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "left",  "L0" ) );
    const vxui_draw_cmd* l1 = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "left",  "L1" ) );
    const vxui_draw_cmd* r0 = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "right", "R0" ) );
    const vxui_draw_cmd* r1 = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "right", "R1" ) );
    ASSERT_TRUE( l0 && l1 && r0 && r1 );
    ASSERT_EQ( l0->row_index, 0 );
    ASSERT_EQ( l1->row_index, 1 );
    ASSERT_EQ( r0->row_index, 0 );
    ASSERT_EQ( r1->row_index, 1 );
}

UTEST(row_index, frame_count_resets_each_frame) {
    vxui_ctx ctx; init_ctx( &ctx );
    for ( int i = 0; i < 5; i++ )
    {
        vxui_frame( &ctx, 1.0f / 60.0f );
        if ( vxui_menu( &ctx, "m" ) )
        {
            vxui_menu_action( &ctx, "A" );
            vxui_menu_action( &ctx, "B" );
            vxui_menu_end( &ctx );
        }
        vxui_render( &ctx );
        ASSERT_EQ( ctx.frame_row_count, 2 );
    }
}

UTEST(row_index, leading_section_uses_final_focus) {
    vxui_ctx ctx; init_ctx( &ctx );
    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_section( &ctx, "S" );
        vxui_menu_action ( &ctx, "A" );
        vxui_menu_action ( &ctx, "B" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 1 );
    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 3 );
    for ( int i = 0; i < 3; i++ )
    {
        const vxui_draw_cmd* c = vxui_draw_nth( dl, VXUI_DRAW_RECT, i );
        EXPECT_EQ( c->row_index, i );
        EXPECT_EQ( c->focused_row_index, 1 );
        EXPECT_EQ( ( c->state & VXUI_DRAW_FOCUSED ) != 0, i == 1 );
    }
}

UTEST(row_index, leading_label_uses_final_focus) {
    vxui_ctx ctx; init_ctx( &ctx );
    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_label ( &ctx, "L" );
        vxui_menu_action( &ctx, "A" );
        vxui_menu_action( &ctx, "B" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 1 );
    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 3 );
    for ( int i = 0; i < 3; i++ )
    {
        const vxui_draw_cmd* c = vxui_draw_nth( dl, VXUI_DRAW_RECT, i );
        EXPECT_EQ( c->row_index, i );
        EXPECT_EQ( c->focused_row_index, 1 );
        EXPECT_EQ( ( c->state & VXUI_DRAW_FOCUSED ) != 0, i == 1 );
    }
}

UTEST(row_index, leading_skip_rows_use_final_focus) {
    vxui_ctx ctx; init_ctx( &ctx );
    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_section( &ctx, "S" );
        vxui_menu_label  ( &ctx, "L" );
        vxui_menu_action ( &ctx, "A" );
        vxui_menu_action ( &ctx, "B" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 2 );
    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 4 );
    for ( int i = 0; i < 4; i++ )
    {
        const vxui_draw_cmd* c = vxui_draw_nth( dl, VXUI_DRAW_RECT, i );
        EXPECT_EQ( c->row_index, i );
        EXPECT_EQ( c->focused_row_index, 2 );
        EXPECT_EQ( ( c->state & VXUI_DRAW_FOCUSED ) != 0, i == 2 );
    }
}

UTEST(row_index, carried_click_to_later_row_updates_all_indices) {
    vxui_ctx ctx; init_ctx( &ctx );
    action_frame( &ctx, 0 );
    vxui_draw_list dl = action_frame( &ctx, 0, row_id( "m", "C" ) );

    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 2 );
    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 3 );
    for ( int i = 0; i < 3; i++ )
    {
        const vxui_draw_cmd* c = vxui_draw_nth( dl, VXUI_DRAW_RECT, i );
        EXPECT_EQ( c->row_index, i );
        EXPECT_EQ( c->focused_row_index, 2 );
        EXPECT_EQ( ( c->state & VXUI_DRAW_FOCUSED ) != 0, i == 2 );
    }
}

UTEST(row_index, carried_click_to_earlier_row_updates_all_indices) {
    vxui_ctx ctx; init_ctx( &ctx );
    action_frame( &ctx, 0 );
    action_frame( &ctx, 0, row_id( "m", "C" ) );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 2 );
    vxui_draw_list dl = action_frame( &ctx, 0, row_id( "m", "A" ) );

    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 0 );
    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 3 );
    for ( int i = 0; i < 3; i++ )
    {
        const vxui_draw_cmd* c = vxui_draw_nth( dl, VXUI_DRAW_RECT, i );
        EXPECT_EQ( c->row_index, i );
        EXPECT_EQ( c->focused_row_index, 0 );
        EXPECT_EQ( ( c->state & VXUI_DRAW_FOCUSED ) != 0, i == 0 );
    }
}

UTEST(row_index, same_labels_in_two_menus_use_their_own_final_focus) {
    vxui_ctx ctx; init_ctx( &ctx );
    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "left" ) )
    {
        vxui_menu_section( &ctx, "S" );
        vxui_menu_action ( &ctx, "A" );
        vxui_menu_action ( &ctx, "B" );
        vxui_menu_end( &ctx );
    }
    if ( vxui_menu( &ctx, "right" ) )
    {
        vxui_menu_label  ( &ctx, "L" );
        vxui_menu_section( &ctx, "S" );
        vxui_menu_action ( &ctx, "A" );
        vxui_menu_action ( &ctx, "B" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 1 );
    ASSERT_EQ( ctx.menu_state[1].y, (uint32_t) 2 );
    const char* labels[] = { "S", "A", "B" };
    for ( int i = 0; i < 3; i++ )
    {
        const vxui_draw_cmd* left  = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "left", labels[i] ) );
        const vxui_draw_cmd* right = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "right", labels[i] ) );
        ASSERT_TRUE( left && right );
        EXPECT_EQ( left->row_index, i );
        EXPECT_EQ( right->row_index, i + 1 );
        EXPECT_EQ( left->focused_row_index, 1 );
        EXPECT_EQ( right->focused_row_index, 2 );
        EXPECT_EQ( ( left->state & VXUI_DRAW_FOCUSED ) != 0, i == 1 );
        EXPECT_EQ( ( right->state & VXUI_DRAW_FOCUSED ) != 0, i == 1 );
    }
}

UTEST(row_index, reordered_menus_keep_their_own_final_focus) {
    vxui_ctx ctx; init_ctx( &ctx );
    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "left" ) )
    {
        vxui_menu_action( &ctx, "A" );
        vxui_menu_end( &ctx );
    }
    if ( vxui_menu( &ctx, "right" ) )
    {
        vxui_menu_action( &ctx, "A" );
        vxui_menu_action( &ctx, "B" );
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );

    vxui_frame( &ctx, 1.0f / 60.0f );
    ctx.mouse_click_row_id = row_id( "right", "B" );
    if ( vxui_menu( &ctx, "right" ) )
    {
        vxui_menu_action( &ctx, "A" );
        vxui_menu_action( &ctx, "B" );
        vxui_menu_end( &ctx );
    }
    if ( vxui_menu( &ctx, "left" ) )
    {
        vxui_menu_action( &ctx, "A" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 0 );
    ASSERT_EQ( ctx.menu_state[1].y, (uint32_t) 1 );
    const vxui_draw_cmd* left  = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "left", "A" ) );
    const vxui_draw_cmd* right = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "right", "A" ) );
    const vxui_draw_cmd* b     = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "right", "B" ) );
    ASSERT_TRUE( left && right && b );
    EXPECT_EQ( left->row_index, 0 );
    EXPECT_EQ( right->row_index, 0 );
    EXPECT_EQ( b->row_index, 1 );
    EXPECT_EQ( left->focused_row_index, 0 );
    EXPECT_EQ( right->focused_row_index, 1 );
    EXPECT_EQ( b->focused_row_index, 1 );
    EXPECT_TRUE( ( left->state & VXUI_DRAW_FOCUSED ) != 0 );
    EXPECT_EQ( right->state & VXUI_DRAW_FOCUSED, (uint8_t) 0 );
    EXPECT_TRUE( ( b->state & VXUI_DRAW_FOCUSED ) != 0 );
}

UTEST(row_index, resolver_sees_final_focus_on_every_row) {
    vxui_ctx ctx; init_ctx( &ctx );
    vxui_set_render_data_fn( &ctx, resolve_row_indices, nullptr );
    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_section( &ctx, "S" );
        vxui_menu_action ( &ctx, "A" );
        vxui_menu_action ( &ctx, "B" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 1 );
    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 3 );
    for ( int i = 0; i < 3; i++ )
    {
        const vxui_draw_cmd* c = vxui_draw_nth( dl, VXUI_DRAW_RECT, i );
        EXPECT_EQ( c->render.params[0], (float) i );
        EXPECT_EQ( c->render.params[1], 1.0f );
    }
}

UTEST(row_index, text_cmds_do_not_get_row_indices) {
    vxui_ctx ctx; init_ctx( &ctx );
    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_section( &ctx, "S" );
        vxui_menu_action ( &ctx, "A" );
        vxui_menu_end( &ctx );
    }
    vxui_text( &ctx, "outside", "Outside" );
    vxui_draw_list dl = vxui_render( &ctx );

    int n = vxui_draw_count( dl, VXUI_DRAW_TEXT );
    ASSERT_GT( n, 0 );
    for ( int i = 0; i < n; i++ )
    {
        const vxui_draw_cmd* c = vxui_draw_nth( dl, VXUI_DRAW_TEXT, i );
        EXPECT_EQ( c->row_index, -1 );
        EXPECT_EQ( c->focused_row_index, -1 );
        EXPECT_EQ( c->state & VXUI_DRAW_FOCUSED, (uint8_t) 0 );
    }
}

UTEST(row_index, all_skipped_rows_remain_unfocused) {
    vxui_ctx ctx; init_ctx( &ctx );
    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_section( &ctx, "S" );
        vxui_menu_label  ( &ctx, "L" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( ctx.focused_row_count, 0 );
    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 2 );
    for ( int i = 0; i < 2; i++ )
    {
        const vxui_draw_cmd* c = vxui_draw_nth( dl, VXUI_DRAW_RECT, i );
        EXPECT_EQ( c->row_index, i );
        EXPECT_EQ( c->focused_row_index, (int) ctx.menu_state[0].y );
        EXPECT_EQ( c->state & VXUI_DRAW_FOCUSED, (uint8_t) 0 );
    }
}

UTEST_MAIN();
