#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
    #define NOMINMAX
#endif
#ifdef _WIN32
    #include <windows.h>
#endif

#include "utest.h"
#include "../vxui.h"
#define VXUI_IMPL
#include "../vxui_impl.h"
#define VXUI_HUD_IMPL
#include "../vxui_hud.h"
#include <climits>
#include <cstdlib>
#ifdef _MSC_VER
#include <crtdbg.h>
#endif

static const char* s_executable = nullptr;
static uint8_t s_clay_mem[16 * 1024 * 1024];

UTEST(hud_tile, rectangular_grid_keeps_every_complete_tile) {
    for ( int i = 0; i < 128; i++ )
    {
        glm::vec4 uv = vxui_hud_tile( 1024, 128, 32, 32, i );
        ASSERT_EQ( uv.x, (float) ( i % 32 ) / 32.0f );
        ASSERT_EQ( uv.y, (float) ( i / 32 ) / 4.0f );
        ASSERT_EQ( uv.z, (float) ( i % 32 + 1 ) / 32.0f );
        ASSERT_EQ( uv.w, (float) ( i / 32 + 1 ) / 4.0f );
    }
}

UTEST(hud_tile, row_boundary_moves_back_to_the_first_column) {
    glm::vec4 last = vxui_hud_tile( 1024, 128, 32, 32, 31 );
    glm::vec4 next = vxui_hud_tile( 1024, 128, 32, 32, 32 );
    ASSERT_EQ( last.x, 31.0f / 32.0f );
    ASSERT_EQ( last.z, 1.0f );
    ASSERT_EQ( next.x, 0.0f );
    ASSERT_EQ( next.y, 0.25f );
    ASSERT_EQ( next.z, 1.0f / 32.0f );
}

UTEST(hud_tile, single_tile_uses_the_whole_texture) {
    glm::vec4 uv = vxui_hud_tile( 37, 19, 37, 19, 0 );
    ASSERT_EQ( uv.x, 0.0f );
    ASSERT_EQ( uv.y, 0.0f );
    ASSERT_EQ( uv.z, 1.0f );
    ASSERT_EQ( uv.w, 1.0f );
}

UTEST(hud_tile, texture_remainders_do_not_change_complete_tile_uvs) {
    glm::vec4 uv = vxui_hud_tile( 100, 70, 32, 16, 11 );
    ASSERT_EQ( uv.x, 64.0f / 100.0f );
    ASSERT_EQ( uv.y, 48.0f / 70.0f );
    ASSERT_EQ( uv.z, 96.0f / 100.0f );
    ASSERT_EQ( uv.w, 64.0f / 70.0f );
}

UTEST(hud_tile, one_pixel_tiles_reach_the_last_row_and_column) {
    glm::vec4 uv = vxui_hud_tile( 4, 3, 1, 1, 11 );
    ASSERT_EQ( uv.x, 0.75f );
    ASSERT_EQ( uv.y, 2.0f / 3.0f );
    ASSERT_EQ( uv.z, 1.0f );
    ASSERT_EQ( uv.w, 1.0f );
}

UTEST(hud_tile, square_wrapper_matches_the_rectangular_calculation) {
    for ( int i = 0; i < 64; i++ )
    {
        glm::vec4 square = vxui_hud_tile1d( 1024, 128, i );
        glm::vec4 rect = vxui_hud_tile( 1024, 1024, 128, 128, i );
        for ( int j = 0; j < 4; j++ ) ASSERT_EQ( square[j], rect[j] );
    }
}

UTEST(hud_tile, valid_index_does_not_require_a_signed_grid_area_product) {
    glm::vec4 uv = vxui_hud_tile( 65536, 65536, 1, 1, INT_MAX );
    ASSERT_EQ( uv.x, 65535.0f / 65536.0f );
    ASSERT_EQ( uv.y, 32767.0f / 65536.0f );
    ASSERT_EQ( uv.z, 1.0f );
    ASSERT_EQ( uv.w, 0.5f );
    glm::vec4 square = vxui_hud_tile1d( 65536, 1, INT_MAX );
    for ( int j = 0; j < 4; j++ ) ASSERT_EQ( square[j], uv[j] );
}

UTEST(hud_tile, computed_uvs_reach_the_hud_record_unchanged) {
    vxui_ctx ctx = {};
    vxui_init( &ctx, 960, 540, s_clay_mem, sizeof( s_clay_mem ) );
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    glm::vec4 uv = vxui_hud_tile( 100, 70, 32, 16, 11 );
    vxui_hud_image( &hud, "tile", 42, 32, 16, uv );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( dl.count, 1 );
    vxui_render_data out = {};
    ASSERT_TRUE( vxui_hud_resolve( &hud, &dl.cmds[0], &out ) );
    for ( int j = 0; j < 4; j++ ) ASSERT_EQ( out.uv[j], uv[j] );
}

#ifndef NDEBUG
static bool invalid_asserts( const char* mode, int index, const char* message )
{
    char command[4096];
#ifdef _MSC_VER
    int len = snprintf( command, sizeof( command ), "\"\"%s\" --%s %d 2>&1\"", s_executable, mode, index );
#else
    int len = snprintf( command, sizeof( command ), "ulimit -c 0; \"%s\" --%s %d 2>&1", s_executable, mode, index );
#endif
    assert( len > 0 && len < (int) sizeof( command ) );
#ifdef _MSC_VER
    FILE* pipe = _popen( command, "r" );
#else
    FILE* pipe = popen( command, "r" );
#endif
    if ( !pipe ) return false;
    bool seen = false;
    char line[512];
    while ( fgets( line, sizeof( line ), pipe ) )
        if ( strstr( line, message ) ) seen = true;
#ifdef _MSC_VER
    int status = _pclose( pipe );
#else
    int status = pclose( pipe );
#endif
    return status != 0 && seen;
}

UTEST(hud_tile, zero_or_negative_dimensions_assert_before_division) {
    for ( int i = 0; i < 8; i++ ) ASSERT_TRUE( invalid_asserts( "dimensions", i, "invalid tile dimensions" ) );
}

UTEST(hud_tile, oversized_tiles_assert_instead_of_fabricating_a_grid) {
    for ( int i = 0; i < 2; i++ ) ASSERT_TRUE( invalid_asserts( "oversize", i, "tile larger than texture" ) );
}

UTEST(hud_tile, negative_and_out_of_range_indices_assert) {
    for ( int i = 0; i < 3; i++ ) ASSERT_TRUE( invalid_asserts( "index", i, "tile index out of range" ) );
}

UTEST(hud_tile, square_wrapper_uses_the_same_checked_preconditions) {
    ASSERT_TRUE( invalid_asserts( "tile1d", 0, "tile larger than texture" ) );
    ASSERT_TRUE( invalid_asserts( "tile1d", 1, "tile index out of range" ) );
}
#endif

UTEST_STATE();

int main( int argc, const char* const argv[] )
{
#ifdef _WIN32
    SetErrorMode( SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX );
#endif
#ifdef _MSC_VER
    _set_error_mode( _OUT_TO_STDERR );
    _set_abort_behavior( 0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT );
    _CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_FILE );
    _CrtSetReportFile( _CRT_ASSERT, _CRTDBG_FILE_STDERR );
#endif
    s_executable = argv[0];
#ifndef NDEBUG
    if ( argc == 3 )
    {
        int i = atoi( argv[2] );
        if ( strcmp( argv[1], "--dimensions" ) == 0 )
        {
            int dimensions[8][4] = {
                { 0, 128, 32, 32 }, { 128, 0, 32, 32 }, { 128, 128, 0, 32 }, { 128, 128, 32, 0 },
                { -1, 128, 32, 32 }, { 128, -1, 32, 32 }, { 128, 128, -1, 32 }, { 128, 128, 32, -1 }
            };
            assert( i >= 0 && i < 8 );
            vxui_hud_tile( dimensions[i][0], dimensions[i][1], dimensions[i][2], dimensions[i][3], 0 );
            return 0;
        }
        if ( strcmp( argv[1], "--oversize" ) == 0 )
        {
            assert( i >= 0 && i < 2 );
            vxui_hud_tile( 128, 128, i == 0 ? 256 : 32, i == 1 ? 256 : 32, 0 );
            return 0;
        }
        if ( strcmp( argv[1], "--index" ) == 0 )
        {
            int indices[] = { -1, 16, INT_MAX };
            assert( i >= 0 && i < 3 );
            vxui_hud_tile( 128, 128, 32, 32, indices[i] );
            return 0;
        }
        if ( strcmp( argv[1], "--tile1d" ) == 0 )
        {
            assert( i >= 0 && i < 2 );
            vxui_hud_tile1d( 128, i == 0 ? 256 : 32, i == 1 ? 16 : 0 );
            return 0;
        }
    }
#endif
    return utest_main( argc, argv );
}
