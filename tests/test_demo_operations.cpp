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
    resolver->screen       = screen;
    resolver->ops_thumb_id = vxui_id( "ops_thumb" );
    vxui_set_render_data_fn( ctx, demo_render_data, resolver );
}

static vxui_draw_list operations_frame( vxui_ctx* ctx, demo_screen_state* screen, demo_resolver_state* resolver, uint32_t input, float dt = 1.0f / 60.0f )
{
    vxui_frame( ctx, dt );
    ctx->input = input;
    vxui_rect( ctx, "demo_frame", { .width = { VXUI_FIXED, DEMO_PORTRAIT_W }, .height = { VXUI_FIXED, DEMO_PORTRAIT_H }, .col = true, .padding = { 18, 18, 18, 18 }, .gap = 8 } );
        vxui_rect( ctx, "demo_panel", { .width = { VXUI_GROW, 0 }, .height = { VXUI_GROW, 0 }, .col = true, .padding = { 14, 14, 14, 14 }, .gap = 6 } );
            demo_dispatch_screen( ctx, screen, nullptr );
        vxui_div_end( ctx );
    vxui_div_end( ctx );

    int focused = demo_menu_focused( ctx, "operations" );
    assert( focused >= 0 && focused < DEMO_MISSION_COUNT );
    resolver->ops_thumb_texture = 42;
    resolver->ops_thumb_tint    = s_sector_tint[s_demo_missions[focused].sector];
    return vxui_render( ctx );
}

static bool text_is( const vxui_draw_list& dl, const char* id, const char* text )
{
    const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_TEXT, vxui_text_id( id ) );
    return c && c->text_len == (int) strlen( text ) && memcmp( c->text, text, c->text_len ) == 0;
}

UTEST(demo_operations, first_frame_preview_matches_initial_focus) {
    vxui_ctx ctx;
    demo_screen_state screen;
    demo_resolver_state resolver;
    init_ctx( &ctx, &screen, &resolver );
    vxui_draw_list dl = operations_frame( &ctx, &screen, &resolver, 0 );

    ASSERT_EQ( demo_menu_focused( &ctx, "operations" ), 0 );
    ASSERT_TRUE( text_is( dl, "ops_sector", "Debris Belt" ) );
    ASSERT_TRUE( text_is( dl, "ops_status", "PERFECT" ) );
    const vxui_draw_cmd* thumb = vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "ops_thumb" ) );
    ASSERT_TRUE( thumb );
    ASSERT_EQ( thumb->render.material_id, (uint32_t) DEMO_MATERIAL_IMAGE );
    ASSERT_EQ( thumb->render.texture_id, (uint32_t) 42 );
    ASSERT_EQ( thumb->render.colour.r, s_sector_tint[0].r );
    ASSERT_EQ( thumb->render.colour.g, s_sector_tint[0].g );
    ASSERT_EQ( thumb->render.colour.b, s_sector_tint[0].b );
    ASSERT_EQ( thumb->render.colour.a, s_sector_tint[0].a );
}

UTEST(demo_operations, first_frame_down_does_not_navigate_without_row_count) {
    vxui_ctx ctx;
    demo_screen_state screen;
    demo_resolver_state resolver;
    init_ctx( &ctx, &screen, &resolver );
    vxui_draw_list dl = operations_frame( &ctx, &screen, &resolver, VXUI_INPUT_DOWN );

    ASSERT_EQ( demo_menu_focused( &ctx, "operations" ), 0 );
    ASSERT_EQ( ctx.menu_state[0].z, (uint32_t) DEMO_MISSION_COUNT );
    ASSERT_TRUE( text_is( dl, "ops_sector", "Debris Belt" ) );
    ASSERT_TRUE( text_is( dl, "ops_status", "PERFECT" ) );
    const vxui_draw_cmd* thumb = vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "ops_thumb" ) );
    ASSERT_TRUE( thumb );
    ASSERT_EQ( thumb->render.colour.r, s_sector_tint[0].r );
    ASSERT_EQ( thumb->render.colour.g, s_sector_tint[0].g );
    ASSERT_EQ( thumb->render.colour.b, s_sector_tint[0].b );
}

UTEST(demo_operations, down_updates_preview_text_and_tint_in_same_frame) {
    vxui_ctx ctx;
    demo_screen_state screen;
    demo_resolver_state resolver;
    init_ctx( &ctx, &screen, &resolver );
    operations_frame( &ctx, &screen, &resolver, 0 );
    vxui_draw_list dl = operations_frame( &ctx, &screen, &resolver, VXUI_INPUT_DOWN );

    ASSERT_EQ( demo_menu_focused( &ctx, "operations" ), 1 );
    ASSERT_TRUE( text_is( dl, "ops_sector", "Signal Array" ) );
    ASSERT_TRUE( text_is( dl, "ops_status", "CLEARED" ) );
    const vxui_draw_cmd* thumb = vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "ops_thumb" ) );
    ASSERT_TRUE( thumb );
    ASSERT_EQ( thumb->render.material_id, (uint32_t) DEMO_MATERIAL_IMAGE );
    ASSERT_EQ( thumb->render.texture_id, (uint32_t) 42 );
    ASSERT_EQ( thumb->render.colour.r, s_sector_tint[1].r );
    ASSERT_EQ( thumb->render.colour.g, s_sector_tint[1].g );
    ASSERT_EQ( thumb->render.colour.b, s_sector_tint[1].b );
    ASSERT_EQ( thumb->render.colour.a, s_sector_tint[1].a );
}

UTEST(demo_operations, up_updates_preview_text_and_tint_in_same_frame) {
    vxui_ctx ctx;
    demo_screen_state screen;
    demo_resolver_state resolver;
    init_ctx( &ctx, &screen, &resolver );
    operations_frame( &ctx, &screen, &resolver, 0 );
    ctx.menu_state[0].y = 2;
    vxui_draw_list dl = operations_frame( &ctx, &screen, &resolver, VXUI_INPUT_UP );

    ASSERT_EQ( demo_menu_focused( &ctx, "operations" ), 1 );
    ASSERT_TRUE( text_is( dl, "ops_sector", "Signal Array" ) );
    ASSERT_TRUE( text_is( dl, "ops_status", "CLEARED" ) );
    const vxui_draw_cmd* thumb = vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "ops_thumb" ) );
    ASSERT_TRUE( thumb );
    ASSERT_EQ( thumb->render.colour.r, s_sector_tint[1].r );
    ASSERT_EQ( thumb->render.colour.g, s_sector_tint[1].g );
    ASSERT_EQ( thumb->render.colour.b, s_sector_tint[1].b );
}

UTEST(demo_operations, up_wraps_preview_to_last_mission) {
    vxui_ctx ctx;
    demo_screen_state screen;
    demo_resolver_state resolver;
    init_ctx( &ctx, &screen, &resolver );
    operations_frame( &ctx, &screen, &resolver, 0 );
    vxui_draw_list dl = operations_frame( &ctx, &screen, &resolver, VXUI_INPUT_UP );

    ASSERT_EQ( demo_menu_focused( &ctx, "operations" ), DEMO_MISSION_COUNT - 1 );
    ASSERT_TRUE( text_is( dl, "ops_sector", "?????" ) );
    ASSERT_TRUE( text_is( dl, "ops_status", "LOCKED" ) );
    const vxui_draw_cmd* thumb = vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "ops_thumb" ) );
    ASSERT_TRUE( thumb );
    ASSERT_EQ( thumb->render.colour.r, s_sector_tint[5].r );
    ASSERT_EQ( thumb->render.colour.g, s_sector_tint[5].g );
    ASSERT_EQ( thumb->render.colour.b, s_sector_tint[5].b );
}

UTEST(demo_operations, down_wraps_preview_to_first_mission) {
    vxui_ctx ctx;
    demo_screen_state screen;
    demo_resolver_state resolver;
    init_ctx( &ctx, &screen, &resolver );
    operations_frame( &ctx, &screen, &resolver, 0 );
    ctx.menu_state[0].y = DEMO_MISSION_COUNT - 1;
    vxui_draw_list dl = operations_frame( &ctx, &screen, &resolver, VXUI_INPUT_DOWN );

    ASSERT_EQ( demo_menu_focused( &ctx, "operations" ), 0 );
    ASSERT_TRUE( text_is( dl, "ops_sector", "Debris Belt" ) );
    ASSERT_TRUE( text_is( dl, "ops_status", "PERFECT" ) );
    const vxui_draw_cmd* thumb = vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "ops_thumb" ) );
    ASSERT_TRUE( thumb );
    ASSERT_EQ( thumb->render.colour.r, s_sector_tint[0].r );
    ASSERT_EQ( thumb->render.colour.g, s_sector_tint[0].g );
    ASSERT_EQ( thumb->render.colour.b, s_sector_tint[0].b );
}

UTEST(demo_operations, entering_locked_mission_updates_sector_and_status) {
    vxui_ctx ctx;
    demo_screen_state screen;
    demo_resolver_state resolver;
    init_ctx( &ctx, &screen, &resolver );
    operations_frame( &ctx, &screen, &resolver, 0 );
    ctx.menu_state[0].y = 4;
    vxui_draw_list dl = operations_frame( &ctx, &screen, &resolver, VXUI_INPUT_DOWN );

    ASSERT_EQ( demo_menu_focused( &ctx, "operations" ), 5 );
    ASSERT_TRUE( text_is( dl, "ops_sector", "?????" ) );
    ASSERT_TRUE( text_is( dl, "ops_status", "LOCKED" ) );
    const vxui_draw_cmd* thumb = vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "ops_thumb" ) );
    ASSERT_TRUE( thumb );
    ASSERT_EQ( thumb->render.colour.r, s_sector_tint[4].r );
    ASSERT_EQ( thumb->render.colour.g, s_sector_tint[4].g );
    ASSERT_EQ( thumb->render.colour.b, s_sector_tint[4].b );
}

UTEST(demo_operations, leaving_locked_mission_updates_sector_and_status) {
    vxui_ctx ctx;
    demo_screen_state screen;
    demo_resolver_state resolver;
    init_ctx( &ctx, &screen, &resolver );
    operations_frame( &ctx, &screen, &resolver, 0 );
    ctx.menu_state[0].y = 5;
    vxui_draw_list dl = operations_frame( &ctx, &screen, &resolver, VXUI_INPUT_UP );

    ASSERT_EQ( demo_menu_focused( &ctx, "operations" ), 4 );
    ASSERT_TRUE( text_is( dl, "ops_sector", "Helix Foundry" ) );
    ASSERT_TRUE( text_is( dl, "ops_status", "OPEN" ) );
    const vxui_draw_cmd* thumb = vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "ops_thumb" ) );
    ASSERT_TRUE( thumb );
    ASSERT_EQ( thumb->render.colour.r, s_sector_tint[3].r );
    ASSERT_EQ( thumb->render.colour.g, s_sector_tint[3].g );
    ASSERT_EQ( thumb->render.colour.b, s_sector_tint[3].b );
}

UTEST(demo_operations, no_input_keeps_preview_on_selected_mission) {
    vxui_ctx ctx;
    demo_screen_state screen;
    demo_resolver_state resolver;
    init_ctx( &ctx, &screen, &resolver );
    operations_frame( &ctx, &screen, &resolver, 0 );
    ctx.menu_state[0].y = 2;
    vxui_draw_list dl = operations_frame( &ctx, &screen, &resolver, 0 );

    ASSERT_EQ( demo_menu_focused( &ctx, "operations" ), 2 );
    ASSERT_TRUE( text_is( dl, "ops_sector", "Glass Reactor" ) );
    ASSERT_TRUE( text_is( dl, "ops_status", "CLEARED" ) );
    const vxui_draw_cmd* thumb = vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "ops_thumb" ) );
    ASSERT_TRUE( thumb );
    ASSERT_EQ( thumb->render.colour.r, s_sector_tint[2].r );
    ASSERT_EQ( thumb->render.colour.g, s_sector_tint[2].g );
    ASSERT_EQ( thumb->render.colour.b, s_sector_tint[2].b );
}

UTEST(demo_operations, same_sector_navigation_keeps_matching_preview) {
    vxui_ctx ctx;
    demo_screen_state screen;
    demo_resolver_state resolver;
    init_ctx( &ctx, &screen, &resolver );
    operations_frame( &ctx, &screen, &resolver, 0 );
    ctx.menu_state[0].y = 3;
    vxui_draw_list dl = operations_frame( &ctx, &screen, &resolver, VXUI_INPUT_DOWN );

    ASSERT_EQ( demo_menu_focused( &ctx, "operations" ), 4 );
    ASSERT_TRUE( text_is( dl, "ops_sector", "Helix Foundry" ) );
    ASSERT_TRUE( text_is( dl, "ops_status", "OPEN" ) );
    const vxui_draw_cmd* thumb = vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "ops_thumb" ) );
    ASSERT_TRUE( thumb );
    ASSERT_EQ( thumb->render.colour.r, s_sector_tint[3].r );
    ASSERT_EQ( thumb->render.colour.g, s_sector_tint[3].g );
    ASSERT_EQ( thumb->render.colour.b, s_sector_tint[3].b );
}

UTEST(demo_operations, held_navigation_updates_preview_only_when_repeat_fires) {
    vxui_ctx ctx;
    demo_screen_state screen;
    demo_resolver_state resolver;
    init_ctx( &ctx, &screen, &resolver );
    operations_frame( &ctx, &screen, &resolver, 0 );
    operations_frame( &ctx, &screen, &resolver, VXUI_INPUT_DOWN );
    vxui_draw_list dl = operations_frame( &ctx, &screen, &resolver, VXUI_INPUT_DOWN, 0.1f );

    ASSERT_EQ( demo_menu_focused( &ctx, "operations" ), 1 );
    ASSERT_TRUE( text_is( dl, "ops_sector", "Signal Array" ) );
    ASSERT_TRUE( text_is( dl, "ops_status", "CLEARED" ) );

    dl = operations_frame( &ctx, &screen, &resolver, VXUI_INPUT_DOWN, VXUI_INPUT_DELAY - 0.1f );
    ASSERT_EQ( demo_menu_focused( &ctx, "operations" ), 2 );
    ASSERT_TRUE( text_is( dl, "ops_sector", "Glass Reactor" ) );
    ASSERT_TRUE( text_is( dl, "ops_status", "CLEARED" ) );
    const vxui_draw_cmd* thumb = vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "ops_thumb" ) );
    ASSERT_TRUE( thumb );
    ASSERT_EQ( thumb->render.colour.r, s_sector_tint[2].r );
    ASSERT_EQ( thumb->render.colour.g, s_sector_tint[2].g );
    ASSERT_EQ( thumb->render.colour.b, s_sector_tint[2].b );

    dl = operations_frame( &ctx, &screen, &resolver, VXUI_INPUT_DOWN, VXUI_INPUT_REPEAT );
    ASSERT_EQ( demo_menu_focused( &ctx, "operations" ), 3 );
    ASSERT_TRUE( text_is( dl, "ops_sector", "Helix Foundry" ) );
    ASSERT_TRUE( text_is( dl, "ops_status", "OPEN" ) );
    thumb = vxui_draw_find( dl, VXUI_DRAW_RECT, vxui_id( "ops_thumb" ) );
    ASSERT_TRUE( thumb );
    ASSERT_EQ( thumb->render.colour.r, s_sector_tint[3].r );
    ASSERT_EQ( thumb->render.colour.g, s_sector_tint[3].g );
    ASSERT_EQ( thumb->render.colour.b, s_sector_tint[3].b );
}

UTEST_MAIN();
