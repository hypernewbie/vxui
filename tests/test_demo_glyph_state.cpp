#include "demo_gl_record.h"

UTEST(demo_glyph_state, glyph_pass_restores_winding_stencil_state_from_dirty_gl) {
    vxui_gl_state gl;
    init_gl( &gl );
    s_current.blend = true;
    s_current.srgb = true;
    s_current.framebuffer = 999;
    s_current.colour_mask = { true, true, true, true };
    ve_fontcache cache = {};
    push_draw( &cache, VE_FONTCACHE_FRAMEBUFFER_PASS_GLYPH );
    vxui_gl_execute( &gl, &cache, 640, 480 );

    ASSERT_EQ( s_draw_count, 1 );
    const gl_snapshot& s = s_draws[0];
    ASSERT_EQ( s.program, gl.shader_render_glyph );
    ASSERT_EQ( s.framebuffer, gl.fbo[0] );
    ASSERT_FALSE( s.blend );
    ASSERT_FALSE( s.srgb );
    ASSERT_TRUE( s.stencil );
    ASSERT_FALSE( s.colour_mask.r || s.colour_mask.g || s.colour_mask.b || s.colour_mask.a );
    ASSERT_EQ( s.stencil_write_mask, (GLuint) 0xFF );
    ASSERT_EQ( s.stencil_compare, (GLenum) GL_ALWAYS );
    ASSERT_EQ( s.stencil_ref, 0 );
    ASSERT_EQ( s.stencil_compare_mask, (GLuint) 0xFF );
    ASSERT_EQ( s.stencil_front.x, (GLuint) GL_KEEP );
    ASSERT_EQ( s.stencil_front.y, (GLuint) GL_KEEP );
    ASSERT_EQ( s.stencil_front.z, (GLuint) GL_INCR_WRAP );
    ASSERT_EQ( s.stencil_back.x, (GLuint) GL_KEEP );
    ASSERT_EQ( s.stencil_back.y, (GLuint) GL_KEEP );
    ASSERT_EQ( s.stencil_back.z, (GLuint) GL_DECR_WRAP );
    ASSERT_EQ( s.viewport.z, VE_FONTCACHE_GLYPHDRAW_BUFFER_WIDTH );
    ASSERT_EQ( s.viewport.w, VE_FONTCACHE_GLYPHDRAW_BUFFER_HEIGHT );
}

UTEST(demo_glyph_state, resolve_pass_sets_colour_and_zeroes_both_stencil_faces) {
    vxui_gl_state gl;
    init_gl( &gl );
    s_current.stencil_front = { GL_REPLACE, GL_INCR_WRAP, GL_DECR_WRAP };
    s_current.stencil_back = { GL_REPLACE, GL_INCR_WRAP, GL_DECR_WRAP };
    ve_fontcache cache = {};
    push_draw( &cache, VE_FONTCACHE_FRAMEBUFFER_PASS_GLYPH_RESOLVE );
    vxui_gl_execute( &gl, &cache, 640, 480 );

    ASSERT_EQ( s_draw_count, 1 );
    const gl_snapshot& s = s_draws[0];
    ASSERT_EQ( s.program, gl.shader_render_glyph );
    ASSERT_EQ( s.framebuffer, gl.fbo[0] );
    ASSERT_TRUE( s.stencil );
    ASSERT_FALSE( s.blend );
    ASSERT_FALSE( s.srgb );
    ASSERT_TRUE( s.colour_mask.r && s.colour_mask.g && s.colour_mask.b && s.colour_mask.a );
    ASSERT_EQ( s.stencil_compare, (GLenum) GL_NOTEQUAL );
    ASSERT_EQ( s.stencil_ref, 0 );
    ASSERT_EQ( s.stencil_compare_mask, (GLuint) 0xFF );
    ASSERT_EQ( s.stencil_write_mask, (GLuint) 0xFF );
    for ( int i = 0; i < 3; i++ )
    {
        GLuint expected = i == 2 ? GL_ZERO : GL_KEEP;
        ASSERT_EQ( s.stencil_front[i], expected );
        ASSERT_EQ( s.stencil_back[i], expected );
    }
}

UTEST(demo_glyph_state, alternating_glyph_and_resolve_reset_each_others_stencil_policy) {
    vxui_gl_state gl;
    init_gl( &gl );
    ve_fontcache cache = {};
    for ( int i = 0; i < 4; i++ )
        push_draw( &cache, i % 2 == 0 ? VE_FONTCACHE_FRAMEBUFFER_PASS_GLYPH : VE_FONTCACHE_FRAMEBUFFER_PASS_GLYPH_RESOLVE );
    vxui_gl_execute( &gl, &cache, 640, 480 );

    ASSERT_EQ( s_draw_count, 4 );
    for ( int i = 0; i < 4; i++ )
    {
        bool glyph = i % 2 == 0;
        ASSERT_EQ( s_draws[i].stencil_compare, (GLenum) ( glyph ? GL_ALWAYS : GL_NOTEQUAL ) );
        ASSERT_EQ( s_draws[i].stencil_front.z, (GLuint) ( glyph ? GL_INCR_WRAP : GL_ZERO ) );
        ASSERT_EQ( s_draws[i].stencil_back.z, (GLuint) ( glyph ? GL_DECR_WRAP : GL_ZERO ) );
        ASSERT_EQ( s_draws[i].colour_mask.a, !glyph );
    }
}

UTEST(demo_glyph_state, glyph_clear_opens_colour_writes_then_restores_the_stencil_only_draw) {
    vxui_gl_state gl;
    init_gl( &gl );
    ve_fontcache cache = {};
    push_draw( &cache, VE_FONTCACHE_FRAMEBUFFER_PASS_GLYPH );
    cache.drawlist.dcalls[0].clear_before_draw = true;
    vxui_gl_execute( &gl, &cache, 640, 480 );

    ASSERT_EQ( s_clear_count, 1 );
    ASSERT_EQ( s_clear_bits[0], (GLbitfield) ( GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT ) );
    ASSERT_EQ( s_clears[0].framebuffer, gl.fbo[0] );
    ASSERT_TRUE( s_clears[0].colour_mask.r && s_clears[0].colour_mask.g && s_clears[0].colour_mask.b && s_clears[0].colour_mask.a );
    ASSERT_EQ( s_clears[0].stencil_clear_value, 0 );
    ASSERT_EQ( s_clears[0].stencil_write_mask, (GLuint) 0xFF );
    ASSERT_FALSE( s_draws[0].colour_mask.r || s_draws[0].colour_mask.g || s_draws[0].colour_mask.b || s_draws[0].colour_mask.a );
    ASSERT_EQ( s_draws[0].stencil_front.z, (GLuint) GL_INCR_WRAP );
    ASSERT_EQ( s_draws[0].stencil_back.z, (GLuint) GL_DECR_WRAP );
}

UTEST(demo_glyph_state, resolve_clear_does_not_clear_the_stencil_surface) {
    vxui_gl_state gl;
    init_gl( &gl );
    ve_fontcache cache = {};
    push_draw( &cache, VE_FONTCACHE_FRAMEBUFFER_PASS_GLYPH_RESOLVE );
    cache.drawlist.dcalls[0].clear_before_draw = true;
    vxui_gl_execute( &gl, &cache, 640, 480 );

    ASSERT_EQ( s_clear_count, 1 );
    ASSERT_EQ( s_clear_bits[0], (GLbitfield) GL_COLOR_BUFFER_BIT );
    ASSERT_EQ( s_clears[0].stencil_clear_value, -1 );
    ASSERT_TRUE( s_draws[0].colour_mask.a );
    ASSERT_EQ( s_draws[0].stencil_front.z, (GLuint) GL_ZERO );
    ASSERT_EQ( s_draws[0].stencil_back.z, (GLuint) GL_ZERO );
}

UTEST(demo_glyph_state, zero_index_clear_commands_survive_the_optimiser_without_draws) {
    vxui_gl_state gl;
    init_gl( &gl );
    ve_fontcache cache = {};
    push_draw( &cache, VE_FONTCACHE_FRAMEBUFFER_PASS_GLYPH );
    push_draw( &cache, VE_FONTCACHE_FRAMEBUFFER_PASS_GLYPH_RESOLVE );
    for ( int i = 0; i < 2; i++ )
    {
        cache.drawlist.dcalls[i].clear_before_draw = true;
        cache.drawlist.dcalls[i].end_index = 0;
    }
    vxui_gl_execute( &gl, &cache, 640, 480 );

    ASSERT_EQ( s_draw_count, 0 );
    ASSERT_EQ( s_clear_count, 2 );
    ASSERT_EQ( s_clear_bits[0], (GLbitfield) ( GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT ) );
    ASSERT_EQ( s_clear_bits[1], (GLbitfield) GL_COLOR_BUFFER_BIT );
    ASSERT_TRUE( cache.drawlist.dcalls.empty() );
}

UTEST(demo_glyph_state, atlas_and_window_passes_still_restore_their_separate_clip_state) {
    vxui_gl_state gl;
    init_gl( &gl );
    ve_fontcache cache = {};
    vxui_gl_push_scissor_set( &cache.drawlist, &gl, { 0.25f, 0.25f, 0.5f, 0.5f } );
    push_draw( &cache, VE_FONTCACHE_FRAMEBUFFER_PASS_GLYPH );
    push_draw( &cache, VE_FONTCACHE_FRAMEBUFFER_PASS_GLYPH_RESOLVE );
    push_draw( &cache, VE_FONTCACHE_FRAMEBUFFER_PASS_ATLAS );
    push_draw( &cache, VE_FONTCACHE_FRAMEBUFFER_PASS_TARGET );
    vxui_gl_execute( &gl, &cache, 640, 480 );

    ASSERT_EQ( s_draw_count, 4 );
    for ( int i = 0; i < 2; i++ )
    {
        ASSERT_EQ( s_draws[i].viewport.z, VE_FONTCACHE_GLYPHDRAW_BUFFER_WIDTH );
        ASSERT_EQ( s_draws[i].scissor.z, VE_FONTCACHE_GLYPHDRAW_BUFFER_WIDTH );
    }
    ASSERT_EQ( s_draws[2].program, gl.shader_blit_atlas );
    ASSERT_EQ( s_draws[2].framebuffer, gl.fbo[1] );
    ASSERT_EQ( s_draws[2].scissor.z, VE_FONTCACHE_ATLAS_WIDTH );
    ASSERT_EQ( s_draws[3].framebuffer, (GLuint) 0 );
    ASSERT_EQ( s_draws[3].scissor.x, 160 );
    ASSERT_EQ( s_draws[3].scissor.y, 120 );
    ASSERT_EQ( s_draws[3].scissor.z, 320 );
    ASSERT_EQ( s_draws[3].scissor.w, 240 );
}

UTEST(demo_glyph_state, glyph_target_dimensions_do_not_change_with_window_resolution) {
    int widths[] = { 320, 640, 1920 };
    int heights[] = { 240, 480, 1080 };
    for ( int i = 0; i < 3; i++ )
    {
        vxui_gl_state gl;
        init_gl( &gl );
        ve_fontcache cache = {};
        push_draw( &cache, VE_FONTCACHE_FRAMEBUFFER_PASS_GLYPH );
        push_draw( &cache, VE_FONTCACHE_FRAMEBUFFER_PASS_GLYPH_RESOLVE );
        push_draw( &cache, VE_FONTCACHE_FRAMEBUFFER_PASS_TARGET );
        vxui_gl_execute( &gl, &cache, widths[i], heights[i] );
        for ( int j = 0; j < 2; j++ )
        {
            ASSERT_EQ( s_draws[j].viewport.z, VE_FONTCACHE_GLYPHDRAW_BUFFER_WIDTH );
            ASSERT_EQ( s_draws[j].viewport.w, VE_FONTCACHE_GLYPHDRAW_BUFFER_HEIGHT );
        }
        ASSERT_EQ( s_draws[2].viewport.z, widths[i] );
        ASSERT_EQ( s_draws[2].viewport.w, heights[i] );
    }
}

UTEST_MAIN();
