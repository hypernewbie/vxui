#pragma once

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

struct vxui_demo_shot_request
{
    bool enabled = false;
    const char* screen_name = nullptr;
    int width = 0;
    int height = 0;
    std::string out_path;
    int locale_index = 0;
    int prompt_table_index = 0;
    std::string focus_id;
    bool compact_override = false;
    bool disable_scanline = false;
    std::string dump_layout_path;
    bool dump_layout_stdout = false;
};

inline bool vxui_demo_shot_screen_supported( const char* screen_name )
{
    if ( !screen_name || screen_name[ 0 ] == '\0' ) {
        return false;
    }
    return std::strcmp( screen_name, "boot" ) == 0
        || std::strcmp( screen_name, "title" ) == 0
        || std::strcmp( screen_name, "main_menu" ) == 0
        || std::strcmp( screen_name, "sortie" ) == 0
        || std::strcmp( screen_name, "loadout" ) == 0
        || std::strcmp( screen_name, "archives" ) == 0
        || std::strcmp( screen_name, "settings" ) == 0
        || std::strcmp( screen_name, "records" ) == 0
        || std::strcmp( screen_name, "credits" ) == 0
        || std::strcmp( screen_name, "launch_stub" ) == 0
        || std::strcmp( screen_name, "results_stub" ) == 0;
}

inline int vxui_demo_shot_locale_index_from_code( const char* locale_code )
{
    if ( !locale_code || locale_code[ 0 ] == '\0' ) {
        return -1;
    }
    if ( std::strcmp( locale_code, "en" ) == 0 ) return 0;
    if ( std::strcmp( locale_code, "ja" ) == 0 ) return 1;
    if ( std::strcmp( locale_code, "ar" ) == 0 ) return 2;
    return -1;
}

inline int vxui_demo_shot_prompt_table_index_from_name( const char* prompt_name )
{
    if ( !prompt_name || prompt_name[ 0 ] == '\0' ) {
        return -1;
    }
    if ( std::strcmp( prompt_name, "keyboard" ) == 0 ) return 0;
    if ( std::strcmp( prompt_name, "gamepad" ) == 0 ) return 1;
    return -1;
}

inline bool vxui_demo_shot_parse_positive_int( const char* text, int* out_value )
{
    if ( !text || !out_value || text[ 0 ] == '\0' ) {
        return false;
    }
    char* end = nullptr;
    long value = std::strtol( text, &end, 10 );
    if ( !end || *end != '\0' || value <= 0 || value > 32767 ) {
        return false;
    }
    *out_value = ( int ) value;
    return true;
}

inline void vxui_demo_shot_set_error( char* error, size_t error_size, const char* message )
{
    if ( !error || error_size == 0 ) {
        return;
    }
    std::snprintf( error, error_size, "%s", message ? message : "unknown shot parse error" );
}

inline bool vxui_demo_parse_cli(
    int argc,
    char** argv,
    bool* out_backend_test_mode,
    vxui_demo_shot_request* out_shot_request,
    char* error,
    size_t error_size )
{
    if ( out_backend_test_mode ) {
        *out_backend_test_mode = false;
    }
    if ( out_shot_request ) {
        *out_shot_request = {};
    }
    vxui_demo_shot_request request = {};
    bool backend_test_mode = false;
    bool saw_shot_only_flag = false;

    for ( int i = 1; i < argc; ++i ) {
        const char* arg = argv[ i ];
        if ( !arg ) {
            continue;
        }
        if ( std::strcmp( arg, "--vefc-backend-test" ) == 0 ) {
            backend_test_mode = true;
            continue;
        }
        if ( std::strcmp( arg, "--shot" ) == 0 ) {
            request.enabled = true;
            saw_shot_only_flag = true;
            continue;
        }
        if ( std::strcmp( arg, "--compact" ) == 0 ) {
            request.compact_override = true;
            continue;
        }
        if ( std::strcmp( arg, "--no-scanline" ) == 0 ) {
            request.disable_scanline = true;
            continue;
        }
        if ( std::strcmp( arg, "--dump-layout-stdout" ) == 0 ) {
            request.dump_layout_stdout = true;
            continue;
        }
        if ( std::strncmp( arg, "--screen=", 9 ) == 0 ) {
            request.screen_name = arg + 9;
            continue;
        }
        if ( std::strncmp( arg, "--width=", 8 ) == 0 ) {
            if ( !vxui_demo_shot_parse_positive_int( arg + 8, &request.width ) ) {
                vxui_demo_shot_set_error( error, error_size, "invalid --width value" );
                return false;
            }
            continue;
        }
        if ( std::strncmp( arg, "--height=", 9 ) == 0 ) {
            if ( !vxui_demo_shot_parse_positive_int( arg + 9, &request.height ) ) {
                vxui_demo_shot_set_error( error, error_size, "invalid --height value" );
                return false;
            }
            continue;
        }
        if ( std::strncmp( arg, "--out=", 6 ) == 0 ) {
            request.out_path = arg + 6;
            continue;
        }
        if ( std::strncmp( arg, "--locale=", 9 ) == 0 ) {
            request.locale_index = vxui_demo_shot_locale_index_from_code( arg + 9 );
            if ( request.locale_index < 0 ) {
                vxui_demo_shot_set_error( error, error_size, "invalid --locale value" );
                return false;
            }
            continue;
        }
        if ( std::strncmp( arg, "--prompts=", 10 ) == 0 ) {
            request.prompt_table_index = vxui_demo_shot_prompt_table_index_from_name( arg + 10 );
            if ( request.prompt_table_index < 0 ) {
                vxui_demo_shot_set_error( error, error_size, "invalid --prompts value" );
                return false;
            }
            continue;
        }
        if ( std::strncmp( arg, "--focus=", 8 ) == 0 ) {
            request.focus_id = arg + 8;
            if ( request.focus_id.empty() ) {
                vxui_demo_shot_set_error( error, error_size, "invalid --focus value" );
                return false;
            }
            continue;
        }
        if ( std::strncmp( arg, "--dump-layout=", 14 ) == 0 ) {
            request.dump_layout_path = arg + 14;
            if ( request.dump_layout_path.empty() ) {
                vxui_demo_shot_set_error( error, error_size, "invalid --dump-layout value" );
                return false;
            }
            continue;
        }

        vxui_demo_shot_set_error( error, error_size, "unknown demo argument" );
        return false;
    }

    if ( backend_test_mode && request.enabled ) {
        vxui_demo_shot_set_error( error, error_size, "--vefc-backend-test cannot be combined with --shot" );
        return false;
    }

    const bool saw_shot_config =
        request.screen_name != nullptr
        || request.width > 0
        || request.height > 0
        || !request.out_path.empty()
        || request.locale_index != 0
        || request.prompt_table_index != 0
        || !request.focus_id.empty()
        || request.compact_override
        || request.disable_scanline;

    if ( saw_shot_config && !request.enabled && !saw_shot_only_flag ) {
        vxui_demo_shot_set_error( error, error_size, "shot options require --shot" );
        return false;
    }

    if ( request.enabled ) {
        if ( !vxui_demo_shot_screen_supported( request.screen_name ) ) {
            vxui_demo_shot_set_error( error, error_size, "invalid or missing --screen value" );
            return false;
        }
        if ( request.width <= 0 ) {
            vxui_demo_shot_set_error( error, error_size, "missing required --width value" );
            return false;
        }
        if ( request.height <= 0 ) {
            vxui_demo_shot_set_error( error, error_size, "missing required --height value" );
            return false;
        }
        if ( request.out_path.empty() ) {
            vxui_demo_shot_set_error( error, error_size, "missing required --out value" );
            return false;
        }
    }

    if ( out_backend_test_mode ) {
        *out_backend_test_mode = backend_test_mode;
    }
    if ( out_shot_request ) {
        *out_shot_request = request;
    }
    return true;
}
