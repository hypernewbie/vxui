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

UTEST(menu_collision, name_pointer_stored_on_allocate) {
    vxui_ctx ctx = make_ctx();
    const char* name = "main_menu";

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, name ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );

    ASSERT_TRUE( ctx.menu_names[0] == name );
}

UTEST(menu_collision, same_name_reuses_slot_across_frames) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "settings" ) )
    {
        vxui_menu_action( &ctx, "Audio" );
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "settings" ) )
    {
        vxui_menu_action( &ctx, "Audio" );
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );

    ASSERT_EQ( ctx.menu_count, 1 );
}

UTEST(menu_collision, distinct_names_get_distinct_slots) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "a" ) )
    {
        vxui_menu_action( &ctx, "X" );
        vxui_menu_end( &ctx );
    }
    if ( vxui_menu( &ctx, "b" ) )
    {
        vxui_menu_action( &ctx, "Y" );
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );

    ASSERT_EQ( ctx.menu_count, 2 );
    ASSERT_NE( ctx.menu_state[0].x, ctx.menu_state[1].x );
    ASSERT_TRUE( strcmp( ctx.menu_names[0], "a" ) == 0 );
    ASSERT_TRUE( strcmp( ctx.menu_names[1], "b" ) == 0 );
}

UTEST(menu_collision, separate_string_buffers_with_same_content_match) {
    vxui_ctx ctx = make_ctx();
    char buf1[16] = "main";
    char buf2[16] = "main";

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, buf1 ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, buf2 ) )
    {
        vxui_menu_action( &ctx, "Play" );
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );

    ASSERT_EQ( ctx.menu_count, 1 );
}

UTEST(menu_collision, name_pointer_persists_across_frames) {
    vxui_ctx ctx = make_ctx();
    const char* name = "persistent";

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, name ) )
    {
        vxui_menu_action( &ctx, "A" );
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, name ) )
    {
        vxui_menu_action( &ctx, "A" );
        vxui_menu_end( &ctx );
    }
    vxui_render( &ctx );

    ASSERT_TRUE( ctx.menu_names[0] == name );
}

UTEST(menu_collision, multiple_distinct_names_all_stored_correctly) {
    vxui_ctx ctx = make_ctx();

    vxui_frame( &ctx, 1.0f / 60.0f );
    if ( vxui_menu( &ctx, "title"    ) ) { vxui_menu_action( &ctx, "Play" );  vxui_menu_end( &ctx ); }
    if ( vxui_menu( &ctx, "options"  ) ) { vxui_menu_action( &ctx, "Audio" ); vxui_menu_end( &ctx ); }
    if ( vxui_menu( &ctx, "controls" ) ) { vxui_menu_action( &ctx, "Bind" );  vxui_menu_end( &ctx ); }
    if ( vxui_menu( &ctx, "credits"  ) ) { vxui_menu_action( &ctx, "Back" );  vxui_menu_end( &ctx ); }
    vxui_render( &ctx );

    ASSERT_EQ( ctx.menu_count, 4 );
    ASSERT_TRUE( strcmp( ctx.menu_names[0], "title"    ) == 0 );
    ASSERT_TRUE( strcmp( ctx.menu_names[1], "options"  ) == 0 );
    ASSERT_TRUE( strcmp( ctx.menu_names[2], "controls" ) == 0 );
    ASSERT_TRUE( strcmp( ctx.menu_names[3], "credits"  ) == 0 );
}

UTEST_MAIN();
