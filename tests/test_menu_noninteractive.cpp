#include "utest.h"
#include "../vxui.h"
#define VXUI_IMPL
#include "../vxui_impl.h"

// Helper: section then two actions. Returns which action fired (-1 = none).
static int section_frame( vxui_ctx* ctx )
{
    int fired = -1;
    if ( vxui_menu( ctx, "test" ) )
    {
        vxui_menu_section( ctx, "Settings" );
        if ( vxui_menu_action( ctx, "Play" ) )  fired = 0;
        if ( vxui_menu_action( ctx, "Quit" ) )  fired = 1;
        vxui_menu_end( ctx );
    }
    return fired;
}

// Helper: label then two actions.
static int label_frame( vxui_ctx* ctx )
{
    int fired = -1;
    if ( vxui_menu( ctx, "test" ) )
    {
        vxui_menu_label( ctx, "Choose wisely" );
        if ( vxui_menu_action( ctx, "Play" ) )  fired = 0;
        if ( vxui_menu_action( ctx, "Quit" ) )  fired = 1;
        vxui_menu_end( ctx );
    }
    return fired;
}

/* ---- section --------------------------------------------------------- */

UTEST(menu_section, focus_skips_section_row) {
    vxui_ctx ctx = {};
    section_frame( &ctx );          // frame 1: establish [section, Play, Quit]
    ctx.input = 0;

    // Frame 2: no nav input. Initial skip advances focus from 0 (section) to 1 (Play).
    section_frame( &ctx );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 1 );
    ctx.input = 0;

    // Frame 3: confirm — fires Play (row 1), not section (row 0).
    vxui_input( &ctx, "confirm" );
    int fired = section_frame( &ctx );
    ASSERT_EQ( fired, 0 );
}

UTEST(menu_section, counts_as_row) {
    vxui_ctx ctx = {};
    section_frame( &ctx );
    // 3 rows total: section + Play + Quit
    ASSERT_EQ( ctx.menu_state[0].z, (uint32_t) 3 );
}

/* ---- label ----------------------------------------------------------- */

UTEST(menu_label, focus_skips_label_row) {
    vxui_ctx ctx = {};
    label_frame( &ctx );            // frame 1: establish [label, Play, Quit]
    ctx.input = 0;

    // Frame 2: no nav input. Initial skip advances focus from 0 (label) to 1 (Play).
    label_frame( &ctx );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 1 );
    ctx.input = 0;

    // Frame 3: confirm — fires Play (row 1), not label (row 0).
    vxui_input( &ctx, "confirm" );
    int fired = label_frame( &ctx );
    ASSERT_EQ( fired, 0 );
}

UTEST(menu_label, counts_as_row) {
    vxui_ctx ctx = {};
    label_frame( &ctx );
    ASSERT_EQ( ctx.menu_state[0].z, (uint32_t) 3 );
}

UTEST_MAIN();
