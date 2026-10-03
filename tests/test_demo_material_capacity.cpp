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

static void fill_rects( vxui_draw_cmd* cmds, uint32_t material, bool outline, bool clipped = false )
{
    for ( int i = 0; i < VXUI_MAX_DRAW_CMDS; i++ )
    {
        vxui_draw_cmd& c = cmds[i];
        c = {};
        c.type                     = VXUI_DRAW_RECT;
        c.rect                     = { 0, 0, 16, 8 };
        c.clipped                  = clipped;
        c.clip                     = { 0, 0, 16, 8 };
        c.render.material_id       = material;
        c.render.colour            = { 1, 1, 1, 1 };
        c.render.texture_id        = material == DEMO_MATERIAL_IMAGE ? 42 : 0;
        c.render.uv                = { 0, 0, 1, 1 };
        c.render.outline_thickness = outline ? 2.0f : 0.0f;
        c.render.outline_colour    = { 1, 0, 0, 1 };
        c.render.flags             = (uint32_t) i;
        for ( int j = 0; j < 8; j++ ) c.render.params[j] = (float) ( i * 8 + j );
    }
}

UTEST(demo_material_capacity, capacity_covers_a_material_and_outline_per_packet) {
    ASSERT_GE( DEMO_GL_MAX_MATERIAL_DCALLS, VXUI_MAX_DRAW_CMDS * 2 );
}

UTEST(demo_material_capacity, every_rounded_packet_and_outline_has_a_slot) {
    ASSERT_GE( DEMO_GL_MAX_MATERIAL_DCALLS, VXUI_MAX_DRAW_CMDS * 2 );
    vxui_draw_cmd cmds[VXUI_MAX_DRAW_CMDS] = {};
    fill_rects( cmds, DEMO_MATERIAL_ROUND, true );
    vxui_draw_list dl = { cmds, VXUI_MAX_DRAW_CMDS };
    vxui_gl_state gl = {};
    ve_fontcache cache = {};
    vxui_gl_emit( &cache.drawlist, &gl, &cache, -1, dl, 200, 200, {} );

    ASSERT_EQ( gl.material_data_count, VXUI_MAX_DRAW_CMDS * 2 );
    ASSERT_EQ( cache.drawlist.dcalls.size(), (size_t) VXUI_MAX_DRAW_CMDS * 2 );
    ASSERT_EQ( cache.drawlist.indices.size(), (size_t) VXUI_MAX_DRAW_CMDS * 12 );
    for ( int i = 0; i < VXUI_MAX_DRAW_CMDS; i++ )
    {
        ASSERT_EQ( cache.drawlist.dcalls[i * 2].pass, DEMO_GL_PASS_ROUND );
        ASSERT_EQ( cache.drawlist.dcalls[i * 2 + 1].pass, DEMO_GL_PASS_OUTLINE );
        ASSERT_EQ( cache.drawlist.dcalls[i * 2].region, (uint32_t) ( i * 2 ) );
        ASSERT_EQ( cache.drawlist.dcalls[i * 2 + 1].region, (uint32_t) ( i * 2 + 1 ) );
        ASSERT_EQ( gl.material_data[i * 2].params[0], cmds[i].render.params[0] );
        ASSERT_EQ( gl.material_data[i * 2].params[1], 16.0f );
        ASSERT_EQ( gl.material_data[i * 2].params[2], 8.0f );
        ASSERT_EQ( gl.material_data[i * 2 + 1].params[0], 2.0f );
    }
}

UTEST(demo_material_capacity, every_crt_packet_keeps_all_parameters_and_flags) {
    ASSERT_GE( DEMO_GL_MAX_MATERIAL_DCALLS, VXUI_MAX_DRAW_CMDS * 2 );
    vxui_draw_cmd cmds[VXUI_MAX_DRAW_CMDS] = {};
    fill_rects( cmds, DEMO_MATERIAL_CRT, true );
    vxui_draw_list dl = { cmds, VXUI_MAX_DRAW_CMDS };
    vxui_gl_state gl = {};
    ve_fontcache cache = {};
    vxui_gl_emit( &cache.drawlist, &gl, &cache, -1, dl, 200, 200, {} );

    ASSERT_EQ( gl.material_data_count, VXUI_MAX_DRAW_CMDS * 2 );
    for ( int i = 0; i < VXUI_MAX_DRAW_CMDS; i++ )
    {
        ASSERT_EQ( cache.drawlist.dcalls[i * 2].pass, DEMO_GL_PASS_CRT );
        ASSERT_EQ( gl.material_data[i * 2].flags, cmds[i].render.flags );
        for ( int j = 0; j < 8; j++ ) ASSERT_EQ( gl.material_data[i * 2].params[j], cmds[i].render.params[j] );
    }
}

UTEST(demo_material_capacity, solid_and_image_packets_only_use_outline_slots) {
    ASSERT_GE( DEMO_GL_MAX_MATERIAL_DCALLS, VXUI_MAX_DRAW_CMDS * 2 );
    uint32_t materials[] = { 0, DEMO_MATERIAL_IMAGE };
    uint32_t passes[] = { DEMO_GL_PASS_RECT, DEMO_GL_PASS_IMAGE };
    for ( int k = 0; k < 2; k++ )
    {
        vxui_draw_cmd cmds[VXUI_MAX_DRAW_CMDS] = {};
        fill_rects( cmds, materials[k], true );
        vxui_draw_list dl = { cmds, VXUI_MAX_DRAW_CMDS };
        vxui_gl_state gl = {};
        ve_fontcache cache = {};
        vxui_gl_emit( &cache.drawlist, &gl, &cache, -1, dl, 200, 200, {} );

        ASSERT_EQ( gl.material_data_count, VXUI_MAX_DRAW_CMDS );
        ASSERT_EQ( cache.drawlist.dcalls.size(), (size_t) VXUI_MAX_DRAW_CMDS * 2 );
        for ( int i = 0; i < VXUI_MAX_DRAW_CMDS; i++ )
        {
            ASSERT_EQ( cache.drawlist.dcalls[i * 2].pass, passes[k] );
            ASSERT_EQ( cache.drawlist.dcalls[i * 2 + 1].pass, DEMO_GL_PASS_OUTLINE );
            ASSERT_EQ( cache.drawlist.dcalls[i * 2 + 1].region, (uint32_t) i );
        }
    }
}

UTEST(demo_material_capacity, clips_do_not_consume_material_slots) {
    ASSERT_GE( DEMO_GL_MAX_MATERIAL_DCALLS, VXUI_MAX_DRAW_CMDS * 2 );
    vxui_draw_cmd cmds[VXUI_MAX_DRAW_CMDS] = {};
    fill_rects( cmds, DEMO_MATERIAL_ROUND, true, true );
    vxui_draw_list dl = { cmds, VXUI_MAX_DRAW_CMDS };
    vxui_gl_state gl = {};
    ve_fontcache cache = {};
    vxui_gl_emit( &cache.drawlist, &gl, &cache, -1, dl, 200, 200, {} );

    ASSERT_EQ( gl.material_data_count, VXUI_MAX_DRAW_CMDS * 2 );
    ASSERT_EQ( gl.scissor_data_count, VXUI_MAX_DRAW_CMDS );
    ASSERT_EQ( cache.drawlist.dcalls.size(), (size_t) VXUI_MAX_DRAW_CMDS * 4 );
    for ( int i = 0; i < VXUI_MAX_DRAW_CMDS; i++ )
    {
        ASSERT_EQ( cache.drawlist.dcalls[i * 4].pass, DEMO_GL_PASS_SCISSOR_SET );
        ASSERT_EQ( cache.drawlist.dcalls[i * 4 + 1].pass, DEMO_GL_PASS_ROUND );
        ASSERT_EQ( cache.drawlist.dcalls[i * 4 + 2].pass, DEMO_GL_PASS_OUTLINE );
        ASSERT_EQ( cache.drawlist.dcalls[i * 4 + 3].pass, DEMO_GL_PASS_SCISSOR_RESET );
    }
}

UTEST(demo_material_capacity, slots_can_be_reused_after_execution_flush) {
    ASSERT_GE( DEMO_GL_MAX_MATERIAL_DCALLS, VXUI_MAX_DRAW_CMDS * 2 );
    vxui_draw_cmd cmds[VXUI_MAX_DRAW_CMDS] = {};
    fill_rects( cmds, DEMO_MATERIAL_ROUND, true );
    vxui_draw_list dl = { cmds, VXUI_MAX_DRAW_CMDS };
    vxui_gl_state gl = {};
    ve_fontcache cache = {};
    vxui_gl_emit( &cache.drawlist, &gl, &cache, -1, dl, 200, 200, {} );

    ve_fontcache_flush_drawlist( &cache );
    gl.material_data_count = 0;
    gl.scissor_data_count  = 0;
    cmds[0].render.params[0] = 7;
    vxui_draw_list next = { cmds, 1 };
    vxui_gl_emit( &cache.drawlist, &gl, &cache, -1, next, 200, 200, {} );
    ASSERT_EQ( gl.material_data_count, 2 );
    ASSERT_EQ( cache.drawlist.dcalls.size(), (size_t) 2 );
    ASSERT_EQ( cache.drawlist.dcalls[0].region, (uint32_t) 0 );
    ASSERT_EQ( cache.drawlist.dcalls[1].region, (uint32_t) 1 );
    ASSERT_EQ( gl.material_data[0].params[0], 7.0f );
}

UTEST_MAIN();
