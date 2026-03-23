#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "../third_party/utest.h"
#include "../vxui.h"

enum
{
    VXUI_TEST_FONT_UI = 0,
    VXUI_TEST_FONT_TITLE = 1,
    VXUI_TEST_FONT_DEBUG = 2,
    VXUI_TEST_FONT_JAPANESE = 3,
    VXUI_TEST_FONT_ARABIC = 4,
};

typedef struct locale_fixture
{
    uint8_t* memory;
    uint64_t memory_size;
    vxui_ctx ctx;
    const char* long_text;
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
}

UTEST_F_TEARDOWN( locale_fixture )
{
    Clay_SetCurrentContext( nullptr );
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

UTEST_F( locale_fixture, ids_remain_stable_while_rtl_flips_layout_direction )
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
    EXPECT_TRUE( left_x_rtl > right_x_rtl );
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
