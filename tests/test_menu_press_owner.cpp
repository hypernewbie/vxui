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

static int pressed_count( const vxui_draw_list& dl )
{
    int count = 0;
    for ( int i = 0; i < dl.count; i++ )
        if ( dl.cmds[i].state & VXUI_DRAW_PRESSED ) count++;
    return count;
}

static void capture_press( const vxui_draw_cmd* c, vxui_render_data* out, void* )
{
    out->flags = c->state;
}

UTEST(menu_press_owner, keyboard_hold_keeps_press_truth_but_fires_only_on_the_edge) {
    vxui_ctx ctx = make_ctx();
    uint32_t input[] = { VXUI_INPUT_CONFIRM, VXUI_INPUT_CONFIRM, 0, VXUI_INPUT_CONFIRM | VXUI_INPUT_DOWN, VXUI_INPUT_CONFIRM, 0 };
    for ( int frame = 0; frame < 6; frame++ )
    {
        vxui_frame( &ctx, 1.0f / 60.0f );
        ctx.input = input[frame];
        vxui_menu( &ctx, "menu" );
        int fires = 0;
        if ( vxui_menu_action( &ctx, "A" ) ) fires++;
        if ( vxui_menu_action( &ctx, "B" ) ) fires++;
        vxui_menu_end( &ctx );
        vxui_draw_list dl = vxui_render( &ctx );
        bool held = ( input[frame] & VXUI_INPUT_CONFIRM ) != 0;
        ASSERT_EQ( pressed_count( dl ), held ? 1 : 0 );
        ASSERT_EQ( fires, frame == 0 || frame == 3 ? 1 : 0 );
        for ( int row = 0; row < 2; row++ )
        {
            const vxui_draw_cmd* c = vxui_draw_nth( dl, VXUI_DRAW_RECT, row );
            ASSERT_EQ( ( c->state & VXUI_DRAW_PRESSED ) != 0, held && row == (int) ctx.menu_state[0].y );
        }
    }
}

UTEST(menu_press_owner, a_carried_mouse_click_sets_one_press_and_is_consumed_once) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    ctx.mouse_click_row_id = row_id( "menu", "B" );
    vxui_menu( &ctx, "menu" );
    ASSERT_FALSE( vxui_menu_action( &ctx, "A" ) );
    ASSERT_TRUE( vxui_menu_action( &ctx, "B" ) );
    vxui_menu_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );
    ASSERT_EQ( pressed_count( dl ), 1 );
    ASSERT_TRUE( vxui_draw_nth( dl, VXUI_DRAW_RECT, 1 )->state & VXUI_DRAW_PRESSED );
    ASSERT_EQ( ctx.mouse_click_row_id, (uint32_t) 0 );
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_menu( &ctx, "menu" );
    ASSERT_FALSE( vxui_menu_action( &ctx, "A" ) );
    ASSERT_FALSE( vxui_menu_action( &ctx, "B" ) );
    vxui_menu_end( &ctx );
    ASSERT_EQ( pressed_count( vxui_render( &ctx ) ), 0 );
}

UTEST(menu_press_owner, late_mouse_focus_does_not_erase_an_earlier_keyboard_activation) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    ctx.input = VXUI_INPUT_CONFIRM;
    ctx.mouse_click_row_id = row_id( "menu", "B" );
    vxui_menu( &ctx, "menu" );
    ASSERT_TRUE( vxui_menu_action( &ctx, "A" ) );
    ASSERT_TRUE( vxui_menu_action( &ctx, "B" ) );
    vxui_menu_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );
    ASSERT_EQ( pressed_count( dl ), 2 );
    ASSERT_FALSE( vxui_draw_nth( dl, VXUI_DRAW_RECT, 0 )->state & VXUI_DRAW_FOCUSED );
    ASSERT_TRUE( vxui_draw_nth( dl, VXUI_DRAW_RECT, 1 )->state & VXUI_DRAW_FOCUSED );
}

UTEST(menu_press_owner, 32_menus_plus_a_second_activation_do_not_drop_the_last_menu_press) {
    vxui_ctx ctx = make_ctx();
    char names[VXUI_MAX_MENUS][40] = {};
    for ( int i = 0; i < VXUI_MAX_MENUS; i++ ) snprintf( names[i], sizeof( names[i] ), "press.owner.%d.panel", i );
    vxui_frame( &ctx, 1.0f / 60.0f );
    ctx.input = VXUI_INPUT_CONFIRM;
    ctx.mouse_click_row_id = row_id( names[0], "B" );
    int fires = 0;
    for ( int i = 0; i < VXUI_MAX_MENUS; i++ )
    {
        vxui_menu( &ctx, names[i] );
        if ( vxui_menu_action( &ctx, "A" ) ) fires++;
        if ( vxui_menu_action( &ctx, "B" ) ) fires++;
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );
    ASSERT_EQ( fires, VXUI_MAX_MENUS + 1 );
    ASSERT_EQ( pressed_count( dl ), VXUI_MAX_MENUS + 1 );
    for ( int i = 0; i < VXUI_MAX_MENUS; i++ )
    {
        const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( names[i], "A" ) );
        ASSERT_TRUE( c );
        ASSERT_TRUE( c->state & VXUI_DRAW_PRESSED );
    }
}

UTEST(menu_press_owner, focused_options_and_sliders_mirror_action_press_policy) {
    vxui_ctx ctx = make_ctx();
    const char* options[] = { "Off", "On" };
    int index = 0;
    float value = 0.5f;
    for ( int frame = 0; frame < 2; frame++ )
    {
        vxui_frame( &ctx, 1.0f / 60.0f );
        ctx.input = VXUI_INPUT_CONFIRM;
        if ( frame == 1 ) ctx.menu_state[0].y = 3;
        vxui_menu( &ctx, "mixed" );
        vxui_menu_section( &ctx, "section" );
        vxui_menu_option( &ctx, "option", &index, options, 2 );
        vxui_menu_label( &ctx, "label" );
        vxui_menu_slider( &ctx, "slider", &value );
        vxui_menu_action( &ctx, "action" );
        vxui_menu_end( &ctx );
        vxui_draw_list dl = vxui_render( &ctx );
        ASSERT_EQ( pressed_count( dl ), 1 );
        ASSERT_TRUE( vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "mixed", frame == 0 ? "option" : "slider" ) )->state & VXUI_DRAW_PRESSED );
        ASSERT_EQ( index, 0 );
        ASSERT_EQ( value, 0.5f );
    }
}

UTEST(menu_press_owner, physical_press_on_an_ordinary_rect_is_independent_of_keyboard_menu_press) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    ctx.input = VXUI_INPUT_CONFIRM;
    vxui_mouse( &ctx, 4, 4, VXUI_MOUSE_LEFT );
    vxui_root( &ctx, "ordinary.root", 0, 0 );
    vxui_rect( &ctx, "ordinary", { .width = { VXUI_FIXED, 10 }, .height = { VXUI_FIXED, 10 } } );
    vxui_div_end( &ctx );
    vxui_root_end( &ctx );
    vxui_root( &ctx, "menu.root", 200, 200 );
    vxui_menu( &ctx, "menu" );
    vxui_menu_action( &ctx, "A" );
    vxui_menu_end( &ctx );
    vxui_root_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );
    ASSERT_EQ( pressed_count( dl ), 2 );
    const vxui_draw_cmd* ordinary = vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "ordinary" ) );
    ASSERT_TRUE( ordinary );
    ASSERT_TRUE( ordinary->state & VXUI_DRAW_PRESSED );
    ASSERT_FALSE( ordinary->state & VXUI_DRAW_FOCUSED );
    ASSERT_EQ( ordinary->row_index, -1 );
    ASSERT_EQ( ctx.mouse_press_row_id, ordinary->id );
}

UTEST(menu_press_owner, reordering_menus_keeps_row_bits_with_their_persistent_owners) {
    vxui_ctx ctx = make_ctx();
    const char* names[] = { "left", "right" };
    for ( int frame = 0; frame < 2; frame++ )
    {
        vxui_frame( &ctx, 1.0f / 60.0f );
        ctx.input = VXUI_INPUT_CONFIRM;
        if ( frame == 1 ) ctx.menu_state[0].y = 1;
        for ( int i = 0; i < 2; i++ )
        {
            int menu = frame == 0 ? i : 1 - i;
            vxui_menu( &ctx, names[menu] );
            vxui_menu_action( &ctx, "A" );
            vxui_menu_action( &ctx, "B" );
            vxui_menu_end( &ctx );
        }
        vxui_draw_list dl = vxui_render( &ctx );
        ASSERT_EQ( pressed_count( dl ), 2 );
        for ( int menu = 0; menu < 2; menu++ )
        {
            for ( int row = 0; row < 2; row++ )
            {
                const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( names[menu], row == 0 ? "A" : "B" ) );
                ASSERT_TRUE( c );
                ASSERT_EQ( ( c->state & VXUI_DRAW_PRESSED ) != 0, row == (int) ctx.menu_state[menu].y );
            }
        }
    }
}

UTEST(menu_press_owner, row_31_uses_the_high_bit_without_affecting_other_rows) {
    vxui_ctx ctx = make_ctx();
    char labels[VXUI_MAX_MENU_ROWS][32] = {};
    for ( int frame = 0; frame < 3; frame++ )
    {
        vxui_frame( &ctx, 1.0f / 60.0f );
        ctx.input = frame == 1 ? VXUI_INPUT_CONFIRM : 0;
        if ( frame == 1 ) ctx.menu_state[0].y = VXUI_MAX_MENU_ROWS - 1;
        vxui_menu( &ctx, "full" );
        for ( int row = 0; row < VXUI_MAX_MENU_ROWS; row++ )
        {
            snprintf( labels[row], sizeof( labels[row] ), "press.%d.label", row );
            vxui_menu_action( &ctx, labels[row] );
        }
        vxui_menu_end( &ctx );
        vxui_draw_list dl = vxui_render( &ctx );
        ASSERT_EQ( pressed_count( dl ), frame == 1 ? 1 : 0 );
        for ( int row = 0; row < VXUI_MAX_MENU_ROWS; row++ )
        {
            const vxui_draw_cmd* c = vxui_draw_nth( dl, VXUI_DRAW_RECT, row );
            ASSERT_EQ( ( c->state & VXUI_DRAW_PRESSED ) != 0, frame == 1 && row == VXUI_MAX_MENU_ROWS - 1 );
        }
    }
}

UTEST(menu_press_owner, full_packet_batch_and_resolver_preserve_each_menus_pressed_row) {
    vxui_ctx ctx = make_ctx();
    vxui_set_render_data_fn( &ctx, capture_press, nullptr );
    char names[VXUI_MAX_MENUS][40] = {};
    char labels[8][32] = {};
    for ( int i = 0; i < 8; i++ ) snprintf( labels[i], sizeof( labels[i] ), "press.%d.label", i );
    for ( int frame = 0; frame < 2; frame++ )
    {
        vxui_frame( &ctx, 1.0f / 60.0f );
        ctx.input = VXUI_INPUT_CONFIRM;
        for ( int menu = 0; menu < VXUI_MAX_MENUS; menu++ )
        {
            snprintf( names[menu], sizeof( names[menu] ), "press.packet.%d.panel", menu );
            if ( frame == 1 ) ctx.menu_state[menu].y = (uint32_t) ( menu % 8 );
            vxui_menu( &ctx, names[menu] );
            for ( int row = 0; row < 8; row++ ) vxui_menu_action( &ctx, labels[row] );
            vxui_menu_end( &ctx );
        }
        vxui_draw_list dl = vxui_render( &ctx );
        ASSERT_EQ( dl.count, VXUI_MAX_DRAW_CMDS );
        ASSERT_EQ( pressed_count( dl ), VXUI_MAX_MENUS );
        for ( int i = 0; i < dl.count; i++ ) ASSERT_EQ( dl.cmds[i].render.flags, (uint32_t) dl.cmds[i].state );
    }
}

UTEST(menu_press_owner, an_empty_frame_and_later_remount_do_not_retain_press_bits) {
    vxui_ctx ctx = make_ctx();
    for ( int frame = 0; frame < 3; frame++ )
    {
        vxui_frame( &ctx, 1.0f / 60.0f );
        ctx.input = frame == 0 ? VXUI_INPUT_CONFIRM : 0;
        if ( frame != 1 )
        {
            vxui_menu( &ctx, "menu" );
            vxui_menu_action( &ctx, "A" );
            vxui_menu_end( &ctx );
        }
        vxui_draw_list dl = vxui_render( &ctx );
        ASSERT_EQ( pressed_count( dl ), frame == 0 ? 1 : 0 );
    }
}

UTEST_MAIN();
