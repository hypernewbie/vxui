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

static uint32_t row_id( const char* menu, const char* label )
{
    Clay_String cs = { false, (int32_t) strlen( label ), label };
    return Clay__HashString( cs, vxui_hash( menu ) ).id;
}

UTEST(menu_in_div, rows_offset_by_div_padding) {
    vxui_ctx ctx = make_ctx();

    vxui_div_cfg outer = {};
    outer.padding[0]   = 50;    // left
    outer.padding[2]   = 30;    // top
    outer.col          = true;

    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_div( &ctx, "outer", outer );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_action( &ctx, "Quit" );
        vxui_menu_end( &ctx );
    }
    vxui_div_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 2 );
    ASSERT_NEAR( vxui_draw_nth( dl, VXUI_DRAW_RECT, 0 )->rect.x, 50.0f, 1e-3f );
    ASSERT_NEAR( vxui_draw_nth( dl, VXUI_DRAW_RECT, 0 )->rect.y, 30.0f, 1e-3f );
    ASSERT_NEAR( vxui_draw_nth( dl, VXUI_DRAW_RECT, 1 )->rect.x, 50.0f, 1e-3f );
    ASSERT_NEAR( vxui_draw_nth( dl, VXUI_DRAW_RECT, 1 )->rect.y, 30.0f + (float) VXUI_ROW_HEIGHT, 1e-3f );
}

UTEST(menu_in_div, focused_row_positioned_correctly_in_div) {
    vxui_ctx ctx = make_ctx();

    vxui_div_cfg outer = {};
    outer.padding[0]   = 100;
    outer.padding[2]   = 80;
    outer.col          = true;

    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_div( &ctx, "outer", outer );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_action( &ctx, "Quit" );
        vxui_menu_end( &ctx );
    }
    vxui_div_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 2 );
    const vxui_draw_cmd* play = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "m", "Play" ) );
    ASSERT_TRUE( play != nullptr );
    ASSERT_TRUE( ( play->state & VXUI_DRAW_FOCUSED ) != 0 );
    ASSERT_NEAR( play->rect.x, 100.0f, 1e-3f );
    ASSERT_NEAR( play->rect.y, 80.0f,  1e-3f );
}

static const vxui_draw_cmd* find_id( const vxui_draw_list& dl, uint32_t id )
{
    return vxui_draw_find( dl, VXUI_DRAW_RECT, id );
}

UTEST(menu_in_div, two_menus_stack_in_column_div_with_gap) {
    vxui_ctx ctx = make_ctx();

    vxui_div_cfg outer = {};
    outer.col          = true;
    outer.gap          = 16;

    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_div( &ctx, "outer", outer );
    if ( vxui_menu( &ctx, "top" ) )
    {
        vxui_menu_action( &ctx, "A" );
        vxui_menu_end( &ctx );
    }
    if ( vxui_menu( &ctx, "bottom" ) )
    {
        vxui_menu_action( &ctx, "B" );
        vxui_menu_end( &ctx );
    }
    vxui_div_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 2 );

    const vxui_draw_cmd* a = find_id( dl, row_id( "top",    "A" ) );
    const vxui_draw_cmd* b = find_id( dl, row_id( "bottom", "B" ) );
    ASSERT_TRUE( a != nullptr );
    ASSERT_TRUE( b != nullptr );
    ASSERT_TRUE( ( a->state & VXUI_DRAW_FOCUSED ) != 0 );
    ASSERT_TRUE( ( b->state & VXUI_DRAW_FOCUSED ) != 0 );

    ASSERT_NEAR( a->rect.y, 0.0f,                            1e-3f );
    ASSERT_NEAR( b->rect.y, (float) VXUI_ROW_HEIGHT + 16.0f, 1e-3f );
}

UTEST(menu_in_div, menu_in_nested_divs_accumulates_padding) {
    vxui_ctx ctx = make_ctx();

    vxui_div_cfg outer = {};
    outer.padding[0]   = 20;
    outer.padding[2]   = 10;
    outer.col          = true;

    vxui_div_cfg inner = {};
    inner.padding[0]   = 5;
    inner.padding[2]   = 7;
    inner.col          = true;

    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_div( &ctx, "outer", outer );
    vxui_div( &ctx, "inner", inner );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_end( &ctx );
    }
    vxui_div_end( &ctx );
    vxui_div_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 1 );
    ASSERT_NEAR( vxui_draw_nth( dl, VXUI_DRAW_RECT, 0 )->rect.x, 25.0f, 1e-3f );
    ASSERT_NEAR( vxui_draw_nth( dl, VXUI_DRAW_RECT, 0 )->rect.y, 17.0f, 1e-3f );
}

UTEST(menu_in_div, menu_inside_row_div_still_stacks_vertically) {
    vxui_ctx ctx = make_ctx();

    vxui_div_cfg row_outer = {};
    row_outer.col          = false;

    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_div( &ctx, "row", row_outer );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_action( &ctx, "Quit" );
        vxui_menu_end( &ctx );
    }
    vxui_div_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 2 );
    ASSERT_NEAR( vxui_draw_nth( dl, VXUI_DRAW_RECT, 0 )->rect.x, vxui_draw_nth( dl, VXUI_DRAW_RECT, 1 )->rect.x,                            1e-3f );
    ASSERT_NEAR( vxui_draw_nth( dl, VXUI_DRAW_RECT, 1 )->rect.y, vxui_draw_nth( dl, VXUI_DRAW_RECT, 0 )->rect.y + (float) VXUI_ROW_HEIGHT,  1e-3f );
}

UTEST(menu_in_div, navigation_works_when_nested_in_div) {
    vxui_ctx ctx = make_ctx();

    vxui_div_cfg outer = {};
    outer.padding[0]   = 40;
    outer.padding[2]   = 40;
    outer.col          = true;

    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_div( &ctx, "outer", outer );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_action( &ctx, "Quit" );
        vxui_menu_end( &ctx );
    }
    vxui_div_end( &ctx );
    vxui_render( &ctx );

    vxui_frame( &ctx, 1.0f / 60.0f );
    ctx.input = VXUI_INPUT_DOWN;
    vxui_div( &ctx, "outer", outer );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_action( &ctx, "Quit" );
        vxui_menu_end( &ctx );
    }
    vxui_div_end( &ctx );
    vxui_render( &ctx );

    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 1 );
}

UTEST(menu_in_div, menu_at_root_versus_in_div_have_same_relative_layout) {
    vxui_ctx root_ctx = make_ctx();
    vxui_ctx div_ctx  = make_ctx();

    vxui_frame( &root_ctx, 1.0f / 60.0f );
    if ( vxui_menu( &root_ctx, "m" ) )
    {
        vxui_menu_action( &root_ctx, "A" );
        vxui_menu_action( &root_ctx, "B" );
        vxui_menu_action( &root_ctx, "C" );
        vxui_menu_end( &root_ctx );
    }
    vxui_draw_list root_dl = vxui_render( &root_ctx );

    vxui_div_cfg outer = {};
    outer.col          = true;

    vxui_frame( &div_ctx, 1.0f / 60.0f );
    vxui_div( &div_ctx, "outer", outer );
    if ( vxui_menu( &div_ctx, "m" ) )
    {
        vxui_menu_action( &div_ctx, "A" );
        vxui_menu_action( &div_ctx, "B" );
        vxui_menu_action( &div_ctx, "C" );
        vxui_menu_end( &div_ctx );
    }
    vxui_div_end( &div_ctx );
    vxui_draw_list div_dl = vxui_render( &div_ctx );

    int rect_n = vxui_draw_count( root_dl, VXUI_DRAW_RECT );
    ASSERT_EQ( rect_n, vxui_draw_count( div_dl, VXUI_DRAW_RECT ) );
    for ( int i = 0; i < rect_n; i++ )
    {
        float dy_root = vxui_draw_nth( root_dl, VXUI_DRAW_RECT, i )->rect.y - vxui_draw_nth( root_dl, VXUI_DRAW_RECT, 0 )->rect.y;
        float dy_div  = vxui_draw_nth( div_dl,  VXUI_DRAW_RECT, i )->rect.y - vxui_draw_nth( div_dl,  VXUI_DRAW_RECT, 0 )->rect.y;
        ASSERT_NEAR( dy_root, dy_div, 1e-3f );
    }
}

UTEST_MAIN();
