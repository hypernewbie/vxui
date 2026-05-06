#include "utest.h"

#include <cstdio>
#include <cstdint>
#include <filesystem>
#include <vector>

#include <hb.h>

#define VE_FONTCACHE_IMPL
#include "../vefc/ve_fontcache.h"
#include "../vefc/ve_fontcache_backend_test.h"

static std::vector< uint8_t > roboto_bytes()
{
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
}

UTEST(vefc_backend, cpu_suites_pass) {
    ve_fontcache cache = {};
    ve_fontcache_init( &cache, false );

    std::vector< uint8_t > bytes = roboto_bytes();
    std::vector< uint8_t > scratch;
    ve_font_id font = ve_fontcache_load( &cache, bytes.data(), bytes.size(), 24.0f );
    ASSERT_GE( font, 0 );

    ve_fontcache_backend_test_result result = ve_fontcache_backend_test_run( &cache, font );

    for ( const std::string& failure : result.failures )
        printf( "FAIL: %s\n", failure.c_str() );

    EXPECT_EQ( 0, result.failed );
    EXPECT_TRUE( result.passed > 0 );

    ve_fontcache_shutdown( &cache );
}

UTEST_MAIN();
