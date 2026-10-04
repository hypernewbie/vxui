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
#include <filesystem>
#define main vxui_demo_main
#include "../demo/main.cpp"
#undef main

static constexpr int s_packet_limit = VXUI_MAX_DRAW_CMDS;
#undef VXUI_MAX_DRAW_CMDS
#define VXUI_MAX_DRAW_CMDS 1024
static constexpr int s_scissor_capacity_1024 = DEMO_GL_MAX_SCISSOR_DCALLS;
#undef VXUI_MAX_DRAW_CMDS
#define VXUI_MAX_DRAW_CMDS s_packet_limit

static std::vector< uint8_t > roboto_bytes()
{
    std::filesystem::path p = std::filesystem::path( __FILE__ ).parent_path().parent_path() / "fonts" / "Roboto-Regular.ttf";
    FILE* f = fopen( p.string().c_str(), "rb" );
    assert( f );
    fseek( f, 0, SEEK_END );
    long size = ftell( f );
    fseek( f, 0, SEEK_SET );
    std::vector< uint8_t > bytes( (size_t) size );
    fread( bytes.data(), 1, bytes.size(), f );
    fclose( f );
    return bytes;
}

static void fill_commands( vxui_draw_cmd* cmds, bool text = false )
{
    for ( int i = 0; i < VXUI_MAX_DRAW_CMDS; i++ )
    {
        vxui_draw_cmd& c = cmds[i];
        c = {};
        c.type          = text ? VXUI_DRAW_TEXT : VXUI_DRAW_RECT;
        c.rect          = { 0, 0, 16, 24 };
        c.clipped       = true;
        c.clip          = { (float) i, 0, 16, 24 };
        c.text          = text ? "A" : nullptr;
        c.text_len      = text ? 1 : 0;
        c.font_px       = 24;
        c.render.colour = { 1, 1, 1, 1 };
    }
}

static int marker_count( const ve_fontcache_drawlist& dl, uint32_t pass )
{
    int count = 0;
    for ( const ve_fontcache_draw& d : dl.dcalls ) if ( d.pass == pass ) count++;
    return count;
}

UTEST(demo_scissor_capacity, state_has_one_clip_slot_per_packet) {
    vxui_gl_state gl = {};
    ASSERT_EQ( DEMO_GL_MAX_SCISSOR_DCALLS, VXUI_MAX_DRAW_CMDS );
    ASSERT_EQ( sizeof( gl.scissor_data ) / sizeof( gl.scissor_data[0] ), (size_t) VXUI_MAX_DRAW_CMDS );
}

UTEST(demo_scissor_capacity, capacity_tracks_a_changed_packet_limit) {
    ASSERT_EQ( s_scissor_capacity_1024, 1024 );
}

UTEST(demo_scissor_capacity, every_clipped_rectangle_has_one_slot) {
    vxui_draw_cmd cmds[VXUI_MAX_DRAW_CMDS] = {};
    fill_commands( cmds );
    vxui_draw_list dl = { cmds, VXUI_MAX_DRAW_CMDS };
    vxui_gl_state gl = {};
    ve_fontcache cache = {};
    vxui_gl_emit( &cache.drawlist, &gl, &cache, -1, dl, 400, 400, {} );

    ASSERT_EQ( gl.scissor_data_count, VXUI_MAX_DRAW_CMDS );
    ASSERT_EQ( cache.drawlist.dcalls.size(), (size_t) VXUI_MAX_DRAW_CMDS * 3 );
    for ( int i = 0; i < VXUI_MAX_DRAW_CMDS; i++ )
    {
        const ve_fontcache_draw& marker = cache.drawlist.dcalls[i * 3];
        ASSERT_EQ( marker.pass, DEMO_GL_PASS_SCISSOR_SET );
        ASSERT_EQ( marker.region, (uint32_t) i );
        ASSERT_EQ( marker.start_index, marker.end_index );
        ASSERT_EQ( gl.scissor_data[i].x, (float) i / 400.0f );
        ASSERT_EQ( cache.drawlist.dcalls[i * 3 + 2].pass, DEMO_GL_PASS_SCISSOR_RESET );
    }
}

UTEST(demo_scissor_capacity, outlines_and_markers_share_the_rectangle_clip_slot) {
    vxui_draw_cmd cmds[VXUI_MAX_DRAW_CMDS] = {};
    fill_commands( cmds );
    for ( int i = 0; i < VXUI_MAX_DRAW_CMDS; i++ )
    {
        cmds[i].render.material_id       = DEMO_MATERIAL_ROUND;
        cmds[i].render.outline_thickness = 2;
        cmds[i].render.texture_id        = 42;
    }
    vxui_draw_list dl = { cmds, VXUI_MAX_DRAW_CMDS };
    vxui_gl_state gl = {};
    ve_fontcache cache = {};
    vxui_gl_emit( &cache.drawlist, &gl, &cache, -1, dl, 400, 400, {} );

    ASSERT_EQ( gl.scissor_data_count, VXUI_MAX_DRAW_CMDS );
    ASSERT_EQ( cache.drawlist.dcalls.size(), (size_t) VXUI_MAX_DRAW_CMDS * 5 );
    for ( int i = 0; i < VXUI_MAX_DRAW_CMDS; i++ )
    {
        ASSERT_EQ( cache.drawlist.dcalls[i * 5].pass, DEMO_GL_PASS_SCISSOR_SET );
        ASSERT_EQ( cache.drawlist.dcalls[i * 5 + 1].pass, DEMO_GL_PASS_ROUND );
        ASSERT_EQ( cache.drawlist.dcalls[i * 5 + 2].pass, DEMO_GL_PASS_OUTLINE );
        ASSERT_EQ( cache.drawlist.dcalls[i * 5 + 3].pass, DEMO_GL_PASS_RECT_TEXTURED );
        ASSERT_EQ( cache.drawlist.dcalls[i * 5 + 4].pass, DEMO_GL_PASS_SCISSOR_RESET );
    }
}

UTEST(demo_scissor_capacity, every_clipped_text_packet_has_one_slot) {
    std::vector< uint8_t > bytes = roboto_bytes();
    ve_fontcache cache = {};
    ve_fontcache_init( &cache, false );
    ve_font_id font = ve_fontcache_load( &cache, bytes.data(), bytes.size(), 24 );
    ASSERT_GE( font, 0 );
    vxui_draw_cmd cmds[VXUI_MAX_DRAW_CMDS] = {};
    fill_commands( cmds, true );
    vxui_draw_list dl = { cmds, VXUI_MAX_DRAW_CMDS };
    vxui_gl_state gl = {};
    vxui_gl_emit( &cache.drawlist, &gl, &cache, font, dl, 400, 400, {} );

    ASSERT_EQ( gl.scissor_data_count, VXUI_MAX_DRAW_CMDS );
    ASSERT_EQ( marker_count( cache.drawlist, DEMO_GL_PASS_SCISSOR_SET ), VXUI_MAX_DRAW_CMDS );
    ASSERT_EQ( marker_count( cache.drawlist, DEMO_GL_PASS_SCISSOR_RESET ), VXUI_MAX_DRAW_CMDS );
    int slot = 0;
    for ( const ve_fontcache_draw& d : cache.drawlist.dcalls )
    {
        if ( d.pass != DEMO_GL_PASS_SCISSOR_SET ) continue;
        ASSERT_EQ( d.region, (uint32_t) slot++ );
        ASSERT_EQ( d.start_index, d.end_index );
    }
    ve_fontcache_shutdown( &cache );
}

UTEST(demo_scissor_capacity, mixed_text_and_rectangles_share_the_packet_bound) {
    std::vector< uint8_t > bytes = roboto_bytes();
    ve_fontcache cache = {};
    ve_fontcache_init( &cache, false );
    ve_font_id font = ve_fontcache_load( &cache, bytes.data(), bytes.size(), 24 );
    ASSERT_GE( font, 0 );
    vxui_draw_cmd cmds[VXUI_MAX_DRAW_CMDS] = {};
    fill_commands( cmds, true );
    for ( int i = 0; i < VXUI_MAX_DRAW_CMDS; i += 2 ) cmds[i].type = VXUI_DRAW_RECT;
    vxui_draw_list dl = { cmds, VXUI_MAX_DRAW_CMDS };
    vxui_gl_state gl = {};
    vxui_gl_emit( &cache.drawlist, &gl, &cache, font, dl, 400, 400, {} );

    ASSERT_EQ( gl.scissor_data_count, VXUI_MAX_DRAW_CMDS );
    ASSERT_EQ( marker_count( cache.drawlist, DEMO_GL_PASS_SCISSOR_SET ), VXUI_MAX_DRAW_CMDS );
    ASSERT_EQ( marker_count( cache.drawlist, DEMO_GL_PASS_SCISSOR_RESET ), VXUI_MAX_DRAW_CMDS );
    ve_fontcache_shutdown( &cache );
}

UTEST(demo_scissor_capacity, unclipped_packets_do_not_consume_slots) {
    vxui_draw_cmd cmds[VXUI_MAX_DRAW_CMDS] = {};
    fill_commands( cmds );
    for ( int i = 0; i < VXUI_MAX_DRAW_CMDS; i += 2 ) cmds[i].clipped = false;
    vxui_draw_list dl = { cmds, VXUI_MAX_DRAW_CMDS };
    vxui_gl_state gl = {};
    ve_fontcache cache = {};
    vxui_gl_emit( &cache.drawlist, &gl, &cache, -1, dl, 400, 400, {} );

    ASSERT_EQ( gl.scissor_data_count, VXUI_MAX_DRAW_CMDS / 2 );
    ASSERT_EQ( marker_count( cache.drawlist, DEMO_GL_PASS_SCISSOR_SET ), VXUI_MAX_DRAW_CMDS / 2 );
    ASSERT_EQ( marker_count( cache.drawlist, DEMO_GL_PASS_SCISSOR_RESET ), VXUI_MAX_DRAW_CMDS / 2 );
}

UTEST(demo_scissor_capacity, skipped_text_does_not_consume_slots) {
    vxui_draw_cmd cmds[VXUI_MAX_DRAW_CMDS] = {};
    fill_commands( cmds, true );
    vxui_draw_list dl = { cmds, VXUI_MAX_DRAW_CMDS };
    vxui_gl_state gl = {};
    ve_fontcache cache = {};
    vxui_gl_emit( &cache.drawlist, &gl, &cache, -1, dl, 400, 400, {} );

    ASSERT_EQ( gl.scissor_data_count, 0 );
    ASSERT_TRUE( cache.drawlist.dcalls.empty() );
}

UTEST(demo_scissor_capacity, slot_zero_is_reused_after_flush) {
    vxui_draw_cmd cmds[VXUI_MAX_DRAW_CMDS] = {};
    fill_commands( cmds );
    vxui_draw_list dl = { cmds, VXUI_MAX_DRAW_CMDS };
    vxui_gl_state gl = {};
    ve_fontcache cache = {};
    vxui_gl_emit( &cache.drawlist, &gl, &cache, -1, dl, 400, 400, {} );
    ve_fontcache_flush_drawlist( &cache );
    gl.scissor_data_count = 0;
    cmds[0].clip.x = 20;
    vxui_draw_list next = { cmds, 1 };
    vxui_gl_emit( &cache.drawlist, &gl, &cache, -1, next, 400, 400, {} );

    ASSERT_EQ( gl.scissor_data_count, 1 );
    ASSERT_EQ( cache.drawlist.dcalls[0].region, (uint32_t) 0 );
    ASSERT_EQ( gl.scissor_data[0].x, 0.05f );
}

UTEST_MAIN();
