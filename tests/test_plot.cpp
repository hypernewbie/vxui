#include "utest.h"
#include "../vxui.h"
#define VXUI_IMPL
#include "../vxui_impl.h"
#include "dump_layout.h"
#include <cstdio>
#include <filesystem>
#include <vector>

static uint8_t s_clay_mem[16 * 1024 * 1024];

static const char* g_labels[32];
static const char* g_menu = "";

static const char* label_for( uint32_t id )
{
    Clay_String fs = CLAY_STRING( "focus" );
    if ( Clay__HashString( fs, vxui_hash( g_menu ) ).id == id ) return "<focus>";
    for ( int i = 0; g_labels[i]; i++ )
    {
        Clay_String cs = { false, (int32_t) strlen( g_labels[i] ), g_labels[i] };
        if ( Clay__HashString( cs, vxui_hash( g_menu ) ).id == id ) return g_labels[i];
    }
    return nullptr;
}

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

static vxui_ctx make_ctx( float w = 1280, float h = 720 )
{
    vxui_ctx ctx = {};
    vxui_init( &ctx, w, h, s_clay_mem, sizeof( s_clay_mem ) );
    const std::vector< uint8_t >& b = roboto_bytes();
    vxui_load_font( &ctx, b.data(), b.size(), (float) VXUI_FONT_SIZE_DEFAULT );
    return ctx;
}

UTEST(plot, title_menu) {
    vxui_ctx ctx = make_ctx();
    g_menu = "title";
    static const char* labels[] = { "Play", "Options", "Quit", nullptr };
    memcpy( g_labels, labels, sizeof( labels ) );

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "title" ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_action( &ctx, "Options" );
        vxui_menu_action( &ctx, "Quit" );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    dump_layout( dl, 1280, 720, ctx.dt, "tmp/title_menu.layout", label_for );
}

UTEST(plot, scrollable_menu) {
    vxui_ctx ctx = make_ctx();
    g_menu = "scroll";
    static const char* labels[] = {
        "Row0", "Row1", "Row2", "Row3", "Row4", "Row5", "Row6", "Row7", nullptr
    };
    memcpy( g_labels, labels, sizeof( labels ) );

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "scroll", true, 3 ) ) { vxui_menu_end( &ctx ); }
    vxui_render( &ctx );

    for ( int i = 0; i < 4; i++ )
    {
        vxui_frame( &ctx, 1.0f / 60.0f );
        ctx.input = VXUI_INPUT_DOWN;
        if ( vxui_menu( &ctx, "scroll", true, 3 ) )
        {
            for ( int r = 0; labels[r]; r++ ) vxui_menu_action( &ctx, labels[r] );
            vxui_menu_end( &ctx );
        }
        vxui_render( &ctx );
    }

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "scroll", true, 3 ) )
    {
        for ( int r = 0; labels[r]; r++ ) vxui_menu_action( &ctx, labels[r] );
        vxui_menu_end( &ctx );
    }
    vxui_draw_list dl = vxui_render( &ctx );

    dump_layout( dl, 1280, 720, ctx.dt, "tmp/scrollable_menu.layout", label_for );
}

UTEST(plot, menu_in_div) {
    vxui_ctx ctx = make_ctx();
    g_menu = "in_div";
    static const char* labels[] = { "Apple", "Banana", "Cherry", nullptr };
    memcpy( g_labels, labels, sizeof( labels ) );

    vxui_div_cfg outer = {};
    outer.padding[0] = 100; outer.padding[1] = 100;
    outer.padding[2] = 60;  outer.padding[3] = 60;

    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_div( &ctx, "outer", outer );
    if ( vxui_menu( &ctx, "in_div" ) )
    {
        vxui_menu_action( &ctx, "Apple" );
        vxui_menu_action( &ctx, "Banana" );
        vxui_menu_action( &ctx, "Cherry" );
        vxui_menu_end( &ctx );
    }
    vxui_div_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    dump_layout( dl, 1280, 720, ctx.dt, "tmp/menu_in_div.layout", label_for );
}

UTEST(plot, root_absolute) {
    vxui_ctx ctx = make_ctx();
    g_menu = "hud";
    static const char* labels[] = { "Status", "Score", nullptr };
    memcpy( g_labels, labels, sizeof( labels ) );

    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_root( &ctx, "hud_root", 50.0f, 400.0f );
    if ( vxui_menu( &ctx, "hud" ) )
    {
        vxui_menu_action( &ctx, "Status" );
        vxui_menu_action( &ctx, "Score" );
        vxui_menu_end( &ctx );
    }
    vxui_root_end( &ctx );
    vxui_draw_list dl = vxui_render( &ctx );

    dump_layout( dl, 1280, 720, ctx.dt, "tmp/root_absolute.layout", label_for );
}

UTEST(plot, spring_sequence) {
    vxui_ctx ctx = make_ctx();
    g_menu = "spring";
    static const char* labels[] = { "Alpha", "Beta", "Gamma", "Delta", nullptr };
    memcpy( g_labels, labels, sizeof( labels ) );

    auto run = [&]( uint32_t input ) {
        vxui_frame( &ctx, 1.0f / 60.0f );
        ctx.input = input;
        if ( vxui_menu( &ctx, "spring" ) )
        {
            for ( int i = 0; labels[i]; i++ ) vxui_menu_action( &ctx, labels[i] );
            vxui_menu_end( &ctx );
        }
        return vxui_render( &ctx );
    };

    run( 0 );
    run( VXUI_INPUT_DOWN );

    char path[64];
    for ( int i = 0; i < 30; i++ )
    {
        vxui_draw_list dl = run( 0 );
        snprintf( path, sizeof( path ), "tmp/spring_%03d.layout", i + 1 );
        dump_layout( dl, 1280, 720, ctx.dt, path, label_for );
    }
}

UTEST_MAIN();
