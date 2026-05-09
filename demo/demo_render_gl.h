// Demo GL renderer for VXUI. Reference only — not part of the library.
// #define VXUI_DEMO_RENDER_GL_IMPL in one TU.

#ifndef VXUI_DEMO_RENDER_GL_H
#define VXUI_DEMO_RENDER_GL_H

#include "vxui.h"
#include <glad/glad.h>

void     vxui_gl_init    ( vxui_ctx* ctx );
void     vxui_gl_render  ( vxui_ctx* ctx, const vxui_draw_list& dl, float w, float h );
void     vxui_gl_shutdown( vxui_ctx* ctx );
uint32_t vxui_gl_create_chevron_texture();

#endif // VXUI_DEMO_RENDER_GL_H

#ifdef VXUI_DEMO_RENDER_GL_IMPL

#include <cstdio>
#include <cassert>
#include "ve_fontcache.h"

#define VXUI_GL_PASS_RECT          8
#define VXUI_GL_PASS_RECT_TEXTURED 9

struct vxui_gl_state
{
    GLuint vao                  = 0;
    GLuint shader_render_glyph  = 0;
    GLuint shader_blit_atlas    = 0;
    GLuint shader_draw_text     = 0;
    GLuint shader_rect          = 0;
    GLuint shader_rect_textured = 0;
    GLuint fbo[2]               = { 0, 0 };
    GLuint fbo_texture[2]       = { 0, 0 };
};

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

// Shares vs_draw_text geometry layout (normalized vpos, * 2 - 1 to NDC).
// vtex is unused for solid quads but the attrib stays bound for layout match.
static const char* s_vxui_gl_rect_fs = R"(#version 330 core
in vec2 uv;
out vec4 fragc;
uniform vec4 colour;
void main( void ) {
    fragc = colour;
}
)";

static const char* s_vxui_gl_rect_textured_fs = R"(#version 330 core
in vec2 uv;
out vec4 fragc;
uniform vec4      colour;
uniform sampler2D src_texture;
void main( void ) {
    float a = texture( src_texture, uv ).x;
    fragc = vec4( colour.xyz, colour.a * a );
}
)";

static void vxui_gl_check_error( const char* tag )
{
#ifdef VXUI_DEBUG
    GLenum err = glGetError();
    if ( err != GL_NO_ERROR )
    {
        const char* name = "GL_???";
        switch ( err )
        {
            case GL_INVALID_ENUM:                  name = "GL_INVALID_ENUM";                  break;
            case GL_INVALID_VALUE:                 name = "GL_INVALID_VALUE";                 break;
            case GL_INVALID_OPERATION:             name = "GL_INVALID_OPERATION";             break;
            case GL_OUT_OF_MEMORY:                 name = "GL_OUT_OF_MEMORY";                 break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: name = "GL_INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        fprintf( stderr, "vxui_gl: %s at %s\n", name, tag );
        assert( !"vxui_gl error" );
    }
#else
    (void) tag;
#endif
}

static GLuint vxui_gl_compile( const char* vs_src, const char* fs_src )
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
    glBindAttribLocation( p, 1, "vtex" );
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

uint32_t vxui_gl_create_chevron_texture()
{
    uint8_t pixels[32 * 32] = {};
    for ( int y = 0; y < 32; y++ )
    {
        int tip_x = 22 - ( y < 16 ? y : 31 - y );
        int base_x = tip_x - 6;
        if ( base_x < 0 ) base_x = 0;
        for ( int x = base_x; x <= tip_x && x < 32; x++ )
            pixels[y * 32 + x] = 255;
    }

    GLuint tex = 0;
    glGenTextures( 1, &tex );
    glBindTexture( GL_TEXTURE_2D, tex );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_R8, 32, 32, 0, GL_RED, GL_UNSIGNED_BYTE, pixels );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glBindTexture( GL_TEXTURE_2D, 0 );
    return (uint32_t) tex;
}

void vxui_gl_init( vxui_ctx* ctx )
{
    assert( ctx );
    assert( !ctx->renderer && "vxui_gl_init called twice" );

    vxui_gl_state* gl = new vxui_gl_state();
    ctx->renderer = gl;

    gl->shader_render_glyph  = vxui_gl_compile( s_vxui_gl_vefc_vs_shared,    s_vxui_gl_vefc_fs_render_glyph );
    gl->shader_blit_atlas    = vxui_gl_compile( s_vxui_gl_vefc_vs_shared,    s_vxui_gl_vefc_fs_blit_atlas   );
    gl->shader_draw_text     = vxui_gl_compile( s_vxui_gl_vefc_vs_draw_text, s_vxui_gl_vefc_fs_draw_text    );
    gl->shader_rect          = vxui_gl_compile( s_vxui_gl_vefc_vs_draw_text, s_vxui_gl_rect_fs              );
    gl->shader_rect_textured = vxui_gl_compile( s_vxui_gl_vefc_vs_draw_text, s_vxui_gl_rect_textured_fs     );

    glGenVertexArrays( 1, &gl->vao );
    vxui_gl_setup_fbo( gl );
}

static void vxui_gl_push_solid_quad( ve_fontcache_drawlist* dl, float x0, float y0, float x1, float y1, const glm::vec4& colour )
{
    uint32_t base = (uint32_t) dl->vertices.size();
    dl->vertices.push_back( { x0, y0, 0.0f, 0.0f } );
    dl->vertices.push_back( { x1, y0, 0.0f, 0.0f } );
    dl->vertices.push_back( { x1, y1, 0.0f, 0.0f } );
    dl->vertices.push_back( { x0, y1, 0.0f, 0.0f } );

    uint32_t idx_start = (uint32_t) dl->indices.size();
    dl->indices.push_back( base + 0 );
    dl->indices.push_back( base + 1 );
    dl->indices.push_back( base + 2 );
    dl->indices.push_back( base + 0 );
    dl->indices.push_back( base + 2 );
    dl->indices.push_back( base + 3 );

    ve_fontcache_draw dcall;
    dcall.pass        = VXUI_GL_PASS_RECT;
    dcall.start_index = idx_start;
    dcall.end_index   = (uint32_t) dl->indices.size();
    dcall.colour[0]   = colour.x;
    dcall.colour[1]   = colour.y;
    dcall.colour[2]   = colour.z;
    dcall.colour[3]   = colour.w;
    dl->dcalls.push_back( dcall );
}

static void vxui_gl_push_textured_quad( ve_fontcache_drawlist* dl, float x0, float y0, float x1, float y1, const glm::vec4& uv, uint32_t texture_id, const glm::vec4& colour )
{
    float u0 = uv.x, v0 = uv.y, u1 = uv.z, v1 = uv.w;

    uint32_t base = (uint32_t) dl->vertices.size();
    dl->vertices.push_back( { x0, y0, u0, v0 } );
    dl->vertices.push_back( { x1, y0, u1, v0 } );
    dl->vertices.push_back( { x1, y1, u1, v1 } );
    dl->vertices.push_back( { x0, y1, u0, v1 } );

    uint32_t idx_start = (uint32_t) dl->indices.size();
    dl->indices.push_back( base + 0 );
    dl->indices.push_back( base + 1 );
    dl->indices.push_back( base + 2 );
    dl->indices.push_back( base + 0 );
    dl->indices.push_back( base + 2 );
    dl->indices.push_back( base + 3 );

    ve_fontcache_draw dcall;
    dcall.pass        = VXUI_GL_PASS_RECT_TEXTURED;
    dcall.start_index = idx_start;
    dcall.end_index   = (uint32_t) dl->indices.size();
    dcall.colour[0]   = colour.x;
    dcall.colour[1]   = colour.y;
    dcall.colour[2]   = colour.z;
    dcall.colour[3]   = colour.w;
    dcall.region      = texture_id;
    dl->dcalls.push_back( dcall );
}

static void vxui_gl_emit_one_rect( ve_fontcache_drawlist* fdl, const vxui_draw_cmd* c, float px, float py, float fb_w, float fb_h )
{
    float x0 = px / fb_w;
    float x1 = ( px + c->rect.z ) / fb_w;
    float y0 = 1.0f - py / fb_h;
    float y1 = 1.0f - ( py + c->rect.w ) / fb_h;

    vxui_gl_push_solid_quad( fdl, x0, y0, x1, y1, c->render.colour );

    if ( c->render.outline_thickness > 0.0f )
    {
        float t = c->render.outline_thickness;
        float tx = t / fb_w;
        float ty = t / fb_h;
        const glm::vec4& oc = c->render.outline_colour;
        vxui_gl_push_solid_quad( fdl, x0,      y0,      x1,      y0 + ty, oc );
        vxui_gl_push_solid_quad( fdl, x0,      y1 - ty, x1,      y1,      oc );
        vxui_gl_push_solid_quad( fdl, x0,      y0 + ty, x0 + tx, y1 - ty, oc );
        vxui_gl_push_solid_quad( fdl, x1 - tx, y0 + ty, x1,      y1 - ty, oc );
    }

    if ( c->render.texture_id != 0 )
    {
        float icon_h = c->rect.w;
        float icon_w = icon_h;
        float icon_px = px + c->rect.z - icon_w - 6.0f;
        float icon_py = py;
        float ix0 = icon_px / fb_w;
        float ix1 = ( icon_px + icon_w ) / fb_w;
        float iy0 = 1.0f - icon_py / fb_h;
        float iy1 = 1.0f - ( icon_py + icon_h ) / fb_h;
        vxui_gl_push_textured_quad( fdl, ix0, iy0, ix1, iy1, c->render.uv, c->render.texture_id, c->render.colour );
    }
}

static void vxui_gl_emit_rects( ve_fontcache_drawlist* fdl, const vxui_draw_list& dl, float w, float h )
{
    int rect_n = vxui_draw_count( dl, VXUI_DRAW_RECT );
    if ( rect_n <= 0 ) return;

    for ( int i = 0; i < rect_n; i++ )
    {
        const vxui_draw_cmd* c = vxui_draw_nth( dl, VXUI_DRAW_RECT, i );
        if ( c->state & VXUI_DRAW_FOCUSED ) continue;
        vxui_gl_emit_one_rect( fdl, c, c->rect.x, c->rect.y, w, h );
    }

    for ( int i = 0; i < rect_n; i++ )
    {
        const vxui_draw_cmd* c = vxui_draw_nth( dl, VXUI_DRAW_RECT, i );
        if ( !( c->state & VXUI_DRAW_FOCUSED ) ) continue;
        vxui_gl_emit_one_rect( fdl, c, c->rect.x, c->rect.y + c->focus_offset_y, w, h );
    }
}

static void vxui_gl_emit_text( vxui_text_state* st, const vxui_draw_list& dl, float w, float h )
{
    int text_n = vxui_draw_count( dl, VXUI_DRAW_TEXT );
    if ( text_n <= 0 ) return;

    ve_fontcache_configure_snap( &st->cache, (unsigned) w, (unsigned) h );
    float text_colour[4] = { 0.95f, 0.95f, 0.95f, 1.0f };
    ve_fontcache_set_colour( &st->cache, text_colour );

    float inv_w = 1.0f / w;
    float inv_h = 1.0f / h;

    for ( int i = 0; i < text_n; i++ )
    {
        const vxui_draw_cmd* c = vxui_draw_nth( dl, VXUI_DRAW_TEXT, i );
        ve_font_id font = ( c->font != 0 ) ? (ve_font_id) c->font : st->default_font;
        if ( font < 0 || c->text_len <= 0 ) continue;

        std::u8string text( (const char8_t*) c->text, (size_t) c->text_len );
        float posx = c->rect.x * inv_w;
        float posy = 1.0f - ( c->rect.y + c->font_px ) * inv_h;   // VEFC pen = baseline, y-up
        ve_fontcache_draw_text( &st->cache, font, text, posx, posy, inv_w, inv_h );
    }
}

static void vxui_gl_execute( vxui_gl_state* gl, ve_fontcache* cache, int win_w, int win_h )
{
    ve_fontcache_optimise_drawlist( cache );
    ve_fontcache_drawlist* drawlist = ve_fontcache_get_drawlist( cache );
    if ( drawlist->dcalls.empty() )
    {
        ve_fontcache_flush_drawlist( cache );
        return;
    }

    glBindVertexArray( gl->vao );
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
        else if ( dcall.pass == VXUI_GL_PASS_RECT )
        {
            glUseProgram( gl->shader_rect );
            glBindFramebuffer( GL_FRAMEBUFFER, 0 );
            glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
            glViewport( 0, 0, win_w, win_h );
            glScissor( 0, 0, win_w, win_h );
            glUniform4fv( glGetUniformLocation( gl->shader_rect, "colour" ), 1, dcall.colour );
            glEnable( GL_FRAMEBUFFER_SRGB );
        }
        else if ( dcall.pass == VXUI_GL_PASS_RECT_TEXTURED )
        {
            glUseProgram( gl->shader_rect_textured );
            glBindFramebuffer( GL_FRAMEBUFFER, 0 );
            glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
            glViewport( 0, 0, win_w, win_h );
            glScissor( 0, 0, win_w, win_h );
            glUniform4fv( glGetUniformLocation( gl->shader_rect_textured, "colour" ), 1, dcall.colour );
            glUniform1i( glGetUniformLocation( gl->shader_rect_textured, "src_texture" ), 0 );
            glActiveTexture( GL_TEXTURE0 );
            glBindTexture( GL_TEXTURE_2D, (GLuint) dcall.region );
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
    vxui_gl_check_error( "execute" );
}

void vxui_gl_render( vxui_ctx* ctx, const vxui_draw_list& dl, float w, float h )
{
    assert( ctx && ctx->renderer );
    vxui_gl_state* gl = (vxui_gl_state*) ctx->renderer;
    if ( !ctx->text ) return;

    vxui_text_state* st = (vxui_text_state*) ctx->text;
    ve_fontcache_drawlist* fdl = ve_fontcache_get_drawlist( &st->cache );

    vxui_gl_emit_rects( fdl, dl, w, h );
    vxui_gl_emit_text ( st, dl, w, h );
    vxui_gl_execute   ( gl, &st->cache, (int) w, (int) h );
}

void vxui_gl_shutdown( vxui_ctx* ctx )
{
    if ( !ctx || !ctx->renderer ) return;
    vxui_gl_state* gl = (vxui_gl_state*) ctx->renderer;

    glDeleteVertexArrays( 1, &gl->vao );
    glDeleteProgram( gl->shader_render_glyph );
    glDeleteProgram( gl->shader_blit_atlas );
    glDeleteProgram( gl->shader_draw_text );
    glDeleteProgram( gl->shader_rect );
    glDeleteProgram( gl->shader_rect_textured );
    glDeleteFramebuffers( 2, gl->fbo );
    glDeleteTextures( 2, gl->fbo_texture );

    delete gl;
    ctx->renderer = nullptr;
}

#endif // VXUI_DEMO_RENDER_GL_IMPL
