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

// Focus rect id: menu hash as seed, "focus" as key. Matches vxui_menu_end.
static uint32_t focus_id( const char* menu )
{
    Clay_String cs = CLAY_STRING( "focus" );
    return Clay__HashString( cs, vxui_hash( menu ) ).id;
}

UTEST(menu_draw, one_action_two_rects) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 2 );    // 1 row + 1 focus
}

UTEST(menu_draw, three_actions_four_rects) {
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

    ASSERT_EQ( dl.count, 4 );    // 3 rows + 1 focus
}

UTEST(menu_draw, rect_id_matches_label) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 2 );
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

    ASSERT_EQ( dl.count, 4 );
    ASSERT_EQ( dl.cmds[0].id, row_id( "m", "Play" ) );
    ASSERT_EQ( dl.cmds[1].id, row_id( "m", "Options" ) );
    ASSERT_EQ( dl.cmds[2].id, row_id( "m", "Quit" ) );
}

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

    ASSERT_EQ( dl.count, 4 );    // 2 rows + 2 focus
    ASSERT_EQ( dl.cmds[0].id, row_id( "left",  "Back" ) );
    ASSERT_EQ( dl.cmds[1].id, row_id( "right", "Back" ) );
    ASSERT_NE( dl.cmds[0].id, dl.cmds[1].id );
}

UTEST(menu_draw, row_has_fixed_height) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 2 );
    ASSERT_NEAR( dl.cmds[0].rect.w, (float) VXUI_ROW_HEIGHT, 1e-3f );  // rect.w = height (Clay h)
}

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

    ASSERT_EQ( dl.count, 2 );    // 1 row + 1 focus
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

    ASSERT_EQ( dl.count, 2 );
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

    ASSERT_EQ( dl.count, 2 );
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

    ASSERT_EQ( dl.count, 3 );    // 2 rows + 1 focus
    ASSERT_EQ( dl.cmds[0].id, row_id( "m", "Play" ) );
    ASSERT_EQ( dl.cmds[1].id, row_id( "m", "Difficulty" ) );
}

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

    ASSERT_EQ( dl.count, 2 );
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

    ASSERT_EQ( dl.count, 2 );
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

    ASSERT_EQ( dl.count, 2 );
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

    ASSERT_EQ( dl.count, 4 );    // 3 rows + 1 focus
    ASSERT_EQ( dl.cmds[0].id, row_id( "m", "Play" ) );
    ASSERT_EQ( dl.cmds[1].id, row_id( "m", "Difficulty" ) );
    ASSERT_EQ( dl.cmds[2].id, row_id( "m", "Volume" ) );
}

UTEST(menu_draw, section_emits_rect) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_section( &ctx, "Header" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 1 );    // section only, no focus rect (non-interactive)
}

UTEST(menu_draw, section_rect_id_matches_title) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_section( &ctx, "Header" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 1 );
    ASSERT_EQ( dl.cmds[0].id, row_id( "m", "Header" ) );
}

UTEST(menu_draw, section_row_has_fixed_height) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_section( &ctx, "Header" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 1 );
    ASSERT_NEAR( dl.cmds[0].rect.w, (float) VXUI_ROW_HEIGHT, 1e-3f );
}

UTEST(menu_draw, label_emits_rect) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_label( &ctx, "Info text" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 1 );    // label only, no focus rect
}

UTEST(menu_draw, label_rect_id_matches_text) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_label( &ctx, "Info text" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 1 );
    ASSERT_EQ( dl.cmds[0].id, row_id( "m", "Info text" ) );
}

UTEST(menu_draw, label_row_has_fixed_height) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_label( &ctx, "Info text" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 1 );
    ASSERT_NEAR( dl.cmds[0].rect.w, (float) VXUI_ROW_HEIGHT, 1e-3f );
}

UTEST(menu_draw, first_row_at_origin_y) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 2 );
    ASSERT_NEAR( dl.cmds[0].rect.y, 0.0f, 1e-3f );
}

UTEST(menu_draw, second_row_below_first) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_action( &ctx, "Quit" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 3 );    // 2 rows + 1 focus
    ASSERT_NEAR( dl.cmds[1].rect.y, dl.cmds[0].rect.y + (float) VXUI_ROW_HEIGHT, 1e-3f );
}

UTEST(menu_draw, three_rows_stack_vertically) {
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

    ASSERT_EQ( dl.count, 4 );    // 3 rows + 1 focus
    ASSERT_NEAR( dl.cmds[0].rect.y, 0.0f,                              1e-3f );
    ASSERT_NEAR( dl.cmds[1].rect.y, 1.0f * (float) VXUI_ROW_HEIGHT,    1e-3f );
    ASSERT_NEAR( dl.cmds[2].rect.y, 2.0f * (float) VXUI_ROW_HEIGHT,    1e-3f );
}

UTEST(menu_draw, rows_share_same_x) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_action( &ctx, "Quit" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 3 );
    ASSERT_NEAR( dl.cmds[0].rect.x, dl.cmds[1].rect.x, 1e-3f );
}

UTEST(menu_draw, all_row_types_emit_rects) {
    vxui_ctx ctx = make_ctx();
    int idx = 0;
    float v = 0.5f;
    const char* opts[] = { "A", "B" };

    // Frame 1: establish menu (num_rows + skip_mask written by menu_end).
    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_section( &ctx, "Header" );
        vxui_menu_label  ( &ctx, "Info" );
        vxui_menu_action ( &ctx, "Play" );
        vxui_menu_option ( &ctx, "Difficulty", &idx, opts, 2 );
        vxui_menu_slider ( &ctx, "Volume", &v, 0.0f, 1.0f, 0.1f );
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );

    // Frame 2: initial-skip advances focus from row 0 (section) to row 2 (Play).
    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_section( &ctx, "Header" );
        vxui_menu_label  ( &ctx, "Info" );
        vxui_menu_action ( &ctx, "Play" );
        vxui_menu_option ( &ctx, "Difficulty", &idx, opts, 2 );
        vxui_menu_slider ( &ctx, "Volume", &v, 0.0f, 1.0f, 0.1f );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 6 );    // 5 rows + 1 focus
    ASSERT_EQ( dl.cmds[0].id, row_id( "m", "Header" ) );
    ASSERT_EQ( dl.cmds[1].id, row_id( "m", "Info" ) );
    ASSERT_EQ( dl.cmds[2].id, row_id( "m", "Play" ) );
    ASSERT_EQ( dl.cmds[3].id, row_id( "m", "Difficulty" ) );
    ASSERT_EQ( dl.cmds[4].id, row_id( "m", "Volume" ) );
}

// ---- focus rect tests ------------------------------------------------

UTEST(menu_draw, focus_rect_present_when_action_focusable) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 2 );
    ASSERT_EQ( dl.cmds[1].id, focus_id( "m" ) );
}

UTEST(menu_draw, focus_rect_id_is_menu_focus_hash) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "settings" ) )
    {
        vxui_menu_action( &ctx, "Apply" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 2 );
    ASSERT_EQ( dl.cmds[1].id, focus_id( "settings" ) );
}

UTEST(menu_draw, focus_rect_position_matches_focused_row) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_action( &ctx, "Quit" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 3 );
    // focus is row 0 (Play), so focus rect should be at row 0's y position
    ASSERT_NEAR( dl.cmds[2].rect.y, dl.cmds[0].rect.y, 1e-3f );
}

UTEST(menu_draw, focus_rect_moves_with_navigation) {
    vxui_ctx ctx = make_ctx();

    // Frame 1: establish menu (focus = 0)
    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_action( &ctx, "Quit" );
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );

    // Frame 2: down input -> focus = 1
    vxui_frame( &ctx, 1.0f / 60.0f );
    ctx.input = VXUI_INPUT_DOWN;
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_action( &ctx, "Quit" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 3 );
    // focus rect is at row 1 (Quit), so its y should match row 1's y
    ASSERT_NEAR( dl.cmds[2].rect.y, dl.cmds[1].rect.y, 1e-3f );
}

UTEST(menu_draw, focus_rect_skips_section_row) {
    vxui_ctx ctx = make_ctx();

    // Frame 1: establish (num_rows = 2, skip_mask = bit 0 set for section).
    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_section( &ctx, "Header" );
        vxui_menu_action ( &ctx, "Play" );
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );

    // Frame 2: initial-skip moves focus from row 0 (section) to row 1 (Play).
    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_section( &ctx, "Header" );
        vxui_menu_action ( &ctx, "Play" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 3 );    // section + action + focus
    // focus rect should be at row 1 (Play), not row 0 (Header)
    ASSERT_NEAR( dl.cmds[2].rect.y, dl.cmds[1].rect.y, 1e-3f );
    ASSERT_NE  ( dl.cmds[2].id,    dl.cmds[0].id );
}

UTEST(menu_draw, no_focus_rect_when_no_interactive_rows) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_section( &ctx, "Header" );
        vxui_menu_label  ( &ctx, "Info" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 2 );    // 2 non-interactive rows, no focus rect
}

UTEST_MAIN();
