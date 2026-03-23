#include <cstdint>
#include <cstdlib>

#include "../third_party/utest.h"
#include "../vxui.h"

typedef struct clay_bridge_fixture
{
    uint8_t* memory;
    uint64_t memory_size;
    vxui_ctx ctx;
} clay_bridge_fixture;

UTEST_F_SETUP( clay_bridge_fixture )
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

UTEST_F_TEARDOWN( clay_bridge_fixture )
{
    Clay_SetCurrentContext( nullptr );
    std::free( utest_fixture->memory );
    utest_fixture->memory = nullptr;
    utest_fixture->memory_size = 0;
}

UTEST_F( clay_bridge_fixture, translates_rectangle )
{
    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {
        .layout = {
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
    } ) {
        VXUI( &utest_fixture->ctx, "rect", {
            .layout = {
                .sizing = { CLAY_SIZING_FIXED( 40 ), CLAY_SIZING_FIXED( 20 ) },
            },
            .backgroundColor = { 10, 20, 30, 255 },
        } ) {}
    }

    vxui_draw_list list = vxui_end( &utest_fixture->ctx );
    ASSERT_TRUE( list.length >= 1 );
    EXPECT_EQ( list.commands[ 0 ].type, VXUI_CMD_RECT );
}

UTEST_F( clay_bridge_fixture, translates_rounded_rectangle )
{
    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {} ) {
        VXUI( &utest_fixture->ctx, "rounded", {
            .layout = {
                .sizing = { CLAY_SIZING_FIXED( 32 ), CLAY_SIZING_FIXED( 18 ) },
            },
            .backgroundColor = { 100, 110, 120, 255 },
            .cornerRadius = CLAY_CORNER_RADIUS( 6 ),
        } ) {}
    }

    vxui_draw_list list = vxui_end( &utest_fixture->ctx );
    ASSERT_TRUE( list.length >= 1 );
    EXPECT_EQ( list.commands[ 0 ].type, VXUI_CMD_RECT_ROUNDED );
    EXPECT_EQ( list.commands[ 0 ].rect_rounded.radius, 6.0f );
}

UTEST_F( clay_bridge_fixture, text_becomes_queue_item_and_flush_marker )
{
    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {} ) {
        CLAY_TEXT(
            CLAY_STRING( "hello" ),
            CLAY_TEXT_CONFIG( {
                .textColor = { 255, 255, 0, 255 },
                .fontId = 0,
                .fontSize = 18,
            } ) );
    }

    vxui_draw_list list = vxui_end( &utest_fixture->ctx );
    ASSERT_TRUE( list.length >= 1 );
    EXPECT_EQ( list.commands[ 0 ].type, VXUI_CMD_TEXT );
    EXPECT_EQ( utest_fixture->ctx.text_queue_count, 1 );
    EXPECT_STREQ( utest_fixture->ctx.text_queue[ 0 ].text, "hello" );
}

UTEST_F( clay_bridge_fixture, flush_text_drains_queue_without_touching_draw_commands )
{
    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {} ) {
        CLAY_TEXT(
            CLAY_STRING( "hello" ),
            CLAY_TEXT_CONFIG( {
                .textColor = { 255, 255, 255, 255 },
                .fontId = 0,
                .fontSize = 18,
            } ) );
    }

    vxui_draw_list list = vxui_end( &utest_fixture->ctx );
    ASSERT_TRUE( list.length >= 1 );
    ASSERT_EQ( utest_fixture->ctx.text_queue_count, 1 );

    vxui_flush_text( &utest_fixture->ctx );
    EXPECT_EQ( utest_fixture->ctx.text_queue_count, 0 );
    EXPECT_EQ( list.length, 1 );
    EXPECT_EQ( list.commands[ 0 ].type, VXUI_CMD_TEXT );
    EXPECT_STREQ( list.commands[ 0 ].text.text, "hello" );

    vxui_flush_text( &utest_fixture->ctx );
    EXPECT_EQ( utest_fixture->ctx.text_queue_count, 0 );
}

UTEST_F( clay_bridge_fixture, image_payload_survives_translation )
{
    int payload = 42;

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {} ) {
        VXUI( &utest_fixture->ctx, "image", {
            .layout = {
                .sizing = { CLAY_SIZING_FIXED( 64 ), CLAY_SIZING_FIXED( 32 ) },
            },
            .image = {
                .imageData = &payload,
            },
        } ) {}
    }

    vxui_draw_list list = vxui_end( &utest_fixture->ctx );
    ASSERT_TRUE( list.length >= 1 );
    EXPECT_EQ( list.commands[ 0 ].type, VXUI_CMD_IMAGE );
    EXPECT_EQ( list.commands[ 0 ].image.image_data, &payload );
}

UTEST_F( clay_bridge_fixture, translates_clip_pairs )
{
    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {
        .layout = {
            .sizing = { CLAY_SIZING_FIXED( 100 ), CLAY_SIZING_FIXED( 50 ) },
        },
        .clip = {
            .horizontal = true,
            .vertical = true,
        },
    } ) {
        VXUI( &utest_fixture->ctx, "child", {
            .layout = {
                .sizing = { CLAY_SIZING_FIXED( 20 ), CLAY_SIZING_FIXED( 20 ) },
            },
            .backgroundColor = { 255, 0, 0, 255 },
        } ) {}
    }

    vxui_draw_list list = vxui_end( &utest_fixture->ctx );
    ASSERT_TRUE( list.length >= 3 );
    EXPECT_EQ( list.commands[ 0 ].type, VXUI_CMD_CLIP_PUSH );
    EXPECT_EQ( list.commands[ list.length - 1 ].type, VXUI_CMD_CLIP_POP );
}

UTEST_F( clay_bridge_fixture, nested_clip_stacks_remain_balanced )
{
    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "outer", {
        .layout = {
            .sizing = { CLAY_SIZING_FIXED( 100 ), CLAY_SIZING_FIXED( 80 ) },
        },
        .clip = {
            .horizontal = true,
            .vertical = true,
        },
    } ) {
        VXUI( &utest_fixture->ctx, "inner", {
            .layout = {
                .sizing = { CLAY_SIZING_FIXED( 60 ), CLAY_SIZING_FIXED( 40 ) },
            },
            .clip = {
                .horizontal = true,
                .vertical = true,
            },
        } ) {
            VXUI( &utest_fixture->ctx, "rect", {
                .layout = {
                    .sizing = { CLAY_SIZING_FIXED( 12 ), CLAY_SIZING_FIXED( 12 ) },
                },
                .backgroundColor = { 0, 255, 0, 255 },
            } ) {}
        }
    }

    vxui_draw_list list = vxui_end( &utest_fixture->ctx );
    int pushes = 0;
    int pops = 0;
    for ( int i = 0; i < list.length; ++i ) {
        pushes += list.commands[ i ].type == VXUI_CMD_CLIP_PUSH ? 1 : 0;
        pops += list.commands[ i ].type == VXUI_CMD_CLIP_POP ? 1 : 0;
    }

    EXPECT_EQ( pushes, 2 );
    EXPECT_EQ( pops, 2 );
}

UTEST_F( clay_bridge_fixture, mixed_command_order_is_preserved )
{
    int payload = 9;

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {
        .layout = {
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
    } ) {
        VXUI( &utest_fixture->ctx, "rect", {
            .layout = {
                .sizing = { CLAY_SIZING_FIXED( 20 ), CLAY_SIZING_FIXED( 10 ) },
            },
            .backgroundColor = { 20, 40, 60, 255 },
        } ) {}

        CLAY_TEXT(
            CLAY_STRING( "order" ),
            CLAY_TEXT_CONFIG( {
                .textColor = { 200, 200, 200, 255 },
                .fontId = 0,
                .fontSize = 16,
            } ) );

        VXUI( &utest_fixture->ctx, "image", {
            .layout = {
                .sizing = { CLAY_SIZING_FIXED( 30 ), CLAY_SIZING_FIXED( 12 ) },
            },
            .image = {
                .imageData = &payload,
            },
        } ) {}
    }

    vxui_draw_list list = vxui_end( &utest_fixture->ctx );
    ASSERT_TRUE( list.length >= 3 );
    EXPECT_EQ( list.commands[ 0 ].type, VXUI_CMD_RECT );
    EXPECT_EQ( list.commands[ 1 ].type, VXUI_CMD_TEXT );
    EXPECT_EQ( list.commands[ 2 ].type, VXUI_CMD_IMAGE );
}

UTEST_F( clay_bridge_fixture, empty_layout_emits_no_bogus_commands )
{
    vxui_begin( &utest_fixture->ctx, 0.016f );
    vxui_draw_list list = vxui_end( &utest_fixture->ctx );

    EXPECT_EQ( list.length, 0 );
    EXPECT_EQ( utest_fixture->ctx.text_queue_count, 0 );
}

UTEST_F( clay_bridge_fixture, repeated_layout_yields_stable_counts )
{
    int lengths[ 2 ] = {};
    int text_counts[ 2 ] = {};

    for ( int i = 0; i < 2; ++i ) {
        vxui_begin( &utest_fixture->ctx, 0.016f );
        VXUI( &utest_fixture->ctx, "root", {} ) {
            VXUI( &utest_fixture->ctx, "rect", {
                .layout = {
                    .sizing = { CLAY_SIZING_FIXED( 10 ), CLAY_SIZING_FIXED( 10 ) },
                },
                .backgroundColor = { 255, 255, 255, 255 },
            } ) {}
            CLAY_TEXT(
                CLAY_STRING( "stable" ),
                CLAY_TEXT_CONFIG( {
                    .textColor = { 255, 255, 255, 255 },
                    .fontId = 0,
                    .fontSize = 14,
                } ) );
        }

        vxui_draw_list list = vxui_end( &utest_fixture->ctx );
        lengths[ i ] = list.length;
        text_counts[ i ] = utest_fixture->ctx.text_queue_count;
    }

    EXPECT_EQ( lengths[ 0 ], lengths[ 1 ] );
    EXPECT_EQ( text_counts[ 0 ], text_counts[ 1 ] );
}
