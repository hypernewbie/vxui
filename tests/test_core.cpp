#include <cstdint>
#include <vector>

#include "../third_party/utest.h"
#include "../vxui.h"

static vxui_config vxui__test_config( void )
{
    vxui_config cfg = {};
    cfg.screen_width = 1280;
    cfg.screen_height = 720;
    cfg.exit_anim_max_time = 300;
    cfg.focus_ring.color = ( vxui_color ) { 10, 20, 30, 255 };
    cfg.focus_ring.border_width = 3.0f;
    cfg.focus_ring.corner_radius = 6.0f;
    cfg.focus_ring.spring_stiffness = 150.0f;
    cfg.focus_ring.spring_damping = 15.0f;
    cfg.max_elements = 8;
    cfg.max_anim_states = 16;
    cfg.max_sequences = 4;
    cfg.max_seq_steps = 32;
    return cfg;
}

UTEST( core, min_memory_size_is_non_zero )
{
    EXPECT_TRUE( vxui_min_memory_size() > 0 );
}

UTEST( core, create_arena_initializes_fields )
{
    std::vector< uint8_t > memory( 128 );
    vxui_arena arena = vxui_create_arena( ( uint64_t ) memory.size(), memory.data() );

    EXPECT_EQ( arena.memory, memory.data() );
    EXPECT_EQ( arena.capacity, ( uint64_t ) memory.size() );
    EXPECT_EQ( arena.offset, ( uint64_t ) 0 );
}

UTEST( core, init_stores_config_and_zeroes_transient_state )
{
    std::vector< uint8_t > memory( 4096 );
    vxui_ctx ctx = {};
    vxui_config cfg = vxui__test_config();

    vxui_init( &ctx, vxui_create_arena( ( uint64_t ) memory.size(), memory.data() ), cfg );

    EXPECT_EQ( ctx.cfg.screen_width, cfg.screen_width );
    EXPECT_EQ( ctx.cfg.screen_height, cfg.screen_height );
    EXPECT_EQ( ctx.cfg.exit_anim_max_time, cfg.exit_anim_max_time );
    EXPECT_EQ( ctx.cfg.max_elements, cfg.max_elements );
    EXPECT_EQ( ctx.command_count, 0 );
    EXPECT_EQ( ctx.text_queue_count, 0 );
    EXPECT_EQ( ctx.pending_nav_mask, ( uint32_t ) 0 );
    EXPECT_FALSE( ctx.pending_confirm );
    EXPECT_FALSE( ctx.pending_cancel );
    EXPECT_EQ( ctx.pending_tab, 0 );
}

UTEST( core, begin_resets_per_frame_latches )
{
    std::vector< uint8_t > memory( 4096 );
    vxui_ctx ctx = {};

    vxui_init( &ctx, vxui_create_arena( ( uint64_t ) memory.size(), memory.data() ), vxui__test_config() );
    ctx.command_count = 3;
    ctx.text_queue_count = 2;
    ctx.pending_nav_mask = 77;
    ctx.pending_confirm = true;
    ctx.pending_cancel = true;
    ctx.pending_tab = 9;

    vxui_begin( &ctx, 0.25f );

    EXPECT_EQ( ctx.frame_index, ( uint64_t ) 1 );
    EXPECT_NEAR( ctx.delta_time, 0.25f, 0.0001f );
    EXPECT_EQ( ctx.command_count, 0 );
    EXPECT_EQ( ctx.text_queue_count, 0 );
    EXPECT_EQ( ctx.pending_nav_mask, ( uint32_t ) 0 );
    EXPECT_FALSE( ctx.pending_confirm );
    EXPECT_FALSE( ctx.pending_cancel );
    EXPECT_EQ( ctx.pending_tab, 0 );
}

UTEST( core, end_returns_empty_draw_list )
{
    std::vector< uint8_t > memory( 4096 );
    vxui_ctx ctx = {};

    vxui_init( &ctx, vxui_create_arena( ( uint64_t ) memory.size(), memory.data() ), vxui__test_config() );
    vxui_begin( &ctx, 0.016f );
    vxui_draw_list list = vxui_end( &ctx );

    EXPECT_EQ( list.commands, ctx.commands );
    EXPECT_EQ( list.length, 0 );
}

UTEST( ids, match_clay_literal )
{
    uint32_t a = vxui_id( "title" );
    uint32_t b = Clay_GetElementId( CLAY_STRING( "title" ) ).id;
    EXPECT_EQ( a, b );
}

UTEST( ids, match_clay_indexed )
{
    uint32_t a = vxui_idi( "item", 7 );
    uint32_t b = Clay_GetElementIdWithIndex( CLAY_STRING( "item" ), 7 ).id;
    EXPECT_EQ( a, b );
}

UTEST( core, allocations_are_aligned_from_misaligned_base )
{
    std::vector< uint8_t > memory( 4097 );
    vxui_ctx ctx = {};

    vxui_init(
        &ctx,
        vxui_create_arena( ( uint64_t ) memory.size() - 1, memory.data() + 1 ),
        vxui__test_config() );

    ASSERT_TRUE( ctx.commands != nullptr );
    ASSERT_TRUE( ctx.text_queue != nullptr );
    EXPECT_EQ( ( uintptr_t ) ctx.commands % alignof( vxui_cmd ), ( uintptr_t ) 0 );
    EXPECT_EQ( ( uintptr_t ) ctx.text_queue % alignof( vxui_draw_cmd_text ), ( uintptr_t ) 0 );
}

UTEST( core, tiny_arena_gracefully_reports_no_capacity )
{
    std::vector< uint8_t > memory( 64 );
    vxui_ctx ctx = {};
    vxui_config cfg = vxui__test_config();
    cfg.max_elements = 16;

    vxui_init( &ctx, vxui_create_arena( ( uint64_t ) memory.size(), memory.data() ), cfg );

    EXPECT_EQ( ctx.commands, nullptr );
    EXPECT_EQ( ctx.command_capacity, 0 );
    EXPECT_EQ( ctx.text_queue, nullptr );
    EXPECT_EQ( ctx.text_queue_capacity, 0 );
}

UTEST( core, begin_end_cycles_do_not_leak_counts )
{
    std::vector< uint8_t > memory( 4096 );
    vxui_ctx ctx = {};

    vxui_init( &ctx, vxui_create_arena( ( uint64_t ) memory.size(), memory.data() ), vxui__test_config() );

    for ( int i = 0; i < 16; ++i ) {
        vxui_begin( &ctx, 0.016f );
        vxui_draw_list list = vxui_end( &ctx );
        EXPECT_EQ( list.length, 0 );
        EXPECT_EQ( ctx.command_count, 0 );
        EXPECT_EQ( ctx.text_queue_count, 0 );
    }
}

UTEST( core, separate_contexts_do_not_cross_contaminate )
{
    std::vector< uint8_t > memory_a( 4096 );
    std::vector< uint8_t > memory_b( 4096 );
    vxui_ctx ctx_a = {};
    vxui_ctx ctx_b = {};

    vxui_init( &ctx_a, vxui_create_arena( ( uint64_t ) memory_a.size(), memory_a.data() ), vxui__test_config() );
    vxui_init( &ctx_b, vxui_create_arena( ( uint64_t ) memory_b.size(), memory_b.data() ), vxui__test_config() );

    vxui_begin( &ctx_a, 0.1f );
    ctx_a.pending_confirm = true;

    EXPECT_FALSE( ctx_b.pending_confirm );
    EXPECT_TRUE( ctx_a.commands != ctx_b.commands );
}
