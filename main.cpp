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

static const char* vxui_demo_text( const char* key, void* userdata )
{
    ( void ) userdata;

    if ( std::strcmp( key, "menu.title" ) == 0 ) {
        return "VXUI Settings";
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

    int difficulty = 0;
    float volume = 0.40f;
    uint32_t difficulty_id = vxui_id( "difficulty" );
    uint32_t volume_id = vxui_id( "volume" );
    uint32_t saves_id = vxui_id( "save-list" );

    for ( int frame = 0; frame < 5; ++frame ) {
        vxui_begin( &ctx, 1.0f / 60.0f );
        vxui_set_input_table( &ctx, frame == 4 ? &gamepad_table : nullptr );

        if ( frame == 1 ) {
            vxui_set_focus( &ctx, difficulty_id );
            vxui_input_nav( &ctx, VXUI_DIR_RIGHT );
        } else if ( frame == 2 ) {
            vxui_set_focus( &ctx, volume_id );
            vxui_input_nav( &ctx, VXUI_DIR_RIGHT );
        } else if ( frame == 3 ) {
            vxui_set_focus( &ctx, saves_id );
            vxui_input_nav( &ctx, VXUI_DIR_DOWN );
        }

        VXUI( &ctx, "root", {
            .layout = {
                .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_GROW( 0 ) },
                .padding = CLAY_PADDING_ALL( 16 ),
                .childGap = 12,
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
            .backgroundColor = { 8, 12, 24, 255 },
        } ) {
            VXUI_LABEL( &ctx, "menu.title", ( vxui_label_cfg ) {
                .font_size = 42.0f,
                .color = { 255, 255, 255, 255 },
            } );

            VXUI( &ctx, "row.difficulty", {
                .layout = {
                    .childGap = 16,
                    .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                },
            } ) {
                VXUI_LABEL( &ctx, "menu.difficulty", ( vxui_label_cfg ) { 0 } );
                VXUI_OPTION( &ctx, "difficulty", &difficulty, difficulty_keys, 3, ( vxui_option_cfg ) { 0 } );
            }

            VXUI( &ctx, "row.volume", {
                .layout = {
                    .childGap = 16,
                    .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                },
            } ) {
                VXUI_LABEL( &ctx, "menu.volume", ( vxui_label_cfg ) { 0 } );
                VXUI_SLIDER( &ctx, "volume", &volume, 0.0f, 1.0f, ( vxui_slider_cfg ) {
                    .show_value = true,
                } );
            }

            VXUI_LABEL( &ctx, "menu.saves", ( vxui_label_cfg ) { 0 } );
            VXUI_LIST_BEGIN( &ctx, "save-list", ( vxui_list_cfg ) {
                .max_visible = 6,
                .item_height = 20.0f,
            } ) {
                for ( int i = 0; i < 8; ++i ) {
                    VXUI_LIST_ITEM( &ctx, i ) {
                        VXUI_LABEL( &ctx, slot_keys[ i ], ( vxui_label_cfg ) { 0 } );
                    }
                }
            }

            VXUI( &ctx, "row.prompts", {
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
        }

        vxui_draw_list list = vxui_end( &ctx );
        assert( list.length > 0 );
        assert( list.commands[ list.length - 1 ].type == VXUI_CMD_BORDER );

        if ( frame == 0 ) {
            assert( vxui_focused_id( &ctx ) == difficulty_id );
        }
        if ( frame == 1 ) {
            assert( difficulty == 1 );
        }
        if ( frame == 2 ) {
            assert( volume > 0.40f );
        }
        if ( frame == 3 ) {
            vxui_list_state* state = vxui_demo_find_list_state( &ctx, saves_id );
            assert( state != nullptr );
            assert( state->focused_index == 1 );
        }
        if ( frame == 4 ) {
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

        for ( int i = 0; i < list.length; ++i ) {
            if ( list.commands[ i ].type == VXUI_CMD_TEXT ) {
                vxui_flush_text( &ctx );
            }
        }
    }

    ve_fontcache_shutdown( &cache );
    return 0;
}
