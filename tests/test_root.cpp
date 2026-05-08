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

static const vxui_draw_cmd* find_id( const vxui_draw_list& dl, uint32_t id )
{
    return vxui_draw_find( dl, VXUI_DRAW_RECT, id );
}

UTEST(root, menu_inside_root_positioned_at_offset) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_root( &ctx, "hud", 200.0f, 100.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_end( &ctx );
    }
    vxui_root_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 1 );
    const vxui_draw_cmd* play = find_id( dl, row_id( "m", "Play" ) );
    ASSERT_TRUE( play != nullptr );
    ASSERT_NEAR( play->rect.x, 200.0f, 1e-3f );
    ASSERT_NEAR( play->rect.y, 100.0f, 1e-3f );
}

UTEST(root, two_roots_independent) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_root( &ctx, "left", 100.0f, 50.0f );
    if ( vxui_menu( &ctx, "left_menu" ) )
    {
        vxui_menu_action( &ctx, "L" );
        vxui_menu_end( &ctx );
    }
    vxui_root_end( &ctx );

    vxui_root( &ctx, "right", 800.0f, 400.0f );
    if ( vxui_menu( &ctx, "right_menu" ) )
    {
        vxui_menu_action( &ctx, "R" );
        vxui_menu_end( &ctx );
    }
    vxui_root_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* l = find_id( dl, row_id( "left_menu",  "L" ) );
    const vxui_draw_cmd* r = find_id( dl, row_id( "right_menu", "R" ) );
    ASSERT_TRUE( l != nullptr );
    ASSERT_TRUE( r != nullptr );
    ASSERT_NEAR( l->rect.x, 100.0f, 1e-3f );
    ASSERT_NEAR( l->rect.y,  50.0f, 1e-3f );
    ASSERT_NEAR( r->rect.x, 800.0f, 1e-3f );
    ASSERT_NEAR( r->rect.y, 400.0f, 1e-3f );
}

UTEST(root, root_ignores_wrapping_div_padding) {
    vxui_ctx ctx = make_ctx();

    vxui_div_cfg outer = {};
    outer.padding[0]   = 50;    // left
    outer.padding[2]   = 30;    // top
    outer.col          = true;

    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_div( &ctx, "outer", outer );
    vxui_root( &ctx, "hud", 200.0f, 100.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_end( &ctx );
    }
    vxui_root_end( &ctx );
    vxui_div_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* play = find_id( dl, row_id( "m", "Play" ) );
    ASSERT_TRUE( play != nullptr );
    ASSERT_NEAR( play->rect.x, 200.0f, 1e-3f );
    ASSERT_NEAR( play->rect.y, 100.0f, 1e-3f );
}

UTEST(root, two_menus_in_same_root_stack_vertically_via_menu_layout) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_root( &ctx, "hud", 100.0f, 100.0f );
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
    vxui_root_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* a = find_id( dl, row_id( "top",    "A" ) );
    const vxui_draw_cmd* b = find_id( dl, row_id( "bottom", "B" ) );
    ASSERT_TRUE( a != nullptr );
    ASSERT_TRUE( b != nullptr );
    ASSERT_NEAR( a->rect.x, 100.0f, 1e-3f );
    ASSERT_NEAR( a->rect.y, 100.0f, 1e-3f );
}

UTEST(root, navigation_works_inside_root) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_root( &ctx, "hud", 200.0f, 200.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_action( &ctx, "Quit" );
        vxui_menu_end( &ctx );
    }
    vxui_root_end( &ctx );
    vxui_render( &ctx );

    vxui_frame( &ctx, 1.0f / 60.0f );
    ctx.input = VXUI_INPUT_DOWN;
    vxui_root( &ctx, "hud", 200.0f, 200.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_action( &ctx, "Quit" );
        vxui_menu_end( &ctx );
    }
    vxui_root_end( &ctx );
    vxui_render( &ctx );

    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 1 );
}

UTEST(root, focused_row_positioned_correctly_inside_root) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_root( &ctx, "hud", 300.0f, 250.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_action( &ctx, "Quit" );
        vxui_menu_end( &ctx );
    }
    vxui_root_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 2 );
    const vxui_draw_cmd* play = find_id( dl, row_id( "m", "Play" ) );
    ASSERT_TRUE( play != nullptr );
    ASSERT_NEAR( play->rect.y, 250.0f, 1e-3f );
    ASSERT_TRUE( ( play->state & VXUI_DRAW_FOCUSED ) != 0 );
}

UTEST(root, root_id_is_hashed_independently_of_other_roots) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_root( &ctx, "score", 50.0f, 50.0f );
    if ( vxui_menu( &ctx, "score_menu" ) )
    {
        vxui_menu_action( &ctx, "X" );
        vxui_menu_end( &ctx );
    }
    vxui_root_end( &ctx );

    vxui_root( &ctx, "lives", 50.0f, 50.0f );
    if ( vxui_menu( &ctx, "lives_menu" ) )
    {
        vxui_menu_action( &ctx, "X" );
        vxui_menu_end( &ctx );
    }
    vxui_root_end( &ctx );
    vxui_render( &ctx );

    // Different menu ids -> different row ids even with same label.
    ASSERT_NE( row_id( "score_menu", "X" ), row_id( "lives_menu", "X" ) );
}

UTEST_MAIN();
