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

static void resolve_hud( const vxui_draw_cmd* c, vxui_render_data* out, void* userdata )
{
    vxui_hud_resolve( (vxui_hud*) userdata, c, out );
}

static void init_ctx( vxui_ctx* ctx, vxui_hud* hud )
{
    *ctx = {};
    vxui_init( ctx, 200, 200, s_clay_mem, sizeof( s_clay_mem ) );
    Clay_SetMeasureTextFunction( measure_text, nullptr );
    vxui_set_render_data_fn( ctx, resolve_hud, hud );
}

UTEST(hud_z, equal_z_keeps_declaration_order) {
    vxui_ctx ctx;
    vxui_hud hud = {};
    init_ctx( &ctx, &hud );
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud_begin( &hud, &ctx, 200, 200 );
    vxui_hud_image( &hud, "first", 41, 40, 20 );
    vxui_hud_image( &hud, "second", 42, 40, 20 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 2 );
    ASSERT_EQ( dl.cmds[0].id, vxui_id( "first" ) );
    ASSERT_EQ( dl.cmds[1].id, vxui_id( "second" ) );
    ASSERT_EQ( dl.cmds[0].render.texture_id, (uint32_t) 41 );
    ASSERT_EQ( dl.cmds[1].render.texture_id, (uint32_t) 42 );
}

UTEST(hud_z, higher_z_draws_last_without_reordering_records) {
    vxui_ctx ctx;
    vxui_hud hud = {};
    init_ctx( &ctx, &hud );
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud_begin( &hud, &ctx, 200, 200 );
    vxui_hud_set_z( &hud, 7 );
    vxui_hud_set_colour( &hud, { 1, 0, 0, 1 } );
    vxui_hud_image( &hud, "high", 41, 40, 20 );
    vxui_hud_set_z( &hud, 0 );
    vxui_hud_set_colour( &hud, { 0, 1, 0, 1 } );
    vxui_hud_image( &hud, "low", 42, 40, 20 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 2 );
    ASSERT_EQ( dl.cmds[0].id, vxui_id( "low" ) );
    ASSERT_EQ( dl.cmds[1].id, vxui_id( "high" ) );
    ASSERT_EQ( dl.cmds[0].userdata, (const void*) &hud.items[1] );
    ASSERT_EQ( dl.cmds[1].userdata, (const void*) &hud.items[0] );
    ASSERT_EQ( dl.cmds[0].render.colour.g, 1.0f );
    ASSERT_EQ( dl.cmds[1].render.colour.r, 1.0f );
    ASSERT_EQ( dl.cmds[0].render.texture_id, (uint32_t) 42 );
    ASSERT_EQ( dl.cmds[1].render.texture_id, (uint32_t) 41 );
}

UTEST(hud_z, negative_z_draws_before_default_z) {
    vxui_ctx ctx;
    vxui_hud hud = {};
    init_ctx( &ctx, &hud );
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud_begin( &hud, &ctx, 200, 200 );
    vxui_hud_image( &hud, "default", 41, 40, 20 );
    vxui_hud_set_z( &hud, -7 );
    vxui_hud_image( &hud, "negative", 42, 40, 20 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 2 );
    ASSERT_EQ( dl.cmds[0].id, vxui_id( "negative" ) );
    ASSERT_EQ( dl.cmds[1].id, vxui_id( "default" ) );
    ASSERT_EQ( dl.cmds[0].render.texture_id, (uint32_t) 42 );
    ASSERT_EQ( dl.cmds[1].render.texture_id, (uint32_t) 41 );
}

UTEST(hud_z, descending_declarations_keep_metadata_on_sorted_commands) {
    vxui_ctx ctx;
    vxui_hud hud = {};
    init_ctx( &ctx, &hud );
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud_begin( &hud, &ctx, 200, 200 );
    vxui_hud_set_z( &hud, 7 );
    vxui_hud_image( &hud, "high", 41, 40, 20 );
    vxui_hud_set_z( &hud, 3 );
    vxui_hud_image( &hud, "middle", 42, 40, 20 );
    vxui_hud_set_z( &hud, -2 );
    vxui_hud_image( &hud, "low", 43, 40, 20 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 3 );
    const char* ids[] = { "low", "middle", "high" };
    for ( int i = 0; i < 3; i++ )
    {
        ASSERT_EQ( dl.cmds[i].id, vxui_id( ids[i] ) );
        ASSERT_EQ( dl.cmds[i].userdata, (const void*) &hud.items[2 - i] );
        ASSERT_EQ( dl.cmds[i].render.texture_id, (uint32_t) ( 43 - i ) );
    }
}

UTEST(hud_z, every_text_line_keeps_the_sorted_declaration_metadata) {
    vxui_ctx ctx;
    vxui_hud hud = {};
    init_ctx( &ctx, &hud );
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud_begin( &hud, &ctx, 200, 200 );
    vxui_hud_set_z( &hud, 7 );
    vxui_hud_set_colour( &hud, { 1, 0, 0, 1 } );
    vxui_hud_text( &hud, "high", "alpha\nbeta" );
    vxui_hud_set_z( &hud, -7 );
    vxui_hud_set_colour( &hud, { 0, 1, 0, 1 } );
    vxui_hud_text_box( &hud, "low", "gamma\ndelta", 100, 60 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 4 );
    for ( int i = 0; i < 4; i++ )
    {
        ASSERT_EQ( dl.cmds[i].type, VXUI_DRAW_TEXT );
        ASSERT_EQ( dl.cmds[i].userdata, (const void*) &hud.items[i < 2 ? 1 : 0] );
        ASSERT_EQ( dl.cmds[i].render.colour.r, i < 2 ? 0.0f : 1.0f );
        ASSERT_EQ( dl.cmds[i].render.colour.g, i < 2 ? 1.0f : 0.0f );
    }
}

UTEST(hud_z, changing_current_z_does_not_change_pending_declarations) {
    vxui_ctx ctx;
    vxui_hud hud = {};
    init_ctx( &ctx, &hud );
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud_begin( &hud, &ctx, 200, 200 );
    vxui_hud_set_z( &hud, 7 );
    vxui_hud_image( &hud, "high", 41, 40, 20 );
    vxui_hud_set_z( &hud, -7 );
    vxui_hud_image( &hud, "low", 42, 40, 20 );
    vxui_hud_set_z( &hud, 100 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 2 );
    ASSERT_EQ( dl.cmds[0].id, vxui_id( "low" ) );
    ASSERT_EQ( dl.cmds[1].id, vxui_id( "high" ) );
    ASSERT_EQ( dl.cmds[0].render.texture_id, (uint32_t) 42 );
    ASSERT_EQ( dl.cmds[1].render.texture_id, (uint32_t) 41 );
}

UTEST(hud_z, begin_resets_layering_when_record_slots_are_reused) {
    vxui_ctx ctx;
    vxui_hud hud = {};
    init_ctx( &ctx, &hud );
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud_begin( &hud, &ctx, 200, 200 );
    vxui_hud_set_z( &hud, 7 );
    vxui_hud_image( &hud, "first", 41, 40, 20 );
    vxui_hud_set_z( &hud, -7 );
    vxui_hud_image( &hud, "second", 42, 40, 20 );
    vxui_render( &ctx );

    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud_begin( &hud, &ctx, 200, 200 );
    vxui_hud_image( &hud, "first", 43, 40, 20 );
    vxui_hud_image( &hud, "second", 44, 40, 20 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( hud.z, 0 );
    ASSERT_EQ( dl.count, 2 );
    ASSERT_EQ( dl.cmds[0].id, vxui_id( "first" ) );
    ASSERT_EQ( dl.cmds[1].id, vxui_id( "second" ) );
    ASSERT_EQ( dl.cmds[0].render.texture_id, (uint32_t) 43 );
    ASSERT_EQ( dl.cmds[1].render.texture_id, (uint32_t) 44 );
}

UTEST(hud_z, signed_z_limits_sort_without_losing_metadata) {
    vxui_ctx ctx;
    vxui_hud hud = {};
    init_ctx( &ctx, &hud );
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud_begin( &hud, &ctx, 200, 200 );
    vxui_hud_set_z( &hud, 32767 );
    vxui_hud_image( &hud, "maximum", 41, 40, 20 );
    vxui_hud_set_z( &hud, -32768 );
    vxui_hud_image( &hud, "minimum", 42, 40, 20 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 2 );
    ASSERT_EQ( dl.cmds[0].id, vxui_id( "minimum" ) );
    ASSERT_EQ( dl.cmds[1].id, vxui_id( "maximum" ) );
    ASSERT_EQ( dl.cmds[0].render.texture_id, (uint32_t) 42 );
    ASSERT_EQ( dl.cmds[1].render.texture_id, (uint32_t) 41 );
}

UTEST_MAIN();
