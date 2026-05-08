#include "utest.h"
#include "../vxui.h"
#define VXUI_IMPL
#include "../vxui_impl.h"
#include <cstdio>
#include <filesystem>
#include <vector>

static uint8_t s_clay_mem[16 * 1024 * 1024];

static const std::vector< uint8_t >& roboto_bytes()
{
    static std::vector< uint8_t > bytes = []() {
        std::filesystem::path p = std::filesystem::path( __FILE__ ).parent_path().parent_path() / "fonts" / "Roboto-Regular.ttf";
        FILE* f = fopen( p.string().c_str(), "rb" );
        assert( f && "roboto_bytes: failed to open font file" );
        fseek( f, 0, SEEK_END );
        long sz = ftell( f );
        fseek( f, 0, SEEK_SET );
        std::vector< uint8_t > buf( (size_t) sz );
        size_t got = fread( buf.data(), 1, (size_t) sz, f );
        (void) got;
        fclose( f );
        return buf;
    }();
    return bytes;
}

static vxui_ctx make_ctx()
{
    vxui_ctx ctx = {};
    vxui_init( &ctx, 1280, 720, s_clay_mem, sizeof( s_clay_mem ) );
    const std::vector< uint8_t >& b = roboto_bytes();
    vxui_load_font( &ctx, b.data(), b.size(), (float) VXUI_FONT_SIZE_DEFAULT );
    return ctx;
}

// Compute the row id for a given menu+label, mirroring vxui_menu_open_row.
static uint32_t row_id( const char* menu, const char* label )
{
    Clay_String cs = { false, (int32_t) strlen( label ), label };
    return Clay__HashString( cs, vxui_hash( menu ) ).id;
}

UTEST(menu_text, longer_label_yields_wider_row) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "test" ) )
    {
        vxui_menu_action( &ctx, "Hi" );
        vxui_menu_action( &ctx, "Continue Playing" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* hi   = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "test", "Hi" ) );
    const vxui_draw_cmd* long_ = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "test", "Continue Playing" ) );
    ASSERT_TRUE( hi != nullptr );
    ASSERT_TRUE( long_ != nullptr );
    ASSERT_LT( hi->rect.z, long_->rect.z );
}

UTEST(menu_text, focused_row_has_focused_bit_with_font) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "test" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_action( &ctx, "Options" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* play = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "test", "Play" ) );
    ASSERT_TRUE( play != nullptr );
    ASSERT_TRUE( ( play->state & VXUI_DRAW_FOCUSED ) != 0 );
}

UTEST(menu_text, focused_bit_moves_with_navigation) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "test" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_action( &ctx, "Continue Playing" );
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );

    vxui_frame( &ctx, 1.0f / 60.0f );
    ctx.input = VXUI_INPUT_DOWN;
    if ( vxui_menu( &ctx, "test" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_action( &ctx, "Continue Playing" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* cp   = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "test", "Continue Playing" ) );
    const vxui_draw_cmd* play = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "test", "Play" ) );
    ASSERT_TRUE( cp   != nullptr );
    ASSERT_TRUE( play != nullptr );
    ASSERT_TRUE( ( cp->state   & VXUI_DRAW_FOCUSED ) != 0 );
    ASSERT_EQ  ( play->state   & VXUI_DRAW_FOCUSED, (uint8_t) 0 );
}

UTEST(menu_text, no_font_loaded_emits_zero_width_rows) {
    vxui_ctx ctx = {};
    vxui_init( &ctx, 1280, 720, s_clay_mem, sizeof( s_clay_mem ) );

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "test" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    const vxui_draw_cmd* play = vxui_draw_find( dl, VXUI_DRAW_RECT, row_id( "test", "Play" ) );
    ASSERT_TRUE( play != nullptr );
    ASSERT_NEAR( play->rect.z, 0.0f, 0.01f );
}

UTEST_MAIN();
