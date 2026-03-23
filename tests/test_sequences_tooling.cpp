#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "../third_party/utest.h"
#include "test_support.h"
#include "../vxui.h"

typedef struct sequences_tooling_fixture
{
    uint8_t* memory;
    uint64_t memory_size;
    vxui_ctx ctx;
    char temp_path[ 260 ];
} sequences_tooling_fixture;

UTEST_F_SETUP( sequences_tooling_fixture )
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

    std::string temp_path = vxui_test_temp_path( "sequence_hot_reload.toml" );
    std::snprintf( utest_fixture->temp_path, sizeof( utest_fixture->temp_path ), "%s", temp_path.c_str() );
}

UTEST_F_TEARDOWN( sequences_tooling_fixture )
{
    Clay_SetCurrentContext( nullptr );
    std::remove( utest_fixture->temp_path );
    std::free( utest_fixture->memory );
    utest_fixture->memory = nullptr;
    utest_fixture->memory_size = 0;
}

UTEST_F( sequences_tooling_fixture, valid_toml_parses_into_expected_step_count )
{
    char error[ 256 ] = {};
    ASSERT_TRUE( vxui_load_seq_toml( &utest_fixture->ctx, VXUI_TEST_DATA_DIR "/sequence_valid.toml", nullptr, error, sizeof( error ) ) );
    ASSERT_TRUE( error[ 0 ] == '\0' );

    const vxui_registered_seq* seq = vxui_find_seq( &utest_fixture->ctx, "main_menu_enter" );
    ASSERT_TRUE( seq != nullptr );
    EXPECT_EQ( seq->count, 3 );
    EXPECT_EQ( seq->steps[ 0 ].id, vxui_id( "title" ) );
    EXPECT_EQ( seq->steps[ 1 ].prop, VXUI_PROP_SLIDE_Y );
}

UTEST_F( sequences_tooling_fixture, generated_toml_contains_expected_keys )
{
    char error[ 256 ] = {};
    char generated[ 8192 ] = {};

    ASSERT_TRUE( vxui_load_seq_toml( &utest_fixture->ctx, VXUI_TEST_DATA_DIR "/sequence_valid.toml", "main_menu_enter", error, sizeof( error ) ) );
    const vxui_registered_seq* seq = vxui_find_seq( &utest_fixture->ctx, "main_menu_enter" );
    ASSERT_TRUE( seq != nullptr );
    ASSERT_TRUE( vxui_generate_seq_toml( seq, generated, sizeof( generated ) ) );
    EXPECT_TRUE( std::strstr( generated, "[sequence.main_menu_enter]" ) != nullptr );
    EXPECT_TRUE( std::strstr( generated, "prop = \"opacity\"" ) != nullptr );
    EXPECT_TRUE( std::strstr( generated, "id = \"title\"" ) != nullptr );
}

UTEST_F( sequences_tooling_fixture, invalid_toml_does_not_replace_prior_good_sequence_data )
{
    char error[ 256 ] = {};
    ASSERT_TRUE( vxui_load_seq_toml( &utest_fixture->ctx, VXUI_TEST_DATA_DIR "/sequence_valid.toml", "main_menu_enter", error, sizeof( error ) ) );

    const vxui_registered_seq* before = vxui_find_seq( &utest_fixture->ctx, "main_menu_enter" );
    ASSERT_TRUE( before != nullptr );
    float old_target = before->steps[ 1 ].target;

    EXPECT_FALSE( vxui_load_seq_toml( &utest_fixture->ctx, VXUI_TEST_DATA_DIR "/sequence_invalid.toml", "main_menu_enter", error, sizeof( error ) ) );
    ASSERT_TRUE( error[ 0 ] != '\0' );

    const vxui_registered_seq* after = vxui_find_seq( &utest_fixture->ctx, "main_menu_enter" );
    ASSERT_TRUE( after != nullptr );
    EXPECT_EQ( after->steps[ 1 ].target, old_target );
}

UTEST_F( sequences_tooling_fixture, missing_required_field_fails_parse )
{
    char error[ 256 ] = {};
    ASSERT_TRUE( vxui_test_write_text_file(
        utest_fixture->temp_path,
        "[sequence.main_menu_enter]\nsteps = [\n  { id = \"title\", prop = \"opacity\", target = 1.0 },\n]\n" ) );

    EXPECT_FALSE( vxui_load_seq_toml( &utest_fixture->ctx, utest_fixture->temp_path, nullptr, error, sizeof( error ) ) );
    EXPECT_TRUE( std::strstr( error, "missing required field" ) != nullptr );
}

UTEST_F( sequences_tooling_fixture, unknown_property_fails_parse )
{
    char error[ 256 ] = {};
    ASSERT_TRUE( vxui_test_write_text_file(
        utest_fixture->temp_path,
        "[sequence.main_menu_enter]\nsteps = [\n  { delay = 0, id = \"title\", prop = \"shear\", target = 1.0 },\n]\n" ) );

    EXPECT_FALSE( vxui_load_seq_toml( &utest_fixture->ctx, utest_fixture->temp_path, nullptr, error, sizeof( error ) ) );
    EXPECT_TRUE( std::strstr( error, "unknown prop" ) != nullptr );
}

#ifdef VXUI_DEBUG
UTEST_F( sequences_tooling_fixture, hot_reload_while_preview_sequence_is_active_does_not_corrupt_runtime_state )
{
    char error[ 256 ] = {};
    ASSERT_TRUE( vxui_test_write_text_file( utest_fixture->temp_path, "[sequence.main_menu_enter]\nsteps = [\n  { delay = 0, id = \"title\", prop = \"opacity\", target = 0.25 },\n  { delay = 80, id = \"title\", prop = \"opacity\", target = 1.0 },\n]\n" ) );
    ASSERT_TRUE( vxui_watch_seq_file( &utest_fixture->ctx, utest_fixture->temp_path, "main_menu_enter" ) );
    ASSERT_TRUE( vxui_load_seq_toml( &utest_fixture->ctx, utest_fixture->temp_path, "main_menu_enter", error, sizeof( error ) ) );

    vxui_push_screen( &utest_fixture->ctx, "main_menu" );
    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "main_menu", {} ) {
        VXUI_LABEL( &utest_fixture->ctx, "title", ( vxui_label_cfg ) { 0 } );
    }
    vxui_draw_list list = vxui_end( &utest_fixture->ctx );
    vxui_debug_capture_preview( &utest_fixture->ctx, &list );
    utest_fixture->ctx.debug_seq_editor.preview_playing = true;
    vxui_fire_seq( &utest_fixture->ctx, "main_menu_enter" );
    ASSERT_TRUE( vxui_seq_playing( &utest_fixture->ctx, "main_menu_enter" ) );

    ASSERT_TRUE( vxui_test_write_text_file( utest_fixture->temp_path, "[sequence.main_menu_enter]\nsteps = [\n  { delay = 0, id = \"title\", prop = \"opacity\", target = 0.75 },\n  { delay = 80, id = \"title\", prop = \"opacity\", target = 1.0 },\n]\n" ) );
    utest_fixture->ctx.watched_seq_files[ 0 ].last_write_time = 1;
    ASSERT_TRUE( vxui_poll_seq_hot_reload( &utest_fixture->ctx, 300, error, sizeof( error ) ) );

    const vxui_registered_seq* seq = vxui_find_seq( &utest_fixture->ctx, "main_menu_enter" );
    ASSERT_TRUE( seq != nullptr );
    EXPECT_EQ( seq->steps[ 0 ].target, 0.75f );
    EXPECT_TRUE( utest_fixture->ctx.debug_seq_editor.preview_snapshot.command_count > 0 );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "main_menu", {} ) {
        VXUI_LABEL( &utest_fixture->ctx, "title", ( vxui_label_cfg ) { 0 } );
    }
    list = vxui_end( &utest_fixture->ctx );
    EXPECT_TRUE( list.length > 0 );
}

UTEST_F( sequences_tooling_fixture, generated_outputs_refresh_after_debug_editor_changes )
{
    char error[ 256 ] = {};
    ASSERT_TRUE( vxui_load_seq_toml( &utest_fixture->ctx, VXUI_TEST_DATA_DIR "/sequence_valid.toml", "main_menu_enter", error, sizeof( error ) ) );
    utest_fixture->ctx.debug_seq_editor.selected_seq = 0;

    vxui_registered_seq* seq = &utest_fixture->ctx.registered_seqs[ 0 ];
    seq->steps[ 0 ].prop = VXUI_PROP_SLIDE_X;
    seq->steps[ 0 ].target = 12.5f;
    vxui_debug_generate_seq_outputs( &utest_fixture->ctx );

    EXPECT_TRUE( std::strstr( utest_fixture->ctx.debug_seq_editor.generated_c, "VXUI_PROP_SLIDE_X" ) != nullptr );
    EXPECT_TRUE( std::strstr( utest_fixture->ctx.debug_seq_editor.generated_toml, "target = 12.500" ) != nullptr );
}

UTEST_F( sequences_tooling_fixture, failed_hot_reload_keeps_prior_sequence_and_outputs )
{
    char error[ 256 ] = {};
    ASSERT_TRUE( vxui_test_write_text_file( utest_fixture->temp_path, "[sequence.main_menu_enter]\nsteps = [\n  { delay = 0, id = \"title\", prop = \"opacity\", target = 0.25 },\n]\n" ) );
    ASSERT_TRUE( vxui_watch_seq_file( &utest_fixture->ctx, utest_fixture->temp_path, "main_menu_enter" ) );
    ASSERT_TRUE( vxui_load_seq_toml( &utest_fixture->ctx, utest_fixture->temp_path, "main_menu_enter", error, sizeof( error ) ) );
    utest_fixture->ctx.debug_seq_editor.selected_seq = 0;
    vxui_debug_generate_seq_outputs( &utest_fixture->ctx );

    ASSERT_TRUE( vxui_test_write_text_file( utest_fixture->temp_path, "[sequence.main_menu_enter]\nsteps = [\n  { id = \"title\", prop = \"opacity\", target = 1.0 },\n]\n" ) );
    utest_fixture->ctx.watched_seq_files[ 0 ].last_write_time = 1;
    EXPECT_FALSE( vxui_poll_seq_hot_reload( &utest_fixture->ctx, 400, error, sizeof( error ) ) );
    EXPECT_TRUE( error[ 0 ] != '\0' );

    const vxui_registered_seq* seq = vxui_find_seq( &utest_fixture->ctx, "main_menu_enter" );
    ASSERT_TRUE( seq != nullptr );
    EXPECT_EQ( seq->steps[ 0 ].target, 0.25f );
    EXPECT_TRUE( std::strstr( utest_fixture->ctx.debug_seq_editor.generated_toml, "target = 0.250" ) != nullptr );
}
#endif
