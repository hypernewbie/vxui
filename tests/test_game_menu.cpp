#include "utest.h"
#include "../vxui.h"
#define VXUI_IMPL
#include "../vxui_impl.h"
#include <cstdio>

static uint8_t s_clay_mem[16 * 1024 * 1024];

static const char* lookup_label( uint32_t id, const char* menu_name, const char** labels )
{
    for ( int i = 0; labels[i]; i++ )
    {
        Clay_String cs = { false, (int32_t) strlen( labels[i] ), labels[i] };
        if ( Clay__HashString( cs, vxui_hash( menu_name ) ).id == id ) return labels[i];
    }
    Clay_String fcs = CLAY_STRING( "focus" );
    if ( Clay__HashString( fcs, vxui_hash( menu_name ) ).id == id ) return "<focus>";
    return "?";
}

static void plot_layout( const char* title, const vxui_draw_list& dl, const char* menu_name, const char** labels )
{
    int rect_n = vxui_draw_count( dl, VXUI_DRAW_RECT );
    printf( "\n  -- %s -- (%d cmds)\n", title, rect_n );
    for ( int i = 0; i < rect_n; i++ )
    {
        const vxui_draw_cmd& c = *vxui_draw_nth( dl, VXUI_DRAW_RECT, i );
        const char* lbl = lookup_label( c.id, menu_name, labels );
        printf( "    [%2d]  y=%6.1f  h=%6.1f  %-20s\n", i, c.rect.y, c.rect.w, lbl );
    }

    float max_y = 0;
    for ( int i = 0; i < rect_n; i++ )
    {
        const vxui_draw_cmd& c = *vxui_draw_nth( dl, VXUI_DRAW_RECT, i );
        if ( c.rect.y + c.rect.w > max_y ) max_y = c.rect.y + c.rect.w;
    }

    int n_lines = (int) ( max_y / VXUI_ROW_HEIGHT ) + 1;
    printf( "\n  layout strip:\n" );
    for ( int line = 0; line < n_lines; line++ )
    {
        float line_y = (float) ( line * VXUI_ROW_HEIGHT );
        const char* row_lbl = "";
        bool has_focus = false;
        for ( int i = 0; i < rect_n; i++ )
        {
            const vxui_draw_cmd& c = *vxui_draw_nth( dl, VXUI_DRAW_RECT, i );
            if ( c.rect.y >= line_y - 1.0f && c.rect.y < line_y + (float) VXUI_ROW_HEIGHT - 1.0f )
            {
                const char* lbl = lookup_label( c.id, menu_name, labels );
                if ( strcmp( lbl, "<focus>" ) == 0 ) has_focus = true;
                else                                  row_lbl   = lbl;
            }
        }
        printf( "    %3d  %s%-20s%s\n", line * VXUI_ROW_HEIGHT,
                has_focus ? "> " : "  ",
                row_lbl,
                has_focus ? "  <" : "" );
    }
    printf( "\n" );
}

static vxui_ctx make_ctx()
{
    vxui_ctx ctx = {};
    vxui_init( &ctx, 1280, 720, s_clay_mem, sizeof( s_clay_mem ) );
    return ctx;
}

static uint32_t row_id( const char* menu, const char* label )
{
    Clay_String cs = { false, (int32_t) strlen( label ), label };
    return Clay__HashString( cs, vxui_hash( menu ) ).id;
}

static uint32_t focus_id( const char* menu )
{
    Clay_String cs = CLAY_STRING( "focus" );
    return Clay__HashString( cs, vxui_hash( menu ) ).id;
}

static int title_frame( vxui_ctx* ctx, uint32_t input )
{
    int chosen = -1;
    vxui_frame( ctx, 1.0f / 60.0f );
    ctx->input = input;
    if ( vxui_menu( ctx, "title" ) )
    {
        if ( vxui_menu_action( ctx, "Play"        ) ) chosen = 0;
        if ( vxui_menu_action( ctx, "Missions"    ) ) chosen = 1;
        if ( vxui_menu_action( ctx, "Stage Select") ) chosen = 2;
        if ( vxui_menu_action( ctx, "Options"     ) ) chosen = 3;
        if ( vxui_menu_action( ctx, "Unlocks"     ) ) chosen = 4;
        if ( vxui_menu_action( ctx, "Extras"      ) ) chosen = 5;
        if ( vxui_menu_action( ctx, "Quit"        ) ) chosen = 6;
        vxui_menu_end( ctx );
    }
    vxui_render( ctx );
    ctx->input = 0;   // release so next call registers as a fresh edge
    return chosen;
}

UTEST(game_title, has_seven_rows_plus_focus) {
    vxui_ctx ctx = make_ctx();
    title_frame( &ctx, 0 );
    vxui_draw_list dl = ctx.draw_list;

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 8 );
}

UTEST(game_title, all_seven_rows_have_correct_ids) {
    vxui_ctx ctx = make_ctx();
    title_frame( &ctx, 0 );
    vxui_draw_list dl = ctx.draw_list;

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 8 );
    ASSERT_EQ( vxui_draw_nth( dl, VXUI_DRAW_RECT, 0 )->id, row_id( "title", "Play"         ) );
    ASSERT_EQ( vxui_draw_nth( dl, VXUI_DRAW_RECT, 1 )->id, row_id( "title", "Missions"     ) );
    ASSERT_EQ( vxui_draw_nth( dl, VXUI_DRAW_RECT, 2 )->id, row_id( "title", "Stage Select" ) );
    ASSERT_EQ( vxui_draw_nth( dl, VXUI_DRAW_RECT, 3 )->id, row_id( "title", "Options"      ) );
    ASSERT_EQ( vxui_draw_nth( dl, VXUI_DRAW_RECT, 4 )->id, row_id( "title", "Unlocks"      ) );
    ASSERT_EQ( vxui_draw_nth( dl, VXUI_DRAW_RECT, 5 )->id, row_id( "title", "Extras"       ) );
    ASSERT_EQ( vxui_draw_nth( dl, VXUI_DRAW_RECT, 6 )->id, row_id( "title", "Quit"         ) );
    ASSERT_EQ( vxui_draw_nth( dl, VXUI_DRAW_RECT, 7 )->id, focus_id( "title"               ) );
}

UTEST(game_title, all_rows_stack_at_row_height_intervals) {
    vxui_ctx ctx = make_ctx();
    title_frame( &ctx, 0 );
    vxui_draw_list dl = ctx.draw_list;

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 8 );
    for ( int i = 0; i < 7; i++ )
        ASSERT_NEAR( vxui_draw_nth( dl, VXUI_DRAW_RECT, i )->rect.y, (float) ( i * VXUI_ROW_HEIGHT ), 1e-3f );
}

UTEST(game_title, all_rows_have_fixed_height) {
    vxui_ctx ctx = make_ctx();
    title_frame( &ctx, 0 );
    vxui_draw_list dl = ctx.draw_list;

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 8 );
    for ( int i = 0; i < 7; i++ )
        ASSERT_NEAR( vxui_draw_nth( dl, VXUI_DRAW_RECT, i )->rect.w, (float) VXUI_ROW_HEIGHT, 1e-3f );
}

UTEST(game_title, navigate_play_to_quit_via_down) {
    vxui_ctx ctx = make_ctx();
    title_frame( &ctx, 0 );

    for ( int i = 0; i < 6; i++ )
        title_frame( &ctx, VXUI_INPUT_DOWN );

    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 6 );
}

UTEST(game_title, navigate_play_to_quit_via_up_wrap) {
    vxui_ctx ctx = make_ctx();
    title_frame( &ctx, 0 );

    title_frame( &ctx, VXUI_INPUT_UP );

    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 6 );
}

UTEST(game_title, confirm_play_fires_play) {
    vxui_ctx ctx = make_ctx();
    title_frame( &ctx, 0 );

    int chosen = title_frame( &ctx, VXUI_INPUT_CONFIRM );
    ASSERT_EQ( chosen, 0 );
}

UTEST(game_title, confirm_quit_fires_quit) {
    vxui_ctx ctx = make_ctx();
    title_frame( &ctx, 0 );

    title_frame( &ctx, VXUI_INPUT_UP );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 6 );

    int chosen = title_frame( &ctx, VXUI_INPUT_CONFIRM );
    ASSERT_EQ( chosen, 6 );
}

UTEST(game_title, focus_settles_on_quit_after_navigation) {
    vxui_ctx ctx = make_ctx();
    title_frame( &ctx, 0 );
    title_frame( &ctx, VXUI_INPUT_UP );

    for ( int i = 0; i < 120; i++ )
        title_frame( &ctx, 0 );

    vxui_draw_list dl = ctx.draw_list;
    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 8 );
    ASSERT_NEAR( vxui_draw_nth( dl, VXUI_DRAW_RECT, 7 )->rect.y, 6.0f * (float) VXUI_ROW_HEIGHT, 1e-2f );
}

static const char* s_diff_keys [] = { "Easy", "Normal", "Hard" };
static const char* s_speed_keys[] = { "Slow", "Medium", "Fast" };
static const char* s_on_off    [] = { "Off",  "On" };

struct gameplay_state
{
    int  diff        = 1;
    int  speed       = 1;
    int  scoring     = 0;
    bool apply_fired = false;
};

static void gameplay_frame( vxui_ctx* ctx, gameplay_state* s, uint32_t input )
{
    s->apply_fired = false;
    vxui_frame( ctx, 1.0f / 60.0f );
    ctx->input = input;
    if ( vxui_menu( ctx, "gameplay" ) )
    {
        vxui_menu_section( ctx, "GAMEPLAY" );
        vxui_menu_option ( ctx, "Difficulty",        &s->diff,    s_diff_keys,  3 );
        vxui_menu_option ( ctx, "Speed",             &s->speed,   s_speed_keys, 3 );
        vxui_menu_option ( ctx, "Show Scoring Info", &s->scoring, s_on_off,     2 );
        vxui_menu_label  ( ctx, "Changes apply on restart" );
        s->apply_fired = vxui_menu_action( ctx, "Apply" );
        vxui_menu_end( ctx );
    }
    vxui_render( ctx );
    ctx->input = 0;   // release so next call registers as a fresh edge
}

UTEST(game_gameplay, six_rows_plus_focus) {
    vxui_ctx ctx = make_ctx();
    gameplay_state s;
    gameplay_frame( &ctx, &s, 0 );

    vxui_draw_list dl = ctx.draw_list;
    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 7 );
}

UTEST(game_gameplay, focus_starts_on_first_interactive_row) {
    vxui_ctx ctx = make_ctx();
    gameplay_state s;
    gameplay_frame( &ctx, &s, 0 );

    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 1 );
}

UTEST(game_gameplay, navigate_skips_section_and_label) {
    vxui_ctx ctx = make_ctx();
    gameplay_state s;
    gameplay_frame( &ctx, &s, 0 );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 1 );

    gameplay_frame( &ctx, &s, VXUI_INPUT_DOWN );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 2 );

    gameplay_frame( &ctx, &s, VXUI_INPUT_DOWN );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 3 );

    gameplay_frame( &ctx, &s, VXUI_INPUT_DOWN );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 5 );

    gameplay_frame( &ctx, &s, VXUI_INPUT_UP );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 3 );

    gameplay_frame( &ctx, &s, VXUI_INPUT_UP );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 2 );

    gameplay_frame( &ctx, &s, VXUI_INPUT_UP );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 1 );

    gameplay_frame( &ctx, &s, VXUI_INPUT_UP );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 5 );
}

UTEST(game_gameplay, option_left_right_changes_value_and_returns_true) {
    vxui_ctx ctx = make_ctx();
    gameplay_state s;
    gameplay_frame( &ctx, &s, 0 );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 1 );
    ASSERT_EQ( s.diff, 1 );

    gameplay_frame( &ctx, &s, VXUI_INPUT_RIGHT );
    ASSERT_EQ( s.diff, 2 );
}

UTEST(game_gameplay, all_three_options_cycle_independently) {
    vxui_ctx ctx = make_ctx();
    gameplay_state s;
    gameplay_frame( &ctx, &s, 0 );
    ASSERT_EQ( s.diff, 1 );

    gameplay_frame( &ctx, &s, VXUI_INPUT_RIGHT );
    ASSERT_EQ( s.diff, 2 );

    gameplay_frame( &ctx, &s, VXUI_INPUT_DOWN );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 2 );

    gameplay_frame( &ctx, &s, VXUI_INPUT_RIGHT );
    ASSERT_EQ( s.speed, 2 );

    gameplay_frame( &ctx, &s, VXUI_INPUT_DOWN );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 3 );

    gameplay_frame( &ctx, &s, VXUI_INPUT_RIGHT );
    ASSERT_EQ( s.scoring, 1 );

    ASSERT_EQ( s.diff,  2 );
    ASSERT_EQ( s.speed, 2 );
}

UTEST(game_gameplay, apply_action_fires_only_when_focused) {
    vxui_ctx ctx = make_ctx();
    gameplay_state s;
    gameplay_frame( &ctx, &s, 0 );
    ASSERT_FALSE( s.apply_fired );

    gameplay_frame( &ctx, &s, VXUI_INPUT_CONFIRM );
    ASSERT_FALSE( s.apply_fired );

    while ( ctx.menu_state[0].y != 5 )
        gameplay_frame( &ctx, &s, VXUI_INPUT_DOWN );

    gameplay_frame( &ctx, &s, VXUI_INPUT_CONFIRM );
    ASSERT_TRUE( s.apply_fired );
}

UTEST(game_gameplay, all_six_rows_stack_vertically) {
    vxui_ctx ctx = make_ctx();
    gameplay_state s;
    gameplay_frame( &ctx, &s, 0 );
    vxui_draw_list dl = ctx.draw_list;

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 7 );
    for ( int i = 0; i < 6; i++ )
        ASSERT_NEAR( vxui_draw_nth( dl, VXUI_DRAW_RECT, i )->rect.y, (float) ( i * VXUI_ROW_HEIGHT ), 1e-3f );
}

UTEST(game_gameplay, focus_rect_settles_on_apply_after_full_navigation) {
    vxui_ctx ctx = make_ctx();
    gameplay_state s;
    gameplay_frame( &ctx, &s, 0 );

    while ( ctx.menu_state[0].y != 5 )
        gameplay_frame( &ctx, &s, VXUI_INPUT_DOWN );

    for ( int i = 0; i < 120; i++ )
        gameplay_frame( &ctx, &s, 0 );

    vxui_draw_list dl = ctx.draw_list;
    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 7 );
    ASSERT_NEAR( vxui_draw_nth( dl, VXUI_DRAW_RECT, 6 )->rect.y, 5.0f * (float) VXUI_ROW_HEIGHT, 1e-2f );
}

UTEST(plot, title_menu_default_focus) {
    vxui_ctx ctx = make_ctx();
    title_frame( &ctx, 0 );
    static const char* labels[] = {
        "Play", "Missions", "Stage Select", "Options",
        "Unlocks", "Extras", "Quit", nullptr
    };
    plot_layout( "title menu, focus on Play", ctx.draw_list, "title", labels );
}

UTEST(plot, title_menu_focus_on_quit_settled) {
    vxui_ctx ctx = make_ctx();
    title_frame( &ctx, 0 );
    title_frame( &ctx, VXUI_INPUT_UP );
    for ( int i = 0; i < 120; i++ )
        title_frame( &ctx, 0 );

    static const char* labels[] = {
        "Play", "Missions", "Stage Select", "Options",
        "Unlocks", "Extras", "Quit", nullptr
    };
    plot_layout( "title menu, focus settled on Quit", ctx.draw_list, "title", labels );
}

UTEST(plot, title_menu_focus_mid_animation) {
    vxui_ctx ctx = make_ctx();
    title_frame( &ctx, 0 );
    title_frame( &ctx, VXUI_INPUT_DOWN );

    static const char* labels[] = {
        "Play", "Missions", "Stage Select", "Options",
        "Unlocks", "Extras", "Quit", nullptr
    };
    plot_layout( "title menu, focus animating Play -> Missions", ctx.draw_list, "title", labels );
}

UTEST(plot, gameplay_menu_default) {
    vxui_ctx ctx = make_ctx();
    gameplay_state s;
    gameplay_frame( &ctx, &s, 0 );

    static const char* labels[] = {
        "GAMEPLAY", "Difficulty", "Speed", "Show Scoring Info",
        "Changes apply on restart", "Apply", nullptr
    };
    plot_layout( "gameplay menu, focus on Difficulty", ctx.draw_list, "gameplay", labels );
}

UTEST(plot, gameplay_menu_focus_on_apply) {
    vxui_ctx ctx = make_ctx();
    gameplay_state s;
    gameplay_frame( &ctx, &s, 0 );
    while ( ctx.menu_state[0].y != 5 )
        gameplay_frame( &ctx, &s, VXUI_INPUT_DOWN );
    for ( int i = 0; i < 120; i++ )
        gameplay_frame( &ctx, &s, 0 );

    static const char* labels[] = {
        "GAMEPLAY", "Difficulty", "Speed", "Show Scoring Info",
        "Changes apply on restart", "Apply", nullptr
    };
    plot_layout( "gameplay menu, focus settled on Apply", ctx.draw_list, "gameplay", labels );
}

UTEST_MAIN();
