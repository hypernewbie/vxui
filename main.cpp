#include <array>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include <glad/glad.h>
#include "TinyWindow.h"

#ifdef near
#undef near
#endif
#ifdef far
#undef far
#endif

#if defined( VE_FONTCACHE_FREETYPE_RASTERISATION )
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#endif

#if defined( VE_FONTCACHE_HARFBUZZ )
#include <hb.h>
#endif

#define CLAY_IMPLEMENTATION
#include "clay/clay.h"

#define VE_FONTCACHE_IMPL
#include "vefc/ve_fontcache.h"

#define VXUI_IMPL
#include "vxui.h"

#ifndef VXUI_SOURCE_DIR
#define VXUI_SOURCE_DIR "."
#endif

enum vxui_demo_font_id
{
    VXUI_DEMO_FONT_UI = 0,
    VXUI_DEMO_FONT_TITLE,
    VXUI_DEMO_FONT_DEBUG,
    VXUI_DEMO_FONT_JAPANESE,
    VXUI_DEMO_FONT_ARABIC,
};

enum vxui_demo_button
{
    VXUI_DEMO_BTN_NAV_UP,
    VXUI_DEMO_BTN_NAV_DOWN,
    VXUI_DEMO_BTN_NAV_LEFT,
    VXUI_DEMO_BTN_NAV_RIGHT,
    VXUI_DEMO_BTN_CONFIRM,
    VXUI_DEMO_BTN_CANCEL,
    VXUI_DEMO_BTN_TAB_LEFT,
    VXUI_DEMO_BTN_TAB_RIGHT,
    VXUI_DEMO_BTN_LOCALE_EN,
    VXUI_DEMO_BTN_LOCALE_JA,
    VXUI_DEMO_BTN_LOCALE_AR,
    VXUI_DEMO_BTN_PROMPT_KEYBOARD,
    VXUI_DEMO_BTN_PROMPT_GAMEPAD,
#ifdef VXUI_DEBUG
    VXUI_DEMO_BTN_DEBUG_TOGGLE,
    VXUI_DEMO_BTN_DEBUG_RELOAD,
    VXUI_DEMO_BTN_DEBUG_COPY,
#endif
    VXUI_DEMO_BTN_COUNT,
};

typedef struct vxui_demo_app
{
    bool show_settings;
    int difficulty;
    float volume;
    int locale_index;
    int prompt_table_index;
    vxui_ctx* ctx;
    std::string watched_seq_path;
    bool copied_to_clipboard;
    int debug_prop_index;
    float debug_delay_value;
    float debug_target_value;
    int last_selected_seq;
    bool button_prev[ VXUI_DEMO_BTN_COUNT ];
    char locale_status[ 64 ];
    char prompt_status[ 64 ];
    char screen_status[ 128 ];
    char preview_status[ 96 ];
    char clipboard_status[ 96 ];
    char reload_status[ 256 ];
} vxui_demo_app;

typedef struct vxui_demo_pulse
{
    float speed;
    float scale;
    float alpha;
} vxui_demo_pulse;

typedef struct vxui_demo_glow
{
    float padding;
    float alpha;
} vxui_demo_glow;

typedef struct vxui_demo_scanline
{
    float spacing;
    float alpha;
} vxui_demo_scanline;

typedef struct vxui_demo_spin
{
    float speed;
    float padding;
} vxui_demo_spin;

typedef struct vxui_demo_renderer
{
    GLuint primitive_program;
    GLuint fontcache_shader_render_glyph;
    GLuint fontcache_shader_blit_atlas;
    GLuint fontcache_shader_draw_text;
    GLuint vao;
    GLuint fontcache_fbo[ 2 ];
    GLuint fontcache_fbo_texture[ 2 ];
    std::vector< GLuint > cpu_atlas_textures;
    TinyWindow::vec2_t< unsigned int > window_size;
    ve_fontcache cache;
    std::vector< uint8_t > ui_font;
    std::vector< uint8_t > title_font;
    std::vector< uint8_t > debug_font;
    std::vector< uint8_t > japanese_font;
    std::vector< uint8_t > arabic_font;
} vxui_demo_renderer;

static const char* vxui_demo_text( const char* key, void* userdata );
static FILE* vxui_demo_fopen( const char* path, const char* mode );
static bool vxui_demo_write_file( const char* path, const char* text );
static std::string vxui_demo_make_temp_path( const char* filename );
static std::u8string vxui_demo_to_u8( const char* text );
static bool vxui_demo_copy_to_clipboard( const char* text );
static int vxui_demo_find_seq_index( const vxui_ctx* ctx, const char* name );
static void vxui_demo_sync_step_editor( vxui_demo_app* app );
static void vxui_demo_apply_step_editor( vxui_demo_app* app );
static void vxui_demo_refresh_status( vxui_demo_app* app );
static void vxui_demo_open_settings( vxui_ctx* ctx, uint32_t id, void* userdata );
static void vxui_demo_close_settings( vxui_ctx* ctx, uint32_t id, void* userdata );
static GLuint vxui_demo_compile_program( const char* vs_source, const char* fs_source, const char* label );
static bool vxui_demo_init_renderer( vxui_demo_renderer* renderer );
static void vxui_demo_shutdown_renderer( vxui_demo_renderer* renderer );
static bool vxui_demo_load_fonts( vxui_demo_renderer* renderer );
static void vxui_demo_render_draw_list( vxui_demo_renderer* renderer, const vxui_draw_list* list );
static void vxui_demo_draw_primitive( vxui_demo_renderer* renderer, vxui_rect bounds, vxui_color color, float radius, float border_width );
static void vxui_demo_draw_placeholder_image( vxui_demo_renderer* renderer, vxui_rect bounds );
static void vxui_demo_render_text_cmd( vxui_demo_renderer* renderer, const vxui_draw_cmd_text* text, const vxui_rect* clip_rect );
static void vxui_demo_render_fontcache_drawlist( vxui_demo_renderer* renderer, const vxui_rect* clip_rect );
static void vxui_demo_setup_fontcache_fbo( vxui_demo_renderer* renderer );
static void vxui_demo_flip_fontcache_upload( ve_fontcache_drawlist* drawlist, ve_fontcache_draw& dcall );
static bool vxui_demo_button_edge( vxui_demo_app* app, vxui_demo_button button, bool down );
static bool vxui_demo_key_down( const TinyWindow::tWindow* window, int key );
static bool vxui_demo_char_down( const TinyWindow::tWindow* window, char ch );
static const TinyWindow::gamepad_t* vxui_demo_primary_gamepad( TinyWindow::windowManager* manager );
static bool vxui_demo_gamepad_down( const TinyWindow::gamepad_t* gamepad, int button );

static const char* vxui_demo_text( const char* key, void* userdata )
{
    vxui_demo_app* app = ( vxui_demo_app* ) userdata;

    if ( app && app->ctx ) {
#ifdef VXUI_DEBUG
        if ( std::strcmp( key, "debug.overlay.title" ) == 0 ) {
            return "Sequence Debugger";
        }
        if ( std::strcmp( key, "debug.selected_seq" ) == 0 ) {
            if ( app->ctx->debug_seq_editor.selected_seq >= 0 && app->ctx->debug_seq_editor.selected_seq < app->ctx->registered_seq_count ) {
                return app->ctx->registered_seqs[ app->ctx->debug_seq_editor.selected_seq ].name;
            }
            return "none";
        }
        if ( std::strcmp( key, "debug.step.id" ) == 0 ) {
            if ( app->ctx->debug_seq_editor.selected_seq >= 0 && app->ctx->debug_seq_editor.selected_seq < app->ctx->registered_seq_count ) {
                const vxui_registered_seq* seq = &app->ctx->registered_seqs[ app->ctx->debug_seq_editor.selected_seq ];
                if ( seq->count > 0 ) {
                    static char fallback[ 32 ];
                    return vxui__get_seq_step_name( seq, 0, fallback, sizeof( fallback ) );
                }
            }
            return "none";
        }
        if ( std::strcmp( key, "debug.generated_c" ) == 0 ) {
            return app->ctx->debug_seq_editor.generated_c[ 0 ] ? app->ctx->debug_seq_editor.generated_c : "(no generated c output)";
        }
        if ( std::strcmp( key, "debug.generated_toml" ) == 0 ) {
            return app->ctx->debug_seq_editor.generated_toml[ 0 ] ? app->ctx->debug_seq_editor.generated_toml : "(no generated toml output)";
        }
        if ( std::strcmp( key, "debug.preview_status" ) == 0 ) {
            return app->preview_status;
        }
        if ( std::strcmp( key, "debug.clipboard" ) == 0 ) {
            return app->clipboard_status;
        }
        if ( std::strcmp( key, "debug.reload" ) == 0 ) {
            return app->reload_status;
        }
#endif
        if ( std::strcmp( key, "status.locale" ) == 0 ) {
            return app->locale_status;
        }
        if ( std::strcmp( key, "status.prompts" ) == 0 ) {
            return app->prompt_status;
        }
        if ( std::strcmp( key, "status.screens" ) == 0 ) {
            return app->screen_status;
        }
    }

    if ( app && app->locale_index == 1 ) {
        if ( std::strcmp( key, "menu.main" ) == 0 ) return "メインメニュー";
        if ( std::strcmp( key, "menu.settings" ) == 0 ) return "設定";
        if ( std::strcmp( key, "menu.open_settings" ) == 0 ) return "設定を開く";
        if ( std::strcmp( key, "menu.back" ) == 0 ) return "戻る";
        if ( std::strcmp( key, "menu.difficulty" ) == 0 ) return "難易度";
        if ( std::strcmp( key, "menu.volume" ) == 0 ) return "音量";
        if ( std::strcmp( key, "menu.saves" ) == 0 ) return "セーブ";
        if ( std::strcmp( key, "menu.confirm" ) == 0 ) return "決定";
        if ( std::strcmp( key, "menu.cancel" ) == 0 ) return "戻る";
        if ( std::strcmp( key, "menu.locale" ) == 0 ) return "言語";
        if ( std::strcmp( key, "menu.prompts" ) == 0 ) return "プロンプト";
        if ( std::strcmp( key, "menu.controls" ) == 0 ) return "操作";
        if ( std::strcmp( key, "menu.debug" ) == 0 ) return "デバッグ";
        if ( std::strcmp( key, "difficulty.easy" ) == 0 ) return "かんたん";
        if ( std::strcmp( key, "difficulty.normal" ) == 0 ) return "ふつう";
        if ( std::strcmp( key, "difficulty.hard" ) == 0 ) return "むずかしい";
    } else if ( app && app->locale_index == 2 ) {
        if ( std::strcmp( key, "menu.main" ) == 0 ) return "القائمة الرئيسية";
        if ( std::strcmp( key, "menu.settings" ) == 0 ) return "الإعدادات";
        if ( std::strcmp( key, "menu.open_settings" ) == 0 ) return "افتح الإعدادات";
        if ( std::strcmp( key, "menu.back" ) == 0 ) return "رجوع";
        if ( std::strcmp( key, "menu.difficulty" ) == 0 ) return "الصعوبة";
        if ( std::strcmp( key, "menu.volume" ) == 0 ) return "الصوت";
        if ( std::strcmp( key, "menu.saves" ) == 0 ) return "الحفظ";
        if ( std::strcmp( key, "menu.confirm" ) == 0 ) return "تأكيد";
        if ( std::strcmp( key, "menu.cancel" ) == 0 ) return "إلغاء";
        if ( std::strcmp( key, "menu.locale" ) == 0 ) return "اللغة";
        if ( std::strcmp( key, "menu.prompts" ) == 0 ) return "الرموز";
        if ( std::strcmp( key, "menu.controls" ) == 0 ) return "التحكم";
        if ( std::strcmp( key, "menu.debug" ) == 0 ) return "التصحيح";
        if ( std::strcmp( key, "difficulty.easy" ) == 0 ) return "سهل";
        if ( std::strcmp( key, "difficulty.normal" ) == 0 ) return "عادي";
        if ( std::strcmp( key, "difficulty.hard" ) == 0 ) return "صعب";
    } else {
        if ( std::strcmp( key, "menu.main" ) == 0 ) return "Main Menu";
        if ( std::strcmp( key, "menu.settings" ) == 0 ) return "Settings";
        if ( std::strcmp( key, "menu.open_settings" ) == 0 ) return "Open Settings";
        if ( std::strcmp( key, "menu.back" ) == 0 ) return "Back";
        if ( std::strcmp( key, "menu.difficulty" ) == 0 ) return "Difficulty";
        if ( std::strcmp( key, "menu.volume" ) == 0 ) return "Volume";
        if ( std::strcmp( key, "menu.saves" ) == 0 ) return "Save Slots";
        if ( std::strcmp( key, "menu.confirm" ) == 0 ) return "Confirm";
        if ( std::strcmp( key, "menu.cancel" ) == 0 ) return "Cancel";
        if ( std::strcmp( key, "menu.locale" ) == 0 ) return "Locale";
        if ( std::strcmp( key, "menu.prompts" ) == 0 ) return "Prompt Table";
        if ( std::strcmp( key, "menu.controls" ) == 0 ) return "Controls";
        if ( std::strcmp( key, "menu.debug" ) == 0 ) return "Debug Overlay";
        if ( std::strcmp( key, "difficulty.easy" ) == 0 ) return "Easy";
        if ( std::strcmp( key, "difficulty.normal" ) == 0 ) return "Normal";
        if ( std::strcmp( key, "difficulty.hard" ) == 0 ) return "Hard";
    }

    if ( std::strcmp( key, "slot.0" ) == 0 ) return "Autosave";
    if ( std::strcmp( key, "slot.1" ) == 0 ) return "Profile 1";
    if ( std::strcmp( key, "slot.2" ) == 0 ) return "Profile 2";
    if ( std::strcmp( key, "slot.3" ) == 0 ) return "Profile 3";
    if ( std::strcmp( key, "slot.4" ) == 0 ) return "Profile 4";
    if ( std::strcmp( key, "slot.5" ) == 0 ) return "Profile 5";
    if ( std::strcmp( key, "slot.6" ) == 0 ) return "Profile 6";
    if ( std::strcmp( key, "slot.7" ) == 0 ) return "Profile 7";
    if ( std::strcmp( key, "hint.controls.0" ) == 0 ) return "Arrows / D-pad navigate, Enter / Space / A confirm.";
    if ( std::strcmp( key, "hint.controls.1" ) == 0 ) return "Escape / Backspace / B cancels, [ and ] tab between lanes.";
    if ( std::strcmp( key, "hint.controls.2" ) == 0 ) return "1 2 3 switch locale, 4 5 switch prompt table.";
#ifdef VXUI_DEBUG
    if ( std::strcmp( key, "hint.controls.3" ) == 0 ) return "F1 toggles the debugger, R polls hot reload, C copies or prints exports.";
#else
    if ( std::strcmp( key, "hint.controls.3" ) == 0 ) return "Build a Debug configuration to enable the sequence debugger overlay.";
#endif
    return key;
}

static FILE* vxui_demo_fopen( const char* path, const char* mode )
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

static bool vxui_demo_write_file( const char* path, const char* text )
{
    FILE* fp = vxui_demo_fopen( path, "wb" );
    if ( !fp ) {
        return false;
    }

    size_t length = std::strlen( text );
    bool ok = std::fwrite( text, 1, length, fp ) == length;
    std::fclose( fp );
    return ok;
}

static std::string vxui_demo_make_temp_path( const char* filename )
{
    std::filesystem::path temp = std::filesystem::temp_directory_path() / "vxui-demo";
    std::error_code ec;
    std::filesystem::create_directories( temp, ec );
    std::filesystem::path path = temp / filename;
    return path.make_preferred().string();
}

static std::u8string vxui_demo_to_u8( const char* text )
{
    if ( !text ) {
        return {};
    }
    const char8_t* begin = reinterpret_cast< const char8_t* >( text );
    return std::u8string( begin, begin + std::strlen( text ) );
}

static bool vxui_demo_copy_to_clipboard( const char* text )
{
#if defined( _WIN32 )
    if ( !text || !OpenClipboard( nullptr ) ) {
        return false;
    }

    EmptyClipboard();
    size_t length = std::strlen( text ) + 1u;
    HGLOBAL handle = GlobalAlloc( GMEM_MOVEABLE, length );
    if ( !handle ) {
        CloseClipboard();
        return false;
    }

    void* memory = GlobalLock( handle );
    if ( !memory ) {
        GlobalFree( handle );
        CloseClipboard();
        return false;
    }
    std::memcpy( memory, text, length );
    GlobalUnlock( handle );

    if ( !SetClipboardData( CF_TEXT, handle ) ) {
        GlobalFree( handle );
        CloseClipboard();
        return false;
    }
    CloseClipboard();
    return true;
#else
    ( void ) text;
    return false;
#endif
}

static int vxui_demo_find_seq_index( const vxui_ctx* ctx, const char* name )
{
    if ( !ctx || !name ) {
        return -1;
    }
    for ( int i = 0; i < ctx->registered_seq_count; ++i ) {
        if ( std::strcmp( ctx->registered_seqs[ i ].name, name ) == 0 ) {
            return i;
        }
    }
    return -1;
}

static void vxui_demo_sync_step_editor( vxui_demo_app* app )
{
#ifdef VXUI_DEBUG
    if ( !app || !app->ctx ) {
        return;
    }

    int selected = app->ctx->debug_seq_editor.selected_seq;
    if ( selected < 0 || selected >= app->ctx->registered_seq_count ) {
        return;
    }

    vxui_registered_seq* seq = &app->ctx->registered_seqs[ selected ];
    if ( seq->count <= 0 ) {
        return;
    }

    app->debug_delay_value = ( float ) seq->steps[ 0 ].delay_ms;
    app->debug_target_value = seq->steps[ 0 ].target;
    switch ( seq->steps[ 0 ].prop ) {
        case VXUI_PROP_OPACITY: app->debug_prop_index = 0; break;
        case VXUI_PROP_SCALE: app->debug_prop_index = 1; break;
        case VXUI_PROP_SLIDE_X: app->debug_prop_index = 2; break;
        case VXUI_PROP_SLIDE_Y: app->debug_prop_index = 3; break;
    }
    app->last_selected_seq = selected;
#else
    ( void ) app;
#endif
}

static void vxui_demo_apply_step_editor( vxui_demo_app* app )
{
#ifdef VXUI_DEBUG
    if ( !app || !app->ctx ) {
        return;
    }

    int selected = app->ctx->debug_seq_editor.selected_seq;
    if ( selected < 0 || selected >= app->ctx->registered_seq_count ) {
        return;
    }

    vxui_registered_seq* seq = &app->ctx->registered_seqs[ selected ];
    if ( seq->count <= 0 ) {
        return;
    }

    seq->steps[ 0 ].delay_ms = ( uint32_t ) std::lround( app->debug_delay_value );
    seq->steps[ 0 ].target = app->debug_target_value;
    seq->steps[ 0 ].prop = app->debug_prop_index == 0 ? VXUI_PROP_OPACITY :
                           app->debug_prop_index == 1 ? VXUI_PROP_SCALE :
                           app->debug_prop_index == 2 ? VXUI_PROP_SLIDE_X : VXUI_PROP_SLIDE_Y;
#else
    ( void ) app;
#endif
}

static void vxui_demo_refresh_status( vxui_demo_app* app )
{
    if ( !app || !app->ctx ) {
        return;
    }

    const char* locale = app->locale_index == 0 ? "en" : app->locale_index == 1 ? "ja-JP" : "ar";
    const char* prompts = app->prompt_table_index == 0 ? "Keyboard" : "Gamepad";
    std::snprintf( app->locale_status, sizeof( app->locale_status ), "Locale: %s", locale );
    std::snprintf( app->prompt_status, sizeof( app->prompt_status ), "Prompts: %s", prompts );

    const vxui_screen* top = app->ctx->screen_count > 0 ? &app->ctx->screens[ app->ctx->screen_count - 1 ] : nullptr;
    std::snprintf( app->screen_status, sizeof( app->screen_status ), "Screens: %d | Top: %s", app->ctx->screen_count, top ? top->name : "none" );

#ifdef VXUI_DEBUG
    std::snprintf(
        app->preview_status,
        sizeof( app->preview_status ),
        "Preview: %s (%d cmds)",
        app->ctx->debug_seq_editor.preview_playing ? "captured" : "idle",
        app->ctx->debug_seq_editor.preview_snapshot.command_count );
    std::snprintf(
        app->clipboard_status,
        sizeof( app->clipboard_status ),
        "%s",
        app->copied_to_clipboard ? "Copied current TOML export to the clipboard." : "Press C to copy or print the current export." );
    if ( app->reload_status[ 0 ] == '\0' ) {
        std::snprintf( app->reload_status, sizeof( app->reload_status ), "%s", "Press R to poll the watched sequence file." );
    }
#else
    std::snprintf( app->preview_status, sizeof( app->preview_status ), "%s", "Preview capture is only available in Debug builds." );
    std::snprintf( app->clipboard_status, sizeof( app->clipboard_status ), "%s", "Sequence export tools are only available in Debug builds." );
    std::snprintf( app->reload_status, sizeof( app->reload_status ), "%s", "Hot reload is only available in Debug builds." );
#endif
}

static void vxui_demo_open_settings( vxui_ctx* ctx, uint32_t id, void* userdata )
{
    ( void ) id;
    vxui_demo_app* app = ( vxui_demo_app* ) userdata;
    if ( !ctx || !app ) {
        return;
    }
    app->show_settings = true;
    vxui_push_screen( ctx, "settings" );
}

static void vxui_demo_close_settings( vxui_ctx* ctx, uint32_t id, void* userdata )
{
    ( void ) id;
    vxui_demo_app* app = ( vxui_demo_app* ) userdata;
    if ( !ctx || !app || !app->show_settings ) {
        return;
    }
    app->show_settings = false;
    vxui_pop_screen( ctx );
}

static GLuint vxui_demo_compile_program( const char* vs_source, const char* fs_source, const char* label )
{
    auto print_shader_log = []( GLuint shader ) {
        char log[ 4096 ] = {};
        glGetShaderInfoLog( shader, ( GLsizei ) sizeof( log ), nullptr, log );
        if ( log[ 0 ] ) {
            std::fprintf( stderr, "%s\n", log );
        }
    };
    auto print_program_log = []( GLuint program ) {
        char log[ 4096 ] = {};
        glGetProgramInfoLog( program, ( GLsizei ) sizeof( log ), nullptr, log );
        if ( log[ 0 ] ) {
            std::fprintf( stderr, "%s\n", log );
        }
    };

    GLuint vs = glCreateShader( GL_VERTEX_SHADER );
    GLuint fs = glCreateShader( GL_FRAGMENT_SHADER );
    GLint ok = 0;

    glShaderSource( vs, 1, &vs_source, nullptr );
    glCompileShader( vs );
    glGetShaderiv( vs, GL_COMPILE_STATUS, &ok );
    if ( !ok ) {
        std::fprintf( stderr, "Failed to compile %s vertex shader.\n", label );
        print_shader_log( vs );
        glDeleteShader( vs );
        glDeleteShader( fs );
        return 0;
    }

    glShaderSource( fs, 1, &fs_source, nullptr );
    glCompileShader( fs );
    glGetShaderiv( fs, GL_COMPILE_STATUS, &ok );
    if ( !ok ) {
        std::fprintf( stderr, "Failed to compile %s fragment shader.\n", label );
        print_shader_log( fs );
        glDeleteShader( vs );
        glDeleteShader( fs );
        return 0;
    }

    GLuint program = glCreateProgram();
    glAttachShader( program, vs );
    glAttachShader( program, fs );
    glBindAttribLocation( program, 0, "vpos" );
    glBindAttribLocation( program, 1, "vtex" );
    glLinkProgram( program );
    glGetProgramiv( program, GL_LINK_STATUS, &ok );
    if ( !ok ) {
        std::fprintf( stderr, "Failed to link %s program.\n", label );
        print_program_log( program );
        glDeleteProgram( program );
        program = 0;
    }

    if ( program ) {
        glDetachShader( program, vs );
        glDetachShader( program, fs );
    }
    glDeleteShader( vs );
    glDeleteShader( fs );
    return program;
}

static void vxui_demo_setup_fontcache_fbo( vxui_demo_renderer* renderer )
{
    glGenFramebuffers( 2, renderer->fontcache_fbo );
    glGenTextures( 2, renderer->fontcache_fbo_texture );

    glBindFramebuffer( GL_FRAMEBUFFER, renderer->fontcache_fbo[ 0 ] );
    glBindTexture( GL_TEXTURE_2D, renderer->fontcache_fbo_texture[ 0 ] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_R8, VE_FONTCACHE_GLYPHDRAW_BUFFER_WIDTH, VE_FONTCACHE_GLYPHDRAW_BUFFER_HEIGHT, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderer->fontcache_fbo_texture[ 0 ], 0 );

    glBindFramebuffer( GL_FRAMEBUFFER, renderer->fontcache_fbo[ 1 ] );
    glBindTexture( GL_TEXTURE_2D, renderer->fontcache_fbo_texture[ 1 ] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_R8, VE_FONTCACHE_ATLAS_WIDTH, VE_FONTCACHE_ATLAS_HEIGHT, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderer->fontcache_fbo_texture[ 1 ], 0 );

    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
}

static void vxui_demo_flip_fontcache_upload( ve_fontcache_drawlist* drawlist, ve_fontcache_draw& dcall )
{
#ifdef VE_FONTCACHE_FREETYPE_RASTERISATION
    if ( dcall.pass != VE_FONTCACHE_FRAMEBUFFER_PASS_ATLAS_UPLOAD ) {
        return;
    }

    static std::vector< uint8_t > temp_texels;
    size_t total = ( size_t ) dcall.upload_region_w * ( size_t ) dcall.upload_region_h;
    temp_texels.resize( total );
    uint8_t* src = &drawlist->texels[ dcall.texel_offset ];
    for ( uint32_t y = 0; y < dcall.upload_region_h; ++y ) {
        uint32_t dst_row = dcall.upload_region_h - 1u - y;
        std::memcpy( &temp_texels[ dst_row * dcall.upload_region_w ], &src[ y * dcall.upload_region_w ], dcall.upload_region_w );
    }
    std::memcpy( src, temp_texels.data(), total );
#else
    ( void ) drawlist;
    ( void ) dcall;
#endif
}

static bool vxui_demo_init_renderer( vxui_demo_renderer* renderer )
{
    static const char* kPrimitiveVs = R"(#version 330 core
out vec2 local_pos;
uniform vec4 u_bounds;
uniform vec2 u_viewport;
const vec2 kVerts[6] = vec2[6](
    vec2(0.0, 0.0),
    vec2(1.0, 0.0),
    vec2(1.0, 1.0),
    vec2(0.0, 0.0),
    vec2(1.0, 1.0),
    vec2(0.0, 1.0)
);
void main()
{
    vec2 unit = kVerts[ gl_VertexID ];
    vec2 pixel = u_bounds.xy + unit * u_bounds.zw;
    vec2 ndc = vec2(
        pixel.x / u_viewport.x * 2.0 - 1.0,
        1.0 - pixel.y / u_viewport.y * 2.0
    );
    local_pos = unit * u_bounds.zw;
    gl_Position = vec4( ndc, 0.0, 1.0 );
})";
    static const char* kPrimitiveFs = R"(#version 330 core
in vec2 local_pos;
out vec4 fragc;
uniform vec2 u_size;
uniform vec4 u_color;
uniform float u_radius;
uniform float u_border;
float rounded_box_sdf( vec2 p, vec2 half_extents, float radius )
{
    vec2 q = abs( p ) - ( half_extents - vec2( radius ) );
    return length( max( q, 0.0 ) ) + min( max( q.x, q.y ), 0.0 ) - radius;
}
void main()
{
    vec2 half_extents = max( u_size * 0.5, vec2( 0.0 ) );
    float radius = min( u_radius, min( half_extents.x, half_extents.y ) );
    vec2 p = local_pos - half_extents;
    float outer = rounded_box_sdf( p, half_extents, radius );
    float outer_alpha = 1.0 - smoothstep( 0.0, 1.0, outer );
    float alpha = outer_alpha;
    if ( u_border > 0.0 ) {
        vec2 inner_half = max( half_extents - vec2( u_border ), vec2( 0.0 ) );
        float inner_radius = max( radius - u_border, 0.0 );
        float inner = rounded_box_sdf( p, inner_half, inner_radius );
        float inner_alpha = 1.0 - smoothstep( 0.0, 1.0, inner );
        alpha = max( outer_alpha - inner_alpha, 0.0 );
    }
    fragc = vec4( u_color.rgb, u_color.a * alpha );
})";
    static const char* kFontcacheVsShared = R"(#version 330 core
in vec2 vpos;
in vec2 vtex;
out vec2 uv;
void main( void )
{
    uv = vtex;
    gl_Position = vec4( vpos.xy, 0.0, 1.0 );
})";
    static const char* kFontcacheFsRenderGlyph = R"(#version 330 core
out vec4 fragc;
void main( void )
{
    fragc = vec4( 1.0, 1.0, 1.0, 1.0 );
})";
    static const char* kFontcacheFsBlitAtlas = R"(#version 330 core
in vec2 uv;
out vec4 fragc;
uniform uint region;
uniform sampler2D src_texture;
float downsample( vec2 uv, vec2 texsz )
{
    float v =
        texture( src_texture, uv + vec2( 0.0f, 0.0f ) * texsz ).x * 0.25f +
        texture( src_texture, uv + vec2( 0.0f, 1.0f ) * texsz ).x * 0.25f +
        texture( src_texture, uv + vec2( 1.0f, 0.0f ) * texsz ).x * 0.25f +
        texture( src_texture, uv + vec2( 1.0f, 1.0f ) * texsz ).x * 0.25f;
    return v;
}
void main( void )
{
    const vec2 texsz = 1.0f / vec2( 2048, 512 );
    if ( region == 0u || region == 1u || region == 2u ) {
        float v =
            downsample( uv + vec2( -1.5f, -1.5f ) * texsz, texsz ) * 0.25f +
            downsample( uv + vec2(  0.5f, -1.5f ) * texsz, texsz ) * 0.25f +
            downsample( uv + vec2( -1.5f,  0.5f ) * texsz, texsz ) * 0.25f +
            downsample( uv + vec2(  0.5f,  0.5f ) * texsz, texsz ) * 0.25f;
        fragc = vec4( 1, 1, 1, v );
    } else {
        fragc = vec4( 0, 0, 0, 1 );
    }
})";
    static const char* kFontcacheVsDrawText = R"(#version 330 core
in vec2 vpos;
in vec2 vtex;
out vec2 uv;
void main( void )
{
    uv = vtex;
    gl_Position = vec4( vpos.xy * 2.0f - 1.0f, 0.0, 1.0 );
})";
    static const char* kFontcacheFsDrawText = R"(#version 330 core
in vec2 uv;
out vec4 fragc;
uniform sampler2D src_texture;
uniform uint downsample;
uniform vec4 colour;
void main( void )
{
    float v = texture( src_texture, uv ).x;
    if ( downsample == 1u ) {
        const vec2 texsz = 1.0f / vec2( 2048, 512 );
        v =
            texture( src_texture, uv + vec2( -0.5f, -0.5f ) * texsz ).x * 0.25f +
            texture( src_texture, uv + vec2( -0.5f,  0.5f ) * texsz ).x * 0.25f +
            texture( src_texture, uv + vec2(  0.5f, -0.5f ) * texsz ).x * 0.25f +
            texture( src_texture, uv + vec2(  0.5f,  0.5f ) * texsz ).x * 0.25f;
    }
    fragc = vec4( colour.xyz, colour.a * v );
})";

    renderer->primitive_program = vxui_demo_compile_program( kPrimitiveVs, kPrimitiveFs, "primitive" );
    renderer->fontcache_shader_render_glyph = vxui_demo_compile_program( kFontcacheVsShared, kFontcacheFsRenderGlyph, "fontcache-render-glyph" );
    renderer->fontcache_shader_blit_atlas = vxui_demo_compile_program( kFontcacheVsShared, kFontcacheFsBlitAtlas, "fontcache-blit-atlas" );
    renderer->fontcache_shader_draw_text = vxui_demo_compile_program( kFontcacheVsDrawText, kFontcacheFsDrawText, "fontcache-draw-text" );
    if ( !renderer->primitive_program || !renderer->fontcache_shader_render_glyph || !renderer->fontcache_shader_blit_atlas || !renderer->fontcache_shader_draw_text ) {
        return false;
    }

    glGenVertexArrays( 1, &renderer->vao );
    glBindVertexArray( renderer->vao );
    vxui_demo_setup_fontcache_fbo( renderer );
    glEnable( GL_BLEND );
    glBlendEquation( GL_FUNC_ADD );
    return true;
}

static void vxui_demo_shutdown_renderer( vxui_demo_renderer* renderer )
{
    if ( !renderer ) {
        return;
    }
    if ( renderer->primitive_program ) glDeleteProgram( renderer->primitive_program );
    if ( renderer->fontcache_shader_render_glyph ) glDeleteProgram( renderer->fontcache_shader_render_glyph );
    if ( renderer->fontcache_shader_blit_atlas ) glDeleteProgram( renderer->fontcache_shader_blit_atlas );
    if ( renderer->fontcache_shader_draw_text ) glDeleteProgram( renderer->fontcache_shader_draw_text );
    if ( renderer->vao ) glDeleteVertexArrays( 1, &renderer->vao );
    if ( renderer->fontcache_fbo[ 0 ] || renderer->fontcache_fbo[ 1 ] ) glDeleteFramebuffers( 2, renderer->fontcache_fbo );
    if ( renderer->fontcache_fbo_texture[ 0 ] || renderer->fontcache_fbo_texture[ 1 ] ) glDeleteTextures( 2, renderer->fontcache_fbo_texture );
    if ( !renderer->cpu_atlas_textures.empty() ) {
        glDeleteTextures( ( GLsizei ) renderer->cpu_atlas_textures.size(), renderer->cpu_atlas_textures.data() );
    }
    ve_fontcache_shutdown( &renderer->cache );
}

static bool vxui_demo_load_fonts( vxui_demo_renderer* renderer )
{
    struct font_load
    {
        const char* relative_path;
        std::vector< uint8_t >* buffer;
        float size_px;
        int expected_id;
    };
    const std::filesystem::path source_dir = std::filesystem::path( VXUI_SOURCE_DIR );
    const font_load fonts[] = {
        { "vefc/demo/fonts/OpenSans-Regular.ttf", &renderer->ui_font, 24.0f, VXUI_DEMO_FONT_UI },
        { "vefc/demo/fonts/Bitter-Regular.ttf", &renderer->title_font, 44.0f, VXUI_DEMO_FONT_TITLE },
        { "vefc/demo/fonts/NovaMono-Regular.ttf", &renderer->debug_font, 16.0f, VXUI_DEMO_FONT_DEBUG },
        { "vefc/demo/fonts/NotoSansJP-Regular.otf", &renderer->japanese_font, 24.0f, VXUI_DEMO_FONT_JAPANESE },
        { "vefc/demo/fonts/Tajawal-Regular.ttf", &renderer->arabic_font, 24.0f, VXUI_DEMO_FONT_ARABIC },
    };

    ve_fontcache_init( &renderer->cache, true );
    for ( const font_load& font : fonts ) {
        const std::filesystem::path full_path = source_dir / font.relative_path;
        ve_font_id id = ve_fontcache_loadfile( &renderer->cache, full_path.string().c_str(), *font.buffer, font.size_px );
        if ( id != font.expected_id ) {
            std::fprintf( stderr, "Failed to load demo font '%s' with the expected id %d (got %lld).\n", full_path.string().c_str(), font.expected_id, ( long long ) id );
            return false;
        }
    }
    return true;
}

static void vxui_demo_draw_primitive( vxui_demo_renderer* renderer, vxui_rect bounds, vxui_color color, float radius, float border_width )
{
    glUseProgram( renderer->primitive_program );
    glBindVertexArray( renderer->vao );
    glUniform4f( glGetUniformLocation( renderer->primitive_program, "u_bounds" ), bounds.x, bounds.y, bounds.w, bounds.h );
    glUniform2f( glGetUniformLocation( renderer->primitive_program, "u_viewport" ), ( float ) renderer->window_size.width, ( float ) renderer->window_size.height );
    glUniform2f( glGetUniformLocation( renderer->primitive_program, "u_size" ), bounds.w, bounds.h );
    glUniform4f(
        glGetUniformLocation( renderer->primitive_program, "u_color" ),
        color.r / 255.0f,
        color.g / 255.0f,
        color.b / 255.0f,
        color.a / 255.0f );
    glUniform1f( glGetUniformLocation( renderer->primitive_program, "u_radius" ), radius );
    glUniform1f( glGetUniformLocation( renderer->primitive_program, "u_border" ), border_width );
    glDrawArrays( GL_TRIANGLES, 0, 6 );
}

static void vxui_demo_draw_placeholder_image( vxui_demo_renderer* renderer, vxui_rect bounds )
{
    vxui_demo_draw_primitive( renderer, bounds, ( vxui_color ) { 40, 48, 68, 255 }, 6.0f, 0.0f );
    vxui_demo_draw_primitive( renderer, bounds, ( vxui_color ) { 255, 128, 200, 255 }, 6.0f, 2.0f );
}

static void vxui_demo_render_fontcache_drawlist( vxui_demo_renderer* renderer, const vxui_rect* clip_rect )
{
    ve_fontcache_optimise_drawlist( &renderer->cache );
    ve_fontcache_drawlist* drawlist = ve_fontcache_get_drawlist( &renderer->cache );
    if ( !drawlist || drawlist->vertices.empty() || drawlist->indices.empty() ) {
        ve_fontcache_flush_drawlist( &renderer->cache );
        return;
    }

    GLuint vbo = 0;
    GLuint ibo = 0;
    glGenBuffers( 1, &vbo );
    glGenBuffers( 1, &ibo );
    glBindVertexArray( renderer->vao );
    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    glBufferData( GL_ARRAY_BUFFER, drawlist->vertices.size() * sizeof( ve_fontcache_vertex ), drawlist->vertices.data(), GL_DYNAMIC_DRAW );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibo );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, drawlist->indices.size() * sizeof( uint32_t ), drawlist->indices.data(), GL_DYNAMIC_DRAW );
    glEnableVertexAttribArray( 0 );
    glEnableVertexAttribArray( 1 );
    glVertexAttribPointer( 0, 2, GL_FLOAT, false, sizeof( ve_fontcache_vertex ), nullptr );
    glVertexAttribPointer( 1, 2, GL_FLOAT, false, sizeof( ve_fontcache_vertex ), ( GLvoid* ) ( 2 * sizeof( float ) ) );

    glDisable( GL_CULL_FACE );
    glEnable( GL_BLEND );

    for ( ve_fontcache_draw& dcall : drawlist->dcalls ) {
        if ( dcall.pass == VE_FONTCACHE_FRAMEBUFFER_PASS_GLYPH ) {
            glUseProgram( renderer->fontcache_shader_render_glyph );
            glBindFramebuffer( GL_FRAMEBUFFER, renderer->fontcache_fbo[ 0 ] );
            glBlendFunc( GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_COLOR );
            glViewport( 0, 0, VE_FONTCACHE_GLYPHDRAW_BUFFER_WIDTH, VE_FONTCACHE_GLYPHDRAW_BUFFER_HEIGHT );
            glEnable( GL_SCISSOR_TEST );
            glScissor( 0, 0, VE_FONTCACHE_GLYPHDRAW_BUFFER_WIDTH, VE_FONTCACHE_GLYPHDRAW_BUFFER_HEIGHT );
            glDisable( GL_FRAMEBUFFER_SRGB );
        } else if ( dcall.pass == VE_FONTCACHE_FRAMEBUFFER_PASS_ATLAS ) {
            glUseProgram( renderer->fontcache_shader_blit_atlas );
            glBindFramebuffer( GL_FRAMEBUFFER, renderer->fontcache_fbo[ 1 ] );
            glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
            glViewport( 0, 0, VE_FONTCACHE_ATLAS_WIDTH, VE_FONTCACHE_ATLAS_HEIGHT );
            glEnable( GL_SCISSOR_TEST );
            glScissor( 0, 0, VE_FONTCACHE_ATLAS_WIDTH, VE_FONTCACHE_ATLAS_HEIGHT );
            glUniform1i( glGetUniformLocation( renderer->fontcache_shader_blit_atlas, "src_texture" ), 0 );
            glUniform1ui( glGetUniformLocation( renderer->fontcache_shader_blit_atlas, "region" ), dcall.region );
            glActiveTexture( GL_TEXTURE0 );
            glBindTexture( GL_TEXTURE_2D, renderer->fontcache_fbo_texture[ 0 ] );
            glDisable( GL_FRAMEBUFFER_SRGB );
        } else if ( dcall.pass == VE_FONTCACHE_FRAMEBUFFER_PASS_TARGET || dcall.pass == VE_FONTCACHE_FRAMEBUFFER_PASS_TARGET_UNCACHED || dcall.pass == VE_FONTCACHE_FRAMEBUFFER_PASS_TARGET_CPU_CACHED ) {
            glUseProgram( renderer->fontcache_shader_draw_text );
            glBindFramebuffer( GL_FRAMEBUFFER, 0 );
            glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
            glViewport( 0, 0, renderer->window_size.width, renderer->window_size.height );
            if ( clip_rect && clip_rect->w > 0.0f && clip_rect->h > 0.0f ) {
                GLint x = ( GLint ) std::lround( clip_rect->x );
                GLint y = ( GLint ) std::lround( ( float ) renderer->window_size.height - clip_rect->y - clip_rect->h );
                GLsizei w = ( GLsizei ) std::lround( clip_rect->w );
                GLsizei h = ( GLsizei ) std::lround( clip_rect->h );
                glEnable( GL_SCISSOR_TEST );
                glScissor( x, y, w < 0 ? 0 : w, h < 0 ? 0 : h );
            } else {
                glDisable( GL_SCISSOR_TEST );
            }
            glUniform1i( glGetUniformLocation( renderer->fontcache_shader_draw_text, "src_texture" ), 0 );
#ifdef VE_FONTCACHE_FREETYPE_RASTERISATION
            if ( dcall.pass == VE_FONTCACHE_FRAMEBUFFER_PASS_TARGET_CPU_CACHED ) {
                glUniform1ui( glGetUniformLocation( renderer->fontcache_shader_draw_text, "downsample" ), 0 );
                glActiveTexture( GL_TEXTURE0 );
                glBindTexture( GL_TEXTURE_2D, renderer->cpu_atlas_textures[ dcall.atlas_page ] );
            } else
#endif
            {
                glUniform1ui( glGetUniformLocation( renderer->fontcache_shader_draw_text, "downsample" ), dcall.pass == VE_FONTCACHE_FRAMEBUFFER_PASS_TARGET_UNCACHED ? 1u : 0u );
                glActiveTexture( GL_TEXTURE0 );
                glBindTexture( GL_TEXTURE_2D, dcall.pass == VE_FONTCACHE_FRAMEBUFFER_PASS_TARGET_UNCACHED ? renderer->fontcache_fbo_texture[ 0 ] : renderer->fontcache_fbo_texture[ 1 ] );
            }
            glUniform4fv( glGetUniformLocation( renderer->fontcache_shader_draw_text, "colour" ), 1, dcall.colour );
            glEnable( GL_FRAMEBUFFER_SRGB );
#ifdef VE_FONTCACHE_FREETYPE_RASTERISATION
        } else if ( dcall.pass == VE_FONTCACHE_FRAMEBUFFER_PASS_ATLAS_PAGE_TEXTURE_CREATE ) {
            renderer->cpu_atlas_textures.resize( std::max( ( int ) renderer->cpu_atlas_textures.size(), ( int ) dcall.atlas_page + 1 ) );
            glGenTextures( 1, &renderer->cpu_atlas_textures[ dcall.atlas_page ] );
            glActiveTexture( GL_TEXTURE0 );
            glBindTexture( GL_TEXTURE_2D, renderer->cpu_atlas_textures[ dcall.atlas_page ] );
            glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
            glPixelStorei( GL_PACK_ALIGNMENT, 1 );
            glTexImage2D( GL_TEXTURE_2D, 0, GL_R8, VE_FONTCACHE_CPU_ATLAS_PAGE_SIZE, VE_FONTCACHE_CPU_ATLAS_PAGE_SIZE, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        } else if ( dcall.pass == VE_FONTCACHE_FRAMEBUFFER_PASS_ATLAS_UPLOAD ) {
            glActiveTexture( GL_TEXTURE0 );
            glBindTexture( GL_TEXTURE_2D, renderer->cpu_atlas_textures[ dcall.atlas_page ] );
            glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
            glPixelStorei( GL_PACK_ALIGNMENT, 1 );
            vxui_demo_flip_fontcache_upload( drawlist, dcall );
            glTexSubImage2D( GL_TEXTURE_2D, 0, dcall.upload_region_x, dcall.upload_region_y, dcall.upload_region_w, dcall.upload_region_h, GL_RED, GL_UNSIGNED_BYTE, &drawlist->texels[ dcall.texel_offset ] );
#endif
        }

        if ( dcall.clear_before_draw ) {
            glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
            glClear( GL_COLOR_BUFFER_BIT );
        }
        if ( dcall.end_index > dcall.start_index ) {
            glDrawElements( GL_TRIANGLES, dcall.end_index - dcall.start_index, GL_UNSIGNED_INT, ( GLvoid* ) ( dcall.start_index * sizeof( uint32_t ) ) );
        }
    }

    glDeleteBuffers( 1, &vbo );
    glDeleteBuffers( 1, &ibo );
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    ve_fontcache_flush_drawlist( &renderer->cache );
}

static void vxui_demo_render_text_cmd( vxui_demo_renderer* renderer, const vxui_draw_cmd_text* text, const vxui_rect* clip_rect )
{
    if ( !renderer || !text || !text->text ) {
        return;
    }

    float colour[ 4 ] = {
        text->color.r / 255.0f,
        text->color.g / 255.0f,
        text->color.b / 255.0f,
        text->color.a / 255.0f,
    };
    ve_fontcache_set_colour( &renderer->cache, colour );
    float posx = text->pos.x / ( float ) renderer->window_size.width;
    float posy = 1.0f - ( text->pos.y + text->size ) / ( float ) renderer->window_size.height;
    ve_fontcache_draw_text(
        &renderer->cache,
        ( ve_font_id ) text->font_id,
        vxui_demo_to_u8( text->text ),
        posx,
        posy,
        1.0f / ( float ) renderer->window_size.width,
        1.0f / ( float ) renderer->window_size.height,
        true );
    vxui_demo_render_fontcache_drawlist( renderer, clip_rect );
}

static void vxui_demo_render_draw_list( vxui_demo_renderer* renderer, const vxui_draw_list* list )
{
    if ( !renderer || !list ) {
        return;
    }

    std::vector< vxui_rect > clip_stack;
    clip_stack.reserve( 16 );
    glDisable( GL_SCISSOR_TEST );
    glDisable( GL_FRAMEBUFFER_SRGB );

    auto apply_clip = [&]( const vxui_rect* rect ) {
        if ( !rect || rect->w <= 0.0f || rect->h <= 0.0f ) {
            glDisable( GL_SCISSOR_TEST );
            return;
        }
        GLint x = ( GLint ) std::lround( rect->x );
        GLint y = ( GLint ) std::lround( ( float ) renderer->window_size.height - rect->y - rect->h );
        GLsizei w = ( GLsizei ) std::lround( rect->w );
        GLsizei h = ( GLsizei ) std::lround( rect->h );
        glEnable( GL_SCISSOR_TEST );
        glScissor( x, y, w < 0 ? 0 : w, h < 0 ? 0 : h );
    };

    ve_fontcache_flush_drawlist( &renderer->cache );
    for ( int i = 0; i < list->length; ++i ) {
        const vxui_cmd* cmd = &list->commands[ i ];
        switch ( cmd->type ) {
            case VXUI_CMD_RECT:
                vxui_demo_draw_primitive( renderer, cmd->rect.bounds, cmd->rect.color, 0.0f, 0.0f );
                break;

            case VXUI_CMD_RECT_ROUNDED:
                vxui_demo_draw_primitive( renderer, cmd->rect_rounded.bounds, cmd->rect_rounded.color, cmd->rect_rounded.radius, 0.0f );
                break;

            case VXUI_CMD_BORDER:
                vxui_demo_draw_primitive( renderer, cmd->border.bounds, cmd->border.color, cmd->border.radius, cmd->border.width );
                break;

            case VXUI_CMD_IMAGE:
                vxui_demo_draw_placeholder_image( renderer, cmd->image.bounds );
                break;

            case VXUI_CMD_TEXT:
                vxui_demo_render_text_cmd( renderer, &cmd->text, clip_stack.empty() ? nullptr : &clip_stack.back() );
                break;

            case VXUI_CMD_CLIP_PUSH:
                clip_stack.push_back( cmd->clip.rect );
                apply_clip( &clip_stack.back() );
                break;

            case VXUI_CMD_CLIP_POP:
                if ( !clip_stack.empty() ) {
                    clip_stack.pop_back();
                }
                apply_clip( clip_stack.empty() ? nullptr : &clip_stack.back() );
                break;
        }
    }

    glDisable( GL_SCISSOR_TEST );
}

static bool vxui_demo_button_edge( vxui_demo_app* app, vxui_demo_button button, bool down )
{
    bool pressed = down && !app->button_prev[ button ];
    app->button_prev[ button ] = down;
    return pressed;
}

static bool vxui_demo_key_down( const TinyWindow::tWindow* window, int key )
{
    if ( !window || key < 0 || key > TinyWindow::last ) {
        return false;
    }
    return window->keys[ key ] == TinyWindow::keyState_t::down;
}

static bool vxui_demo_char_down( const TinyWindow::tWindow* window, char ch )
{
    unsigned char upper = ( unsigned char ) std::toupper( ( unsigned char ) ch );
    unsigned char lower = ( unsigned char ) std::tolower( ( unsigned char ) ch );
    return vxui_demo_key_down( window, upper ) || vxui_demo_key_down( window, lower );
}

static const TinyWindow::gamepad_t* vxui_demo_primary_gamepad( TinyWindow::windowManager* manager )
{
    if ( !manager ) {
        return nullptr;
    }
    for ( TinyWindow::gamepad_t* gamepad : manager->GetGamepads() ) {
        if ( gamepad ) {
            return gamepad;
        }
    }
    return nullptr;
}

static bool vxui_demo_gamepad_down( const TinyWindow::gamepad_t* gamepad, int button )
{
    return gamepad && button >= 0 && button < ( int ) gamepad->buttonStates.size() && gamepad->buttonStates[ button ];
}

int main( void )
{
    TinyWindow::windowSetting_t cfg;
    cfg.name = "VXUI";
    cfg.versionMajor = 3;
    cfg.versionMinor = 3;
    cfg.enableSRGB = true;
    cfg.resolution.width = 1280;
    cfg.resolution.height = 720;
    cfg.SetProfile( TinyWindow::profile_t::core );

    std::unique_ptr< TinyWindow::windowManager > manager( new TinyWindow::windowManager() );
    std::unique_ptr< TinyWindow::tWindow > window( manager->AddWindow( cfg ) );
    if ( !window || !window->initialized || !window->contextCreated ) {
        std::fprintf( stderr, "Failed to create the TinyWindow demo window.\n" );
        window.reset( nullptr );
        manager->ShutDown();
        return 1;
    }

    if ( !gladLoadGL() ) {
        std::fprintf( stderr, "Failed to load OpenGL symbols through glad.\n" );
        window.reset( nullptr );
        manager->ShutDown();
        return 1;
    }

    vxui_demo_renderer renderer = {};
    renderer.window_size = window->settings.resolution;
    if ( !vxui_demo_init_renderer( &renderer ) || !vxui_demo_load_fonts( &renderer ) ) {
        std::fprintf( stderr, "Failed to initialize the VXUI demo renderer.\n" );
        vxui_demo_shutdown_renderer( &renderer );
        window.reset( nullptr );
        manager->ShutDown();
        return 1;
    }

    ve_fontcache_configure_snap( &renderer.cache, renderer.window_size.width, renderer.window_size.height );

    std::vector< uint8_t > memory( ( size_t ) vxui_min_memory_size() );
    vxui_ctx ctx = {};
    vxui_init(
        &ctx,
        vxui_create_arena( ( uint64_t ) memory.size(), memory.data() ),
        ( vxui_config ) {
            .screen_width = ( int ) renderer.window_size.width,
            .screen_height = ( int ) renderer.window_size.height,
            .max_elements = 256,
            .max_anim_states = 256,
            .max_sequences = 16,
        } );
    vxui_set_fontcache( &ctx, &renderer.cache );
    ctx.default_font_id = VXUI_DEMO_FONT_UI;
    ctx.default_font_size = 24.0f;
    ctx.default_text_color = ( vxui_color ) { 242, 244, 255, 255 };

    vxui_demo_app app = {};
    app.ctx = &ctx;
    app.volume = 0.40f;
    app.watched_seq_path = vxui_demo_make_temp_path( "vxui_demo_sequence.toml" );
    app.last_selected_seq = -1;
    vxui_set_text_fn( &ctx, vxui_demo_text, &app );

    vxui_set_locale_font( &ctx, "en", VXUI_DEMO_FONT_UI );
    vxui_set_locale_font( &ctx, "ja", VXUI_DEMO_FONT_JAPANESE );
    vxui_set_locale_font( &ctx, "ar", VXUI_DEMO_FONT_ARABIC );
    vxui_set_locale( &ctx, "en" );

    const char* difficulty_keys[] = { "difficulty.easy", "difficulty.normal", "difficulty.hard" };
    const char* slot_keys[] = { "slot.0", "slot.1", "slot.2", "slot.3", "slot.4", "slot.5", "slot.6", "slot.7" };
#ifdef VXUI_DEBUG
    const char* prop_labels[] = { "opacity", "scale", "slide_x", "slide_y" };
#endif
    const vxui_input_table keyboard_table = {
        .confirm = { VXUI_DEMO_FONT_UI, 'E' },
        .cancel = { VXUI_DEMO_FONT_UI, 'Q' },
        .tab_left = { VXUI_DEMO_FONT_UI, '[' },
        .tab_right = { VXUI_DEMO_FONT_UI, ']' },
        .up = { VXUI_DEMO_FONT_UI, '^' },
        .down = { VXUI_DEMO_FONT_UI, 'v' },
        .left = { VXUI_DEMO_FONT_UI, '<' },
        .right = { VXUI_DEMO_FONT_UI, '>' },
    };
    const vxui_input_table gamepad_table = {
        .confirm = { VXUI_DEMO_FONT_DEBUG, 'A' },
        .cancel = { VXUI_DEMO_FONT_DEBUG, 'B' },
        .tab_left = { VXUI_DEMO_FONT_DEBUG, 'L' },
        .tab_right = { VXUI_DEMO_FONT_DEBUG, 'R' },
        .up = { VXUI_DEMO_FONT_DEBUG, '^' },
        .down = { VXUI_DEMO_FONT_DEBUG, 'v' },
        .left = { VXUI_DEMO_FONT_DEBUG, '<' },
        .right = { VXUI_DEMO_FONT_DEBUG, '>' },
    };
    vxui_set_input_table( &ctx, &keyboard_table );

    if ( !vxui_demo_write_file(
             app.watched_seq_path.c_str(),
             "[sequence.main_menu_enter]\n"
             "steps = [\n"
             "  { delay = 0, id = \"main_menu\", prop = \"opacity\", target = 0.35 },\n"
             "  { delay = 100, id = \"main_menu\", prop = \"opacity\", target = 1.0 },\n"
             "]\n" ) ) {
        std::fprintf( stderr, "Failed to create the watched sequence file at %s.\n", app.watched_seq_path.c_str() );
        vxui_demo_shutdown_renderer( &renderer );
        window.reset( nullptr );
        manager->ShutDown();
        return 1;
    }

    char error[ 256 ] = {};
    if ( !vxui_load_seq_toml( &ctx, app.watched_seq_path.c_str(), "main_menu_enter", error, sizeof( error ) ) ) {
        std::fprintf( stderr, "Failed to load %s: %s\n", app.watched_seq_path.c_str(), error );
        std::remove( app.watched_seq_path.c_str() );
        vxui_demo_shutdown_renderer( &renderer );
        window.reset( nullptr );
        manager->ShutDown();
        return 1;
    }
#ifdef VXUI_DEBUG
    if ( !vxui_watch_seq_file( &ctx, app.watched_seq_path.c_str(), "main_menu_enter" ) ) {
        std::fprintf( stderr, "Failed to watch %s for hot reload.\n", app.watched_seq_path.c_str() );
        std::remove( app.watched_seq_path.c_str() );
        vxui_demo_shutdown_renderer( &renderer );
        window.reset( nullptr );
        manager->ShutDown();
        return 1;
    }
    ctx.debug_seq_editor.selected_seq = vxui_demo_find_seq_index( &ctx, "main_menu_enter" );
    vxui_debug_generate_seq_outputs( &ctx );
    vxui_demo_sync_step_editor( &app );
#endif

    vxui_seq_step settings_enter[] = {
        { 0, vxui_id( "settings" ), VXUI_PROP_SLIDE_X, 28.0f },
        { 120, vxui_id( "settings" ), VXUI_PROP_SLIDE_X, 0.0f },
    };
    vxui_seq_step settings_exit[] = {
        { 0, vxui_id( "settings" ), VXUI_PROP_OPACITY, 0.4f },
        { 0, vxui_id( "settings" ), VXUI_PROP_SLIDE_X, -20.0f },
    };
    vxui_register_seq( &ctx, "settings_enter", settings_enter, ( int ) ( sizeof( settings_enter ) / sizeof( settings_enter[ 0 ] ) ) );
    vxui_register_seq( &ctx, "settings_exit", settings_exit, ( int ) ( sizeof( settings_exit ) / sizeof( settings_exit[ 0 ] ) ) );
    vxui_push_screen( &ctx, "main_menu" );
    vxui_demo_refresh_status( &app );

    std::chrono::steady_clock::time_point previous = std::chrono::steady_clock::now();
    while ( !window->shouldClose ) {
        manager->PollForEvents();
        renderer.window_size = window->settings.resolution;
        if ( renderer.window_size.width == 0 ) renderer.window_size.width = 1;
        if ( renderer.window_size.height == 0 ) renderer.window_size.height = 1;
        ctx.cfg.screen_width = ( int ) renderer.window_size.width;
        ctx.cfg.screen_height = ( int ) renderer.window_size.height;
        ve_fontcache_configure_snap( &renderer.cache, renderer.window_size.width, renderer.window_size.height );

        std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
        float dt = std::chrono::duration< float >( now - previous ).count();
        previous = now;
        if ( dt <= 0.0f || dt > 0.1f ) {
            dt = 1.0f / 60.0f;
        }
        uint64_t now_ms = ( uint64_t ) std::chrono::duration_cast< std::chrono::milliseconds >( now.time_since_epoch() ).count();

#ifdef VXUI_DEBUG
        if ( ctx.debug_seq_editor.selected_seq != app.last_selected_seq ) {
            vxui_demo_sync_step_editor( &app );
            vxui_debug_generate_seq_outputs( &ctx );
        }
#endif
        vxui_demo_refresh_status( &app );

        vxui_begin( &ctx, dt );
        const TinyWindow::gamepad_t* gamepad = vxui_demo_primary_gamepad( manager.get() );
        if ( vxui_demo_button_edge( &app, VXUI_DEMO_BTN_NAV_UP, vxui_demo_key_down( window.get(), TinyWindow::arrowUp ) || vxui_demo_gamepad_down( gamepad, TinyWindow::gamepad_t::Dpad_top ) ) ) {
            vxui_input_nav( &ctx, VXUI_DIR_UP );
        }
        if ( vxui_demo_button_edge( &app, VXUI_DEMO_BTN_NAV_DOWN, vxui_demo_key_down( window.get(), TinyWindow::arrowDown ) || vxui_demo_gamepad_down( gamepad, TinyWindow::gamepad_t::Dpad_bottom ) ) ) {
            vxui_input_nav( &ctx, VXUI_DIR_DOWN );
        }
        if ( vxui_demo_button_edge( &app, VXUI_DEMO_BTN_NAV_LEFT, vxui_demo_key_down( window.get(), TinyWindow::arrowLeft ) || vxui_demo_gamepad_down( gamepad, TinyWindow::gamepad_t::Dpad_left ) ) ) {
            vxui_input_nav( &ctx, VXUI_DIR_LEFT );
        }
        if ( vxui_demo_button_edge( &app, VXUI_DEMO_BTN_NAV_RIGHT, vxui_demo_key_down( window.get(), TinyWindow::arrowRight ) || vxui_demo_gamepad_down( gamepad, TinyWindow::gamepad_t::Dpad_right ) ) ) {
            vxui_input_nav( &ctx, VXUI_DIR_RIGHT );
        }
        if ( vxui_demo_button_edge( &app, VXUI_DEMO_BTN_CONFIRM, vxui_demo_key_down( window.get(), TinyWindow::enter ) || vxui_demo_key_down( window.get(), TinyWindow::spacebar ) || vxui_demo_gamepad_down( gamepad, TinyWindow::gamepad_t::face_bottom ) ) ) {
            vxui_input_confirm( &ctx );
        }
        if ( vxui_demo_button_edge( &app, VXUI_DEMO_BTN_CANCEL, vxui_demo_key_down( window.get(), TinyWindow::escape ) || vxui_demo_key_down( window.get(), TinyWindow::backspace ) || vxui_demo_gamepad_down( gamepad, TinyWindow::gamepad_t::face_right ) ) ) {
            vxui_input_cancel( &ctx );
            if ( app.show_settings ) {
                vxui_demo_close_settings( &ctx, 0, &app );
            }
        }
        if ( vxui_demo_button_edge( &app, VXUI_DEMO_BTN_TAB_LEFT, vxui_demo_char_down( window.get(), '[' ) || vxui_demo_gamepad_down( gamepad, TinyWindow::gamepad_t::left_shoulder ) ) ) {
            vxui_input_tab( &ctx, -1 );
        }
        if ( vxui_demo_button_edge( &app, VXUI_DEMO_BTN_TAB_RIGHT, vxui_demo_char_down( window.get(), ']' ) || vxui_demo_gamepad_down( gamepad, TinyWindow::gamepad_t::right_shoulder ) ) ) {
            vxui_input_tab( &ctx, 1 );
        }
        if ( vxui_demo_button_edge( &app, VXUI_DEMO_BTN_LOCALE_EN, vxui_demo_char_down( window.get(), '1' ) ) ) {
            app.locale_index = 0;
            vxui_set_locale( &ctx, "en" );
        }
        if ( vxui_demo_button_edge( &app, VXUI_DEMO_BTN_LOCALE_JA, vxui_demo_char_down( window.get(), '2' ) ) ) {
            app.locale_index = 1;
            vxui_set_locale( &ctx, "ja-JP" );
        }
        if ( vxui_demo_button_edge( &app, VXUI_DEMO_BTN_LOCALE_AR, vxui_demo_char_down( window.get(), '3' ) ) ) {
            app.locale_index = 2;
            vxui_set_locale( &ctx, "ar" );
        }
        if ( vxui_demo_button_edge( &app, VXUI_DEMO_BTN_PROMPT_KEYBOARD, vxui_demo_char_down( window.get(), '4' ) ) ) {
            app.prompt_table_index = 0;
            vxui_set_input_table( &ctx, &keyboard_table );
        }
        if ( vxui_demo_button_edge( &app, VXUI_DEMO_BTN_PROMPT_GAMEPAD, vxui_demo_char_down( window.get(), '5' ) ) ) {
            app.prompt_table_index = 1;
            vxui_set_input_table( &ctx, &gamepad_table );
        }
#ifdef VXUI_DEBUG
        if ( vxui_demo_button_edge( &app, VXUI_DEMO_BTN_DEBUG_TOGGLE, vxui_demo_key_down( window.get(), TinyWindow::F1 ) ) ) {
            ctx.debug_seq_editor.open = !ctx.debug_seq_editor.open;
            ctx.debug_seq_editor.preview_playing = false;
        }
        if ( vxui_demo_button_edge( &app, VXUI_DEMO_BTN_DEBUG_RELOAD, vxui_demo_char_down( window.get(), 'R' ) ) ) {
            app.reload_status[ 0 ] = '\0';
            char reload_error[ 256 ] = {};
            if ( vxui_poll_seq_hot_reload( &ctx, now_ms, reload_error, sizeof( reload_error ) ) ) {
                std::snprintf( app.reload_status, sizeof( app.reload_status ), "%s", "Reloaded the watched sequence file." );
                vxui_demo_sync_step_editor( &app );
                vxui_debug_generate_seq_outputs( &ctx );
            } else if ( reload_error[ 0 ] ) {
                std::snprintf( app.reload_status, sizeof( app.reload_status ), "Reload failed: %s", reload_error );
            } else {
                std::snprintf( app.reload_status, sizeof( app.reload_status ), "%s", "No watched sequence change was detected." );
            }
        }
        if ( vxui_demo_button_edge( &app, VXUI_DEMO_BTN_DEBUG_COPY, vxui_demo_char_down( window.get(), 'C' ) ) ) {
            app.copied_to_clipboard = vxui_demo_copy_to_clipboard( ctx.debug_seq_editor.generated_toml );
            if ( !app.copied_to_clipboard ) {
                if ( ctx.debug_seq_editor.generated_c[ 0 ] ) {
                    std::puts( ctx.debug_seq_editor.generated_c );
                }
                if ( ctx.debug_seq_editor.generated_toml[ 0 ] ) {
                    std::puts( ctx.debug_seq_editor.generated_toml );
                }
            }
        }
#endif

#ifdef VXUI_DEBUG
        if ( ctx.debug_seq_editor.open ) {
            vxui_demo_apply_step_editor( &app );
            vxui_debug_generate_seq_outputs( &ctx );
        } else {
            ctx.debug_seq_editor.preview_playing = false;
        }
#endif
        vxui_demo_refresh_status( &app );

        if ( app.show_settings ) {
            VXUI( &ctx, "settings", {
                .layout = {
                    .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_GROW( 0 ) },
                    .padding = CLAY_PADDING_ALL( 18 ),
                    .childGap = 12,
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                },
                .backgroundColor = { 11, 18, 34, 255 },
            } ) {
                VXUI_LABEL( &ctx, "menu.settings", ( vxui_label_cfg ) {
                    .font_id = app.locale_index == 0 ? VXUI_DEMO_FONT_TITLE : 0u,
                    .font_size = 44.0f,
                    .color = { 255, 247, 225, 255 },
                } );
                VXUI_TRAIT( VXUI_TRAIT_SPIN, ( vxui_demo_spin ) { .speed = 2.0f, .padding = 6.0f } );

                VXUI( &ctx, "settings.row.difficulty", {
                    .layout = {
                        .childGap = 16,
                        .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                        .layoutDirection = CLAY_LEFT_TO_RIGHT,
                    },
                } ) {
                    VXUI_LABEL( &ctx, "menu.difficulty", ( vxui_label_cfg ) { 0 } );
                    VXUI_OPTION( &ctx, "settings.difficulty", &app.difficulty, difficulty_keys, 3, ( vxui_option_cfg ) { 0 } );
                }

                VXUI( &ctx, "settings.row.volume", {
                    .layout = {
                        .childGap = 16,
                        .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                        .layoutDirection = CLAY_LEFT_TO_RIGHT,
                    },
                } ) {
                    VXUI_LABEL( &ctx, "menu.volume", ( vxui_label_cfg ) { 0 } );
                    VXUI_SLIDER( &ctx, "settings.volume", &app.volume, 0.0f, 1.0f, ( vxui_slider_cfg ) {
                        .show_value = true,
                    } );
                }

                VXUI_LABEL( &ctx, "menu.saves", ( vxui_label_cfg ) { 0 } );
                VXUI_LIST_BEGIN( &ctx, "settings.saves", ( vxui_list_cfg ) {
                    .max_visible = 6,
                    .item_height = 22.0f,
                } ) {
                    for ( int i = 0; i < 8; ++i ) {
                        VXUI_LIST_ITEM( &ctx, i ) {
                            VXUI_LABEL( &ctx, slot_keys[ i ], ( vxui_label_cfg ) { 0 } );
                        }
                    }
                }

                VXUI( &ctx, "settings.prompts", {
                    .layout = {
                        .childGap = 8,
                        .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                        .layoutDirection = CLAY_LEFT_TO_RIGHT,
                    },
                } ) {
                    VXUI_PROMPT( &ctx, "action.confirm" );
                    VXUI_LABEL( &ctx, "menu.confirm", ( vxui_label_cfg ) { 0 } );
                    VXUI_PROMPT( &ctx, "action.cancel" );
                    VXUI_LABEL( &ctx, "menu.cancel", ( vxui_label_cfg ) { 0 } );
                }

                VXUI_LABEL( &ctx, "status.locale", ( vxui_label_cfg ) { 0 } );
                VXUI_LABEL( &ctx, "status.prompts", ( vxui_label_cfg ) { 0 } );
                VXUI_LABEL( &ctx, "status.screens", ( vxui_label_cfg ) { 0 } );

                VXUI_ACTION( &ctx, "settings.back", "menu.back", vxui_demo_close_settings, ( vxui_action_cfg ) {
                    .userdata = &app,
                } );
                VXUI_TRAIT( VXUI_TRAIT_GLOW, ( vxui_demo_glow ) { .padding = 6.0f, .alpha = 0.25f } );
            }
        } else {
            VXUI( &ctx, "main_menu", {
                .layout = {
                    .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_GROW( 0 ) },
                    .padding = CLAY_PADDING_ALL( 18 ),
                    .childGap = 16,
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                },
                .backgroundColor = { 8, 12, 22, 255 },
            } ) {
                VXUI_LABEL( &ctx, "menu.main", ( vxui_label_cfg ) {
                    .font_id = app.locale_index == 0 ? VXUI_DEMO_FONT_TITLE : 0u,
                    .font_size = 44.0f,
                    .color = { 255, 247, 225, 255 },
                } );
                VXUI_TRAIT( VXUI_TRAIT_PULSE, ( vxui_demo_pulse ) { .speed = 2.0f, .scale = 0.05f, .alpha = 0.14f } );

                VXUI_ACTION( &ctx, "main.settings", "menu.open_settings", vxui_demo_open_settings, ( vxui_action_cfg ) {
                    .userdata = &app,
                } );
                VXUI_TRAIT( VXUI_TRAIT_GLOW, ( vxui_demo_glow ) { .padding = 6.0f, .alpha = 0.20f } );
                VXUI_TRAIT( VXUI_TRAIT_SCANLINE, ( vxui_demo_scanline ) { .spacing = 6.0f, .alpha = 0.10f } );

                VXUI( &ctx, "main.prompts", {
                    .layout = {
                        .childGap = 8,
                        .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                        .layoutDirection = CLAY_LEFT_TO_RIGHT,
                    },
                } ) {
                    VXUI_PROMPT( &ctx, "action.confirm" );
                    VXUI_LABEL( &ctx, "menu.confirm", ( vxui_label_cfg ) { 0 } );
                    VXUI_PROMPT( &ctx, "action.cancel" );
                    VXUI_LABEL( &ctx, "menu.cancel", ( vxui_label_cfg ) { 0 } );
                }

                VXUI_LABEL( &ctx, "status.locale", ( vxui_label_cfg ) { 0 } );
                VXUI_LABEL( &ctx, "status.prompts", ( vxui_label_cfg ) { 0 } );
                VXUI_LABEL( &ctx, "status.screens", ( vxui_label_cfg ) { 0 } );
                VXUI_LABEL( &ctx, "hint.controls.0", ( vxui_label_cfg ) { 0 } );
                VXUI_LABEL( &ctx, "hint.controls.1", ( vxui_label_cfg ) { 0 } );
                VXUI_LABEL( &ctx, "hint.controls.2", ( vxui_label_cfg ) { 0 } );
                VXUI_LABEL( &ctx, "hint.controls.3", ( vxui_label_cfg ) { 0 } );
            }
        }

#ifdef VXUI_DEBUG
        if ( ctx.debug_seq_editor.open ) {
            const char* seq_names[ 16 ] = {};
            int seq_count = ctx.registered_seq_count;
            if ( seq_count > 16 ) {
                seq_count = 16;
            }
            for ( int i = 0; i < seq_count; ++i ) {
                seq_names[ i ] = ctx.registered_seqs[ i ].name;
            }

            VXUI( &ctx, "debug.overlay", {
                .layout = {
                    .padding = CLAY_PADDING_ALL( 12 ),
                    .childGap = 8,
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                },
                .backgroundColor = { 22, 24, 38, 224 },
            } ) {
                VXUI_LABEL( &ctx, "debug.overlay.title", ( vxui_label_cfg ) {
                    .font_id = VXUI_DEMO_FONT_TITLE,
                    .font_size = 28.0f,
                    .color = { 255, 214, 153, 255 },
                } );
                VXUI_LABEL( &ctx, "menu.debug", ( vxui_label_cfg ) { 0 } );
                if ( seq_count > 0 ) {
                    VXUI_OPTION( &ctx, "debug.selected.seq", &ctx.debug_seq_editor.selected_seq, seq_names, seq_count, ( vxui_option_cfg ) { 0 } );
                    VXUI_LABEL( &ctx, "debug.selected_seq", ( vxui_label_cfg ) { 0 } );
                    VXUI_LABEL( &ctx, "debug.step.id", ( vxui_label_cfg ) { 0 } );
                    VXUI_OPTION( &ctx, "debug.step.prop", &app.debug_prop_index, prop_labels, 4, ( vxui_option_cfg ) { 0 } );
                    VXUI_SLIDER( &ctx, "debug.step.delay", &app.debug_delay_value, 0.0f, 200.0f, ( vxui_slider_cfg ) {
                        .show_value = true,
                        .format = "%.0f",
                    } );
                    VXUI_SLIDER( &ctx, "debug.step.target", &app.debug_target_value, -32.0f, 32.0f, ( vxui_slider_cfg ) {
                        .show_value = true,
                        .format = "%.2f",
                    } );
                }
                VXUI_LABEL( &ctx, "debug.preview_status", ( vxui_label_cfg ) { 0 } );
                VXUI_LABEL( &ctx, "debug.reload", ( vxui_label_cfg ) { 0 } );
                VXUI_LABEL( &ctx, "debug.clipboard", ( vxui_label_cfg ) { 0 } );
                VXUI_LABEL( &ctx, "debug.generated_c", ( vxui_label_cfg ) {
                    .font_id = VXUI_DEMO_FONT_DEBUG,
                    .font_size = 16.0f,
                    .color = { 184, 220, 255, 255 },
                } );
                VXUI_LABEL( &ctx, "debug.generated_toml", ( vxui_label_cfg ) {
                    .font_id = VXUI_DEMO_FONT_DEBUG,
                    .font_size = 16.0f,
                    .color = { 184, 255, 208, 255 },
                } );
            }
        }
#endif

        vxui_draw_list list = vxui_end( &ctx );
#ifdef VXUI_DEBUG
        if ( ctx.debug_seq_editor.open ) {
            vxui_debug_capture_preview( &ctx, &list );
            ctx.debug_seq_editor.preview_playing = ctx.debug_seq_editor.preview_snapshot.command_count > 0;
            vxui_demo_refresh_status( &app );
        }
#endif

        glBindFramebuffer( GL_FRAMEBUFFER, 0 );
        glViewport( 0, 0, renderer.window_size.width, renderer.window_size.height );
        glDisable( GL_SCISSOR_TEST );
        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        glDisable( GL_FRAMEBUFFER_SRGB );
        glClearColor( 0.05f, 0.06f, 0.10f, 1.0f );
        glClear( GL_COLOR_BUFFER_BIT );

        vxui_demo_render_draw_list( &renderer, &list );
        vxui_flush_text( &ctx );
        window->SwapDrawBuffers();
    }

    Clay_SetCurrentContext( nullptr );
    std::remove( app.watched_seq_path.c_str() );
    vxui_demo_shutdown_renderer( &renderer );
    manager->ShutDown();
    window.reset( nullptr );
    return 0;
}
