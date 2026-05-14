
/*
	-- VXUI Motion UI --

	Copyright 2026 Xi Chen

	Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
	associated documentation files (the "Software"), to deal in the Software without restriction,
	including without limitation the rights to use, copy, modify, merge, publish, distribute,
	sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all copies or substantial
	portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
	NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
	NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES
   OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include "vxui.h"
#include <glm/glm.hpp>

#define VXUI_HUD_MAX_ITEMS      256
#define VXUI_HUD_MAX_TEXT_BYTES 4096

#define VXUI_HUD_STOCK_ROW     0
#define VXUI_HUD_STOCK_COL     1
#define VXUI_HUD_STOCK_GRID4   2
#define VXUI_HUD_STOCK_GRID2   3
#define VXUI_HUD_STOCK_HEX_COL 4
#define VXUI_HUD_STOCK_HEX_ROW 5

struct vxui_hud_item
{
    uint32_t  id         = 0;
    uint32_t  texture_id = 0;
    int16_t   z          = 0;
    glm::vec4 colour     = { 1.0f, 1.0f, 1.0f, 1.0f };
    glm::vec4 uv         = { 0.0f, 0.0f, 1.0f, 1.0f };
};

struct vxui_hud
{
    vxui_ctx*    ctx     = nullptr;
    float        x       = 0.0f;
    float        y       = 0.0f;
    float        w       = 0.0f;
    float        h       = 0.0f;
    glm::vec4    colour  = { 1.0f, 1.0f, 1.0f, 1.0f };
    vxui_font_id font    = 0;
    uint16_t     font_px = VXUI_FONT_SIZE_DEFAULT;
    int16_t      z       = 0;

    vxui_hud_item items[VXUI_HUD_MAX_ITEMS] = {};
    int            item_count               = 0;

    char text_buf[VXUI_HUD_MAX_TEXT_BYTES] = {};
    int  text_offset                       = 0;
};

void      vxui_hud_begin         ( vxui_hud* hud, vxui_ctx* ctx, float w, float h );
void      vxui_hud_set_pos       ( vxui_hud* hud, float x, float y );
void      vxui_hud_move_pos      ( vxui_hud* hud, float x, float y );
void      vxui_hud_set_colour    ( vxui_hud* hud, glm::vec4 colour );
void      vxui_hud_set_font      ( vxui_hud* hud, vxui_font_id font, uint16_t px );
void      vxui_hud_set_z         ( vxui_hud* hud, int16_t z );
uint32_t  vxui_hud_id            ( const char* id );
glm::vec4 vxui_hud_tile          ( int tex_w, int tex_h, int tile_w, int tile_h, int idx );
glm::vec4 vxui_hud_tile1d        ( int tex_sz, int tile_sz, int idx );
void      vxui_hud_meter         ( vxui_hud* hud, const char* id, uint32_t texture_id, float t, float w, float h, bool vertical = false, bool reverse = false, glm::vec4 uv = { 0.0f, 0.0f, 1.0f, 1.0f } );
void      vxui_hud_resource_stock( vxui_hud* hud, const char* id, uint32_t texture_id, int count, float w, float h, uint8_t layout, glm::vec4 uv, float gap = 0.0f );
void      vxui_hud_image         ( vxui_hud* hud, const char* id, uint32_t texture_id, float w, float h, glm::vec4 uv = { 0.0f, 0.0f, 1.0f, 1.0f } );
void      vxui_hud_wallpaper     ( vxui_hud* hud, const char* id, uint32_t texture_id, glm::vec4 uv = { 0.0f, 0.0f, 1.0f, 1.0f } );
bool      vxui_hud_resolve       ( const vxui_hud* hud, const vxui_draw_cmd* cmd, vxui_render_data* out );

#ifdef VXUI_HUD_IMPL

#include "clay/clay.h"
#include <cstdio>

static void vxui_hud_record( vxui_hud* hud, uint32_t id, uint32_t texture_id, glm::vec4 uv )
{
    assert( hud && hud->item_count < VXUI_HUD_MAX_ITEMS );
    int i = hud->item_count++;
    hud->items[i].id         = id;
    hud->items[i].texture_id = texture_id;
    hud->items[i].z          = hud->z;
    hud->items[i].colour     = hud->colour;
    hud->items[i].uv         = uv;
}

static Clay_ElementId vxui_hud_clay_id( const char* id )
{
    Clay_String cs = { false, (int32_t) strlen( id ), id };
    return Clay__HashString( cs, 0 );
}

static void vxui_hud_rect_at( vxui_hud* hud, const char* id, float x, float y, float w, float h, uint32_t texture_id, glm::vec4 uv )
{
    Clay_ElementId eid = vxui_hud_clay_id( id );
    Clay__OpenElementWithId( eid );

    Clay_ElementDeclaration decl = {};
    decl.layout.sizing.width           = CLAY_SIZING_FIXED( w );
    decl.layout.sizing.height          = CLAY_SIZING_FIXED( h );
    decl.floating.attachTo             = CLAY_ATTACH_TO_ROOT;
    decl.floating.attachPoints.element = CLAY_ATTACH_POINT_LEFT_TOP;
    decl.floating.attachPoints.parent  = CLAY_ATTACH_POINT_LEFT_TOP;
    decl.floating.offset               = { x, y };
    decl.floating.zIndex               = hud->z;
    decl.backgroundColor               = { 0, 0, 0, 1 };

    Clay__ConfigureOpenElement( decl );
    Clay__CloseElement();

    vxui_hud_record( hud, eid.id, texture_id, uv );
}

void vxui_hud_begin( vxui_hud* hud, vxui_ctx* ctx, float w, float h )
{
    assert( hud && ctx );
    hud->ctx         = ctx;
    hud->x           = 0.0f;
    hud->y           = 0.0f;
    hud->w           = w;
    hud->h           = h;
    hud->colour      = { 1.0f, 1.0f, 1.0f, 1.0f };
    hud->font        = 0;
    hud->font_px     = VXUI_FONT_SIZE_DEFAULT;
    hud->z           = 0;
    hud->item_count  = 0;
    hud->text_offset = 0;
}

void vxui_hud_set_pos( vxui_hud* hud, float x, float y )
{
    assert( hud );
    hud->x = x;
    hud->y = y;
}

void vxui_hud_move_pos( vxui_hud* hud, float x, float y )
{
    assert( hud );
    hud->x += x;
    hud->y += y;
}

void vxui_hud_set_colour( vxui_hud* hud, glm::vec4 colour )
{
    assert( hud );
    hud->colour = colour;
}

void vxui_hud_set_font( vxui_hud* hud, vxui_font_id font, uint16_t px )
{
    assert( hud );
    hud->font    = font;
    hud->font_px = px;
}

void vxui_hud_set_z( vxui_hud* hud, int16_t z )
{
    assert( hud );
    hud->z = z;
}

uint32_t vxui_hud_id( const char* id )
{
    return vxui_id( id );
}

glm::vec4 vxui_hud_tile( int tex_w, int tex_h, int tile_w, int tile_h, int idx )
{
    int tilesX = tex_w / tile_w;
    int tilesY = tex_h / tile_h;
    if ( tilesX <= 0 ) tilesX = 1;
    if ( tilesY <= 0 ) tilesY = 1;

    int tileX = idx % tilesX;
    int tileY = idx / tilesX;

    float u0 = (float)( tileX * tile_w ) / (float) tex_w;
    float v0 = (float)( tileY * tile_h ) / (float) tex_h;
    float u1 = (float)( ( tileX + 1 ) * tile_w ) / (float) tex_w;
    float v1 = (float)( ( tileY + 1 ) * tile_h ) / (float) tex_h;

    return { u0, v0, u1, v1 };
}

glm::vec4 vxui_hud_tile1d( int tex_sz, int tile_sz, int idx )
{
    return vxui_hud_tile( tex_sz, tex_sz, tile_sz, tile_sz, idx );
}

void vxui_hud_meter( vxui_hud* hud, const char* id, uint32_t texture_id, float t, float w, float h, bool vertical, bool reverse, glm::vec4 uv )
{
    assert( hud && id );
    t = glm::clamp( t, 0.0f, 1.0f );

    vxui_hud_rect_at( hud, id, hud->x, hud->y, w, h, texture_id, uv );

    float fx = hud->x;
    float fy = hud->y;
    float fw = vertical ? w : w * t;
    float fh = vertical ? h * t : h;

    if ( reverse )
    {
        if ( vertical ) fy = hud->y + h - fh;
        else            fx = hud->x + w - fw;
    }

    char fill_id[128];
    int  n = snprintf( fill_id, sizeof( fill_id ), "%s.fill", id );
    assert( n > 0 && n < (int) sizeof( fill_id ) );

    vxui_hud_rect_at( hud, fill_id, fx, fy, fw, fh, texture_id, uv );

    hud->y += h;
}

static glm::vec2 vxui_hud_stock_pos( int i, float w, float h, uint8_t layout, float gap )
{
    switch ( layout )
    {
        case VXUI_HUD_STOCK_COL:     return { 0.0f, (float) i * ( h + gap ) };
        case VXUI_HUD_STOCK_GRID4:   return { (float) ( i % 4 ) * ( w + gap ), (float) ( i / 4 ) * ( h + gap ) };
        case VXUI_HUD_STOCK_GRID2:   return { (float) ( i % 2 ) * ( w + gap ), (float) ( i / 2 ) * ( h + gap ) };
        case VXUI_HUD_STOCK_HEX_COL: return { (float) ( i % 2 ) * ( w + gap ), (float) ( i / 2 ) * ( h + gap ) + (float) ( i % 2 ) * h * 0.5f };
        case VXUI_HUD_STOCK_HEX_ROW: return { (float) ( i / 2 ) * ( w + gap ) + (float) ( i % 2 ) * w * 0.5f, (float) ( i % 2 ) * ( h + gap ) };
        default:                     return { (float) i * ( w + gap ), 0.0f };
    }
}

void vxui_hud_resource_stock( vxui_hud* hud, const char* id, uint32_t texture_id, int count, float w, float h, uint8_t layout, glm::vec4 uv, float gap )
{
    assert( hud && id && count >= 0 && w > 0.0f && h > 0.0f );
    if ( count <= 0 ) return;

    float bh = 0.0f;
    for ( int i = 0; i < count; i++ )
    {
        glm::vec2 p = vxui_hud_stock_pos( i, w, h, layout, gap );
        bh = glm::max( bh, p.y + h );
    }

    for ( int i = 0; i < count; i++ )
    {
        char slot_id[128];
        int  n = snprintf( slot_id, sizeof( slot_id ), "%s.%d", id, i );
        assert( n > 0 && n < (int) sizeof( slot_id ) );

        glm::vec2 p = vxui_hud_stock_pos( i, w, h, layout, gap );
        vxui_hud_rect_at( hud, slot_id, hud->x + p.x, hud->y + p.y, w, h, texture_id, uv );
    }

    hud->y += bh;
}

void vxui_hud_image( vxui_hud* hud, const char* id, uint32_t texture_id, float w, float h, glm::vec4 uv )
{
    assert( hud && id && w > 0.0f && h > 0.0f );
    vxui_hud_rect_at( hud, id, hud->x, hud->y, w, h, texture_id, uv );
    hud->y += h;
}

void vxui_hud_wallpaper( vxui_hud* hud, const char* id, uint32_t texture_id, glm::vec4 uv )
{
    assert( hud && id );
    vxui_hud_rect_at( hud, id, 0.0f, 0.0f, hud->w, hud->h, texture_id, uv );
}

bool vxui_hud_resolve( const vxui_hud* hud, const vxui_draw_cmd* cmd, vxui_render_data* out )
{
    assert( hud && cmd && out );
    for ( int i = 0; i < hud->item_count; i++ )
    {
        if ( hud->items[i].id != cmd->id ) continue;
        out->texture_id = hud->items[i].texture_id;
        out->uv         = hud->items[i].uv;
        out->colour     = hud->items[i].colour;
        return true;
    }
    return false;
}

#endif // #ifdef VXUI_HUD_IMPL
