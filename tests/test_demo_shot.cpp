#include <cstring>
#include <cstdlib>
#include <filesystem>
#include <process.h>
#include <string>
#include <vector>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <objbase.h>
#include <wincodec.h>

#include "../demo/internal/shot.h"
#include "../third_party/utest.h"
#include "test_support.h"

static bool vxui_demo_shot_png_has_variation( const std::filesystem::path& path )
{
    HRESULT init_hr = CoInitializeEx( nullptr, COINIT_MULTITHREADED );
    const bool should_uninitialize = SUCCEEDED( init_hr );
    if ( init_hr == RPC_E_CHANGED_MODE ) {
        init_hr = S_OK;
    }
    if ( FAILED( init_hr ) ) {
        return false;
    }

    IWICImagingFactory* factory = nullptr;
    IWICBitmapDecoder* decoder = nullptr;
    IWICBitmapFrameDecode* frame = nullptr;
    IWICFormatConverter* converter = nullptr;
    bool varied = false;

    const std::wstring wide_path = path.wstring();
    HRESULT hr = CoCreateInstance( CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS( &factory ) );
    if ( FAILED( hr ) ) {
        goto cleanup;
    }
    hr = factory->CreateDecoderFromFilename( wide_path.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &decoder );
    if ( FAILED( hr ) ) {
        goto cleanup;
    }
    hr = decoder->GetFrame( 0, &frame );
    if ( FAILED( hr ) ) {
        goto cleanup;
    }
    hr = factory->CreateFormatConverter( &converter );
    if ( FAILED( hr ) ) {
        goto cleanup;
    }
    hr = converter->Initialize( frame, GUID_WICPixelFormat32bppBGRA, WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom );
    if ( FAILED( hr ) ) {
        goto cleanup;
    }

    UINT width = 0;
    UINT height = 0;
    hr = converter->GetSize( &width, &height );
    if ( FAILED( hr ) || width == 0 || height == 0 ) {
        goto cleanup;
    }

    std::vector< uint8_t > pixels( ( size_t ) width * ( size_t ) height * 4u );
    hr = converter->CopyPixels( nullptr, width * 4u, ( UINT ) pixels.size(), pixels.data() );
    if ( FAILED( hr ) ) {
        goto cleanup;
    }

    const uint8_t* first = pixels.data();
    for ( size_t i = 4; i < pixels.size(); i += 4 ) {
        if ( std::memcmp( first, pixels.data() + i, 4 ) != 0 ) {
            varied = true;
            break;
        }
    }

cleanup:
    if ( converter ) converter->Release();
    if ( frame ) frame->Release();
    if ( decoder ) decoder->Release();
    if ( factory ) factory->Release();
    if ( should_uninitialize ) {
        CoUninitialize();
    }
    return varied;
}

UTEST( demo_shot_cli, parses_required_shot_flags )
{
    const char* argv[] = {
        "vxui_demo",
        "--shot",
        "--screen=main_menu",
        "--width=1280",
        "--height=720",
        "--out=main_menu.png",
    };

    bool backend_test_mode = false;
    vxui_demo_shot_request shot = {};
    char error[ 256 ] = {};

    ASSERT_TRUE( vxui_demo_parse_cli( ( int ) ( sizeof( argv ) / sizeof( argv[ 0 ] ) ), const_cast< char** >( argv ), &backend_test_mode, &shot, error, sizeof( error ) ) );
    EXPECT_FALSE( backend_test_mode );
    EXPECT_TRUE( shot.enabled );
    EXPECT_STREQ( shot.screen_name, "main_menu" );
    EXPECT_EQ( shot.width, 1280 );
    EXPECT_EQ( shot.height, 720 );
    EXPECT_STREQ( shot.out_path.c_str(), "main_menu.png" );
}

UTEST( demo_shot_cli, invalid_screen_name_fails )
{
    const char* argv[] = {
        "vxui_demo",
        "--shot",
        "--screen=bogus",
        "--width=1280",
        "--height=720",
        "--out=bad.png",
    };

    bool backend_test_mode = false;
    vxui_demo_shot_request shot = {};
    char error[ 256 ] = {};

    EXPECT_FALSE( vxui_demo_parse_cli( ( int ) ( sizeof( argv ) / sizeof( argv[ 0 ] ) ), const_cast< char** >( argv ), &backend_test_mode, &shot, error, sizeof( error ) ) );
    EXPECT_TRUE( std::string( error ).find( "--screen" ) != std::string::npos );
}

UTEST( demo_shot_cli, missing_output_fails )
{
    const char* argv[] = {
        "vxui_demo",
        "--shot",
        "--screen=main_menu",
        "--width=1280",
        "--height=720",
    };

    bool backend_test_mode = false;
    vxui_demo_shot_request shot = {};
    char error[ 256 ] = {};

    EXPECT_FALSE( vxui_demo_parse_cli( ( int ) ( sizeof( argv ) / sizeof( argv[ 0 ] ) ), const_cast< char** >( argv ), &backend_test_mode, &shot, error, sizeof( error ) ) );
    EXPECT_TRUE( std::string( error ).find( "--out" ) != std::string::npos );
}

UTEST( demo_shot_cli, no_scanline_changes_shot_config )
{
    const char* argv[] = {
        "vxui_demo",
        "--shot",
        "--screen=sortie",
        "--width=1100",
        "--height=648",
        "--out=sortie.png",
        "--locale=ja",
        "--prompts=gamepad",
        "--focus=start",
        "--compact",
        "--no-scanline",
    };

    bool backend_test_mode = false;
    vxui_demo_shot_request shot = {};
    char error[ 256 ] = {};

    ASSERT_TRUE( vxui_demo_parse_cli( ( int ) ( sizeof( argv ) / sizeof( argv[ 0 ] ) ), const_cast< char** >( argv ), &backend_test_mode, &shot, error, sizeof( error ) ) );
    EXPECT_TRUE( shot.disable_scanline );
    EXPECT_TRUE( shot.compact_override );
    EXPECT_EQ( shot.locale_index, 1 );
    EXPECT_EQ( shot.prompt_table_index, 1 );
    EXPECT_STREQ( shot.focus_id.c_str(), "start" );
}

UTEST( demo_shot_cli, layout_dump_flags_parse_without_shot_mode )
{
    const char* argv[] = {
        "vxui_demo",
        "--dump-layout=layout.json",
        "--dump-layout-stdout",
    };

    bool backend_test_mode = false;
    vxui_demo_shot_request shot = {};
    char error[ 256 ] = {};

    ASSERT_TRUE( vxui_demo_parse_cli( ( int ) ( sizeof( argv ) / sizeof( argv[ 0 ] ) ), const_cast< char** >( argv ), &backend_test_mode, &shot, error, sizeof( error ) ) );
    EXPECT_FALSE( shot.enabled );
    EXPECT_STREQ( shot.dump_layout_path.c_str(), "layout.json" );
    EXPECT_TRUE( shot.dump_layout_stdout );
}

UTEST( demo_shot_cli, supported_shot_screens_cover_demo_targets )
{
    EXPECT_TRUE( vxui_demo_shot_screen_supported( "boot" ) );
    EXPECT_TRUE( vxui_demo_shot_screen_supported( "title" ) );
    EXPECT_TRUE( vxui_demo_shot_screen_supported( "main_menu" ) );
    EXPECT_TRUE( vxui_demo_shot_screen_supported( "sortie" ) );
    EXPECT_TRUE( vxui_demo_shot_screen_supported( "loadout" ) );
    EXPECT_TRUE( vxui_demo_shot_screen_supported( "archives" ) );
    EXPECT_TRUE( vxui_demo_shot_screen_supported( "settings" ) );
    EXPECT_TRUE( vxui_demo_shot_screen_supported( "records" ) );
    EXPECT_TRUE( vxui_demo_shot_screen_supported( "credits" ) );
    EXPECT_TRUE( vxui_demo_shot_screen_supported( "launch_stub" ) );
    EXPECT_TRUE( vxui_demo_shot_screen_supported( "results_stub" ) );
}

#ifdef VXUI_DEMO_EXE_PATH
UTEST( demo_shot_cli, one_shot_mode_writes_non_blank_png )
{
    const std::filesystem::path demo_path = std::filesystem::path( VXUI_DEMO_EXE_PATH );
    if ( !std::filesystem::exists( demo_path ) ) {
        UTEST_SKIP( "vxui_demo executable not available" );
    }

    const std::filesystem::path out_path = std::filesystem::path( vxui_test_temp_path( "shot-main-menu.png" ) );
    std::error_code ec;
    std::filesystem::remove( out_path, ec );

    const std::string exe = demo_path.string();
    const std::string out = out_path.string();
    const std::string out_arg = "--out=" + out;
    const int exit_code = _spawnl(
        _P_WAIT,
        exe.c_str(),
        exe.c_str(),
        "--shot",
        "--screen=main_menu",
        "--width=640",
        "--height=360",
        out_arg.c_str(),
        "--no-scanline",
        nullptr );
    EXPECT_EQ( exit_code, 0 );
    ASSERT_TRUE( std::filesystem::exists( out_path ) );
    EXPECT_TRUE( std::filesystem::file_size( out_path ) > 0 );
    EXPECT_TRUE( vxui_demo_shot_png_has_variation( out_path ) );

    std::filesystem::remove( out_path, ec );
}
#endif
