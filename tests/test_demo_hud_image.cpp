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

static void init_ctx( vxui_ctx* ctx, vxui_hud* hud, demo_resolver_state* resolver )
{
    *ctx = {};
    vxui_init( ctx, 200, 200, s_clay_mem, sizeof( s_clay_mem ) );
    Clay_SetMeasureTextFunction( measure_text, nullptr );
    *resolver = {};
    resolver->hud = hud;
    vxui_set_render_data_fn( ctx, demo_render_data, resolver );
    vxui_frame( ctx, 1.0f / 60.0f );
    vxui_hud_begin( hud, ctx, 200, 200 );
}

UTEST(demo_hud_image, wide_image_fills_declared_rectangle) {
    vxui_ctx ctx;
    vxui_hud hud = {};
    demo_resolver_state resolver;
    init_ctx( &ctx, &hud, &resolver );
    vxui_hud_set_pos( &hud, 10, 30 );
    vxui_hud_image( &hud, "image", 42, 100, 20 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 1 );
    ASSERT_EQ( dl.cmds[0].render.material_id, (uint32_t) DEMO_MATERIAL_IMAGE );
    ve_fontcache cache = {};
    vxui_gl_state gl = {};
    vxui_gl_emit( &cache.drawlist, &gl, &cache, -1, dl, 200, 200, {} );
    ASSERT_EQ( cache.drawlist.dcalls.size(), (size_t) 1 );
    ASSERT_EQ( cache.drawlist.dcalls[0].pass, DEMO_GL_PASS_IMAGE );
    ASSERT_EQ( cache.drawlist.dcalls[0].region, (uint32_t) 42 );
    EXPECT_NEAR( cache.drawlist.vertices[0].x, 0.05f, 1e-6f );
    EXPECT_NEAR( cache.drawlist.vertices[0].y, 0.85f, 1e-6f );
    EXPECT_NEAR( cache.drawlist.vertices[2].x, 0.55f, 1e-6f );
    EXPECT_NEAR( cache.drawlist.vertices[2].y, 0.75f, 1e-6f );
}

UTEST(demo_hud_image, tall_image_uses_layout_transform) {
    vxui_ctx ctx;
    vxui_hud hud = {};
    demo_resolver_state resolver;
    init_ctx( &ctx, &hud, &resolver );
    vxui_hud_set_pos( &hud, 10, 20 );
    vxui_hud_image( &hud, "image", 42, 20, 100 );
    vxui_draw_list dl = vxui_render( &ctx );

    ve_fontcache cache = {};
    vxui_gl_state gl = {};
    vxui_gl_xform xf = { { 5, 10 }, 1.25f };
    vxui_gl_emit( &cache.drawlist, &gl, &cache, -1, dl, 200, 200, xf );
    ASSERT_EQ( cache.drawlist.dcalls.size(), (size_t) 1 );
    ASSERT_EQ( cache.drawlist.dcalls[0].pass, DEMO_GL_PASS_IMAGE );
    EXPECT_NEAR( cache.drawlist.vertices[0].x, 17.5f / 200.0f, 1e-6f );
    EXPECT_NEAR( cache.drawlist.vertices[0].y, 1.0f - 35.0f / 200.0f, 1e-6f );
    EXPECT_NEAR( cache.drawlist.vertices[2].x, 42.5f / 200.0f, 1e-6f );
    EXPECT_NEAR( cache.drawlist.vertices[2].y, 1.0f - 160.0f / 200.0f, 1e-6f );
}

UTEST(demo_hud_image, image_keeps_uv_and_tint) {
    vxui_ctx ctx;
    vxui_hud hud = {};
    demo_resolver_state resolver;
    init_ctx( &ctx, &hud, &resolver );
    vxui_hud_set_colour( &hud, { 0.2f, 0.3f, 0.4f, 0.5f } );
    vxui_hud_image( &hud, "image", 42, 100, 20, { 0.1f, 0.2f, 0.7f, 0.8f } );
    vxui_draw_list dl = vxui_render( &ctx );

    ve_fontcache cache = {};
    vxui_gl_state gl = {};
    vxui_gl_emit( &cache.drawlist, &gl, &cache, -1, dl, 200, 200, {} );
    ASSERT_EQ( cache.drawlist.dcalls.size(), (size_t) 1 );
    ASSERT_EQ( cache.drawlist.dcalls[0].pass, DEMO_GL_PASS_IMAGE );
    EXPECT_NEAR( cache.drawlist.vertices[0].u, 0.1f, 1e-6f );
    EXPECT_NEAR( cache.drawlist.vertices[0].v, 0.2f, 1e-6f );
    EXPECT_NEAR( cache.drawlist.vertices[2].u, 0.7f, 1e-6f );
    EXPECT_NEAR( cache.drawlist.vertices[2].v, 0.8f, 1e-6f );
    EXPECT_NEAR( cache.drawlist.dcalls[0].colour[0], 0.2f, 1e-6f );
    EXPECT_NEAR( cache.drawlist.dcalls[0].colour[1], 0.3f, 1e-6f );
    EXPECT_NEAR( cache.drawlist.dcalls[0].colour[2], 0.4f, 1e-6f );
    EXPECT_NEAR( cache.drawlist.dcalls[0].colour[3], 0.5f, 1e-6f );
}

UTEST(demo_hud_image, untextured_hud_rectangle_stays_solid) {
    vxui_ctx ctx;
    vxui_hud hud = {};
    demo_resolver_state resolver;
    init_ctx( &ctx, &hud, &resolver );
    vxui_hud_image( &hud, "image", 0, 100, 20 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 1 );
    ASSERT_EQ( dl.cmds[0].render.material_id, (uint32_t) 0 );
    ve_fontcache cache = {};
    vxui_gl_state gl = {};
    vxui_gl_emit( &cache.drawlist, &gl, &cache, -1, dl, 200, 200, {} );
    ASSERT_EQ( cache.drawlist.dcalls.size(), (size_t) 1 );
    ASSERT_EQ( cache.drawlist.dcalls[0].pass, DEMO_GL_PASS_RECT );
}

UTEST(demo_hud_image, stock_slots_are_full_images) {
    vxui_ctx ctx;
    vxui_hud hud = {};
    demo_resolver_state resolver;
    init_ctx( &ctx, &hud, &resolver );
    vxui_hud_resource_stock( &hud, "stock", 42, 2, 40, 10, VXUI_HUD_STOCK_ROW, { 0, 0, 1, 1 }, 5 );
    vxui_draw_list dl = vxui_render( &ctx );

    ve_fontcache cache = {};
    vxui_gl_state gl = {};
    vxui_gl_emit( &cache.drawlist, &gl, &cache, -1, dl, 200, 200, {} );
    ASSERT_EQ( cache.drawlist.dcalls.size(), (size_t) 2 );
    for ( int i = 0; i < 2; i++ )
    {
        ASSERT_EQ( cache.drawlist.dcalls[i].pass, DEMO_GL_PASS_IMAGE );
        ASSERT_EQ( cache.drawlist.dcalls[i].region, (uint32_t) 42 );
        EXPECT_NEAR( cache.drawlist.vertices[i * 4 + 2].x - cache.drawlist.vertices[i * 4].x, 0.2f, 1e-6f );
        EXPECT_NEAR( cache.drawlist.vertices[i * 4].y - cache.drawlist.vertices[i * 4 + 2].y, 0.05f, 1e-6f );
    }
}

UTEST(demo_hud_image, resource_bar_keeps_partial_width_and_uv) {
    vxui_ctx ctx;
    vxui_hud hud = {};
    demo_resolver_state resolver;
    init_ctx( &ctx, &hud, &resolver );
    vxui_hud_resource_bar( &hud, "bar", 42, 0.5f, 100, 20, { 0.2f, 0, 0.8f, 1 } );
    vxui_draw_list dl = vxui_render( &ctx );

    ve_fontcache cache = {};
    vxui_gl_state gl = {};
    vxui_gl_emit( &cache.drawlist, &gl, &cache, -1, dl, 200, 200, {} );
    ASSERT_EQ( cache.drawlist.dcalls.size(), (size_t) 1 );
    ASSERT_EQ( cache.drawlist.dcalls[0].pass, DEMO_GL_PASS_IMAGE );
    EXPECT_NEAR( cache.drawlist.vertices[2].x - cache.drawlist.vertices[0].x, 0.25f, 1e-6f );
    EXPECT_NEAR( cache.drawlist.vertices[0].u, 0.2f, 1e-6f );
    EXPECT_NEAR( cache.drawlist.vertices[2].u, 0.5f, 1e-6f );
}

UTEST(demo_hud_image, menu_texture_keeps_marker_semantics) {
    vxui_ctx ctx;
    vxui_hud hud = {};
    demo_resolver_state resolver;
    init_ctx( &ctx, &hud, &resolver );
    resolver.focus_icon_texture = 42;
    if ( vxui_menu( &ctx, "menu" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ve_fontcache cache = {};
    vxui_gl_state gl = {};
    vxui_gl_emit( &cache.drawlist, &gl, &cache, -1, dl, 200, 200, {} );
    ASSERT_EQ( cache.drawlist.dcalls.size(), (size_t) 3 );
    ASSERT_EQ( cache.drawlist.dcalls[0].pass, DEMO_GL_PASS_ROUND );
    ASSERT_EQ( cache.drawlist.dcalls[1].pass, DEMO_GL_PASS_OUTLINE );
    ASSERT_EQ( cache.drawlist.dcalls[2].pass, DEMO_GL_PASS_RECT_TEXTURED );
    ASSERT_EQ( cache.drawlist.dcalls[2].region, (uint32_t) 42 );
}

UTEST(demo_hud_image, text_is_not_given_image_material) {
    vxui_ctx ctx;
    vxui_hud hud = {};
    demo_resolver_state resolver;
    init_ctx( &ctx, &hud, &resolver );
    vxui_hud_set_colour( &hud, { 0.2f, 0.3f, 0.4f, 1 } );
    vxui_hud_text( &hud, "text", "alpha\nbeta" );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 2 );
    for ( int i = 0; i < dl.count; i++ )
    {
        ASSERT_EQ( dl.cmds[i].type, VXUI_DRAW_TEXT );
        ASSERT_EQ( dl.cmds[i].render.material_id, (uint32_t) 0 );
        ASSERT_EQ( dl.cmds[i].render.colour.r, 0.2f );
    }
}

UTEST_MAIN();
