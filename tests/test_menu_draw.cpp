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

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 2 );
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

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 4 );
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

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 2 );
    ASSERT_EQ( vxui_draw_nth( dl, VXUI_DRAW_RECT, 0 )->id, row_id( "m", "Play" ) );
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

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 4 );
    ASSERT_EQ( vxui_draw_nth( dl, VXUI_DRAW_RECT, 0 )->id, row_id( "m", "Play" ) );
    ASSERT_EQ( vxui_draw_nth( dl, VXUI_DRAW_RECT, 1 )->id, row_id( "m", "Options" ) );
    ASSERT_EQ( vxui_draw_nth( dl, VXUI_DRAW_RECT, 2 )->id, row_id( "m", "Quit" ) );
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

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 4 );
    ASSERT_EQ( vxui_draw_nth( dl, VXUI_DRAW_RECT, 0 )->id, row_id( "left",  "Back" ) );
    ASSERT_EQ( vxui_draw_nth( dl, VXUI_DRAW_RECT, 1 )->id, row_id( "right", "Back" ) );
    ASSERT_NE( vxui_draw_nth( dl, VXUI_DRAW_RECT, 0 )->id, vxui_draw_nth( dl, VXUI_DRAW_RECT, 1 )->id );
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

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 2 );
    ASSERT_NEAR( vxui_draw_nth( dl, VXUI_DRAW_RECT, 0 )->rect.w, (float) VXUI_ROW_HEIGHT, 1e-3f );  // rect.w = height (Clay h)
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

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 2 );
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

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 2 );
    ASSERT_EQ( vxui_draw_nth( dl, VXUI_DRAW_RECT, 0 )->id, row_id( "m", "Difficulty" ) );
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

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 2 );
    ASSERT_NEAR( vxui_draw_nth( dl, VXUI_DRAW_RECT, 0 )->rect.w, (float) VXUI_ROW_HEIGHT, 1e-3f );
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

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 3 );
    ASSERT_EQ( vxui_draw_nth( dl, VXUI_DRAW_RECT, 0 )->id, row_id( "m", "Play" ) );
    ASSERT_EQ( vxui_draw_nth( dl, VXUI_DRAW_RECT, 1 )->id, row_id( "m", "Difficulty" ) );
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

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 2 );
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

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 2 );
    ASSERT_EQ( vxui_draw_nth( dl, VXUI_DRAW_RECT, 0 )->id, row_id( "m", "Volume" ) );
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

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 2 );
    ASSERT_NEAR( vxui_draw_nth( dl, VXUI_DRAW_RECT, 0 )->rect.w, (float) VXUI_ROW_HEIGHT, 1e-3f );
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

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 4 );
    ASSERT_EQ( vxui_draw_nth( dl, VXUI_DRAW_RECT, 0 )->id, row_id( "m", "Play" ) );
    ASSERT_EQ( vxui_draw_nth( dl, VXUI_DRAW_RECT, 1 )->id, row_id( "m", "Difficulty" ) );
    ASSERT_EQ( vxui_draw_nth( dl, VXUI_DRAW_RECT, 2 )->id, row_id( "m", "Volume" ) );
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

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 1 );
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

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 1 );
    ASSERT_EQ( vxui_draw_nth( dl, VXUI_DRAW_RECT, 0 )->id, row_id( "m", "Header" ) );
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

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 1 );
    ASSERT_NEAR( vxui_draw_nth( dl, VXUI_DRAW_RECT, 0 )->rect.w, (float) VXUI_ROW_HEIGHT, 1e-3f );
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

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 1 );
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

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 1 );
    ASSERT_EQ( vxui_draw_nth( dl, VXUI_DRAW_RECT, 0 )->id, row_id( "m", "Info text" ) );
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

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 1 );
    ASSERT_NEAR( vxui_draw_nth( dl, VXUI_DRAW_RECT, 0 )->rect.w, (float) VXUI_ROW_HEIGHT, 1e-3f );
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

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 2 );
    ASSERT_NEAR( vxui_draw_nth( dl, VXUI_DRAW_RECT, 0 )->rect.y, 0.0f, 1e-3f );
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

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 3 );
    ASSERT_NEAR( vxui_draw_nth( dl, VXUI_DRAW_RECT, 1 )->rect.y, vxui_draw_nth( dl, VXUI_DRAW_RECT, 0 )->rect.y + (float) VXUI_ROW_HEIGHT, 1e-3f );
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

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 4 );
    ASSERT_NEAR( vxui_draw_nth( dl, VXUI_DRAW_RECT, 0 )->rect.y, 0.0f,                              1e-3f );
    ASSERT_NEAR( vxui_draw_nth( dl, VXUI_DRAW_RECT, 1 )->rect.y, 1.0f * (float) VXUI_ROW_HEIGHT,    1e-3f );
    ASSERT_NEAR( vxui_draw_nth( dl, VXUI_DRAW_RECT, 2 )->rect.y, 2.0f * (float) VXUI_ROW_HEIGHT,    1e-3f );
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

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 3 );
    ASSERT_NEAR( vxui_draw_nth( dl, VXUI_DRAW_RECT, 0 )->rect.x, vxui_draw_nth( dl, VXUI_DRAW_RECT, 1 )->rect.x, 1e-3f );
}

UTEST(menu_draw, all_row_types_emit_rects) {
    vxui_ctx ctx = make_ctx();
    int idx = 0;
    float v = 0.5f;
    const char* opts[] = { "A", "B" };

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

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 6 );
    ASSERT_EQ( vxui_draw_nth( dl, VXUI_DRAW_RECT, 0 )->id, row_id( "m", "Header" ) );
    ASSERT_EQ( vxui_draw_nth( dl, VXUI_DRAW_RECT, 1 )->id, row_id( "m", "Info" ) );
    ASSERT_EQ( vxui_draw_nth( dl, VXUI_DRAW_RECT, 2 )->id, row_id( "m", "Play" ) );
    ASSERT_EQ( vxui_draw_nth( dl, VXUI_DRAW_RECT, 3 )->id, row_id( "m", "Difficulty" ) );
    ASSERT_EQ( vxui_draw_nth( dl, VXUI_DRAW_RECT, 4 )->id, row_id( "m", "Volume" ) );
}


UTEST(menu_draw, focus_rect_present_when_action_focusable) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 2 );
    ASSERT_EQ( vxui_draw_nth( dl, VXUI_DRAW_RECT, 1 )->id, focus_id( "m" ) );
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

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 2 );
    ASSERT_EQ( vxui_draw_nth( dl, VXUI_DRAW_RECT, 1 )->id, focus_id( "settings" ) );
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

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 3 );
    ASSERT_NEAR( vxui_draw_nth( dl, VXUI_DRAW_RECT, 2 )->rect.y, vxui_draw_nth( dl, VXUI_DRAW_RECT, 0 )->rect.y, 1e-3f );
}

UTEST(menu_draw, focus_rect_settles_after_many_frames) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_action( &ctx, "Quit" );
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );

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

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 3 );
    ASSERT_NEAR( vxui_draw_nth( dl, VXUI_DRAW_RECT, 2 )->rect.y, vxui_draw_nth( dl, VXUI_DRAW_RECT, 0 )->rect.y, 1e-3f );
}

UTEST(menu_draw, focus_rect_animates_after_navigation) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_action( &ctx, "Quit" );
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );

    vxui_frame( &ctx, 1.0f / 60.0f );
    ctx.input = VXUI_INPUT_DOWN;
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_action( &ctx, "Quit" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 3 );
    float row0_y = vxui_draw_nth( dl, VXUI_DRAW_RECT, 0 )->rect.y;
    float row1_y = vxui_draw_nth( dl, VXUI_DRAW_RECT, 1 )->rect.y;
    float focus_y = vxui_draw_nth( dl, VXUI_DRAW_RECT, 2 )->rect.y;
    ASSERT_GT( focus_y, row0_y - 1e-3f );
    ASSERT_LT( focus_y, row1_y + 1e-3f );
    ASSERT_GT( focus_y, row0_y + 1e-3f );
    ASSERT_LT( focus_y, row1_y - 1e-3f );
}

UTEST(menu_draw, focus_rect_settles_after_navigation) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_action( &ctx, "Quit" );
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );

    vxui_frame( &ctx, 1.0f / 60.0f );
    ctx.input = VXUI_INPUT_DOWN;
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_action( &ctx, "Quit" );
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );

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

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 3 );
    ASSERT_NEAR( vxui_draw_nth( dl, VXUI_DRAW_RECT, 2 )->rect.y, vxui_draw_nth( dl, VXUI_DRAW_RECT, 1 )->rect.y, 1e-2f );
}

UTEST(menu_draw, focus_rect_first_frame_snaps) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_action( &ctx, "Quit" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 3 );
    ASSERT_NEAR( vxui_draw_nth( dl, VXUI_DRAW_RECT, 2 )->rect.y, vxui_draw_nth( dl, VXUI_DRAW_RECT, 0 )->rect.y, 1e-3f );
}

UTEST(menu_draw, focus_spring_per_menu_isolation) {
    vxui_ctx ctx = make_ctx();

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

    ASSERT_NEAR( ctx.menu_focus_spring[0].x, 0.0f, 1e-2f );
    ASSERT_NEAR( ctx.menu_focus_spring[1].x, 0.0f, 1e-2f );
}

UTEST(menu_draw, focus_rect_skips_section_row) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_section( &ctx, "Header" );
        vxui_menu_action ( &ctx, "Play" );
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_section( &ctx, "Header" );
        vxui_menu_action ( &ctx, "Play" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 3 );
    ASSERT_NEAR( vxui_draw_nth( dl, VXUI_DRAW_RECT, 2 )->rect.y, vxui_draw_nth( dl, VXUI_DRAW_RECT, 1 )->rect.y, 1e-3f );
    ASSERT_NE  ( vxui_draw_nth( dl, VXUI_DRAW_RECT, 2 )->id,    vxui_draw_nth( dl, VXUI_DRAW_RECT, 0 )->id );
}

UTEST(menu_draw, section_then_action_action_below) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_section( &ctx, "Header" );
        vxui_menu_action ( &ctx, "Play" );
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_section( &ctx, "Header" );
        vxui_menu_action ( &ctx, "Play" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 3 );
    ASSERT_NEAR( vxui_draw_nth( dl, VXUI_DRAW_RECT, 0 )->rect.y, 0.0f,                            1e-3f );
    ASSERT_NEAR( vxui_draw_nth( dl, VXUI_DRAW_RECT, 1 )->rect.y, 1.0f * (float) VXUI_ROW_HEIGHT,  1e-3f );
}

UTEST(menu_draw, label_then_action_action_below) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_label ( &ctx, "Info" );
        vxui_menu_action( &ctx, "Play" );
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

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 3 );
    ASSERT_NEAR( vxui_draw_nth( dl, VXUI_DRAW_RECT, 0 )->rect.y, 0.0f,                            1e-3f );
    ASSERT_NEAR( vxui_draw_nth( dl, VXUI_DRAW_RECT, 1 )->rect.y, 1.0f * (float) VXUI_ROW_HEIGHT,  1e-3f );
}

UTEST(menu_draw, interleaved_skip_rows_stack) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_section( &ctx, "Top" );
        vxui_menu_action ( &ctx, "Play" );
        vxui_menu_label  ( &ctx, "Note" );
        vxui_menu_action ( &ctx, "Quit" );
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

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 5 );
    ASSERT_NEAR( vxui_draw_nth( dl, VXUI_DRAW_RECT, 0 )->rect.y, 0.0f,                            1e-3f );
    ASSERT_NEAR( vxui_draw_nth( dl, VXUI_DRAW_RECT, 1 )->rect.y, 1.0f * (float) VXUI_ROW_HEIGHT,  1e-3f );
    ASSERT_NEAR( vxui_draw_nth( dl, VXUI_DRAW_RECT, 2 )->rect.y, 2.0f * (float) VXUI_ROW_HEIGHT,  1e-3f );
    ASSERT_NEAR( vxui_draw_nth( dl, VXUI_DRAW_RECT, 3 )->rect.y, 3.0f * (float) VXUI_ROW_HEIGHT,  1e-3f );
}

UTEST(menu_draw, focus_rect_y_matches_action_after_section) {
    vxui_ctx ctx = make_ctx();

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

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 3 );
    ASSERT_NEAR( vxui_draw_nth( dl, VXUI_DRAW_RECT, 2 )->rect.y, (float) VXUI_ROW_HEIGHT, 1e-2f );
    ASSERT_NEAR( vxui_draw_nth( dl, VXUI_DRAW_RECT, 2 )->rect.y, vxui_draw_nth( dl, VXUI_DRAW_RECT, 1 )->rect.y,        1e-2f );
}

UTEST(menu_draw, focus_rect_y_matches_action_between_section_and_label) {
    vxui_ctx ctx = make_ctx();

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

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 4 );
    ASSERT_NEAR( vxui_draw_nth( dl, VXUI_DRAW_RECT, 3 )->rect.y, (float) VXUI_ROW_HEIGHT, 1e-2f );
    ASSERT_NEAR( vxui_draw_nth( dl, VXUI_DRAW_RECT, 3 )->rect.y, vxui_draw_nth( dl, VXUI_DRAW_RECT, 1 )->rect.y,        1e-2f );
}

UTEST(menu_draw, focus_rect_y_at_third_action_after_two_sections) {
    vxui_ctx ctx = make_ctx();

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

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 4 );
    ASSERT_NEAR( vxui_draw_nth( dl, VXUI_DRAW_RECT, 3 )->rect.y, 2.0f * (float) VXUI_ROW_HEIGHT, 1e-2f );
    ASSERT_NEAR( vxui_draw_nth( dl, VXUI_DRAW_RECT, 3 )->rect.y, vxui_draw_nth( dl, VXUI_DRAW_RECT, 2 )->rect.y,                1e-2f );
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

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 2 );
}

UTEST(menu_draw, focus_present_on_frame_1_with_leading_section) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_section( &ctx, "Header" );
        vxui_menu_action ( &ctx, "Play" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 3 );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 1 );
}

UTEST(menu_draw, focus_present_on_frame_1_with_leading_label) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_label ( &ctx, "Info" );
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 3 );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 1 );
}

UTEST(menu_draw, current_row_promoted_past_multiple_skip_rows) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_section( &ctx, "S" );
        vxui_menu_label  ( &ctx, "L" );
        vxui_menu_action ( &ctx, "A" );
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );

    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 2 );
}

UTEST(menu_draw, only_first_interactive_row_gets_promoted_focus) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_section( &ctx, "S" );
        vxui_menu_action ( &ctx, "A1" );
        vxui_menu_action ( &ctx, "A2" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 4 );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 1 );
    ASSERT_NEAR( vxui_draw_nth( dl, VXUI_DRAW_RECT, 3 )->rect.y, vxui_draw_nth( dl, VXUI_DRAW_RECT, 1 )->rect.y, 1e-3f );
}

UTEST(menu_draw, action_first_does_not_get_promoted) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action ( &ctx, "A" );
        vxui_menu_section( &ctx, "S" );
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );

    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 0 );
}

UTEST(menu_draw, promoted_focus_works_with_option) {
    vxui_ctx ctx = make_ctx();

    static const char* keys[] = { "L", "M", "H" };
    int idx = 1;

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_section( &ctx, "S" );
        vxui_menu_option ( &ctx, "Diff", &idx, keys, 3 );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 3 );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 1 );
}

UTEST(menu_draw, promoted_focus_works_with_slider) {
    vxui_ctx ctx = make_ctx();

    float val = 0.5f;

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_section( &ctx, "S" );
        vxui_menu_slider ( &ctx, "Vol", &val, 0.0f, 1.0f, 0.1f );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 3 );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 1 );
}

UTEST(menu_draw, promoted_action_confirm_fires_on_frame_1) {
    vxui_ctx ctx = make_ctx();

    bool fired = false;
    vxui_frame( &ctx, 1.0f / 60.0f );
    ctx.input = VXUI_INPUT_CONFIRM;
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_section( &ctx, "S" );
        if ( vxui_menu_action( &ctx, "Play" ) ) fired = true;
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );

    ASSERT_TRUE( fired );
}

UTEST(menu_draw, all_five_row_types_with_distinct_labels) {
    vxui_ctx ctx = make_ctx();

    int idx = 0;
    float v = 0.5f;
    const char* opts[] = { "A", "B" };

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_section( &ctx, "Top" );
        vxui_menu_label  ( &ctx, "Info" );
        vxui_menu_action ( &ctx, "Play" );
        vxui_menu_option ( &ctx, "Diff", &idx, opts, 2 );
        vxui_menu_slider ( &ctx, "Vol",  &v, 0.0f, 1.0f, 0.1f );
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );

    ASSERT_EQ( ctx.active_menu_row_ids[0], row_id( "m", "Top"  ) );
    ASSERT_EQ( ctx.active_menu_row_ids[1], row_id( "m", "Info" ) );
    ASSERT_EQ( ctx.active_menu_row_ids[2], row_id( "m", "Play" ) );
    ASSERT_EQ( ctx.active_menu_row_ids[3], row_id( "m", "Diff" ) );
    ASSERT_EQ( ctx.active_menu_row_ids[4], row_id( "m", "Vol"  ) );
}

UTEST(menu_draw, row_ids_reset_between_frames) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_action( &ctx, "Quit" );
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_action( &ctx, "Quit" );
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );

    ASSERT_EQ( ctx.active_menu_row_ids[0], row_id( "m", "Play" ) );
    ASSERT_EQ( ctx.active_menu_row_ids[1], row_id( "m", "Quit" ) );
}

UTEST(menu_draw, row_ids_isolated_across_two_menus) {
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
    vxui_render( &ctx );

    ASSERT_EQ( ctx.active_menu_row_ids[0], row_id( "right", "Back" ) );
}

UTEST(menu_draw, empty_menu_emits_no_rects) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 0 );
}

UTEST(menu_draw, empty_menu_records_zero_rows) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );

    ASSERT_EQ( ctx.menu_state[0].z, (uint32_t) 0 );
    ASSERT_EQ( ctx.menu_state[0].w, (uint32_t) 0 );
}

UTEST(menu_draw, empty_menu_then_populated_works) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_action( &ctx, "Quit" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 3 );
    ASSERT_EQ( ctx.menu_state[0].z, (uint32_t) 2 );
}

UTEST(menu_draw, empty_menu_no_focus_rect) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    for ( int i = 0; i < vxui_draw_count( dl, VXUI_DRAW_RECT ); i++ )
        ASSERT_NE( vxui_draw_nth( dl, VXUI_DRAW_RECT, i )->id, focus_id( "m" ) );
}

UTEST(menu_draw, focus_rect_width_matches_row_width) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 2 );
    ASSERT_NEAR( vxui_draw_nth( dl, VXUI_DRAW_RECT, 1 )->rect.z, vxui_draw_nth( dl, VXUI_DRAW_RECT, 0 )->rect.z, 1e-3f );  // rect.z = width (Clay w)
}

UTEST(menu_draw, focus_rect_x_matches_row_x) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 2 );
    ASSERT_NEAR( vxui_draw_nth( dl, VXUI_DRAW_RECT, 1 )->rect.x, vxui_draw_nth( dl, VXUI_DRAW_RECT, 0 )->rect.x, 1e-3f );
}

UTEST(menu_draw, focus_rect_height_matches_row_height) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 2 );
    ASSERT_NEAR( vxui_draw_nth( dl, VXUI_DRAW_RECT, 1 )->rect.w, vxui_draw_nth( dl, VXUI_DRAW_RECT, 0 )->rect.w, 1e-3f );
}

UTEST(menu_draw, focus_rect_width_tracks_wider_label) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_action( &ctx, "Continue Playing" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 3 );
    ASSERT_NEAR( vxui_draw_nth( dl, VXUI_DRAW_RECT, 0 )->rect.z, vxui_draw_nth( dl, VXUI_DRAW_RECT, 1 )->rect.z, 1e-3f );
    ASSERT_NEAR( vxui_draw_nth( dl, VXUI_DRAW_RECT, 2 )->rect.z, vxui_draw_nth( dl, VXUI_DRAW_RECT, 0 )->rect.z, 1e-3f );
}

UTEST(menu_draw, distinct_menu_names_get_distinct_slots) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "left" ) )
    {
        vxui_menu_action( &ctx, "A" );
        vxui_menu_end( &ctx );
    }
    if ( vxui_menu( &ctx, "right" ) )
    {
        vxui_menu_action( &ctx, "A" );
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );

    ASSERT_EQ( ctx.menu_count, 2 );
    ASSERT_NE( ctx.menu_state[0].x, ctx.menu_state[1].x );
}

UTEST(menu_draw, menu_state_persists_by_hash_id) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "A" );
        vxui_menu_action( &ctx, "B" );
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );

    vxui_frame( &ctx, 1.0f / 60.0f );
    ctx.input = VXUI_INPUT_DOWN;
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "A" );
        vxui_menu_action( &ctx, "B" );
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );

    ASSERT_EQ( ctx.menu_count, 1 );
    ASSERT_EQ( ctx.menu_state[0].x, vxui_hash( "m" ) );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 1 );
}

UTEST(menu_draw, two_menus_independent_focus) {
    vxui_ctx ctx = make_ctx();

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

    vxui_frame( &ctx, 1.0f / 60.0f );
    ctx.input = VXUI_INPUT_DOWN;
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

    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 1 );
    ASSERT_EQ( ctx.menu_state[1].y, (uint32_t) 1 );
}

UTEST_MAIN();