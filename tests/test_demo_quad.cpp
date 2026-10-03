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

static vxui_draw_cmd textured_rect( uint32_t material, uint32_t texture )
{
    vxui_draw_cmd c      = {};
    c.type               = VXUI_DRAW_RECT;
    c.rect               = { 10, 15, 40, 20 };
    c.render.material_id = material;
    c.render.texture_id  = texture;
    c.render.uv          = { 0.1f, 0.2f, 0.8f, 0.9f };
    c.render.colour      = { 0.2f, 0.3f, 0.4f, 0.5f };
    return c;
}

UTEST(demo_quad, image_has_one_quad_with_full_geometry_and_uv) {
    ve_fontcache_drawlist dl = {};
    vxui_gl_state gl = {};
    vxui_draw_cmd c = textured_rect( DEMO_MATERIAL_IMAGE, 42 );
    vxui_gl_emit_one_rect( &dl, &gl, &c, 10, 15, 100, 100, {} );

    ASSERT_EQ( dl.dcalls.size(), (size_t) 1 );
    ASSERT_EQ( dl.vertices.size(), (size_t) 4 );
    ASSERT_EQ( dl.indices.size(), (size_t) 6 );
    ASSERT_EQ( dl.dcalls[0].pass, DEMO_GL_PASS_IMAGE );
    ASSERT_EQ( dl.dcalls[0].region, (uint32_t) 42 );
    ASSERT_EQ( dl.dcalls[0].start_index, (uint32_t) 0 );
    ASSERT_EQ( dl.dcalls[0].end_index, (uint32_t) 6 );
    float x[] = { 0.1f, 0.5f, 0.5f, 0.1f };
    float y[] = { 0.85f, 0.85f, 0.65f, 0.65f };
    float u[] = { 0.1f, 0.8f, 0.8f, 0.1f };
    float v[] = { 0.2f, 0.2f, 0.9f, 0.9f };
    for ( int i = 0; i < 4; i++ )
    {
        EXPECT_NEAR( dl.vertices[i].x, x[i], 1e-6f );
        EXPECT_NEAR( dl.vertices[i].y, y[i], 1e-6f );
        EXPECT_NEAR( dl.vertices[i].u, u[i], 1e-6f );
        EXPECT_NEAR( dl.vertices[i].v, v[i], 1e-6f );
        EXPECT_NEAR( dl.dcalls[0].colour[i], c.render.colour[i], 1e-6f );
    }
    uint32_t indices[] = { 0, 1, 2, 0, 2, 3 };
    for ( int i = 0; i < 6; i++ ) ASSERT_EQ( dl.indices[i], indices[i] );
}

UTEST(demo_quad, marker_is_square_after_the_solid_quad) {
    ve_fontcache_drawlist dl = {};
    vxui_gl_state gl = {};
    vxui_draw_cmd c = textured_rect( 0, 42 );
    vxui_gl_emit_one_rect( &dl, &gl, &c, 10, 15, 100, 100, {} );

    ASSERT_EQ( dl.dcalls.size(), (size_t) 2 );
    ASSERT_EQ( dl.vertices.size(), (size_t) 8 );
    ASSERT_EQ( dl.indices.size(), (size_t) 12 );
    ASSERT_EQ( dl.dcalls[0].pass, DEMO_GL_PASS_RECT );
    ASSERT_EQ( dl.dcalls[1].pass, DEMO_GL_PASS_RECT_TEXTURED );
    ASSERT_EQ( dl.dcalls[1].region, (uint32_t) 42 );
    ASSERT_EQ( dl.dcalls[1].start_index, (uint32_t) 6 );
    ASSERT_EQ( dl.dcalls[1].end_index, (uint32_t) 12 );
    float x[] = { 0.14f, 0.34f, 0.34f, 0.14f };
    float y[] = { 0.85f, 0.85f, 0.65f, 0.65f };
    float u[] = { 0.1f, 0.8f, 0.8f, 0.1f };
    float v[] = { 0.2f, 0.2f, 0.9f, 0.9f };
    for ( int i = 0; i < 4; i++ )
    {
        EXPECT_NEAR( dl.vertices[i + 4].x, x[i], 1e-6f );
        EXPECT_NEAR( dl.vertices[i + 4].y, y[i], 1e-6f );
        EXPECT_NEAR( dl.vertices[i + 4].u, u[i], 1e-6f );
        EXPECT_NEAR( dl.vertices[i + 4].v, v[i], 1e-6f );
        EXPECT_NEAR( dl.dcalls[1].colour[i], c.render.colour[i], 1e-6f );
    }
    uint32_t indices[] = { 4, 5, 6, 4, 6, 7 };
    for ( int i = 0; i < 6; i++ ) ASSERT_EQ( dl.indices[i + 6], indices[i] );
}

UTEST(demo_quad, appended_images_keep_offsets_and_texture_ids) {
    ve_fontcache_drawlist dl = {};
    vxui_gl_state gl = {};
    for ( int i = 0; i < 3; i++ )
    {
        vxui_draw_cmd c = textured_rect( DEMO_MATERIAL_IMAGE, 42 + i );
        vxui_gl_emit_one_rect( &dl, &gl, &c, 10, 15, 100, 100, {} );
    }

    ASSERT_EQ( dl.dcalls.size(), (size_t) 3 );
    ASSERT_EQ( dl.vertices.size(), (size_t) 12 );
    ASSERT_EQ( dl.indices.size(), (size_t) 18 );
    uint32_t indices[] = { 0, 1, 2, 0, 2, 3 };
    for ( int i = 0; i < 3; i++ )
    {
        ASSERT_EQ( dl.dcalls[i].pass, DEMO_GL_PASS_IMAGE );
        ASSERT_EQ( dl.dcalls[i].region, (uint32_t) ( 42 + i ) );
        ASSERT_EQ( dl.dcalls[i].start_index, (uint32_t) ( i * 6 ) );
        ASSERT_EQ( dl.dcalls[i].end_index, (uint32_t) ( ( i + 1 ) * 6 ) );
        for ( int j = 0; j < 6; j++ ) ASSERT_EQ( dl.indices[i * 6 + j], indices[j] + i * 4 );
    }
}

UTEST(demo_quad, appended_markers_keep_offsets_and_texture_ids) {
    ve_fontcache_drawlist dl = {};
    vxui_gl_state gl = {};
    for ( int i = 0; i < 3; i++ )
    {
        vxui_draw_cmd c = textured_rect( 0, 42 + i );
        vxui_gl_emit_one_rect( &dl, &gl, &c, 10, 15, 100, 100, {} );
    }

    ASSERT_EQ( dl.dcalls.size(), (size_t) 6 );
    ASSERT_EQ( dl.vertices.size(), (size_t) 24 );
    ASSERT_EQ( dl.indices.size(), (size_t) 36 );
    uint32_t indices[] = { 0, 1, 2, 0, 2, 3 };
    for ( int i = 0; i < 3; i++ )
    {
        int marker = i * 2 + 1;
        ASSERT_EQ( dl.dcalls[marker].pass, DEMO_GL_PASS_RECT_TEXTURED );
        ASSERT_EQ( dl.dcalls[marker].region, (uint32_t) ( 42 + i ) );
        ASSERT_EQ( dl.dcalls[marker].start_index, (uint32_t) ( i * 12 + 6 ) );
        ASSERT_EQ( dl.dcalls[marker].end_index, (uint32_t) ( i * 12 + 12 ) );
        for ( int j = 0; j < 6; j++ ) ASSERT_EQ( dl.indices[i * 12 + 6 + j], indices[j] + i * 8 + 4 );
    }
}

UTEST(demo_quad, image_and_marker_passes_remain_distinct_after_optimisation) {
    ve_fontcache cache = {};
    vxui_gl_state gl = {};
    vxui_draw_cmd image = textured_rect( DEMO_MATERIAL_IMAGE, 42 );
    vxui_draw_cmd marker = textured_rect( 0, 42 );
    vxui_gl_emit_one_rect( &cache.drawlist, &gl, &image, 10, 15, 100, 100, {} );
    vxui_gl_emit_one_rect( &cache.drawlist, &gl, &marker, 10, 15, 100, 100, {} );
    ve_fontcache_optimise_drawlist( &cache );

    ASSERT_EQ( cache.drawlist.dcalls.size(), (size_t) 3 );
    ASSERT_EQ( cache.drawlist.dcalls[0].pass, DEMO_GL_PASS_IMAGE );
    ASSERT_EQ( cache.drawlist.dcalls[1].pass, DEMO_GL_PASS_RECT );
    ASSERT_EQ( cache.drawlist.dcalls[2].pass, DEMO_GL_PASS_RECT_TEXTURED );
    ASSERT_EQ( cache.drawlist.dcalls[0].region, (uint32_t) 42 );
    ASSERT_EQ( cache.drawlist.dcalls[2].region, (uint32_t) 42 );
}

UTEST(demo_quad, reversed_uvs_are_not_reordered) {
    ve_fontcache_drawlist dl = {};
    vxui_gl_state gl = {};
    vxui_draw_cmd c = textured_rect( DEMO_MATERIAL_IMAGE, 42 );
    c.render.uv = { 0.8f, 0.9f, 0.1f, 0.2f };
    vxui_gl_emit_one_rect( &dl, &gl, &c, 10, 15, 100, 100, {} );

    EXPECT_NEAR( dl.vertices[0].u, 0.8f, 1e-6f );
    EXPECT_NEAR( dl.vertices[0].v, 0.9f, 1e-6f );
    EXPECT_NEAR( dl.vertices[2].u, 0.1f, 1e-6f );
    EXPECT_NEAR( dl.vertices[2].v, 0.2f, 1e-6f );
}

UTEST(demo_quad, transparent_tint_with_rgb_is_preserved) {
    ve_fontcache_drawlist dl = {};
    vxui_gl_state gl = {};
    vxui_draw_cmd c = textured_rect( DEMO_MATERIAL_IMAGE, 42 );
    c.render.colour = { 0.2f, 0.3f, 0.4f, 0 };
    vxui_gl_emit_one_rect( &dl, &gl, &c, 10, 15, 100, 100, {} );

    ASSERT_EQ( dl.dcalls.size(), (size_t) 1 );
    for ( int i = 0; i < 4; i++ ) ASSERT_EQ( dl.dcalls[0].colour[i], c.render.colour[i] );
}

UTEST_MAIN();
