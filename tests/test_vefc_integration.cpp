#include <cstdint>

#include "../third_party/utest.h"
#include "../vxui.h"
#include "test_support.h"

static constexpr const char* kOpenSansPath = VXUI_SOURCE_DIR "/vefc/demo/fonts/OpenSans-Regular.ttf";

static vxui_config vxui__test_config( void )
{
    vxui_config cfg = {};
    cfg.screen_width = 1280;
    cfg.screen_height = 720;
    cfg.exit_anim_max_time = 300;
    cfg.focus_ring.color = ( vxui_color ) { 10, 20, 30, 255 };
    cfg.focus_ring.border_width = 3.0f;
    cfg.focus_ring.corner_radius = 6.0f;
    cfg.max_elements = 8;
    cfg.max_anim_states = 16;
    cfg.max_sequences = 4;
    cfg.max_seq_steps = 32;
    return cfg;
}

UTEST( vefc_integration, layout_measurement_does_not_mutate_vefc_drawlist )
{
    vxui_test_fontcache_handle* fc = vxui_test_fontcache_create( true );
    ASSERT_TRUE( fc != nullptr );

    int64_t font_id = vxui_test_fontcache_load_file( fc, kOpenSansPath, 24.0f );
    ASSERT_TRUE( font_id >= 0 );

    std::vector< uint8_t > memory( 512 * 1024 );
    vxui_ctx ctx = {};
    vxui_init( &ctx, vxui_create_arena( ( uint64_t ) memory.size(), memory.data() ), vxui__test_config() );
    vxui_set_fontcache( &ctx, vxui_test_fontcache_ptr( fc ) );

    vxui_begin( &ctx, 0.016f );
    {
        CLAY_TEXT(
            CLAY_STRING( "Hello World" ),
            CLAY_TEXT_CONFIG( {
                .textColor = { 255, 255, 255, 255 },
                .fontId = ( uint16_t ) font_id,
                .fontSize = 24,
            } ) );
    }
    vxui_draw_list list = vxui_end( &ctx );
    ( void ) list;

    vxui_test_fontcache_flush_drawlist( fc );
    EXPECT_EQ( vxui_test_fontcache_drawlist_dcall_count( fc ), ( size_t ) 0 );
    EXPECT_EQ( vxui_test_fontcache_drawlist_vertex_count( fc ), ( size_t ) 0 );
    EXPECT_EQ( vxui_test_fontcache_drawlist_index_count( fc ), ( size_t ) 0 );
    EXPECT_EQ( vxui_test_fontcache_drawlist_texel_count( fc ), ( size_t ) 0 );

    vxui_test_fontcache_destroy( fc );
}

UTEST( vefc_integration, layout_measurement_does_not_create_cpu_atlas_pages )
{
    vxui_test_fontcache_handle* fc = vxui_test_fontcache_create( true );
    ASSERT_TRUE( fc != nullptr );

    int64_t font_id = vxui_test_fontcache_load_file( fc, kOpenSansPath, 24.0f );
    ASSERT_TRUE( font_id >= 0 );

    std::vector< uint8_t > memory( 512 * 1024 );
    vxui_ctx ctx = {};
    vxui_init( &ctx, vxui_create_arena( ( uint64_t ) memory.size(), memory.data() ), vxui__test_config() );
    vxui_set_fontcache( &ctx, vxui_test_fontcache_ptr( fc ) );

    vxui_begin( &ctx, 0.016f );
    {
        CLAY_TEXT(
            CLAY_STRING( "Hello World" ),
            CLAY_TEXT_CONFIG( {
                .textColor = { 255, 255, 255, 255 },
                .fontId = ( uint16_t ) font_id,
                .fontSize = 24,
            } ) );
    }
    vxui_draw_list list = vxui_end( &ctx );
    ( void ) list;

    vxui_test_fontcache_flush_drawlist( fc );
    EXPECT_EQ( vxui_test_fontcache_cpu_atlas_page_count( fc ), ( size_t ) 0 );
    EXPECT_EQ( vxui_test_fontcache_cpu_atlas_dcall_count( fc ), ( size_t ) 0 );
    EXPECT_EQ( vxui_test_fontcache_cpu_atlas_texel_count( fc ), ( size_t ) 0 );

    vxui_test_fontcache_destroy( fc );
}

UTEST( vefc_integration, first_real_draw_after_layout_still_emits_cpu_atlas_setup )
{
    vxui_test_fontcache_handle* fc = vxui_test_fontcache_create( true );
    ASSERT_TRUE( fc != nullptr );

    int64_t font_id = vxui_test_fontcache_load_file( fc, kOpenSansPath, 24.0f );
    ASSERT_TRUE( font_id >= 0 );

    std::vector< uint8_t > memory( 512 * 1024 );
    vxui_ctx ctx = {};
    vxui_init( &ctx, vxui_create_arena( ( uint64_t ) memory.size(), memory.data() ), vxui__test_config() );
    vxui_set_fontcache( &ctx, vxui_test_fontcache_ptr( fc ) );

    vxui_begin( &ctx, 0.016f );
    {
        CLAY_TEXT(
            CLAY_STRING( "Hello World" ),
            CLAY_TEXT_CONFIG( {
                .textColor = { 255, 255, 255, 255 },
                .fontId = ( uint16_t ) font_id,
                .fontSize = 24,
            } ) );
    }
    vxui_draw_list list = vxui_end( &ctx );
    ( void ) list;

    vxui_test_fontcache_flush_drawlist( fc );

    bool ok = vxui_test_fontcache_draw_text( fc, font_id, "Hello World", 0.0f, 0.0f, 1.0f / 1280.0f, 1.0f / 720.0f, true );
    EXPECT_TRUE( ok );

    EXPECT_TRUE( vxui_test_fontcache_has_atlas_create_pass( fc ) );
    EXPECT_TRUE( vxui_test_fontcache_has_atlas_upload_pass( fc ) );
    EXPECT_TRUE( vxui_test_fontcache_has_target_cpu_cached_pass( fc ) );

    vxui_test_fontcache_destroy( fc );
}
