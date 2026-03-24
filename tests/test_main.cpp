#include <string>
#include <vector>

#include "../third_party/utest.h"

#define CLAY_IMPLEMENTATION
#include "../clay/clay.h"

#define VE_FONTCACHE_IMPL
#define VXUI_IMPL
#include "../vxui.h"

#include "test_support.h"

struct vxui_test_fontcache_handle
{
    ve_fontcache cache;
    std::vector< std::vector< uint8_t > > font_buffers;
};

vxui_test_fontcache_handle* vxui_test_fontcache_create( bool use_freetype_cpu )
{
    ( void ) use_freetype_cpu;
    auto* handle = new vxui_test_fontcache_handle();
    ve_fontcache_init( &handle->cache, true );
    return handle;
}

void vxui_test_fontcache_destroy( vxui_test_fontcache_handle* handle )
{
    if ( !handle ) return;
    ve_fontcache_shutdown( &handle->cache );
    delete handle;
}

struct ve_fontcache* vxui_test_fontcache_ptr( vxui_test_fontcache_handle* handle )
{
    return &handle->cache;
}

int64_t vxui_test_fontcache_load_file( vxui_test_fontcache_handle* handle, const char* path, float size_px )
{
    if ( !handle || !path ) return -1;
    FILE* f = std::fopen( path, "rb" );
    if ( !f ) return -1;
    std::fseek( f, 0, SEEK_END );
    size_t sz = ( size_t ) std::ftell( f );
    std::fseek( f, 0, SEEK_SET );
    handle->font_buffers.emplace_back();
    handle->font_buffers.back().resize( sz );
    std::fread( handle->font_buffers.back().data(), 1, sz, f );
    std::fclose( f );
    return ve_fontcache_load( &handle->cache, handle->font_buffers.back().data(), sz, size_px );
}

void vxui_test_fontcache_flush_drawlist( vxui_test_fontcache_handle* handle )
{
    if ( !handle ) return;
    ve_fontcache_flush_drawlist( &handle->cache );
}

size_t vxui_test_fontcache_drawlist_dcall_count( vxui_test_fontcache_handle* handle )
{
    if ( !handle ) return 0;
    ve_fontcache_drawlist* dl = ve_fontcache_get_drawlist( &handle->cache );
    return dl ? dl->dcalls.size() : 0;
}

size_t vxui_test_fontcache_drawlist_vertex_count( vxui_test_fontcache_handle* handle )
{
    if ( !handle ) return 0;
    ve_fontcache_drawlist* dl = ve_fontcache_get_drawlist( &handle->cache );
    return dl ? dl->vertices.size() : 0;
}

size_t vxui_test_fontcache_drawlist_index_count( vxui_test_fontcache_handle* handle )
{
    if ( !handle ) return 0;
    ve_fontcache_drawlist* dl = ve_fontcache_get_drawlist( &handle->cache );
    return dl ? dl->indices.size() : 0;
}

size_t vxui_test_fontcache_drawlist_texel_count( vxui_test_fontcache_handle* handle )
{
    if ( !handle ) return 0;
    ve_fontcache_drawlist* dl = ve_fontcache_get_drawlist( &handle->cache );
    return dl ? dl->texels.size() : 0;
}

size_t vxui_test_fontcache_cpu_atlas_page_count( vxui_test_fontcache_handle* handle )
{
    if ( !handle ) return 0;
    return handle->cache.atlasCPU.pages.size();
}

size_t vxui_test_fontcache_cpu_atlas_dcall_count( vxui_test_fontcache_handle* handle )
{
    if ( !handle ) return 0;
    return handle->cache.atlasCPU.drawlist.dcalls.size();
}

size_t vxui_test_fontcache_cpu_atlas_texel_count( vxui_test_fontcache_handle* handle )
{
    if ( !handle ) return 0;
    return handle->cache.atlasCPU.drawlist.texels.size();
}

bool vxui_test_fontcache_draw_text(
    vxui_test_fontcache_handle* handle,
    int64_t font_id,
    const char* text_utf8,
    float posx,
    float posy,
    float scalex,
    float scaley,
    bool shape_cache )
{
    if ( !handle || !text_utf8 ) return false;
    return ve_fontcache_draw_text(
        &handle->cache,
        ( ve_font_id ) font_id,
        std::u8string( ( const char8_t* ) text_utf8 ),
        posx, posy, scalex, scaley, shape_cache );
}

static bool vxui_test_has_pass( vxui_test_fontcache_handle* handle, int pass )
{
    if ( !handle ) return false;
    ve_fontcache_drawlist* dl = ve_fontcache_get_drawlist( &handle->cache );
    if ( !dl ) return false;
    for ( const ve_fontcache_draw& d : dl->dcalls ) {
        if ( d.pass == pass ) return true;
    }
    return false;
}

static void vxui_test_fontcache_unsafe_flush_drawlist( vxui_test_fontcache_handle* handle )
{
    if ( !handle ) return;
    ve_fontcache_flush_drawlist( &handle->cache );
}

bool vxui_test_fontcache_has_atlas_create_pass( vxui_test_fontcache_handle* handle )
{
    return vxui_test_has_pass( handle, VE_FONTCACHE_FRAMEBUFFER_PASS_ATLAS_PAGE_TEXTURE_CREATE );
}

bool vxui_test_fontcache_has_atlas_upload_pass( vxui_test_fontcache_handle* handle )
{
    return vxui_test_has_pass( handle, VE_FONTCACHE_FRAMEBUFFER_PASS_ATLAS_UPLOAD );
}

bool vxui_test_fontcache_has_target_cpu_cached_pass( vxui_test_fontcache_handle* handle )
{
    return vxui_test_has_pass( handle, VE_FONTCACHE_FRAMEBUFFER_PASS_TARGET_CPU_CACHED );
}

UTEST( smoke, build_links )
{
    EXPECT_TRUE( true );
}

UTEST( smoke, fontcache_with_vxui_ctx_and_text )
{
    vxui_test_fontcache_handle* fc = vxui_test_fontcache_create( true );
    ASSERT_TRUE( fc != nullptr );
    int64_t id = vxui_test_fontcache_load_file( fc, VXUI_SOURCE_DIR "/vefc/demo/fonts/OpenSans-Regular.ttf", 24.0f );
    ASSERT_TRUE( id >= 0 );

    std::vector< uint8_t > memory( 512 * 1024 );
    vxui_ctx ctx = {};
    vxui_init( &ctx, vxui_create_arena( ( uint64_t ) memory.size(), memory.data() ), ( vxui_config ) {
        .screen_width = 1280,
        .screen_height = 720,
        .exit_anim_max_time = 300,
        .focus_ring.color = { 10, 20, 30, 255 },
        .focus_ring.border_width = 3.0f,
        .focus_ring.corner_radius = 6.0f,
        .max_elements = 8,
        .max_anim_states = 16,
        .max_sequences = 4,
        .max_seq_steps = 32,
    } );

    vxui_begin( &ctx, 0.016f );
    {
        CLAY_TEXT(
            CLAY_STRING( "Hello World" ),
            CLAY_TEXT_CONFIG( {
                .textColor = { 255, 255, 255, 255 },
                .fontId = ( uint16_t ) id,
                .fontSize = 24,
            } ) );
    }
    vxui_draw_list list = vxui_end( &ctx );
    ( void ) list;

    vxui_test_fontcache_destroy( fc );
}

UTEST_MAIN();
