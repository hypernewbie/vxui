#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "../third_party/utest.h"
#include "../vxui.h"

typedef struct advanced_primitives_fixture
{
    uint8_t* memory;
    uint64_t memory_size;
    vxui_ctx ctx;
    int option_change_count;
    float slider_change_value;
} advanced_primitives_fixture;

static const char* vxui__advanced_text_fn( const char* key, void* userdata )
{
    ( void ) userdata;

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
        return "Slot 0";
    }
    if ( std::strcmp( key, "slot.1" ) == 0 ) {
        return "Slot 1";
    }
    if ( std::strcmp( key, "slot.2" ) == 0 ) {
        return "Slot 2";
    }
    if ( std::strcmp( key, "slot.3" ) == 0 ) {
        return "Slot 3";
    }
    if ( std::strcmp( key, "slot.4" ) == 0 ) {
        return "Slot 4";
    }
    if ( std::strcmp( key, "slot.5" ) == 0 ) {
        return "Slot 5";
    }
    if ( std::strcmp( key, "slot.6" ) == 0 ) {
        return "Slot 6";
    }
    if ( std::strcmp( key, "slot.7" ) == 0 ) {
        return "Slot 7";
    }
    if ( std::strcmp( key, "action.next" ) == 0 ) {
        return "Next";
    }

    return key;
}

static void vxui__advanced_option_changed( vxui_ctx* ctx, int value, void* userdata )
{
    ( void ) ctx;
    ( void ) value;
    int* count = ( int* ) userdata;
    *count += 1;
}

static void vxui__advanced_slider_changed( vxui_ctx* ctx, float value, void* userdata )
{
    ( void ) ctx;
    float* out_value = ( float* ) userdata;
    *out_value = value;
}

static vxui_list_state* vxui__advanced_find_list_state( vxui_ctx* ctx, uint32_t id )
{
    for ( int i = 0; i < ctx->list_state_count; ++i ) {
        if ( ctx->list_states[ i ].id == id ) {
            return &ctx->list_states[ i ];
        }
    }
    return nullptr;
}

UTEST_F_SETUP( advanced_primitives_fixture )
{
    utest_fixture->memory_size = vxui_min_memory_size();
    utest_fixture->memory = ( uint8_t* ) std::malloc( ( size_t ) utest_fixture->memory_size );
    ASSERT_TRUE( utest_fixture->memory != nullptr );

    utest_fixture->option_change_count = 0;
    utest_fixture->slider_change_value = 0.0f;

    vxui_init(
        &utest_fixture->ctx,
        vxui_create_arena( utest_fixture->memory_size, utest_fixture->memory ),
        ( vxui_config ) {
            .screen_width = 640,
            .screen_height = 360,
            .max_elements = 128,
            .max_anim_states = 128,
        } );
    vxui_set_text_fn( &utest_fixture->ctx, vxui__advanced_text_fn, nullptr );
}

UTEST_F_TEARDOWN( advanced_primitives_fixture )
{
    Clay_SetCurrentContext( nullptr );
    std::free( utest_fixture->memory );
    utest_fixture->memory = nullptr;
    utest_fixture->memory_size = 0;
}

UTEST_F( advanced_primitives_fixture, option_wraps_at_both_ends )
{
    const char* difficulty_keys[] = { "difficulty.easy", "difficulty.normal", "difficulty.hard" };
    int index = 2;

    vxui_begin( &utest_fixture->ctx, 0.016f );
    vxui_set_focus( &utest_fixture->ctx, vxui_id( "difficulty" ) );
    vxui_input_nav( &utest_fixture->ctx, VXUI_DIR_RIGHT );
    VXUI( &utest_fixture->ctx, "root", {} ) {
        VXUI_OPTION( &utest_fixture->ctx, "difficulty", &index, difficulty_keys, 3, ( vxui_option_cfg ) { 0 } );
    }
    vxui_end( &utest_fixture->ctx );
    EXPECT_EQ( index, 0 );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    vxui_set_focus( &utest_fixture->ctx, vxui_id( "difficulty" ) );
    vxui_input_nav( &utest_fixture->ctx, VXUI_DIR_LEFT );
    VXUI( &utest_fixture->ctx, "root", {} ) {
        VXUI_OPTION( &utest_fixture->ctx, "difficulty", &index, difficulty_keys, 3, ( vxui_option_cfg ) { 0 } );
    }
    vxui_end( &utest_fixture->ctx );
    EXPECT_EQ( index, 2 );
}

UTEST_F( advanced_primitives_fixture, option_change_callback_fires_once_per_successful_mutation )
{
    const char* difficulty_keys[] = { "difficulty.easy", "difficulty.normal", "difficulty.hard" };
    int index = 1;

    vxui_begin( &utest_fixture->ctx, 0.016f );
    vxui_set_focus( &utest_fixture->ctx, vxui_id( "difficulty" ) );
    vxui_input_nav( &utest_fixture->ctx, VXUI_DIR_RIGHT );
    VXUI( &utest_fixture->ctx, "root", {} ) {
        VXUI_OPTION( &utest_fixture->ctx, "difficulty", &index, difficulty_keys, 3, ( vxui_option_cfg ) {
            .on_change = vxui__advanced_option_changed,
            .userdata = &utest_fixture->option_change_count,
        } );
    }
    vxui_end( &utest_fixture->ctx );

    EXPECT_EQ( index, 2 );
    EXPECT_EQ( utest_fixture->option_change_count, 1 );
}

UTEST_F( advanced_primitives_fixture, slider_clamps_min_and_max )
{
    float value = 0.95f;

    vxui_begin( &utest_fixture->ctx, 0.016f );
    vxui_set_focus( &utest_fixture->ctx, vxui_id( "volume" ) );
    vxui_input_nav( &utest_fixture->ctx, VXUI_DIR_RIGHT );
    VXUI( &utest_fixture->ctx, "root", {} ) {
        VXUI_SLIDER( &utest_fixture->ctx, "volume", &value, 0.0f, 1.0f, ( vxui_slider_cfg ) {
            .step = 0.5f,
            .on_change = vxui__advanced_slider_changed,
            .userdata = &utest_fixture->slider_change_value,
        } );
    }
    vxui_end( &utest_fixture->ctx );
    EXPECT_TRUE( std::fabs( value - 1.0f ) < 0.0001f );
    EXPECT_TRUE( std::fabs( utest_fixture->slider_change_value - 1.0f ) < 0.0001f );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    vxui_set_focus( &utest_fixture->ctx, vxui_id( "volume" ) );
    vxui_input_nav( &utest_fixture->ctx, VXUI_DIR_LEFT );
    VXUI( &utest_fixture->ctx, "root", {} ) {
        VXUI_SLIDER( &utest_fixture->ctx, "volume", &value, 0.0f, 1.0f, ( vxui_slider_cfg ) {
            .step = 2.0f,
        } );
    }
    vxui_end( &utest_fixture->ctx );
    EXPECT_TRUE( std::fabs( value - 0.0f ) < 0.0001f );
}

UTEST_F( advanced_primitives_fixture, slider_uses_default_step_when_zero )
{
    float value = 0.0f;

    vxui_begin( &utest_fixture->ctx, 0.016f );
    vxui_set_focus( &utest_fixture->ctx, vxui_id( "volume" ) );
    vxui_input_nav( &utest_fixture->ctx, VXUI_DIR_RIGHT );
    VXUI( &utest_fixture->ctx, "root", {} ) {
        VXUI_SLIDER( &utest_fixture->ctx, "volume", &value, 0.0f, 1.0f, ( vxui_slider_cfg ) { 0 } );
    }
    vxui_end( &utest_fixture->ctx );

    EXPECT_TRUE( std::fabs( value - 0.05f ) < 0.0001f );
}

UTEST_F( advanced_primitives_fixture, list_state_allocates_and_persists_by_id )
{
    uint32_t list_id = vxui_id( "saves" );
    const char* slot_keys[] = { "slot.0", "slot.1", "slot.2", "slot.3", "slot.4", "slot.5", "slot.6", "slot.7" };

    for ( int frame = 0; frame < 7; ++frame ) {
        vxui_begin( &utest_fixture->ctx, 0.016f );
        vxui_set_focus( &utest_fixture->ctx, list_id );
        vxui_input_nav( &utest_fixture->ctx, VXUI_DIR_DOWN );
        VXUI( &utest_fixture->ctx, "root", {} ) {
            VXUI_LIST_BEGIN( &utest_fixture->ctx, "saves", ( vxui_list_cfg ) {
                .max_visible = 6,
                .item_height = 10.0f,
            } ) {
                for ( int i = 0; i < 8; ++i ) {
                    VXUI_LIST_ITEM( &utest_fixture->ctx, i ) {
                        VXUI_LABEL( &utest_fixture->ctx, slot_keys[ i ], ( vxui_label_cfg ) { 0 } );
                    }
                }
            }
        }
        vxui_end( &utest_fixture->ctx );
    }

    vxui_list_state* state = vxui__advanced_find_list_state( &utest_fixture->ctx, list_id );
    ASSERT_TRUE( state != nullptr );
    EXPECT_EQ( state->focused_index, 7 );
    EXPECT_TRUE( std::fabs( state->scroll_target - 20.0f ) < 0.0001f );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    vxui_set_focus( &utest_fixture->ctx, list_id );
    VXUI( &utest_fixture->ctx, "root", {} ) {
        VXUI_LIST_BEGIN( &utest_fixture->ctx, "saves", ( vxui_list_cfg ) {
            .max_visible = 6,
            .item_height = 10.0f,
        } ) {
            for ( int i = 0; i < 8; ++i ) {
                VXUI_LIST_ITEM( &utest_fixture->ctx, i ) {
                    VXUI_LABEL( &utest_fixture->ctx, slot_keys[ i ], ( vxui_label_cfg ) { 0 } );
                }
            }
        }
    }
    vxui_end( &utest_fixture->ctx );

    state = vxui__advanced_find_list_state( &utest_fixture->ctx, list_id );
    ASSERT_TRUE( state != nullptr );
    EXPECT_EQ( state->focused_index, 7 );
    EXPECT_TRUE( state->scroll_current > 0.0f );
}

UTEST_F( advanced_primitives_fixture, nested_slider_consumes_left_right_before_list )
{
    float value = 0.5f;
    uint32_t list_id = vxui_id( "saves" );
    uint32_t slider_id = vxui_id( "inner.volume" );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    vxui_set_focus( &utest_fixture->ctx, slider_id );
    vxui_input_nav( &utest_fixture->ctx, VXUI_DIR_LEFT );
    VXUI( &utest_fixture->ctx, "root", {} ) {
        VXUI_LIST_BEGIN( &utest_fixture->ctx, "saves", ( vxui_list_cfg ) {
            .max_visible = 3,
            .item_height = 18.0f,
        } ) {
            VXUI_LIST_ITEM( &utest_fixture->ctx, 0 ) {
                VXUI_SLIDER( &utest_fixture->ctx, "inner.volume", &value, 0.0f, 1.0f, ( vxui_slider_cfg ) {
                    .step = 0.25f,
                } );
            }
        }
    }
    vxui_end( &utest_fixture->ctx );

    vxui_list_state* state = vxui__advanced_find_list_state( &utest_fixture->ctx, list_id );
    ASSERT_TRUE( state != nullptr );
    EXPECT_TRUE( std::fabs( value - 0.25f ) < 0.0001f );
    EXPECT_EQ( state->focused_index, 0 );
    EXPECT_EQ( vxui_focused_id( &utest_fixture->ctx ), slider_id );
}

UTEST_F( advanced_primitives_fixture, list_bubbles_out_only_at_edges )
{
    uint32_t list_id = vxui_id( "saves" );
    uint32_t next_id = vxui_id( "next" );
    const char* slot_keys[] = { "slot.0", "slot.1", "slot.2" };

    vxui_begin( &utest_fixture->ctx, 0.016f );
    vxui_set_focus( &utest_fixture->ctx, list_id );
    vxui_input_nav( &utest_fixture->ctx, VXUI_DIR_DOWN );
    VXUI( &utest_fixture->ctx, "root", {
        .layout = {
            .childGap = 12,
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
    } ) {
        VXUI_LIST_BEGIN( &utest_fixture->ctx, "saves", ( vxui_list_cfg ) {
            .max_visible = 3,
            .item_height = 18.0f,
        } ) {
            for ( int i = 0; i < 3; ++i ) {
                VXUI_LIST_ITEM( &utest_fixture->ctx, i ) {
                    VXUI_LABEL( &utest_fixture->ctx, slot_keys[ i ], ( vxui_label_cfg ) { 0 } );
                }
            }
        }
        VXUI_ACTION( &utest_fixture->ctx, "next", "action.next", nullptr, ( vxui_action_cfg ) { 0 } );
    }
    vxui_end( &utest_fixture->ctx );

    vxui_list_state* state = vxui__advanced_find_list_state( &utest_fixture->ctx, list_id );
    ASSERT_TRUE( state != nullptr );
    EXPECT_EQ( state->focused_index, 1 );
    EXPECT_EQ( vxui_focused_id( &utest_fixture->ctx ), list_id );

    state->focused_index = 2;

    vxui_begin( &utest_fixture->ctx, 0.016f );
    vxui_set_focus( &utest_fixture->ctx, list_id );
    vxui_input_nav( &utest_fixture->ctx, VXUI_DIR_DOWN );
    VXUI( &utest_fixture->ctx, "root", {
        .layout = {
            .childGap = 12,
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
    } ) {
        VXUI_LIST_BEGIN( &utest_fixture->ctx, "saves", ( vxui_list_cfg ) {
            .max_visible = 3,
            .item_height = 18.0f,
        } ) {
            for ( int i = 0; i < 3; ++i ) {
                VXUI_LIST_ITEM( &utest_fixture->ctx, i ) {
                    VXUI_LABEL( &utest_fixture->ctx, slot_keys[ i ], ( vxui_label_cfg ) { 0 } );
                }
            }
        }
        VXUI_ACTION( &utest_fixture->ctx, "next", "action.next", nullptr, ( vxui_action_cfg ) { 0 } );
    }
    vxui_end( &utest_fixture->ctx );

    EXPECT_EQ( vxui_focused_id( &utest_fixture->ctx ), next_id );
}

UTEST_F( advanced_primitives_fixture, prompt_resolves_existing_binding )
{
    vxui_input_table table = {
        .confirm = {
            .font_id = 7,
            .glyph = 'A',
        },
    };
    vxui_set_input_table( &utest_fixture->ctx, &table );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {} ) {
        VXUI_PROMPT( &utest_fixture->ctx, "action.confirm" );
    }
    vxui_end( &utest_fixture->ctx );

    ASSERT_TRUE( utest_fixture->ctx.text_queue_count >= 1 );
    EXPECT_EQ( utest_fixture->ctx.text_queue[ 0 ].font_id, 7u );
    EXPECT_STREQ( utest_fixture->ctx.text_queue[ 0 ].text, "A" );
}

UTEST_F( advanced_primitives_fixture, missing_prompt_binding_is_safe )
{
    vxui_input_table table = {};
    vxui_set_input_table( &utest_fixture->ctx, &table );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {} ) {
        VXUI_PROMPT( &utest_fixture->ctx, "action.confirm" );
    }
    vxui_end( &utest_fixture->ctx );

    EXPECT_EQ( utest_fixture->ctx.text_queue_count, 0 );
}
