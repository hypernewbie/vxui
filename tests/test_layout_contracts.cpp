#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "../third_party/utest.h"
#include "../vxui.h"
#include "test_layout_helpers.h"

#ifdef VXUI_DEBUG

typedef struct layout_fixture
{
    uint8_t* memory;
    uint64_t memory_size;
    vxui_ctx ctx;
} layout_fixture;

UTEST_F_SETUP( layout_fixture )
{
    utest_fixture->memory_size = vxui_min_memory_size();
    utest_fixture->memory = ( uint8_t* ) std::malloc( ( size_t ) utest_fixture->memory_size );
    ASSERT_TRUE( utest_fixture->memory != nullptr );

    vxui_init(
        &utest_fixture->ctx,
        vxui_create_arena( utest_fixture->memory_size, utest_fixture->memory ),
        ( vxui_config ) {
            .screen_width = 640,
            .screen_height = 360,
            .max_elements = 128,
        } );
}

UTEST_F_TEARDOWN( layout_fixture )
{
    vxui_shutdown( &utest_fixture->ctx );
    std::free( utest_fixture->memory );
    utest_fixture->memory = nullptr;
    utest_fixture->memory_size = 0;
}

UTEST_F( layout_fixture, debug_layout_issue_count_starts_at_zero )
{
    vxui_begin( &utest_fixture->ctx, 0.016f );
    vxui_end( &utest_fixture->ctx );
    EXPECT_EQ( vxui_debug_layout_issue_count( &utest_fixture->ctx ), 0 );
}

UTEST_F( layout_fixture, debug_clear_layout_issues_resets_count )
{
    vxui_begin( &utest_fixture->ctx, 0.016f );
    vxui_end( &utest_fixture->ctx );
    vxui_debug_clear_layout_issues( &utest_fixture->ctx );
    EXPECT_EQ( vxui_debug_layout_issue_count( &utest_fixture->ctx ), 0 );
}

UTEST_F( layout_fixture, geometry_helpers_rect_contains )
{
    vxui_rect outer = { 0, 0, 100, 100 };
    vxui_rect inner = { 10, 10, 20, 20 };
    vxui_rect outside = { 50, 50, 60, 60 };

    EXPECT_TRUE( vxui_layout_helpers::rect_contains( outer, inner ) );
    EXPECT_FALSE( vxui_layout_helpers::rect_contains( outer, outside ) );
}

UTEST_F( layout_fixture, geometry_helpers_gap_between_adjacent_rects )
{
    vxui_rect left = { 0, 0, 50, 50 };
    vxui_rect right = { 60, 0, 50, 50 };

    float gap = vxui_layout_helpers::horizontal_gap_between( left, right );
    EXPECT_EQ( gap, 10.0f );
}

UTEST_F( layout_fixture, geometry_helpers_width_in_range )
{
    vxui_rect r = { 0, 0, 100, 50 };
    EXPECT_TRUE( vxui_layout_helpers::width_in_range( r, 80.0f, 120.0f ) );
    EXPECT_FALSE( vxui_layout_helpers::width_in_range( r, 40.0f, 60.0f ) );
}

UTEST_F( layout_fixture, geometry_helpers_is_approx_aligned_x )
{
    EXPECT_TRUE( vxui_layout_helpers::is_approx_aligned_x( 100.0f, 100.0f, 0.5f ) );
    EXPECT_TRUE( vxui_layout_helpers::is_approx_aligned_x( 100.0f, 100.4f, 0.5f ) );
    EXPECT_FALSE( vxui_layout_helpers::is_approx_aligned_x( 100.0f, 101.0f, 0.5f ) );
}

UTEST_F( layout_fixture, issue_kind_name_returns_valid_string )
{
    const char* name = vxui_layout_issue_kind_name( VXUI_LAYOUT_ISSUE_TINY_FOCUSABLE_BOUNDS );
    EXPECT_STRNE( name, "" );
    EXPECT_STRNE( name, "unknown" );

    name = vxui_layout_issue_kind_name( VXUI_LAYOUT_ISSUE_FOCUS_RING_DECGENERATE );
    EXPECT_STRNE( name, "" );
    EXPECT_STRNE( name, "unknown" );
}

UTEST_F( layout_fixture, log_layout_issues_toggle_works )
{
    vxui_set_debug_log_layout_issues( &utest_fixture->ctx, false );
    EXPECT_FALSE( utest_fixture->ctx.layout_issues_log_to_stderr );

    vxui_set_debug_log_layout_issues( &utest_fixture->ctx, true );
    EXPECT_TRUE( utest_fixture->ctx.layout_issues_log_to_stderr );
}

#else

UTEST( layout_contracts, requires_debug_build )
{
    UTEST_SKIP();
}

#endif
