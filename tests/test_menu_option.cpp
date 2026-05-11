#include "utest.h"
#include "../vxui.h"
#define VXUI_IMPL
#include "../vxui_impl.h"
#include <cstring>

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

// Does any TEXT cmd in the draw list contain `needle` as a substring?
static bool any_text_contains( const vxui_draw_list& dl, const char* needle )
{
    int needle_len = (int) strlen( needle );
    int n = vxui_draw_count( dl, VXUI_DRAW_TEXT );
    for ( int i = 0; i < n; i++ )
    {
        const vxui_draw_cmd* t = vxui_draw_nth( dl, VXUI_DRAW_TEXT, i );
        for ( int j = 0; j + needle_len <= t->text_len; j++ )
            if ( memcmp( t->text + j, needle, needle_len ) == 0 ) return true;
    }
    return false;
}

static const char* s_diff[] = { "Easy", "Normal", "Hard" };
static const int   s_diff_n = 3;

// Helper: full frame with a single option row.
static bool option_frame( vxui_ctx* ctx, int* index, uint32_t input )
{
    vxui_frame( ctx, 1.0f / 60.0f );
    ctx->input = input;
    bool changed = false;
    if ( vxui_menu( ctx, "test" ) )
    {
        changed = vxui_menu_option( ctx, "Difficulty", index, s_diff, s_diff_n );
        vxui_menu_end( ctx );
    }
    vxui_render( ctx );
    return changed;
}

/* ---- basic ----------------------------------------------------------- */

UTEST(menu_option, no_change_by_default) {
    vxui_ctx ctx = make_ctx();
    int idx = 1;
    option_frame( &ctx, &idx, 0 );          // establish

    bool changed = option_frame( &ctx, &idx, 0 );
    ASSERT_FALSE( changed );
    ASSERT_EQ( idx, 1 );
}

/* ---- left/right ------------------------------------------------------ */

UTEST(menu_option, right_increments) {
    vxui_ctx ctx = make_ctx();
    int idx = 0;
    option_frame( &ctx, &idx, 0 );          // establish

    bool changed = option_frame( &ctx, &idx, VXUI_INPUT_RIGHT );
    ASSERT_TRUE( changed );
    ASSERT_EQ( idx, 1 );
}

UTEST(menu_option, left_decrements) {
    vxui_ctx ctx = make_ctx();
    int idx = 2;
    option_frame( &ctx, &idx, 0 );          // establish

    bool changed = option_frame( &ctx, &idx, VXUI_INPUT_LEFT );
    ASSERT_TRUE( changed );
    ASSERT_EQ( idx, 1 );
}

UTEST(menu_option, right_wraps) {
    vxui_ctx ctx = make_ctx();
    int idx = 2;
    option_frame( &ctx, &idx, 0 );          // establish

    option_frame( &ctx, &idx, VXUI_INPUT_RIGHT );
    ASSERT_EQ( idx, 0 );
}

UTEST(menu_option, left_wraps) {
    vxui_ctx ctx = make_ctx();
    int idx = 0;
    option_frame( &ctx, &idx, 0 );          // establish

    option_frame( &ctx, &idx, VXUI_INPUT_LEFT );
    ASSERT_EQ( idx, s_diff_n - 1 );
}

/* ---- focus ----------------------------------------------------------- */

UTEST(menu_option, only_fires_when_focused) {
    vxui_ctx ctx = make_ctx();
    int idx = 0;

    // Frame 1: establish a two-row menu (action + option), focus on row 0.
    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "test" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_option( &ctx, "Difficulty", &idx, s_diff, s_diff_n );
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );

    // Frame 2: right input, focus is on row 0 (action), not the option.
    bool changed = false;
    vxui_frame( &ctx, 1.0f / 60.0f );
    ctx.input = VXUI_INPUT_RIGHT;
    if ( vxui_menu( &ctx, "test" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        changed = vxui_menu_option( &ctx, "Difficulty", &idx, s_diff, s_diff_n );
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );
    ASSERT_FALSE( changed );
    ASSERT_EQ( idx, 0 );
}

/* ---- value display --------------------------------------------------- */

UTEST(menu_option, value_text_emitted) {
    vxui_ctx ctx = make_ctx();
    int idx = 1;

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_option( &ctx, "Difficulty", &idx, s_diff, s_diff_n );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    // Two TEXT cmds per option row: the label and the value display.
    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), 2 );
    ASSERT_TRUE( any_text_contains( dl, "Difficulty" ) );
    ASSERT_TRUE( any_text_contains( dl, "Normal"     ) );
}

UTEST(menu_option, value_text_updates_with_index) {
    vxui_ctx ctx = make_ctx();
    int idx = 0;
    option_frame( &ctx, &idx, 0 );          // establish, idx=0 -> "Easy"

    option_frame( &ctx, &idx, VXUI_INPUT_RIGHT );
    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "test" ) )
    {
        vxui_menu_option( &ctx, "Difficulty", &idx, s_diff, s_diff_n );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( idx, 1 );
    ASSERT_TRUE ( any_text_contains( dl, "Normal" ) );
    ASSERT_FALSE( any_text_contains( dl, "Easy"   ) );
}

UTEST(menu_option, row_id_stable_across_value_change) {
    vxui_ctx ctx = make_ctx();
    int idx = 0;

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_option( &ctx, "Difficulty", &idx, s_diff, s_diff_n );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl_0 = vxui_render( &ctx );
    uint32_t id_easy = vxui_draw_nth( dl_0, VXUI_DRAW_RECT, 0 )->id;

    idx = 2;
    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_option( &ctx, "Difficulty", &idx, s_diff, s_diff_n );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl_2 = vxui_render( &ctx );
    uint32_t id_hard = vxui_draw_nth( dl_2, VXUI_DRAW_RECT, 0 )->id;

    ASSERT_EQ( id_easy, id_hard );
    ASSERT_EQ( id_easy, row_id( "m", "Difficulty" ) );
}

/* ---- fill rows ------------------------------------------------------- */

UTEST(menu_option, fill_row_grows_with_value_present) {
    vxui_ctx ctx = make_ctx();
    int idx = 0;

    // Fill menu inside a fixed-width parent: rows grow to fill it.
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_div( &ctx, "parent", { .width = { VXUI_FIXED, 480 }, .height = { VXUI_FIXED, 100 } } );
        if ( vxui_menu( &ctx, "m", true, 0, true ) )
        {
            vxui_menu_option( &ctx, "Difficulty", &idx, s_diff, s_diff_n );
            vxui_menu_end( &ctx );
        }
    vxui_div_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* r = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "m", "Difficulty" ) );
    ASSERT_TRUE( r != nullptr );
    ASSERT_NEAR( r->rect.z, 480.0f, 0.5f );
}

UTEST_MAIN();
