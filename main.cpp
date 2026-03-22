#include <cassert>
#include <cstring>
#include <string>
#include <vector>

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
} vxui_demo_app;

static const char* vxui_demo_text( const char* key, void* userdata )
{
    ( void ) userdata;

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
    if ( std::strcmp( key, "difficulty.easy" ) == 0 ) {
        return "Easy";
    }
    if ( std::strcmp( key, "difficulty.normal" ) == 0 ) {
        return "Normal";
    }
    if ( std::strcmp( key, "difficulty.hard" ) == 0 ) {
        return "Hard";
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

static vxui_list_state* vxui_demo_find_list_state( vxui_ctx* ctx, uint32_t id )
{
    for ( int i = 0; i < ctx->list_state_count; ++i ) {
        if ( ctx->list_states[ i ].id == id ) {
            return &ctx->list_states[ i ];
        }
    }
    return nullptr;
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
            .max_elements = 128,
            .max_anim_states = 128,
            .max_sequences = 16,
        } );

    ve_fontcache cache = {};
    ve_fontcache_init( &cache, false );
    vxui_set_fontcache( &ctx, &cache );
    vxui_set_text_fn( &ctx, vxui_demo_text, nullptr );

    const char* difficulty_keys[] = { "difficulty.easy", "difficulty.normal", "difficulty.hard" };
    const char* slot_keys[] = { "slot.0", "slot.1", "slot.2", "slot.3", "slot.4", "slot.5", "slot.6", "slot.7" };
    vxui_input_table gamepad_table = {
        .confirm = { 3, 'A' },
        .cancel = { 4, 'B' },
    };
    vxui_seq_step main_enter[] = {
        { 0, vxui_id( "main_menu" ), VXUI_PROP_OPACITY, 0.35f },
        { 100, vxui_id( "main_menu" ), VXUI_PROP_OPACITY, 1.0f },
    };
    vxui_seq_step settings_enter[] = {
        { 0, vxui_id( "settings" ), VXUI_PROP_SLIDE_X, 28.0f },
        { 120, vxui_id( "settings" ), VXUI_PROP_SLIDE_X, 0.0f },
    };
    vxui_seq_step settings_exit[] = {
        { 0, vxui_id( "settings" ), VXUI_PROP_OPACITY, 0.4f },
        { 0, vxui_id( "settings" ), VXUI_PROP_SLIDE_X, -20.0f },
    };

    vxui_register_seq( &ctx, "main_menu_enter", main_enter, ( int ) ( sizeof( main_enter ) / sizeof( main_enter[ 0 ] ) ) );
    vxui_register_seq( &ctx, "settings_enter", settings_enter, ( int ) ( sizeof( settings_enter ) / sizeof( settings_enter[ 0 ] ) ) );
    vxui_register_seq( &ctx, "settings_exit", settings_exit, ( int ) ( sizeof( settings_exit ) / sizeof( settings_exit[ 0 ] ) ) );

    vxui_demo_app app = {
        .show_settings = false,
        .difficulty = 0,
        .volume = 0.40f,
    };
    vxui_push_screen( &ctx, "main_menu" );

    for ( int frame = 0; frame < 5; ++frame ) {
        vxui_begin( &ctx, 1.0f / 60.0f );
        vxui_set_input_table( &ctx, frame == 2 ? &gamepad_table : nullptr );

        if ( frame == 1 ) {
            vxui_set_focus( &ctx, vxui_id( "main.settings" ) );
            vxui_input_confirm( &ctx );
        } else if ( frame == 3 ) {
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

                VXUI( &ctx, "settings.row.difficulty", {
                    .layout = {
                        .childGap = 16,
                        .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                    },
                } ) {
                    VXUI_LABEL( &ctx, "menu.difficulty", ( vxui_label_cfg ) { 0 } );
                    VXUI_OPTION( &ctx, "settings.difficulty", &app.difficulty, difficulty_keys, 3, ( vxui_option_cfg ) { 0 } );
                }

                VXUI( &ctx, "settings.row.volume", {
                    .layout = {
                        .childGap = 16,
                        .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
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
                VXUI_ACTION( &ctx, "main.settings", "menu.open_settings", vxui_demo_open_settings, ( vxui_action_cfg ) {
                    .userdata = &app,
                } );
            }
        }

        vxui_draw_list list = vxui_end( &ctx );
        assert( list.length > 0 );

        if ( frame == 0 ) {
            assert( vxui_seq_playing( &ctx, "main_menu_enter" ) );
        }
        if ( frame == 1 ) {
            assert( app.show_settings );
            assert( ctx.screen_count == 2 );
            assert( ctx.screens[ 1 ].state == VXUI_SCREEN_ENTERING );
            assert( vxui_seq_playing( &ctx, "settings_enter" ) );
        }
        if ( frame == 2 ) {
            assert( vxui_demo_list_has_text( &list, "Settings" ) );
            bool saw_a = false;
            bool saw_b = false;
            for ( int i = 0; i < ctx.text_queue_count; ++i ) {
                if ( ctx.text_queue[ i ].font_id == 3 && std::strcmp( ctx.text_queue[ i ].text, "A" ) == 0 ) {
                    saw_a = true;
                }
                if ( ctx.text_queue[ i ].font_id == 4 && std::strcmp( ctx.text_queue[ i ].text, "B" ) == 0 ) {
                    saw_b = true;
                }
            }
            assert( saw_a );
            assert( saw_b );
        }
        if ( frame == 3 ) {
            assert( !app.show_settings );
            assert( ctx.screens[ 1 ].state == VXUI_SCREEN_EXITING );
            assert( vxui_seq_playing( &ctx, "settings_exit" ) );
        }
        if ( frame == 4 ) {
            assert( vxui_demo_list_has_text( &list, "Main Menu" ) );
            assert( vxui_demo_list_has_text( &list, "Settings" ) );
            vxui_list_state* saves = vxui_demo_find_list_state( &ctx, vxui_id( "settings.saves" ) );
            assert( saves == nullptr || saves->focused_index >= 0 );
        }

        for ( int i = 0; i < list.length; ++i ) {
            if ( list.commands[ i ].type == VXUI_CMD_TEXT ) {
                vxui_flush_text( &ctx );
            }
        }
    }

    ve_fontcache_shutdown( &cache );
    return 0;
}
