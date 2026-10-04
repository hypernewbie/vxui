#include "utest.h"
#include "../vxui.h"
#define VXUI_IMPL
#include "../vxui_impl.h"
#define VXUI_HUD_IMPL
#include "../vxui_hud.h"

static uint8_t s_clay_mem[16 * 1024 * 1024];
static int s_resolver_count = 0;

static Clay_Dimensions measure_text( Clay_StringSlice s, Clay_TextElementConfig* cfg, void* )
{
    return { (float) s.length * 8.0f, (float) cfg->fontSize };
}

static Clay_Dimensions measure_unit( Clay_StringSlice s, Clay_TextElementConfig*, void* )
{
    return { (float) s.length, 1.0f };
}

static vxui_ctx make_ctx()
{
    vxui_ctx ctx = {};
    vxui_init( &ctx, 960, 540, s_clay_mem, sizeof( s_clay_mem ) );
    Clay_SetMeasureTextFunction( measure_text, nullptr );
    return ctx;
}

static void poison_packets( vxui_ctx* ctx )
{
    vxui_draw_cmd stale     = {};
    stale.id                = 99;
    stale.type              = 99;
    stale.state             = 0xff;
    stale.clipped           = true;
    stale.rect              = { 9, 9, 9, 9 };
    stale.clip              = { 9, 9, 9, 9 };
    stale.focus_offset_y    = 9;
    stale.text              = "stale";
    stale.userdata          = ctx;
    stale.text_len          = 5;
    stale.font              = 7;
    stale.font_px           = 31;
    stale.row_index         = 9;
    stale.focused_row_index = 9;
    memset( &stale.render, 0xcd, sizeof( stale.render ) );
    for ( int i = 0; i < VXUI_MAX_DRAW_CMDS; i++ ) ctx->draw_buf[i] = stale;
}

static void snapshot_packet( const vxui_draw_cmd* cmd, vxui_render_data* out, void* userdata )
{
    vxui_draw_cmd* snapshots = (vxui_draw_cmd*) userdata;
    snapshots[s_resolver_count++] = *cmd;
    out->flags = cmd->id;
}

UTEST(draw_conversion, rectangle_overwrites_every_stale_truth_field) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    poison_packets( &ctx );
    vxui_rect( &ctx, "rect", { .width = { VXUI_FIXED, 40 }, .height = { VXUI_FIXED, 20 } } );
    vxui_div_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 1 );
    const vxui_draw_cmd& c = dl.cmds[0];
    ASSERT_EQ( c.id, vxui_id( "rect" ) );
    ASSERT_EQ( c.type, VXUI_DRAW_RECT );
    ASSERT_EQ( c.state, (uint8_t) 0 );
    ASSERT_FALSE( c.clipped );
    ASSERT_EQ( c.rect.z, 40.0f );
    ASSERT_EQ( c.rect.w, 20.0f );
    for ( int i = 0; i < 4; i++ ) ASSERT_EQ( c.clip[i], 0.0f );
    ASSERT_EQ( c.focus_offset_y, 0.0f );
    ASSERT_TRUE( c.text == nullptr );
    ASSERT_TRUE( c.userdata == nullptr );
    ASSERT_EQ( c.text_len, 0 );
    ASSERT_EQ( c.font, (uint16_t) 0 );
    ASSERT_EQ( c.font_px, (uint16_t) 0 );
    ASSERT_EQ( c.row_index, -1 );
    ASSERT_EQ( c.focused_row_index, -1 );
    vxui_render_data zero = {};
    ASSERT_EQ( memcmp( &c.render, &zero, sizeof( zero ) ), 0 );
}

UTEST(draw_conversion, text_keeps_clay_font_and_attachment_and_clears_stale_truth) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    poison_packets( &ctx );
    int attachment = 42;
    Clay__OpenElementWithId( vxui_clay_id( "text" ) );
    Clay__ConfigureOpenElement( {} );
    CLAY_TEXT( CLAY_STRING( "alpha" ), CLAY_TEXT_CONFIG( { .userData = &attachment, .fontId = 7, .fontSize = 31 } ) );
    Clay__CloseElement();
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 1 );
    const vxui_draw_cmd& c = dl.cmds[0];
    ASSERT_EQ( c.id, vxui_text_id( "text" ) );
    ASSERT_EQ( c.type, VXUI_DRAW_TEXT );
    ASSERT_EQ( c.state, (uint8_t) 0 );
    ASSERT_FALSE( c.clipped );
    for ( int i = 0; i < 4; i++ ) ASSERT_EQ( c.clip[i], 0.0f );
    ASSERT_EQ( c.focus_offset_y, 0.0f );
    ASSERT_EQ( c.text_len, 5 );
    ASSERT_EQ( memcmp( c.text, "alpha", 5 ), 0 );
    ASSERT_EQ( c.userdata, (const void*) &attachment );
    ASSERT_EQ( c.font, (uint16_t) 7 );
    ASSERT_EQ( c.font_px, (uint16_t) 31 );
    ASSERT_EQ( c.row_index, -1 );
    ASSERT_EQ( c.focused_row_index, -1 );
}

UTEST(draw_conversion, changing_packet_type_clears_previous_attachment_and_font) {
    vxui_ctx ctx = make_ctx();
    vxui_hud hud = {};
    for ( int frame = 0; frame < 3; frame++ )
    {
        vxui_frame( &ctx, 1.0f / 60.0f );
        vxui_hud_begin( &hud, &ctx, 960, 540 );
        if ( frame == 1 )
        {
            vxui_rect( &ctx, "rect", { .width = { VXUI_FIXED, 40 }, .height = { VXUI_FIXED, 20 } } );
            vxui_div_end( &ctx );
        }
        else
        {
            vxui_hud_set_font( &hud, 7, 31 );
            vxui_hud_text( &hud, "text", "alpha" );
        }
        vxui_draw_list dl = vxui_render( &ctx );

        ASSERT_EQ( dl.count, 1 );
        ASSERT_EQ( dl.cmds[0].type, frame == 1 ? VXUI_DRAW_RECT : VXUI_DRAW_TEXT );
        ASSERT_EQ( dl.cmds[0].font, (uint16_t) ( frame == 1 ? 0 : 7 ) );
        ASSERT_EQ( dl.cmds[0].font_px, (uint16_t) ( frame == 1 ? 0 : 31 ) );
        ASSERT_EQ( dl.cmds[0].text_len, frame == 1 ? 0 : 5 );
        ASSERT_EQ( dl.cmds[0].userdata, frame == 1 ? nullptr : (const void*) &hud.items[0] );
    }
}

UTEST(draw_conversion, rectangle_and_text_share_the_nested_clip_intersection) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    Clay__OpenElementWithId( vxui_clay_id( "outer" ) );
    Clay_ElementDeclaration outer = {};
    outer.layout.sizing.width     = CLAY_SIZING_FIXED( 80 );
    outer.layout.sizing.height    = CLAY_SIZING_FIXED( 60 );
    outer.layout.padding          = { 20, 0, 20, 0 };
    outer.clip.vertical           = true;
    Clay__ConfigureOpenElement( outer );
    Clay__OpenElementWithId( vxui_clay_id( "inner" ) );
    Clay_ElementDeclaration inner = {};
    inner.layout.sizing.width     = CLAY_SIZING_FIXED( 60 );
    inner.layout.sizing.height    = CLAY_SIZING_FIXED( 40 );
    inner.layout.layoutDirection  = CLAY_TOP_TO_BOTTOM;
    inner.clip.vertical           = true;
    Clay__ConfigureOpenElement( inner );
    vxui_rect( &ctx, "rect", { .width = { VXUI_FIXED, 10 }, .height = { VXUI_FIXED, 10 } } );
    vxui_div_end( &ctx );
    vxui_text( &ctx, "text", "A", 24 );
    Clay__CloseElement();
    Clay__CloseElement();
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 2 );
    ASSERT_EQ( dl.cmds[0].type, VXUI_DRAW_RECT );
    ASSERT_EQ( dl.cmds[1].type, VXUI_DRAW_TEXT );
    for ( int i = 0; i < 2; i++ )
    {
        ASSERT_TRUE( dl.cmds[i].clipped );
        ASSERT_EQ( dl.cmds[i].clip.x, 20.0f );
        ASSERT_EQ( dl.cmds[i].clip.y, 20.0f );
        ASSERT_EQ( dl.cmds[i].clip.z, 60.0f );
        ASSERT_EQ( dl.cmds[i].clip.w, 40.0f );
    }
}

UTEST(draw_conversion, unsupported_clay_commands_do_not_take_a_packet_slot) {
    vxui_ctx ctx = make_ctx();
    vxui_frame( &ctx, 1.0f / 60.0f );
    int image = 42;
    Clay__OpenElementWithId( vxui_clay_id( "image" ) );
    Clay_ElementDeclaration decl = {};
    decl.layout.sizing.width     = CLAY_SIZING_FIXED( 8 );
    decl.layout.sizing.height    = CLAY_SIZING_FIXED( 8 );
    decl.image.imageData         = &image;
    Clay__ConfigureOpenElement( decl );
    Clay__CloseElement();
    vxui_text( &ctx, "text", "A", 24 );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 1 );
    ASSERT_EQ( dl.cmds[0].id, vxui_text_id( "text" ) );
    ASSERT_EQ( dl.cmds[0].type, VXUI_DRAW_TEXT );
}

UTEST(draw_conversion, mixed_packets_keep_producer_order_and_zero_resolver_inputs) {
    vxui_ctx ctx = make_ctx();
    vxui_draw_cmd snapshots[3] = {};
    s_resolver_count = 0;
    vxui_set_render_data_fn( &ctx, snapshot_packet, snapshots );
    vxui_frame( &ctx, 1.0f / 60.0f );
    poison_packets( &ctx );
    vxui_rect( &ctx, "first", { .width = { VXUI_FIXED, 10 }, .height = { VXUI_FIXED, 10 } } );
    vxui_div_end( &ctx );
    vxui_text( &ctx, "middle", "alpha", 24 );
    vxui_rect( &ctx, "last", { .width = { VXUI_FIXED, 10 }, .height = { VXUI_FIXED, 10 } } );
    vxui_div_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 3 );
    ASSERT_EQ( s_resolver_count, 3 );
    uint32_t ids[] = { vxui_id( "first" ), vxui_text_id( "middle" ), vxui_id( "last" ) };
    vxui_render_data zero = {};
    for ( int i = 0; i < 3; i++ )
    {
        ASSERT_EQ( dl.cmds[i].id, ids[i] );
        ASSERT_EQ( snapshots[i].id, ids[i] );
        ASSERT_EQ( memcmp( &snapshots[i].render, &zero, sizeof( zero ) ), 0 );
        ASSERT_EQ( dl.cmds[i].render.flags, ids[i] );
    }
}

UTEST(draw_conversion, exact_mixed_capacity_has_no_uninitialised_packet_fields) {
    vxui_ctx ctx = make_ctx();
    Clay_SetMeasureTextFunction( measure_unit, nullptr );
    char ids[VXUI_MAX_DRAW_CMDS][16] = {};
    vxui_frame( &ctx, 1.0f / 60.0f );
    poison_packets( &ctx );
    for ( int i = 0; i < VXUI_MAX_DRAW_CMDS; i++ )
    {
        snprintf( ids[i], sizeof( ids[i] ), "cmd%d", i );
        if ( i & 1 ) vxui_text( &ctx, ids[i], "A", 1 );
        else
        {
            vxui_rect( &ctx, ids[i], { .width = { VXUI_FIXED, 1 }, .height = { VXUI_FIXED, 1 } } );
            vxui_div_end( &ctx );
        }
    }
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, VXUI_MAX_DRAW_CMDS );
    for ( int i = 0; i < dl.count; i++ )
    {
        const vxui_draw_cmd& c = dl.cmds[i];
        ASSERT_EQ( c.id, i & 1 ? vxui_text_id( ids[i] ) : vxui_id( ids[i] ) );
        ASSERT_EQ( c.type, i & 1 ? VXUI_DRAW_TEXT : VXUI_DRAW_RECT );
        ASSERT_EQ( c.state, (uint8_t) 0 );
        ASSERT_FALSE( c.clipped );
        ASSERT_TRUE( c.userdata == nullptr );
        ASSERT_EQ( c.row_index, -1 );
        ASSERT_EQ( c.focused_row_index, -1 );
        ASSERT_EQ( c.font, (uint16_t) 0 );
        ASSERT_EQ( c.font_px, (uint16_t) ( i & 1 ? 1 : 0 ) );
        ASSERT_EQ( c.text_len, i & 1 ? 1 : 0 );
    }
}

UTEST(draw_conversion, empty_frame_does_not_resolve_stale_slots) {
    vxui_ctx ctx = make_ctx();
    vxui_draw_cmd snapshot = {};
    s_resolver_count = 0;
    vxui_set_render_data_fn( &ctx, snapshot_packet, &snapshot );
    vxui_frame( &ctx, 1.0f / 60.0f );
    poison_packets( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 0 );
    ASSERT_EQ( s_resolver_count, 0 );
}

UTEST_MAIN();
