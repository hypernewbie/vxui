#define CLAY_IMPLEMENTATION
#include "clay/clay.h"

#define VXUI_IMPL
#include "vxui.h"

ve_fontcache_vec2 ve_fontcache_measure_text( ve_fontcache* cache, ve_font_id font, const std::u8string& text_utf8, float scalex, float scaley, bool shape_cache )
{
    ( void ) cache;
    ( void ) font;
    ( void ) text_utf8;
    ( void ) scalex;
    ( void ) scaley;
    ( void ) shape_cache;
    return {};
}

int main( void )
{
    return ( int ) vxui_min_memory_size();
}
