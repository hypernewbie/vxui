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

struct gl_snapshot
{
    GLuint framebuffer = 0;
    GLuint program = 0;
    GLuint texture = 0;
    bool blend = false;
    bool stencil = false;
    bool srgb = false;
    glm::bvec4 colour_mask = {};
    glm::ivec4 viewport = {};
    glm::ivec4 scissor = {};
    GLenum blend_src = 0;
    GLenum blend_dst = 0;
    float params[4] = {};
    GLuint flags = 0;
    GLuint downsample = 0;
};

static gl_snapshot s_current;
static gl_snapshot s_draws[16];
static int s_draw_count = 0;

static void APIENTRY bind_vertex_array( GLuint ) {}
static void APIENTRY gen_buffers( GLsizei count, GLuint* buffers ) { for ( int i = 0; i < count; i++ ) buffers[i] = i + 1; }
static void APIENTRY bind_buffer( GLenum, GLuint ) {}
static void APIENTRY buffer_data( GLenum, GLsizeiptr, const void*, GLenum ) {}
static void APIENTRY enable_attrib( GLuint ) {}
static void APIENTRY attrib_pointer( GLuint, GLint, GLenum, GLboolean, GLsizei, const void* ) {}
static void APIENTRY use_program( GLuint program ) { s_current.program = program; }
static void APIENTRY bind_framebuffer( GLenum, GLuint framebuffer ) { s_current.framebuffer = framebuffer; }
static void APIENTRY viewport( GLint x, GLint y, GLsizei w, GLsizei h ) { s_current.viewport = { x, y, w, h }; }
static void APIENTRY scissor( GLint x, GLint y, GLsizei w, GLsizei h ) { s_current.scissor = { x, y, w, h }; }
static void APIENTRY colour_mask( GLboolean r, GLboolean g, GLboolean b, GLboolean a ) { s_current.colour_mask = { r != 0, g != 0, b != 0, a != 0 }; }
static void APIENTRY blend_func( GLenum src, GLenum dst ) { s_current.blend_src = src; s_current.blend_dst = dst; }
static void APIENTRY stencil_mask( GLuint ) {}
static void APIENTRY stencil_func( GLenum, GLint, GLuint ) {}
static void APIENTRY stencil_op( GLenum, GLenum, GLenum ) {}
static void APIENTRY stencil_op_separate( GLenum, GLenum, GLenum, GLenum ) {}
static void APIENTRY active_texture( GLenum ) {}
static void APIENTRY bind_texture( GLenum, GLuint texture ) { s_current.texture = texture; }
static void APIENTRY uniform_int( GLint, GLint ) {}
static void APIENTRY clear_colour( GLfloat, GLfloat, GLfloat, GLfloat ) {}
static void APIENTRY clear_stencil( GLint ) {}
static void APIENTRY clear( GLbitfield ) {}
static void APIENTRY delete_buffers( GLsizei, const GLuint* ) {}
static GLenum APIENTRY get_error() { return GL_NO_ERROR; }

static void APIENTRY enable( GLenum cap )
{
    if ( cap == GL_BLEND ) s_current.blend = true;
    if ( cap == GL_STENCIL_TEST ) s_current.stencil = true;
    if ( cap == GL_FRAMEBUFFER_SRGB ) s_current.srgb = true;
}

static void APIENTRY disable( GLenum cap )
{
    if ( cap == GL_BLEND ) s_current.blend = false;
    if ( cap == GL_STENCIL_TEST ) s_current.stencil = false;
    if ( cap == GL_FRAMEBUFFER_SRGB ) s_current.srgb = false;
}

static GLint APIENTRY uniform_location( GLuint, const GLchar* name )
{
    if ( strcmp( name, "mparams" ) == 0 ) return 1;
    if ( strcmp( name, "mflags" ) == 0 ) return 2;
    if ( strcmp( name, "downsample" ) == 0 ) return 3;
    return 0;
}

static void APIENTRY uniform_uint( GLint location, GLuint value )
{
    if ( location == 2 ) s_current.flags = value;
    if ( location == 3 ) s_current.downsample = value;
}

static void APIENTRY uniform_vec4( GLint location, GLsizei, const GLfloat* value )
{
    if ( location == 1 ) memcpy( s_current.params, value, sizeof( s_current.params ) );
}

static void APIENTRY draw_elements( GLenum, GLsizei, GLenum, const void* )
{
    assert( s_draw_count < 16 );
    s_draws[s_draw_count++] = s_current;
}

static void init_gl( vxui_gl_state* gl )
{
    *gl = {};
    gl->shader_render_glyph    = 1;
    gl->shader_blit_atlas      = 2;
    gl->shader_draw_text       = 3;
    gl->shader_rect            = 4;
    gl->shader_rect_textured   = 5;
    gl->shader_image           = 6;
    gl->shader_crt             = 7;
    gl->shader_round           = 8;
    gl->shader_outline         = 9;
    gl->fbo[0]                 = 101;
    gl->fbo[1]                 = 102;
    gl->fbo_texture[0]         = 201;
    gl->fbo_texture[1]         = 202;
    gl->material_data_count    = 1;
    gl->material_data[0].flags = 42;
    for ( int i = 0; i < 4; i++ ) gl->material_data[0].params[i] = (float) i + 1.0f;

    s_current = {};
    s_draw_count = 0;
    glBindVertexArray = bind_vertex_array;
    glGenBuffers = gen_buffers;
    glBindBuffer = bind_buffer;
    glBufferData = buffer_data;
    glEnableVertexAttribArray = enable_attrib;
    glVertexAttribPointer = attrib_pointer;
    glUseProgram = use_program;
    glBindFramebuffer = bind_framebuffer;
    glViewport = viewport;
    glScissor = scissor;
    glColorMask = colour_mask;
    glBlendFunc = blend_func;
    glStencilMask = stencil_mask;
    glStencilFunc = stencil_func;
    glStencilOp = stencil_op;
    glStencilOpSeparate = stencil_op_separate;
    glActiveTexture = active_texture;
    glBindTexture = bind_texture;
    glGetUniformLocation = uniform_location;
    glUniform1i = uniform_int;
    glUniform1ui = uniform_uint;
    glUniform4fv = uniform_vec4;
    glClearColor = clear_colour;
    glClearStencil = clear_stencil;
    glClear = clear;
    glDeleteBuffers = delete_buffers;
    glGetError = get_error;
    glEnable = enable;
    glDisable = disable;
    glDrawElements = draw_elements;
}

static void push_draw( ve_fontcache* cache, uint32_t pass, uint32_t region = 0 )
{
    if ( cache->drawlist.vertices.empty() )
    {
        cache->drawlist.vertices.resize( 3 );
        cache->drawlist.indices = { 0, 1, 2 };
    }
    ve_fontcache_draw dcall = {};
    dcall.pass             = pass;
    dcall.region           = region;
    dcall.end_index        = 3;
    cache->drawlist.dcalls.push_back( dcall );
}

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
