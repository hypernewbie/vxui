#define CLAY_IMPLEMENTATION
#include "clay/clay.h"

#define VXUI_IMPL
#include "vxui.h"

bool ve_fontcache_draw_text( ve_fontcache* cache, ve_font_id font, const std::u8string& text_utf8, float posx, float posy, float scalex, float scaley, bool shape_cache )
{
    ( void ) cache;
    ( void ) font;
    ( void ) text_utf8;
    ( void ) posx;
    ( void ) posy;
    ( void ) scalex;
    ( void ) scaley;
    ( void ) shape_cache;
    return false;
}

ve_fontcache_vec2 ve_fontcache_get_cursor_pos( ve_fontcache* cache )
{
    ( void ) cache;
    return {};
}

int main( void )
{
    return ( int ) vxui_min_memory_size();
}
