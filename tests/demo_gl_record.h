#pragma once

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
static int s_invalid_scissors = 0;

static void APIENTRY bind_vertex_array( GLuint ) {}
static void APIENTRY gen_buffers( GLsizei count, GLuint* buffers ) { for ( int i = 0; i < count; i++ ) buffers[i] = i + 1; }
static void APIENTRY bind_buffer( GLenum, GLuint ) {}
static void APIENTRY buffer_data( GLenum, GLsizeiptr, const void*, GLenum ) {}
static void APIENTRY enable_attrib( GLuint ) {}
static void APIENTRY attrib_pointer( GLuint, GLint, GLenum, GLboolean, GLsizei, const void* ) {}
static void APIENTRY use_program( GLuint program ) { s_current.program = program; }
static void APIENTRY bind_framebuffer( GLenum, GLuint framebuffer ) { s_current.framebuffer = framebuffer; }
static void APIENTRY viewport( GLint x, GLint y, GLsizei w, GLsizei h ) { s_current.viewport = { x, y, w, h }; }
static void APIENTRY scissor( GLint x, GLint y, GLsizei w, GLsizei h )
{
    if ( w < 0 || h < 0 ) s_invalid_scissors++;
    s_current.scissor = { x, y, w, h };
}
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
    s_invalid_scissors = 0;
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
