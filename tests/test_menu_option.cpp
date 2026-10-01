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

static bool any_text_equals( const vxui_draw_list& dl, const char* text )
{
    int len = (int) strlen( text );
    int n = vxui_draw_count( dl, VXUI_DRAW_TEXT );
    for ( int i = 0; i < n; i++ )
    {
        const vxui_draw_cmd* t = vxui_draw_nth( dl, VXUI_DRAW_TEXT, i );
        if ( t->text_len == len && memcmp( t->text, text, len ) == 0 ) return true;
    }
    return false;
}

static Clay_Dimensions measure_text( Clay_StringSlice s, Clay_TextElementConfig* cfg, void* )
{
    return { (float) s.length * 8.0f, (float) cfg->fontSize };
}

static const char* s_diff[] = { "Easy", "Normal", "Hard" };
static const int   s_diff_n = 3;

// Helper: full frame with a single option row.
static bool option_frame( vxui_ctx* ctx, int* index, uint32_t input, float dt = 1.0f / 60.0f )
{
    vxui_frame( ctx, dt );
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

static bool mixed_option_frame( vxui_ctx* ctx, int* index, uint32_t input, uint32_t click_row_id = 0 )
{
    vxui_frame( ctx, 1.0f / 60.0f );
    ctx->input              = input;
    ctx->mouse_click_row_id = click_row_id;
    bool changed = false;
    if ( vxui_menu( ctx, "test" ) )
    {
        vxui_menu_action( ctx, "Play" );
        changed = vxui_menu_option( ctx, "Difficulty", index, s_diff, s_diff_n );
        vxui_menu_action( ctx, "Quit" );
        vxui_menu_end( ctx );
    }
    vxui_render( ctx );
    return changed;
}

UTEST(menu_option, no_change_by_default) {
    vxui_ctx ctx = make_ctx();
    int idx = 1;
    option_frame( &ctx, &idx, 0 );

    bool changed = option_frame( &ctx, &idx, 0 );
    ASSERT_FALSE( changed );
    ASSERT_EQ( idx, 1 );
    ASSERT_EQ( vxui_draw_count( ctx.draw_list, VXUI_DRAW_TEXT ), 2 );
    ASSERT_TRUE( any_text_equals( ctx.draw_list, "Normal" ) );
}

UTEST(menu_option, right_increments) {
    vxui_ctx ctx = make_ctx();
    int idx = 0;
    option_frame( &ctx, &idx, 0 );

    bool changed = option_frame( &ctx, &idx, VXUI_INPUT_RIGHT );
    ASSERT_TRUE( changed );
    ASSERT_EQ( idx, 1 );
    ASSERT_TRUE ( any_text_equals( ctx.draw_list, "Normal" ) );
    ASSERT_FALSE( any_text_equals( ctx.draw_list, "Easy" ) );
}

UTEST(menu_option, left_decrements) {
    vxui_ctx ctx = make_ctx();
    int idx = 2;
    option_frame( &ctx, &idx, 0 );

    bool changed = option_frame( &ctx, &idx, VXUI_INPUT_LEFT );
    ASSERT_TRUE( changed );
    ASSERT_EQ( idx, 1 );
    ASSERT_TRUE ( any_text_equals( ctx.draw_list, "Normal" ) );
    ASSERT_FALSE( any_text_equals( ctx.draw_list, "Hard" ) );
}

UTEST(menu_option, right_wraps) {
    vxui_ctx ctx = make_ctx();
    int idx = 2;
    option_frame( &ctx, &idx, 0 );

    ASSERT_TRUE( option_frame( &ctx, &idx, VXUI_INPUT_RIGHT ) );
    ASSERT_EQ( idx, 0 );
    ASSERT_TRUE ( any_text_equals( ctx.draw_list, "Easy" ) );
    ASSERT_FALSE( any_text_equals( ctx.draw_list, "Hard" ) );
}

UTEST(menu_option, left_wraps) {
    vxui_ctx ctx = make_ctx();
    int idx = 0;
    option_frame( &ctx, &idx, 0 );

    ASSERT_TRUE( option_frame( &ctx, &idx, VXUI_INPUT_LEFT ) );
    ASSERT_EQ( idx, s_diff_n - 1 );
    ASSERT_TRUE ( any_text_equals( ctx.draw_list, "Hard" ) );
    ASSERT_FALSE( any_text_equals( ctx.draw_list, "Easy" ) );
}

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
    ASSERT_EQ( vxui_draw_count( ctx.draw_list, VXUI_DRAW_RECT ), 2 );
    ASSERT_EQ( vxui_draw_count( ctx.draw_list, VXUI_DRAW_TEXT ), 3 );
    ASSERT_TRUE( any_text_equals( ctx.draw_list, "Easy" ) );
}

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

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), 2 );
    ASSERT_TRUE( any_text_equals( dl, "Difficulty" ) );
    ASSERT_TRUE( any_text_equals( dl, "Normal"     ) );
}

UTEST(menu_option, value_text_updates_with_index_in_same_frame) {
    vxui_ctx ctx = make_ctx();
    int idx = 0;
    option_frame( &ctx, &idx, 0 );
    uint32_t id = vxui_draw_nth( ctx.draw_list, VXUI_DRAW_RECT, 0 )->id;

    ASSERT_TRUE( option_frame( &ctx, &idx, VXUI_INPUT_RIGHT ) );
    vxui_draw_list dl = ctx.draw_list;

    ASSERT_EQ( idx, 1 );
    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 1 );
    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), 2 );
    const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, id );
    ASSERT_TRUE( c != nullptr );
    ASSERT_EQ( c->row_index, 0 );
    ASSERT_EQ( c->focused_row_index, 0 );
    ASSERT_TRUE( ( c->state & VXUI_DRAW_FOCUSED ) != 0 );
    ASSERT_TRUE ( any_text_equals( dl, "Normal" ) );
    ASSERT_FALSE( any_text_equals( dl, "Easy" ) );
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

UTEST(menu_option, fill_row_grows_with_value_present) {
    vxui_ctx ctx = make_ctx();
    int idx = 0;

    vxui_frame( &ctx, 1.0f / 60.0f );
    ctx.input = VXUI_INPUT_RIGHT;
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
    ASSERT_EQ( idx, 1 );
    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), 2 );
    ASSERT_TRUE( any_text_equals( dl, "Normal" ) );
}

UTEST(menu_option, down_and_right_update_newly_focused_value_text) {
    vxui_ctx ctx = make_ctx();
    int idx = 0;
    mixed_option_frame( &ctx, &idx, 0 );

    ASSERT_TRUE( mixed_option_frame( &ctx, &idx, VXUI_INPUT_DOWN | VXUI_INPUT_RIGHT ) );
    ASSERT_EQ( idx, 1 );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 1 );
    ASSERT_EQ( vxui_draw_count( ctx.draw_list, VXUI_DRAW_RECT ), 3 );
    ASSERT_EQ( vxui_draw_count( ctx.draw_list, VXUI_DRAW_TEXT ), 4 );
    const vxui_draw_cmd* c = vxui_draw_find( ctx.draw_list, VXUI_DRAW_RECT, row_id( "test", "Difficulty" ) );
    ASSERT_TRUE( c != nullptr );
    ASSERT_EQ( c->row_index, 1 );
    ASSERT_EQ( c->focused_row_index, 1 );
    ASSERT_TRUE( ( c->state & VXUI_DRAW_FOCUSED ) != 0 );
    ASSERT_TRUE ( any_text_equals( ctx.draw_list, "Normal" ) );
    ASSERT_FALSE( any_text_equals( ctx.draw_list, "Easy" ) );
}

UTEST(menu_option, leading_section_promotes_focus_before_value_update) {
    vxui_ctx ctx = make_ctx();
    int idx = 0;
    vxui_frame( &ctx, 1.0f / 60.0f );
    ctx.input = VXUI_INPUT_RIGHT;
    bool changed = false;
    if ( vxui_menu( &ctx, "test" ) )
    {
        vxui_menu_section( &ctx, "S" );
        changed = vxui_menu_option( &ctx, "Difficulty", &idx, s_diff, s_diff_n );
        vxui_menu_action( &ctx, "Quit" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_TRUE( changed );
    ASSERT_EQ( idx, 1 );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 1 );
    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 3 );
    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), 4 );
    const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "test", "Difficulty" ) );
    ASSERT_TRUE( c != nullptr );
    ASSERT_EQ( c->row_index, 1 );
    ASSERT_EQ( c->focused_row_index, 1 );
    ASSERT_TRUE( ( c->state & VXUI_DRAW_FOCUSED ) != 0 );
    ASSERT_TRUE ( any_text_equals( dl, "Normal" ) );
    ASSERT_FALSE( any_text_equals( dl, "Easy" ) );
}

UTEST(menu_option, mouse_click_and_right_update_newly_focused_value_text) {
    vxui_ctx ctx = make_ctx();
    int idx = 0;
    mixed_option_frame( &ctx, &idx, 0 );

    ASSERT_TRUE( mixed_option_frame( &ctx, &idx, VXUI_INPUT_RIGHT, row_id( "test", "Difficulty" ) ) );
    ASSERT_EQ( idx, 1 );
    ASSERT_EQ( ctx.menu_state[0].y, (uint32_t) 1 );
    ASSERT_EQ( vxui_draw_count( ctx.draw_list, VXUI_DRAW_RECT ), 3 );
    ASSERT_EQ( vxui_draw_count( ctx.draw_list, VXUI_DRAW_TEXT ), 4 );
    ASSERT_TRUE ( any_text_equals( ctx.draw_list, "Normal" ) );
    ASSERT_FALSE( any_text_equals( ctx.draw_list, "Easy" ) );
}

UTEST(menu_option, repeat_updates_value_text_in_same_frame) {
    vxui_ctx ctx = make_ctx();
    int idx = 0;
    option_frame( &ctx, &idx, 0, 0.05f );
    ASSERT_TRUE( option_frame( &ctx, &idx, VXUI_INPUT_RIGHT, 0.05f ) );
    ASSERT_EQ( idx, 1 );

    for ( int i = 0; i < 7; i++ )
    {
        ASSERT_FALSE( option_frame( &ctx, &idx, VXUI_INPUT_RIGHT, 0.05f ) );
        ASSERT_EQ( idx, 1 );
        ASSERT_TRUE( any_text_equals( ctx.draw_list, "Normal" ) );
    }

    ASSERT_TRUE( option_frame( &ctx, &idx, VXUI_INPUT_RIGHT, 0.05f ) );
    ASSERT_EQ( idx, 2 );
    ASSERT_TRUE ( any_text_equals( ctx.draw_list, "Hard" ) );
    ASSERT_FALSE( any_text_equals( ctx.draw_list, "Normal" ) );
}

UTEST(menu_option, both_directions_leave_value_and_text_unchanged) {
    vxui_ctx ctx = make_ctx();
    int idx = 1;
    option_frame( &ctx, &idx, 0 );

    ASSERT_FALSE( option_frame( &ctx, &idx, VXUI_INPUT_RIGHT | VXUI_INPUT_LEFT ) );
    ASSERT_EQ( idx, 1 );
    ASSERT_EQ( vxui_draw_count( ctx.draw_list, VXUI_DRAW_TEXT ), 2 );
    ASSERT_TRUE( any_text_equals( ctx.draw_list, "Normal" ) );
}

UTEST(menu_option, single_option_stays_unchanged_on_input) {
    vxui_ctx ctx = make_ctx();
    int idx = 0;
    const char* options[] = { "Only" };
    vxui_frame( &ctx, 1.0f / 60.0f );
    ctx.input = VXUI_INPUT_RIGHT;
    bool changed = false;
    if ( vxui_menu( &ctx, "test" ) )
    {
        changed = vxui_menu_option( &ctx, "Mode", &idx, options, 1 );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_FALSE( changed );
    ASSERT_EQ( idx, 0 );
    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 1 );
    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), 2 );
    ASSERT_TRUE( any_text_equals( dl, "Only" ) );
}

UTEST(menu_option, longer_value_is_measured_in_same_frame) {
    vxui_ctx ctx = {};
    vxui_init( &ctx, 1280, 720, s_clay_mem, sizeof( s_clay_mem ) );
    Clay_SetMeasureTextFunction( measure_text, nullptr );
    int idx = 0;
    const char* options[] = { "Off", "Expanded" };
    vxui_frame( &ctx, 1.0f / 60.0f );
    ctx.input = VXUI_INPUT_RIGHT;
    bool changed = false;
    if ( vxui_menu( &ctx, "test" ) )
    {
        changed = vxui_menu_option( &ctx, "Mode", &idx, options, 2 );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_TRUE( changed );
    ASSERT_EQ( idx, 1 );
    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 1 );
    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), 2 );
    const vxui_draw_cmd* r = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "test", "Mode" ) );
    const vxui_draw_cmd* t = vxui_draw_nth( dl, VXUI_DRAW_TEXT, 1 );
    ASSERT_TRUE( r && t );
    ASSERT_EQ( t->text_len, 8 );
    ASSERT_EQ( memcmp( t->text, "Expanded", 8 ), 0 );
    ASSERT_NEAR( t->rect.z, 64.0f, 0.01f );
    ASSERT_NEAR( r->rect.z, 104.0f, 0.01f );
}

UTEST(menu_option, shorter_value_is_measured_in_same_frame) {
    vxui_ctx ctx = {};
    vxui_init( &ctx, 1280, 720, s_clay_mem, sizeof( s_clay_mem ) );
    Clay_SetMeasureTextFunction( measure_text, nullptr );
    int idx = 1;
    const char* options[] = { "Off", "Expanded" };
    vxui_frame( &ctx, 1.0f / 60.0f );
    ctx.input = VXUI_INPUT_LEFT;
    bool changed = false;
    if ( vxui_menu( &ctx, "test" ) )
    {
        changed = vxui_menu_option( &ctx, "Mode", &idx, options, 2 );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_TRUE( changed );
    ASSERT_EQ( idx, 0 );
    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 1 );
    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), 2 );
    const vxui_draw_cmd* r = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "test", "Mode" ) );
    const vxui_draw_cmd* t = vxui_draw_nth( dl, VXUI_DRAW_TEXT, 1 );
    ASSERT_TRUE( r && t );
    ASSERT_EQ( t->text_len, 3 );
    ASSERT_EQ( memcmp( t->text, "Off", 3 ), 0 );
    ASSERT_NEAR( t->rect.z, 24.0f, 0.01f );
    ASSERT_NEAR( r->rect.z, 64.0f, 0.01f );
}

UTEST(menu_option, two_rows_keep_separate_stable_value_text) {
    vxui_ctx ctx = make_ctx();
    int first = 0, second = 2;
    vxui_frame( &ctx, 1.0f / 60.0f );
    ctx.input = VXUI_INPUT_RIGHT;
    bool first_changed = false, second_changed = false;
    if ( vxui_menu( &ctx, "test" ) )
    {
        first_changed  = vxui_menu_option( &ctx, "Difficulty", &first, s_diff, s_diff_n );
        second_changed = vxui_menu_option( &ctx, "Other", &second, s_diff, s_diff_n );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_TRUE ( first_changed );
    ASSERT_FALSE( second_changed );
    ASSERT_EQ( first, 1 );
    ASSERT_EQ( second, 2 );
    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 2 );
    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), 4 );
    const vxui_draw_cmd* a = vxui_draw_nth( dl, VXUI_DRAW_TEXT, 1 );
    const vxui_draw_cmd* b = vxui_draw_nth( dl, VXUI_DRAW_TEXT, 3 );
    ASSERT_TRUE( a && b );
    ASSERT_EQ( a->text_len, 6 );
    ASSERT_EQ( b->text_len, 4 );
    ASSERT_EQ( memcmp( a->text, "Normal", 6 ), 0 );
    ASSERT_EQ( memcmp( b->text, "Hard", 4 ), 0 );
}

UTEST(menu_option, long_value_is_not_truncated) {
    vxui_ctx ctx = {};
    vxui_init( &ctx, 1280, 720, s_clay_mem, sizeof( s_clay_mem ) );
    Clay_SetMeasureTextFunction( measure_text, nullptr );
    char value[97] = {};
    for ( int i = 0; i < 96; i++ ) value[i] = (char) ( 'a' + i % 26 );
    const char* options[] = { "Short", value };
    int idx = 0;

    vxui_frame( &ctx, 1.0f / 60.0f );
    ctx.input = VXUI_INPUT_RIGHT;
    bool changed = false;
    if ( vxui_menu( &ctx, "test" ) )
    {
        changed = vxui_menu_option( &ctx, "Mode", &idx, options, 2 );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_TRUE( changed );
    ASSERT_EQ( idx, 1 );
    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), 2 );
    const vxui_draw_cmd* r = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "test", "Mode" ) );
    const vxui_draw_cmd* t = vxui_draw_nth( dl, VXUI_DRAW_TEXT, 1 );
    ASSERT_TRUE( r && t );
    ASSERT_EQ( t->text_len, 96 );
    ASSERT_EQ( memcmp( t->text, value, 96 ), 0 );
    ASSERT_NEAR( t->rect.z, 768.0f, 0.01f );
    ASSERT_NEAR( r->rect.z, 808.0f, 0.01f );
}

UTEST(menu_option, selected_value_is_copied_before_render) {
    vxui_ctx ctx = make_ctx();
    char value[] = "Normal";
    const char* options[] = { "Easy", value };
    int idx = 0;

    vxui_frame( &ctx, 1.0f / 60.0f );
    ctx.input = VXUI_INPUT_RIGHT;
    bool changed = false;
    if ( vxui_menu( &ctx, "test" ) )
    {
        changed = vxui_menu_option( &ctx, "Mode", &idx, options, 2 );
        memset( value, '?', sizeof( value ) - 1 );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_TRUE( changed );
    ASSERT_EQ( idx, 1 );
    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), 2 );
    const vxui_draw_cmd* t = vxui_draw_nth( dl, VXUI_DRAW_TEXT, 1 );
    ASSERT_TRUE( t != nullptr );
    ASSERT_TRUE( t->text != value );
    ASSERT_EQ( t->text_len, 6 );
    ASSERT_EQ( memcmp( t->text, "Normal", 6 ), 0 );
}

UTEST(menu_option, caller_punctuation_is_preserved) {
    vxui_ctx ctx = make_ctx();
    const char* options[] = { "< 50% >" };
    int idx = 0;

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "test" ) )
    {
        vxui_menu_option( &ctx, "Mode", &idx, options, 1 );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), 2 );
    ASSERT_TRUE( any_text_equals( dl, options[0] ) );
}

UTEST(menu_option, utf8_value_is_preserved) {
    vxui_ctx ctx = make_ctx();
    const char* options[] = { "日本語" };
    int idx = 0;

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "test" ) )
    {
        vxui_menu_option( &ctx, "Language", &idx, options, 1 );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), 2 );
    ASSERT_TRUE( any_text_equals( dl, options[0] ) );
}

UTEST(menu_option, empty_value_emits_only_label) {
    vxui_ctx ctx = make_ctx();
    const char* options[] = { "" };
    int idx = 0;

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "test" ) )
    {
        vxui_menu_option( &ctx, "Mode", &idx, options, 1 );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 1 );
    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), 1 );
    ASSERT_TRUE( any_text_equals( dl, "Mode" ) );
}

UTEST_MAIN();
