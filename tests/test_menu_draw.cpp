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

UTEST(menu_draw, focus_rect_settles_after_many_frames) {
    vxui_ctx ctx = make_ctx();

    // Establish menu, focus = 0. Spring snaps to row 0 on first frame (no jump).
    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_action( &ctx, "Quit" );
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );

    // Many idle frames, focus stays at row 0. Spring settled.
    for ( int i = 0; i < 60; i++ )
    {
        vxui_frame( &ctx, 1.0f / 60.0f );
        if ( vxui_menu( &ctx, "m" ) )
        {
            vxui_menu_action( &ctx, "Play" );
            vxui_menu_action( &ctx, "Quit" );
            vxui_menu_end( &ctx );
        }
        vxui_render( &ctx );
    }

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_action( &ctx, "Quit" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 3 );
    // Focus on row 0, spring settled, focus rect at row 0's y.
    ASSERT_NEAR( dl.cmds[2].rect.y, dl.cmds[0].rect.y, 1e-3f );
}

UTEST(menu_draw, focus_rect_animates_after_navigation) {
    vxui_ctx ctx = make_ctx();

    // Frame 1: establish, focus = 0, spring snaps.
    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_action( &ctx, "Quit" );
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );

    // Frame 2: down input -> focus = 1. Spring jumps offset to compensate, then steps once.
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
    // Focus rect's y is BETWEEN row 0 and row 1 (not yet at the new row).
    float row0_y = dl.cmds[0].rect.y;
    float row1_y = dl.cmds[1].rect.y;
    float focus_y = dl.cmds[2].rect.y;
    ASSERT_GT( focus_y, row0_y - 1e-3f );
    ASSERT_LT( focus_y, row1_y + 1e-3f );
    ASSERT_GT( focus_y, row0_y + 1e-3f );   // moved away from start
    ASSERT_LT( focus_y, row1_y - 1e-3f );   // not yet at end
}

UTEST(menu_draw, focus_rect_settles_after_navigation) {
    vxui_ctx ctx = make_ctx();

    // Frame 1: establish.
    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_action( &ctx, "Quit" );
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );

    // Frame 2: down input -> focus = 1.
    vxui_frame( &ctx, 1.0f / 60.0f );
    ctx.input = VXUI_INPUT_DOWN;
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_action( &ctx, "Quit" );
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );

    // Many idle frames -> spring settles to new row.
    for ( int i = 0; i < 120; i++ )
    {
        vxui_frame( &ctx, 1.0f / 60.0f );
        if ( vxui_menu( &ctx, "m" ) )
        {
            vxui_menu_action( &ctx, "Play" );
            vxui_menu_action( &ctx, "Quit" );
            vxui_menu_end( &ctx );
        }
        vxui_render( &ctx );
    }

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_action( &ctx, "Quit" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 3 );
    ASSERT_NEAR( dl.cmds[2].rect.y, dl.cmds[1].rect.y, 1e-2f );
}

UTEST(menu_draw, focus_rect_first_frame_snaps) {
    vxui_ctx ctx = make_ctx();

    // First frame ever: focus is at row 0. Spring should snap immediately
    // (no animation from origin), focus rect at row 0's position.
    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_action( &ctx, "Quit" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 3 );
    ASSERT_NEAR( dl.cmds[2].rect.y, dl.cmds[0].rect.y, 1e-3f );
}

UTEST(menu_draw, focus_spring_per_menu_isolation) {
    vxui_ctx ctx = make_ctx();

    // Two menus. Establish both with focus on row 0.
    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "left" ) )
    {
        vxui_menu_action( &ctx, "L1" );
        vxui_menu_action( &ctx, "L2" );
        vxui_menu_end( &ctx );
    }
    if ( vxui_menu( &ctx, "right" ) )
    {
        vxui_menu_action( &ctx, "R1" );
        vxui_menu_action( &ctx, "R2" );
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );

    // Settle both for many frames.
    for ( int i = 0; i < 120; i++ )
    {
        vxui_frame( &ctx, 1.0f / 60.0f );
        if ( vxui_menu( &ctx, "left" ) )
        {
            vxui_menu_action( &ctx, "L1" );
            vxui_menu_action( &ctx, "L2" );
            vxui_menu_end( &ctx );
        }
        if ( vxui_menu( &ctx, "right" ) )
        {
            vxui_menu_action( &ctx, "R1" );
            vxui_menu_action( &ctx, "R2" );
            vxui_menu_end( &ctx );
        }
        vxui_render( &ctx );
    }

    // Each menu's spring offset is independently 0.
    ASSERT_NEAR( ctx.menu_focus_spring[0].x, 0.0f, 1e-2f );
    ASSERT_NEAR( ctx.menu_focus_spring[1].x, 0.0f, 1e-2f );
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

UTEST(menu_draw, section_then_action_action_below) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_section( &ctx, "Header" );    // row 0
        vxui_menu_action ( &ctx, "Play" );       // row 1
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );

    // Frame 2: focus skipped to row 1.
    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_section( &ctx, "Header" );
        vxui_menu_action ( &ctx, "Play" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 3 );    // section + action + focus
    ASSERT_NEAR( dl.cmds[0].rect.y, 0.0f,                            1e-3f );
    ASSERT_NEAR( dl.cmds[1].rect.y, 1.0f * (float) VXUI_ROW_HEIGHT,  1e-3f );
}

UTEST(menu_draw, label_then_action_action_below) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_label ( &ctx, "Info" );        // row 0
        vxui_menu_action( &ctx, "Play" );        // row 1
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_label ( &ctx, "Info" );
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 3 );
    ASSERT_NEAR( dl.cmds[0].rect.y, 0.0f,                            1e-3f );
    ASSERT_NEAR( dl.cmds[1].rect.y, 1.0f * (float) VXUI_ROW_HEIGHT,  1e-3f );
}

UTEST(menu_draw, interleaved_skip_rows_stack) {
    vxui_ctx ctx = make_ctx();

    // Frame 1: establish [section, action, label, action] (4 rows).
    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_section( &ctx, "Top" );        // row 0
        vxui_menu_action ( &ctx, "Play" );       // row 1
        vxui_menu_label  ( &ctx, "Note" );       // row 2
        vxui_menu_action ( &ctx, "Quit" );       // row 3
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_section( &ctx, "Top" );
        vxui_menu_action ( &ctx, "Play" );
        vxui_menu_label  ( &ctx, "Note" );
        vxui_menu_action ( &ctx, "Quit" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 5 );    // 4 rows + 1 focus
    ASSERT_NEAR( dl.cmds[0].rect.y, 0.0f,                            1e-3f );
    ASSERT_NEAR( dl.cmds[1].rect.y, 1.0f * (float) VXUI_ROW_HEIGHT,  1e-3f );
    ASSERT_NEAR( dl.cmds[2].rect.y, 2.0f * (float) VXUI_ROW_HEIGHT,  1e-3f );
    ASSERT_NEAR( dl.cmds[3].rect.y, 3.0f * (float) VXUI_ROW_HEIGHT,  1e-3f );
}

UTEST(menu_draw, focus_rect_y_matches_action_after_section) {
    vxui_ctx ctx = make_ctx();

    // Menu = [section, action]. Frame 1 establishes; frame 2 settles focus to row 1.
    for ( int i = 0; i < 60; i++ )
    {
        vxui_frame( &ctx, 1.0f / 60.0f );
        if ( vxui_menu( &ctx, "m" ) )
        {
            vxui_menu_section( &ctx, "Header" );
            vxui_menu_action ( &ctx, "Play" );
            vxui_menu_end( &ctx );
        }
        vxui_render( &ctx );
    }

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_section( &ctx, "Header" );
        vxui_menu_action ( &ctx, "Play" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 3 );
    // Focus rect at row 1 (Play, y=32), not row 0 (Header, y=0).
    ASSERT_NEAR( dl.cmds[2].rect.y, (float) VXUI_ROW_HEIGHT, 1e-2f );
    ASSERT_NEAR( dl.cmds[2].rect.y, dl.cmds[1].rect.y,        1e-2f );
}

UTEST(menu_draw, focus_rect_y_matches_action_between_section_and_label) {
    vxui_ctx ctx = make_ctx();

    // Menu = [section, action, label]. Focus settles on action (row 1, y=32).
    for ( int i = 0; i < 60; i++ )
    {
        vxui_frame( &ctx, 1.0f / 60.0f );
        if ( vxui_menu( &ctx, "m" ) )
        {
            vxui_menu_section( &ctx, "Top" );
            vxui_menu_action ( &ctx, "Play" );
            vxui_menu_label  ( &ctx, "Note" );
            vxui_menu_end( &ctx );
        }
        vxui_render( &ctx );
    }

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_section( &ctx, "Top" );
        vxui_menu_action ( &ctx, "Play" );
        vxui_menu_label  ( &ctx, "Note" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 4 );
    ASSERT_NEAR( dl.cmds[3].rect.y, (float) VXUI_ROW_HEIGHT, 1e-2f );
    ASSERT_NEAR( dl.cmds[3].rect.y, dl.cmds[1].rect.y,        1e-2f );
}

UTEST(menu_draw, focus_rect_y_at_third_action_after_two_sections) {
    vxui_ctx ctx = make_ctx();

    // Menu = [section, label, action]. Focus settles on action (row 2, y=64).
    for ( int i = 0; i < 60; i++ )
    {
        vxui_frame( &ctx, 1.0f / 60.0f );
        if ( vxui_menu( &ctx, "m" ) )
        {
            vxui_menu_section( &ctx, "Top" );
            vxui_menu_label  ( &ctx, "Note" );
            vxui_menu_action ( &ctx, "Play" );
            vxui_menu_end( &ctx );
        }
        vxui_render( &ctx );
    }

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_section( &ctx, "Top" );
        vxui_menu_label  ( &ctx, "Note" );
        vxui_menu_action ( &ctx, "Play" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 4 );
    ASSERT_NEAR( dl.cmds[3].rect.y, 2.0f * (float) VXUI_ROW_HEIGHT, 1e-2f );
    ASSERT_NEAR( dl.cmds[3].rect.y, dl.cmds[2].rect.y,                1e-2f );
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
