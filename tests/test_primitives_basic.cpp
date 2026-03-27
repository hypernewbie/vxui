#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "../third_party/utest.h"
#include "../vxui.h"

typedef struct primitives_fixture
{
    uint8_t* memory;
    uint64_t memory_size;
    vxui_ctx ctx;
} primitives_fixture;

static char vxui__primitive_text_buffer[ 64 ];

static const char* vxui__primitive_text_fn( const char* key, void* userdata )
{
    ( void ) userdata;

    if ( std::strcmp( key, "menu.title" ) == 0 ) {
        return "Main Menu";
    }
    if ( std::strcmp( key, "menu.start" ) == 0 ) {
        return "Start Game";
    }
    if ( std::strcmp( key, "menu.volume" ) == 0 ) {
        return "Volume";
    }
    if ( std::strcmp( key, "menu.status" ) == 0 ) {
        return "Status";
    }
    if ( std::strcmp( key, "menu.ready" ) == 0 ) {
        return "Ready";
    }
    if ( std::strcmp( key, "menu.hint" ) == 0 ) {
        return "Press Start";
    }
    if ( std::strcmp( key, "menu.quit" ) == 0 ) {
        return "Quit";
    }
    if ( std::strcmp( key, "menu.dynamic" ) == 0 ) {
        return vxui__primitive_text_buffer;
    }
    if ( std::strcmp( key, "menu.wrap.long" ) == 0 ) {
        return "This is a deliberately long localized hint string.\nIt should emit more than one text row.\nThose rows should keep increasing Y positions.";
    }
    return nullptr;
}

static void vxui__test_action( vxui_ctx* ctx, uint32_t id, void* userdata )
{
    ( void ) ctx;
    ( void ) id;
    ( void ) userdata;
}

static int vxui__collect_text_command_positions( const vxui_draw_list* list, float* ys, int capacity )
{
    int count = 0;
    for ( int i = 0; i < list->length && count < capacity; ++i ) {
        if ( list->commands[ i ].type != VXUI_CMD_TEXT ) {
            continue;
        }
        ys[ count++ ] = list->commands[ i ].text.pos.y;
    }
    return count;
}

UTEST_F_SETUP( primitives_fixture )
{
    utest_fixture->memory_size = vxui_min_memory_size();
    utest_fixture->memory = ( uint8_t* ) std::malloc( ( size_t ) utest_fixture->memory_size );
    ASSERT_TRUE( utest_fixture->memory != nullptr );

    vxui_init(
        &utest_fixture->ctx,
        vxui_create_arena( utest_fixture->memory_size, utest_fixture->memory ),
        ( vxui_config ) {
            .screen_width = 640,
            .screen_height = 360,
            .max_elements = 64,
        } );
}

UTEST_F_TEARDOWN( primitives_fixture )
{
    vxui_shutdown( &utest_fixture->ctx );
    std::free( utest_fixture->memory );
    utest_fixture->memory = nullptr;
    utest_fixture->memory_size = 0;
}

UTEST_F( primitives_fixture, label_resolves_through_localisation_callback )
{
    vxui_set_text_fn( &utest_fixture->ctx, vxui__primitive_text_fn, nullptr );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {} ) {
        VXUI_LABEL( &utest_fixture->ctx, "menu.start", ( vxui_label_cfg ) { 0 } );
    }

    vxui_end( &utest_fixture->ctx );
    EXPECT_EQ( utest_fixture->ctx.text_queue_count, 1 );
    EXPECT_STREQ( utest_fixture->ctx.text_queue[ 0 ].text, "Start Game" );
}

UTEST_F( primitives_fixture, label_falls_back_to_key )
{
    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {} ) {
        VXUI_LABEL( &utest_fixture->ctx, "menu.start", ( vxui_label_cfg ) { 0 } );
    }

    vxui_end( &utest_fixture->ctx );
    EXPECT_EQ( utest_fixture->ctx.text_queue_count, 1 );
    EXPECT_STREQ( utest_fixture->ctx.text_queue[ 0 ].text, "menu.start" );
}

UTEST_F( primitives_fixture, value_formats_integers_correctly )
{
    vxui_set_text_fn( &utest_fixture->ctx, vxui__primitive_text_fn, nullptr );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {} ) {
        VXUI_VALUE( &utest_fixture->ctx, "menu.volume", 7.0f, ( vxui_value_cfg ) {
            .format = "%.0f",
        } );
    }

    vxui_end( &utest_fixture->ctx );
    EXPECT_EQ( utest_fixture->ctx.text_queue_count, 2 );
    EXPECT_STREQ( utest_fixture->ctx.text_queue[ 1 ].text, "7" );
}

UTEST_F( primitives_fixture, value_formats_floats_correctly )
{
    vxui_set_text_fn( &utest_fixture->ctx, vxui__primitive_text_fn, nullptr );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {} ) {
        VXUI_VALUE( &utest_fixture->ctx, "menu.volume", 0.85f, ( vxui_value_cfg ) {
            .format = "%.2f",
        } );
    }

    vxui_end( &utest_fixture->ctx );
    EXPECT_EQ( utest_fixture->ctx.text_queue_count, 2 );
    EXPECT_STREQ( utest_fixture->ctx.text_queue[ 1 ].text, "0.85" );
}

UTEST_F( primitives_fixture, action_registers_focusable_declaration )
{
    vxui_set_text_fn( &utest_fixture->ctx, vxui__primitive_text_fn, nullptr );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {} ) {
        VXUI_ACTION( &utest_fixture->ctx, "start", "menu.start", vxui__test_action, ( vxui_action_cfg ) { 0 } );
    }

    vxui_end( &utest_fixture->ctx );
    ASSERT_TRUE( utest_fixture->ctx.decl_count >= 1 );
    EXPECT_EQ( utest_fixture->ctx.decls[ 0 ].kind, VXUI_DECL_ACTION );
    EXPECT_TRUE( utest_fixture->ctx.decls[ 0 ].focusable );
    EXPECT_EQ( utest_fixture->ctx.decls[ 0 ].id, vxui_id( "start" ) );
}

UTEST_F( primitives_fixture, disabled_action_is_not_focusable )
{
    vxui_set_text_fn( &utest_fixture->ctx, vxui__primitive_text_fn, nullptr );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {} ) {
        VXUI_ACTION( &utest_fixture->ctx, "quit", "menu.quit", vxui__test_action, ( vxui_action_cfg ) {
            .disabled = true,
        } );
    }

    vxui_end( &utest_fixture->ctx );
    ASSERT_TRUE( utest_fixture->ctx.decl_count >= 1 );
    EXPECT_FALSE( utest_fixture->ctx.decls[ 0 ].focusable );
    EXPECT_TRUE( utest_fixture->ctx.decls[ 0 ].disabled );
}

UTEST_F( primitives_fixture, multiple_labels_and_actions_preserve_order )
{
    vxui_set_text_fn( &utest_fixture->ctx, vxui__primitive_text_fn, nullptr );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {} ) {
        VXUI_LABEL( &utest_fixture->ctx, "menu.start", ( vxui_label_cfg ) { 0 } );
        VXUI_ACTION( &utest_fixture->ctx, "start", "menu.start", vxui__test_action, ( vxui_action_cfg ) { 0 } );
        VXUI_LABEL( &utest_fixture->ctx, "menu.quit", ( vxui_label_cfg ) { 0 } );
    }

    vxui_end( &utest_fixture->ctx );
    ASSERT_TRUE( utest_fixture->ctx.decl_count >= 3 );
    EXPECT_EQ( utest_fixture->ctx.decls[ 0 ].kind, VXUI_DECL_LABEL );
    EXPECT_EQ( utest_fixture->ctx.decls[ 1 ].kind, VXUI_DECL_ACTION );
    EXPECT_EQ( utest_fixture->ctx.decls[ 2 ].kind, VXUI_DECL_LABEL );
}

UTEST_F( primitives_fixture, frame_owned_text_buffers_remain_valid_until_end )
{
    std::memcpy( vxui__primitive_text_buffer, "Original", sizeof( "Original" ) );
    vxui_set_text_fn( &utest_fixture->ctx, vxui__primitive_text_fn, nullptr );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {} ) {
        VXUI_LABEL( &utest_fixture->ctx, "menu.dynamic", ( vxui_label_cfg ) { 0 } );
        std::memcpy( vxui__primitive_text_buffer, "Mutated", sizeof( "Mutated" ) );
    }

    vxui_end( &utest_fixture->ctx );
    EXPECT_STREQ( utest_fixture->ctx.text_queue[ 0 ].text, "Original" );
}

UTEST_F( primitives_fixture, null_format_uses_fallback )
{
    vxui_set_text_fn( &utest_fixture->ctx, vxui__primitive_text_fn, nullptr );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {} ) {
        VXUI_VALUE( &utest_fixture->ctx, "menu.volume", 12.5f, ( vxui_value_cfg ) { 0 } );
    }

    vxui_end( &utest_fixture->ctx );
    EXPECT_STREQ( utest_fixture->ctx.text_queue[ 1 ].text, "12.5" );
}

UTEST_F( primitives_fixture, action_ids_remain_stable_across_repeated_frames )
{
    vxui_set_text_fn( &utest_fixture->ctx, vxui__primitive_text_fn, nullptr );

    uint32_t ids[ 2 ] = {};
    for ( int i = 0; i < 2; ++i ) {
        vxui_begin( &utest_fixture->ctx, 0.016f );
        VXUI( &utest_fixture->ctx, "root", {} ) {
            VXUI_ACTION( &utest_fixture->ctx, "start", "menu.start", vxui__test_action, ( vxui_action_cfg ) { 0 } );
        }

        vxui_end( &utest_fixture->ctx );
        ids[ i ] = utest_fixture->ctx.decls[ 0 ].id;
    }

    EXPECT_EQ( ids[ 0 ], ids[ 1 ] );
}

UTEST_F( primitives_fixture, stacked_labels_emit_increasing_text_y_positions )
{
    vxui_set_text_fn( &utest_fixture->ctx, vxui__primitive_text_fn, nullptr );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {
        .layout = {
            .childGap = 10,
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
    } ) {
        VXUI_LABEL( &utest_fixture->ctx, "menu.title", ( vxui_label_cfg ) { 0 } );
        VXUI_LABEL( &utest_fixture->ctx, "menu.start", ( vxui_label_cfg ) { 0 } );
        VXUI_LABEL( &utest_fixture->ctx, "menu.quit", ( vxui_label_cfg ) { 0 } );
    }

    vxui_draw_list list = vxui_end( &utest_fixture->ctx );
    float ys[ 3 ] = {};
    ASSERT_EQ( vxui__collect_text_command_positions( &list, ys, 3 ), 3 );
    EXPECT_LT( ys[ 0 ], ys[ 1 ] );
    EXPECT_LT( ys[ 1 ], ys[ 2 ] );
}

UTEST_F( primitives_fixture, mixed_text_widgets_render_below_title_in_vertical_stack )
{
    vxui_set_text_fn( &utest_fixture->ctx, vxui__primitive_text_fn, nullptr );

    vxui_input_table table = {
        .confirm = {
            .font_id = 7,
            .glyph = 'A',
        },
    };
    vxui_set_input_table( &utest_fixture->ctx, &table );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {
        .layout = {
            .childGap = 12,
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
    } ) {
        VXUI_LABEL( &utest_fixture->ctx, "menu.title", ( vxui_label_cfg ) { 0 } );
        VXUI_ACTION( &utest_fixture->ctx, "start", "menu.start", vxui__test_action, ( vxui_action_cfg ) { 0 } );
        VXUI( &utest_fixture->ctx, "prompt-row", {
            .layout = {
                .childGap = 8,
                .layoutDirection = CLAY_LEFT_TO_RIGHT,
            },
        } ) {
            VXUI_PROMPT( &utest_fixture->ctx, "action.confirm" );
            VXUI_LABEL( &utest_fixture->ctx, "menu.status", ( vxui_label_cfg ) { 0 } );
        }
        VXUI_LABEL( &utest_fixture->ctx, "menu.ready", ( vxui_label_cfg ) { 0 } );
        VXUI_LABEL( &utest_fixture->ctx, "menu.hint", ( vxui_label_cfg ) { 0 } );
        VXUI_LABEL( &utest_fixture->ctx, "menu.quit", ( vxui_label_cfg ) { 0 } );
    }

    vxui_draw_list list = vxui_end( &utest_fixture->ctx );
    float ys[ 7 ] = {};
    int text_count = vxui__collect_text_command_positions( &list, ys, 7 );
    ASSERT_EQ( text_count, 7 );

    float title_y = ys[ 0 ];
    for ( int i = 1; i < text_count; ++i ) {
        EXPECT_GT( ys[ i ], title_y );
    }
}

UTEST_F( primitives_fixture, narrow_container_wraps_label_into_increasing_text_rows )
{
    vxui_set_text_fn( &utest_fixture->ctx, vxui__primitive_text_fn, nullptr );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {
        .layout = {
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
    } ) {
        VXUI( &utest_fixture->ctx, "wrap-box", {
            .layout = {
                .sizing = { CLAY_SIZING_FIXED( 120 ), CLAY_SIZING_FIT( 0 ) },
            },
        } ) {
            const char* wrapped_text = vxui__primitive_text_fn( "menu.wrap.long", nullptr );
            Clay_String wrapped = {
                .isStaticallyAllocated = false,
                .length = ( int32_t ) std::strlen( wrapped_text ),
                .chars = wrapped_text,
            };
            CLAY( CLAY_ID( "wrap.text" ), {
                .layout = {
                    .sizing = { CLAY_SIZING_FIXED( 120 ), CLAY_SIZING_FIT( 0 ) },
                },
            } ) {
                CLAY_TEXT(
                    wrapped,
                    CLAY_TEXT_CONFIG( {
                        .userData = ( void* ) ( uintptr_t ) vxui_id( "menu.wrap.long" ),
                        .textColor = { 255, 255, 255, 255 },
                        .fontId = 0,
                        .fontSize = 18,
                        .wrapMode = CLAY_TEXT_WRAP_NEWLINES,
                    } ) );
            }
        }
    }

    vxui_draw_list list = vxui_end( &utest_fixture->ctx );
    float ys[ 8 ] = {};
    int text_count = vxui__collect_text_command_positions( &list, ys, 8 );

    ASSERT_GT( text_count, 1 );
    for ( int i = 1; i < text_count; ++i ) {
        EXPECT_LT( ys[ i - 1 ], ys[ i ] );
    }
}
