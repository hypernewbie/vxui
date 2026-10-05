#include "demo_gl_record.h"

UTEST(demo_texture_dispatch, image_and_mask_keep_distinct_shaders_with_identical_texture_binding) {
    for ( uint32_t pass : { DEMO_GL_PASS_RECT_TEXTURED, DEMO_GL_PASS_IMAGE } )
    {
        vxui_gl_state gl;
        init_gl( &gl );
        ve_fontcache cache = {};
        push_draw( &cache, pass, 42 );
        vxui_gl_execute( &gl, &cache, 640, 480 );
        ASSERT_EQ( s_draw_count, 1 );
        ASSERT_EQ( s_draws[0].program, pass == DEMO_GL_PASS_IMAGE ? gl.shader_image : gl.shader_rect_textured );
        ASSERT_EQ( s_draws[0].texture, 42u );
        ASSERT_EQ( s_draws[0].framebuffer, 0u );
    }
}

UTEST(demo_texture_dispatch, alternating_passes_keep_order_tint_and_texture_identity) {
    vxui_gl_state gl;
    init_gl( &gl );
    ve_fontcache cache = {};
    for ( int i = 0; i < 12; i++ )
    {
        push_draw( &cache, i % 2 == 0 ? DEMO_GL_PASS_IMAGE : DEMO_GL_PASS_RECT_TEXTURED, (uint32_t) i + 100 );
        cache.drawlist.dcalls.back().colour[0] = (float) i / 16;
    }
    vxui_gl_execute( &gl, &cache, 640, 480 );
    ASSERT_EQ( s_draw_count, 12 );
    for ( int i = 0; i < 12; i++ )
    {
        ASSERT_EQ( s_draws[i].program, i % 2 == 0 ? gl.shader_image : gl.shader_rect_textured );
        ASSERT_EQ( s_draws[i].texture, (GLuint) i + 100 );
        ASSERT_EQ( s_draws[i].colour.x, (float) i / 16 );
    }
}

UTEST(demo_texture_dispatch, transparent_and_signed_colour_components_arrive_unchanged) {
    for ( uint32_t pass : { DEMO_GL_PASS_RECT_TEXTURED, DEMO_GL_PASS_IMAGE } )
    {
        vxui_gl_state gl;
        init_gl( &gl );
        ve_fontcache cache = {};
        push_draw( &cache, pass, 42 );
        float colour[] = { -0.25f, 2.5f, 0.125f, 0.0f };
        memcpy( cache.drawlist.dcalls[0].colour, colour, sizeof( colour ) );
        vxui_gl_execute( &gl, &cache, 640, 480 );
        for ( int i = 0; i < 4; i++ ) ASSERT_EQ( s_draws[0].colour[i], colour[i] );
        ASSERT_EQ( s_draws[0].colour_uploads, 1 );
        ASSERT_EQ( s_draws[0].params_uploads, 0 );
        ASSERT_EQ( s_draws[0].flags_uploads, 0 );
    }
}

UTEST(demo_texture_dispatch, zero_and_high_texture_handles_are_not_material_slots) {
    for ( uint32_t texture : { 0u, 1u, UINT32_MAX } )
        for ( uint32_t pass : { DEMO_GL_PASS_RECT_TEXTURED, DEMO_GL_PASS_IMAGE } )
        {
            vxui_gl_state gl;
            init_gl( &gl );
            gl.material_data_count = 0;
            ve_fontcache cache = {};
            push_draw( &cache, pass, texture );
            vxui_gl_execute( &gl, &cache, 640, 480 );
            ASSERT_EQ( s_draws[0].texture, texture );
        }
}

UTEST(demo_texture_dispatch, glyph_and_atlas_changes_do_not_leak_into_texture_targets) {
    vxui_gl_state gl;
    init_gl( &gl );
    ve_fontcache cache = {};
    push_draw( &cache, VE_FONTCACHE_FRAMEBUFFER_PASS_GLYPH );
    push_draw( &cache, DEMO_GL_PASS_IMAGE, 41 );
    push_draw( &cache, VE_FONTCACHE_FRAMEBUFFER_PASS_ATLAS );
    push_draw( &cache, DEMO_GL_PASS_RECT_TEXTURED, 42 );
    vxui_gl_execute( &gl, &cache, 640, 480 );
    for ( int i : { 1, 3 } )
    {
        ASSERT_EQ( s_draws[i].framebuffer, 0u );
        ASSERT_TRUE( s_draws[i].blend );
        ASSERT_FALSE( s_draws[i].stencil );
        ASSERT_TRUE( s_draws[i].srgb );
        ASSERT_EQ( s_draws[i].viewport.z, 640 );
        ASSERT_EQ( s_draws[i].viewport.w, 480 );
    }
    ASSERT_EQ( s_draws[1].texture, 41u );
    ASSERT_EQ( s_draws[3].texture, 42u );
}

UTEST(demo_texture_dispatch, both_texture_passes_restore_the_ui_clip_after_glyph_passes) {
    vxui_gl_state gl;
    init_gl( &gl );
    ve_fontcache cache = {};
    vxui_gl_push_scissor_set( &cache.drawlist, &gl, { 0.25f, 0.5f, 0.5f, 0.25f } );
    push_draw( &cache, VE_FONTCACHE_FRAMEBUFFER_PASS_GLYPH );
    push_draw( &cache, DEMO_GL_PASS_IMAGE, 41 );
    push_draw( &cache, VE_FONTCACHE_FRAMEBUFFER_PASS_GLYPH_RESOLVE );
    push_draw( &cache, DEMO_GL_PASS_RECT_TEXTURED, 42 );
    vxui_gl_execute( &gl, &cache, 640, 480 );
    for ( int i : { 1, 3 } )
    {
        ASSERT_EQ( s_draws[i].scissor.x, 160 );
        ASSERT_EQ( s_draws[i].scissor.y, 240 );
        ASSERT_EQ( s_draws[i].scissor.z, 320 );
        ASSERT_EQ( s_draws[i].scissor.w, 120 );
    }
}

UTEST(demo_texture_dispatch, clear_only_texture_commands_select_their_shader_and_texture_without_drawing) {
    for ( uint32_t pass : { DEMO_GL_PASS_RECT_TEXTURED, DEMO_GL_PASS_IMAGE } )
    {
        vxui_gl_state gl;
        init_gl( &gl );
        ve_fontcache cache = {};
        push_draw( &cache, pass, 42 );
        cache.drawlist.dcalls[0].end_index = 0;
        cache.drawlist.dcalls[0].clear_before_draw = true;
        vxui_gl_execute( &gl, &cache, 640, 480 );
        ASSERT_EQ( s_draw_count, 0 );
        ASSERT_EQ( s_clear_count, 1 );
        ASSERT_EQ( s_clears[0].texture, 42u );
        ASSERT_EQ( s_clears[0].program, pass == DEMO_GL_PASS_IMAGE ? gl.shader_image : gl.shader_rect_textured );
    }
}

UTEST(demo_texture_dispatch, full_alternating_batch_never_uploads_material_payloads) {
    vxui_gl_state gl;
    init_gl( &gl );
    gl.material_data_count = 0;
    ve_fontcache cache = {};
    for ( int i = 0; i < VXUI_MAX_DRAW_CMDS; i++ ) push_draw( &cache, i % 2 == 0 ? DEMO_GL_PASS_IMAGE : DEMO_GL_PASS_RECT_TEXTURED, (uint32_t) i );
    vxui_gl_execute( &gl, &cache, 640, 480 );
    ASSERT_EQ( s_draw_count, VXUI_MAX_DRAW_CMDS );
    for ( int i = 0; i < s_draw_count; i++ )
    {
        ASSERT_EQ( s_draws[i].texture, (GLuint) i );
        ASSERT_EQ( s_draws[i].params_uploads, 0 );
        ASSERT_EQ( s_draws[i].flags_uploads, 0 );
    }
}

UTEST_MAIN();
