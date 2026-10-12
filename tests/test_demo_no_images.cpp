#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
    #define NOMINMAX
#endif
#ifdef _WIN32
    #include <windows.h>
#endif

#include "utest.h"
#define main vxui_demo_main
#include "../demo/main.cpp"
#undef main

static Clay_Dimensions measure_text( Clay_StringSlice s, Clay_TextElementConfig* cfg, void* )
{
    return { (float) s.length * 8.0f, (float) cfg->fontSize };
}

static void init_ctx( vxui_ctx* ctx, demo_screen_state* screen, demo_resolver_state* resolver )
{
    *ctx = {};
    vxui_init( ctx, DEMO_PORTRAIT_W, DEMO_PORTRAIT_H, s_clay_mem, sizeof( s_clay_mem ) );
    Clay_SetMeasureTextFunction( measure_text, nullptr );
    *screen = {};
    screen->stack[0] = "operations";
    *resolver = {};
    resolver->ops_thumb_id = vxui_id( "ops_thumb" );
    vxui_set_render_data_fn( ctx, demo_render_data, resolver );
}

static vxui_draw_list operations_frame( vxui_ctx* ctx, demo_screen_state* screen, demo_resolver_state* resolver, uint32_t input, float mouse_x = FLT_MAX, float mouse_y = FLT_MAX, float dt = 1.0f / 60.0f )
{
    vxui_frame( ctx, dt );
    ctx->input = input;
    vxui_mouse( ctx, mouse_x, mouse_y, 0 );
    vxui_rect( ctx, "demo_frame", { .width = { VXUI_FIXED, DEMO_PORTRAIT_W }, .height = { VXUI_FIXED, DEMO_PORTRAIT_H }, .col = true, .padding = { 18, 18, 18, 18 }, .gap = 8 } );
        vxui_rect( ctx, "demo_panel", { .width = { VXUI_GROW, 0 }, .height = { VXUI_GROW, 0 }, .col = true, .padding = { 14, 14, 14, 14 }, .gap = 6 } );
            demo_dispatch_screen( ctx, screen, nullptr );
        vxui_div_end( ctx );
    vxui_div_end( ctx );

    int focused = demo_menu_focused( ctx, "operations" );
    assert( focused >= 0 && focused < DEMO_MISSION_COUNT );
    resolver->ops_thumb_tint = s_sector_tint[s_demo_missions[focused].sector];
    return vxui_render( ctx );
}

static const vxui_draw_cmd* menu_row( const vxui_draw_list& dl, int row )
{
    for ( int i = 0; i < dl.count; i++ )
        if ( dl.cmds[i].type == VXUI_DRAW_RECT && dl.cmds[i].row_index == row ) return &dl.cmds[i];
    return nullptr;
}

UTEST(demo_no_images, operations_menu_rows_have_no_content_texture) {
    vxui_ctx ctx;
    demo_screen_state screen;
    demo_resolver_state resolver;
    init_ctx( &ctx, &screen, &resolver );
    vxui_draw_list dl = operations_frame( &ctx, &screen, &resolver, 0 );

    for ( int i = 0; i < DEMO_MISSION_COUNT; i++ )
    {
        const vxui_draw_cmd* row = menu_row( dl, i );
        ASSERT_TRUE( row );
        ASSERT_EQ( row->render.texture_id, (uint32_t) 0 );
    }
}

UTEST(demo_no_images, hovered_row_keeps_its_fill_without_a_texture) {
    vxui_ctx ctx;
    demo_screen_state screen;
    demo_resolver_state resolver;
    init_ctx( &ctx, &screen, &resolver );
    vxui_draw_list dl = operations_frame( &ctx, &screen, &resolver, 0 );
    const vxui_draw_cmd* row = menu_row( dl, 1 );
    ASSERT_TRUE( row );
    dl = operations_frame( &ctx, &screen, &resolver, 0, row->rect.x + row->rect.z * 0.5f, row->rect.y + row->rect.w * 0.5f );
    row = menu_row( dl, 1 );

    ASSERT_TRUE( row );
    ASSERT_TRUE( row->state & VXUI_DRAW_HOVERED );
    ASSERT_EQ( row->render.texture_id, (uint32_t) 0 );
    ASSERT_EQ( row->render.colour.r, 0.16f );
    ASSERT_EQ( row->render.colour.g, 0.24f );
    ASSERT_EQ( row->render.colour.b, 0.36f );
    ASSERT_EQ( row->render.colour.a, 0.80f );
}

UTEST(demo_no_images, pressed_row_keeps_its_fill_without_a_texture) {
    vxui_ctx ctx;
    demo_screen_state screen;
    demo_resolver_state resolver;
    init_ctx( &ctx, &screen, &resolver );
    vxui_draw_list dl = operations_frame( &ctx, &screen, &resolver, VXUI_INPUT_CONFIRM );
    const vxui_draw_cmd* row = menu_row( dl, 0 );

    ASSERT_TRUE( row );
    ASSERT_TRUE( row->state & VXUI_DRAW_PRESSED );
    ASSERT_EQ( row->render.texture_id, (uint32_t) 0 );
    ASSERT_EQ( row->render.colour.r, 0.55f );
    ASSERT_EQ( row->render.colour.g, 0.95f );
    ASSERT_EQ( row->render.colour.b, 1.00f );
    ASSERT_EQ( row->render.colour.a, 0.95f );
}

UTEST(demo_no_images, focused_row_keeps_its_outline_and_focus_spring) {
    vxui_ctx ctx;
    demo_screen_state screen;
    demo_resolver_state resolver;
    init_ctx( &ctx, &screen, &resolver );
    operations_frame( &ctx, &screen, &resolver, 0 );
    vxui_draw_list dl = operations_frame( &ctx, &screen, &resolver, VXUI_INPUT_DOWN );
    const vxui_draw_cmd* row = menu_row( dl, 1 );

    ASSERT_TRUE( row );
    ASSERT_TRUE( row->state & VXUI_DRAW_FOCUSED );
    ASSERT_NE( row->focus_offset_y, 0.0f );
    ASSERT_EQ( row->render.texture_id, (uint32_t) 0 );
    ASSERT_EQ( row->render.colour.r, 0.10f );
    ASSERT_EQ( row->render.colour.g, 0.40f );
    ASSERT_EQ( row->render.colour.b, 0.85f );
    ASSERT_EQ( row->render.colour.a, 0.85f );
    ASSERT_EQ( row->render.outline_colour.r, 0.55f );
    ASSERT_EQ( row->render.outline_colour.g, 0.95f );
    ASSERT_EQ( row->render.outline_colour.b, 1.00f );
    ASSERT_EQ( row->render.outline_thickness, 2.0f );
}

UTEST(demo_no_images, distance_ramp_keeps_its_colour_without_an_icon) {
    vxui_ctx ctx;
    demo_screen_state screen;
    demo_resolver_state resolver;
    init_ctx( &ctx, &screen, &resolver );
    vxui_draw_list dl = operations_frame( &ctx, &screen, &resolver, 0 );
    const vxui_draw_cmd* row = menu_row( dl, 3 );

    ASSERT_TRUE( row );
    ASSERT_EQ( row->render.texture_id, (uint32_t) 0 );
    ASSERT_EQ( row->render.colour.r, 0.03f );
    ASSERT_EQ( row->render.colour.g, 0.07f );
    ASSERT_EQ( row->render.colour.b, 0.14f );
    ASSERT_EQ( row->render.colour.a, 0.40f );
}

UTEST_MAIN();
