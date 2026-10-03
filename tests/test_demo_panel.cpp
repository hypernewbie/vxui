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

UTEST(demo_panel, transparent_panel_has_no_material_or_scanline_parameters) {
    demo_resolver_state resolver = {};
    resolver.panel_id            = vxui_id( "panel" );
    resolver.time_seconds        = 3.5f;
    vxui_draw_cmd c = {};
    c.id            = resolver.panel_id;
    c.type          = VXUI_DRAW_RECT;
    vxui_render_data out = {};
    demo_render_data( &c, &out, &resolver );

    ASSERT_EQ( out.material_id, (uint32_t) 0 );
    ASSERT_EQ( out.flags, (uint32_t) 0 );
    ASSERT_EQ( out.texture_id, (uint32_t) 0 );
    ASSERT_EQ( out.outline_thickness, 0.0f );
    for ( int i = 0; i < 4; i++ ) ASSERT_EQ( out.colour[i], 0.0f );
    for ( int i = 0; i < 8; i++ ) ASSERT_EQ( out.params[i], 0.0f );
}

UTEST(demo_panel, panel_ignores_hover_and_focus_decoration) {
    demo_resolver_state resolver = {};
    resolver.panel_id            = vxui_id( "panel" );
    vxui_draw_cmd c = {};
    c.id            = resolver.panel_id;
    c.type          = VXUI_DRAW_RECT;
    c.state         = VXUI_DRAW_HOVERED | VXUI_DRAW_FOCUSED | VXUI_DRAW_PRESSED;
    vxui_render_data out = {};
    demo_render_data( &c, &out, &resolver );

    ASSERT_EQ( out.material_id, (uint32_t) 0 );
    ASSERT_EQ( out.colour.a, 0.0f );
    ASSERT_EQ( out.outline_thickness, 0.0f );
}

UTEST(demo_panel, panel_geometry_and_children_are_unchanged) {
    vxui_ctx ctx = {};
    vxui_init( &ctx, 200, 200, s_clay_mem, sizeof( s_clay_mem ) );
    demo_resolver_state resolver = {};
    resolver.panel_id = vxui_id( "panel" );
    vxui_set_render_data_fn( &ctx, demo_render_data, &resolver );
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_rect( &ctx, "panel", { .width = { VXUI_FIXED, 100 }, .height = { VXUI_FIXED, 80 }, .col = true } );
        vxui_text( &ctx, "child", "alpha", 24 );
    vxui_div_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 2 );
    ASSERT_EQ( dl.cmds[0].id, resolver.panel_id );
    ASSERT_EQ( dl.cmds[0].rect.z, 100.0f );
    ASSERT_EQ( dl.cmds[0].rect.w, 80.0f );
    ASSERT_EQ( dl.cmds[0].render.material_id, (uint32_t) 0 );
    ASSERT_EQ( dl.cmds[1].type, VXUI_DRAW_TEXT );
    ASSERT_EQ( dl.cmds[1].text_len, 5 );
    ASSERT_EQ( memcmp( dl.cmds[1].text, "alpha", 5 ), 0 );
    ASSERT_EQ( dl.cmds[1].render.colour.r, 0.85f );

    ve_fontcache cache = {};
    vxui_gl_state gl = {};
    vxui_gl_emit( &cache.drawlist, &gl, &cache, -1, dl, 200, 200, {} );
    ASSERT_EQ( gl.material_data_count, 0 );
    ASSERT_EQ( cache.drawlist.dcalls.size(), (size_t) 1 );
    ASSERT_EQ( cache.drawlist.dcalls[0].pass, DEMO_GL_PASS_RECT );
    ASSERT_EQ( cache.drawlist.dcalls[0].colour[3], 0.0f );
}

UTEST(demo_panel, background_remains_opaque_deep_space) {
    demo_resolver_state resolver = {};
    resolver.bg_id = vxui_id( "background" );
    vxui_draw_cmd c = {};
    c.id            = resolver.bg_id;
    c.type          = VXUI_DRAW_RECT;
    vxui_render_data out = {};
    demo_render_data( &c, &out, &resolver );

    ASSERT_EQ( out.material_id, (uint32_t) 0 );
    ASSERT_EQ( out.colour.r, 0.02f );
    ASSERT_EQ( out.colour.g, 0.03f );
    ASSERT_EQ( out.colour.b, 0.07f );
    ASSERT_EQ( out.colour.a, 1.0f );
}

UTEST(demo_panel, frame_outline_is_not_removed) {
    demo_resolver_state resolver = {};
    resolver.frame_id = vxui_id( "frame" );
    vxui_draw_cmd c = {};
    c.id            = resolver.frame_id;
    c.type          = VXUI_DRAW_RECT;
    vxui_render_data out = {};
    demo_render_data( &c, &out, &resolver );

    ASSERT_EQ( out.material_id, (uint32_t) 0 );
    ASSERT_EQ( out.colour.a, 0.0f );
    ASSERT_EQ( out.outline_thickness, 2.0f );
    ASSERT_EQ( out.outline_colour.r, 0.30f );
    ASSERT_EQ( out.outline_colour.g, 0.70f );
    ASSERT_EQ( out.outline_colour.b, 0.95f );
    ASSERT_EQ( out.outline_colour.a, 1.0f );
}

UTEST_MAIN();
