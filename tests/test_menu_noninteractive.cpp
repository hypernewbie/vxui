#include "utest.h"
#include "../vxui.h"
#define VXUI_IMPL
#include "../vxui_impl.h"

static uint8_t s_clay_mem[16 * 1024 * 1024];

static vxui_ctx make_ctx()
{
    vxui_ctx ctx = {};
    vxui_init( &ctx, 1280, 720, s_clay_mem, sizeof( s_clay_mem ) );
    return ctx;
}

// Helper: section then two actions. Returns which action fired (-1 = none).
static int section_frame( vxui_ctx* ctx, uint32_t input )
{
    int fired = -1;
    vxui_frame( ctx, 1.0f / 60.0f );
    ctx->input = input;
    if ( vxui_menu( ctx, "test" ) )
    {
        vxui_menu_section( ctx, "Settings" );
        if ( vxui_menu_action( ctx, "Play" ) )  fired = 0;
        if ( vxui_menu_action( ctx, "Quit" ) )  fired = 1;
        vxui_menu_end( ctx );
    }
    vxui_render( ctx );
    return fired;
}

// Helper: label then two actions.
static int label_frame( vxui_ctx* ctx, uint32_t input )
{
    int fired = -1;
    vxui_frame( ctx, 1.0f / 60.0f );
    ctx->input = input;
    if ( vxui_menu( ctx, "test" ) )
    {
        vxui_menu_label( ctx, "Choose wisely" );
        if ( vxui_menu_action( ctx, "Play" ) )  fired = 0;
        if ( vxui_menu_action( ctx, "Quit" ) )  fired = 1;
        vxui_menu_end( ctx );
    }
    vxui_render( ctx );
    return fired;
}

/* ---- section --------------------------------------------------------- */

UTEST(menu_section, focus_skips_section_row) {
    vxui_ctx ctx = make_ctx();
    section_frame( &ctx, 0 );       // frame 1: establish [section, Play, Quit]

    // Frame 2: no nav input. Initial skip advances focus from 0 (section) to 1 (Play).
    section_frame( &ctx, 0 );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 1 );

    // Frame 3: confirm — fires Play (row 1), not section (row 0).
    int fired = section_frame( &ctx, VXUI_INPUT_CONFIRM );
    ASSERT_EQ( fired, 0 );
}

UTEST(menu_section, counts_as_row) {
    vxui_ctx ctx = make_ctx();
    section_frame( &ctx, 0 );
    // 3 rows total: section + Play + Quit
    ASSERT_EQ( ctx.menu_state[0].z, (uint32_t) 3 );
}

/* ---- label ----------------------------------------------------------- */

UTEST(menu_label, focus_skips_label_row) {
    vxui_ctx ctx = make_ctx();
    label_frame( &ctx, 0 );         // frame 1: establish [label, Play, Quit]

    // Frame 2: no nav input. Initial skip advances focus from 0 (label) to 1 (Play).
    label_frame( &ctx, 0 );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 1 );

    // Frame 3: confirm — fires Play (row 1), not label (row 0).
    int fired = label_frame( &ctx, VXUI_INPUT_CONFIRM );
    ASSERT_EQ( fired, 0 );
}

UTEST(menu_label, counts_as_row) {
    vxui_ctx ctx = make_ctx();
    label_frame( &ctx, 0 );
    ASSERT_EQ( ctx.menu_state[0].z, (uint32_t) 3 );
}

UTEST_MAIN();
