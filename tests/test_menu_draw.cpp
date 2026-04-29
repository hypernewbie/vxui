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

// Replicate composite id: menu hash as seed, label as key. Matches vxui_menu_open_row.
static uint32_t row_id( const char* menu, const char* label )
{
    Clay_String cs = { false, (int32_t) strlen( label ), label };
    return Clay__HashString( cs, vxui_hash( menu ) ).id;
}

/* ---- action emits rect ----------------------------------------------- */

UTEST(menu_draw, one_action_one_rect) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 1 );
}

UTEST(menu_draw, three_actions_three_rects) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_action( &ctx, "Options" );
        vxui_menu_action( &ctx, "Quit" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 3 );
}

/* ---- rect id matches label hash -------------------------------------- */

UTEST(menu_draw, rect_id_matches_label) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 1 );
    ASSERT_EQ( dl.cmds[0].id, row_id( "m", "Play" ) );
}

UTEST(menu_draw, three_rect_ids_match_labels) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_action( &ctx, "Options" );
        vxui_menu_action( &ctx, "Quit" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 3 );
    ASSERT_EQ( dl.cmds[0].id, row_id( "m", "Play" ) );
    ASSERT_EQ( dl.cmds[1].id, row_id( "m", "Options" ) );
    ASSERT_EQ( dl.cmds[2].id, row_id( "m", "Quit" ) );
}

/* ---- composite id — same label different menus don't collide --------- */

UTEST(menu_draw, same_label_different_menus_distinct_ids) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "left" ) )
    {
        vxui_menu_action( &ctx, "Back" );
        vxui_menu_end( &ctx );
    }
    if ( vxui_menu( &ctx, "right" ) )
    {
        vxui_menu_action( &ctx, "Back" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 2 );
    ASSERT_NE( dl.cmds[0].id, dl.cmds[1].id );
}

/* ---- row height ------------------------------------------------------ */

UTEST(menu_draw, row_has_fixed_height) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 1 );
    ASSERT_NEAR( dl.cmds[0].rect.w, (float) VXUI_ROW_HEIGHT, 1e-3f );  // rect.w = height (Clay h)
}

/* ---- option emits rect ----------------------------------------------- */

UTEST(menu_draw, option_emits_rect) {
    vxui_ctx ctx = make_ctx();
    int idx = 0;
    const char* opts[] = { "Easy", "Normal", "Hard" };

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_option( &ctx, "Difficulty", &idx, opts, 3 );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 1 );
}

UTEST(menu_draw, option_rect_id_matches_label) {
    vxui_ctx ctx = make_ctx();
    int idx = 0;
    const char* opts[] = { "Easy", "Normal", "Hard" };

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_option( &ctx, "Difficulty", &idx, opts, 3 );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 1 );
    ASSERT_EQ( dl.cmds[0].id, row_id( "m", "Difficulty" ) );
}

UTEST(menu_draw, option_row_has_fixed_height) {
    vxui_ctx ctx = make_ctx();
    int idx = 0;
    const char* opts[] = { "Easy", "Normal", "Hard" };

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_option( &ctx, "Difficulty", &idx, opts, 3 );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 1 );
    ASSERT_NEAR( dl.cmds[0].rect.w, (float) VXUI_ROW_HEIGHT, 1e-3f );
}

UTEST(menu_draw, action_and_option_emit_two_rects) {
    vxui_ctx ctx = make_ctx();
    int idx = 0;
    const char* opts[] = { "A", "B" };

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_option( &ctx, "Difficulty", &idx, opts, 2 );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 2 );
    ASSERT_EQ( dl.cmds[0].id, row_id( "m", "Play" ) );
    ASSERT_EQ( dl.cmds[1].id, row_id( "m", "Difficulty" ) );
}

/* ---- slider emits rect ----------------------------------------------- */

UTEST(menu_draw, slider_emits_rect) {
    vxui_ctx ctx = make_ctx();
    float v = 0.5f;

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_slider( &ctx, "Volume", &v, 0.0f, 1.0f, 0.1f );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 1 );
}

UTEST(menu_draw, slider_rect_id_matches_label) {
    vxui_ctx ctx = make_ctx();
    float v = 0.5f;

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_slider( &ctx, "Volume", &v, 0.0f, 1.0f, 0.1f );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 1 );
    ASSERT_EQ( dl.cmds[0].id, row_id( "m", "Volume" ) );
}

UTEST(menu_draw, slider_row_has_fixed_height) {
    vxui_ctx ctx = make_ctx();
    float v = 0.5f;

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_slider( &ctx, "Volume", &v, 0.0f, 1.0f, 0.1f );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 1 );
    ASSERT_NEAR( dl.cmds[0].rect.w, (float) VXUI_ROW_HEIGHT, 1e-3f );
}

UTEST(menu_draw, action_option_slider_emit_three_rects) {
    vxui_ctx ctx = make_ctx();
    int idx = 0;
    float v = 0.5f;
    const char* opts[] = { "A", "B" };

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_option( &ctx, "Difficulty", &idx, opts, 2 );
        vxui_menu_slider( &ctx, "Volume", &v, 0.0f, 1.0f, 0.1f );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 3 );
    ASSERT_EQ( dl.cmds[0].id, row_id( "m", "Play" ) );
    ASSERT_EQ( dl.cmds[1].id, row_id( "m", "Difficulty" ) );
    ASSERT_EQ( dl.cmds[2].id, row_id( "m", "Volume" ) );
}

/* ---- section/label do not emit rects --------------------------------- */

UTEST(menu_draw, section_emits_no_rect) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_section( &ctx, "Header" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 0 );
}

UTEST(menu_draw, label_emits_no_rect) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_label( &ctx, "Info text" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 0 );
}

UTEST_MAIN();
