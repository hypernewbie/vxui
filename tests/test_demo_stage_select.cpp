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
    screen->stack[0] = "stageselect";
    *resolver = {};
    resolver->screen       = screen;
    resolver->stg_thumb_id = vxui_id( "stg_thumb" );
    vxui_set_render_data_fn( ctx, demo_render_data, resolver );
}

static vxui_draw_list stage_select_frame( vxui_ctx* ctx, demo_screen_state* screen, demo_resolver_state* resolver, uint32_t input, float dt = 1.0f / 60.0f )
{
    vxui_frame( ctx, dt );
    ctx->input = input;
    vxui_rect( ctx, "demo_frame", { .width = { VXUI_FIXED, DEMO_PORTRAIT_W }, .height = { VXUI_FIXED, DEMO_PORTRAIT_H }, .col = true, .padding = { 18, 18, 18, 18 }, .gap = 8 } );
        vxui_rect( ctx, "demo_panel", { .width = { VXUI_GROW, 0 }, .height = { VXUI_GROW, 0 }, .col = true, .padding = { 14, 14, 14, 14 }, .gap = 6 } );
            demo_dispatch_screen( ctx, screen, nullptr );
        vxui_div_end( ctx );
    vxui_div_end( ctx );

    int focused = demo_menu_focused( ctx, "stageselect" );
    assert( focused >= 0 && focused < DEMO_SECTOR_COUNT );
    resolver->stg_thumb_texture = 42;
    resolver->stg_thumb_tint    = s_sector_tint[focused];
    return vxui_render( ctx );
}

static bool text_is( const vxui_draw_list& dl, const char* id, const char* text )
{
    const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_TEXT, vxui_text_id( id ) );
    return c && c->text_len == (int) strlen( text ) && memcmp( c->text, text, c->text_len ) == 0;
}

UTEST(demo_stage_select, first_frame_preview_matches_initial_focus) {
    vxui_ctx ctx;
    demo_screen_state screen;
    demo_resolver_state resolver;
    init_ctx( &ctx, &screen, &resolver );
    vxui_draw_list dl = stage_select_frame( &ctx, &screen, &resolver, 0 );

    ASSERT_EQ( demo_menu_focused( &ctx, "stageselect" ), 0 );
    ASSERT_TRUE( text_is( dl, "stg_sector", "Debris Belt" ) );
    ASSERT_TRUE( text_is( dl, "stg_status", "PERFECT" ) );
    const vxui_draw_cmd* thumb = vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "stg_thumb" ) );
    ASSERT_TRUE( thumb );
    ASSERT_EQ( thumb->render.material_id, (uint32_t) DEMO_MATERIAL_IMAGE );
    ASSERT_EQ( thumb->render.texture_id, (uint32_t) 42 );
    ASSERT_EQ( thumb->render.colour.r, s_sector_tint[0].r );
    ASSERT_EQ( thumb->render.colour.g, s_sector_tint[0].g );
    ASSERT_EQ( thumb->render.colour.b, s_sector_tint[0].b );
    ASSERT_EQ( thumb->render.colour.a, s_sector_tint[0].a );
}

UTEST(demo_stage_select, first_frame_down_does_not_navigate_without_row_count) {
    vxui_ctx ctx;
    demo_screen_state screen;
    demo_resolver_state resolver;
    init_ctx( &ctx, &screen, &resolver );
    vxui_draw_list dl = stage_select_frame( &ctx, &screen, &resolver, VXUI_INPUT_DOWN );

    ASSERT_EQ( demo_menu_focused( &ctx, "stageselect" ), 0 );
    ASSERT_EQ( ctx.menu_state[0].z, (uint32_t) DEMO_SECTOR_COUNT );
    ASSERT_TRUE( text_is( dl, "stg_sector", "Debris Belt" ) );
    ASSERT_TRUE( text_is( dl, "stg_status", "PERFECT" ) );
    const vxui_draw_cmd* thumb = vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "stg_thumb" ) );
    ASSERT_TRUE( thumb );
    ASSERT_EQ( thumb->render.colour.r, s_sector_tint[0].r );
    ASSERT_EQ( thumb->render.colour.g, s_sector_tint[0].g );
    ASSERT_EQ( thumb->render.colour.b, s_sector_tint[0].b );
}

UTEST(demo_stage_select, down_updates_preview_text_and_tint_in_same_frame) {
    vxui_ctx ctx;
    demo_screen_state screen;
    demo_resolver_state resolver;
    init_ctx( &ctx, &screen, &resolver );
    stage_select_frame( &ctx, &screen, &resolver, 0 );
    vxui_draw_list dl = stage_select_frame( &ctx, &screen, &resolver, VXUI_INPUT_DOWN );

    ASSERT_EQ( demo_menu_focused( &ctx, "stageselect" ), 1 );
    ASSERT_TRUE( text_is( dl, "stg_sector", "Signal Array" ) );
    ASSERT_TRUE( text_is( dl, "stg_status", "CLEARED" ) );
    const vxui_draw_cmd* thumb = vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "stg_thumb" ) );
    ASSERT_TRUE( thumb );
    ASSERT_EQ( thumb->render.material_id, (uint32_t) DEMO_MATERIAL_IMAGE );
    ASSERT_EQ( thumb->render.texture_id, (uint32_t) 42 );
    ASSERT_EQ( thumb->render.colour.r, s_sector_tint[1].r );
    ASSERT_EQ( thumb->render.colour.g, s_sector_tint[1].g );
    ASSERT_EQ( thumb->render.colour.b, s_sector_tint[1].b );
    ASSERT_EQ( thumb->render.colour.a, s_sector_tint[1].a );
}

UTEST(demo_stage_select, up_updates_preview_text_and_tint_in_same_frame) {
    vxui_ctx ctx;
    demo_screen_state screen;
    demo_resolver_state resolver;
    init_ctx( &ctx, &screen, &resolver );
    stage_select_frame( &ctx, &screen, &resolver, 0 );
    ctx.menu_state[0].y = 2;
    vxui_draw_list dl = stage_select_frame( &ctx, &screen, &resolver, VXUI_INPUT_UP );

    ASSERT_EQ( demo_menu_focused( &ctx, "stageselect" ), 1 );
    ASSERT_TRUE( text_is( dl, "stg_sector", "Signal Array" ) );
    ASSERT_TRUE( text_is( dl, "stg_status", "CLEARED" ) );
    const vxui_draw_cmd* thumb = vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "stg_thumb" ) );
    ASSERT_TRUE( thumb );
    ASSERT_EQ( thumb->render.colour.r, s_sector_tint[1].r );
    ASSERT_EQ( thumb->render.colour.g, s_sector_tint[1].g );
    ASSERT_EQ( thumb->render.colour.b, s_sector_tint[1].b );
}

UTEST(demo_stage_select, up_wraps_preview_to_boss_rush) {
    vxui_ctx ctx;
    demo_screen_state screen;
    demo_resolver_state resolver;
    init_ctx( &ctx, &screen, &resolver );
    stage_select_frame( &ctx, &screen, &resolver, 0 );
    vxui_draw_list dl = stage_select_frame( &ctx, &screen, &resolver, VXUI_INPUT_UP );

    ASSERT_EQ( demo_menu_focused( &ctx, "stageselect" ), DEMO_SECTOR_COUNT - 1 );
    ASSERT_TRUE( text_is( dl, "stg_sector", "Boss Rush" ) );
    ASSERT_TRUE( text_is( dl, "stg_status", "LOCKED" ) );
    const vxui_draw_cmd* thumb = vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "stg_thumb" ) );
    ASSERT_TRUE( thumb );
    ASSERT_EQ( thumb->render.colour.r, s_sector_tint[5].r );
    ASSERT_EQ( thumb->render.colour.g, s_sector_tint[5].g );
    ASSERT_EQ( thumb->render.colour.b, s_sector_tint[5].b );
}

UTEST(demo_stage_select, down_wraps_preview_to_first_sector) {
    vxui_ctx ctx;
    demo_screen_state screen;
    demo_resolver_state resolver;
    init_ctx( &ctx, &screen, &resolver );
    stage_select_frame( &ctx, &screen, &resolver, 0 );
    ctx.menu_state[0].y = DEMO_SECTOR_COUNT - 1;
    vxui_draw_list dl = stage_select_frame( &ctx, &screen, &resolver, VXUI_INPUT_DOWN );

    ASSERT_EQ( demo_menu_focused( &ctx, "stageselect" ), 0 );
    ASSERT_TRUE( text_is( dl, "stg_sector", "Debris Belt" ) );
    ASSERT_TRUE( text_is( dl, "stg_status", "PERFECT" ) );
    const vxui_draw_cmd* thumb = vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "stg_thumb" ) );
    ASSERT_TRUE( thumb );
    ASSERT_EQ( thumb->render.colour.r, s_sector_tint[0].r );
    ASSERT_EQ( thumb->render.colour.g, s_sector_tint[0].g );
    ASSERT_EQ( thumb->render.colour.b, s_sector_tint[0].b );
}

UTEST(demo_stage_select, entering_locked_sector_updates_name_and_status) {
    vxui_ctx ctx;
    demo_screen_state screen;
    demo_resolver_state resolver;
    init_ctx( &ctx, &screen, &resolver );
    stage_select_frame( &ctx, &screen, &resolver, 0 );
    ctx.menu_state[0].y = 3;
    vxui_draw_list dl = stage_select_frame( &ctx, &screen, &resolver, VXUI_INPUT_DOWN );

    ASSERT_EQ( demo_menu_focused( &ctx, "stageselect" ), 4 );
    ASSERT_TRUE( text_is( dl, "stg_sector", "Null Cathedral" ) );
    ASSERT_TRUE( text_is( dl, "stg_status", "LOCKED" ) );
    const vxui_draw_cmd* thumb = vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "stg_thumb" ) );
    ASSERT_TRUE( thumb );
    ASSERT_EQ( thumb->render.colour.r, s_sector_tint[4].r );
    ASSERT_EQ( thumb->render.colour.g, s_sector_tint[4].g );
    ASSERT_EQ( thumb->render.colour.b, s_sector_tint[4].b );
}

UTEST(demo_stage_select, leaving_locked_sector_updates_name_and_status) {
    vxui_ctx ctx;
    demo_screen_state screen;
    demo_resolver_state resolver;
    init_ctx( &ctx, &screen, &resolver );
    stage_select_frame( &ctx, &screen, &resolver, 0 );
    ctx.menu_state[0].y = 4;
    vxui_draw_list dl = stage_select_frame( &ctx, &screen, &resolver, VXUI_INPUT_UP );

    ASSERT_EQ( demo_menu_focused( &ctx, "stageselect" ), 3 );
    ASSERT_TRUE( text_is( dl, "stg_sector", "Helix Foundry" ) );
    ASSERT_TRUE( text_is( dl, "stg_status", "OPEN" ) );
    const vxui_draw_cmd* thumb = vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "stg_thumb" ) );
    ASSERT_TRUE( thumb );
    ASSERT_EQ( thumb->render.colour.r, s_sector_tint[3].r );
    ASSERT_EQ( thumb->render.colour.g, s_sector_tint[3].g );
    ASSERT_EQ( thumb->render.colour.b, s_sector_tint[3].b );
}

UTEST(demo_stage_select, no_input_keeps_preview_on_selected_sector) {
    vxui_ctx ctx;
    demo_screen_state screen;
    demo_resolver_state resolver;
    init_ctx( &ctx, &screen, &resolver );
    stage_select_frame( &ctx, &screen, &resolver, 0 );
    ctx.menu_state[0].y = 2;
    vxui_draw_list dl = stage_select_frame( &ctx, &screen, &resolver, 0 );

    ASSERT_EQ( demo_menu_focused( &ctx, "stageselect" ), 2 );
    ASSERT_TRUE( text_is( dl, "stg_sector", "Glass Reactor" ) );
    ASSERT_TRUE( text_is( dl, "stg_status", "CLEARED" ) );
    const vxui_draw_cmd* thumb = vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "stg_thumb" ) );
    ASSERT_TRUE( thumb );
    ASSERT_EQ( thumb->render.colour.r, s_sector_tint[2].r );
    ASSERT_EQ( thumb->render.colour.g, s_sector_tint[2].g );
    ASSERT_EQ( thumb->render.colour.b, s_sector_tint[2].b );
}

UTEST(demo_stage_select, horizontal_input_does_not_change_preview) {
    vxui_ctx ctx;
    demo_screen_state screen;
    demo_resolver_state resolver;
    init_ctx( &ctx, &screen, &resolver );
    stage_select_frame( &ctx, &screen, &resolver, 0 );
    ctx.menu_state[0].y = 2;
    vxui_draw_list dl = stage_select_frame( &ctx, &screen, &resolver, VXUI_INPUT_LEFT | VXUI_INPUT_RIGHT );

    ASSERT_EQ( demo_menu_focused( &ctx, "stageselect" ), 2 );
    ASSERT_TRUE( text_is( dl, "stg_sector", "Glass Reactor" ) );
    ASSERT_TRUE( text_is( dl, "stg_status", "CLEARED" ) );
    const vxui_draw_cmd* thumb = vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "stg_thumb" ) );
    ASSERT_TRUE( thumb );
    ASSERT_EQ( thumb->render.colour.r, s_sector_tint[2].r );
    ASSERT_EQ( thumb->render.colour.g, s_sector_tint[2].g );
    ASSERT_EQ( thumb->render.colour.b, s_sector_tint[2].b );
}

UTEST(demo_stage_select, held_navigation_updates_preview_only_when_repeat_fires) {
    vxui_ctx ctx;
    demo_screen_state screen;
    demo_resolver_state resolver;
    init_ctx( &ctx, &screen, &resolver );
    stage_select_frame( &ctx, &screen, &resolver, 0 );
    stage_select_frame( &ctx, &screen, &resolver, VXUI_INPUT_DOWN );
    vxui_draw_list dl = stage_select_frame( &ctx, &screen, &resolver, VXUI_INPUT_DOWN, 0.1f );

    ASSERT_EQ( demo_menu_focused( &ctx, "stageselect" ), 1 );
    ASSERT_TRUE( text_is( dl, "stg_sector", "Signal Array" ) );
    ASSERT_TRUE( text_is( dl, "stg_status", "CLEARED" ) );

    dl = stage_select_frame( &ctx, &screen, &resolver, VXUI_INPUT_DOWN, VXUI_INPUT_DELAY - 0.1f );
    ASSERT_EQ( demo_menu_focused( &ctx, "stageselect" ), 2 );
    ASSERT_TRUE( text_is( dl, "stg_sector", "Glass Reactor" ) );
    ASSERT_TRUE( text_is( dl, "stg_status", "CLEARED" ) );
    const vxui_draw_cmd* thumb = vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "stg_thumb" ) );
    ASSERT_TRUE( thumb );
    ASSERT_EQ( thumb->render.colour.r, s_sector_tint[2].r );
    ASSERT_EQ( thumb->render.colour.g, s_sector_tint[2].g );
    ASSERT_EQ( thumb->render.colour.b, s_sector_tint[2].b );

    dl = stage_select_frame( &ctx, &screen, &resolver, VXUI_INPUT_DOWN, VXUI_INPUT_REPEAT );
    ASSERT_EQ( demo_menu_focused( &ctx, "stageselect" ), 3 );
    ASSERT_TRUE( text_is( dl, "stg_sector", "Helix Foundry" ) );
    ASSERT_TRUE( text_is( dl, "stg_status", "OPEN" ) );
    thumb = vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "stg_thumb" ) );
    ASSERT_TRUE( thumb );
    ASSERT_EQ( thumb->render.colour.r, s_sector_tint[3].r );
    ASSERT_EQ( thumb->render.colour.g, s_sector_tint[3].g );
    ASSERT_EQ( thumb->render.colour.b, s_sector_tint[3].b );
}

UTEST_MAIN();
