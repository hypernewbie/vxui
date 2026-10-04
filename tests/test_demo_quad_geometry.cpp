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

static vxui_draw_cmd rectangle( uint32_t material )
{
    vxui_draw_cmd c      = {};
    c.type               = VXUI_DRAW_RECT;
    c.rect               = { 10, 15, 40, 20 };
    c.render.material_id = material;
    c.render.colour      = { 0.2f, 0.3f, 0.4f, 0.5f };
    return c;
}

UTEST(demo_quad_geometry, solid_quad_has_zero_uvs_and_no_material_slot) {
    ve_fontcache_drawlist dl = {};
    vxui_gl_state gl = {};
    vxui_draw_cmd c = rectangle( 0 );
    vxui_gl_emit_one_rect( &dl, &gl, &c, 10, 15, 100, 100, {} );

    ASSERT_EQ( dl.vertices.size(), (size_t) 4 );
    ASSERT_EQ( dl.indices.size(), (size_t) 6 );
    ASSERT_EQ( dl.dcalls.size(), (size_t) 1 );
    ASSERT_EQ( dl.dcalls[0].pass, DEMO_GL_PASS_RECT );
    ASSERT_EQ( dl.dcalls[0].region, (uint32_t) 0 );
    ASSERT_EQ( gl.material_data_count, 0 );
    for ( int i = 0; i < 4; i++ )
    {
        ASSERT_EQ( dl.vertices[i].u, 0.0f );
        ASSERT_EQ( dl.vertices[i].v, 0.0f );
        ASSERT_EQ( dl.dcalls[0].colour[i], c.render.colour[i] );
    }
}

UTEST(demo_quad_geometry, crt_quad_has_unit_uvs_and_unmodified_material_bytes) {
    ve_fontcache_drawlist dl = {};
    vxui_gl_state gl = {};
    vxui_draw_cmd c = rectangle( DEMO_MATERIAL_CRT );
    c.render.flags = 0x1234;
    for ( int i = 0; i < 8; i++ ) c.render.params[i] = (float) i * 0.25f;
    vxui_gl_emit_one_rect( &dl, &gl, &c, 10, 15, 100, 100, {} );

    ASSERT_EQ( dl.dcalls.size(), (size_t) 1 );
    ASSERT_EQ( dl.dcalls[0].pass, DEMO_GL_PASS_CRT );
    ASSERT_EQ( dl.dcalls[0].region, (uint32_t) 0 );
    ASSERT_EQ( gl.material_data_count, 1 );
    ASSERT_EQ( gl.material_data[0].flags, c.render.flags );
    for ( int i = 0; i < 8; i++ ) ASSERT_EQ( gl.material_data[0].params[i], c.render.params[i] );
    float u[] = { 0, 1, 1, 0 };
    float v[] = { 0, 0, 1, 1 };
    for ( int i = 0; i < 4; i++ )
    {
        ASSERT_EQ( dl.vertices[i].u, u[i] );
        ASSERT_EQ( dl.vertices[i].v, v[i] );
    }
}

UTEST(demo_quad_geometry, round_quad_keeps_scaled_dimensions_radius_and_softness) {
    ve_fontcache_drawlist dl = {};
    vxui_gl_state gl = {};
    vxui_draw_cmd c = rectangle( DEMO_MATERIAL_ROUND );
    c.render.params[0] = 6;
    c.render.params[1] = 0.5f;
    vxui_gl_xform xf = {};
    xf.scale = 1.125f;
    vxui_gl_emit_one_rect( &dl, &gl, &c, 10, 15, 100, 100, xf );

    ASSERT_EQ( dl.dcalls[0].pass, DEMO_GL_PASS_ROUND );
    ASSERT_EQ( gl.material_data[0].params[0], 6.75f );
    ASSERT_EQ( gl.material_data[0].params[1], 45.0f );
    ASSERT_EQ( gl.material_data[0].params[2], 22.5f );
    ASSERT_EQ( gl.material_data[0].params[3], 0.5625f );
    for ( int i = 4; i < 8; i++ ) ASSERT_EQ( gl.material_data[0].params[i], 0.0f );
    ASSERT_EQ( dl.vertices[2].u, 1.0f );
    ASSERT_EQ( dl.vertices[2].v, 1.0f );
    ASSERT_NEAR( dl.vertices[2].x, 0.55f, 1e-6f );
    ASSERT_NEAR( dl.vertices[2].y, 0.625f, 1e-6f );
}

UTEST(demo_quad_geometry, outline_uses_a_second_quad_and_one_material_slot) {
    ve_fontcache_drawlist dl = {};
    vxui_gl_state gl = {};
    vxui_draw_cmd c = rectangle( 0 );
    c.render.outline_thickness = 2;
    c.render.outline_colour = { 0.6f, 0.7f, 0.8f, 0.9f };
    vxui_gl_emit_one_rect( &dl, &gl, &c, 10, 15, 100, 100, {} );

    ASSERT_EQ( dl.vertices.size(), (size_t) 8 );
    ASSERT_EQ( dl.indices.size(), (size_t) 12 );
    ASSERT_EQ( dl.dcalls[0].pass, DEMO_GL_PASS_RECT );
    ASSERT_EQ( dl.dcalls[1].pass, DEMO_GL_PASS_OUTLINE );
    ASSERT_EQ( dl.dcalls[1].region, (uint32_t) 0 );
    ASSERT_EQ( gl.material_data_count, 1 );
    ASSERT_EQ( gl.material_data[0].params[0], 2.0f );
    ASSERT_EQ( gl.material_data[0].params[1], 40.0f );
    ASSERT_EQ( gl.material_data[0].params[2], 20.0f );
    ASSERT_EQ( gl.material_data[0].flags, (uint32_t) 0 );
    for ( int i = 0; i < 4; i++ )
    {
        ASSERT_EQ( dl.vertices[i].x, dl.vertices[i + 4].x );
        ASSERT_EQ( dl.vertices[i].y, dl.vertices[i + 4].y );
        ASSERT_EQ( dl.dcalls[1].colour[i], c.render.outline_colour[i] );
    }
}

UTEST(demo_quad_geometry, mixed_passes_share_indices_but_keep_uv_and_region_semantics) {
    ve_fontcache_drawlist dl = {};
    vxui_gl_state gl = {};
    uint32_t materials[] = { 0, DEMO_MATERIAL_CRT, DEMO_MATERIAL_ROUND, DEMO_MATERIAL_IMAGE };
    uint32_t passes[] = { DEMO_GL_PASS_RECT, DEMO_GL_PASS_CRT, DEMO_GL_PASS_ROUND, DEMO_GL_PASS_IMAGE };
    uint32_t regions[] = { 0, 0, 1, 42 };
    for ( int i = 0; i < 4; i++ )
    {
        vxui_draw_cmd c = rectangle( materials[i] );
        if ( i == 3 )
        {
            c.render.texture_id = 42;
            c.render.uv = { 0.8f, 0.9f, 0.1f, 0.2f };
        }
        vxui_gl_emit_one_rect( &dl, &gl, &c, 10, 15, 100, 100, {} );
    }

    ASSERT_EQ( dl.dcalls.size(), (size_t) 4 );
    ASSERT_EQ( gl.material_data_count, 2 );
    uint32_t indices[] = { 0, 1, 2, 0, 2, 3 };
    for ( int i = 0; i < 4; i++ )
    {
        ASSERT_EQ( dl.dcalls[i].pass, passes[i] );
        ASSERT_EQ( dl.dcalls[i].region, regions[i] );
        ASSERT_EQ( dl.dcalls[i].start_index, (uint32_t) i * 6 );
        ASSERT_EQ( dl.dcalls[i].end_index, (uint32_t) ( i + 1 ) * 6 );
        for ( int j = 0; j < 6; j++ ) ASSERT_EQ( dl.indices[i * 6 + j], indices[j] + (uint32_t) i * 4 );
        ASSERT_NEAR( dl.vertices[i * 4].x, 0.1f, 1e-6f );
        ASSERT_NEAR( dl.vertices[i * 4 + 2].y, 0.65f, 1e-6f );
    }
    ASSERT_EQ( dl.vertices[0].u, 0.0f );
    ASSERT_EQ( dl.vertices[6].u, 1.0f );
    ASSERT_EQ( dl.vertices[10].v, 1.0f );
    ASSERT_EQ( dl.vertices[12].u, 0.8f );
    ASSERT_EQ( dl.vertices[14].v, 0.2f );
}

UTEST(demo_quad_geometry, clip_markers_do_not_shift_quad_index_offsets) {
    ve_fontcache_drawlist dl = {};
    vxui_gl_state gl = {};
    vxui_gl_push_scissor_set( &dl, &gl, { 0, 0, 0.5f, 1 } );
    vxui_draw_cmd c = rectangle( DEMO_MATERIAL_CRT );
    vxui_gl_emit_one_rect( &dl, &gl, &c, 10, 15, 100, 100, {} );
    vxui_gl_push_scissor_reset( &dl );
    c.render.material_id = 0;
    vxui_gl_emit_one_rect( &dl, &gl, &c, 10, 15, 100, 100, {} );

    ASSERT_EQ( dl.dcalls.size(), (size_t) 4 );
    ASSERT_EQ( dl.vertices.size(), (size_t) 8 );
    ASSERT_EQ( dl.dcalls[0].start_index, dl.dcalls[0].end_index );
    ASSERT_EQ( dl.dcalls[1].start_index, (uint32_t) 0 );
    ASSERT_EQ( dl.dcalls[1].end_index, (uint32_t) 6 );
    ASSERT_EQ( dl.dcalls[2].start_index, (uint32_t) 6 );
    ASSERT_EQ( dl.dcalls[2].end_index, (uint32_t) 6 );
    ASSERT_EQ( dl.dcalls[3].start_index, (uint32_t) 6 );
    ASSERT_EQ( dl.dcalls[3].end_index, (uint32_t) 12 );
}

UTEST(demo_quad_geometry, optimiser_does_not_merge_distinct_material_slots) {
    ve_fontcache cache = {};
    vxui_gl_state gl = {};
    vxui_draw_cmd c = rectangle( DEMO_MATERIAL_CRT );
    vxui_gl_emit_one_rect( &cache.drawlist, &gl, &c, 10, 15, 100, 100, {} );
    c.render.params[0] = 1;
    vxui_gl_emit_one_rect( &cache.drawlist, &gl, &c, 10, 15, 100, 100, {} );
    ve_fontcache_optimise_drawlist( &cache );

    ASSERT_EQ( cache.drawlist.dcalls.size(), (size_t) 2 );
    ASSERT_EQ( cache.drawlist.dcalls[0].region, (uint32_t) 0 );
    ASSERT_EQ( cache.drawlist.dcalls[1].region, (uint32_t) 1 );
    ASSERT_EQ( gl.material_data[0].params[0], 0.0f );
    ASSERT_EQ( gl.material_data[1].params[0], 1.0f );
}

UTEST(demo_quad_geometry, full_packet_batch_keeps_every_fill_and_outline_quad) {
    ve_fontcache_drawlist dl = {};
    vxui_gl_state gl = {};
    vxui_draw_cmd c = rectangle( DEMO_MATERIAL_CRT );
    c.render.outline_thickness = 2;
    for ( int i = 0; i < VXUI_MAX_DRAW_CMDS; i++ ) vxui_gl_emit_one_rect( &dl, &gl, &c, 10, 15, 100, 100, {} );

    int quads = VXUI_MAX_DRAW_CMDS * 2;
    ASSERT_EQ( gl.material_data_count, quads );
    ASSERT_EQ( dl.vertices.size(), (size_t) quads * 4 );
    ASSERT_EQ( dl.indices.size(), (size_t) quads * 6 );
    ASSERT_EQ( dl.dcalls.size(), (size_t) quads );
    uint32_t indices[] = { 0, 1, 2, 0, 2, 3 };
    for ( int i = 0; i < quads; i++ )
    {
        ASSERT_EQ( dl.dcalls[i].region, (uint32_t) i );
        ASSERT_EQ( dl.dcalls[i].pass, i & 1 ? DEMO_GL_PASS_OUTLINE : DEMO_GL_PASS_CRT );
        for ( int j = 0; j < 6; j++ ) ASSERT_EQ( dl.indices[i * 6 + j], indices[j] + (uint32_t) i * 4 );
        ASSERT_EQ( dl.vertices[i * 4 + 2].u, 1.0f );
        ASSERT_EQ( dl.vertices[i * 4 + 2].v, 1.0f );
    }
}

UTEST_MAIN();
