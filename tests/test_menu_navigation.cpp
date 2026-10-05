#include "utest.h"
#include "../vxui.h"
#define VXUI_IMPL
#include "../vxui_impl.h"

static uint32_t reference_next( uint32_t from, uint32_t count, uint32_t skip, int step, bool wrap )
{
    for ( uint32_t i = 0; i < count; i++ )
    {
        int candidate = (int) from + ( (int) i + 1 ) * step;
        if ( !wrap && ( candidate < 0 || candidate >= (int) count ) ) return from;
        while ( candidate < 0 ) candidate += (int) count;
        candidate %= (int) count;
        if ( !( skip & ( 1u << candidate ) ) ) return (uint32_t) candidate;
    }
    return from;
}

UTEST(menu_navigation, exhaustive_small_skip_masks_match_a_signed_reference_walk) {
    for ( uint32_t count = 1; count <= 8; count++ )
        for ( uint32_t skip = 0; skip < ( 1u << count ); skip++ )
            for ( uint32_t from = 0; from < count; from++ )
                for ( int step : { -1, 1 } )
                    for ( bool wrap : { false, true } )
                        ASSERT_EQ( vxui_menu_next_row( from, count, skip, step, wrap ), reference_next( from, count, skip, step, wrap ) );
}

UTEST(menu_navigation, every_row_count_and_origin_match_patterned_full_width_masks) {
    uint32_t patterns[] = { 0, UINT32_MAX, 0xaaaaaaaau, 0x55555555u, 0x80000001u, 0x7ffffffeu, 0x01010101u, 0xf00ff00fu };
    for ( uint32_t count = 1; count <= VXUI_MAX_MENU_ROWS; count++ )
        for ( uint32_t skip : patterns )
            for ( uint32_t from = 0; from < count; from++ )
                for ( int step : { -1, 1 } )
                    for ( bool wrap : { false, true } )
                        ASSERT_EQ( vxui_menu_next_row( from, count, skip, step, wrap ), reference_next( from, count, skip, step, wrap ) );
}

UTEST(menu_navigation, a_single_unskipped_row_is_reachable_from_every_origin_when_wrapping) {
    for ( uint32_t target = 0; target < VXUI_MAX_MENU_ROWS; target++ )
        for ( uint32_t from = 0; from < VXUI_MAX_MENU_ROWS; from++ )
            for ( int step : { -1, 1 } )
                ASSERT_EQ( vxui_menu_next_row( from, VXUI_MAX_MENU_ROWS, ~( 1u << target ), step, true ), target );
}

UTEST(menu_navigation, all_skipped_rows_and_empty_counts_keep_the_origin) {
    ASSERT_EQ( vxui_menu_next_row( 0, 0, 0, 1, true ), 0u );
    for ( uint32_t count = 1; count <= VXUI_MAX_MENU_ROWS; count++ )
        for ( uint32_t from = 0; from < count; from++ )
            for ( int step : { -1, 1 } )
                for ( bool wrap : { false, true } )
                    ASSERT_EQ( vxui_menu_next_row( from, count, UINT32_MAX, step, wrap ), from );
}

UTEST(menu_navigation, shrink_search_from_one_past_the_last_row_preserves_the_last_valid_row) {
    for ( uint32_t count = 1; count <= VXUI_MAX_MENU_ROWS; count++ )
        for ( uint32_t target = 0; target < count; target++ )
            ASSERT_EQ( vxui_menu_next_row( count, count, ~( 1u << target ), -1, false ), target );
}

UTEST(menu_navigation, nonwrapping_edges_never_jump_past_the_origin) {
    for ( uint32_t count = 1; count <= VXUI_MAX_MENU_ROWS; count++ )
    {
        ASSERT_EQ( vxui_menu_next_row( 0, count, 0, -1, false ), 0u );
        ASSERT_EQ( vxui_menu_next_row( count - 1, count, 0, 1, false ), count - 1 );
    }
}

UTEST(menu_navigation, the_high_skip_bit_and_wrapping_boundary_are_distinct) {
    ASSERT_EQ( vxui_menu_next_row( 30, 32, 1u << 31, 1, true ), 0u );
    ASSERT_EQ( vxui_menu_next_row( 30, 32, 1u << 31, 1, false ), 30u );
    ASSERT_EQ( vxui_menu_next_row( 0, 32, 1u << 31, -1, true ), 30u );
    ASSERT_EQ( vxui_menu_next_row( 0, 32, 1u << 31, -1, false ), 0u );
}

UTEST(menu_navigation, random_skip_masks_match_the_reference_at_maximum_rows) {
    uint32_t random = 123456789;
    for ( int i = 0; i < 256; i++ )
    {
        random ^= random << 13;
        random ^= random >> 17;
        random ^= random << 5;
        for ( uint32_t from = 0; from < VXUI_MAX_MENU_ROWS; from++ )
            for ( int step : { -1, 1 } )
                for ( bool wrap : { false, true } )
                    ASSERT_EQ( vxui_menu_next_row( from, 32, random, step, wrap ), reference_next( from, 32, random, step, wrap ) );
    }
}

UTEST_MAIN();
