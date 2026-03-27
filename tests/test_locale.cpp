#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "../third_party/utest.h"
#include "../vxui.h"
#include "test_support.h"

enum
{
    VXUI_TEST_FONT_UI = 0,
    VXUI_TEST_FONT_TITLE = 1,
    VXUI_TEST_FONT_DEBUG = 2,
    VXUI_TEST_FONT_JAPANESE = 3,
    VXUI_TEST_FONT_ARABIC = 4,
    VXUI_TEST_FONT_UI_LARGE = 5,
    VXUI_TEST_FONT_JAPANESE_TITLE = 6,
    VXUI_TEST_FONT_ARABIC_TITLE = 7,
    VXUI_TEST_FONT_ROLE_BODY = 100,
    VXUI_TEST_FONT_ROLE_TITLE = 101,
};

typedef struct locale_fixture
{
    uint8_t* memory;
    uint64_t memory_size;
    vxui_ctx ctx;
    const char* long_text;
    vxui_test_fontcache_handle* fontcache;
} locale_fixture;

static const char* vxui__locale_text( const char* key, void* userdata )
{
    locale_fixture* fixture = ( locale_fixture* ) userdata;

    if ( std::strcmp( key, "label.hello" ) == 0 ) {
        return "Hello";
    }
    if ( std::strcmp( key, "label.left" ) == 0 ) {
        return "Left";
    }
    if ( std::strcmp( key, "label.right" ) == 0 ) {
        return "Right";
    }
    if ( std::strcmp( key, "label.long" ) == 0 ) {
        return fixture->long_text;
    }
    if ( std::strcmp( key, "label.body" ) == 0 ) {
        return "Typography";
    }
    if ( std::strcmp( key, "label.title" ) == 0 ) {
        return "Typography";
    }
    if ( std::strcmp( key, "label.measure.small" ) == 0 ) {
        return "MMMMMM";
    }
    if ( std::strcmp( key, "label.measure.large" ) == 0 ) {
        return "MMMMMM";
    }
    if ( std::strcmp( key, "prompt.confirm.label" ) == 0 ) {
        return fixture->ctx.rtl ? "تأكيد" : "Confirm";
    }
    if ( std::strcmp( key, "status.label.screens" ) == 0 ) {
        return fixture->ctx.rtl ? "الشاشات" : "Screens";
    }

    return key;
}

static float vxui__locale_find_text_x( const vxui_draw_list* list, const char* text )
{
    for ( int i = 0; i < list->length; ++i ) {
        if ( list->commands[ i ].type == VXUI_CMD_TEXT && list->commands[ i ].text.text && std::strcmp( list->commands[ i ].text.text, text ) == 0 ) {
            return list->commands[ i ].text.pos.x;
        }
    }
    return -1.0f;
}

static bool vxui__locale_find_text_pos( const vxui_draw_list* list, const char* text, vxui_vec2* out )
{
    if ( !list || !text ) {
        return false;
    }

    for ( int i = 0; i < list->length; ++i ) {
        if ( list->commands[ i ].type != VXUI_CMD_TEXT || !list->commands[ i ].text.text ) {
            continue;
        }
        if ( std::strcmp( list->commands[ i ].text.text, text ) != 0 ) {
            continue;
        }
        if ( out ) {
            *out = list->commands[ i ].text.pos;
        }
        return true;
    }
    return false;
}

static bool vxui__locale_find_anim_bounds( const vxui_ctx* ctx, uint32_t id, vxui_rect* out )
{
    if ( !ctx || !ctx->anim_slots || ctx->anim_capacity <= 0 ) {
        return false;
    }

    for ( int i = 0; i < ctx->anim_capacity; ++i ) {
        const vxui_anim_slot* slot = &ctx->anim_slots[ i ];
        if ( !slot->occupied || slot->state.id != id ) {
            continue;
        }
        if ( out ) {
            *out = slot->state.bounds;
        }
        return true;
    }
    return false;
}

static bool vxui__locale_matches( const char* locale, const char* prefix )
{
    if ( !locale || !prefix ) {
        return false;
    }

    size_t prefix_length = std::strlen( prefix );
    if ( std::strncmp( locale, prefix, prefix_length ) != 0 ) {
        return false;
    }

    return locale[ prefix_length ] == '\0' || locale[ prefix_length ] == '-' || locale[ prefix_length ] == '_';
}

static void vxui__locale_font_resolver(
    vxui_ctx* ctx,
    uint32_t requested_font_id,
    float requested_font_size,
    const char* locale,
    void* userdata,
    vxui_resolved_font* out )
{
    ( void ) ctx;
    locale_fixture* fixture = ( locale_fixture* ) userdata;
    if ( !fixture || !out ) {
        return;
    }

    switch ( requested_font_id ) {
        case VXUI_TEST_FONT_ROLE_BODY:
            out->font_id = requested_font_size >= 40.0f ? VXUI_TEST_FONT_UI_LARGE : VXUI_TEST_FONT_UI;
            out->line_height = requested_font_size >= 40.0f ? 48.0f : 28.0f;
            return;

        case VXUI_TEST_FONT_ROLE_TITLE:
            if ( vxui__locale_matches( locale, "ja" ) ) {
                out->font_id = VXUI_TEST_FONT_JAPANESE_TITLE;
            } else if ( vxui__locale_matches( locale, "ar" ) ) {
                out->font_id = VXUI_TEST_FONT_ARABIC_TITLE;
            } else {
                out->font_id = VXUI_TEST_FONT_TITLE;
            }
            out->line_height = 52.0f;
            return;

        default:
            return;
    }
}

UTEST_F_SETUP( locale_fixture )
{
    static const char* kLongText = "長い長い長い長い長い長い長い長い長い長い長い長い長い長い長い長い長い長い長い長い";

    utest_fixture->memory_size = vxui_min_memory_size();
    utest_fixture->memory = ( uint8_t* ) std::malloc( ( size_t ) utest_fixture->memory_size );
    ASSERT_TRUE( utest_fixture->memory != nullptr );
    utest_fixture->long_text = kLongText;

    vxui_init(
        &utest_fixture->ctx,
        vxui_create_arena( utest_fixture->memory_size, utest_fixture->memory ),
        ( vxui_config ) {
            .screen_width = 640,
            .screen_height = 360,
            .max_elements = 128,
        } );
    vxui_set_text_fn( &utest_fixture->ctx, vxui__locale_text, utest_fixture );
    utest_fixture->fontcache = vxui_test_fontcache_create( true );
    ASSERT_TRUE( utest_fixture->fontcache != nullptr );
    EXPECT_EQ( vxui_test_fontcache_load_file( utest_fixture->fontcache, VXUI_SOURCE_DIR "/vefc/demo/fonts/OpenSans-Regular.ttf", 24.0f ), ( int64_t ) VXUI_TEST_FONT_UI );
    EXPECT_EQ( vxui_test_fontcache_load_file( utest_fixture->fontcache, VXUI_SOURCE_DIR "/vefc/demo/fonts/Bitter-Regular.ttf", 44.0f ), ( int64_t ) VXUI_TEST_FONT_TITLE );
    EXPECT_EQ( vxui_test_fontcache_load_file( utest_fixture->fontcache, VXUI_SOURCE_DIR "/vefc/demo/fonts/NovaMono-Regular.ttf", 16.0f ), ( int64_t ) VXUI_TEST_FONT_DEBUG );
    EXPECT_EQ( vxui_test_fontcache_load_file( utest_fixture->fontcache, VXUI_SOURCE_DIR "/vefc/demo/fonts/NotoSansJP-Regular.otf", 24.0f ), ( int64_t ) VXUI_TEST_FONT_JAPANESE );
    EXPECT_EQ( vxui_test_fontcache_load_file( utest_fixture->fontcache, VXUI_SOURCE_DIR "/vefc/demo/fonts/Tajawal-Regular.ttf", 24.0f ), ( int64_t ) VXUI_TEST_FONT_ARABIC );
    EXPECT_EQ( vxui_test_fontcache_load_file( utest_fixture->fontcache, VXUI_SOURCE_DIR "/vefc/demo/fonts/OpenSans-Regular.ttf", 44.0f ), ( int64_t ) VXUI_TEST_FONT_UI_LARGE );
    EXPECT_EQ( vxui_test_fontcache_load_file( utest_fixture->fontcache, VXUI_SOURCE_DIR "/vefc/demo/fonts/NotoSansJP-Regular.otf", 44.0f ), ( int64_t ) VXUI_TEST_FONT_JAPANESE_TITLE );
    EXPECT_EQ( vxui_test_fontcache_load_file( utest_fixture->fontcache, VXUI_SOURCE_DIR "/vefc/demo/fonts/Tajawal-Regular.ttf", 44.0f ), ( int64_t ) VXUI_TEST_FONT_ARABIC_TITLE );
    vxui_set_fontcache( &utest_fixture->ctx, vxui_test_fontcache_ptr( utest_fixture->fontcache ) );
    utest_fixture->ctx.default_font_id = VXUI_TEST_FONT_UI;
    utest_fixture->ctx.default_font_size = 24.0f;
}

UTEST_F_TEARDOWN( locale_fixture )
{
    vxui_shutdown( &utest_fixture->ctx );
    vxui_test_fontcache_destroy( utest_fixture->fontcache );
    utest_fixture->fontcache = nullptr;
    std::free( utest_fixture->memory );
    utest_fixture->memory = nullptr;
    utest_fixture->memory_size = 0;
}

UTEST_F( locale_fixture, locale_string_is_stored_correctly )
{
    vxui_set_locale( &utest_fixture->ctx, "ja-JP" );
    EXPECT_STREQ( utest_fixture->ctx.locale, "ja-JP" );
}

UTEST_F( locale_fixture, locale_font_mapping_overrides_default_font )
{
    vxui_set_locale_font( &utest_fixture->ctx, "ja", VXUI_TEST_FONT_JAPANESE );
    vxui_set_locale( &utest_fixture->ctx, "ja" );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {} ) {
        VXUI_LABEL( &utest_fixture->ctx, "label.hello", ( vxui_label_cfg ) { 0 } );
    }
    vxui_end( &utest_fixture->ctx );

    EXPECT_EQ( utest_fixture->ctx.text_queue[ 0 ].font_id, VXUI_TEST_FONT_JAPANESE );
}

UTEST_F( locale_fixture, missing_locale_mapping_falls_back_to_default_font )
{
    utest_fixture->ctx.default_font_id = VXUI_TEST_FONT_TITLE;
    vxui_set_locale( &utest_fixture->ctx, "fr" );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {} ) {
        VXUI_LABEL( &utest_fixture->ctx, "label.hello", ( vxui_label_cfg ) { 0 } );
    }
    vxui_end( &utest_fixture->ctx );

    EXPECT_EQ( utest_fixture->ctx.text_queue[ 0 ].font_id, VXUI_TEST_FONT_TITLE );
}

UTEST_F( locale_fixture, prefix_locale_lookup_works_for_ja_jp )
{
    vxui_set_locale_font( &utest_fixture->ctx, "ja", VXUI_TEST_FONT_JAPANESE );
    vxui_set_locale( &utest_fixture->ctx, "ja-JP" );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {} ) {
        VXUI_LABEL( &utest_fixture->ctx, "label.hello", ( vxui_label_cfg ) { 0 } );
    }
    vxui_end( &utest_fixture->ctx );

    EXPECT_EQ( utest_fixture->ctx.text_queue[ 0 ].font_id, VXUI_TEST_FONT_JAPANESE );
}

UTEST_F( locale_fixture, input_table_swap_changes_prompt_source )
{
    vxui_input_table keyboard = {
        .confirm = { VXUI_TEST_FONT_UI, 'E' },
    };
    vxui_input_table gamepad = {
        .confirm = { VXUI_TEST_FONT_DEBUG, 'A' },
    };

    vxui_set_input_table( &utest_fixture->ctx, &keyboard );
    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {} ) {
        VXUI_PROMPT( &utest_fixture->ctx, "action.confirm" );
    }
    vxui_end( &utest_fixture->ctx );
    EXPECT_STREQ( utest_fixture->ctx.text_queue[ 0 ].text, "E" );
    EXPECT_EQ( utest_fixture->ctx.text_queue[ 0 ].font_id, VXUI_TEST_FONT_UI );

    vxui_set_input_table( &utest_fixture->ctx, &gamepad );
    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {} ) {
        VXUI_PROMPT( &utest_fixture->ctx, "action.confirm" );
    }
    vxui_end( &utest_fixture->ctx );
    EXPECT_STREQ( utest_fixture->ctx.text_queue[ 0 ].text, "A" );
    EXPECT_EQ( utest_fixture->ctx.text_queue[ 0 ].font_id, VXUI_TEST_FONT_DEBUG );
}

UTEST_F( locale_fixture, rtl_detection_for_ar_returns_true )
{
    vxui_set_locale( &utest_fixture->ctx, "ar" );
    EXPECT_TRUE( utest_fixture->ctx.rtl );
}

UTEST_F( locale_fixture, plain_label_rows_preserve_explicit_order_under_rtl )
{
    uint32_t ids_ltr[ 2 ] = {};
    uint32_t ids_rtl[ 2 ] = {};

    vxui_set_locale( &utest_fixture->ctx, "en" );
    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "row", {
        .layout = {
            .childGap = 20,
            .layoutDirection = CLAY_LEFT_TO_RIGHT,
        },
    } ) {
        VXUI_LABEL( &utest_fixture->ctx, "label.left", ( vxui_label_cfg ) { 0 } );
        VXUI_LABEL( &utest_fixture->ctx, "label.right", ( vxui_label_cfg ) { 0 } );
    }
    vxui_draw_list ltr = vxui_end( &utest_fixture->ctx );
    ids_ltr[ 0 ] = utest_fixture->ctx.decls[ 0 ].id;
    ids_ltr[ 1 ] = utest_fixture->ctx.decls[ 1 ].id;
    float left_x_ltr = vxui__locale_find_text_x( &ltr, "Left" );
    float right_x_ltr = vxui__locale_find_text_x( &ltr, "Right" );

    vxui_set_locale( &utest_fixture->ctx, "ar" );
    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "row", {
        .layout = {
            .childGap = 20,
            .layoutDirection = CLAY_LEFT_TO_RIGHT,
        },
    } ) {
        VXUI_LABEL( &utest_fixture->ctx, "label.left", ( vxui_label_cfg ) { 0 } );
        VXUI_LABEL( &utest_fixture->ctx, "label.right", ( vxui_label_cfg ) { 0 } );
    }
    vxui_draw_list rtl = vxui_end( &utest_fixture->ctx );
    ids_rtl[ 0 ] = utest_fixture->ctx.decls[ 0 ].id;
    ids_rtl[ 1 ] = utest_fixture->ctx.decls[ 1 ].id;
    float left_x_rtl = vxui__locale_find_text_x( &rtl, "Left" );
    float right_x_rtl = vxui__locale_find_text_x( &rtl, "Right" );

    EXPECT_EQ( ids_ltr[ 0 ], ids_rtl[ 0 ] );
    EXPECT_EQ( ids_ltr[ 1 ], ids_rtl[ 1 ] );
    EXPECT_TRUE( left_x_ltr < right_x_ltr );
    EXPECT_TRUE( left_x_rtl < right_x_rtl );
}

UTEST_F( locale_fixture, value_widget_swaps_label_and_value_order_in_rtl )
{
    vxui_set_font_resolver( &utest_fixture->ctx, vxui__locale_font_resolver, utest_fixture );

    vxui_set_locale( &utest_fixture->ctx, "en" );
    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {} ) {
        VXUI_VALUE( &utest_fixture->ctx, "status.label.screens", 2.0f, ( vxui_value_cfg ) {
            .font_id = VXUI_TEST_FONT_ROLE_BODY,
            .font_size = 24.0f,
            .format = "%.0f",
        } );
    }
    vxui_draw_list ltr = vxui_end( &utest_fixture->ctx );
    float label_x_ltr = vxui__locale_find_text_x( &ltr, "Screens" );
    float value_x_ltr = vxui__locale_find_text_x( &ltr, "2" );
    uint32_t value_id_ltr = utest_fixture->ctx.decls[ 0 ].id;

    vxui_set_locale( &utest_fixture->ctx, "ar" );
    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {} ) {
        VXUI_VALUE( &utest_fixture->ctx, "status.label.screens", 2.0f, ( vxui_value_cfg ) {
            .font_id = VXUI_TEST_FONT_ROLE_BODY,
            .font_size = 24.0f,
            .format = "%.0f",
        } );
    }
    vxui_draw_list rtl = vxui_end( &utest_fixture->ctx );
    float label_x_rtl = vxui__locale_find_text_x( &rtl, "الشاشات" );
    float value_x_rtl = vxui__locale_find_text_x( &rtl, "2" );
    uint32_t value_id_rtl = utest_fixture->ctx.decls[ 0 ].id;

    EXPECT_EQ( value_id_ltr, value_id_rtl );
    EXPECT_TRUE( label_x_ltr < value_x_ltr );
    EXPECT_TRUE( value_x_rtl < label_x_rtl );
}

UTEST_F( locale_fixture, font_resolver_maps_title_faces_by_locale )
{
    vxui_set_font_resolver( &utest_fixture->ctx, vxui__locale_font_resolver, utest_fixture );

    struct locale_case
    {
        const char* locale;
        uint32_t expected_font_id;
    };
    const locale_case cases[] = {
        { "en", VXUI_TEST_FONT_TITLE },
        { "ja-JP", VXUI_TEST_FONT_JAPANESE_TITLE },
        { "ar", VXUI_TEST_FONT_ARABIC_TITLE },
    };

    for ( const locale_case& test_case : cases ) {
        vxui_set_locale( &utest_fixture->ctx, test_case.locale );
        vxui_begin( &utest_fixture->ctx, 0.016f );
        VXUI( &utest_fixture->ctx, "root", {} ) {
            VXUI_LABEL( &utest_fixture->ctx, "label.title", ( vxui_label_cfg ) {
                .font_id = VXUI_TEST_FONT_ROLE_TITLE,
                .font_size = 44.0f,
            } );
        }
        vxui_end( &utest_fixture->ctx );

        ASSERT_TRUE( utest_fixture->ctx.text_queue_count >= 1 );
        EXPECT_EQ( utest_fixture->ctx.text_queue[ 0 ].font_id, test_case.expected_font_id );
    }
}

UTEST_F( locale_fixture, resolved_line_height_changes_text_bounds )
{
    vxui_set_font_resolver( &utest_fixture->ctx, vxui__locale_font_resolver, utest_fixture );
    vxui_set_locale( &utest_fixture->ctx, "en" );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {
        .layout = {
            .childGap = 8,
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
    } ) {
        VXUI_LABEL( &utest_fixture->ctx, "label.body", ( vxui_label_cfg ) {
            .font_id = VXUI_TEST_FONT_ROLE_BODY,
            .font_size = 24.0f,
        } );
        VXUI_LABEL( &utest_fixture->ctx, "label.title", ( vxui_label_cfg ) {
            .font_id = VXUI_TEST_FONT_ROLE_TITLE,
            .font_size = 44.0f,
        } );
    }
    vxui_end( &utest_fixture->ctx );

    vxui_rect body_bounds = {};
    vxui_rect title_bounds = {};
    ASSERT_TRUE( vxui__locale_find_anim_bounds( &utest_fixture->ctx, vxui_id( "label.body" ), &body_bounds ) );
    ASSERT_TRUE( vxui__locale_find_anim_bounds( &utest_fixture->ctx, vxui_id( "label.title" ), &title_bounds ) );
    EXPECT_GT( body_bounds.h, 20.0f );
    EXPECT_GT( title_bounds.h, body_bounds.h );
}

UTEST_F( locale_fixture, resolver_uses_requested_size_for_measured_width )
{
    vxui_set_font_resolver( &utest_fixture->ctx, vxui__locale_font_resolver, utest_fixture );
    vxui_set_locale( &utest_fixture->ctx, "en" );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {
        .layout = {
            .childGap = 8,
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
    } ) {
        VXUI_LABEL( &utest_fixture->ctx, "label.measure.small", ( vxui_label_cfg ) {
            .font_id = VXUI_TEST_FONT_ROLE_BODY,
            .font_size = 24.0f,
        } );
        VXUI_LABEL( &utest_fixture->ctx, "label.measure.large", ( vxui_label_cfg ) {
            .font_id = VXUI_TEST_FONT_ROLE_BODY,
            .font_size = 44.0f,
        } );
    }
    vxui_end( &utest_fixture->ctx );

    vxui_rect small_bounds = {};
    vxui_rect large_bounds = {};
    ASSERT_TRUE( vxui__locale_find_anim_bounds( &utest_fixture->ctx, vxui_id( "label.measure.small" ), &small_bounds ) );
    ASSERT_TRUE( vxui__locale_find_anim_bounds( &utest_fixture->ctx, vxui_id( "label.measure.large" ), &large_bounds ) );
    EXPECT_EQ( utest_fixture->ctx.text_queue[ 0 ].font_id, ( uint32_t ) VXUI_TEST_FONT_UI );
    EXPECT_EQ( utest_fixture->ctx.text_queue[ 1 ].font_id, ( uint32_t ) VXUI_TEST_FONT_UI_LARGE );
    EXPECT_GT( small_bounds.w, 0.0f );
    EXPECT_GT( large_bounds.w, small_bounds.w );
}

UTEST_F( locale_fixture, arabic_prompt_and_status_rows_stay_on_screen_with_semantic_order )
{
    vxui_set_font_resolver( &utest_fixture->ctx, vxui__locale_font_resolver, utest_fixture );
    vxui_set_locale( &utest_fixture->ctx, "ar" );

    vxui_input_table table = {
        .confirm = { VXUI_TEST_FONT_ROLE_BODY, 'A' },
    };
    vxui_set_input_table( &utest_fixture->ctx, &table );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {
        .layout = {
            .sizing = { CLAY_SIZING_FIXED( 640 ), CLAY_SIZING_GROW( 0 ) },
            .childGap = 12,
            .childAlignment = { .x = CLAY_ALIGN_X_RIGHT },
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
    } ) {
        VXUI( &utest_fixture->ctx, "prompt.row", {
            .layout = {
                .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                .childGap = 8,
                .childAlignment = { .x = CLAY_ALIGN_X_RIGHT, .y = CLAY_ALIGN_Y_CENTER },
                .layoutDirection = CLAY_LEFT_TO_RIGHT,
            },
        } ) {
            VXUI_PROMPT( &utest_fixture->ctx, "action.confirm" );
            VXUI_LABEL( &utest_fixture->ctx, "prompt.confirm.label", ( vxui_label_cfg ) {
                .font_id = VXUI_TEST_FONT_ROLE_BODY,
                .font_size = 24.0f,
            } );
        }

        VXUI( &utest_fixture->ctx, "status.row", {
            .layout = {
                .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                .childGap = 8,
                .childAlignment = { .x = CLAY_ALIGN_X_RIGHT, .y = CLAY_ALIGN_Y_CENTER },
                .layoutDirection = CLAY_LEFT_TO_RIGHT,
            },
        } ) {
            VXUI_VALUE( &utest_fixture->ctx, "status.label.screens", 2.0f, ( vxui_value_cfg ) {
                .font_id = VXUI_TEST_FONT_ROLE_BODY,
                .font_size = 24.0f,
                .format = "%.0f",
            } );
        }
    }
    vxui_draw_list list = vxui_end( &utest_fixture->ctx );

    vxui_vec2 prompt_pos = {};
    vxui_vec2 prompt_label_pos = {};
    vxui_vec2 value_pos = {};
    vxui_vec2 status_label_pos = {};
    ASSERT_TRUE( vxui__locale_find_text_pos( &list, "A", &prompt_pos ) );
    ASSERT_TRUE( vxui__locale_find_text_pos( &list, "تأكيد", &prompt_label_pos ) );
    ASSERT_TRUE( vxui__locale_find_text_pos( &list, "2", &value_pos ) );
    ASSERT_TRUE( vxui__locale_find_text_pos( &list, "الشاشات", &status_label_pos ) );
    EXPECT_LT( prompt_pos.x, prompt_label_pos.x );
    EXPECT_LT( value_pos.x, status_label_pos.x );

    for ( int i = 0; i < list.length; ++i ) {
        if ( list.commands[ i ].type != VXUI_CMD_TEXT ) {
            continue;
        }
        EXPECT_GE( list.commands[ i ].text.pos.x, 0.0f );
        EXPECT_LE( list.commands[ i ].text.pos.x, ( float ) utest_fixture->ctx.cfg.screen_width );
        EXPECT_GE( list.commands[ i ].text.pos.y, 0.0f );
        EXPECT_LT( list.commands[ i ].text.pos.y, ( float ) utest_fixture->ctx.cfg.screen_height );
    }
}

UTEST_F( locale_fixture, repeated_locale_swaps_do_not_corrupt_font_mapping )
{
    vxui_set_locale_font( &utest_fixture->ctx, "en", VXUI_TEST_FONT_UI );
    vxui_set_locale_font( &utest_fixture->ctx, "ja", VXUI_TEST_FONT_JAPANESE );
    vxui_set_locale_font( &utest_fixture->ctx, "ar", VXUI_TEST_FONT_ARABIC );

    const char* locales[] = { "en", "ja", "ar", "ja-JP", "en" };
    const uint32_t expected_fonts[] = { VXUI_TEST_FONT_UI, VXUI_TEST_FONT_JAPANESE, VXUI_TEST_FONT_ARABIC, VXUI_TEST_FONT_JAPANESE, VXUI_TEST_FONT_UI };
    for ( int i = 0; i < 5; ++i ) {
        vxui_set_locale( &utest_fixture->ctx, locales[ i ] );
        vxui_begin( &utest_fixture->ctx, 0.016f );
        VXUI( &utest_fixture->ctx, "root", {} ) {
            VXUI_LABEL( &utest_fixture->ctx, "label.hello", ( vxui_label_cfg ) { 0 } );
        }
        vxui_end( &utest_fixture->ctx );
        EXPECT_EQ( utest_fixture->ctx.text_queue[ 0 ].font_id, expected_fonts[ i ] );
    }
}

UTEST_F( locale_fixture, named_demo_font_map_matches_expected_locale_faces )
{
    utest_fixture->ctx.default_font_id = VXUI_TEST_FONT_UI;
    vxui_set_locale_font( &utest_fixture->ctx, "ja", VXUI_TEST_FONT_JAPANESE );
    vxui_set_locale_font( &utest_fixture->ctx, "ar", VXUI_TEST_FONT_ARABIC );

    const char* locales[] = { "en", "ja-JP", "ar" };
    const uint32_t expected_fonts[] = { VXUI_TEST_FONT_UI, VXUI_TEST_FONT_JAPANESE, VXUI_TEST_FONT_ARABIC };
    for ( int i = 0; i < 3; ++i ) {
        vxui_set_locale( &utest_fixture->ctx, locales[ i ] );
        vxui_begin( &utest_fixture->ctx, 0.016f );
        VXUI( &utest_fixture->ctx, "root", {} ) {
            VXUI_LABEL( &utest_fixture->ctx, "label.hello", ( vxui_label_cfg ) { 0 } );
        }
        vxui_end( &utest_fixture->ctx );
        EXPECT_EQ( utest_fixture->ctx.text_queue[ 0 ].font_id, expected_fonts[ i ] );
    }
}

UTEST_F( locale_fixture, long_utf8_strings_still_measure_safely )
{
    vxui_set_locale( &utest_fixture->ctx, "ja" );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {} ) {
        VXUI_LABEL( &utest_fixture->ctx, "label.long", ( vxui_label_cfg ) { 0 } );
    }
    vxui_end( &utest_fixture->ctx );

    EXPECT_TRUE( utest_fixture->ctx.text_queue_count >= 1 );
    EXPECT_STREQ( utest_fixture->ctx.text_queue[ 0 ].text, utest_fixture->long_text );
}

UTEST_F( locale_fixture, repeated_label_keys_generate_unique_clay_ids )
{
    vxui_set_locale_font( &utest_fixture->ctx, "en", VXUI_TEST_FONT_UI );
    vxui_set_locale( &utest_fixture->ctx, "en" );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {} ) {
        VXUI_LABEL( &utest_fixture->ctx, "label.hello", ( vxui_label_cfg ) { 0 } );
        VXUI_LABEL( &utest_fixture->ctx, "label.hello", ( vxui_label_cfg ) { 0 } );
        VXUI_LABEL( &utest_fixture->ctx, "label.hello", ( vxui_label_cfg ) { 0 } );
        VXUI_LABEL( &utest_fixture->ctx, "label.left", ( vxui_label_cfg ) { 0 } );
    }
    vxui_end( &utest_fixture->ctx );

    ASSERT_GE( utest_fixture->ctx.decl_count, 4 );
    uint32_t decl_id_hello = vxui_id( "label.hello" );

    int hello_count = 0;
    for ( int i = 0; i < utest_fixture->ctx.decl_count; ++i ) {
        if ( utest_fixture->ctx.decls[ i ].kind != VXUI_DECL_LABEL ) continue;
        if ( utest_fixture->ctx.decls[ i ].id == decl_id_hello ) {
            hello_count++;
        }
    }
    EXPECT_GE( hello_count, 3 );

    for ( int i = 0; i < utest_fixture->ctx.decl_count; ++i ) {
        for ( int j = i + 1; j < utest_fixture->ctx.decl_count; ++j ) {
            if ( utest_fixture->ctx.decls[ i ].kind != VXUI_DECL_LABEL ) continue;
            if ( utest_fixture->ctx.decls[ j ].kind != VXUI_DECL_LABEL ) continue;
            EXPECT_NE( utest_fixture->ctx.decls[ i ].clay_id, utest_fixture->ctx.decls[ j ].clay_id );
        }
    }
}

UTEST_F( locale_fixture, repeated_value_keys_generate_unique_clay_ids )
{
    vxui_set_locale_font( &utest_fixture->ctx, "en", VXUI_TEST_FONT_UI );
    vxui_set_locale( &utest_fixture->ctx, "en" );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {} ) {
        VXUI_VALUE( &utest_fixture->ctx, "label.hello", 1.0f, ( vxui_value_cfg ) { .format = "%.0f" } );
        VXUI_VALUE( &utest_fixture->ctx, "label.hello", 2.0f, ( vxui_value_cfg ) { .format = "%.0f" } );
    }
    vxui_end( &utest_fixture->ctx );

    for ( int i = 0; i < utest_fixture->ctx.decl_count; ++i ) {
        for ( int j = i + 1; j < utest_fixture->ctx.decl_count; ++j ) {
            if ( utest_fixture->ctx.decls[ i ].kind != VXUI_DECL_VALUE ) continue;
            if ( utest_fixture->ctx.decls[ j ].kind != VXUI_DECL_VALUE ) continue;
            EXPECT_NE( utest_fixture->ctx.decls[ i ].clay_id, utest_fixture->ctx.decls[ j ].clay_id );
        }
    }
}

UTEST_F( locale_fixture, clay_ids_unique_across_screen_stack_snapshots )
{
    vxui_set_locale_font( &utest_fixture->ctx, "en", VXUI_TEST_FONT_UI );
    vxui_set_locale( &utest_fixture->ctx, "en" );

    vxui_push_screen( &utest_fixture->ctx, "screen_a" );
    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "screen_a", {} ) {
        VXUI_LABEL( &utest_fixture->ctx, "label.hello", ( vxui_label_cfg ) { 0 } );
    }
    vxui_end( &utest_fixture->ctx );

    vxui_push_screen( &utest_fixture->ctx, "screen_b" );
    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "screen_b", {} ) {
        VXUI_LABEL( &utest_fixture->ctx, "label.hello", ( vxui_label_cfg ) { 0 } );
    }
    vxui_draw_list list = vxui_end( &utest_fixture->ctx );

    bool found_text = false;
    for ( int i = 0; i < list.length; ++i ) {
        if ( list.commands[ i ].type == VXUI_CMD_TEXT ) {
            found_text = true;
            break;
        }
    }
    EXPECT_TRUE( found_text );
}

UTEST_F( locale_fixture, label_decl_id_stable_but_clay_id_unique )
{
    vxui_set_locale_font( &utest_fixture->ctx, "en", VXUI_TEST_FONT_UI );
    vxui_set_locale( &utest_fixture->ctx, "en" );

    uint32_t expected_decl_id = vxui_id( "label.hello" );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {} ) {
        VXUI_LABEL( &utest_fixture->ctx, "label.hello", ( vxui_label_cfg ) { 0 } );
        VXUI_LABEL( &utest_fixture->ctx, "label.hello", ( vxui_label_cfg ) { 0 } );
    }
    vxui_end( &utest_fixture->ctx );

    int label_count = 0;
    for ( int i = 0; i < utest_fixture->ctx.decl_count; ++i ) {
        if ( utest_fixture->ctx.decls[ i ].kind != VXUI_DECL_LABEL ) continue;
        label_count++;
        EXPECT_EQ( utest_fixture->ctx.decls[ i ].id, expected_decl_id );
        EXPECT_NE( utest_fixture->ctx.decls[ i ].clay_id, 0u );
    }
    ASSERT_GE( label_count, 2 );

    EXPECT_NE( utest_fixture->ctx.decls[ 0 ].clay_id, utest_fixture->ctx.decls[ 1 ].clay_id );
}

UTEST_F( locale_fixture, rtl_text_stays_within_screen_bounds )
{
    vxui_set_locale_font( &utest_fixture->ctx, "ar", VXUI_TEST_FONT_ARABIC );
    vxui_set_locale( &utest_fixture->ctx, "ar" );
    utest_fixture->ctx.rtl = true;

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {
        .layout = {
            .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_GROW( 0 ) },
            .padding = CLAY_PADDING_ALL( 18 ),
            .childGap = 12,
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
    } ) {
        VXUI_LABEL( &utest_fixture->ctx, "label.hello", ( vxui_label_cfg ) { 0 } );
        VXUI_LABEL( &utest_fixture->ctx, "label.right", ( vxui_label_cfg ) { 0 } );

        VXUI( &utest_fixture->ctx, "row.difficulty", {
            .layout = {
                .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                .childGap = 8,
                .childAlignment = { .x = CLAY_ALIGN_X_RIGHT, .y = CLAY_ALIGN_Y_CENTER },
                .layoutDirection = CLAY_LEFT_TO_RIGHT,
            },
        } ) {
            VXUI_LABEL( &utest_fixture->ctx, "label.right", ( vxui_label_cfg ) { 0 } );
            VXUI_LABEL( &utest_fixture->ctx, "label.left", ( vxui_label_cfg ) { 0 } );
        }
    }
    vxui_draw_list list = vxui_end( &utest_fixture->ctx );

    float sw = ( float ) utest_fixture->ctx.cfg.screen_width;
    float sh = ( float ) utest_fixture->ctx.cfg.screen_height;
    for ( int i = 0; i < list.length; ++i ) {
        if ( list.commands[ i ].type != VXUI_CMD_TEXT ) continue;
        EXPECT_GE( list.commands[ i ].text.pos.x, 0.0f );
        EXPECT_LE( list.commands[ i ].text.pos.x, sw );
        EXPECT_GE( list.commands[ i ].text.pos.y, 0.0f );
        EXPECT_LT( list.commands[ i ].text.pos.y, sh );
    }
}

UTEST_F( locale_fixture, content_column_bounds_child_elements )
{
    vxui_set_locale_font( &utest_fixture->ctx, "en", VXUI_TEST_FONT_UI );
    vxui_set_locale( &utest_fixture->ctx, "en" );

    utest_fixture->ctx.cfg.screen_width = 1280;
    utest_fixture->ctx.cfg.screen_height = 720;

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "screen", {
        .layout = {
            .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_GROW( 0 ) },
            .padding = CLAY_PADDING_ALL( 18 ),
        },
    } ) {
        VXUI( &utest_fixture->ctx, "content", {
            .layout = {
                .sizing = { CLAY_SIZING_GROW( 0, 620.0f ), CLAY_SIZING_FIT( 0 ) },
                .childGap = 16,
                .childAlignment = { .x = CLAY_ALIGN_X_LEFT },
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
        } ) {
            VXUI_LABEL( &utest_fixture->ctx, "label.hello", ( vxui_label_cfg ) { 0 } );
        }
    }
    vxui_draw_list list = vxui_end( &utest_fixture->ctx );

    float screen_w = ( float ) utest_fixture->ctx.cfg.screen_width;
    float max_content_w = 620.0f;
    float max_allowed_right = 18.0f + max_content_w;

    for ( int i = 0; i < list.length; ++i ) {
        if ( list.commands[ i ].type != VXUI_CMD_TEXT ) continue;
        float right_edge = list.commands[ i ].text.pos.x;
        EXPECT_LT( right_edge, max_allowed_right + 100.0f );
        EXPECT_LE( right_edge, screen_w );
    }
}

UTEST_F( locale_fixture, screen_stack_text_reuse_drawlist_valid )
{
    vxui_set_locale_font( &utest_fixture->ctx, "en", VXUI_TEST_FONT_UI );
    vxui_set_locale( &utest_fixture->ctx, "en" );

    vxui_push_screen( &utest_fixture->ctx, "screen_a" );
    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "screen_a", {} ) {
        VXUI_LABEL( &utest_fixture->ctx, "label.hello", ( vxui_label_cfg ) { 0 } );
        VXUI_LABEL( &utest_fixture->ctx, "label.left", ( vxui_label_cfg ) { 0 } );
    }
    vxui_end( &utest_fixture->ctx );

    vxui_push_screen( &utest_fixture->ctx, "screen_b" );
    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "screen_b", {} ) {
        VXUI_LABEL( &utest_fixture->ctx, "label.hello", ( vxui_label_cfg ) { 0 } );
    }
    vxui_draw_list list = vxui_end( &utest_fixture->ctx );

    int text_count = 0;
    for ( int i = 0; i < list.length; ++i ) {
        if ( list.commands[ i ].type == VXUI_CMD_TEXT ) {
            text_count++;
            EXPECT_TRUE( list.commands[ i ].text.text != nullptr );
        }
    }
    EXPECT_GE( text_count, 1 );
}

UTEST_F( locale_fixture, repeated_label_keys_both_appear_in_draw_list_without_conflict )
{
    vxui_set_locale_font( &utest_fixture->ctx, "en", VXUI_TEST_FONT_UI );
    vxui_set_locale( &utest_fixture->ctx, "en" );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {
        .layout = {
            .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_GROW( 0 ) },
            .childGap = 8,
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
    } ) {
        VXUI_LABEL( &utest_fixture->ctx, "label.hello", ( vxui_label_cfg ) { 0 } );
        VXUI_LABEL( &utest_fixture->ctx, "label.hello", ( vxui_label_cfg ) { 0 } );
    }
    vxui_draw_list list = vxui_end( &utest_fixture->ctx );

    int text_count = 0;
    for ( int i = 0; i < list.length; ++i ) {
        if ( list.commands[ i ].type == VXUI_CMD_TEXT ) {
            text_count++;
            EXPECT_TRUE( list.commands[ i ].text.text != nullptr );
            EXPECT_GE( list.commands[ i ].text.pos.x, 0.0f );
            EXPECT_GE( list.commands[ i ].text.pos.y, 0.0f );
        }
    }
    EXPECT_EQ( text_count, 2 );
}

UTEST_F( locale_fixture, prompt_pair_helper_uses_compact_fit_sizing )
{
    vxui_input_table table = {
        .confirm = { VXUI_TEST_FONT_ROLE_BODY, 'A' },
        .cancel = { VXUI_TEST_FONT_ROLE_BODY, 'B' },
    };
    vxui_set_input_table( &utest_fixture->ctx, &table );
    vxui_set_locale( &utest_fixture->ctx, "en" );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {
        .layout = {
            .sizing = { CLAY_SIZING_FIXED( 640 ), CLAY_SIZING_GROW( 0 ) },
            .padding = CLAY_PADDING_ALL( 18 ),
            .childGap = 12,
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
    } ) {
        CLAY( CLAY_ID( "prompt_pair" ), {
            .layout = {
                .sizing = { CLAY_SIZING_FIT( 0 ), CLAY_SIZING_FIT( 0 ) },
                .childGap = 8,
                .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                .layoutDirection = CLAY_LEFT_TO_RIGHT,
            },
        } ) {
            VXUI_PROMPT( &utest_fixture->ctx, "action.confirm" );
            VXUI_LABEL( &utest_fixture->ctx, "label.hello", ( vxui_label_cfg ) {
                .font_id = VXUI_TEST_FONT_ROLE_BODY,
                .font_size = 24.0f,
            } );
        }
    }
    vxui_draw_list list = vxui_end( &utest_fixture->ctx );

    vxui_vec2 confirm_pos = {};
    vxui_vec2 label_pos = {};
    ASSERT_TRUE( vxui__locale_find_text_pos( &list, "A", &confirm_pos ) );
    ASSERT_TRUE( vxui__locale_find_text_pos( &list, "Hello", &label_pos ) );
    float pair_width = label_pos.x + 100.0f - confirm_pos.x;
    EXPECT_LT( pair_width, 400.0f );
}

UTEST_F( locale_fixture, label_pair_helper_uses_compact_fit_sizing )
{
    vxui_set_locale( &utest_fixture->ctx, "en" );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {
        .layout = {
            .sizing = { CLAY_SIZING_FIXED( 640 ), CLAY_SIZING_GROW( 0 ) },
            .padding = CLAY_PADDING_ALL( 18 ),
            .childGap = 12,
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
    } ) {
        CLAY( CLAY_ID( "label_pair" ), {
            .layout = {
                .sizing = { CLAY_SIZING_FIT( 0 ), CLAY_SIZING_FIT( 0 ) },
                .childGap = 8,
                .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                .layoutDirection = CLAY_LEFT_TO_RIGHT,
            },
        } ) {
            VXUI_LABEL( &utest_fixture->ctx, "label.hello", ( vxui_label_cfg ) {
                .font_id = VXUI_TEST_FONT_ROLE_BODY,
                .font_size = 24.0f,
            } );
            VXUI_LABEL( &utest_fixture->ctx, "label.right", ( vxui_label_cfg ) {
                .font_id = VXUI_TEST_FONT_ROLE_BODY,
                .font_size = 24.0f,
            } );
        }
    }
    vxui_draw_list list = vxui_end( &utest_fixture->ctx );

    vxui_vec2 hello_pos = {};
    vxui_vec2 right_pos = {};
    ASSERT_TRUE( vxui__locale_find_text_pos( &list, "Hello", &hello_pos ) );
    ASSERT_TRUE( vxui__locale_find_text_pos( &list, "Right", &right_pos ) );
    float pair_width = right_pos.x + 100.0f - hello_pos.x;
    EXPECT_LT( pair_width, 400.0f );
}

UTEST_F( locale_fixture, status_rows_stay_compact )
{
    vxui_input_table table = {
        .confirm = { VXUI_TEST_FONT_ROLE_BODY, 'A' },
    };
    vxui_set_input_table( &utest_fixture->ctx, &table );
    vxui_set_locale( &utest_fixture->ctx, "en" );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {
        .layout = {
            .sizing = { CLAY_SIZING_FIXED( 640 ), CLAY_SIZING_GROW( 0 ) },
            .padding = CLAY_PADDING_ALL( 18 ),
            .childGap = 12,
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
    } ) {
        CLAY( CLAY_ID( "status_compact" ), {
            .layout = {
                .sizing = { CLAY_SIZING_FIT( 0 ), CLAY_SIZING_FIT( 0 ) },
                .childGap = 12,
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
        } ) {
            CLAY( CLAY_ID( "status_row_1" ), {
                .layout = {
                    .sizing = { CLAY_SIZING_FIT( 0 ), CLAY_SIZING_FIT( 0 ) },
                    .childGap = 8,
                    .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                    .layoutDirection = CLAY_LEFT_TO_RIGHT,
                },
            } ) {
                VXUI_LABEL( &utest_fixture->ctx, "label.hello", ( vxui_label_cfg ) {
                    .font_id = VXUI_TEST_FONT_ROLE_BODY,
                    .font_size = 24.0f,
                } );
                VXUI_VALUE( &utest_fixture->ctx, "label.value", 42.0f, ( vxui_value_cfg ) {
                    .font_id = VXUI_TEST_FONT_ROLE_BODY,
                    .font_size = 24.0f,
                    .format = "%.0f",
                } );
            }
            CLAY( CLAY_ID( "status_row_2" ), {
                .layout = {
                    .sizing = { CLAY_SIZING_FIT( 0 ), CLAY_SIZING_FIT( 0 ) },
                    .childGap = 8,
                    .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                    .layoutDirection = CLAY_LEFT_TO_RIGHT,
                },
            } ) {
                VXUI_PROMPT( &utest_fixture->ctx, "action.confirm" );
                VXUI_LABEL( &utest_fixture->ctx, "label.right", ( vxui_label_cfg ) {
                    .font_id = VXUI_TEST_FONT_ROLE_BODY,
                    .font_size = 24.0f,
                } );
            }
        }
    }
    vxui_draw_list list = vxui_end( &utest_fixture->ctx );

    vxui_vec2 row1_start = {};
    vxui_vec2 row2_start = {};
    ASSERT_TRUE( vxui__locale_find_text_pos( &list, "Hello", &row1_start ) );
    ASSERT_TRUE( vxui__locale_find_text_pos( &list, "A", &row2_start ) );
    EXPECT_GT( row2_start.y, row1_start.y );
}

UTEST_F( locale_fixture, form_controls_share_alignment )
{
    vxui_set_locale( &utest_fixture->ctx, "en" );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {
        .layout = {
            .sizing = { CLAY_SIZING_FIXED( 640 ), CLAY_SIZING_GROW( 0 ) },
            .padding = CLAY_PADDING_ALL( 18 ),
            .childGap = 12,
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
    } ) {
        CLAY( CLAY_ID( "form" ), {
            .layout = {
                .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                .childGap = 12,
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
        } ) {
            CLAY( CLAY_ID( "form_row_1" ), {
                .layout = {
                    .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                    .childGap = 12,
                    .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                    .layoutDirection = CLAY_LEFT_TO_RIGHT,
                },
            } ) {
                CLAY( CLAY_ID( "form_row_1_label" ), {
                    .layout = {
                        .sizing = { CLAY_SIZING_FIXED( 140.0f ), CLAY_SIZING_FIT( 0 ) },
                    },
                } ) {
                    VXUI_LABEL( &utest_fixture->ctx, "label.hello", ( vxui_label_cfg ) {
                        .font_id = VXUI_TEST_FONT_ROLE_BODY,
                        .font_size = 24.0f,
                    } );
                }
                VXUI_VALUE( &utest_fixture->ctx, "form.row1.value", 1.0f, ( vxui_value_cfg ) {
                    .font_id = VXUI_TEST_FONT_ROLE_BODY,
                    .font_size = 24.0f,
                    .format = "%.0f",
                } );
            }
            CLAY( CLAY_ID( "form_row_2" ), {
                .layout = {
                    .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                    .childGap = 12,
                    .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                    .layoutDirection = CLAY_LEFT_TO_RIGHT,
                },
            } ) {
                CLAY( CLAY_ID( "form_row_2_label" ), {
                    .layout = {
                        .sizing = { CLAY_SIZING_FIXED( 140.0f ), CLAY_SIZING_FIT( 0 ) },
                    },
                } ) {
                    VXUI_LABEL( &utest_fixture->ctx, "label.right", ( vxui_label_cfg ) {
                        .font_id = VXUI_TEST_FONT_ROLE_BODY,
                        .font_size = 24.0f,
                    } );
                }
                VXUI_VALUE( &utest_fixture->ctx, "form.row2.value", 2.0f, ( vxui_value_cfg ) {
                    .font_id = VXUI_TEST_FONT_ROLE_BODY,
                    .font_size = 24.0f,
                    .format = "%.0f",
                } );
            }
        }
    }
    vxui_draw_list list = vxui_end( &utest_fixture->ctx );

    vxui_vec2 row1_value_pos = {};
    vxui_vec2 row2_value_pos = {};
    ASSERT_TRUE( vxui__locale_find_text_pos( &list, "1", &row1_value_pos ) );
    ASSERT_TRUE( vxui__locale_find_text_pos( &list, "2", &row2_value_pos ) );
    float x_diff = row2_value_pos.x - row1_value_pos.x;
    EXPECT_LT( std::abs( x_diff ), 50.0f );
}

UTEST_F( locale_fixture, main_menu_hero_above_meta )
{
    vxui_input_table table = {
        .confirm = { VXUI_TEST_FONT_ROLE_BODY, 'A' },
    };
    vxui_set_input_table( &utest_fixture->ctx, &table );
    vxui_set_locale( &utest_fixture->ctx, "en" );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {
        .layout = {
            .sizing = { CLAY_SIZING_FIXED( 640 ), CLAY_SIZING_GROW( 0 ) },
            .padding = CLAY_PADDING_ALL( 18 ),
            .childGap = 16,
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
    } ) {
        CLAY( CLAY_ID( "hero" ), {
            .layout = {
                .sizing = { CLAY_SIZING_FIT( 0 ), CLAY_SIZING_FIT( 0 ) },
                .childGap = 0,
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
        } ) {
            VXUI_LABEL( &utest_fixture->ctx, "label.title", ( vxui_label_cfg ) {
                .font_id = VXUI_TEST_FONT_ROLE_TITLE,
                .font_size = 44.0f,
            } );
        }
        CLAY( CLAY_ID( "meta" ), {
            .layout = {
                .sizing = { CLAY_SIZING_FIT( 0 ), CLAY_SIZING_FIT( 0 ) },
                .childGap = 12,
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
        } ) {
            CLAY( CLAY_ID( "meta_prompts" ), {
                .layout = {
                    .sizing = { CLAY_SIZING_FIT( 0 ), CLAY_SIZING_FIT( 0 ) },
                    .childGap = 8,
                    .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                    .layoutDirection = CLAY_LEFT_TO_RIGHT,
                },
            } ) {
                VXUI_PROMPT( &utest_fixture->ctx, "action.confirm" );
                VXUI_LABEL( &utest_fixture->ctx, "label.hello", ( vxui_label_cfg ) {
                    .font_id = VXUI_TEST_FONT_ROLE_BODY,
                    .font_size = 24.0f,
                } );
            }
        }
    }
    vxui_draw_list list = vxui_end( &utest_fixture->ctx );

    vxui_vec2 title_pos = {};
    vxui_vec2 meta_pos = {};
    ASSERT_TRUE( vxui__locale_find_text_pos( &list, "Typography", &title_pos ) );
    ASSERT_TRUE( vxui__locale_find_text_pos( &list, "A", &meta_pos ) );
    EXPECT_LT( title_pos.y, meta_pos.y );
}

UTEST_F( locale_fixture, main_menu_help_below_meta )
{
    vxui_input_table table = {
        .confirm = { VXUI_TEST_FONT_ROLE_BODY, 'A' },
    };
    vxui_set_input_table( &utest_fixture->ctx, &table );
    vxui_set_locale( &utest_fixture->ctx, "en" );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {
        .layout = {
            .sizing = { CLAY_SIZING_FIXED( 640 ), CLAY_SIZING_GROW( 0 ) },
            .padding = CLAY_PADDING_ALL( 18 ),
            .childGap = 16,
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
    } ) {
        CLAY( CLAY_ID( "meta" ), {
            .layout = {
                .sizing = { CLAY_SIZING_FIT( 0 ), CLAY_SIZING_FIT( 0 ) },
                .childGap = 12,
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
        } ) {
            CLAY( CLAY_ID( "meta_prompts" ), {
                .layout = {
                    .sizing = { CLAY_SIZING_FIT( 0 ), CLAY_SIZING_FIT( 0 ) },
                    .childGap = 8,
                    .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                    .layoutDirection = CLAY_LEFT_TO_RIGHT,
                },
            } ) {
                VXUI_PROMPT( &utest_fixture->ctx, "action.confirm" );
                VXUI_LABEL( &utest_fixture->ctx, "label.hello", ( vxui_label_cfg ) {
                    .font_id = VXUI_TEST_FONT_ROLE_BODY,
                    .font_size = 24.0f,
                } );
            }
        }
        CLAY( CLAY_ID( "help" ), {
            .layout = {
                .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                .childGap = 12,
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
        } ) {
            VXUI_LABEL( &utest_fixture->ctx, "label.body", ( vxui_label_cfg ) {
                .font_id = VXUI_TEST_FONT_ROLE_BODY,
                .font_size = 24.0f,
            } );
        }
    }
    vxui_draw_list list = vxui_end( &utest_fixture->ctx );

    vxui_vec2 meta_pos = {};
    vxui_vec2 help_pos = {};
    ASSERT_TRUE( vxui__locale_find_text_pos( &list, "A", &meta_pos ) );
    ASSERT_TRUE( vxui__locale_find_text_pos( &list, "Typography", &help_pos ) );
    EXPECT_LT( meta_pos.y, help_pos.y );
}

UTEST_F( locale_fixture, settings_screen_sections_within_content_column )
{
    vxui_set_locale( &utest_fixture->ctx, "en" );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {
        .layout = {
            .sizing = { CLAY_SIZING_FIXED( 640 ), CLAY_SIZING_GROW( 0 ) },
            .padding = CLAY_PADDING_ALL( 18 ),
        },
    } ) {
        VXUI( &utest_fixture->ctx, "content", {
            .layout = {
                .sizing = { CLAY_SIZING_GROW( 0, 620.0f ), CLAY_SIZING_FIT( 0 ) },
                .childGap = 16,
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
        } ) {
            VXUI( &utest_fixture->ctx, "title", {
                .layout = {
                    .sizing = { CLAY_SIZING_FIT( 0 ), CLAY_SIZING_FIT( 0 ) },
                    .childGap = 0,
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                },
            } ) {
                VXUI_LABEL( &utest_fixture->ctx, "label.title", ( vxui_label_cfg ) {
                    .font_id = VXUI_TEST_FONT_ROLE_TITLE,
                    .font_size = 44.0f,
                } );
            }
            VXUI( &utest_fixture->ctx, "form", {
                .layout = {
                    .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                    .childGap = 12,
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                },
            } ) {
                CLAY( CLAY_ID( "form_row" ), {
                    .layout = {
                        .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                        .childGap = 12,
                        .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                        .layoutDirection = CLAY_LEFT_TO_RIGHT,
                    },
                } ) {
                    CLAY( CLAY_ID( "form_label" ), {
                        .layout = {
                            .sizing = { CLAY_SIZING_FIXED( 140.0f ), CLAY_SIZING_FIT( 0 ) },
                        },
                    } ) {
                        VXUI_LABEL( &utest_fixture->ctx, "label.hello", ( vxui_label_cfg ) {
                            .font_id = VXUI_TEST_FONT_ROLE_BODY,
                            .font_size = 24.0f,
                        } );
                    }
                    VXUI_VALUE( &utest_fixture->ctx, "form.value", 1.0f, ( vxui_value_cfg ) {
                        .font_id = VXUI_TEST_FONT_ROLE_BODY,
                        .font_size = 24.0f,
                        .format = "%.0f",
                    } );
                }
            }
        }
    }
    vxui_draw_list list = vxui_end( &utest_fixture->ctx );

    vxui_vec2 form_value_pos = {};
    ASSERT_TRUE( vxui__locale_find_text_pos( &list, "1", &form_value_pos ) );
    EXPECT_LT( form_value_pos.x, 620.0f );
    EXPECT_GT( form_value_pos.x, 140.0f );
}
