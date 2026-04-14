#ifdef VXUI_IMPL

#include "vxui_impl_util.h"

bool vxui_page( vxui_ctx* ctx, const char* name )
{
    assert( ctx );
    return ctx->active_page == vxui_hash( name );
}

void vxui_switch( vxui_ctx* ctx, const char* name )
{
    assert( ctx );
    ctx->active_page = vxui_hash( name );
}

static constexpr uint32_t s_vxui_inputs[] =
{
    vxui_hash( "up" ),
    vxui_hash( "down" ),
    vxui_hash( "left" ),
    vxui_hash( "right" ),
    vxui_hash( "confirm" ),
    vxui_hash( "cancel" )
};
static constexpr size_t s_vxui_inputs_n = sizeof( s_vxui_inputs ) / sizeof( s_vxui_inputs[0] );
static_assert( s_vxui_inputs_n == 6, "outdated vxui_input_action." );

void vxui_input( vxui_ctx* ctx, const char* action, bool pressed )
{
    uint32_t hash = vxui_hash( action );
    for ( int i = 0; i < s_vxui_inputs_n; i++ )
    {
        if ( hash != s_vxui_inputs[i] ) continue;
        if ( pressed )
            ctx->input |= ( 1 << i );
        else
            ctx->input &= ~( 1 << i );
        break;
    }
}

bool vxui_input_pressed( vxui_ctx* ctx, const char* action )
{
    uint32_t hash = vxui_hash( action );
    for ( int i = 0; i < s_vxui_inputs_n; i++ )
    {
        if ( hash != s_vxui_inputs[i] ) continue;
        return ( ctx->input & ( 1 << i ) ) != 0;
    }
    assert( !"invalid input action" );
    return false;
}

static int vxui_menu_get( vxui_ctx* ctx, uint32_t id )
{
    for ( int i = 0; i < ctx->menu_count; i++ )
        if ( ctx->menu_state[i].x == id ) return i;

    assert( ctx->menu_count < VXUI_MAX_MENUS );
    int idx = ctx->menu_count++;
    ctx->menu_state[idx] = { id, 0, 0 };

    return idx;
}

bool vxui_menu( vxui_ctx* ctx, const char* id, bool wrap )
{
    assert( ctx );
    assert( ctx->active_menu == -1 );   // no nested menus

    int idx = vxui_menu_get( ctx, vxui_hash( id ) );
    ctx->active_menu     = idx;
    ctx->active_menu_row = 0;

    glm::uvec3& m = ctx->menu_state[idx];
    uint32_t& hash_id     = m.x;
    uint32_t& current_row = m.y;
    uint32_t& num_rows    = m.z;

    // Navigate using last frame's count.

    if ( num_rows > 0 )
    {
        if ( ctx->input & VXUI_INPUT_UP )
        {
            if ( current_row == 0 )
                current_row = wrap ? num_rows - 1 : 0;
            else
                current_row--;
        }
        if ( ctx->input & VXUI_INPUT_DOWN )
        {
            current_row++;
            if ( current_row >= num_rows )
                current_row = wrap ? 0 : num_rows - 1;
        }
    }

    return true;
}

bool vxui_menu_action( vxui_ctx* ctx, const char* label )
{
    assert( ctx && ctx->active_menu >= 0 );

    glm::uvec3& m = ctx->menu_state[ctx->active_menu];
    const uint32_t& current_row = m.y;
    uint32_t row = ctx->active_menu_row++;

    return ( row == current_row ) && ( ctx->input & VXUI_INPUT_CONFIRM );
}

bool vxui_menu_option( vxui_ctx* ctx, const char* label, int* index, const char** options, int count )
{
    assert( ctx && ctx->active_menu >= 0 );
    assert( index && options && count > 0 );

    glm::uvec3& m = ctx->menu_state[ctx->active_menu];
    const uint32_t& current_row = m.y;
    uint32_t row = ctx->active_menu_row++;
    if ( row != current_row ) return false;

    int prev = *index;

    if ( ctx->input & VXUI_INPUT_RIGHT )
        *index = ( *index + 1 ) % count;

    if ( ctx->input & VXUI_INPUT_LEFT )
        *index = ( *index - 1 + count ) % count;

    return *index != prev;
}

bool vxui_menu_slider( vxui_ctx* ctx, const char* label, float* value, float mn, float mx, float step )
{
    assert( ctx && ctx->active_menu >= 0 );
    assert( value && mn < mx && step > 0.0f );

    glm::uvec3& m = ctx->menu_state[ctx->active_menu];
    const uint32_t& current_row = m.y;
    uint32_t row = ctx->active_menu_row++;
    if ( row != current_row ) return false;

    float prev = *value;

    if ( ctx->input & VXUI_INPUT_RIGHT )
        *value = glm::min( *value + step, mx );

    if ( ctx->input & VXUI_INPUT_LEFT )
        *value = glm::max( *value - step, mn );

    return *value != prev;
}

void vxui_menu_section( vxui_ctx* ctx, const char* title )
{
    assert( ctx && ctx->active_menu >= 0 );
    ctx->active_menu_row++;   // takes up a row but is not focusable
}

void vxui_menu_label( vxui_ctx* ctx, const char* text )
{
    assert( ctx && ctx->active_menu >= 0 );
    ctx->active_menu_row++;   // takes up a row but is not focusable
}

bool vxui_menu_cancelled( vxui_ctx* ctx )
{
    assert( ctx );
    return ( ctx->input & VXUI_INPUT_CANCEL ) != 0;
}

void vxui_menu_end( vxui_ctx* ctx )
{
    assert( ctx && ctx->active_menu >= 0 );

    uint32_t& num_rows = ctx->menu_state[ctx->active_menu].z;
    num_rows = ctx->active_menu_row;

    ctx->active_menu     = -1;
    ctx->active_menu_row = 0;
}

#endif // #ifdef VXUI_IMPL

