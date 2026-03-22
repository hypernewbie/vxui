#include <cstdint>
#include <cstdlib>

#include "../third_party/utest.h"
#include "../vxui.h"

typedef struct motion_fixture
{
    uint8_t* memory;
    uint64_t memory_size;
    vxui_ctx ctx;
} motion_fixture;

static const char* vxui__motion_text_fn( const char* key, void* userdata )
{
    ( void ) userdata;
    if ( std::strcmp( key, "motion.label" ) == 0 ) {
        return "Motion";
    }
    return key;
}

static vxui_anim_state* vxui__find_anim_state( vxui_ctx* ctx, uint32_t id )
{
    for ( int i = 0; i < ctx->anim_capacity; ++i ) {
        if ( ctx->anim_slots[ i ].occupied && ctx->anim_slots[ i ].state.id == id ) {
            return &ctx->anim_slots[ i ].state;
        }
    }
    return nullptr;
}

static int vxui__count_anim_states( vxui_ctx* ctx )
{
    int count = 0;
    for ( int i = 0; i < ctx->anim_capacity; ++i ) {
        count += ctx->anim_slots[ i ].occupied ? 1 : 0;
    }
    return count;
}

UTEST_F_SETUP( motion_fixture )
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
            .exit_anim_max_time = 250,
            .max_elements = 64,
            .max_anim_states = 64,
        } );
    vxui_set_text_fn( &utest_fixture->ctx, vxui__motion_text_fn, nullptr );
}

UTEST_F_TEARDOWN( motion_fixture )
{
    Clay_SetCurrentContext( nullptr );
    std::free( utest_fixture->memory );
    utest_fixture->memory = nullptr;
    utest_fixture->memory_size = 0;
}

UTEST_F( motion_fixture, first_seen_anim_state_gets_sane_defaults )
{
    vxui_begin( &utest_fixture->ctx, 1.0f / 60.0f );
    VXUI( &utest_fixture->ctx, "root", {} ) {
        VXUI_LABEL( &utest_fixture->ctx, "motion.label", ( vxui_label_cfg ) { 0 } );
    }
    vxui_end( &utest_fixture->ctx );

    uint32_t id = utest_fixture->ctx.decls[ 0 ].id;
    vxui_anim_state* st = vxui__find_anim_state( &utest_fixture->ctx, id );
    ASSERT_TRUE( st != nullptr );
    EXPECT_EQ( st->last_seen_frame, 1u );
    EXPECT_EQ( st->opacity_current, 1.0f );
    EXPECT_EQ( st->scale_current, 1.0f );
    EXPECT_EQ( st->slide_x_current, 0.0f );
    EXPECT_EQ( st->slide_y_current, 0.0f );
    EXPECT_GT( st->bounds.h, 0.0f );
}

UTEST_F( motion_fixture, seen_state_updates_last_seen_frame )
{
    for ( int i = 0; i < 2; ++i ) {
        vxui_begin( &utest_fixture->ctx, 1.0f / 60.0f );
        VXUI( &utest_fixture->ctx, "root", {} ) {
            VXUI_LABEL( &utest_fixture->ctx, "motion.label", ( vxui_label_cfg ) { 0 } );
        }
        vxui_end( &utest_fixture->ctx );
    }

    uint32_t id = utest_fixture->ctx.decls[ 0 ].id;
    vxui_anim_state* st = vxui__find_anim_state( &utest_fixture->ctx, id );
    ASSERT_TRUE( st != nullptr );
    EXPECT_EQ( st->last_seen_frame, 2u );
}

UTEST_F( motion_fixture, disappearing_text_is_retained_and_fades )
{
    vxui_begin( &utest_fixture->ctx, 1.0f / 60.0f );
    VXUI( &utest_fixture->ctx, "root", {} ) {
        VXUI_LABEL( &utest_fixture->ctx, "motion.label", ( vxui_label_cfg ) { 0 } );
    }
    vxui_end( &utest_fixture->ctx );

    vxui_begin( &utest_fixture->ctx, 1.0f / 60.0f );
    VXUI( &utest_fixture->ctx, "root", {} ) {}
    vxui_draw_list list = vxui_end( &utest_fixture->ctx );

    ASSERT_TRUE( list.length >= 1 );
    EXPECT_EQ( list.commands[ 0 ].type, VXUI_CMD_TEXT );
    EXPECT_STREQ( list.commands[ 0 ].text.text, "Motion" );
    EXPECT_LT( list.commands[ 0 ].text.color.a, 255 );
    EXPECT_GT( list.commands[ 0 ].text.color.a, 0 );
}

UTEST_F( motion_fixture, unseen_states_eventually_settle_and_evict )
{
    vxui_begin( &utest_fixture->ctx, 1.0f / 60.0f );
    VXUI( &utest_fixture->ctx, "root", {} ) {
        VXUI_LABEL( &utest_fixture->ctx, "motion.label", ( vxui_label_cfg ) { 0 } );
    }
    vxui_end( &utest_fixture->ctx );

    for ( int i = 0; i < 180; ++i ) {
        vxui_begin( &utest_fixture->ctx, 1.0f / 60.0f );
        VXUI( &utest_fixture->ctx, "root", {} ) {}
        vxui_end( &utest_fixture->ctx );
    }

    EXPECT_EQ( vxui__count_anim_states( &utest_fixture->ctx ), 0 );
}

UTEST_F( motion_fixture, timeout_evicts_even_if_not_fully_settled )
{
    Clay_SetCurrentContext( nullptr );
    vxui_init(
        &utest_fixture->ctx,
        vxui_create_arena( utest_fixture->memory_size, utest_fixture->memory ),
        ( vxui_config ) {
            .screen_width = 640,
            .screen_height = 360,
            .exit_anim_max_time = 1,
            .max_elements = 64,
            .max_anim_states = 64,
        } );
    vxui_set_text_fn( &utest_fixture->ctx, vxui__motion_text_fn, nullptr );

    vxui_begin( &utest_fixture->ctx, 1.0f / 60.0f );
    VXUI( &utest_fixture->ctx, "root", {} ) {
        VXUI_LABEL( &utest_fixture->ctx, "motion.label", ( vxui_label_cfg ) { 0 } );
    }
    vxui_end( &utest_fixture->ctx );

    vxui_begin( &utest_fixture->ctx, 1.0f / 60.0f );
    VXUI( &utest_fixture->ctx, "root", {} ) {}
    vxui_end( &utest_fixture->ctx );

    EXPECT_EQ( vxui__count_anim_states( &utest_fixture->ctx ), 0 );
}

UTEST_F( motion_fixture, large_dt_converges_without_exploding )
{
    vxui_begin( &utest_fixture->ctx, 1.0f / 60.0f );
    VXUI( &utest_fixture->ctx, "root", {} ) {
        VXUI_LABEL( &utest_fixture->ctx, "motion.label", ( vxui_label_cfg ) { 0 } );
    }
    vxui_end( &utest_fixture->ctx );

    uint32_t id = utest_fixture->ctx.decls[ 0 ].id;
    vxui_anim_state* st = vxui__find_anim_state( &utest_fixture->ctx, id );
    ASSERT_TRUE( st != nullptr );
    st->opacity_current = 0.0f;
    st->opacity_velocity = 0.0f;
    st->scale_current = 0.5f;
    st->scale_velocity = 0.0f;

    vxui_begin( &utest_fixture->ctx, 1.0f );
    VXUI( &utest_fixture->ctx, "root", {} ) {
        VXUI_LABEL( &utest_fixture->ctx, "motion.label", ( vxui_label_cfg ) { 0 } );
    }
    vxui_end( &utest_fixture->ctx );

    st = vxui__find_anim_state( &utest_fixture->ctx, id );
    ASSERT_TRUE( st != nullptr );
    EXPECT_GT( st->opacity_current, 0.0f );
    EXPECT_LE( st->opacity_current, 1.0f );
    EXPECT_GT( st->scale_current, 0.5f );
    EXPECT_LE( st->scale_current, 1.0f );
}

UTEST_F( motion_fixture, reused_id_after_eviction_starts_cleanly )
{
    vxui_begin( &utest_fixture->ctx, 1.0f / 60.0f );
    VXUI( &utest_fixture->ctx, "root", {} ) {
        VXUI_LABEL( &utest_fixture->ctx, "motion.label", ( vxui_label_cfg ) { 0 } );
    }
    vxui_end( &utest_fixture->ctx );

    uint32_t id = utest_fixture->ctx.decls[ 0 ].id;
    for ( int i = 0; i < 180; ++i ) {
        vxui_begin( &utest_fixture->ctx, 1.0f / 60.0f );
        VXUI( &utest_fixture->ctx, "root", {} ) {}
        vxui_end( &utest_fixture->ctx );
    }

    EXPECT_EQ( vxui__count_anim_states( &utest_fixture->ctx ), 0 );

    vxui_begin( &utest_fixture->ctx, 1.0f / 60.0f );
    VXUI( &utest_fixture->ctx, "root", {} ) {
        VXUI_LABEL( &utest_fixture->ctx, "motion.label", ( vxui_label_cfg ) { 0 } );
    }
    vxui_end( &utest_fixture->ctx );

    vxui_anim_state* st = vxui__find_anim_state( &utest_fixture->ctx, id );
    ASSERT_TRUE( st != nullptr );
    EXPECT_EQ( st->opacity_current, 1.0f );
    EXPECT_EQ( st->opacity_velocity, 0.0f );
    EXPECT_EQ( st->scale_current, 1.0f );
}
