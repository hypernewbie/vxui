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

static void init_ctx( vxui_ctx* ctx, demo_resolver_state* state, vxui_hud* hud )
{
    *ctx = {};
    vxui_init( ctx, DEMO_PORTRAIT_W, DEMO_PORTRAIT_H, s_clay_mem, sizeof( s_clay_mem ) );
    Clay_SetMeasureTextFunction( measure_text, nullptr );
    *state = {};
    *hud = {};
    state->panel_id  = vxui_id( "demo_panel" );
    state->frame_id  = vxui_id( "demo_frame" );
    state->bg_id     = vxui_id( "demo_bg" );
    state->sample_id = vxui_id( "showcase_sample" );
    state->hud       = hud;
    vxui_set_render_data_fn( ctx, demo_render_data, state );
}

static bool showcase_frame( vxui_ctx* ctx, demo_resolver_state* state, uint32_t input, float dt, vxui_draw_list* dl, float mouse_x = FLT_MAX, float mouse_y = FLT_MAX )
{
    vxui_frame( ctx, dt, (float) DEMO_PORTRAIT_W, (float) DEMO_PORTRAIT_H );
    ctx->input = input;
    vxui_mouse( ctx, mouse_x, mouse_y, 0 );
    vxui_hud_begin( state->hud, ctx, (float) DEMO_PORTRAIT_W, (float) DEMO_PORTRAIT_H );
    vxui_rect( ctx, "demo_frame", { .width = { VXUI_FIXED, DEMO_PORTRAIT_W }, .height = { VXUI_FIXED, DEMO_PORTRAIT_H }, .col = true, .padding = { 18, 18, 18, 18 }, .gap = 8 } );
        vxui_rect( ctx, "demo_panel", { .width = { VXUI_GROW, 0 }, .height = { VXUI_GROW, 0 }, .col = true, .padding = { 14, 14, 14, 14 }, .gap = 6 } );
            bool close = demo_showcase( ctx, state );
        vxui_div_end( ctx );
    vxui_div_end( ctx );
    *dl = vxui_render( ctx );
    return close;
}

static const vxui_draw_cmd* menu_row( const vxui_draw_list& dl, int row )
{
    for ( int i = 0; i < dl.count; i++ )
        if ( dl.cmds[i].type == VXUI_DRAW_RECT && dl.cmds[i].row_index == row ) return &dl.cmds[i];
    return nullptr;
}

static bool text_is( const vxui_draw_list& dl, const char* id, const char* text )
{
    const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_TEXT, vxui_text_id( id ) );
    return c && c->text_len == (int) strlen( text ) && memcmp( c->text, text, c->text_len ) == 0;
}

UTEST(demo_showcase, first_frame_has_fixed_menu_and_opaque_rounded_sample) {
    vxui_ctx ctx;
    demo_resolver_state state;
    vxui_hud hud;
    init_ctx( &ctx, &state, &hud );
    vxui_draw_list dl = {};
    ASSERT_FALSE( showcase_frame( &ctx, &state, 0, 1.0f / 60.0f, &dl ) );

    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 0 );
    ASSERT_EQ( ctx.menu_state[0].z, (uint32_t) 18 );
    const vxui_draw_cmd* row = menu_row( dl, 0 );
    ASSERT_TRUE( row );
    ASSERT_EQ( row->rect.z, 240.0f );
    for ( int i = 0; i < 18; i++ )
    {
        row = menu_row( dl, i );
        ASSERT_TRUE( row );
        ASSERT_EQ( row->render.texture_id, (uint32_t) 0 );
    }
    const vxui_draw_cmd* sample = vxui_draw_find( dl, VXUI_DRAW_RECT, state.sample_id );
    ASSERT_TRUE( sample );
    ASSERT_EQ( sample->render.material_id, (uint32_t) DEMO_MATERIAL_ROUND );
    ASSERT_EQ( sample->render.texture_id, (uint32_t) 0 );
    ASSERT_EQ( sample->render.params[0], 4.0f );
    ASSERT_EQ( sample->render.params[1], 1.0f );
    ASSERT_EQ( sample->render.colour.r, s_demo_colours[0].r );
    ASSERT_EQ( sample->render.colour.g, s_demo_colours[0].g );
    ASSERT_EQ( sample->render.colour.b, s_demo_colours[0].b );
    ASSERT_EQ( sample->render.colour.a, 1.0f );
    ASSERT_EQ( sample->rect.x, 278.0f );
    ASSERT_EQ( sample->rect.z, 170.0f );
    ASSERT_EQ( sample->rect.w, 200.0f );
    ASSERT_TRUE( text_is( dl, "showcase_selected", "Item 01" ) );

    ve_fontcache_drawlist fdl = {};
    vxui_gl_state gl = {};
    vxui_gl_emit_one_rect( &fdl, &gl, sample, sample->rect.x, sample->rect.y, DEMO_PORTRAIT_W, DEMO_PORTRAIT_H, {} );
    ASSERT_EQ( fdl.dcalls.size(), (size_t) 1 );
    ASSERT_EQ( fdl.dcalls[0].pass, DEMO_GL_PASS_ROUND );
}

UTEST(demo_showcase, hovered_row_keeps_its_fill_without_a_texture) {
    vxui_ctx ctx;
    demo_resolver_state state;
    vxui_hud hud;
    init_ctx( &ctx, &state, &hud );
    vxui_draw_list dl = {};
    showcase_frame( &ctx, &state, 0, 1.0f / 60.0f, &dl );
    const vxui_draw_cmd* row = menu_row( dl, 1 );
    ASSERT_TRUE( row );
    float mouse_x = row->rect.x + row->rect.z * 0.5f;
    float mouse_y = row->rect.y + row->rect.w * 0.5f;
    showcase_frame( &ctx, &state, 0, 1.0f / 60.0f, &dl, mouse_x, mouse_y );
    row = menu_row( dl, 1 );

    ASSERT_TRUE( row );
    ASSERT_TRUE( row->state & VXUI_DRAW_HOVERED );
    ASSERT_EQ( row->render.texture_id, (uint32_t) 0 );
    ASSERT_EQ( row->render.colour.r, 0.16f );
    ASSERT_EQ( row->render.colour.g, 0.24f );
    ASSERT_EQ( row->render.colour.b, 0.36f );
    ASSERT_EQ( row->render.colour.a, 0.80f );
}

UTEST(demo_showcase, first_frame_down_only_mounts_the_menu) {
    vxui_ctx ctx;
    demo_resolver_state state;
    vxui_hud hud;
    init_ctx( &ctx, &state, &hud );
    vxui_draw_list dl = {};
    showcase_frame( &ctx, &state, VXUI_INPUT_DOWN, 1.0f / 60.0f, &dl );

    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 0 );
    ASSERT_EQ( ctx.menu_state[0].z, (uint32_t) 18 );
    ASSERT_TRUE( text_is( dl, "showcase_selected", "Item 01" ) );
}

UTEST(demo_showcase, pressed_row_keeps_its_fill_without_a_texture) {
    vxui_ctx ctx;
    demo_resolver_state state;
    vxui_hud hud;
    init_ctx( &ctx, &state, &hud );
    vxui_draw_list dl = {};
    showcase_frame( &ctx, &state, VXUI_INPUT_CONFIRM, 1.0f / 60.0f, &dl );
    const vxui_draw_cmd* row = menu_row( dl, 0 );

    ASSERT_TRUE( row );
    ASSERT_TRUE( row->state & VXUI_DRAW_PRESSED );
    ASSERT_EQ( row->render.texture_id, (uint32_t) 0 );
    ASSERT_EQ( row->render.colour.r, 0.55f );
    ASSERT_EQ( row->render.colour.g, 0.95f );
    ASSERT_EQ( row->render.colour.b, 1.00f );
    ASSERT_EQ( row->render.colour.a, 0.95f );
}

UTEST(demo_showcase, final_focus_selects_item_label_and_colour_in_same_frame) {
    vxui_ctx ctx;
    demo_resolver_state state;
    vxui_hud hud;
    init_ctx( &ctx, &state, &hud );
    vxui_draw_list dl = {};
    showcase_frame( &ctx, &state, 0, 1.0f / 60.0f, &dl );
    ctx.menu_state[0].y = 4;
    showcase_frame( &ctx, &state, VXUI_INPUT_DOWN, 1.0f / 60.0f, &dl );

    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 5 );
    ASSERT_EQ( state.selected, 1 );
    ASSERT_TRUE( text_is( dl, "showcase_selected", "Item 02" ) );
    const vxui_draw_cmd* sample = vxui_draw_find( dl, VXUI_DRAW_RECT, state.sample_id );
    ASSERT_TRUE( sample );
    ASSERT_EQ( sample->render.colour.r, s_demo_colours[1].r );
    ASSERT_EQ( sample->render.colour.g, s_demo_colours[1].g );
    ASSERT_EQ( sample->render.colour.b, s_demo_colours[1].b );
}

UTEST(demo_showcase, focused_row_keeps_its_outline_and_spring_without_a_texture) {
    vxui_ctx ctx;
    demo_resolver_state state;
    vxui_hud hud;
    init_ctx( &ctx, &state, &hud );
    vxui_draw_list dl = {};
    showcase_frame( &ctx, &state, 0, 1.0f / 60.0f, &dl );
    ctx.menu_state[0].y = 4;
    showcase_frame( &ctx, &state, VXUI_INPUT_DOWN, 1.0f / 60.0f, &dl );
    const vxui_draw_cmd* row = menu_row( dl, 5 );

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

UTEST(demo_showcase, distance_ramp_keeps_its_colour_without_an_icon) {
    vxui_ctx ctx;
    demo_resolver_state state;
    vxui_hud hud;
    init_ctx( &ctx, &state, &hud );
    vxui_draw_list dl = {};
    showcase_frame( &ctx, &state, 0, 1.0f / 60.0f, &dl );
    const vxui_draw_cmd* row = menu_row( dl, 3 );

    ASSERT_TRUE( row );
    ASSERT_EQ( row->render.texture_id, (uint32_t) 0 );
    ASSERT_EQ( row->render.colour.r, 0.03f );
    ASSERT_EQ( row->render.colour.g, 0.07f );
    ASSERT_EQ( row->render.colour.b, 0.14f );
    ASSERT_EQ( row->render.colour.a, 0.40f );
}

UTEST(demo_showcase, up_wraps_to_the_last_item) {
    vxui_ctx ctx;
    demo_resolver_state state;
    vxui_hud hud;
    init_ctx( &ctx, &state, &hud );
    vxui_draw_list dl = {};
    showcase_frame( &ctx, &state, 0, 1.0f / 60.0f, &dl );
    showcase_frame( &ctx, &state, VXUI_INPUT_UP, 1.0f / 60.0f, &dl );

    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 17 );
    ASSERT_TRUE( text_is( dl, "showcase_selected", "Item 14" ) );
    ASSERT_EQ( state.selected, 13 );
    const vxui_draw_cmd* last = menu_row( dl, 17 );
    ASSERT_TRUE( last );
    ASSERT_TRUE( last->clipped );
    ASSERT_EQ( last->clip.z, 240.0f );
    ASSERT_EQ( last->clip.w, (float) ( 9 * VXUI_ROW_HEIGHT ) );
    const vxui_draw_cmd* sample = vxui_draw_find( dl, VXUI_DRAW_RECT, state.sample_id );
    ASSERT_TRUE( sample );
    ASSERT_EQ( sample->render.colour.r, s_demo_colours[1].r );
    ASSERT_EQ( sample->render.colour.g, s_demo_colours[1].g );
    ASSERT_EQ( sample->render.colour.b, s_demo_colours[1].b );
}

UTEST(demo_showcase, down_wraps_to_the_material_control) {
    vxui_ctx ctx;
    demo_resolver_state state;
    vxui_hud hud;
    init_ctx( &ctx, &state, &hud );
    vxui_draw_list dl = {};
    showcase_frame( &ctx, &state, 0, 1.0f / 60.0f, &dl );
    ctx.menu_state[0].y = 17;
    showcase_frame( &ctx, &state, VXUI_INPUT_DOWN, 1.0f / 60.0f, &dl );

    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 0 );
    ASSERT_TRUE( text_is( dl, "showcase_selected", "Item 01" ) );
    ASSERT_EQ( state.selected, 0 );
}

UTEST(demo_showcase, held_navigation_keeps_the_existing_repeat_rate) {
    vxui_ctx ctx;
    demo_resolver_state state;
    vxui_hud hud;
    init_ctx( &ctx, &state, &hud );
    vxui_draw_list dl = {};
    showcase_frame( &ctx, &state, 0, 1.0f / 60.0f, &dl );
    ctx.menu_state[0].y = 4;
    showcase_frame( &ctx, &state, VXUI_INPUT_DOWN, 1.0f / 60.0f, &dl );
    showcase_frame( &ctx, &state, VXUI_INPUT_DOWN, 0.1f, &dl );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 5 );
    showcase_frame( &ctx, &state, VXUI_INPUT_DOWN, VXUI_INPUT_DELAY - 0.1f, &dl );

    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 6 );
    ASSERT_TRUE( text_is( dl, "showcase_selected", "Item 03" ) );
}

UTEST(demo_showcase, material_option_drives_solid_rounded_and_crt_packets) {
    vxui_ctx ctx;
    demo_resolver_state state;
    vxui_hud hud;
    init_ctx( &ctx, &state, &hud );
    vxui_draw_list dl = {};
    showcase_frame( &ctx, &state, 0, 1.0f / 60.0f, &dl );
    showcase_frame( &ctx, &state, VXUI_INPUT_RIGHT, 1.0f / 60.0f, &dl );

    const vxui_draw_cmd* sample = vxui_draw_find( dl, VXUI_DRAW_RECT, state.sample_id );
    ASSERT_TRUE( sample );
    ASSERT_EQ( state.material_index, 2 );
    ASSERT_EQ( sample->render.material_id, (uint32_t) DEMO_MATERIAL_CRT );
    ASSERT_EQ( sample->render.flags, (uint32_t) DEMO_MATERIAL_FLAG_SCANLINES );

    ve_fontcache_drawlist fdl = {};
    vxui_gl_state gl = {};
    vxui_gl_emit_one_rect( &fdl, &gl, sample, sample->rect.x, sample->rect.y, DEMO_PORTRAIT_W, DEMO_PORTRAIT_H, {} );
    ASSERT_EQ( fdl.dcalls[0].pass, DEMO_GL_PASS_CRT );
    ASSERT_EQ( gl.material_data[0].flags, (uint32_t) DEMO_MATERIAL_FLAG_SCANLINES );

    showcase_frame( &ctx, &state, 0, 1.0f / 60.0f, &dl );
    showcase_frame( &ctx, &state, VXUI_INPUT_RIGHT, 1.0f / 60.0f, &dl );
    sample = vxui_draw_find( dl, VXUI_DRAW_RECT, state.sample_id );
    ASSERT_EQ( state.material_index, 0 );
    ASSERT_EQ( sample->render.material_id, (uint32_t) 0 );
    fdl = {};
    gl = {};
    vxui_gl_emit_one_rect( &fdl, &gl, sample, sample->rect.x, sample->rect.y, DEMO_PORTRAIT_W, DEMO_PORTRAIT_H, {} );
    ASSERT_EQ( fdl.dcalls[0].pass, DEMO_GL_PASS_RECT );
}

UTEST(demo_showcase, radius_slider_changes_the_rounded_sample) {
    vxui_ctx ctx;
    demo_resolver_state state;
    vxui_hud hud;
    init_ctx( &ctx, &state, &hud );
    vxui_draw_list dl = {};
    showcase_frame( &ctx, &state, 0, 1.0f / 60.0f, &dl );
    ctx.menu_state[0].y = 1;
    showcase_frame( &ctx, &state, VXUI_INPUT_RIGHT, 1.0f / 60.0f, &dl );

    const vxui_draw_cmd* sample = vxui_draw_find( dl, VXUI_DRAW_RECT, state.sample_id );
    ASSERT_TRUE( sample );
    ASSERT_EQ( state.radius, 5.0f );
    ASSERT_EQ( sample->render.params[0], 5.0f );
}

UTEST(demo_showcase, intensity_slider_changes_the_crt_sample) {
    vxui_ctx ctx;
    demo_resolver_state state;
    vxui_hud hud;
    init_ctx( &ctx, &state, &hud );
    state.material_index = 2;
    vxui_draw_list dl = {};
    showcase_frame( &ctx, &state, 0, 1.0f / 60.0f, &dl );
    ctx.menu_state[0].y = 2;
    showcase_frame( &ctx, &state, VXUI_INPUT_RIGHT, 1.0f / 60.0f, &dl );

    const vxui_draw_cmd* sample = vxui_draw_find( dl, VXUI_DRAW_RECT, state.sample_id );
    ASSERT_TRUE( sample );
    ASSERT_NEAR( state.intensity, 0.40f, 1e-6f );
    ASSERT_NEAR( sample->render.params[1], 0.40f, 1e-6f );
}

UTEST(demo_showcase, cancel_requests_application_close) {
    vxui_ctx ctx;
    demo_resolver_state state;
    vxui_hud hud;
    init_ctx( &ctx, &state, &hud );
    vxui_draw_list dl = {};
    showcase_frame( &ctx, &state, 0, 1.0f / 60.0f, &dl );

    ASSERT_TRUE( showcase_frame( &ctx, &state, VXUI_INPUT_CANCEL, 1.0f / 60.0f, &dl ) );
}

UTEST_MAIN();
