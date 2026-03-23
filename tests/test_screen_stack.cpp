#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "../third_party/utest.h"
#include "../vxui.h"

typedef struct screen_stack_fixture
{
    uint8_t* memory;
    uint64_t memory_size;
    vxui_ctx ctx;
} screen_stack_fixture;

static const char* vxui__screen_stack_text( const char* key, void* userdata )
{
    ( void ) userdata;

    if ( std::strcmp( key, "screen.main" ) == 0 ) {
        return "Main Menu";
    }
    if ( std::strcmp( key, "screen.settings" ) == 0 ) {
        return "Settings";
    }
    if ( std::strcmp( key, "screen.back" ) == 0 ) {
        return "Back";
    }

    return key;
}

static vxui_anim_state* vxui__screen_stack_find_anim( vxui_ctx* ctx, uint32_t id )
{
    for ( int i = 0; i < ctx->anim_capacity; ++i ) {
        if ( ctx->anim_slots[ i ].occupied && ctx->anim_slots[ i ].state.id == id ) {
            return &ctx->anim_slots[ i ].state;
        }
    }
    return nullptr;
}

static bool vxui__screen_stack_has_text( const vxui_draw_list* list, const char* text )
{
    for ( int i = 0; i < list->length; ++i ) {
        if ( list->commands[ i ].type == VXUI_CMD_TEXT && list->commands[ i ].text.text && std::strcmp( list->commands[ i ].text.text, text ) == 0 ) {
            return true;
        }
    }
    return false;
}

UTEST_F_SETUP( screen_stack_fixture )
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
            .max_elements = 128,
            .max_anim_states = 128,
            .max_sequences = 16,
        } );
    vxui_set_text_fn( &utest_fixture->ctx, vxui__screen_stack_text, nullptr );
}

UTEST_F_TEARDOWN( screen_stack_fixture )
{
    Clay_SetCurrentContext( nullptr );
    std::free( utest_fixture->memory );
    utest_fixture->memory = nullptr;
    utest_fixture->memory_size = 0;
}

UTEST_F( screen_stack_fixture, push_adds_entering_screen )
{
    vxui_push_screen( &utest_fixture->ctx, "main_menu" );

    ASSERT_EQ( utest_fixture->ctx.screen_count, 1 );
    EXPECT_EQ( utest_fixture->ctx.screens[ 0 ].state, VXUI_SCREEN_ENTERING );
    EXPECT_EQ( utest_fixture->ctx.screens[ 0 ].name_id, vxui_id( "main_menu" ) );
}

UTEST_F( screen_stack_fixture, pop_marks_top_screen_exiting )
{
    vxui_push_screen( &utest_fixture->ctx, "settings" );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "settings", {} ) {
        VXUI_LABEL( &utest_fixture->ctx, "screen.settings", ( vxui_label_cfg ) { 0 } );
    }
    vxui_end( &utest_fixture->ctx );

    vxui_pop_screen( &utest_fixture->ctx );

    EXPECT_EQ( utest_fixture->ctx.screens[ 0 ].state, VXUI_SCREEN_EXITING );
    EXPECT_TRUE( utest_fixture->ctx.screens[ 0 ].snapshot.command_count > 0 );
}

UTEST_F( screen_stack_fixture, replace_swaps_without_empty_stack )
{
    vxui_push_screen( &utest_fixture->ctx, "main_menu" );
    vxui_replace_screen( &utest_fixture->ctx, "settings" );

    ASSERT_EQ( utest_fixture->ctx.screen_count, 1 );
    EXPECT_EQ( utest_fixture->ctx.screens[ 0 ].name_id, vxui_id( "settings" ) );
    EXPECT_EQ( utest_fixture->ctx.screens[ 0 ].state, VXUI_SCREEN_ENTERING );
}

UTEST_F( screen_stack_fixture, missing_sequence_name_is_safe )
{
    vxui_fire_seq( &utest_fixture->ctx, "missing" );
    vxui_stop_seq( &utest_fixture->ctx, "missing" );
    EXPECT_FALSE( vxui_seq_playing( &utest_fixture->ctx, "missing" ) );
}

UTEST_F( screen_stack_fixture, snapshot_copies_text_payloads_safely )
{
    vxui_push_screen( &utest_fixture->ctx, "settings" );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "settings", {} ) {
        VXUI_LABEL( &utest_fixture->ctx, "screen.settings", ( vxui_label_cfg ) { 0 } );
    }
    vxui_end( &utest_fixture->ctx );
    vxui_pop_screen( &utest_fixture->ctx );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    vxui_draw_list list = vxui_end( &utest_fixture->ctx );

    EXPECT_TRUE( vxui__screen_stack_has_text( &list, "Settings" ) );
}

UTEST_F( screen_stack_fixture, exiting_screen_remains_renderable_after_live_declarations_stop )
{
    vxui_push_screen( &utest_fixture->ctx, "settings" );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "settings", {} ) {
        VXUI_LABEL( &utest_fixture->ctx, "screen.settings", ( vxui_label_cfg ) { 0 } );
    }
    vxui_end( &utest_fixture->ctx );
    vxui_pop_screen( &utest_fixture->ctx );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    vxui_draw_list list = vxui_end( &utest_fixture->ctx );

    EXPECT_TRUE( list.length > 0 );
    EXPECT_TRUE( vxui__screen_stack_has_text( &list, "Settings" ) );
}

UTEST_F( screen_stack_fixture, focus_transfers_to_next_screen_on_pop )
{
    vxui_push_screen( &utest_fixture->ctx, "main_menu" );
    vxui_push_screen( &utest_fixture->ctx, "settings" );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "settings", {} ) {
        VXUI_ACTION( &utest_fixture->ctx, "settings.back", "screen.back", nullptr, ( vxui_action_cfg ) { 0 } );
    }
    vxui_end( &utest_fixture->ctx );
    vxui_pop_screen( &utest_fixture->ctx );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "main_menu", {} ) {
        VXUI_ACTION( &utest_fixture->ctx, "main.open", "screen.main", nullptr, ( vxui_action_cfg ) { 0 } );
    }
    vxui_end( &utest_fixture->ctx );

    EXPECT_EQ( vxui_focused_id( &utest_fixture->ctx ), vxui_id( "main.open" ) );
}

UTEST_F( screen_stack_fixture, sequence_refire_reuses_single_active_slot )
{
    vxui_seq_step steps[] = {
        { 0, vxui_id( "main_menu" ), VXUI_PROP_OPACITY, 0.5f },
        { 50, vxui_id( "main_menu" ), VXUI_PROP_OPACITY, 1.0f },
    };

    vxui_register_seq( &utest_fixture->ctx, "main_menu_enter", steps, 2 );
    vxui_fire_seq( &utest_fixture->ctx, "main_menu_enter" );
    vxui_begin( &utest_fixture->ctx, 0.016f );
    vxui_end( &utest_fixture->ctx );
    vxui_fire_seq( &utest_fixture->ctx, "main_menu_enter" );

    EXPECT_EQ( utest_fixture->ctx.active_seq_count, 1 );
    EXPECT_TRUE( vxui_seq_playing( &utest_fixture->ctx, "main_menu_enter" ) );
}

UTEST_F( screen_stack_fixture, stopping_sequence_halts_future_steps )
{
    uint32_t target_id = vxui_id( "settings" );
    vxui_seq_step steps[] = {
        { 0, target_id, VXUI_PROP_SLIDE_X, 12.0f },
        { 80, target_id, VXUI_PROP_SLIDE_X, 48.0f },
    };

    vxui_register_seq( &utest_fixture->ctx, "settings_enter", steps, 2 );
    vxui_fire_seq( &utest_fixture->ctx, "settings_enter" );

    vxui_begin( &utest_fixture->ctx, 0.020f );
    vxui_end( &utest_fixture->ctx );
    vxui_stop_seq( &utest_fixture->ctx, "settings_enter" );

    vxui_begin( &utest_fixture->ctx, 0.200f );
    vxui_end( &utest_fixture->ctx );

    vxui_anim_state* anim = vxui__screen_stack_find_anim( &utest_fixture->ctx, target_id );
    ASSERT_TRUE( anim != nullptr );
    EXPECT_FALSE( vxui_seq_playing( &utest_fixture->ctx, "settings_enter" ) );
    EXPECT_TRUE( std::fabs( anim->slide_x_target - 48.0f ) > 0.0001f );
}

UTEST_F( screen_stack_fixture, active_and_exiting_screens_coexist_in_draw_list )
{
    vxui_push_screen( &utest_fixture->ctx, "main_menu" );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "main_menu", {} ) {
        VXUI_LABEL( &utest_fixture->ctx, "screen.main", ( vxui_label_cfg ) { 0 } );
    }
    vxui_end( &utest_fixture->ctx );

    vxui_push_screen( &utest_fixture->ctx, "settings" );
    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "settings", {} ) {
        VXUI_LABEL( &utest_fixture->ctx, "screen.settings", ( vxui_label_cfg ) { 0 } );
    }
    vxui_end( &utest_fixture->ctx );
    vxui_pop_screen( &utest_fixture->ctx );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "main_menu", {} ) {
        VXUI_LABEL( &utest_fixture->ctx, "screen.main", ( vxui_label_cfg ) { 0 } );
    }
    vxui_draw_list list = vxui_end( &utest_fixture->ctx );

    EXPECT_TRUE( vxui__screen_stack_has_text( &list, "Main Menu" ) );
    EXPECT_TRUE( vxui__screen_stack_has_text( &list, "Settings" ) );
}

UTEST_F( screen_stack_fixture, pop_keeps_exiting_snapshot_while_focus_returns_to_main_screen )
{
    vxui_push_screen( &utest_fixture->ctx, "main_menu" );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "main_menu", {} ) {
        VXUI_ACTION( &utest_fixture->ctx, "main.open", "screen.main", nullptr, ( vxui_action_cfg ) { 0 } );
    }
    vxui_end( &utest_fixture->ctx );

    vxui_push_screen( &utest_fixture->ctx, "settings" );
    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "settings", {} ) {
        VXUI_LABEL( &utest_fixture->ctx, "screen.settings", ( vxui_label_cfg ) { 0 } );
        VXUI_ACTION( &utest_fixture->ctx, "settings.back", "screen.back", nullptr, ( vxui_action_cfg ) { 0 } );
    }
    vxui_end( &utest_fixture->ctx );
    vxui_pop_screen( &utest_fixture->ctx );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "main_menu", {} ) {
        VXUI_ACTION( &utest_fixture->ctx, "main.open", "screen.main", nullptr, ( vxui_action_cfg ) { 0 } );
    }
    vxui_draw_list list = vxui_end( &utest_fixture->ctx );

    EXPECT_EQ( vxui_focused_id( &utest_fixture->ctx ), vxui_id( "main.open" ) );
    EXPECT_TRUE( vxui__screen_stack_has_text( &list, "Settings" ) );
}
