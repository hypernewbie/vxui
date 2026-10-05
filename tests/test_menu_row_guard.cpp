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
    return Clay__HashString( { false, (int32_t) strlen( label ), label }, vxui_hash( menu ) ).id;
}

UTEST(menu_row_guard, every_row_kind_enters_the_same_owner_record) {
    vxui_ctx ctx = make_ctx();
    const char* labels[] = { "Section", "Label", "Action", "Option", "Slider" };
    const char* options[] = { "Off", "On" };
    int index = 0;
    float value = 0;
    vxui_frame( &ctx, 0.01f );
    vxui_menu( &ctx, "guard" );
    vxui_menu_section( &ctx, labels[0] );
    vxui_menu_label( &ctx, labels[1] );
    vxui_menu_action( &ctx, labels[2] );
    vxui_menu_option( &ctx, labels[3], &index, options, 2 );
    vxui_menu_slider( &ctx, labels[4], &value );
    vxui_menu_end( &ctx );
    vxui_render( &ctx );
    ASSERT_EQ( ctx.frame_row_count, 5 );
    for ( int i = 0; i < 5; i++ )
    {
        ASSERT_EQ( ctx.frame_rows[i].x, row_id( "guard", labels[i] ) );
        ASSERT_EQ( ctx.frame_rows[i].y, (uint32_t) i );
        ASSERT_EQ( ctx.frame_rows[i].z, 0u );
    }
}

UTEST(menu_row_guard, empty_menus_do_not_reserve_a_row_record) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 0.01f );
    vxui_menu( &ctx, "empty" );
    vxui_menu_end( &ctx );
    vxui_menu( &ctx, "guard" );
    vxui_menu_action( &ctx, "Action" );
    vxui_menu_end( &ctx );
    vxui_render( &ctx );
    ASSERT_EQ( ctx.frame_row_count, 1 );
    ASSERT_EQ( ctx.frame_rows[0].z, 1u );
    ASSERT_EQ( ctx.frame_rows[0].y, 0u );
}

UTEST(menu_row_guard, labels_repeat_in_distinct_owners_without_overwriting_earlier_records) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 0.01f );
    const char* menus[] = { "left", "right", "bottom" };
    for ( int i = 0; i < 3; i++ )
    {
        vxui_menu( &ctx, menus[i] );
        vxui_menu_action( &ctx, "Back" );
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );
    for ( int i = 0; i < 3; i++ )
    {
        ASSERT_EQ( ctx.frame_rows[i].x, row_id( menus[i], "Back" ) );
        ASSERT_EQ( ctx.frame_rows[i].z, (uint32_t) i );
        for ( int j = 0; j < i; j++ ) ASSERT_NE( ctx.frame_rows[i].x, ctx.frame_rows[j].x );
    }
}

UTEST(menu_row_guard, every_local_row_slot_keeps_the_full_frame_record) {
    vxui_ctx ctx = make_ctx();
    char labels[VXUI_MAX_MENU_ROWS][16] = {};
    vxui_frame( &ctx, 0.01f );
    vxui_menu( &ctx, "guard" );
    for ( int i = 0; i < VXUI_MAX_MENU_ROWS; i++ )
    {
        snprintf( labels[i], sizeof( labels[i] ), "guard.%d", i );
        vxui_menu_action( &ctx, labels[i] );
    }
    vxui_menu_end( &ctx );
    vxui_render( &ctx );
    ASSERT_EQ( ctx.frame_row_count, VXUI_MAX_MENU_ROWS );
    for ( int i = 0; i < VXUI_MAX_MENU_ROWS; i++ ) ASSERT_EQ( ctx.frame_rows[i].x, row_id( "guard", labels[i] ) );
}

UTEST(menu_row_guard, changing_menu_order_keeps_persistent_owner_indices) {
    vxui_ctx ctx = make_ctx();
    const char* menus[] = { "left", "right" };
    for ( int frame = 0; frame < 3; frame++ )
    {
        vxui_frame( &ctx, 0.01f );
        for ( int i = 0; i < 2; i++ )
        {
            int owner = frame % 2 == 0 ? i : 1 - i;
            vxui_menu( &ctx, menus[owner] );
            vxui_menu_action( &ctx, "Back" );
            vxui_menu_end( &ctx );
            ASSERT_EQ( ctx.frame_rows[i].z, (uint32_t) owner );
        }
        vxui_render( &ctx );
        ASSERT_EQ( ctx.frame_row_count, 2 );
    }
}

UTEST(menu_row_guard, frame_reset_ignores_inactive_records) {
    vxui_ctx ctx = make_ctx();
    for ( int frame = 0; frame < 4; frame++ )
    {
        vxui_frame( &ctx, 0.01f );
        ASSERT_EQ( ctx.frame_row_count, 0 );
        vxui_menu( &ctx, "guard" );
        vxui_menu_action( &ctx, "Back" );
        if ( frame % 2 == 0 ) vxui_menu_action( &ctx, "Extra" );
        vxui_menu_end( &ctx );
        vxui_render( &ctx );
        ASSERT_EQ( ctx.frame_row_count, frame % 2 == 0 ? 2 : 1 );
    }
}

UTEST(menu_row_guard, colliding_seed_pairs_remain_valid_in_separate_frames) {
    vxui_ctx ctx = make_ctx();
    for ( int frame = 0; frame < 4; frame++ )
    {
        vxui_frame( &ctx, 0.01f );
        vxui_menu( &ctx, frame % 2 == 0 ? "menu0" : "menu1" );
        vxui_menu_action( &ctx, frame % 2 == 0 ? "B" : "A" );
        vxui_menu_end( &ctx );
        vxui_render( &ctx );
        ASSERT_EQ( ctx.frame_row_count, 1 );
        ASSERT_EQ( ctx.frame_rows[0].x, 2726600868u );
    }
}

UTEST(menu_row_guard, all_menu_slots_share_the_same_bounded_guard_records) {
    vxui_ctx ctx = make_ctx();
    char menus[VXUI_MAX_MENUS][32] = {};
    vxui_frame( &ctx, 0.01f );
    for ( int i = 0; i < VXUI_MAX_MENUS; i++ )
    {
        snprintf( menus[i], sizeof( menus[i] ), "state.menu.%d.panel", i );
        vxui_menu( &ctx, menus[i] );
        vxui_menu_action( &ctx, "A" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );
    ASSERT_EQ( ctx.frame_row_count, VXUI_MAX_MENUS );
    for ( int i = 0; i < VXUI_MAX_MENUS; i++ )
    {
        ASSERT_EQ( ctx.frame_rows[i].z, (uint32_t) i );
        ASSERT_TRUE( vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( menus[i], "A" ) ) );
    }
}

UTEST_MAIN();
