// Demo GL renderer for VXUI. Reference only — not part of the library.
// #define VXUI_DEMO_RENDER_GL_IMPL in one TU.

#ifndef VXUI_DEMO_RENDER_GL_H
#define VXUI_DEMO_RENDER_GL_H

#include "vxui.h"
#include <glad/glad.h>

void vxui_gl_init    ( vxui_ctx* ctx );
void vxui_gl_render  ( vxui_ctx* ctx, const vxui_draw_list& dl, float w, float h );
void vxui_gl_shutdown( vxui_ctx* ctx );

#endif // VXUI_DEMO_RENDER_GL_H

#ifdef VXUI_DEMO_RENDER_GL_IMPL

#include <cstdio>
#include <cassert>
#include <vector>
#include "ve_fontcache.h"

struct vxui_gl_state
{
    // Rect path.
    GLuint rect_vao    = 0;
    GLuint rect_vbo    = 0;
    GLuint rect_ibo    = 0;
    GLuint rect_prog   = 0;
    GLint  rect_colour = -1;
    GLint  rect_screen = -1;

    // Text path (VEFC backend).
    GLuint text_vao            = 0;
    GLuint shader_render_glyph = 0;
    GLuint shader_blit_atlas   = 0;
    GLuint shader_draw_text    = 0;
    GLuint fbo[2]              = { 0, 0 };
    GLuint fbo_texture[2]      = { 0, 0 };
};

static const char* s_vxui_gl_rect_vs = R"(#version 330 core
in vec2 vpos;
uniform vec2 screen;
void main( void )
{
    vec2 ndc = vpos / screen * 2.0 - 1.0;
    gl_Position = vec4( ndc.x, -ndc.y, 0.0, 1.0 );
}
)";

static const char* s_vxui_gl_rect_fs = R"(#version 330 core
out vec4 fragc;
uniform vec4 colour;
void main( void )
{
    fragc = colour;
}
)";

// VEFC text shaders — byte-identical to vefc/demo/demo.cpp.
static const char* s_vxui_gl_vefc_vs_shared = R"(#version 330 core
in vec2 vpos;
in vec2 vtex;
out vec2 uv;
void main( void ) {
    uv = vtex;
    gl_Position = vec4( vpos.xy, 0.0, 1.0 );
}
)";

static const char* s_vxui_gl_vefc_fs_render_glyph = R"(#version 330 core
out vec4 fragc;
void main( void ) {
    fragc = vec4( 1.0, 1.0, 1.0, 1.0 );
}
)";

static const char* s_vxui_gl_vefc_fs_blit_atlas = R"(#version 330 core
in vec2 uv;
out vec4 fragc;
uniform uint region;
uniform sampler2D src_texture;
float downsample( vec2 uv, vec2 texsz )
{
    float v =
        texture( src_texture, uv + vec2( 0.0f, 0.0f ) * texsz ).x * 0.25f +
        texture( src_texture, uv + vec2( 0.0f, 1.0f ) * texsz ).x * 0.25f +
        texture( src_texture, uv + vec2( 1.0f, 0.0f ) * texsz ).x * 0.25f +
        texture( src_texture, uv + vec2( 1.0f, 1.0f ) * texsz ).x * 0.25f;
    return v;
}
void main( void ) {
    const vec2 texsz = 1.0f / vec2( 2048, 512 );
    if ( region == 0u || region == 1u || region == 2u ) {
        float v =
            downsample( uv + vec2( -1.5f, -1.5f ) * texsz, texsz ) * 0.25f +
            downsample( uv + vec2(  0.5f, -1.5f ) * texsz, texsz ) * 0.25f +
            downsample( uv + vec2( -1.5f,  0.5f ) * texsz, texsz ) * 0.25f +
            downsample( uv + vec2(  0.5f,  0.5f ) * texsz, texsz ) * 0.25f;
        fragc = vec4( 1, 1, 1, v );
    } else {
        fragc = vec4( 0, 0, 0, 1 );
    }
}
)";

static const char* s_vxui_gl_vefc_vs_draw_text = R"(#version 330 core
in vec2 vpos;
in vec2 vtex;
out vec2 uv;
void main( void ) {
    uv = vtex;
    gl_Position = vec4( vpos.xy * 2.0f - 1.0f, 0.0, 1.0 );
}
)";

static const char* s_vxui_gl_vefc_fs_draw_text = R"(#version 330 core
in vec2 uv;
out vec4 fragc;
uniform sampler2D src_texture;
uniform uint downsample;
uniform vec4 colour;
void main( void ) {
    float v = texture( src_texture, uv ).x;
    if ( downsample == 1u ) {
        const vec2 texsz = 1.0f / vec2( 2048, 512 );
        v =
            texture( src_texture, uv + vec2(-0.5f,-0.5f ) * texsz ).x * 0.25f +
            texture( src_texture, uv + vec2(-0.5f, 0.5f ) * texsz ).x * 0.25f +
            texture( src_texture, uv + vec2( 0.5f,-0.5f ) * texsz ).x * 0.25f +
            texture( src_texture, uv + vec2( 0.5f, 0.5f ) * texsz ).x * 0.25f;
    }
    fragc = vec4( colour.xyz, colour.a * v );
}
)";

static GLuint vxui_gl_compile( const char* vs_src, const char* fs_src, bool with_vtex )
{
    char log[1024];
    GLint ok = 0;

    GLuint vs = glCreateShader( GL_VERTEX_SHADER );
    glShaderSource( vs, 1, &vs_src, nullptr );
    glCompileShader( vs );
    glGetShaderiv( vs, GL_COMPILE_STATUS, &ok );
    if ( !ok ) { glGetShaderInfoLog( vs, sizeof( log ), nullptr, log ); fprintf( stderr, "vxui_gl: vs compile: %s\n", log ); }

    GLuint fs = glCreateShader( GL_FRAGMENT_SHADER );
    glShaderSource( fs, 1, &fs_src, nullptr );
    glCompileShader( fs );
    glGetShaderiv( fs, GL_COMPILE_STATUS, &ok );
    if ( !ok ) { glGetShaderInfoLog( fs, sizeof( log ), nullptr, log ); fprintf( stderr, "vxui_gl: fs compile: %s\n", log ); }

    GLuint p = glCreateProgram();
    glAttachShader( p, vs );
    glAttachShader( p, fs );
    glBindAttribLocation( p, 0, "vpos" );
    if ( with_vtex ) glBindAttribLocation( p, 1, "vtex" );
    glLinkProgram( p );
    glGetProgramiv( p, GL_LINK_STATUS, &ok );
    if ( !ok ) { glGetProgramInfoLog( p, sizeof( log ), nullptr, log ); fprintf( stderr, "vxui_gl: link: %s\n", log ); }

    glDeleteShader( vs );
    glDeleteShader( fs );
    return p;
}

static void vxui_gl_setup_fbo( vxui_gl_state* gl )
{
    glGenFramebuffers( 2, gl->fbo );
    glGenTextures( 2, gl->fbo_texture );

    glBindFramebuffer( GL_FRAMEBUFFER, gl->fbo[0] );
    glBindTexture( GL_TEXTURE_2D, gl->fbo_texture[0] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_R8, VE_FONTCACHE_GLYPHDRAW_BUFFER_WIDTH, VE_FONTCACHE_GLYPHDRAW_BUFFER_HEIGHT, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gl->fbo_texture[0], 0 );

    glBindFramebuffer( GL_FRAMEBUFFER, gl->fbo[1] );
    glBindTexture( GL_TEXTURE_2D, gl->fbo_texture[1] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_R8, VE_FONTCACHE_ATLAS_WIDTH, VE_FONTCACHE_ATLAS_HEIGHT, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gl->fbo_texture[1], 0 );

    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
}

void vxui_gl_init( vxui_ctx* ctx )
{
    assert( ctx );
    assert( !ctx->renderer && "vxui_gl_init called twice" );

    vxui_gl_state* gl = new vxui_gl_state();
    ctx->renderer = gl;

    // Rect path.
    gl->rect_prog   = vxui_gl_compile( s_vxui_gl_rect_vs, s_vxui_gl_rect_fs, false );
    gl->rect_colour = glGetUniformLocation( gl->rect_prog, "colour" );
    gl->rect_screen = glGetUniformLocation( gl->rect_prog, "screen" );

    glGenVertexArrays( 1, &gl->rect_vao );
    glGenBuffers( 1, &gl->rect_vbo );
    glGenBuffers( 1, &gl->rect_ibo );
    glBindVertexArray( gl->rect_vao );
    glBindBuffer( GL_ARRAY_BUFFER, gl->rect_vbo );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, gl->rect_ibo );
    glEnableVertexAttribArray( 0 );
    glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof( float ), (void*) 0 );
    glBindVertexArray( 0 );

    // Text path (VEFC).
    gl->shader_render_glyph = vxui_gl_compile( s_vxui_gl_vefc_vs_shared,    s_vxui_gl_vefc_fs_render_glyph, true );
    gl->shader_blit_atlas   = vxui_gl_compile( s_vxui_gl_vefc_vs_shared,    s_vxui_gl_vefc_fs_blit_atlas,   true );
    gl->shader_draw_text    = vxui_gl_compile( s_vxui_gl_vefc_vs_draw_text, s_vxui_gl_vefc_fs_draw_text,    true );
    glGenVertexArrays( 1, &gl->text_vao );
    vxui_gl_setup_fbo( gl );
}

static void vxui_gl_push_quad( std::vector< float >& verts, std::vector< uint32_t >& idx,
                               float x, float y, float w, float h )
{
    uint32_t base = (uint32_t) ( verts.size() / 2 );
    verts.insert( verts.end(), { x, y,  x + w, y,  x + w, y + h,  x, y + h } );
    idx.insert( idx.end(), { base, base + 1, base + 2,  base, base + 2, base + 3 } );
}

static void vxui_gl_render_rects( vxui_gl_state* gl, const vxui_draw_list& dl, float w, float h )
{
    int rect_n = vxui_draw_count( dl, VXUI_DRAW_RECT );
    if ( rect_n <= 0 ) return;

    // TODO Phase 3: per-rect colour via VEFC dcall. Phase 1 = last rect = focus.
    std::vector< float >    verts;
    std::vector< uint32_t > idx;

    for ( int i = 0; i < rect_n - 1; i++ )
    {
        const vxui_draw_cmd* c = vxui_draw_nth( dl, VXUI_DRAW_RECT, i );
        vxui_gl_push_quad( verts, idx, c->rect.x, c->rect.y, c->rect.z, c->rect.w );
    }
    int row_idx_count   = (int) idx.size();
    int focus_idx_start = row_idx_count;
    {
        const vxui_draw_cmd* c = vxui_draw_nth( dl, VXUI_DRAW_RECT, rect_n - 1 );
        vxui_gl_push_quad( verts, idx, c->rect.x, c->rect.y, c->rect.z, c->rect.w );
    }
    int focus_idx_count = (int) idx.size() - focus_idx_start;

    glBindVertexArray( gl->rect_vao );
    glBindBuffer( GL_ARRAY_BUFFER, gl->rect_vbo );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, gl->rect_ibo );
    glBufferData( GL_ARRAY_BUFFER, (GLsizeiptr)( verts.size() * sizeof( float ) ), verts.data(), GL_DYNAMIC_DRAW );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)( idx.size() * sizeof( uint32_t ) ), idx.data(), GL_DYNAMIC_DRAW );

    glUseProgram( gl->rect_prog );
    glUniform2f( gl->rect_screen, w, h );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    if ( row_idx_count > 0 )
    {
        glUniform4f( gl->rect_colour, 0.15f, 0.15f, 0.18f, 1.0f );
        glDrawElements( GL_TRIANGLES, row_idx_count, GL_UNSIGNED_INT, (void*) 0 );
    }
    if ( focus_idx_count > 0 )
    {
        glUniform4f( gl->rect_colour, 0.30f, 0.55f, 0.85f, 1.0f );
        glDrawElements( GL_TRIANGLES, focus_idx_count, GL_UNSIGNED_INT, (void*) ( (size_t) focus_idx_start * sizeof( uint32_t ) ) );
    }
    glBindVertexArray( 0 );
}

// VEFC drawlist execute loop. Phase 2a: stands up the dispatch; Phase 2b
// fills the drawlist with text via ve_fontcache_draw_text.
static void vxui_gl_execute_text( vxui_gl_state* gl, ve_fontcache* cache, int win_w, int win_h )
{
    ve_fontcache_optimise_drawlist( cache );
    ve_fontcache_drawlist* drawlist = ve_fontcache_get_drawlist( cache );
    if ( drawlist->dcalls.empty() )
    {
        ve_fontcache_flush_drawlist( cache );
        return;
    }

    glBindVertexArray( gl->text_vao );
    GLuint vbo = 0, ibo = 0;
    glGenBuffers( 1, &vbo );
    glGenBuffers( 1, &ibo );
    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    glBufferData( GL_ARRAY_BUFFER, (GLsizeiptr)( drawlist->vertices.size() * sizeof( ve_fontcache_vertex ) ), drawlist->vertices.data(), GL_DYNAMIC_DRAW );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibo );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)( drawlist->indices.size() * sizeof( uint32_t ) ), drawlist->indices.data(), GL_DYNAMIC_DRAW );

    glEnableVertexAttribArray( 0 );
    glEnableVertexAttribArray( 1 );
    glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, sizeof( ve_fontcache_vertex ), (void*) 0 );
    glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, sizeof( ve_fontcache_vertex ), (void*) ( 2 * sizeof( float ) ) );

    glDisable( GL_CULL_FACE );
    glEnable( GL_BLEND );
    glBlendEquation( GL_FUNC_ADD );

    for ( ve_fontcache_draw& dcall : drawlist->dcalls )
    {
        if ( dcall.pass == VE_FONTCACHE_FRAMEBUFFER_PASS_GLYPH )
        {
            glUseProgram( gl->shader_render_glyph );
            glBindFramebuffer( GL_FRAMEBUFFER, gl->fbo[0] );
            glBlendFunc( GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_COLOR );
            glViewport( 0, 0, VE_FONTCACHE_GLYPHDRAW_BUFFER_WIDTH, VE_FONTCACHE_GLYPHDRAW_BUFFER_HEIGHT );
            glScissor( 0, 0, VE_FONTCACHE_GLYPHDRAW_BUFFER_WIDTH, VE_FONTCACHE_GLYPHDRAW_BUFFER_HEIGHT );
            glDisable( GL_FRAMEBUFFER_SRGB );
        }
        else if ( dcall.pass == VE_FONTCACHE_FRAMEBUFFER_PASS_ATLAS )
        {
            glUseProgram( gl->shader_blit_atlas );
            glBindFramebuffer( GL_FRAMEBUFFER, gl->fbo[1] );
            glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
            glViewport( 0, 0, VE_FONTCACHE_ATLAS_WIDTH, VE_FONTCACHE_ATLAS_HEIGHT );
            glScissor( 0, 0, VE_FONTCACHE_ATLAS_WIDTH, VE_FONTCACHE_ATLAS_HEIGHT );
            glUniform1i( glGetUniformLocation( gl->shader_blit_atlas, "src_texture" ), 0 );
            glUniform1ui( glGetUniformLocation( gl->shader_blit_atlas, "region" ), dcall.region );
            glActiveTexture( GL_TEXTURE0 );
            glBindTexture( GL_TEXTURE_2D, gl->fbo_texture[0] );
            glDisable( GL_FRAMEBUFFER_SRGB );
        }
        else if ( dcall.pass == VE_FONTCACHE_FRAMEBUFFER_PASS_TARGET || dcall.pass == VE_FONTCACHE_FRAMEBUFFER_PASS_TARGET_UNCACHED )
        {
            glUseProgram( gl->shader_draw_text );
            glBindFramebuffer( GL_FRAMEBUFFER, 0 );
            glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
            glViewport( 0, 0, win_w, win_h );
            glScissor( 0, 0, win_w, win_h );
            glUniform1i( glGetUniformLocation( gl->shader_draw_text, "src_texture" ), 0 );
            glUniform1ui( glGetUniformLocation( gl->shader_draw_text, "downsample" ), dcall.pass == VE_FONTCACHE_FRAMEBUFFER_PASS_TARGET_UNCACHED ? 1u : 0u );
            glActiveTexture( GL_TEXTURE0 );
            glBindTexture( GL_TEXTURE_2D, dcall.pass == VE_FONTCACHE_FRAMEBUFFER_PASS_TARGET_UNCACHED ? gl->fbo_texture[0] : gl->fbo_texture[1] );
            glUniform4fv( glGetUniformLocation( gl->shader_draw_text, "colour" ), 1, dcall.colour );
            glEnable( GL_FRAMEBUFFER_SRGB );
        }
        if ( dcall.clear_before_draw )
        {
            glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
            glClear( GL_COLOR_BUFFER_BIT );
        }
        if ( dcall.end_index - dcall.start_index == 0 ) continue;
        glDrawElements( GL_TRIANGLES, dcall.end_index - dcall.start_index, GL_UNSIGNED_INT, (GLvoid*) ( dcall.start_index * sizeof( uint32_t ) ) );
    }

    glDeleteBuffers( 1, &vbo );
    glDeleteBuffers( 1, &ibo );
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    glBindVertexArray( 0 );
    ve_fontcache_flush_drawlist( cache );
}

void vxui_gl_render( vxui_ctx* ctx, const vxui_draw_list& dl, float w, float h )
{
    assert( ctx && ctx->renderer );
    vxui_gl_state* gl = (vxui_gl_state*) ctx->renderer;

    vxui_gl_render_rects( gl, dl, w, h );

    if ( ctx->text )
    {
        vxui_text_state* st = (vxui_text_state*) ctx->text;
        vxui_gl_execute_text( gl, &st->cache, (int) w, (int) h );
    }
}

void vxui_gl_shutdown( vxui_ctx* ctx )
{
    if ( !ctx || !ctx->renderer ) return;
    vxui_gl_state* gl = (vxui_gl_state*) ctx->renderer;

    glDeleteBuffers( 1, &gl->rect_vbo );
    glDeleteBuffers( 1, &gl->rect_ibo );
    glDeleteVertexArrays( 1, &gl->rect_vao );
    glDeleteProgram( gl->rect_prog );

    glDeleteVertexArrays( 1, &gl->text_vao );
    glDeleteProgram( gl->shader_render_glyph );
    glDeleteProgram( gl->shader_blit_atlas );
    glDeleteProgram( gl->shader_draw_text );
    glDeleteFramebuffers( 2, gl->fbo );
    glDeleteTextures( 2, gl->fbo_texture );

    delete gl;
    ctx->renderer = nullptr;
}

#endif // VXUI_DEMO_RENDER_GL_IMPL
