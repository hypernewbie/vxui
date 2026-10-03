#include "utest.h"
#include "../vxui.h"
#define VXUI_IMPL
#include "../vxui_impl.h"
#define VXUI_HUD_IMPL
#include "../vxui_hud.h"

static uint8_t s_clay_mem[16 * 1024 * 1024];

static Clay_Dimensions measure_text( Clay_StringSlice s, Clay_TextElementConfig* cfg, void* )
{
    return { (float) s.length * 8.0f, (float) cfg->fontSize };
}

static vxui_ctx make_ctx()
{
    vxui_ctx ctx = {};
    vxui_init( &ctx, 960, 540, s_clay_mem, sizeof( s_clay_mem ) );
    Clay_SetMeasureTextFunction( measure_text, nullptr );
    return ctx;
}

static void resolve_hud( const vxui_draw_cmd* cmd, vxui_render_data* out, void* userdata )
{
    vxui_hud* hud = (vxui_hud*) userdata;
    if ( !vxui_hud_resolve( hud, cmd, out ) ) out->colour = { -1, -1, -1, -1 };
}

UTEST(hud_metadata, text_lines_carry_the_declaration_record) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_text( &hud, "score", "alpha\nbeta\ngamma" );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( hud.item_count, 1 );
    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), 3 );
    for ( int i = 0; i < dl.count; i++ )
        ASSERT_EQ( dl.cmds[i].userdata, (const void*) &hud.items[0] );
}

UTEST(hud_metadata, boxed_lines_carry_the_declaration_record) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_text_box( &hud, "score", "alpha beta gamma", 48, 96, 0, 0 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( hud.item_count, 1 );
    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), 3 );
    for ( int i = 0; i < dl.count; i++ )
        ASSERT_EQ( dl.cmds[i].userdata, (const void*) &hud.items[0] );
}

UTEST(hud_metadata, rectangles_carry_the_declaration_record) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_meter( &hud, "health", 42, 0.5f, 200, 40 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( hud.item_count, 2 );
    ASSERT_EQ( dl.count, 2 );
    ASSERT_EQ( dl.cmds[0].userdata, (const void*) &hud.items[0] );
    ASSERT_EQ( dl.cmds[1].userdata, (const void*) &hud.items[1] );
}

UTEST(hud_metadata, changing_text_length_keeps_current_metadata_and_ids) {
    vxui_ctx ctx = make_ctx();
    vxui_hud hud = {};
    vxui_set_render_data_fn( &ctx, resolve_hud, &hud );
    const char* values[] = { "100", "123456789", "7", "", "alpha\nbeta" };
    uint32_t score_id = 0, tail_id = 0;
    for ( int frame = 0; frame < 5; frame++ )
    {
        vxui_frame( &ctx, 1.0f / 60.0f );
        vxui_hud_begin( &hud, &ctx, 960, 540 );
        vxui_hud_set_colour( &hud, { (float) frame, 0, 0, 1 } );
        vxui_hud_text( &hud, "score", values[frame] );
        vxui_hud_set_colour( &hud, { 0, (float) frame + 1.0f, 0, 1 } );
        vxui_hud_text( &hud, "tail", "tail" );
        vxui_draw_list dl = vxui_render( &ctx );

        ASSERT_EQ( hud.item_count, 2 );
        int score_lines = frame == 3 ? 0 : frame == 4 ? 2 : 1;
        ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), score_lines + 1 );
        for ( int i = 0; i < score_lines; i++ )
        {
            const vxui_draw_cmd* c = vxui_draw_nth( dl, VXUI_DRAW_TEXT, i );
            ASSERT_EQ( c->userdata, (const void*) &hud.items[0] );
            ASSERT_EQ( c->render.colour.r, (float) frame );
            ASSERT_EQ( c->render.colour.g, 0.0f );
            if ( i == 0 )
            {
                if ( frame == 0 ) score_id = c->id;
                ASSERT_EQ( c->id, score_id );
            }
        }
        const vxui_draw_cmd* tail = vxui_draw_nth( dl, VXUI_DRAW_TEXT, score_lines );
        ASSERT_EQ( tail->userdata, (const void*) &hud.items[1] );
        ASSERT_EQ( tail->render.colour.r, 0.0f );
        ASSERT_EQ( tail->render.colour.g, (float) frame + 1.0f );
        if ( frame == 0 ) tail_id = tail->id;
        ASSERT_EQ( tail->id, tail_id );
        ASSERT_EQ( tail->text, ctx.text_buf + strlen( values[frame] ) );
    }
}

UTEST(hud_metadata, changing_wrap_count_keeps_current_metadata_and_ids) {
    vxui_ctx ctx = make_ctx();
    vxui_hud hud = {};
    vxui_set_render_data_fn( &ctx, resolve_hud, &hud );
    const char* values[] = { "alpha", "alpha beta gamma", "", "beta gamma" };
    int line_counts[] = { 1, 3, 0, 2 };
    uint32_t first_id = 0;
    for ( int frame = 0; frame < 4; frame++ )
    {
        vxui_frame( &ctx, 1.0f / 60.0f );
        vxui_hud_begin( &hud, &ctx, 960, 540 );
        vxui_hud_set_colour( &hud, { 0, 0, (float) frame + 1.0f, 1 } );
        vxui_hud_text_box( &hud, "score", values[frame], 48, 96, 0, 0 );
        vxui_draw_list dl = vxui_render( &ctx );

        ASSERT_EQ( hud.item_count, 1 );
        ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), line_counts[frame] );
        for ( int i = 0; i < dl.count; i++ )
        {
            ASSERT_EQ( dl.cmds[i].userdata, (const void*) &hud.items[0] );
            ASSERT_EQ( dl.cmds[i].render.colour.b, (float) frame + 1.0f );
            if ( i == 0 )
            {
                if ( frame == 0 ) first_id = dl.cmds[i].id;
                ASSERT_EQ( dl.cmds[i].id, first_id );
            }
        }
    }
}

UTEST(hud_metadata, changed_declaration_order_keeps_metadata_and_ids) {
    vxui_ctx ctx = make_ctx();
    vxui_hud hud = {};
    vxui_set_render_data_fn( &ctx, resolve_hud, &hud );
    uint32_t ids[2] = {};
    for ( int frame = 0; frame < 2; frame++ )
    {
        vxui_frame( &ctx, 1.0f / 60.0f );
        vxui_hud_begin( &hud, &ctx, 960, 540 );
        for ( int slot = 0; slot < 2; slot++ )
        {
            int logical = frame == 0 ? slot : 1 - slot;
            vxui_hud_set_colour( &hud, logical == 0 ? glm::vec4{ 1, 0, 0, 1 } : glm::vec4{ 0, 1, 0, 1 } );
            vxui_hud_text( &hud, logical == 0 ? "first" : "second", frame == 0 ? "alpha" : "alpha\nbeta" );
        }
        vxui_draw_list dl = vxui_render( &ctx );

        int lines = frame + 1;
        ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), lines * 2 );
        for ( int slot = 0; slot < 2; slot++ )
        {
            int logical = frame == 0 ? slot : 1 - slot;
            for ( int line = 0; line < lines; line++ )
            {
                const vxui_draw_cmd* c = vxui_draw_nth( dl, VXUI_DRAW_TEXT, slot * lines + line );
                ASSERT_EQ( c->userdata, (const void*) &hud.items[slot] );
                ASSERT_EQ( c->render.colour.r, logical == 0 ? 1.0f : 0.0f );
                ASSERT_EQ( c->render.colour.g, logical == 0 ? 0.0f : 1.0f );
                if ( line == 0 )
                {
                    if ( frame == 0 ) ids[logical] = c->id;
                    ASSERT_EQ( c->id, ids[logical] );
                }
            }
        }
    }
}

UTEST(hud_metadata, raw_clay_text_passes_frame_attachment_to_every_line) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    int attachment = 42;
    CLAY_TEXT( CLAY_STRING( "alpha\nbeta" ), CLAY_TEXT_CONFIG( { .userData = &attachment, .fontSize = 24 } ) );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), 2 );
    for ( int i = 0; i < dl.count; i++ )
        ASSERT_EQ( dl.cmds[i].userdata, (const void*) &attachment );
}

UTEST(hud_metadata, unattached_same_id_command_cannot_claim_metadata) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_text( &hud, "score", "alpha\nbeta" );
    vxui_draw_list dl = vxui_render( &ctx );
    vxui_draw_cmd c = *vxui_draw_nth( dl, VXUI_DRAW_TEXT, 0 );
    c.userdata = nullptr;
    vxui_render_data out = {};
    out.texture_id = 99;

    ASSERT_FALSE( vxui_hud_resolve( &hud, &c, &out ) );
    ASSERT_EQ( out.texture_id, (uint32_t) 99 );
}

UTEST(hud_metadata, foreign_attachment_is_not_dereferenced) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_text( &hud, "score", "alpha" );
    vxui_draw_list dl = vxui_render( &ctx );
    vxui_draw_cmd c = *vxui_draw_nth( dl, VXUI_DRAW_TEXT, 0 );
    int foreign = 42;
    c.userdata = &foreign;
    vxui_render_data out = {};

    ASSERT_FALSE( vxui_hud_resolve( &hud, &c, &out ) );
    ASSERT_EQ( foreign, 42 );
}

UTEST(hud_metadata, begin_discards_previous_frame_records) {
    vxui_ctx ctx = make_ctx();
    vxui_hud hud = {};
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_text( &hud, "score", "alpha" );
    vxui_render( &ctx );

    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_text( &ctx, "score", "alpha", 24 );
    vxui_draw_list dl = vxui_render( &ctx );
    ASSERT_EQ( hud.item_count, 0 );
    ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_TEXT ), 1 );
    ASSERT_TRUE( dl.cmds[0].userdata == nullptr );
    vxui_render_data out = {};
    ASSERT_FALSE( vxui_hud_resolve( &hud, &dl.cmds[0], &out ) );
}

UTEST(hud_metadata, reused_command_slot_clears_attachment) {
    vxui_ctx ctx = make_ctx();
    vxui_hud hud = {};
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_text( &hud, "score", "alpha" );
    vxui_draw_list dl = vxui_render( &ctx );
    ASSERT_EQ( dl.cmds[0].userdata, (const void*) &hud.items[0] );

    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_rect( &ctx, "ordinary", { .width = { VXUI_FIXED, 40 }, .height = { VXUI_FIXED, 20 } } );
    vxui_div_end( &ctx );
    dl = vxui_render( &ctx );
    ASSERT_EQ( dl.count, 1 );
    ASSERT_TRUE( dl.cmds[0].userdata == nullptr );
    vxui_render_data out = {};
    ASSERT_FALSE( vxui_hud_resolve( &hud, &dl.cmds[0], &out ) );
}

UTEST(hud_metadata, changing_hud_text_does_not_reset_menu_focus) {
    vxui_ctx ctx = make_ctx();
    vxui_hud hud = {};
    const char* values[] = { "1", "alpha\nbeta", "alpha beta gamma" };
    for ( int frame = 0; frame < 3; frame++ )
    {
        vxui_frame( &ctx, 1.0f / 60.0f );
        ctx.input = frame == 1 ? VXUI_INPUT_DOWN : 0;
        vxui_hud_begin( &hud, &ctx, 960, 540 );
        vxui_hud_text_box( &hud, "score", values[frame], 48, 96, 0, 0 );
        if ( vxui_menu( &ctx, "main" ) )
        {
            vxui_menu_action( &ctx, "Play" );
            vxui_menu_action( &ctx, "Quit" );
            vxui_menu_end( &ctx );
        }
        vxui_draw_list dl = vxui_render( &ctx );

        uint32_t focused = frame == 0 ? 0u : 1u;
        ASSERT_EQ( ctx.menu_state[0].y, focused );
        ASSERT_EQ( vxui_draw_count( dl, VXUI_DRAW_RECT ), 2 );
        for ( int i = 0; i < 2; i++ )
        {
            const vxui_draw_cmd* c = vxui_draw_nth( dl, VXUI_DRAW_RECT, i );
            ASSERT_EQ( c->focused_row_index, (int) focused );
            ASSERT_EQ( ( c->state & VXUI_DRAW_FOCUSED ) != 0, i == (int) focused );
            ASSERT_TRUE( c->userdata == nullptr );
        }
    }
}

UTEST_MAIN();
