// Demo GL renderer for VXUI. Reference only — not part of the library.
// #define VXUI_DEMO_RENDER_GL_IMPL in one TU.

#ifndef VXUI_DEMO_RENDER_GL_H
#define VXUI_DEMO_RENDER_GL_H

#include "vxui.h"
#include <glad/glad.h>

// Demo-renderer material contract. Caller writes one of these into
// vxui_render_data.material_id; the renderer dispatches accordingly.
#define DEMO_MATERIAL_CRT            1     // params: { time, intensity, curvature, aberration }
#define DEMO_MATERIAL_ROUND          2     // params: { radius_px, softness_px, _, _ }
#define DEMO_MATERIAL_IMAGE          3     // RGBA texture fills rect; texture_id + uv + colour as tint

#define DEMO_MATERIAL_FLAG_SCANLINES 1     // CRT: enable scanlines
#define DEMO_MATERIAL_FLAG_CURVE     2     // CRT: enable barrel curve

void     vxui_gl_init    ( vxui_ctx* ctx );
void     vxui_gl_render  ( vxui_ctx* ctx, const vxui_draw_list& dl, float w, float h );
void     vxui_gl_shutdown( vxui_ctx* ctx );
uint32_t vxui_gl_create_chevron_texture();
uint32_t vxui_gl_load_image( const char* path );

#endif // VXUI_DEMO_RENDER_GL_H

#ifdef VXUI_DEMO_RENDER_GL_IMPL

#include <cstdio>
#include <cassert>
#include "ve_fontcache.h"
#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO_DEPRECATED
#include "stb_image.h"

// Pass IDs share VEFC's dcall.pass uint32_t namespace; values >= 8 are demo-owned.
#define DEMO_GL_PASS_RECT          8
#define DEMO_GL_PASS_RECT_TEXTURED 9
#define DEMO_GL_PASS_CRT           10
#define DEMO_GL_PASS_ROUND         11
#define DEMO_GL_PASS_OUTLINE       12
#define DEMO_GL_PASS_IMAGE         13
#define DEMO_GL_MAX_MATERIAL_DCALLS 64

struct vxui_gl_material_data
{
    float    params[8];
    uint32_t flags;
};

struct vxui_gl_state
{
    GLuint vao                  = 0;
    GLuint shader_render_glyph  = 0;
    GLuint shader_blit_atlas    = 0;
    GLuint shader_draw_text     = 0;
    GLuint shader_rect          = 0;
    GLuint shader_rect_textured = 0;
    GLuint shader_image         = 0;
    GLuint shader_crt           = 0;
    GLuint shader_round         = 0;
    GLuint shader_outline       = 0;
    GLuint fbo[2]               = { 0, 0 };
    GLuint fbo_texture[2]       = { 0, 0 };

    vxui_gl_material_data material_data[DEMO_GL_MAX_MATERIAL_DCALLS] = {};
    int                   material_data_count                         = 0;
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

static const char* s_vxui_gl_image_fs = R"(#version 330 core
in vec2 uv;
out vec4 fragc;
uniform vec4      colour;
uniform sampler2D src_texture;
void main( void ) {
    vec4 t = texture( src_texture, uv );
    fragc = vec4( t.rgb * colour.rgb, t.a * colour.a );
}
)";

static const char* s_vxui_gl_crt_fs = R"(#version 330 core
in vec2 uv;
out vec4 fragc;
uniform vec4 colour;
uniform vec4 mparams;
uniform uint mflags;
void main( void ) {
    vec2 p = uv * 2.0 - 1.0;
    if ( ( mflags & 2u ) != 0u ) p *= 1.0 + mparams.z * dot( p, p );
    if ( abs( p.x ) > 1.0 || abs( p.y ) > 1.0 ) discard;
    vec2 puv = ( p + 1.0 ) * 0.5;

    float r = colour.r;
    float g = colour.g;
    float b = colour.b;
    if ( ( mflags & 2u ) != 0u ) {
        r *= 1.0 - mparams.w;
        b *= 1.0 + mparams.w;
    }

    float scan = 0.5 + 0.5 * sin( puv.y * 800.0 + mparams.x * 6.0 );
    float v    = ( ( mflags & 1u ) != 0u ) ? mix( 1.0, scan, mparams.y ) : 1.0;

    fragc = vec4( r * v, g * v, b * v, colour.a );
}
)";

// SDF rounded box. mparams = { radius_px, width_px, height_px, softness_px }.
// d = sdRoundBox(p_px, halfsize - r) - r  (Inigo Quilez 2D rounded box SDF).
static const char* s_vxui_gl_round_fs = R"(#version 330 core
in vec2 uv;
out vec4 fragc;
uniform vec4 colour;
uniform vec4 mparams;
void main( void ) {
    vec2  half_size = vec2( mparams.y, mparams.z ) * 0.5;
    vec2  p_px      = ( uv * 2.0 - 1.0 ) * half_size;
    float r         = mparams.x;
    vec2  q         = abs( p_px ) - half_size + vec2( r );
    float d         = length( max( q, vec2( 0.0 ) ) ) + min( max( q.x, q.y ), 0.0 ) - r;
    float a         = 1.0 - smoothstep( 0.0, mparams.w, d );
    fragc = vec4( colour.xyz, colour.a * a );
}
)";

// SDF inside-the-rect outline. mparams = { thickness_px, width_px, height_px, softness_px }.
// d_box is signed distance to the box edge (negative inside). Visible band lives
// in d_box ∈ [-t, 0] with smoothstep edges of width softness.
static const char* s_vxui_gl_outline_fs = R"(#version 330 core
in vec2 uv;
out vec4 fragc;
uniform vec4 colour;
uniform vec4 mparams;
void main( void ) {
    vec2  half_size = vec2( mparams.y, mparams.z ) * 0.5;
    vec2  p_px      = ( uv * 2.0 - 1.0 ) * half_size;
    vec2  q         = abs( p_px ) - half_size;
    float d         = length( max( q, vec2( 0.0 ) ) ) + min( max( q.x, q.y ), 0.0 );
    float t         = mparams.x;
    float s         = mparams.w;
    float a_outer   = smoothstep( -t - s, -t + s, d );
    float a_inner   = 1.0 - smoothstep( -s, s, d );
    fragc = vec4( colour.xyz, colour.a * a_outer * a_inner );
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

uint32_t vxui_gl_load_image( const char* path )
{
    int w = 0, h = 0, comp = 0;
    stbi_uc* pixels = stbi_load( path, &w, &h, &comp, 4 );
    if ( !pixels )
    {
        fprintf( stderr, "vxui_gl_load_image: failed to load %s\n", path );
        return 0;
    }

    GLuint tex = 0;
    glGenTextures( 1, &tex );
    glBindTexture( GL_TEXTURE_2D, tex );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glBindTexture( GL_TEXTURE_2D, 0 );
    stbi_image_free( pixels );
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
    gl->shader_image         = vxui_gl_compile( s_vxui_gl_vefc_vs_draw_text, s_vxui_gl_image_fs             );
    gl->shader_crt           = vxui_gl_compile( s_vxui_gl_vefc_vs_draw_text, s_vxui_gl_crt_fs               );
    gl->shader_round         = vxui_gl_compile( s_vxui_gl_vefc_vs_draw_text, s_vxui_gl_round_fs             );
    gl->shader_outline       = vxui_gl_compile( s_vxui_gl_vefc_vs_draw_text, s_vxui_gl_outline_fs           );

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
    dcall.pass        = DEMO_GL_PASS_RECT;
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
    dcall.pass        = DEMO_GL_PASS_RECT_TEXTURED;
    dcall.start_index = idx_start;
    dcall.end_index   = (uint32_t) dl->indices.size();
    dcall.colour[0]   = colour.x;
    dcall.colour[1]   = colour.y;
    dcall.colour[2]   = colour.z;
    dcall.colour[3]   = colour.w;
    dcall.region      = texture_id;
    dl->dcalls.push_back( dcall );
}

static void vxui_gl_push_image_quad( ve_fontcache_drawlist* dl, float x0, float y0, float x1, float y1, const glm::vec4& uv, uint32_t texture_id, const glm::vec4& colour )
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
    dcall.pass        = DEMO_GL_PASS_IMAGE;
    dcall.start_index = idx_start;
    dcall.end_index   = (uint32_t) dl->indices.size();
    dcall.colour[0]   = colour.x;
    dcall.colour[1]   = colour.y;
    dcall.colour[2]   = colour.z;
    dcall.colour[3]   = colour.w;
    dcall.region      = texture_id;
    dl->dcalls.push_back( dcall );
}

// Material-driven push: single quad, dcall.region indexes into material_data[].
// All material passes share this shape; only pass id and material_data semantics differ.
static void vxui_gl_push_material_quad( ve_fontcache_drawlist* dl, vxui_gl_state* gl, uint32_t pass,
                                        float x0, float y0, float x1, float y1,
                                        const float params[8], uint32_t flags,
                                        const glm::vec4& colour )
{
    uint32_t base = (uint32_t) dl->vertices.size();
    dl->vertices.push_back( { x0, y0, 0.0f, 0.0f } );
    dl->vertices.push_back( { x1, y0, 1.0f, 0.0f } );
    dl->vertices.push_back( { x1, y1, 1.0f, 1.0f } );
    dl->vertices.push_back( { x0, y1, 0.0f, 1.0f } );

    uint32_t idx_start = (uint32_t) dl->indices.size();
    dl->indices.push_back( base + 0 );
    dl->indices.push_back( base + 1 );
    dl->indices.push_back( base + 2 );
    dl->indices.push_back( base + 0 );
    dl->indices.push_back( base + 2 );
    dl->indices.push_back( base + 3 );

    assert( gl->material_data_count < DEMO_GL_MAX_MATERIAL_DCALLS );
    int mat_idx = gl->material_data_count++;
    memcpy( gl->material_data[mat_idx].params, params, sizeof( gl->material_data[mat_idx].params ) );
    gl->material_data[mat_idx].flags = flags;

    ve_fontcache_draw dcall;
    dcall.pass        = pass;
    dcall.start_index = idx_start;
    dcall.end_index   = (uint32_t) dl->indices.size();
    dcall.colour[0]   = colour.x;
    dcall.colour[1]   = colour.y;
    dcall.colour[2]   = colour.z;
    dcall.colour[3]   = colour.w;
    dcall.region      = (uint32_t) mat_idx;
    dl->dcalls.push_back( dcall );
}

static void vxui_gl_emit_one_rect( ve_fontcache_drawlist* fdl, vxui_gl_state* gl, const vxui_draw_cmd* c, float px, float py, float fb_w, float fb_h )
{
    float x0 = px / fb_w;
    float x1 = ( px + c->rect.z ) / fb_w;
    float y0 = 1.0f - py / fb_h;
    float y1 = 1.0f - ( py + c->rect.w ) / fb_h;

    switch ( c->render.material_id )
    {
        case 0:
            vxui_gl_push_solid_quad( fdl, x0, y0, x1, y1, c->render.colour );
            break;
        case DEMO_MATERIAL_CRT:
            // Caller-supplied params pass through unchanged: time/intensity/curve/aberration.
            vxui_gl_push_material_quad( fdl, gl, DEMO_GL_PASS_CRT, x0, y0, x1, y1,
                                        c->render.params, c->render.flags, c->render.colour );
            break;
        case DEMO_MATERIAL_ROUND:
        {
            // Round material wants pixel-space mparams; rebuild from caller's
            // params[0]=radius, params[1]=softness plus the cmd's rect size.
            float mp[8] = {};
            mp[0] = c->render.params[0];
            mp[1] = c->rect.z;
            mp[2] = c->rect.w;
            mp[3] = c->render.params[1];
            vxui_gl_push_material_quad( fdl, gl, DEMO_GL_PASS_ROUND, x0, y0, x1, y1,
                                        mp, c->render.flags, c->render.colour );
            break;
        }
        case DEMO_MATERIAL_IMAGE:
        {
            glm::vec4 uv = c->render.uv;
            if ( uv.z <= 0.0f && uv.w <= 0.0f ) uv = { 0.0f, 0.0f, 1.0f, 1.0f };
            glm::vec4 tint = c->render.colour;
            if ( tint.r == 0 && tint.g == 0 && tint.b == 0 && tint.a == 0 ) tint = { 1.0f, 1.0f, 1.0f, 1.0f };
            vxui_gl_push_image_quad( fdl, x0, y0, x1, y1, uv, c->render.texture_id, tint );
            break;
        }
        default:
            assert( !"unknown material_id" );
            vxui_gl_push_solid_quad( fdl, x0, y0, x1, y1, c->render.colour );
            break;
    }

    if ( c->render.outline_thickness > 0.0f )
    {
        float mp[8] = {};
        mp[0] = c->render.outline_thickness;
        mp[1] = c->rect.z;
        mp[2] = c->rect.w;
        mp[3] = 1.0f;     // softness; demo-fixed, not a render_data field
        vxui_gl_push_material_quad( fdl, gl, DEMO_GL_PASS_OUTLINE, x0, y0, x1, y1,
                                    mp, 0u, c->render.outline_colour );
    }

    if ( c->render.texture_id != 0 && c->render.material_id != DEMO_MATERIAL_IMAGE )
    {
        // Left-side marker. The row's left padding reserves space for this so
        // the chevron sits over a clear area rather than over the value text
        // which lives on the right side of option/slider rows.
        float icon_h = c->rect.w;
        float icon_w = icon_h;
        float icon_px = px + 4.0f;
        float icon_py = py;
        float ix0 = icon_px / fb_w;
        float ix1 = ( icon_px + icon_w ) / fb_w;
        float iy0 = 1.0f - icon_py / fb_h;
        float iy1 = 1.0f - ( icon_py + icon_h ) / fb_h;
        vxui_gl_push_textured_quad( fdl, ix0, iy0, ix1, iy1, c->render.uv, c->render.texture_id, c->render.colour );
    }
}

static void vxui_gl_emit_rects( ve_fontcache_drawlist* fdl, vxui_gl_state* gl, const vxui_draw_list& dl, float w, float h )
{
    int rect_n = vxui_draw_count( dl, VXUI_DRAW_RECT );
    if ( rect_n <= 0 ) return;

    for ( int i = 0; i < rect_n; i++ )
    {
        const vxui_draw_cmd* c = vxui_draw_nth( dl, VXUI_DRAW_RECT, i );
        if ( c->state & VXUI_DRAW_FOCUSED ) continue;
        vxui_gl_emit_one_rect( fdl, gl, c, c->rect.x, c->rect.y, w, h );
    }

    for ( int i = 0; i < rect_n; i++ )
    {
        const vxui_draw_cmd* c = vxui_draw_nth( dl, VXUI_DRAW_RECT, i );
        if ( !( c->state & VXUI_DRAW_FOCUSED ) ) continue;
        vxui_gl_emit_one_rect( fdl, gl, c, c->rect.x, c->rect.y + c->focus_offset_y, w, h );
    }
}

static void vxui_gl_emit_text( vxui_text_state* st, const vxui_draw_list& dl, float w, float h )
{
    int text_n = vxui_draw_count( dl, VXUI_DRAW_TEXT );
    if ( text_n <= 0 ) return;

    ve_fontcache_configure_snap( &st->cache, (unsigned) w, (unsigned) h );

    float inv_w = 1.0f / w;
    float inv_h = 1.0f / h;

    for ( int i = 0; i < text_n; i++ )
    {
        const vxui_draw_cmd* c = vxui_draw_nth( dl, VXUI_DRAW_TEXT, i );
        ve_font_id font = ( c->font != 0 ) ? (ve_font_id) c->font : st->default_font;
        if ( font < 0 || c->text_len <= 0 ) continue;

        glm::vec4 col = c->render.colour;
        if ( col.a <= 0.0f ) col = { 0.95f, 0.95f, 0.95f, 1.0f };
        float text_colour[4] = { col.r, col.g, col.b, col.a };
        ve_fontcache_set_colour   ( &st->cache, text_colour );
        ve_fontcache_set_font_size( &st->cache, font, (float) c->font_px );

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
        else if ( dcall.pass == DEMO_GL_PASS_RECT )
        {
            glUseProgram( gl->shader_rect );
            glBindFramebuffer( GL_FRAMEBUFFER, 0 );
            glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
            glViewport( 0, 0, win_w, win_h );
            glScissor( 0, 0, win_w, win_h );
            glUniform4fv( glGetUniformLocation( gl->shader_rect, "colour" ), 1, dcall.colour );
            glEnable( GL_FRAMEBUFFER_SRGB );
        }
        else if ( dcall.pass == DEMO_GL_PASS_RECT_TEXTURED )
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
        else if ( dcall.pass == DEMO_GL_PASS_IMAGE )
        {
            glUseProgram( gl->shader_image );
            glBindFramebuffer( GL_FRAMEBUFFER, 0 );
            glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
            glViewport( 0, 0, win_w, win_h );
            glScissor( 0, 0, win_w, win_h );
            glUniform4fv( glGetUniformLocation( gl->shader_image, "colour" ), 1, dcall.colour );
            glUniform1i( glGetUniformLocation( gl->shader_image, "src_texture" ), 0 );
            glActiveTexture( GL_TEXTURE0 );
            glBindTexture( GL_TEXTURE_2D, (GLuint) dcall.region );
            glEnable( GL_FRAMEBUFFER_SRGB );
        }
        else if ( dcall.pass == DEMO_GL_PASS_CRT )
        {
            glUseProgram( gl->shader_crt );
            glBindFramebuffer( GL_FRAMEBUFFER, 0 );
            glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
            glViewport( 0, 0, win_w, win_h );
            glScissor( 0, 0, win_w, win_h );
            glUniform4fv( glGetUniformLocation( gl->shader_crt, "colour" ), 1, dcall.colour );
            int mi = (int) dcall.region;
            assert( mi >= 0 && mi < gl->material_data_count );
            glUniform4fv( glGetUniformLocation( gl->shader_crt, "mparams" ), 1, gl->material_data[mi].params );
            glUniform1ui( glGetUniformLocation( gl->shader_crt, "mflags" ), gl->material_data[mi].flags );
            glEnable( GL_FRAMEBUFFER_SRGB );
        }
        else if ( dcall.pass == DEMO_GL_PASS_ROUND )
        {
            glUseProgram( gl->shader_round );
            glBindFramebuffer( GL_FRAMEBUFFER, 0 );
            glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
            glViewport( 0, 0, win_w, win_h );
            glScissor( 0, 0, win_w, win_h );
            glUniform4fv( glGetUniformLocation( gl->shader_round, "colour" ), 1, dcall.colour );
            int mi = (int) dcall.region;
            assert( mi >= 0 && mi < gl->material_data_count );
            glUniform4fv( glGetUniformLocation( gl->shader_round, "mparams" ), 1, gl->material_data[mi].params );
            glEnable( GL_FRAMEBUFFER_SRGB );
        }
        else if ( dcall.pass == DEMO_GL_PASS_OUTLINE )
        {
            glUseProgram( gl->shader_outline );
            glBindFramebuffer( GL_FRAMEBUFFER, 0 );
            glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
            glViewport( 0, 0, win_w, win_h );
            glScissor( 0, 0, win_w, win_h );
            glUniform4fv( glGetUniformLocation( gl->shader_outline, "colour" ), 1, dcall.colour );
            int mi = (int) dcall.region;
            assert( mi >= 0 && mi < gl->material_data_count );
            glUniform4fv( glGetUniformLocation( gl->shader_outline, "mparams" ), 1, gl->material_data[mi].params );
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

    gl->material_data_count = 0;
    vxui_gl_emit_rects( fdl, gl, dl, w, h );
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
    glDeleteProgram( gl->shader_image );
    glDeleteProgram( gl->shader_crt );
    glDeleteProgram( gl->shader_round );
    glDeleteProgram( gl->shader_outline );
    glDeleteFramebuffers( 2, gl->fbo );
    glDeleteTextures( 2, gl->fbo_texture );

    delete gl;
    ctx->renderer = nullptr;
}

#endif // VXUI_DEMO_RENDER_GL_IMPL
