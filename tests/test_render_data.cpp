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

UTEST(render_data, rect_default_state_zero) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_label( &ctx, "Hello" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 1 );
    ASSERT_EQ( vxui_draw_nth( dl, VXUI_DRAW_RECT, 0 )->state, (uint8_t) 0 );
}

UTEST(render_data, focused_row_has_focused_bit) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "m", "Play" ) );
    ASSERT_TRUE( c != nullptr );
    ASSERT_TRUE( ( c->state & VXUI_DRAW_FOCUSED ) != 0 );
}

UTEST(render_data, unfocused_rows_no_focused_bit) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_action( &ctx, "Quit" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "m", "Quit" ) );
    ASSERT_TRUE( c != nullptr );
    ASSERT_EQ( c->state & VXUI_DRAW_FOCUSED, (uint8_t) 0 );
}

UTEST(render_data, section_row_no_focused_bit) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_section( &ctx, "Header" );
        vxui_menu_action ( &ctx, "Play" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "m", "Header" ) );
    ASSERT_TRUE( c != nullptr );
    ASSERT_EQ( c->state & VXUI_DRAW_FOCUSED, (uint8_t) 0 );
}

UTEST(render_data, label_row_no_focused_bit) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_label ( &ctx, "Note" );
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "m", "Note" ) );
    ASSERT_TRUE( c != nullptr );
    ASSERT_EQ( c->state & VXUI_DRAW_FOCUSED, (uint8_t) 0 );
}

UTEST(render_data, text_cmd_state_zero) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    int n = vxui_draw_count( dl, VXUI_DRAW_TEXT );
    for ( int i = 0; i < n; i++ )
        ASSERT_EQ( vxui_draw_nth( dl, VXUI_DRAW_TEXT, i )->state, (uint8_t) 0 );
}

UTEST(render_data, confirm_held_focused_action_has_pressed) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    ctx.input = VXUI_INPUT_CONFIRM;
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "m", "Play" ) );
    ASSERT_TRUE( c != nullptr );
    ASSERT_TRUE( ( c->state & VXUI_DRAW_PRESSED ) != 0 );
    ASSERT_TRUE( ( c->state & VXUI_DRAW_FOCUSED ) != 0 );
}

UTEST(render_data, confirm_released_no_pressed) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "m", "Play" ) );
    ASSERT_TRUE( c != nullptr );
    ASSERT_EQ( c->state & VXUI_DRAW_PRESSED, (uint8_t) 0 );
}

UTEST(render_data, confirm_held_unfocused_actions_not_pressed) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    ctx.input = VXUI_INPUT_CONFIRM;
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_action( &ctx, "Quit" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "m", "Quit" ) );
    ASSERT_TRUE( c != nullptr );
    ASSERT_EQ( c->state & VXUI_DRAW_PRESSED, (uint8_t) 0 );
}

UTEST(render_data, two_menus_each_focused_action_has_pressed) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    ctx.input = VXUI_INPUT_CONFIRM;
    if ( vxui_menu( &ctx, "left" ) )
    {
        vxui_menu_action( &ctx, "Back" );
        vxui_menu_end( &ctx );
    }
    if ( vxui_menu( &ctx, "right" ) )
    {
        vxui_menu_action( &ctx, "Next" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* l = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "left",  "Back" ) );
    const vxui_draw_cmd* r = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "right", "Next" ) );
    ASSERT_TRUE( l != nullptr );
    ASSERT_TRUE( r != nullptr );
    ASSERT_TRUE( ( l->state & VXUI_DRAW_PRESSED ) != 0 );
    ASSERT_TRUE( ( r->state & VXUI_DRAW_PRESSED ) != 0 );
}

UTEST(render_data, two_menus_each_focused_row_has_focused_bit) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "left" ) )
    {
        vxui_menu_action( &ctx, "Back" );
        vxui_menu_end( &ctx );
    }
    if ( vxui_menu( &ctx, "right" ) )
    {
        vxui_menu_action( &ctx, "Next" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* l = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "left",  "Back" ) );
    const vxui_draw_cmd* r = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "right", "Next" ) );
    ASSERT_TRUE( l != nullptr );
    ASSERT_TRUE( r != nullptr );
    ASSERT_TRUE( ( l->state & VXUI_DRAW_FOCUSED ) != 0 );
    ASSERT_TRUE( ( r->state & VXUI_DRAW_FOCUSED ) != 0 );
}

UTEST(render_data, no_resolver_render_block_zero) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    for ( int i = 0; i < dl.count; i++ )
    {
        const vxui_render_data& r = dl.cmds[i].render;
        ASSERT_EQ( r.material_id,      (uint32_t) 0 );
        ASSERT_EQ( r.texture_id,       (uint32_t) 0 );
        ASSERT_EQ( r.flags,            (uint32_t) 0 );
        ASSERT_EQ( r.outline_thickness, 0.0f );
        ASSERT_EQ( r.colour.x,   0.0f );
        ASSERT_EQ( r.params[7], 0.0f );
    }
}

UTEST(render_data, resolver_called_once_per_cmd) {
    vxui_ctx ctx = make_ctx();

    struct ud_t { int count = 0; };
    ud_t ud;
    vxui_set_render_data_fn( &ctx,
        []( const vxui_draw_cmd*, vxui_render_data*, void* u ) { ((ud_t*)u)->count++; },
        &ud );

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_action( &ctx, "Quit" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( ud.count, dl.count );
}

UTEST(render_data, resolver_sees_correct_state_on_focused_row) {
    vxui_ctx ctx = make_ctx();

    struct ud_t { uint8_t seen_state = 0; uint32_t target_id = 0; };
    ud_t ud;
    ud.target_id = row_id( "m", "Play" );

    vxui_set_render_data_fn( &ctx,
        []( const vxui_draw_cmd* c, vxui_render_data*, void* u )
        {
            ud_t* d = (ud_t*) u;
            if ( c->id == d->target_id ) d->seen_state = c->state;
        },
        &ud );

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );

    ASSERT_TRUE( ( ud.seen_state & VXUI_DRAW_FOCUSED ) != 0 );
}

UTEST(render_data, resolver_sees_correct_id_on_each_cmd) {
    vxui_ctx ctx = make_ctx();

    struct ud_t { uint32_t ids[8] = {}; int n = 0; };
    ud_t ud;

    vxui_set_render_data_fn( &ctx,
        []( const vxui_draw_cmd* c, vxui_render_data*, void* u )
        {
            ud_t* d = (ud_t*) u;
            if ( d->n < 8 ) d->ids[d->n++] = c->id;
        },
        &ud );

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_action( &ctx, "Quit" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( ud.n, dl.count );
    bool found_play = false, found_quit = false;
    for ( int i = 0; i < ud.n; i++ )
    {
        if ( ud.ids[i] == row_id( "m", "Play" ) ) found_play = true;
        if ( ud.ids[i] == row_id( "m", "Quit" ) ) found_quit = true;
    }
    ASSERT_TRUE( found_play );
    ASSERT_TRUE( found_quit );
}

UTEST(render_data, resolver_output_appears_in_packet_unchanged) {
    vxui_ctx ctx = make_ctx();

    vxui_set_render_data_fn( &ctx,
        []( const vxui_draw_cmd*, vxui_render_data* out, void* )
        {
            out->material_id      = 0xC0FFEE;
            out->texture_id       = 42;
            out->flags            = 7;
            out->outline_thickness = 3.14f;
            out->colour.x         = 0.11f;
            out->outline_colour.w = 0.99f;
            out->uv.z             = 0.75f;
            out->params[7]        = 0.5f;
        },
        nullptr );

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "m", "Play" ) );
    ASSERT_TRUE( c != nullptr );
    ASSERT_EQ  ( c->render.material_id,      (uint32_t) 0xC0FFEE );
    ASSERT_EQ  ( c->render.texture_id,       (uint32_t) 42 );
    ASSERT_EQ  ( c->render.flags,            (uint32_t) 7 );
    ASSERT_NEAR( c->render.outline_thickness, 3.14f,  1e-5f );
    ASSERT_NEAR( c->render.colour.x,         0.11f, 1e-5f );
    ASSERT_NEAR( c->render.outline_colour.w, 0.99f, 1e-5f );
    ASSERT_NEAR( c->render.uv.z,             0.75f, 1e-5f );
    ASSERT_NEAR( c->render.params[7],         0.5f,   1e-5f );
}

UTEST(render_data, resolver_can_distinguish_rect_vs_text_by_type) {
    vxui_ctx ctx = make_ctx();

    struct ud_t { int rect_n = 0; int text_n = 0; };
    ud_t ud;

    vxui_set_render_data_fn( &ctx,
        []( const vxui_draw_cmd* c, vxui_render_data*, void* u )
        {
            ud_t* d = (ud_t*) u;
            if ( c->type == VXUI_DRAW_RECT ) d->rect_n++;
            if ( c->type == VXUI_DRAW_TEXT ) d->text_n++;
        },
        &ud );

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( ud.rect_n, vxui_draw_count( dl, VXUI_DRAW_RECT ) );
    ASSERT_EQ( ud.text_n, vxui_draw_count( dl, VXUI_DRAW_TEXT ) );
    ASSERT_EQ( ud.rect_n + ud.text_n, dl.count );
}

UTEST(render_data, resolver_userdata_passes_through) {
    vxui_ctx ctx = make_ctx();

    struct ud_t { void* seen = nullptr; };
    ud_t ud;

    vxui_set_render_data_fn( &ctx,
        []( const vxui_draw_cmd*, vxui_render_data*, void* u )
        {
            ud_t* d = (ud_t*) u;
            d->seen = u;
        },
        &ud );

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );

    ASSERT_TRUE( ud.seen == &ud );
}

UTEST(render_data, text_cmd_render_zero_when_resolver_skips_text) {
    vxui_ctx ctx = make_ctx();

    vxui_set_render_data_fn( &ctx,
        []( const vxui_draw_cmd* c, vxui_render_data* out, void* )
        {
            if ( c->type != VXUI_DRAW_RECT ) return;
            out->colour.x = 1.0f;
        },
        nullptr );

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    int n = vxui_draw_count( dl, VXUI_DRAW_TEXT );
    for ( int i = 0; i < n; i++ )
    {
        const vxui_render_data& r = vxui_draw_nth( dl, VXUI_DRAW_TEXT, i )->render;
        ASSERT_EQ( r.material_id, (uint32_t) 0 );
        ASSERT_EQ( r.colour.x,    0.0f );
        ASSERT_EQ( r.params[7],   0.0f );
    }
}

UTEST(render_data, focused_row_carries_focus_offset_zero_on_snap) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "m", "Play" ) );
    ASSERT_TRUE( c != nullptr );
    ASSERT_TRUE( ( c->state & VXUI_DRAW_FOCUSED ) != 0 );
    ASSERT_NEAR( c->focus_offset_y, 0.0f, 1e-3f );
}

UTEST(render_data, unfocused_rows_focus_offset_zero) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_action( &ctx, "Quit" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "m", "Quit" ) );
    ASSERT_TRUE( c != nullptr );
    ASSERT_EQ  ( c->state & VXUI_DRAW_FOCUSED, (uint8_t) 0 );
    ASSERT_NEAR( c->focus_offset_y, 0.0f, 1e-3f );
}

UTEST(render_data, text_cmd_focus_offset_zero) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    int n = vxui_draw_count( dl, VXUI_DRAW_TEXT );
    for ( int i = 0; i < n; i++ )
        ASSERT_NEAR( vxui_draw_nth( dl, VXUI_DRAW_TEXT, i )->focus_offset_y, 0.0f, 1e-3f );
}

UTEST(render_data, focus_offset_nonzero_after_focus_move) {
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

    const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "m", "Quit" ) );
    ASSERT_TRUE( c != nullptr );
    ASSERT_TRUE( ( c->state & VXUI_DRAW_FOCUSED ) != 0 );
    ASSERT_LT( c->focus_offset_y, -(float) VXUI_ROW_HEIGHT * 0.1f );
}

UTEST(render_data, focus_offset_settles_toward_zero) {
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

    float prev_abs = (float) VXUI_ROW_HEIGHT;
    for ( int i = 0; i < 10; i++ )
    {
        vxui_frame( &ctx, 1.0f / 60.0f );
        if ( vxui_menu( &ctx, "m" ) )
        {
            vxui_menu_action( &ctx, "Play" );
            vxui_menu_action( &ctx, "Quit" );
            vxui_menu_end( &ctx );
        }
        vxui_draw_list dl = vxui_render( &ctx );

        const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "m", "Quit" ) );
        ASSERT_TRUE( c != nullptr );
        float abs_offset = c->focus_offset_y < 0.0f ? -c->focus_offset_y : c->focus_offset_y;
        ASSERT_LT( abs_offset, prev_abs + 1e-3f );
        prev_abs = abs_offset;
    }
}

UTEST(render_data, same_label_two_menus_only_focused_one_has_bit) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "left" ) )
    {
        vxui_menu_action( &ctx, "Same" );
        vxui_menu_action( &ctx, "Tail" );
        vxui_menu_end( &ctx );
    }
    if ( vxui_menu( &ctx, "right" ) )
    {
        vxui_menu_action( &ctx, "Head" );
        vxui_menu_action( &ctx, "Same" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* l_same = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "left",  "Same" ) );
    const vxui_draw_cmd* r_same = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "right", "Same" ) );
    ASSERT_TRUE( l_same != nullptr );
    ASSERT_TRUE( r_same != nullptr );
    ASSERT_TRUE( ( l_same->state & VXUI_DRAW_FOCUSED ) != 0 );
    ASSERT_EQ  ( r_same->state & VXUI_DRAW_FOCUSED, (uint8_t) 0 );
}

static uint32_t elem_id( const char* name )
{
    Clay_String cs = { false, (int32_t) strlen( name ), name };
    return Clay__HashString( cs, 0 ).id;
}

UTEST(render_data, rect_primitive_emits_rect_cmd) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_rect( &ctx, "panel" );
    vxui_div_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, elem_id( "panel" ) );
    ASSERT_TRUE( c != nullptr );
}

UTEST(render_data, rect_primitive_state_zero) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_rect( &ctx, "panel" );
    vxui_div_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, elem_id( "panel" ) );
    ASSERT_TRUE( c != nullptr );
    ASSERT_EQ( c->state, (uint8_t) 0 );
}

UTEST(render_data, rect_wrapping_menu_emits_both) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_rect( &ctx, "panel" );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_end( &ctx );
    }
    vxui_div_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_TRUE( vxui_draw_find( dl, VXUI_DRAW_RECT, elem_id( "panel" ) ) != nullptr );
    ASSERT_TRUE( vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "m", "Play" ) ) != nullptr );
}

UTEST(render_data, resolver_returns_outline_colour_unchanged) {
    vxui_ctx ctx = make_ctx();

    vxui_set_render_data_fn( &ctx,
        []( const vxui_draw_cmd*, vxui_render_data* out, void* )
        {
            out->outline_colour = { 0.3f, 0.7f, 0.95f, 1.0f };
        },
        nullptr );

    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_rect( &ctx, "panel" );
    vxui_div_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, elem_id( "panel" ) );
    ASSERT_TRUE( c != nullptr );
    ASSERT_NEAR( c->render.outline_colour.x, 0.3f,  1e-5f );
    ASSERT_NEAR( c->render.outline_colour.y, 0.7f,  1e-5f );
    ASSERT_NEAR( c->render.outline_colour.z, 0.95f, 1e-5f );
    ASSERT_NEAR( c->render.outline_colour.w, 1.0f,  1e-5f );
}

UTEST(render_data, resolver_returns_outline_thickness_unchanged) {
    vxui_ctx ctx = make_ctx();

    vxui_set_render_data_fn( &ctx,
        []( const vxui_draw_cmd*, vxui_render_data* out, void* )
        {
            out->outline_thickness = 2.5f;
        },
        nullptr );

    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_rect( &ctx, "panel" );
    vxui_div_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, elem_id( "panel" ) );
    ASSERT_TRUE( c != nullptr );
    ASSERT_NEAR( c->render.outline_thickness, 2.5f, 1e-5f );
}

UTEST(render_data, resolver_returns_texture_id_unchanged) {
    vxui_ctx ctx = make_ctx();

    vxui_set_render_data_fn( &ctx,
        []( const vxui_draw_cmd*, vxui_render_data* out, void* )
        {
            out->texture_id = 77;
        },
        nullptr );

    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_rect( &ctx, "panel" );
    vxui_div_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, elem_id( "panel" ) );
    ASSERT_TRUE( c != nullptr );
    ASSERT_EQ( c->render.texture_id, (uint32_t) 77 );
}

UTEST(render_data, resolver_returns_uv_unchanged) {
    vxui_ctx ctx = make_ctx();

    vxui_set_render_data_fn( &ctx,
        []( const vxui_draw_cmd*, vxui_render_data* out, void* )
        {
            out->uv = { 0.1f, 0.2f, 0.9f, 0.8f };
        },
        nullptr );

    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_rect( &ctx, "panel" );
    vxui_div_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, elem_id( "panel" ) );
    ASSERT_TRUE( c != nullptr );
    ASSERT_NEAR( c->render.uv.x, 0.1f, 1e-5f );
    ASSERT_NEAR( c->render.uv.y, 0.2f, 1e-5f );
    ASSERT_NEAR( c->render.uv.z, 0.9f, 1e-5f );
    ASSERT_NEAR( c->render.uv.w, 0.8f, 1e-5f );
}

UTEST(render_data, text_cmd_does_not_get_panel_outline) {
    vxui_ctx ctx = make_ctx();

    vxui_set_render_data_fn( &ctx,
        []( const vxui_draw_cmd* c, vxui_render_data* out, void* )
        {
            if ( c->type != VXUI_DRAW_RECT ) return;
            out->outline_thickness = 2.0f;
            out->outline_colour = { 1.0f, 1.0f, 1.0f, 1.0f };
        },
        nullptr );

    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_rect( &ctx, "panel" );
    if ( vxui_menu( &ctx, "m" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_end( &ctx );
    }
    vxui_div_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    int n = vxui_draw_count( dl, VXUI_DRAW_TEXT );
    for ( int i = 0; i < n; i++ )
    {
        const vxui_render_data& r = vxui_draw_nth( dl, VXUI_DRAW_TEXT, i )->render;
        ASSERT_EQ( r.outline_thickness, 0.0f );
        ASSERT_EQ( r.outline_colour.x,  0.0f );
    }
}

UTEST(render_data, resolver_returns_material_id_unchanged) {
    vxui_ctx ctx = make_ctx();

    vxui_set_render_data_fn( &ctx,
        []( const vxui_draw_cmd*, vxui_render_data* out, void* )
        {
            out->material_id = 0xDEAD;
        },
        nullptr );

    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_rect( &ctx, "panel" );
    vxui_div_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, elem_id( "panel" ) );
    ASSERT_TRUE( c != nullptr );
    ASSERT_EQ( c->render.material_id, (uint32_t) 0xDEAD );
}

UTEST(render_data, resolver_returns_params_unchanged) {
    vxui_ctx ctx = make_ctx();

    vxui_set_render_data_fn( &ctx,
        []( const vxui_draw_cmd*, vxui_render_data* out, void* )
        {
            out->params[0] = 3.14159f;
            out->params[1] = 2.71828f;
            out->params[2] = 1.41421f;
            out->params[3] = 1.61803f;
            out->params[4] = 0.57721f;
            out->params[5] = 2.30258f;
            out->params[6] = 0.69314f;
            out->params[7] = 1.20205f;
        },
        nullptr );

    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_rect( &ctx, "panel" );
    vxui_div_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, elem_id( "panel" ) );
    ASSERT_TRUE( c != nullptr );
    ASSERT_NEAR( c->render.params[0], 3.14159f, 1e-5f );
    ASSERT_NEAR( c->render.params[1], 2.71828f, 1e-5f );
    ASSERT_NEAR( c->render.params[2], 1.41421f, 1e-5f );
    ASSERT_NEAR( c->render.params[3], 1.61803f, 1e-5f );
    ASSERT_NEAR( c->render.params[4], 0.57721f, 1e-5f );
    ASSERT_NEAR( c->render.params[5], 2.30258f, 1e-5f );
    ASSERT_NEAR( c->render.params[6], 0.69314f, 1e-5f );
    ASSERT_NEAR( c->render.params[7], 1.20205f, 1e-5f );
}

UTEST(render_data, resolver_returns_flags_unchanged) {
    vxui_ctx ctx = make_ctx();

    vxui_set_render_data_fn( &ctx,
        []( const vxui_draw_cmd*, vxui_render_data* out, void* )
        {
            out->flags = 0xABCD1234;
        },
        nullptr );

    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_rect( &ctx, "panel" );
    vxui_div_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, elem_id( "panel" ) );
    ASSERT_TRUE( c != nullptr );
    ASSERT_EQ( c->render.flags, (uint32_t) 0xABCD1234 );
}

UTEST(render_data, material_id_zero_default_no_resolver) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_rect( &ctx, "panel" );
    vxui_div_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, elem_id( "panel" ) );
    ASSERT_TRUE( c != nullptr );
    ASSERT_EQ( c->render.material_id, (uint32_t) 0 );
}

UTEST(render_data, flags_zero_default_no_resolver) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_rect( &ctx, "panel" );
    vxui_div_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, elem_id( "panel" ) );
    ASSERT_TRUE( c != nullptr );
    ASSERT_EQ( c->render.flags, (uint32_t) 0 );
}

UTEST(render_data, params_zero_default_no_resolver) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_rect( &ctx, "panel" );
    vxui_div_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, elem_id( "panel" ) );
    ASSERT_TRUE( c != nullptr );
    for ( int i = 0; i < 8; i++ )
        ASSERT_EQ( c->render.params[i], 0.0f );
}

UTEST(render_data, outline_thickness_zero_default_no_resolver) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_rect( &ctx, "panel" );
    vxui_div_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* c = vxui_draw_find( dl, VXUI_DRAW_RECT, elem_id( "panel" ) );
    ASSERT_TRUE( c != nullptr );
    ASSERT_EQ( c->render.outline_thickness, 0.0f );
    ASSERT_EQ( c->render.outline_colour.x,  0.0f );
    ASSERT_EQ( c->render.outline_colour.y,  0.0f );
    ASSERT_EQ( c->render.outline_colour.z,  0.0f );
    ASSERT_EQ( c->render.outline_colour.w,  0.0f );
}

UTEST_MAIN();
