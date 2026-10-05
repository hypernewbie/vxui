#include "utest.h"
#include "../vxui.h"
#define VXUI_IMPL
#include "../vxui_impl.h"

static uint8_t s_clay_mem[16 * 1024 * 1024];

static Clay_Dimensions measure( Clay_StringSlice s, Clay_TextElementConfig* cfg, void* )
{
    return { (float) s.length * 8, (float) cfg->fontSize };
}

static vxui_ctx make_ctx()
{
    vxui_ctx ctx = {};
    vxui_init( &ctx, 1280, 720, s_clay_mem, sizeof( s_clay_mem ) );
    Clay_SetMeasureTextFunction( measure, nullptr );
    return ctx;
}

static uint32_t row_id( const char* menu, const char* label )
{
    return Clay__HashString( { false, (int32_t) strlen( label ), label }, vxui_hash( menu ) ).id;
}

UTEST(packet_resolution, mixed_packets_keep_source_order_and_text_only_truth_fields) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 0.01f );
    vxui_div( &ctx, "column", { .col = true } );
    vxui_rect( &ctx, "a", { .width = { VXUI_FIXED, 64 }, .height = { VXUI_FIXED, 16 } } );
    vxui_div_end( &ctx );
    vxui_text( &ctx, "text", "one", 24 );
    vxui_rect( &ctx, "b", { .width = { VXUI_FIXED, 64 }, .height = { VXUI_FIXED, 16 } } );
    vxui_div_end( &ctx );
    vxui_div_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );
    ASSERT_EQ( dl.count, 3 );
    ASSERT_EQ( dl.cmds[0].id, vxui_id( "a" ) );
    ASSERT_EQ( dl.cmds[1].id, vxui_text_id( "text" ) );
    ASSERT_EQ( dl.cmds[2].id, vxui_id( "b" ) );
    ASSERT_EQ( dl.cmds[1].font_px, (uint16_t) 24 );
    ASSERT_EQ( dl.cmds[1].text_len, 3 );
    ASSERT_EQ( dl.cmds[1].row_index, -1 );
    ASSERT_EQ( dl.cmds[1].state, (uint8_t) 0 );
}

UTEST(packet_resolution, row_resolution_ignores_containing_text_packets) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 0.01f );
    vxui_mouse( &ctx, 4, 48, VXUI_MOUSE_LEFT );
    vxui_menu( &ctx, "menu" );
    vxui_menu_section( &ctx, "Heading" );
    vxui_menu_action( &ctx, "Action" );
    vxui_menu_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );
    const vxui_draw_cmd* row = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "menu", "Action" ) );
    ASSERT_TRUE( row );
    ASSERT_EQ( row->row_index, 1 );
    ASSERT_EQ( row->state, (uint8_t) ( VXUI_DRAW_FOCUSED | VXUI_DRAW_HOVERED | VXUI_DRAW_PRESSED ) );
    for ( int i = 0; i < dl.count; i++ )
        if ( dl.cmds[i].type == VXUI_DRAW_TEXT )
        {
            ASSERT_EQ( dl.cmds[i].state, (uint8_t) 0 );
            ASSERT_EQ( dl.cmds[i].row_index, -1 );
            ASSERT_EQ( dl.cmds[i].focused_row_index, -1 );
        }
}

UTEST(packet_resolution, current_menu_and_spring_values_resolve_after_all_declarations) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 0.01f );
    vxui_menu( &ctx, "menu" );
    vxui_menu_action( &ctx, "A" );
    vxui_menu_action( &ctx, "B" );
    vxui_menu_action( &ctx, "C" );
    vxui_menu_end( &ctx );
    ctx.menu_state[0].y = 2;
    ctx.menu_focus_spring[0].x = -12.5f;
    vxui_draw_list dl = vxui_render( &ctx );
    for ( int i = 0; i < 3; i++ )
    {
        const vxui_draw_cmd* c = vxui_draw_nth( dl, VXUI_DRAW_RECT, i );
        ASSERT_TRUE( c );
        ASSERT_EQ( c->focused_row_index, 2 );
        ASSERT_EQ( ( c->state & VXUI_DRAW_FOCUSED ) != 0, i == 2 );
        ASSERT_EQ( c->focus_offset_y, i == 2 ? -12.5f : 0 );
    }
}

UTEST(packet_resolution, clip_restoration_and_hit_testing_use_each_packet_active_intersection) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 0.01f );
    vxui_mouse( &ctx, 4, 48, VXUI_MOUSE_LEFT );
    vxui_menu( &ctx, "scroll", true, 1 );
    vxui_menu_action( &ctx, "A" );
    vxui_menu_action( &ctx, "B" );
    vxui_menu_end( &ctx );
    vxui_root( &ctx, "overlay.root", 0, 40 );
    vxui_rect( &ctx, "overlay", { .width = { VXUI_FIXED, 80 }, .height = { VXUI_FIXED, 32 } } );
    vxui_div_end( &ctx );
    vxui_root_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );
    const vxui_draw_cmd* hidden = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "scroll", "B" ) );
    const vxui_draw_cmd* overlay = vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "overlay" ) );
    ASSERT_TRUE( hidden && overlay );
    ASSERT_TRUE( hidden->clipped );
    ASSERT_FALSE( hidden->state & VXUI_DRAW_HOVERED );
    ASSERT_FALSE( overlay->clipped );
    ASSERT_TRUE( overlay->state & VXUI_DRAW_HOVERED );
    ASSERT_EQ( ctx.mouse_press_row_id, overlay->id );
}

static void inspect_final_packets( const vxui_draw_cmd*, vxui_render_data* out, void* userdata )
{
    vxui_ctx* ctx = (vxui_ctx*) userdata;
    out->flags = ctx->draw_buf[2].state;
    out->texture_id = (uint32_t) ctx->draw_buf[2].row_index;
}

UTEST(packet_resolution, earliest_resolver_sees_later_rectangle_final_state_and_metadata) {
    vxui_ctx ctx = make_ctx();
    vxui_set_render_data_fn( &ctx, inspect_final_packets, &ctx );
    vxui_frame( &ctx, 0.01f );
    ctx.input = VXUI_INPUT_CONFIRM;
    vxui_menu( &ctx, "menu" );
    vxui_menu_action( &ctx, "A" );
    vxui_menu_action( &ctx, "B" );
    vxui_menu_end( &ctx );
    ctx.menu_state[0].y = 1;
    vxui_draw_list dl = vxui_render( &ctx );
    ASSERT_EQ( dl.count, 4 );
    ASSERT_EQ( dl.cmds[0].render.flags, (uint32_t) VXUI_DRAW_FOCUSED );
    ASSERT_EQ( dl.cmds[0].render.texture_id, 1u );
    ASSERT_EQ( dl.cmds[0].render.flags, (uint32_t) dl.cmds[2].state );
}

UTEST(packet_resolution, a_reused_slot_loses_its_previous_row_clip_and_hover_truth) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 0.01f );
    vxui_mouse( &ctx, 4, 16, VXUI_MOUSE_LEFT );
    vxui_menu( &ctx, "menu", true, 1 );
    vxui_menu_action( &ctx, "Action" );
    vxui_menu_end( &ctx );
    vxui_render( &ctx );
    vxui_frame( &ctx, 0.01f );
    vxui_mouse( &ctx, FLT_MAX, FLT_MAX, 0 );
    vxui_text( &ctx, "plain", "new" );
    vxui_draw_list dl = vxui_render( &ctx );
    ASSERT_EQ( dl.count, 1 );
    ASSERT_EQ( dl.cmds[0].state, (uint8_t) 0 );
    ASSERT_EQ( dl.cmds[0].row_index, -1 );
    ASSERT_EQ( dl.cmds[0].focus_offset_y, 0.0f );
    ASSERT_FALSE( dl.cmds[0].clipped );
}

UTEST(packet_resolution, unsupported_clay_commands_do_not_reserve_a_packet_slot) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 0.01f );
    CLAY( CLAY_ID( "border" ), { .layout = { .sizing = { CLAY_SIZING_FIXED( 100 ), CLAY_SIZING_FIXED( 100 ) } }, .border = { .color = { 1, 1, 1, 1 }, .width = { 2, 2, 2, 2, 0 } } } ) {}
    vxui_rect( &ctx, "plain", { .width = { VXUI_FIXED, 20 }, .height = { VXUI_FIXED, 20 } } );
    vxui_div_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );
    ASSERT_EQ( dl.count, 1 );
    ASSERT_EQ( dl.cmds[0].id, vxui_id( "plain" ) );
}

UTEST(packet_resolution, full_mixed_packet_capacity_resolves_every_menu_owner) {
    vxui_ctx ctx = make_ctx();
    char names[VXUI_MAX_MENUS][32] = {};
    const char* labels[] = { "A", "B", "C", "D", "E", "F", "G", "H" };
    vxui_frame( &ctx, 0.01f );
    for ( int m = 0; m < VXUI_MAX_MENUS; m++ )
    {
        snprintf( names[m], sizeof( names[m] ), "state.menu.%d.panel", m );
        vxui_menu( &ctx, names[m] );
        for ( const char* label : labels ) vxui_menu_action( &ctx, label );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );
    ASSERT_EQ( dl.count, VXUI_MAX_DRAW_CMDS );
    for ( int i = 0; i < ctx.frame_row_count; i++ )
    {
        const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, ctx.frame_rows[i].x );
        ASSERT_TRUE( c );
        ASSERT_EQ( c->row_index, (int) ctx.frame_rows[i].y );
        ASSERT_EQ( c->focused_row_index, 0 );
    }
}

UTEST_MAIN();
