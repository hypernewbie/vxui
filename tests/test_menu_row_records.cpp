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

static glm::uvec3 recorded_row( const vxui_ctx& ctx, int i )
{
    return ctx.frame_rows[i];
}

static uint32_t row_id( const char* menu, const char* label )
{
    return Clay__HashString( { false, (int32_t) strlen( label ), label }, vxui_hash( menu ) ).id;
}

UTEST(menu_row_records, all_row_kinds_record_id_index_and_owner_together) {
    vxui_ctx ctx = make_ctx();
    const char* labels[] = { "section", "action", "option", "slider", "label" };
    const char* options[] = { "Off", "On" };
    int index = 0;
    float value = 0.5f;
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_menu( &ctx, "mixed" );
    vxui_menu_section( &ctx, labels[0] );
    vxui_menu_action( &ctx, labels[1] );
    vxui_menu_option( &ctx, labels[2], &index, options, 2 );
    vxui_menu_slider( &ctx, labels[3], &value );
    vxui_menu_label( &ctx, labels[4] );
    vxui_menu_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( ctx.frame_row_count, 5 );
    for ( int i = 0; i < 5; i++ )
    {
        glm::uvec3 row = recorded_row( ctx, i );
        ASSERT_EQ( row.x, row_id( "mixed", labels[i] ) );
        ASSERT_EQ( row.y, (uint32_t) i );
        ASSERT_EQ( row.z, (uint32_t) 0 );
        const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, row.x );
        ASSERT_TRUE( c );
        ASSERT_EQ( c->row_index, i );
        ASSERT_EQ( c->focused_row_index, 1 );
    }
}

UTEST(menu_row_records, changing_declaration_order_keeps_the_persistent_menu_slot) {
    vxui_ctx ctx = make_ctx();
    const char* names[] = { "left", "right" };
    for ( int frame = 0; frame < 2; frame++ )
    {
        vxui_frame( &ctx, 1.0f / 60.0f );
        for ( int i = 0; i < 2; i++ )
        {
            int menu = frame == 0 ? i : 1 - i;
            vxui_menu( &ctx, names[menu] );
            vxui_menu_action( &ctx, "A" );
            vxui_menu_action( &ctx, "B" );
            vxui_menu_end( &ctx );
        }
        vxui_render( &ctx );
        ASSERT_EQ( ctx.frame_row_count, 4 );
        for ( int i = 0; i < 4; i++ )
        {
            int menu = frame == 0 ? i / 2 : 1 - i / 2;
            glm::uvec3 row = recorded_row( ctx, i );
            ASSERT_EQ( row.z, (uint32_t) menu );
            ASSERT_EQ( row.y, (uint32_t) ( i % 2 ) );
            ASSERT_EQ( row.x, row_id( names[menu], i % 2 == 0 ? "A" : "B" ) );
        }
    }
}

UTEST(menu_row_records, shorter_frames_overwrite_the_active_prefix_only) {
    vxui_ctx ctx = make_ctx();
    for ( int frame = 0; frame < 2; frame++ )
    {
        vxui_frame( &ctx, 1.0f / 60.0f );
        const char* name = frame == 0 ? "old" : "new";
        vxui_menu( &ctx, name );
        vxui_menu_action( &ctx, "A" );
        if ( frame == 0 ) vxui_menu_action( &ctx, "B" );
        vxui_menu_end( &ctx );
        vxui_draw_list dl = vxui_render( &ctx );
        ASSERT_EQ( ctx.frame_row_count, frame == 0 ? 2 : 1 );
        glm::uvec3 row = recorded_row( ctx, 0 );
        ASSERT_EQ( row.x, row_id( name, "A" ) );
        ASSERT_EQ( row.y, (uint32_t) 0 );
        ASSERT_EQ( row.z, (uint32_t) frame );
        ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), ctx.frame_row_count );
    }
}

UTEST(menu_row_records, empty_and_ordinary_frames_do_not_claim_old_menu_records) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_menu( &ctx, "menu" );
    vxui_menu_action( &ctx, "A" );
    vxui_menu_end( &ctx );
    vxui_render( &ctx );
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_rect( &ctx, "ordinary", { .width = { VXUI_FIXED, 10 }, .height = { VXUI_FIXED, 10 } } );
    vxui_div_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );
    ASSERT_EQ( ctx.frame_row_count, 0 );
    ASSERT_EQ( dl.count, 1 );
    ASSERT_EQ( dl.cmds[0].row_index, -1 );
    ASSERT_EQ( dl.cmds[0].focused_row_index, -1 );
    vxui_frame( &ctx, 1.0f / 60.0f );
    ASSERT_EQ( vxui_render( &ctx ).count, 0 );
}

UTEST(menu_row_records, all_32_row_indices_include_noninteractive_rows) {
    vxui_ctx ctx = make_ctx();
    char labels[VXUI_MAX_MENU_ROWS][32] = {};
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_menu( &ctx, "full" );
    for ( int i = 0; i < VXUI_MAX_MENU_ROWS; i++ )
    {
        snprintf( labels[i], sizeof( labels[i] ), "record.%d.label", i );
        if ( i % 2 == 0 ) vxui_menu_label( &ctx, labels[i] );
        else vxui_menu_action( &ctx, labels[i] );
    }
    vxui_menu_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );
    ASSERT_EQ( ctx.frame_row_count, VXUI_MAX_MENU_ROWS );
    for ( int i = 0; i < VXUI_MAX_MENU_ROWS; i++ )
    {
        glm::uvec3 row = recorded_row( ctx, i );
        ASSERT_EQ( row.y, (uint32_t) i );
        ASSERT_EQ( row.z, (uint32_t) 0 );
        const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, row.x );
        ASSERT_TRUE( c );
        ASSERT_EQ( c->row_index, i );
        ASSERT_EQ( c->focused_row_index, 1 );
    }
}

UTEST(menu_row_records, full_menu_packet_batch_keeps_all_owners_and_indices) {
    vxui_ctx ctx = make_ctx();
    char names[VXUI_MAX_MENUS][40] = {};
    char labels[8][32] = {};
    for ( int i = 0; i < 8; i++ ) snprintf( labels[i], sizeof( labels[i] ), "record.%d.label", i );
    vxui_frame( &ctx, 1.0f / 60.0f );
    for ( int i = 0; i < VXUI_MAX_MENUS; i++ )
    {
        snprintf( names[i], sizeof( names[i] ), "record.menu.%d.panel", i );
        vxui_menu( &ctx, names[i] );
        for ( int j = 0; j < 8; j++ ) vxui_menu_action( &ctx, labels[j] );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );
    ASSERT_EQ( dl.count, VXUI_MAX_DRAW_CMDS );
    ASSERT_EQ( ctx.frame_row_count, VXUI_MAX_MENUS * 8 );
    for ( int i = 0; i < ctx.frame_row_count; i++ )
    {
        glm::uvec3 row = recorded_row( ctx, i );
        ASSERT_EQ( row.x, row_id( names[i / 8], labels[i % 8] ) );
        ASSERT_EQ( row.y, (uint32_t) ( i % 8 ) );
        ASSERT_EQ( row.z, (uint32_t) ( i / 8 ) );
        const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, row.x );
        ASSERT_TRUE( c );
        ASSERT_EQ( c->row_index, i % 8 );
        ASSERT_EQ( c->focused_row_index, 0 );
    }
}

UTEST(menu_row_records, scrolled_rows_keep_logical_indices_and_raw_userdata_stays_unclaimed) {
    vxui_ctx ctx = make_ctx();
    for ( int frame = 0; frame < 2; frame++ )
    {
        vxui_frame( &ctx, 1.0f / 60.0f );
        ctx.input = frame == 0 ? 0 : VXUI_INPUT_DOWN;
        vxui_menu( &ctx, "scroll", true, 1 );
        vxui_menu_action( &ctx, "A" );
        vxui_menu_action( &ctx, "B" );
        vxui_menu_end( &ctx );
        vxui_draw_list dl = vxui_render( &ctx );
        for ( int i = 0; i < 2; i++ )
        {
            glm::uvec3 row = recorded_row( ctx, i );
            const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, row.x );
            ASSERT_TRUE( c );
            ASSERT_EQ( c->row_index, i );
            ASSERT_EQ( c->focused_row_index, frame );
            ASSERT_TRUE( c->clipped );
            ASSERT_TRUE( c->userdata == nullptr );
        }
    }
}

UTEST(menu_row_records, value_changes_do_not_replace_the_row_record) {
    vxui_ctx ctx = make_ctx();
    const char* options[] = { "Off", "Longer value" };
    int index = 0;
    uint32_t id = 0;
    for ( int frame = 0; frame < 2; frame++ )
    {
        vxui_frame( &ctx, 1.0f / 60.0f );
        ctx.input = frame == 0 ? 0 : VXUI_INPUT_RIGHT;
        vxui_menu( &ctx, "settings" );
        vxui_menu_option( &ctx, "mode", &index, options, 2 );
        vxui_menu_end( &ctx );
        vxui_draw_list dl = vxui_render( &ctx );
        glm::uvec3 row = recorded_row( ctx, 0 );
        if ( frame == 0 ) id = row.x;
        ASSERT_EQ( row.x, id );
        ASSERT_EQ( row.y, (uint32_t) 0 );
        ASSERT_EQ( row.z, (uint32_t) 0 );
        ASSERT_TRUE( vxui_draw_find( dl, VXUI_DRAW_RECT, id ) );
        ASSERT_EQ( index, frame );
    }
}

UTEST_MAIN();
