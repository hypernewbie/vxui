#include "demo_gl_record.h"

static Clay_Dimensions measure_text( Clay_StringSlice s, Clay_TextElementConfig* cfg, void* )
{
    return { (float) s.length * 8.0f, (float) cfg->fontSize };
}

static vxui_draw_cmd clipped_rect( glm::vec4 clip )
{
    vxui_draw_cmd c  = {};
    c.type          = VXUI_DRAW_RECT;
    c.rect          = { 0, 0, 400, 400 };
    c.clipped       = true;
    c.clip          = clip;
    c.render.colour = { 0, 1, 0, 1 };
    return c;
}

UTEST(demo_empty_clip, zero_width_or_height_keeps_a_zero_area_scissor) {
    glm::vec4 clips[] = { { 10, 20, 0, 100 }, { 10, 20, 100, 0 }, { 10, 20, 0, 0 } };
    for ( int i = 0; i < 3; i++ )
    {
        vxui_gl_state gl;
        init_gl( &gl );
        ve_fontcache cache = {};
        vxui_draw_cmd c = clipped_rect( clips[i] );
        vxui_gl_emit( &cache.drawlist, &gl, &cache, -1, { &c, 1 }, 640, 480, {} );
        vxui_gl_execute( &gl, &cache, 640, 480 );

        ASSERT_EQ( s_draw_count, 1 );
        ASSERT_EQ( s_invalid_scissors, 0 );
        ASSERT_GE( s_draws[0].scissor.z, 0 );
        ASSERT_GE( s_draws[0].scissor.w, 0 );
        ASSERT_TRUE( s_draws[0].scissor.z == 0 || s_draws[0].scissor.w == 0 );
    }
}

UTEST(demo_empty_clip, negative_width_is_an_empty_gl_scissor) {
    vxui_gl_state gl;
    init_gl( &gl );
    ve_fontcache cache = {};
    vxui_draw_cmd c = clipped_rect( { 10, 20, -8, 100 } );
    vxui_gl_emit( &cache.drawlist, &gl, &cache, -1, { &c, 1 }, 640, 480, {} );
    ASSERT_GE( gl.scissor_data[0].z, 0.0f );
    vxui_gl_execute( &gl, &cache, 640, 480 );
    ASSERT_EQ( s_invalid_scissors, 0 );
    ASSERT_EQ( s_draws[0].scissor.z, 0 );
    ASSERT_EQ( s_draws[0].scissor.w, 100 );
}

UTEST(demo_empty_clip, negative_height_is_an_empty_gl_scissor) {
    vxui_gl_state gl;
    init_gl( &gl );
    ve_fontcache cache = {};
    vxui_draw_cmd c = clipped_rect( { 10, 20, 100, -8 } );
    vxui_gl_emit( &cache.drawlist, &gl, &cache, -1, { &c, 1 }, 640, 480, {} );
    ASSERT_GE( gl.scissor_data[0].w, 0.0f );
    vxui_gl_execute( &gl, &cache, 640, 480 );
    ASSERT_EQ( s_invalid_scissors, 0 );
    ASSERT_EQ( s_draws[0].scissor.w, 0 );
    ASSERT_EQ( s_draws[0].scissor.z, 100 );
}

UTEST(demo_empty_clip, negative_extents_stay_empty_at_fractional_scales_and_offsets) {
    float scales[] = { 0.625f, 1.125f, 2.0f };
    for ( int i = 0; i < 3; i++ )
    {
        vxui_gl_state gl;
        init_gl( &gl );
        ve_fontcache cache = {};
        vxui_draw_cmd c = clipped_rect( { 10, 20, -8, -12 } );
        vxui_gl_emit( &cache.drawlist, &gl, &cache, -1, { &c, 1 }, 640, 480, { { 7, 11 }, scales[i] } );
        vxui_gl_execute( &gl, &cache, 640, 480 );
        ASSERT_EQ( s_invalid_scissors, 0 );
        ASSERT_EQ( s_draws[0].scissor.z, 0 );
        ASSERT_EQ( s_draws[0].scissor.w, 0 );
    }
}

UTEST(demo_empty_clip, an_empty_clip_covers_fill_outline_and_marker_then_resets) {
    vxui_gl_state gl;
    init_gl( &gl );
    ve_fontcache cache = {};
    vxui_draw_cmd c[2] = { clipped_rect( { 10, 20, -8, 100 } ), clipped_rect( {} ) };
    c[0].render.outline_thickness = 2;
    c[0].render.texture_id = 42;
    c[1].clipped = false;
    vxui_gl_emit( &cache.drawlist, &gl, &cache, -1, { c, 2 }, 640, 480, {} );
    vxui_gl_execute( &gl, &cache, 640, 480 );

    ASSERT_EQ( s_draw_count, 4 );
    ASSERT_EQ( s_invalid_scissors, 0 );
    for ( int i = 0; i < 3; i++ ) ASSERT_EQ( s_draws[i].scissor.z, 0 );
    ASSERT_EQ( s_draws[3].scissor.z, 640 );
    ASSERT_EQ( s_draws[3].scissor.w, 480 );
}

UTEST(demo_empty_clip, glyph_work_keeps_its_clip_but_text_targets_get_the_empty_ui_clip) {
    vxui_gl_state gl;
    init_gl( &gl );
    ve_fontcache cache = {};
    vxui_draw_cmd c = clipped_rect( { 10, 20, 100, -8 } );
    vxui_gl_push_scissor_set( &cache.drawlist, &gl, vxui_gl_scissor_rect( &c, 640, 480, {} ) );
    push_draw( &cache, VE_FONTCACHE_FRAMEBUFFER_PASS_GLYPH );
    push_draw( &cache, VE_FONTCACHE_FRAMEBUFFER_PASS_GLYPH_RESOLVE );
    push_draw( &cache, VE_FONTCACHE_FRAMEBUFFER_PASS_TARGET );
    push_draw( &cache, VE_FONTCACHE_FRAMEBUFFER_PASS_TARGET_UNCACHED );
    vxui_gl_push_scissor_reset( &cache.drawlist );
    push_draw( &cache, DEMO_GL_PASS_RECT );
    vxui_gl_execute( &gl, &cache, 640, 480 );

    ASSERT_EQ( s_draw_count, 5 );
    ASSERT_EQ( s_invalid_scissors, 0 );
    ASSERT_EQ( s_draws[0].scissor.w, VE_FONTCACHE_GLYPHDRAW_BUFFER_HEIGHT );
    ASSERT_EQ( s_draws[1].scissor.w, VE_FONTCACHE_GLYPHDRAW_BUFFER_HEIGHT );
    ASSERT_EQ( s_draws[2].scissor.w, 0 );
    ASSERT_EQ( s_draws[3].scissor.w, 0 );
    ASSERT_EQ( s_draws[4].scissor.w, 480 );
}

UTEST(demo_empty_clip, positive_clips_keep_their_transform_and_window_conversion) {
    vxui_gl_state gl;
    init_gl( &gl );
    ve_fontcache cache = {};
    vxui_draw_cmd c = clipped_rect( { 8, 16, 40, 24 } );
    vxui_gl_emit( &cache.drawlist, &gl, &cache, -1, { &c, 1 }, 640, 480, { { 10, 20 }, 1.125f } );
    vxui_gl_execute( &gl, &cache, 640, 480 );

    ASSERT_EQ( s_invalid_scissors, 0 );
    ASSERT_EQ( s_draws[0].scissor.x, 19 );
    ASSERT_EQ( s_draws[0].scissor.y, 415 );
    ASSERT_EQ( s_draws[0].scissor.z, 45 );
    ASSERT_EQ( s_draws[0].scissor.w, 27 );
}

UTEST(demo_empty_clip, disjoint_nested_clay_clips_remain_unhittable_and_empty_in_gl) {
    vxui_ctx ctx = {};
    vxui_init( &ctx, 640, 480, s_clay_mem, sizeof( s_clay_mem ) );
    Clay_SetMeasureTextFunction( measure_text, nullptr );
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_mouse( &ctx, 104, 104, VXUI_MOUSE_LEFT );
    Clay__OpenElementWithId( vxui_clay_id( "outer" ) );
    Clay_ElementDeclaration outer = {};
    outer.layout.sizing.width     = CLAY_SIZING_FIXED( 80 );
    outer.layout.sizing.height    = CLAY_SIZING_FIXED( 60 );
    outer.clip.vertical           = true;
    outer.clip.childOffset        = { 100, 100 };
    Clay__ConfigureOpenElement( outer );
    Clay__OpenElementWithId( vxui_clay_id( "inner" ) );
    Clay_ElementDeclaration inner = {};
    inner.layout.sizing.width     = CLAY_SIZING_FIXED( 60 );
    inner.layout.sizing.height    = CLAY_SIZING_FIXED( 40 );
    inner.layout.layoutDirection  = CLAY_TOP_TO_BOTTOM;
    inner.clip.vertical           = true;
    Clay__ConfigureOpenElement( inner );
    vxui_rect( &ctx, "rect", { .width = { VXUI_FIXED, 10 }, .height = { VXUI_FIXED, 10 } } );
    vxui_div_end( &ctx );
    vxui_text( &ctx, "text", "A", 24 );
    Clay__CloseElement();
    Clay__CloseElement();
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 2 );
    ASSERT_LT( dl.cmds[0].clip.z, 0.0f );
    ASSERT_LT( dl.cmds[0].clip.w, 0.0f );
    ASSERT_FALSE( dl.cmds[0].state & VXUI_DRAW_HOVERED );
    ASSERT_EQ( ctx.mouse_press_row_id, (uint32_t) 0 );
    vxui_gl_state gl;
    init_gl( &gl );
    ve_fontcache cache = {};
    vxui_gl_emit( &cache.drawlist, &gl, &cache, -1, dl, 640, 480, {} );
    vxui_gl_execute( &gl, &cache, 640, 480 );
    ASSERT_EQ( s_invalid_scissors, 0 );
    ASSERT_EQ( s_draws[0].scissor.z, 0 );
    ASSERT_EQ( s_draws[0].scissor.w, 0 );
}

UTEST_MAIN();
