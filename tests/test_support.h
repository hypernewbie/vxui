#pragma once

#include <chrono>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <string>
#include <system_error>

static inline FILE* vxui_test_fopen( const char* path, const char* mode )
{
    if ( !path || !mode ) {
        return nullptr;
    }
#if defined( _MSC_VER )
    FILE* fp = nullptr;
    return fopen_s( &fp, path, mode ) == 0 ? fp : nullptr;
#else
    return std::fopen( path, mode );
#endif
}

static inline bool vxui_test_write_text_file( const char* path, const char* text )
{
    FILE* fp = vxui_test_fopen( path, "wb" );
    if ( !fp ) {
        return false;
    }

    size_t length = std::strlen( text );
    bool ok = std::fwrite( text, 1, length, fp ) == length;
    std::fclose( fp );
    return ok;
}

static inline std::string vxui_test_temp_path( const char* filename )
{
    std::filesystem::path dir = std::filesystem::temp_directory_path() / "vxui-tests";
    std::error_code ec;
    std::filesystem::create_directories( dir, ec );
    uint64_t stamp = ( uint64_t ) std::chrono::steady_clock::now().time_since_epoch().count();
    std::filesystem::path path = dir / ( std::to_string( stamp ) + "_" + filename );
    return path.make_preferred().string();
}

#ifndef VXUI_TEST_SUPPORT_H_FONTCACHE
#define VXUI_TEST_SUPPORT_H_FONTCACHE

#include <stdbool.h>

typedef struct vxui_test_fontcache_handle vxui_test_fontcache_handle;

vxui_test_fontcache_handle* vxui_test_fontcache_create( bool use_freetype_cpu );
void vxui_test_fontcache_destroy( vxui_test_fontcache_handle* handle );
struct ve_fontcache* vxui_test_fontcache_ptr( vxui_test_fontcache_handle* handle );

int64_t vxui_test_fontcache_load_file( vxui_test_fontcache_handle* handle, const char* path, float size_px );

void vxui_test_fontcache_flush_drawlist( vxui_test_fontcache_handle* handle );

size_t vxui_test_fontcache_drawlist_dcall_count( vxui_test_fontcache_handle* handle );
size_t vxui_test_fontcache_drawlist_vertex_count( vxui_test_fontcache_handle* handle );
size_t vxui_test_fontcache_drawlist_index_count( vxui_test_fontcache_handle* handle );
size_t vxui_test_fontcache_drawlist_texel_count( vxui_test_fontcache_handle* handle );

size_t vxui_test_fontcache_cpu_atlas_page_count( vxui_test_fontcache_handle* handle );
size_t vxui_test_fontcache_cpu_atlas_dcall_count( vxui_test_fontcache_handle* handle );
size_t vxui_test_fontcache_cpu_atlas_texel_count( vxui_test_fontcache_handle* handle );

bool vxui_test_fontcache_draw_text(
    vxui_test_fontcache_handle* handle,
    int64_t font_id,
    const char* text_utf8,
    float posx,
    float posy,
    float scalex,
    float scaley,
    bool shape_cache );

bool vxui_test_fontcache_has_atlas_create_pass( vxui_test_fontcache_handle* handle );
bool vxui_test_fontcache_has_atlas_upload_pass( vxui_test_fontcache_handle* handle );
bool vxui_test_fontcache_has_target_cpu_cached_pass( vxui_test_fontcache_handle* handle );

#endif
