#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "../third_party/utest.h"
#include "../vxui.h"

typedef struct soak_trait_params
{
    float alpha;
} soak_trait_params;

static const char* vxui__soak_text( const char* key, void* userdata )
{
    ( void ) userdata;
    return key;
}

static bool vxui__soak_write_text_file( const char* path, const char* text )
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

static void vxui__soak_overlay_trait( vxui_anim_state* element, vxui_draw_list* draw_list, vxui_rect bounds, float t, bool focused, const void* params )
{
    ( void ) t;
    ( void ) focused;
    const soak_trait_params* cfg = ( const soak_trait_params* ) params;
    if ( !element || !draw_list || !draw_list->commands || draw_list->length <= 0 ) {
        return;
    }

    vxui_cmd* cmd = &draw_list->commands[ draw_list->length - 1 ];
    if ( cmd->type == VXUI_CMD_TEXT ) {
        cmd->text.color.a = ( uint8_t ) ( cfg ? cfg->alpha * 255.0f : 128.0f );
    } else if ( cmd->type == VXUI_CMD_RECT ) {
        cmd->rect.color.a = ( uint8_t ) ( cfg ? cfg->alpha * 255.0f : 128.0f );
    }

    element->bounds = bounds;
}

static bool vxui__soak_init_ctx( vxui_ctx* ctx, uint8_t** memory, uint64_t* memory_size )
{
    *memory_size = vxui_min_memory_size();
    *memory = ( uint8_t* ) std::malloc( ( size_t ) *memory_size );
    if ( !*memory ) {
        return false;
    }

    vxui_init(
        ctx,
        vxui_create_arena( *memory_size, *memory ),
        ( vxui_config ) {
            .screen_width = 640,
            .screen_height = 360,
            .max_elements = 192,
            .max_anim_states = 192,
            .max_sequences = 16,
        } );
    vxui_set_text_fn( ctx, vxui__soak_text, nullptr );
    return true;
}

UTEST( soak, long_frame_loop_over_springs )
{
    uint8_t* memory = nullptr;
    uint64_t memory_size = 0;
    vxui_ctx ctx = {};
    ASSERT_TRUE( vxui__soak_init_ctx( &ctx, &memory, &memory_size ) );

    for ( int frame = 0; frame < 600; ++frame ) {
        vxui_begin( &ctx, 1.0f / 60.0f );
        VXUI( &ctx, "soak.root", {} ) {
            VXUI_LABEL( &ctx, "soak.label", ( vxui_label_cfg ) { 0 } );
        }
        vxui_draw_list list = vxui_end( &ctx );
        EXPECT_TRUE( list.length > 0 );
    }

    Clay_SetCurrentContext( nullptr );
    std::free( memory );
}

UTEST( soak, repeated_push_pop )
{
    uint8_t* memory = nullptr;
    uint64_t memory_size = 0;
    vxui_ctx ctx = {};
    ASSERT_TRUE( vxui__soak_init_ctx( &ctx, &memory, &memory_size ) );

    for ( int i = 0; i < 300; ++i ) {
        vxui_push_screen( &ctx, "menu" );
        vxui_begin( &ctx, 1.0f / 60.0f );
        VXUI( &ctx, "menu", {} ) {
            VXUI_LABEL( &ctx, "menu", ( vxui_label_cfg ) { 0 } );
        }
        ( void ) vxui_end( &ctx );
        vxui_pop_screen( &ctx );
        vxui_begin( &ctx, 1.0f / 60.0f );
        ( void ) vxui_end( &ctx );
    }

    EXPECT_TRUE( ctx.screen_count >= 0 );
    Clay_SetCurrentContext( nullptr );
    std::free( memory );
}

UTEST( soak, repeated_sequence_fire_stop_reload_cycles )
{
    uint8_t* memory = nullptr;
    uint64_t memory_size = 0;
    vxui_ctx ctx = {};
    char temp_path[ 260 ] = {};
    ASSERT_TRUE( vxui__soak_init_ctx( &ctx, &memory, &memory_size ) );
    std::snprintf( temp_path, sizeof( temp_path ), "%s/soak_sequence.toml", VXUI_TEST_TEMP_DIR );

    ASSERT_TRUE( vxui__soak_write_text_file(
        temp_path,
        "[sequence.soak_enter]\n"
        "steps = [\n"
        "  { delay = 0, id = \"menu\", prop = \"opacity\", target = 0.5 },\n"
        "  { delay = 60, id = \"menu\", prop = \"opacity\", target = 1.0 },\n"
        "]\n" ) );
    ASSERT_TRUE( vxui_load_seq_toml( &ctx, temp_path, "soak_enter", nullptr, 0 ) );

#ifdef VXUI_DEBUG
    ASSERT_TRUE( vxui_watch_seq_file( &ctx, temp_path, "soak_enter" ) );
#endif

    for ( int i = 0; i < 120; ++i ) {
        vxui_fire_seq( &ctx, "soak_enter" );
        vxui_begin( &ctx, 1.0f / 60.0f );
        VXUI( &ctx, "menu", {} ) {
            VXUI_LABEL( &ctx, "menu", ( vxui_label_cfg ) { 0 } );
        }
        ( void ) vxui_end( &ctx );
        if ( ( i % 3 ) == 0 ) {
            vxui_stop_seq( &ctx, "soak_enter" );
        }

#ifdef VXUI_DEBUG
        if ( ( i % 20 ) == 0 ) {
            ASSERT_TRUE( vxui__soak_write_text_file(
                temp_path,
                "[sequence.soak_enter]\n"
                "steps = [\n"
                "  { delay = 0, id = \"menu\", prop = \"opacity\", target = 0.5 },\n"
                "  { delay = 60, id = \"menu\", prop = \"opacity\", target = 1.0 },\n"
                "]\n" ) );
            ctx.watched_seq_files[ 0 ].last_write_time = 1;
            char error[ 256 ] = {};
            ( void ) vxui_poll_seq_hot_reload( &ctx, ( uint64_t ) ( i + 1 ) * 300u, error, sizeof( error ) );
        }
#endif
    }

    std::remove( temp_path );
    Clay_SetCurrentContext( nullptr );
    std::free( memory );
}

UTEST( soak, repeated_locale_and_input_table_swaps )
{
    uint8_t* memory = nullptr;
    uint64_t memory_size = 0;
    vxui_ctx ctx = {};
    ASSERT_TRUE( vxui__soak_init_ctx( &ctx, &memory, &memory_size ) );

    vxui_input_table keyboard = {
        .confirm = { 1, 'E' },
        .cancel = { 2, 'Q' },
    };
    vxui_input_table gamepad = {
        .confirm = { 3, 'A' },
        .cancel = { 4, 'B' },
    };

    for ( int i = 0; i < 240; ++i ) {
        vxui_set_locale( &ctx, ( i % 3 ) == 0 ? "en" : ( ( i % 3 ) == 1 ? "ja-JP" : "ar" ) );
        vxui_set_input_table( &ctx, ( i & 1 ) ? &keyboard : &gamepad );
        vxui_begin( &ctx, 1.0f / 60.0f );
        VXUI( &ctx, "locale.root", {} ) {
            VXUI_PROMPT( &ctx, "action.confirm" );
            VXUI_LABEL( &ctx, "swap.label", ( vxui_label_cfg ) { 0 } );
        }
        vxui_draw_list list = vxui_end( &ctx );
        EXPECT_TRUE( list.length > 0 );
    }

    Clay_SetCurrentContext( nullptr );
    std::free( memory );
}

UTEST( soak, repeated_trait_attachment_many_elements )
{
    uint8_t* memory = nullptr;
    uint64_t memory_size = 0;
    vxui_ctx ctx = {};
    ASSERT_TRUE( vxui__soak_init_ctx( &ctx, &memory, &memory_size ) );
    vxui_register_trait( &ctx, 77u, vxui__soak_overlay_trait, sizeof( soak_trait_params ) );

    for ( int frame = 0; frame < 120; ++frame ) {
        vxui_begin( &ctx, 1.0f / 60.0f );
        VXUI( &ctx, "traits.root", {} ) {
            for ( int i = 0; i < 48; ++i ) {
                char id[ 32 ] = {};
                std::snprintf( id, sizeof( id ), "trait.%d", i );
                VXUI_LABEL( &ctx, id, ( vxui_label_cfg ) { 0 } );
                VXUI_TRAIT( 77u, ( soak_trait_params ) { .alpha = 0.5f } );
            }
        }
        vxui_draw_list list = vxui_end( &ctx );
        EXPECT_TRUE( list.length > 0 );
    }

    Clay_SetCurrentContext( nullptr );
    std::free( memory );
}

UTEST( soak, navigation_spam_across_asymmetric_layouts )
{
    uint8_t* memory = nullptr;
    uint64_t memory_size = 0;
    vxui_ctx ctx = {};
    ASSERT_TRUE( vxui__soak_init_ctx( &ctx, &memory, &memory_size ) );

    for ( int frame = 0; frame < 180; ++frame ) {
        vxui_begin( &ctx, 1.0f / 60.0f );
        vxui_input_nav( &ctx, ( vxui_dir ) ( frame % 4 ) );
        VXUI( &ctx, "nav.root", {
            .layout = {
                .childGap = 12,
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
        } ) {
            VXUI_ACTION( &ctx, "nav.a", "nav.a", nullptr, ( vxui_action_cfg ) { 0 } );
            VXUI( &ctx, "nav.row", {
                .layout = {
                    .childGap = 20,
                    .layoutDirection = CLAY_LEFT_TO_RIGHT,
                },
            } ) {
                VXUI_ACTION( &ctx, "nav.b", "nav.b", nullptr, ( vxui_action_cfg ) { 0 } );
                VXUI_ACTION( &ctx, "nav.c", "nav.c", nullptr, ( vxui_action_cfg ) { 0 } );
            }
            VXUI_ACTION( &ctx, "nav.d", "nav.d", nullptr, ( vxui_action_cfg ) { 0 } );
        }
        vxui_draw_list list = vxui_end( &ctx );
        EXPECT_TRUE( list.length > 0 );
    }

    Clay_SetCurrentContext( nullptr );
    std::free( memory );
}
