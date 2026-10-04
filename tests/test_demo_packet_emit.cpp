#include "demo_gl_record.h"

static std::vector< uint8_t > roboto_bytes()
{
    std::filesystem::path path = std::filesystem::path( __FILE__ ).parent_path().parent_path() / "fonts" / "Roboto-Regular.ttf";
    return load_file( path.string().c_str() );
}

static vxui_draw_cmd packet( bool text, bool clipped = true )
{
    vxui_draw_cmd c  = {};
    c.type          = text ? VXUI_DRAW_TEXT : VXUI_DRAW_RECT;
    c.rect          = { 8, 16, 40, 24 };
    c.clipped       = clipped;
    c.clip          = { 0, 0, 80, 80 };
    c.text          = text ? "A" : nullptr;
    c.text_len      = text ? 1 : 0;
    c.font_px       = 24;
    c.render.colour = { 0.2f, 0.4f, 0.6f, 0.8f };
    return c;
}

UTEST(demo_packet_emit, mixed_packets_have_one_clip_pair_each_in_producer_order) {
    std::vector< uint8_t > bytes = roboto_bytes();
    ve_fontcache cache = {};
    ve_fontcache_init( &cache, false );
    ve_font_id font = ve_fontcache_load( &cache, bytes.data(), bytes.size(), 24 );
    ASSERT_GE( font, 0 );
    vxui_gl_state gl = {};
    vxui_draw_cmd c[] = { packet( false ), packet( true ), packet( false ) };
    c[2].render.material_id = DEMO_MATERIAL_IMAGE;
    c[2].render.texture_id = 42;
    vxui_draw_cmd before[3];
    memcpy( before, c, sizeof( c ) );
    vxui_gl_emit( &cache.drawlist, &gl, &cache, font, { c, 3 }, 640, 480, { { 10, 20 }, 1.125f } );

    ASSERT_EQ( gl.scissor_data_count, 3 );
    int active = -1;
    int sets = 0;
    int resets = 0;
    int targets = 0;
    for ( const ve_fontcache_draw& d : cache.drawlist.dcalls )
    {
        if ( d.pass == DEMO_GL_PASS_SCISSOR_SET )
        {
            ASSERT_EQ( active, -1 );
            ASSERT_EQ( d.region, (uint32_t) sets );
            active = sets++;
            ASSERT_EQ( d.start_index, d.end_index );
        }
        else if ( d.pass == DEMO_GL_PASS_SCISSOR_RESET )
        {
            ASSERT_GE( active, 0 );
            active = -1;
            resets++;
        }
        else if ( d.pass == DEMO_GL_PASS_RECT ) ASSERT_EQ( active, 0 );
        else if ( d.pass == DEMO_GL_PASS_IMAGE )
        {
            ASSERT_EQ( active, 2 );
            ASSERT_EQ( d.region, (uint32_t) 42 );
        }
        else if ( d.pass == VE_FONTCACHE_FRAMEBUFFER_PASS_TARGET )
        {
            ASSERT_EQ( active, 1 );
            targets++;
        }
    }
    ASSERT_EQ( sets, 3 );
    ASSERT_EQ( resets, 3 );
    ASSERT_GT( targets, 0 );
    ASSERT_EQ( memcmp( before, c, sizeof( c ) ), 0 );
    ve_fontcache_shutdown( &cache );
}

UTEST(demo_packet_emit, only_focused_rectangles_apply_spring_displacement) {
    std::vector< uint8_t > bytes = roboto_bytes();
    ve_fontcache cache = {};
    ve_fontcache_init( &cache, false );
    ve_font_id font = ve_fontcache_load( &cache, bytes.data(), bytes.size(), 24 );
    ASSERT_GE( font, 0 );
    vxui_gl_state gl = {};
    vxui_draw_cmd c[] = { packet( false ), packet( true ), packet( false, false ) };
    for ( int i = 0; i < 3; i++ ) c[i].focus_offset_y = 8;
    c[0].state = VXUI_DRAW_FOCUSED;
    c[1].state = VXUI_DRAW_FOCUSED;
    vxui_gl_emit( &cache.drawlist, &gl, &cache, font, { c, 3 }, 640, 480, { { 10, 20 }, 1.125f } );

    ASSERT_NEAR( cache.drawlist.vertices[0].x, 19.0f / 640.0f, 1e-6f );
    ASSERT_NEAR( cache.drawlist.vertices[0].y, 1.0f - 47.0f / 480.0f, 1e-6f );
    float baseline = 1.0f - 65.0f / 480.0f;
    ASSERT_NEAR( ve_fontcache_get_cursor_pos( &cache ).y, baseline, 1e-6f );
    const ve_fontcache_draw& last = cache.drawlist.dcalls.back();
    ASSERT_EQ( last.pass, DEMO_GL_PASS_RECT );
    uint32_t vertex = cache.drawlist.indices[last.start_index];
    ASSERT_NEAR( cache.drawlist.vertices[vertex].y, 1.0f - 38.0f / 480.0f, 1e-6f );
    ASSERT_EQ( gl.scissor_data_count, 2 );
    ASSERT_NEAR( gl.scissor_data[0].x, 10.0f / 640.0f, 1e-6f );
    ASSERT_NEAR( gl.scissor_data[0].y, 1.0f - 110.0f / 480.0f, 1e-6f );
    ve_fontcache_shutdown( &cache );
}

UTEST(demo_packet_emit, skipped_text_does_not_allocate_markers_before_a_fontless_rectangle) {
    vxui_gl_state gl = {};
    ve_fontcache cache = {};
    vxui_draw_cmd c[] = { packet( true ), packet( true ), packet( false ) };
    c[1].text_len = 0;
    c[2].font = VXUI_FONT_INVALID;
    c[2].text_len = -1;
    vxui_gl_emit( &cache.drawlist, &gl, &cache, -1, { c, 3 }, 640, 480, {} );
    ASSERT_EQ( gl.scissor_data_count, 1 );
    ASSERT_EQ( cache.drawlist.dcalls.size(), (size_t) 3 );
    ASSERT_EQ( cache.drawlist.dcalls[0].pass, DEMO_GL_PASS_SCISSOR_SET );
    ASSERT_EQ( cache.drawlist.dcalls[1].pass, DEMO_GL_PASS_RECT );
    ASSERT_EQ( cache.drawlist.dcalls[2].pass, DEMO_GL_PASS_SCISSOR_RESET );
}

UTEST(demo_packet_emit, empty_text_and_missing_default_font_remain_skipped_with_loaded_entries) {
    std::vector< uint8_t > bytes = roboto_bytes();
    ve_fontcache cache = {};
    ve_fontcache_init( &cache, false );
    ASSERT_GE( ve_fontcache_load( &cache, bytes.data(), bytes.size(), 24 ), 0 );
    vxui_gl_state gl = {};
    vxui_draw_cmd c[] = { packet( true ), packet( true ), packet( false, false ) };
    c[1].text_len = 0;
    vxui_gl_emit( &cache.drawlist, &gl, &cache, -1, { c, 3 }, 640, 480, {} );
    ASSERT_EQ( gl.scissor_data_count, 0 );
    ASSERT_EQ( cache.drawlist.dcalls.size(), (size_t) 1 );
    ASSERT_EQ( cache.drawlist.dcalls[0].pass, DEMO_GL_PASS_RECT );
    ve_fontcache_shutdown( &cache );
}

UTEST(demo_packet_emit, default_and_explicit_fonts_keep_exact_scaled_size_and_text_colour) {
    std::vector< uint8_t > bytes = roboto_bytes();
    ve_fontcache cache = {};
    ve_fontcache_init( &cache, false );
    ve_font_id a = ve_fontcache_load( &cache, bytes.data(), bytes.size(), 17 );
    ve_font_id b = ve_fontcache_load( &cache, bytes.data(), bytes.size(), 31 );
    ASSERT_EQ( a, 0 );
    ASSERT_EQ( b, 1 );
    vxui_gl_state gl = {};
    vxui_draw_cmd c[] = { packet( true ), packet( true ) };
    c[0].font_px = 17;
    c[1].font = (uint16_t) b;
    c[1].font_px = 31;
    c[1].render.colour = { 0.8f, 0.6f, 0.4f, 0.2f };
    vxui_gl_emit( &cache.drawlist, &gl, &cache, a, { c, 2 }, 640, 480, { { 10, 20 }, 1.125f } );

    ASSERT_EQ( cache.entry[a].size, 19.125f );
    ASSERT_EQ( cache.entry[b].size, 34.875f );
    int active = -1;
    int targets = 0;
    for ( const ve_fontcache_draw& d : cache.drawlist.dcalls )
    {
        if ( d.pass == DEMO_GL_PASS_SCISSOR_SET ) active = (int) d.region;
        if ( d.pass != VE_FONTCACHE_FRAMEBUFFER_PASS_TARGET ) continue;
        ASSERT_TRUE( active == 0 || active == 1 );
        for ( int i = 0; i < 4; i++ ) ASSERT_EQ( d.colour[i], c[active].render.colour[i] );
        targets++;
    }
    ASSERT_EQ( gl.scissor_data_count, 2 );
    ASSERT_GT( targets, 1 );
    ve_fontcache_shutdown( &cache );
}

UTEST(demo_packet_emit, fill_outline_and_marker_share_one_clip_then_the_next_packet_is_unclipped) {
    vxui_gl_state gl;
    init_gl( &gl );
    gl.material_data_count = 0;
    ve_fontcache cache = {};
    vxui_draw_cmd c[] = { packet( false ), packet( false, false ) };
    c[0].render.material_id = DEMO_MATERIAL_ROUND;
    c[0].render.outline_thickness = 2;
    c[0].render.texture_id = 42;
    vxui_gl_emit( &cache.drawlist, &gl, &cache, -1, { c, 2 }, 640, 480, {} );
    ASSERT_EQ( gl.scissor_data_count, 1 );
    ASSERT_EQ( gl.material_data_count, 2 );
    vxui_gl_execute( &gl, &cache, 640, 480 );
    ASSERT_EQ( s_draw_count, 4 );
    for ( int i = 0; i < 3; i++ )
    {
        ASSERT_EQ( s_draws[i].scissor.z, 80 );
        ASSERT_EQ( s_draws[i].scissor.w, 80 );
    }
    ASSERT_EQ( s_draws[3].scissor.z, 640 );
    ASSERT_EQ( s_draws[3].scissor.w, 480 );
}

UTEST(demo_packet_emit, full_mixed_packet_batch_keeps_one_marker_pair_per_emitted_packet) {
    std::vector< uint8_t > bytes = roboto_bytes();
    ve_fontcache cache = {};
    ve_fontcache_init( &cache, false );
    ve_font_id font = ve_fontcache_load( &cache, bytes.data(), bytes.size(), 24 );
    ASSERT_GE( font, 0 );
    vxui_gl_state gl = {};
    vxui_draw_cmd c[VXUI_MAX_DRAW_CMDS] = {};
    for ( int i = 0; i < VXUI_MAX_DRAW_CMDS; i++ ) c[i] = packet( i % 2 != 0 );
    vxui_gl_emit( &cache.drawlist, &gl, &cache, font, { c, VXUI_MAX_DRAW_CMDS }, 640, 480, {} );
    ASSERT_EQ( gl.scissor_data_count, VXUI_MAX_DRAW_CMDS );
    int sets = 0;
    int resets = 0;
    int rects = 0;
    for ( const ve_fontcache_draw& d : cache.drawlist.dcalls )
    {
        if ( d.pass == DEMO_GL_PASS_SCISSOR_SET ) ASSERT_EQ( d.region, (uint32_t) sets++ );
        if ( d.pass == DEMO_GL_PASS_SCISSOR_RESET ) resets++;
        if ( d.pass == DEMO_GL_PASS_RECT ) rects++;
    }
    ASSERT_EQ( sets, VXUI_MAX_DRAW_CMDS );
    ASSERT_EQ( resets, VXUI_MAX_DRAW_CMDS );
    ASSERT_EQ( rects, VXUI_MAX_DRAW_CMDS / 2 );
    ve_fontcache_shutdown( &cache );
}

UTEST(demo_packet_emit, empty_frames_and_reused_clip_slots_do_not_leave_markers_behind) {
    vxui_gl_state gl = {};
    ve_fontcache cache = {};
    vxui_draw_cmd c = packet( false );
    vxui_gl_emit( &cache.drawlist, &gl, &cache, -1, { &c, 1 }, 640, 480, {} );
    ve_fontcache_flush_drawlist( &cache );
    gl.scissor_data_count = 0;
    vxui_gl_emit( &cache.drawlist, &gl, &cache, -1, {}, 640, 480, {} );
    ASSERT_EQ( gl.scissor_data_count, 0 );
    ASSERT_TRUE( cache.drawlist.dcalls.empty() );
    c.clip = { 4, 8, 20, 40 };
    vxui_gl_emit( &cache.drawlist, &gl, &cache, -1, { &c, 1 }, 640, 480, {} );
    ASSERT_EQ( gl.scissor_data_count, 1 );
    ASSERT_EQ( cache.drawlist.dcalls[0].region, (uint32_t) 0 );
    ASSERT_NEAR( gl.scissor_data[0].x, 4.0f / 640.0f, 1e-6f );
}

UTEST_MAIN();
