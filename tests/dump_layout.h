#pragma once

#include "../vxui.h"
#include <cstdio>

// Dump a draw list as a text file for python tools/plot_layout.py to render.
// label_for: optional callback mapping id -> short label string. Returns nullptr if unknown.
static void dump_layout( const vxui_draw_list& dl, float w, float h, float dt,
                         const char* path,
                         const char* (*label_for)( uint32_t id ) = nullptr )
{
    FILE* f = fopen( path, "w" );
    assert( f && "dump_layout: failed to open file" );

    fprintf( f, "# vxui layout dump\n" );
    fprintf( f, "# w %.1f h %.1f dt %.6f\n", w, h, dt );
    fprintf( f, "# columns: id x y w h label\n" );

    int rect_n = vxui_draw_count( dl, VXUI_DRAW_RECT );
    for ( int i = 0; i < rect_n; i++ )
    {
        const vxui_draw_cmd& c = *vxui_draw_nth( dl, VXUI_DRAW_RECT, i );
        const char* label = label_for ? label_for( c.id ) : nullptr;
        fprintf( f, "0x%08x  %.1f  %.1f  %.1f  %.1f  %s\n",
                 c.id, c.rect.x, c.rect.y, c.rect.z, c.rect.w,
                 label ? label : "-" );
    }

    fclose( f );
}
