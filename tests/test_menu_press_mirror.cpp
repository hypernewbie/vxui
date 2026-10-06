#include "utest.h"
#include "../vxui.h"
#define VXUI_IMPL
#include "../vxui_impl.h"
#include <cstring>

static uint8_t s_clay_mem[16 * 1024 * 1024];

static Clay_Dimensions measure_text( Clay_StringSlice s, Clay_TextElementConfig* cfg, void* )
{
    return { (float) s.length * 8.0f, (float) cfg->fontSize };
}

static vxui_ctx make_ctx()
{
    vxui_ctx ctx = {};
    vxui_init( &ctx, 1280, 720, s_clay_mem, sizeof( s_clay_mem ) );
    Clay_SetMeasureTextFunction( measure_text, nullptr );
    return ctx;
}

static uint32_t row_id( const char* menu, const char* label )
{
    Clay_String cs = { false, (int32_t) strlen( label ), label };
    return Clay__HashString( cs, vxui_hash( menu ) ).id;
}

static int pressed_count( const vxui_draw_list& dl )
{
    int n = 0;
    for ( int i = 0; i < dl.count; i++ )
        if ( dl.cmds[i].state & VXUI_DRAW_PRESSED ) n++;
    return n;
}

UTEST(menu_press_mirror, confirm_hold_presses_the_focused_option) {
    vxui_ctx ctx = make_ctx();
    const char* options[] = { "Off", "On" };
    int index = 0;
    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "menu" ) )
    {
        vxui_menu_option( &ctx, "Choice", &index, options, 2 );
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );
    vxui_frame( &ctx, 1.0f / 60.0f );
    ctx.input = VXUI_INPUT_CONFIRM;
    if ( vxui_menu( &ctx, "menu" ) )
    {
        vxui_menu_option( &ctx, "Choice", &index, options, 2 );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );
    ASSERT_EQ( pressed_count( dl ), 1 );
    ASSERT_TRUE( vxui_draw_nth( dl, VXUI_DRAW_RECT, 0 )->state & VXUI_DRAW_PRESSED );
}

UTEST(menu_press_mirror, confirm_hold_presses_the_focused_slider) {
    vxui_ctx ctx = make_ctx();
    float value = 0.5f;
    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "menu" ) )
    {
        vxui_menu_slider( &ctx, "Volume", &value, 0.0f, 1.0f, 0.1f );
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );
    vxui_frame( &ctx, 1.0f / 60.0f );
    ctx.input = VXUI_INPUT_CONFIRM;
    if ( vxui_menu( &ctx, "menu" ) )
    {
        vxui_menu_slider( &ctx, "Volume", &value, 0.0f, 1.0f, 0.1f );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );
    ASSERT_EQ( pressed_count( dl ), 1 );
    ASSERT_TRUE( vxui_draw_nth( dl, VXUI_DRAW_RECT, 0 )->state & VXUI_DRAW_PRESSED );
}

UTEST(menu_press_mirror, mouse_click_presses_option_and_is_consumed) {
    vxui_ctx ctx = make_ctx();
    const char* options[] = { "Off", "On" };
    int index = 0;
    vxui_frame( &ctx, 1.0f / 60.0f );
    ctx.mouse_click_row_id = row_id( "menu", "Choice" );
    if ( vxui_menu( &ctx, "menu" ) )
    {
        vxui_menu_option( &ctx, "Choice", &index, options, 2 );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );
    ASSERT_EQ( pressed_count( dl ), 1 );
    ASSERT_EQ( ctx.mouse_click_row_id, (uint32_t) 0 );
}

UTEST(menu_press_mirror, mouse_click_presses_slider_and_is_consumed) {
    vxui_ctx ctx = make_ctx();
    float value = 0.5f;
    vxui_frame( &ctx, 1.0f / 60.0f );
    ctx.mouse_click_row_id = row_id( "menu", "Volume" );
    if ( vxui_menu( &ctx, "menu" ) )
    {
        vxui_menu_slider( &ctx, "Volume", &value, 0.0f, 1.0f, 0.1f );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );
    ASSERT_EQ( pressed_count( dl ), 1 );
    ASSERT_EQ( ctx.mouse_click_row_id, (uint32_t) 0 );
}

UTEST(menu_press_mirror, unfocused_option_does_not_gain_press) {
    vxui_ctx ctx = make_ctx();
    const char* options[] = { "Off", "On" };
    int index = 0;
    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "menu" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_option( &ctx, "Choice", &index, options, 2 );
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );
    vxui_frame( &ctx, 1.0f / 60.0f );
    ctx.input = VXUI_INPUT_CONFIRM;
    if ( vxui_menu( &ctx, "menu" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_option( &ctx, "Choice", &index, options, 2 );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );
    ASSERT_EQ( pressed_count( dl ), 1 );
    ASSERT_TRUE( vxui_draw_nth( dl, VXUI_DRAW_RECT, 0 )->state & VXUI_DRAW_PRESSED );
    ASSERT_FALSE( vxui_draw_nth( dl, VXUI_DRAW_RECT, 1 )->state & VXUI_DRAW_PRESSED );
}

UTEST(menu_press_mirror, unfocused_slider_does_not_gain_press) {
    vxui_ctx ctx = make_ctx();
    float value = 0.5f;
    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "menu" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_slider( &ctx, "Volume", &value, 0.0f, 1.0f, 0.1f );
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );
    vxui_frame( &ctx, 1.0f / 60.0f );
    ctx.input = VXUI_INPUT_CONFIRM;
    if ( vxui_menu( &ctx, "menu" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_slider( &ctx, "Volume", &value, 0.0f, 1.0f, 0.1f );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );
    ASSERT_EQ( pressed_count( dl ), 1 );
    ASSERT_TRUE( vxui_draw_nth( dl, VXUI_DRAW_RECT, 0 )->state & VXUI_DRAW_PRESSED );
    ASSERT_FALSE( vxui_draw_nth( dl, VXUI_DRAW_RECT, 1 )->state & VXUI_DRAW_PRESSED );
}

UTEST_MAIN();
