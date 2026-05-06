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

struct vxui_gl_state
{
    GLuint vao        = 0;
    GLuint vbo        = 0;
    GLuint ibo        = 0;
    GLuint program    = 0;
    GLint  loc_colour = -1;
    GLint  loc_screen = -1;
};

static const char* s_vxui_gl_vs = R"(#version 330 core
in vec2 vpos;
uniform vec2 screen;
void main( void )
{
    vec2 ndc = vpos / screen * 2.0 - 1.0;
    gl_Position = vec4( ndc.x, -ndc.y, 0.0, 1.0 );
}
)";

static const char* s_vxui_gl_fs = R"(#version 330 core
out vec4 fragc;
uniform vec4 colour;
void main( void )
{
    fragc = colour;
}
)";

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
    glLinkProgram( p );
    glGetProgramiv( p, GL_LINK_STATUS, &ok );
    if ( !ok ) { glGetProgramInfoLog( p, sizeof( log ), nullptr, log ); fprintf( stderr, "vxui_gl: link: %s\n", log ); }

    glDeleteShader( vs );
    glDeleteShader( fs );
    return p;
}

void vxui_gl_init( vxui_ctx* ctx )
{
    assert( ctx );
    assert( !ctx->renderer && "vxui_gl_init called twice" );

    vxui_gl_state* gl = new vxui_gl_state();
    ctx->renderer = gl;

    gl->program    = vxui_gl_compile( s_vxui_gl_vs, s_vxui_gl_fs );
    gl->loc_colour = glGetUniformLocation( gl->program, "colour" );
    gl->loc_screen = glGetUniformLocation( gl->program, "screen" );

    glGenVertexArrays( 1, &gl->vao );
    glGenBuffers( 1, &gl->vbo );
    glGenBuffers( 1, &gl->ibo );

    glBindVertexArray( gl->vao );
    glBindBuffer( GL_ARRAY_BUFFER, gl->vbo );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, gl->ibo );
    glEnableVertexAttribArray( 0 );
    glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof( float ), (void*) 0 );
    glBindVertexArray( 0 );
}

static void vxui_gl_push_quad( std::vector< float >& verts, std::vector< uint32_t >& idx,
                               float x, float y, float w, float h )
{
    uint32_t base = (uint32_t) ( verts.size() / 2 );
    verts.insert( verts.end(), { x, y,  x + w, y,  x + w, y + h,  x, y + h } );
    idx.insert( idx.end(), { base, base + 1, base + 2,  base, base + 2, base + 3 } );
}

void vxui_gl_render( vxui_ctx* ctx, const vxui_draw_list& dl, float w, float h )
{
    assert( ctx && ctx->renderer );
    vxui_gl_state* gl = (vxui_gl_state*) ctx->renderer;

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

    glBindVertexArray( gl->vao );
    glBindBuffer( GL_ARRAY_BUFFER, gl->vbo );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, gl->ibo );
    glBufferData( GL_ARRAY_BUFFER, (GLsizeiptr)( verts.size() * sizeof( float ) ), verts.data(), GL_DYNAMIC_DRAW );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)( idx.size() * sizeof( uint32_t ) ), idx.data(), GL_DYNAMIC_DRAW );

    glUseProgram( gl->program );
    glUniform2f( gl->loc_screen, w, h );

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    if ( row_idx_count > 0 )
    {
        glUniform4f( gl->loc_colour, 0.15f, 0.15f, 0.18f, 1.0f );
        glDrawElements( GL_TRIANGLES, row_idx_count, GL_UNSIGNED_INT, (void*) 0 );
    }
    if ( focus_idx_count > 0 )
    {
        glUniform4f( gl->loc_colour, 0.30f, 0.55f, 0.85f, 1.0f );
        glDrawElements( GL_TRIANGLES, focus_idx_count, GL_UNSIGNED_INT, (void*) ( (size_t) focus_idx_start * sizeof( uint32_t ) ) );
    }

    glBindVertexArray( 0 );
}

void vxui_gl_shutdown( vxui_ctx* ctx )
{
    if ( !ctx || !ctx->renderer ) return;
    vxui_gl_state* gl = (vxui_gl_state*) ctx->renderer;
    glDeleteBuffers( 1, &gl->vbo );
    glDeleteBuffers( 1, &gl->ibo );
    glDeleteVertexArrays( 1, &gl->vao );
    glDeleteProgram( gl->program );
    delete gl;
    ctx->renderer = nullptr;
}

#endif // VXUI_DEMO_RENDER_GL_IMPL
