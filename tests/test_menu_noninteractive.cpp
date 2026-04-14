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
    section_frame( &ctx );          // establish: rows = [section, Play, Quit]
    ctx.input = 0;

    // Focus starts at 0 (section row). Down should move to Play (row 1).
    vxui_input( &ctx, "down" );
    section_frame( &ctx );
    ctx.input = 0;

    // Confirm — should fire Play (row 1), not section (row 0).
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
    label_frame( &ctx );            // establish: rows = [label, Play, Quit]
    ctx.input = 0;

    vxui_input( &ctx, "down" );
    label_frame( &ctx );
    ctx.input = 0;

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
