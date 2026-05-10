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
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_rect( &ctx, "panel", { .width = { VXUI_FIXED, 100 }, .height = { VXUI_FIXED, 100 } } );
    vxui_div_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* p = vxui_draw_nth( dl, VXUI_DRAW_RECT, 0 );
    ASSERT_TRUE( p != nullptr );
    ASSERT_EQ( p->row_index,         -1 );
    ASSERT_EQ( p->focused_row_index, -1 );
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

UTEST_MAIN();
