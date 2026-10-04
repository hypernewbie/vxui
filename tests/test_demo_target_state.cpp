#include "demo_gl_record.h"

UTEST(demo_target_state, every_target_pass_restores_window_state_after_glyph_draws) {
    uint32_t passes[] = { VE_FONTCACHE_FRAMEBUFFER_PASS_TARGET, VE_FONTCACHE_FRAMEBUFFER_PASS_TARGET_UNCACHED, DEMO_GL_PASS_RECT, DEMO_GL_PASS_RECT_TEXTURED, DEMO_GL_PASS_IMAGE, DEMO_GL_PASS_CRT, DEMO_GL_PASS_ROUND, DEMO_GL_PASS_OUTLINE };
    GLuint programs[] = { 3, 3, 4, 5, 6, 7, 8, 9 };
    for ( int i = 0; i < 8; i++ )
    {
        vxui_gl_state gl;
        init_gl( &gl );
        ve_fontcache cache = {};
        push_draw( &cache, VE_FONTCACHE_FRAMEBUFFER_PASS_GLYPH );
        push_draw( &cache, passes[i] );
        vxui_gl_execute( &gl, &cache, 640, 480 );

        ASSERT_EQ( s_draw_count, 2 );
        const gl_snapshot& s = s_draws[1];
        ASSERT_EQ( s.program, programs[i] );
        ASSERT_EQ( s.framebuffer, (GLuint) 0 );
        ASSERT_TRUE( s.blend );
        ASSERT_FALSE( s.stencil );
        ASSERT_TRUE( s.srgb );
        ASSERT_TRUE( s.colour_mask.r && s.colour_mask.g && s.colour_mask.b && s.colour_mask.a );
        ASSERT_EQ( s.blend_src, (GLenum) GL_SRC_ALPHA );
        ASSERT_EQ( s.blend_dst, (GLenum) GL_ONE_MINUS_SRC_ALPHA );
        ASSERT_EQ( s.viewport.z, 640 );
        ASSERT_EQ( s.viewport.w, 480 );
        ASSERT_EQ( s.scissor.x, 0 );
        ASSERT_EQ( s.scissor.y, 0 );
        ASSERT_EQ( s.scissor.z, 640 );
        ASSERT_EQ( s.scissor.w, 480 );
        ASSERT_TRUE( cache.drawlist.dcalls.empty() );
    }
}

UTEST(demo_target_state, glyph_and_resolve_passes_keep_their_own_state) {
    vxui_gl_state gl;
    init_gl( &gl );
    ve_fontcache cache = {};
    push_draw( &cache, VE_FONTCACHE_FRAMEBUFFER_PASS_GLYPH );
    push_draw( &cache, VE_FONTCACHE_FRAMEBUFFER_PASS_GLYPH_RESOLVE );
    vxui_gl_execute( &gl, &cache, 640, 480 );

    ASSERT_EQ( s_draw_count, 2 );
    for ( int i = 0; i < 2; i++ )
    {
        ASSERT_EQ( s_draws[i].framebuffer, (GLuint) 101 );
        ASSERT_TRUE( s_draws[i].stencil );
        ASSERT_FALSE( s_draws[i].blend );
        ASSERT_FALSE( s_draws[i].srgb );
        ASSERT_EQ( s_draws[i].viewport.z, VE_FONTCACHE_GLYPHDRAW_BUFFER_WIDTH );
        ASSERT_EQ( s_draws[i].viewport.w, VE_FONTCACHE_GLYPHDRAW_BUFFER_HEIGHT );
        ASSERT_EQ( s_draws[i].colour_mask.r, i == 1 );
    }
}

UTEST(demo_target_state, atlas_pass_is_not_given_window_state) {
    vxui_gl_state gl;
    init_gl( &gl );
    ve_fontcache cache = {};
    push_draw( &cache, VE_FONTCACHE_FRAMEBUFFER_PASS_GLYPH );
    push_draw( &cache, VE_FONTCACHE_FRAMEBUFFER_PASS_ATLAS );
    vxui_gl_execute( &gl, &cache, 640, 480 );

    ASSERT_EQ( s_draw_count, 2 );
    const gl_snapshot& s = s_draws[1];
    ASSERT_EQ( s.program, (GLuint) 2 );
    ASSERT_EQ( s.framebuffer, (GLuint) 102 );
    ASSERT_EQ( s.texture, (GLuint) 201 );
    ASSERT_FALSE( s.stencil );
    ASSERT_TRUE( s.blend );
    ASSERT_FALSE( s.srgb );
    ASSERT_EQ( s.viewport.z, VE_FONTCACHE_ATLAS_WIDTH );
    ASSERT_EQ( s.viewport.w, VE_FONTCACHE_ATLAS_HEIGHT );
}

UTEST(demo_target_state, glyph_draws_do_not_replace_the_ui_clip) {
    vxui_gl_state gl;
    init_gl( &gl );
    gl.scissor_data_count = 1;
    gl.scissor_data[0] = { 0.25f, 0.25f, 0.5f, 0.5f };
    ve_fontcache cache = {};
    vxui_gl_push_scissor_set( &cache.drawlist, &gl, gl.scissor_data[0] );
    push_draw( &cache, VE_FONTCACHE_FRAMEBUFFER_PASS_GLYPH );
    push_draw( &cache, VE_FONTCACHE_FRAMEBUFFER_PASS_TARGET );
    vxui_gl_push_scissor_reset( &cache.drawlist );
    push_draw( &cache, DEMO_GL_PASS_RECT );
    vxui_gl_execute( &gl, &cache, 640, 480 );

    ASSERT_EQ( s_draw_count, 3 );
    ASSERT_EQ( s_draws[1].scissor.x, 160 );
    ASSERT_EQ( s_draws[1].scissor.y, 120 );
    ASSERT_EQ( s_draws[1].scissor.z, 320 );
    ASSERT_EQ( s_draws[1].scissor.w, 240 );
    ASSERT_EQ( s_draws[2].scissor.x, 0 );
    ASSERT_EQ( s_draws[2].scissor.y, 0 );
    ASSERT_EQ( s_draws[2].scissor.z, 640 );
    ASSERT_EQ( s_draws[2].scissor.w, 480 );
}

UTEST(demo_target_state, cached_and_uncached_text_bind_different_textures) {
    uint32_t passes[] = { VE_FONTCACHE_FRAMEBUFFER_PASS_TARGET, VE_FONTCACHE_FRAMEBUFFER_PASS_TARGET_UNCACHED };
    for ( int i = 0; i < 2; i++ )
    {
        vxui_gl_state gl;
        init_gl( &gl );
        ve_fontcache cache = {};
        push_draw( &cache, passes[i] );
        vxui_gl_execute( &gl, &cache, 640, 480 );
        ASSERT_EQ( s_draw_count, 1 );
        ASSERT_EQ( s_draws[0].texture, (GLuint) ( i == 0 ? 202 : 201 ) );
        ASSERT_EQ( s_draws[0].downsample, (GLuint) i );
    }
}

UTEST(demo_target_state, material_parameters_and_flags_still_reach_the_shader) {
    uint32_t passes[] = { DEMO_GL_PASS_CRT, DEMO_GL_PASS_ROUND, DEMO_GL_PASS_OUTLINE };
    for ( int i = 0; i < 3; i++ )
    {
        vxui_gl_state gl;
        init_gl( &gl );
        ve_fontcache cache = {};
        push_draw( &cache, passes[i] );
        vxui_gl_execute( &gl, &cache, 640, 480 );
        ASSERT_EQ( s_draw_count, 1 );
        for ( int j = 0; j < 4; j++ ) ASSERT_EQ( s_draws[0].params[j], (float) j + 1.0f );
        if ( i == 0 ) ASSERT_EQ( s_draws[0].flags, (GLuint) 42 );
    }
}

UTEST(demo_target_state, image_and_marker_bind_the_declared_texture) {
    uint32_t passes[] = { DEMO_GL_PASS_IMAGE, DEMO_GL_PASS_RECT_TEXTURED };
    for ( int i = 0; i < 2; i++ )
    {
        vxui_gl_state gl;
        init_gl( &gl );
        ve_fontcache cache = {};
        push_draw( &cache, passes[i], 77 );
        vxui_gl_execute( &gl, &cache, 640, 480 );
        ASSERT_EQ( s_draw_count, 1 );
        ASSERT_EQ( s_draws[0].texture, (GLuint) 77 );
    }
}

UTEST_MAIN();
