#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#if defined( _WIN32 )
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#ifdef near
#undef near
#endif
#ifdef far
#undef far
#endif
#endif

#define CLAY_IMPLEMENTATION
#include "clay/clay.h"

#define VE_FONTCACHE_IMPL
#include "vefc/ve_fontcache.h"

#define VXUI_IMPL
#include "vxui.h"

typedef struct vxui_demo_app
{
    bool show_settings;
    int difficulty;
    float volume;
    int locale_index;
    vxui_ctx* ctx;
    const char* watched_seq_path;
    bool copied_to_clipboard;
    int debug_prop_index;
    float debug_delay_value;
    float debug_target_value;
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

static const char* vxui_demo_text( const char* key, void* userdata )
{
    vxui_demo_app* app = ( vxui_demo_app* ) userdata;

    if ( app->ctx ) {
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
        if ( std::strcmp( key, "debug.preview_status" ) == 0 ) {
            return app->ctx->debug_seq_editor.preview_playing ? "Preview active" : "Preview idle";
        }
        if ( std::strcmp( key, "debug.generated_c" ) == 0 ) {
            return app->ctx->debug_seq_editor.generated_c[ 0 ] ? app->ctx->debug_seq_editor.generated_c : "(no generated c)";
        }
        if ( std::strcmp( key, "debug.generated_toml" ) == 0 ) {
            return app->ctx->debug_seq_editor.generated_toml[ 0 ] ? app->ctx->debug_seq_editor.generated_toml : "(no generated toml)";
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
        if ( std::strcmp( key, "debug.clipboard" ) == 0 ) {
            return app->copied_to_clipboard ? "Clipboard updated" : "Printed to stdout";
        }
#endif
    }

    if ( app->locale_index == 1 ) {
        if ( std::strcmp( key, "menu.main" ) == 0 ) {
            return "メインメニュー";
        }
        if ( std::strcmp( key, "menu.settings" ) == 0 ) {
            return "設定";
        }
        if ( std::strcmp( key, "menu.open_settings" ) == 0 ) {
            return "設定";
        }
        if ( std::strcmp( key, "menu.back" ) == 0 ) {
            return "戻る";
        }
        if ( std::strcmp( key, "menu.difficulty" ) == 0 ) {
            return "難易度";
        }
        if ( std::strcmp( key, "menu.volume" ) == 0 ) {
            return "音量";
        }
        if ( std::strcmp( key, "menu.saves" ) == 0 ) {
            return "セーブ";
        }
        if ( std::strcmp( key, "menu.confirm" ) == 0 ) {
            return "決定";
        }
        if ( std::strcmp( key, "menu.cancel" ) == 0 ) {
            return "戻る";
        }
        if ( std::strcmp( key, "menu.debug" ) == 0 ) {
            return "デバッグ";
        }
        if ( std::strcmp( key, "difficulty.easy" ) == 0 ) {
            return "かんたん";
        }
        if ( std::strcmp( key, "difficulty.normal" ) == 0 ) {
            return "ふつう";
        }
        if ( std::strcmp( key, "difficulty.hard" ) == 0 ) {
            return "むずかしい";
        }
    } else if ( app->locale_index == 2 ) {
        if ( std::strcmp( key, "menu.main" ) == 0 ) {
            return "القائمة الرئيسية";
        }
        if ( std::strcmp( key, "menu.settings" ) == 0 ) {
            return "الإعدادات";
        }
        if ( std::strcmp( key, "menu.open_settings" ) == 0 ) {
            return "الإعدادات";
        }
        if ( std::strcmp( key, "menu.back" ) == 0 ) {
            return "رجوع";
        }
        if ( std::strcmp( key, "menu.difficulty" ) == 0 ) {
            return "الصعوبة";
        }
        if ( std::strcmp( key, "menu.volume" ) == 0 ) {
            return "الصوت";
        }
        if ( std::strcmp( key, "menu.saves" ) == 0 ) {
            return "الحفظ";
        }
        if ( std::strcmp( key, "menu.confirm" ) == 0 ) {
            return "تأكيد";
        }
        if ( std::strcmp( key, "menu.cancel" ) == 0 ) {
            return "إلغاء";
        }
        if ( std::strcmp( key, "menu.debug" ) == 0 ) {
            return "التصحيح";
        }
        if ( std::strcmp( key, "difficulty.easy" ) == 0 ) {
            return "سهل";
        }
        if ( std::strcmp( key, "difficulty.normal" ) == 0 ) {
            return "عادي";
        }
        if ( std::strcmp( key, "difficulty.hard" ) == 0 ) {
            return "صعب";
        }
    } else {
        if ( std::strcmp( key, "menu.main" ) == 0 ) {
            return "Main Menu";
        }
        if ( std::strcmp( key, "menu.settings" ) == 0 ) {
            return "Settings";
        }
        if ( std::strcmp( key, "menu.open_settings" ) == 0 ) {
            return "Settings";
        }
        if ( std::strcmp( key, "menu.back" ) == 0 ) {
            return "Back";
        }
        if ( std::strcmp( key, "menu.difficulty" ) == 0 ) {
            return "Difficulty";
        }
        if ( std::strcmp( key, "menu.volume" ) == 0 ) {
            return "Volume";
        }
        if ( std::strcmp( key, "menu.saves" ) == 0 ) {
            return "Save Slots";
        }
        if ( std::strcmp( key, "menu.confirm" ) == 0 ) {
            return "Confirm";
        }
        if ( std::strcmp( key, "menu.cancel" ) == 0 ) {
            return "Cancel";
        }
        if ( std::strcmp( key, "menu.debug" ) == 0 ) {
            return "Debug Overlay";
        }
        if ( std::strcmp( key, "difficulty.easy" ) == 0 ) {
            return "Easy";
        }
        if ( std::strcmp( key, "difficulty.normal" ) == 0 ) {
            return "Normal";
        }
        if ( std::strcmp( key, "difficulty.hard" ) == 0 ) {
            return "Hard";
        }
    }

    if ( std::strcmp( key, "slot.0" ) == 0 ) {
        return "Autosave";
    }
    if ( std::strcmp( key, "slot.1" ) == 0 ) {
        return "Profile 1";
    }
    if ( std::strcmp( key, "slot.2" ) == 0 ) {
        return "Profile 2";
    }
    if ( std::strcmp( key, "slot.3" ) == 0 ) {
        return "Profile 3";
    }
    if ( std::strcmp( key, "slot.4" ) == 0 ) {
        return "Profile 4";
    }
    if ( std::strcmp( key, "slot.5" ) == 0 ) {
        return "Profile 5";
    }
    if ( std::strcmp( key, "slot.6" ) == 0 ) {
        return "Profile 6";
    }
    if ( std::strcmp( key, "slot.7" ) == 0 ) {
        return "Profile 7";
    }

    return key;
}

static bool vxui_demo_list_has_text( const vxui_draw_list* list, const char* text )
{
    for ( int i = 0; i < list->length; ++i ) {
        if ( list->commands[ i ].type == VXUI_CMD_TEXT && list->commands[ i ].text.text && std::strcmp( list->commands[ i ].text.text, text ) == 0 ) {
            return true;
        }
    }
    return false;
}

static float vxui_demo_find_text_x( const vxui_draw_list* list, const char* text )
{
    for ( int i = 0; i < list->length; ++i ) {
        if ( list->commands[ i ].type == VXUI_CMD_TEXT && list->commands[ i ].text.text && std::strcmp( list->commands[ i ].text.text, text ) == 0 ) {
            return list->commands[ i ].text.pos.x;
        }
    }
    return -1.0f;
}

static bool vxui_demo_write_file( const char* path, const char* text )
{
    FILE* fp = std::fopen( path, "wb" );
    if ( !fp ) {
        return false;
    }
    size_t length = std::strlen( text );
    bool ok = std::fwrite( text, 1, length, fp ) == length;
    std::fclose( fp );
    return ok;
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
        case VXUI_PROP_OPACITY:
            app->debug_prop_index = 0;
            break;
        case VXUI_PROP_SCALE:
            app->debug_prop_index = 1;
            break;
        case VXUI_PROP_SLIDE_X:
            app->debug_prop_index = 2;
            break;
        case VXUI_PROP_SLIDE_Y:
            app->debug_prop_index = 3;
            break;
    }
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

static void vxui_demo_open_settings( vxui_ctx* ctx, uint32_t id, void* userdata )
{
    ( void ) id;
    vxui_demo_app* app = ( vxui_demo_app* ) userdata;
    app->show_settings = true;
    vxui_push_screen( ctx, "settings" );
}

static void vxui_demo_close_settings( vxui_ctx* ctx, uint32_t id, void* userdata )
{
    ( void ) id;
    vxui_demo_app* app = ( vxui_demo_app* ) userdata;
    app->show_settings = false;
    vxui_pop_screen( ctx );
}

int main( void )
{
    std::vector< uint8_t > memory( ( size_t ) vxui_min_memory_size() );
    vxui_ctx ctx = {};
    vxui_init(
        &ctx,
        vxui_create_arena( ( uint64_t ) memory.size(), memory.data() ),
        ( vxui_config ) {
            .screen_width = 640,
            .screen_height = 360,
            .max_elements = 192,
            .max_anim_states = 192,
            .max_sequences = 16,
        } );

    ve_fontcache cache = {};
    ve_fontcache_init( &cache, false );
    vxui_set_fontcache( &ctx, &cache );

    vxui_demo_app app = {
        .show_settings = false,
        .difficulty = 0,
        .volume = 0.40f,
        .locale_index = 0,
        .ctx = &ctx,
        .watched_seq_path = "tmp\\vxui_demo_sequence.toml",
        .copied_to_clipboard = false,
        .debug_prop_index = 0,
        .debug_delay_value = 0.0f,
        .debug_target_value = 1.0f,
    };
    vxui_set_text_fn( &ctx, vxui_demo_text, &app );

    vxui_set_locale_font( &ctx, "en", 1 );
    vxui_set_locale_font( &ctx, "ja", 7 );
    vxui_set_locale_font( &ctx, "ar", 9 );

    const char* difficulty_keys[] = { "difficulty.easy", "difficulty.normal", "difficulty.hard" };
    const char* slot_keys[] = { "slot.0", "slot.1", "slot.2", "slot.3", "slot.4", "slot.5", "slot.6", "slot.7" };
    const char* prop_labels[] = { "opacity", "scale", "slide_x", "slide_y" };
    vxui_input_table keyboard_table = {
        .confirm = { 5, 'E' },
        .cancel = { 6, 'Q' },
        .tab_left = { 0, '[' },
        .tab_right = { 0, ']' },
        .up = { 0, '^' },
        .down = { 0, 'v' },
        .left = { 0, '<' },
        .right = { 0, '>' },
    };
    vxui_input_table gamepad_table = {
        .confirm = { 3, 'A' },
        .cancel = { 4, 'B' },
        .tab_left = { 0, 'L' },
        .tab_right = { 0, 'R' },
        .up = { 0, '^' },
        .down = { 0, 'v' },
        .left = { 0, '<' },
        .right = { 0, '>' },
    };
    vxui_seq_step settings_enter[] = {
        { 0, vxui_id( "settings" ), VXUI_PROP_SLIDE_X, 28.0f },
        { 120, vxui_id( "settings" ), VXUI_PROP_SLIDE_X, 0.0f },
    };
    vxui_seq_step settings_exit[] = {
        { 0, vxui_id( "settings" ), VXUI_PROP_OPACITY, 0.4f },
        { 0, vxui_id( "settings" ), VXUI_PROP_SLIDE_X, -20.0f },
    };

    assert( vxui_demo_write_file(
        app.watched_seq_path,
        "[sequence.main_menu_enter]\n"
        "steps = [\n"
        "  { delay = 0, id = \"main_menu\", prop = \"opacity\", target = 0.35 },\n"
        "  { delay = 100, id = \"main_menu\", prop = \"opacity\", target = 1.0 },\n"
        "]\n" ) );
    assert( vxui_load_seq_toml( &ctx, app.watched_seq_path, "main_menu_enter", nullptr, 0 ) );
#ifdef VXUI_DEBUG
    assert( vxui_watch_seq_file( &ctx, app.watched_seq_path, "main_menu_enter" ) );
    ctx.debug_seq_editor.selected_seq = vxui_demo_find_seq_index( &ctx, "main_menu_enter" );
    assert( ctx.debug_seq_editor.selected_seq >= 0 );
    vxui_debug_generate_seq_outputs( &ctx );
    vxui_demo_sync_step_editor( &app );
#endif
    vxui_register_seq( &ctx, "settings_enter", settings_enter, ( int ) ( sizeof( settings_enter ) / sizeof( settings_enter[ 0 ] ) ) );
    vxui_register_seq( &ctx, "settings_exit", settings_exit, ( int ) ( sizeof( settings_exit ) / sizeof( settings_exit[ 0 ] ) ) );

    vxui_push_screen( &ctx, "main_menu" );
    float prompt_a_ltr_x = -1.0f;
    float prompt_b_ltr_x = -1.0f;
    uint64_t debug_clock_ms = 0;

    for ( int frame = 0; frame < 8; ++frame ) {
        debug_clock_ms += 100;

        if ( frame == 1 || frame == 2 ) {
            app.locale_index = 1;
            vxui_set_locale( &ctx, "ja-JP" );
            vxui_set_input_table( &ctx, frame == 2 ? &gamepad_table : &keyboard_table );
        } else if ( frame >= 3 ) {
            app.locale_index = 2;
            vxui_set_locale( &ctx, "ar" );
            vxui_set_input_table( &ctx, frame == 3 ? &gamepad_table : &keyboard_table );
        } else {
            app.locale_index = 0;
            vxui_set_locale( &ctx, "en" );
            vxui_set_input_table( &ctx, &keyboard_table );
        }

#ifdef VXUI_DEBUG
        if ( frame == 2 ) {
            ctx.debug_seq_editor.open = true;
            ctx.debug_seq_editor.preview_playing = false;
            app.debug_prop_index = 2;
            app.debug_delay_value = 40.0f;
            app.debug_target_value = -12.0f;
        }
        if ( frame == 4 ) {
            assert( vxui_demo_write_file(
                app.watched_seq_path,
                "[sequence.main_menu_enter]\n"
                "steps = [\n"
                "  { delay = 0, id = \"main_menu\", prop = \"opacity\", target = 0.65 },\n"
                "  { delay = 100, id = \"main_menu\", prop = \"opacity\", target = 1.0 },\n"
                "]\n" ) );
            ctx.watched_seq_files[ 0 ].last_write_time = 1;
            char reload_error[ 256 ] = {};
            assert( vxui_poll_seq_hot_reload( &ctx, debug_clock_ms, reload_error, sizeof( reload_error ) ) );
            vxui_demo_sync_step_editor( &app );
            vxui_debug_generate_seq_outputs( &ctx );
            assert( std::strstr( ctx.debug_seq_editor.generated_toml, "target = 0.650" ) != nullptr );
            std::puts( ctx.debug_seq_editor.generated_toml );
            app.copied_to_clipboard = vxui_demo_copy_to_clipboard( ctx.debug_seq_editor.generated_toml );
        }
#endif

        vxui_begin( &ctx, 1.0f / 60.0f );
        if ( frame == 1 ) {
            vxui_set_focus( &ctx, vxui_id( "main.settings" ) );
            vxui_input_confirm( &ctx );
        } else if ( frame == 5 ) {
            vxui_set_focus( &ctx, vxui_id( "settings.back" ) );
            vxui_input_confirm( &ctx );
        }

        if ( app.show_settings ) {
            VXUI( &ctx, "settings", {
                .layout = {
                    .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_GROW( 0 ) },
                    .padding = CLAY_PADDING_ALL( 16 ),
                    .childGap = 12,
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                },
                .backgroundColor = { 10, 16, 32, 255 },
            } ) {
                VXUI_LABEL( &ctx, "menu.settings", ( vxui_label_cfg ) {
                    .font_size = 42.0f,
                    .color = { 255, 255, 255, 255 },
                } );
                VXUI_TRAIT( VXUI_TRAIT_SPIN, ( vxui_demo_spin ) { .speed = 2.0f, .padding = 4.0f } );

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
                    .item_height = 20.0f,
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

                VXUI_ACTION( &ctx, "settings.back", "menu.back", vxui_demo_close_settings, ( vxui_action_cfg ) {
                    .userdata = &app,
                } );
                VXUI_TRAIT( VXUI_TRAIT_GLOW, ( vxui_demo_glow ) { .padding = 4.0f, .alpha = 0.25f } );
            }
        } else {
            VXUI( &ctx, "main_menu", {
                .layout = {
                    .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_GROW( 0 ) },
                    .padding = CLAY_PADDING_ALL( 16 ),
                    .childGap = 16,
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                },
                .backgroundColor = { 8, 12, 24, 255 },
            } ) {
                VXUI_LABEL( &ctx, "menu.main", ( vxui_label_cfg ) {
                    .font_size = 44.0f,
                    .color = { 255, 255, 255, 255 },
                } );
                VXUI_TRAIT( VXUI_TRAIT_PULSE, ( vxui_demo_pulse ) { .speed = 2.0f, .scale = 0.06f, .alpha = 0.15f } );
                VXUI_ACTION( &ctx, "main.settings", "menu.open_settings", vxui_demo_open_settings, ( vxui_action_cfg ) {
                    .userdata = &app,
                } );
                VXUI_TRAIT( VXUI_TRAIT_GLOW, ( vxui_demo_glow ) { .padding = 4.0f, .alpha = 0.2f } );
                VXUI_TRAIT( VXUI_TRAIT_SCANLINE, ( vxui_demo_scanline ) { .spacing = 6.0f, .alpha = 0.1f } );
            }
        }

#ifdef VXUI_DEBUG
        if ( ctx.debug_seq_editor.open ) {
            vxui_demo_apply_step_editor( &app );
            vxui_debug_generate_seq_outputs( &ctx );

            VXUI( &ctx, "debug.overlay", {
                .layout = {
                    .padding = CLAY_PADDING_ALL( 12 ),
                    .childGap = 8,
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                },
                .backgroundColor = { 24, 24, 36, 220 },
            } ) {
                VXUI_LABEL( &ctx, "debug.overlay.title", ( vxui_label_cfg ) {
                    .font_size = 24.0f,
                    .color = { 255, 220, 160, 255 },
                } );
                VXUI_LABEL( &ctx, "menu.debug", ( vxui_label_cfg ) { 0 } );
                for ( int i = 0; i < ctx.registered_seq_count; ++i ) {
                    VXUI_LABEL( &ctx, ctx.registered_seqs[ i ].name, ( vxui_label_cfg ) { 0 } );
                }
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
                VXUI_LABEL( &ctx, "debug.preview_status", ( vxui_label_cfg ) { 0 } );
                VXUI_LABEL( &ctx, "debug.clipboard", ( vxui_label_cfg ) { 0 } );
                VXUI_LABEL( &ctx, "debug.generated_c", ( vxui_label_cfg ) {
                    .font_size = 14.0f,
                    .color = { 180, 220, 255, 255 },
                } );
                VXUI_LABEL( &ctx, "debug.generated_toml", ( vxui_label_cfg ) {
                    .font_size = 14.0f,
                    .color = { 180, 255, 200, 255 },
                } );
            }
        }
#endif

        vxui_draw_list list = vxui_end( &ctx );
        assert( list.length > 0 );

#ifdef VXUI_DEBUG
        if ( ctx.debug_seq_editor.open ) {
            vxui_debug_capture_preview( &ctx, &list );
            ctx.debug_seq_editor.preview_playing = true;
            assert( ctx.debug_seq_editor.preview_snapshot.command_count > 0 );
        }
#endif

        if ( frame == 0 ) {
            assert( vxui_demo_list_has_text( &list, "Main Menu" ) );
            assert( ctx.text_queue[ 0 ].font_id == 1u );
        }
        if ( frame == 1 ) {
            assert( vxui_demo_list_has_text( &list, "メインメニュー" ) );
            assert( ctx.text_queue[ 0 ].font_id == 7u );
            assert( app.show_settings );
        }
        if ( frame == 2 ) {
            bool saw_a = false;
            bool saw_b = false;
            prompt_a_ltr_x = vxui_demo_find_text_x( &list, "A" );
            prompt_b_ltr_x = vxui_demo_find_text_x( &list, "B" );
            for ( int i = 0; i < ctx.text_queue_count; ++i ) {
                if ( ctx.text_queue[ i ].font_id == 3 && std::strcmp( ctx.text_queue[ i ].text, "A" ) == 0 ) {
                    saw_a = true;
                }
                if ( ctx.text_queue[ i ].font_id == 4 && std::strcmp( ctx.text_queue[ i ].text, "B" ) == 0 ) {
                    saw_b = true;
                }
            }
            assert( vxui_demo_list_has_text( &list, "設定" ) );
            assert( saw_a );
            assert( saw_b );
#ifdef VXUI_DEBUG
            assert( vxui_demo_list_has_text( &list, "Sequence Debugger" ) );
            assert( std::strstr( ctx.debug_seq_editor.generated_c, "VXUI_PROP_SLIDE_X" ) != nullptr );
#endif
        }
        if ( frame == 3 ) {
            float prompt_a_rtl_x = vxui_demo_find_text_x( &list, "A" );
            float prompt_b_rtl_x = vxui_demo_find_text_x( &list, "B" );
            assert( ctx.rtl );
            assert( ctx.text_queue[ 0 ].font_id == 9u );
            assert( prompt_a_ltr_x != prompt_a_rtl_x || prompt_b_ltr_x != prompt_b_rtl_x );
        }
        if ( frame == 4 ) {
#ifdef VXUI_DEBUG
            assert( std::strstr( ctx.debug_seq_editor.generated_toml, "target = 0.650" ) != nullptr );
            assert( ctx.debug_seq_editor.preview_playing );
#endif
        }
        if ( frame == 5 ) {
            assert( !app.show_settings );
            assert( ctx.screens[ 1 ].state == VXUI_SCREEN_EXITING );
        }
        if ( frame == 6 ) {
            assert( vxui_demo_list_has_text( &list, "القائمة الرئيسية" ) );
            assert( vxui_demo_list_has_text( &list, "Sequence Debugger" ) );
        }
        if ( frame == 7 ) {
            const vxui_registered_seq* seq = vxui_find_seq( &ctx, "main_menu_enter" );
            assert( seq != nullptr );
            assert( std::fabs( seq->steps[ 0 ].target - 0.65f ) < 0.0001f );
        }

        for ( int i = 0; i < list.length; ++i ) {
            if ( list.commands[ i ].type == VXUI_CMD_TEXT ) {
                vxui_flush_text( &ctx );
            }
        }
    }

    std::remove( app.watched_seq_path );
    ve_fontcache_shutdown( &cache );
    return 0;
}
