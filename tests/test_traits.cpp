#include <cmath>
#include <cstdint>
#include <cstdlib>

#include "../third_party/utest.h"
#include "../vxui.h"

typedef struct traits_fixture
{
    uint8_t* memory;
    uint64_t memory_size;
    vxui_ctx ctx;
} traits_fixture;

static int vxui__trait_exec_count = 0;
static int vxui__trait_order_index = 0;
static int vxui__trait_order_values[ 8 ] = {};
static bool vxui__trait_last_focused = false;

typedef struct vxui__test_trait_params
{
    float value;
    int order;
} vxui__test_trait_params;

typedef struct vxui__pulse_params
{
    float speed;
    float scale;
    float alpha;
} vxui__pulse_params;

typedef struct vxui__impulse_params
{
    float amount;
    float alpha;
} vxui__impulse_params;

static void vxui__test_trait( vxui_anim_state* element, vxui_draw_list* draw_list, vxui_rect bounds, float t, bool focused, const void* params )
{
    ( void ) draw_list;
    ( void ) bounds;
    ( void ) t;
    const vxui__test_trait_params* p = ( const vxui__test_trait_params* ) params;
    vxui__trait_exec_count += 1;
    vxui__trait_last_focused = focused;
    element->opacity_target = p ? p->value : 0.5f;
    if ( p ) {
        vxui__trait_order_values[ vxui__trait_order_index++ ] = p->order;
    }
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

UTEST_F_SETUP( traits_fixture )
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
        } );

    vxui__trait_exec_count = 0;
    vxui__trait_order_index = 0;
    vxui__trait_last_focused = false;
    for ( int i = 0; i < 8; ++i ) {
        vxui__trait_order_values[ i ] = 0;
    }
}

UTEST_F_TEARDOWN( traits_fixture )
{
    Clay_SetCurrentContext( nullptr );
    std::free( utest_fixture->memory );
    utest_fixture->memory = nullptr;
    utest_fixture->memory_size = 0;
}

UTEST_F( traits_fixture, trait_registration_stores_handler_and_param_size )
{
    vxui_register_trait( &utest_fixture->ctx, 99u, vxui__test_trait, sizeof( vxui__test_trait_params ) );

    ASSERT_TRUE( utest_fixture->ctx.trait_desc_count > 0 );
    EXPECT_EQ( utest_fixture->ctx.trait_descs[ utest_fixture->ctx.trait_desc_count - 1 ].id, 99u );
    EXPECT_EQ( utest_fixture->ctx.trait_descs[ utest_fixture->ctx.trait_desc_count - 1 ].params_size, sizeof( vxui__test_trait_params ) );
}

UTEST_F( traits_fixture, attached_trait_count_increments_on_declaration )
{
    vxui_register_trait( &utest_fixture->ctx, 99u, vxui__test_trait, sizeof( vxui__test_trait_params ) );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI_LABEL( &utest_fixture->ctx, "trait.host", ( vxui_label_cfg ) { 0 } );
    VXUI_TRAIT( 99u, ( vxui__test_trait_params ) { .value = 0.8f, .order = 1 } );
    vxui_end( &utest_fixture->ctx );

    vxui_anim_state* st = vxui__find_anim_state( &utest_fixture->ctx, vxui_id( "trait.host" ) );
    ASSERT_TRUE( st != nullptr );
    EXPECT_EQ( st->trait_count, 1 );
}

UTEST_F( traits_fixture, trait_executes_when_attached )
{
    vxui_register_trait( &utest_fixture->ctx, 99u, vxui__test_trait, sizeof( vxui__test_trait_params ) );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI_LABEL( &utest_fixture->ctx, "trait.host", ( vxui_label_cfg ) { 0 } );
    VXUI_TRAIT( 99u, ( vxui__test_trait_params ) { .value = 0.25f, .order = 1 } );
    vxui_end( &utest_fixture->ctx );

    vxui_anim_state* st = vxui__find_anim_state( &utest_fixture->ctx, vxui_id( "trait.host" ) );
    ASSERT_TRUE( st != nullptr );
    EXPECT_EQ( vxui__trait_exec_count, 1 );
    EXPECT_TRUE( std::fabs( st->opacity_target - 0.25f ) < 0.0001f );
}

UTEST_F( traits_fixture, pulse_modifies_target_properties )
{
    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI_LABEL( &utest_fixture->ctx, "pulse.host", ( vxui_label_cfg ) { 0 } );
    VXUI_TRAIT( VXUI_TRAIT_PULSE, ( vxui__pulse_params ) { .speed = 2.0f, .scale = 0.1f, .alpha = 0.2f } );
    vxui_end( &utest_fixture->ctx );

    vxui_anim_state* st = vxui__find_anim_state( &utest_fixture->ctx, vxui_id( "pulse.host" ) );
    ASSERT_TRUE( st != nullptr );
    EXPECT_TRUE( st->scale_current != 1.0f || st->opacity_current != 1.0f );
}

UTEST_F( traits_fixture, multiple_traits_preserve_declaration_order )
{
    vxui_register_trait( &utest_fixture->ctx, 99u, vxui__test_trait, sizeof( vxui__test_trait_params ) );
    vxui_register_trait( &utest_fixture->ctx, 100u, vxui__test_trait, sizeof( vxui__test_trait_params ) );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI_LABEL( &utest_fixture->ctx, "trait.host", ( vxui_label_cfg ) { 0 } );
    VXUI_TRAIT( 99u, ( vxui__test_trait_params ) { .value = 0.6f, .order = 1 } );
    VXUI_TRAIT( 100u, ( vxui__test_trait_params ) { .value = 0.7f, .order = 2 } );
    vxui_end( &utest_fixture->ctx );

    EXPECT_EQ( vxui__trait_order_values[ 0 ], 1 );
    EXPECT_EQ( vxui__trait_order_values[ 1 ], 2 );
}

UTEST_F( traits_fixture, one_shot_generation_prevents_repeated_blip )
{
    uint32_t id = vxui_id( "blip.host" );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI_LABEL( &utest_fixture->ctx, "blip.host", ( vxui_label_cfg ) { 0 } );
    VXUI_TRAIT( VXUI_TRAIT_BLIP, ( vxui__impulse_params ) { .amount = 0.2f, .alpha = 0.1f } );
    vxui_end( &utest_fixture->ctx );

    vxui_anim_state* st = vxui__find_anim_state( &utest_fixture->ctx, id );
    ASSERT_TRUE( st != nullptr );
    EXPECT_EQ( st->traits[ 0 ].generation, 0u );

    float first_scale = st->scale_current;
    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI_LABEL( &utest_fixture->ctx, "blip.host", ( vxui_label_cfg ) { 0 } );
    VXUI_TRAIT( VXUI_TRAIT_BLIP, ( vxui__impulse_params ) { .amount = 0.2f, .alpha = 0.1f } );
    vxui_end( &utest_fixture->ctx );

    st = vxui__find_anim_state( &utest_fixture->ctx, id );
    ASSERT_TRUE( st != nullptr );
    EXPECT_EQ( st->traits[ 0 ].generation, 0u );
    EXPECT_TRUE( st->scale_current <= first_scale );
}

UTEST_F( traits_fixture, focused_flag_is_reported_correctly )
{
    vxui_register_trait( &utest_fixture->ctx, 99u, vxui__test_trait, sizeof( vxui__test_trait_params ) );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    vxui_set_focus( &utest_fixture->ctx, vxui_id( "focus.action" ) );
    VXUI( &utest_fixture->ctx, "root", {} ) {
        VXUI_ACTION( &utest_fixture->ctx, "focus.action", "focus.action", nullptr, ( vxui_action_cfg ) { 0 } );
        VXUI_TRAIT( 99u, ( vxui__test_trait_params ) { .value = 0.5f, .order = 1 } );
    }
    vxui_end( &utest_fixture->ctx );

    EXPECT_TRUE( vxui__trait_last_focused );
}

UTEST_F( traits_fixture, nested_vxui_scope_restores_current_decl_id_for_following_traits )
{
    vxui_register_trait( &utest_fixture->ctx, 99u, vxui__test_trait, sizeof( vxui__test_trait_params ) );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "trait.host", {} ) {
        EXPECT_EQ( utest_fixture->ctx.current_decl_id, vxui_id( "trait.host" ) );
        VXUI( &utest_fixture->ctx, "trait.child", {} ) {
            EXPECT_EQ( utest_fixture->ctx.current_decl_id, vxui_id( "trait.child" ) );
            VXUI_LABEL( &utest_fixture->ctx, "trait.child.label", ( vxui_label_cfg ) { 0 } );
        }
        EXPECT_EQ( utest_fixture->ctx.current_decl_id, vxui_id( "trait.host" ) );
        VXUI_TRAIT( 99u, ( vxui__test_trait_params ) { .value = 0.8f, .order = 1 } );
    }
    vxui_end( &utest_fixture->ctx );

    vxui_anim_state* host = vxui__find_anim_state( &utest_fixture->ctx, vxui_id( "trait.host" ) );
    vxui_anim_state* child = vxui__find_anim_state( &utest_fixture->ctx, vxui_id( "trait.child" ) );
    ASSERT_TRUE( host != nullptr );
    EXPECT_EQ( host->trait_count, 1 );
    EXPECT_TRUE( child == nullptr || child->trait_count == 0 );
}
