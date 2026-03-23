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
