#ifdef VXUI_IMPL

#define CLAY_IMPLEMENTATION
#include "clay/clay.h"
#include "vxui_impl_util.h"
#include <cstring>

static void vxui_clay_error( Clay_ErrorData e )
{
    (void) e;
    assert( !"clay error" );
}

void vxui_init( vxui_ctx* ctx, float w, float h, void* clay_memory, size_t clay_size )
{
    assert( ctx && clay_memory && clay_size >= Clay_MinMemorySize() );
    Clay_Arena arena = Clay_CreateArenaWithCapacityAndMemory( clay_size, clay_memory );
    ctx->clay = Clay_Initialize( arena, { w, h }, { vxui_clay_error, nullptr } );
}

static Clay_SizingAxis vxui_sizing_to_clay( vxui_sizing s )
{
    switch ( s.type )
    {
        case VXUI_GROW:    return CLAY_SIZING_GROW( s.value );
        case VXUI_FIXED:   return CLAY_SIZING_FIXED( s.value );
        case VXUI_PERCENT: return CLAY_SIZING_PERCENT( s.value );
        default:           return CLAY_SIZING_FIT( s.value );
    }
}

static Clay_LayoutAlignmentX vxui_align_x_to_clay( uint8_t a )
{
    if ( a == 1 ) return CLAY_ALIGN_X_CENTER;
    if ( a == 2 ) return CLAY_ALIGN_X_RIGHT;
    return CLAY_ALIGN_X_LEFT;
}

static Clay_LayoutAlignmentY vxui_align_y_to_clay( uint8_t a )
{
    if ( a == 1 ) return CLAY_ALIGN_Y_CENTER;
    if ( a == 2 ) return CLAY_ALIGN_Y_BOTTOM;
    return CLAY_ALIGN_Y_TOP;
}

void vxui_frame( vxui_ctx* ctx, float dt )
{
    assert( ctx );
    assert( ctx->active_menu == -1 );   // mismatched menu_begin/end from last frame
    ctx->input = 0;
    ctx->dt    = dt;
    if ( ctx->clay )
    {
        Clay_SetCurrentContext( (Clay_Context*) ctx->clay );
        Clay_BeginLayout();
    }
}

vxui_draw_list vxui_render( vxui_ctx* ctx )
{
    assert( ctx );
    assert( ctx->active_menu == -1 );   // unclosed menu

    if ( !ctx->clay )
    {
        ctx->draw_list = {};
        return ctx->draw_list;
    }

    Clay_RenderCommandArray cmds = Clay_EndLayout();

    int count = 0;
    for ( int i = 0; i < cmds.length && count < VXUI_MAX_DRAW_CMDS; i++ )
    {
        Clay_RenderCommand* cmd = &cmds.internalArray[i];
        if ( cmd->commandType != CLAY_RENDER_COMMAND_TYPE_RECTANGLE ) continue;

        vxui_draw_cmd& out = ctx->draw_buf[count++];
        out.id   = cmd->id;
        out.rect = { cmd->boundingBox.x, cmd->boundingBox.y, cmd->boundingBox.width, cmd->boundingBox.height };
    }

    ctx->draw_list.cmds  = ctx->draw_buf;
    ctx->draw_list.count = count;
    return ctx->draw_list;
}

void vxui_div( vxui_ctx* ctx, const char* id, vxui_div_cfg cfg )
{
    assert( ctx );

    Clay_String clay_id = { false, (int32_t) strlen( id ), id };
    Clay__OpenElementWithId( Clay__HashString( clay_id, 0 ) );

    Clay_ElementDeclaration decl = {};
    decl.layout.sizing.width     = vxui_sizing_to_clay( cfg.width );
    decl.layout.sizing.height    = vxui_sizing_to_clay( cfg.height );
    decl.layout.layoutDirection  = cfg.col ? CLAY_TOP_TO_BOTTOM : CLAY_LEFT_TO_RIGHT;
    decl.layout.padding          = { cfg.padding[0], cfg.padding[1], cfg.padding[2], cfg.padding[3] };
    decl.layout.childGap         = cfg.gap;
    decl.layout.childAlignment.x = vxui_align_x_to_clay( cfg.align_x );
    decl.layout.childAlignment.y = vxui_align_y_to_clay( cfg.align_y );

    Clay__ConfigureOpenElement( decl );
}

void vxui_div_end( vxui_ctx* ctx )
{
    assert( ctx );
    Clay__CloseElement();
}

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
    for ( int i = 0; i < (int) s_vxui_inputs_n; i++ )
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
    for ( int i = 0; i < (int) s_vxui_inputs_n; i++ )
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
    ctx->menu_state[idx] = { id, 0, 0, 0 };

    return idx;
}

static uint32_t vxui_menu_next_row( uint32_t from, uint32_t num_rows,
                                    uint32_t skip_mask, int step, bool wrap )
{
    // step: +1 forward, -1 backward.
    for ( uint32_t i = 0; i < num_rows; i++ )
    {
        uint32_t next = ( from + ( i + 1 ) * step + num_rows * num_rows ) % num_rows;
        if ( !wrap && ( ( step > 0 && next <= from ) || ( step < 0 && next >= from ) ) )
            return from;
        if ( !( skip_mask & ( 1u << next ) ) )
            return next;
    }
    return from;
}

bool vxui_menu( vxui_ctx* ctx, const char* id, bool wrap )
{
    assert( ctx );
    assert( ctx->active_menu == -1 );   // no nested menus

    int idx = vxui_menu_get( ctx, vxui_hash( id ) );
    ctx->active_menu      = idx;
    ctx->active_menu_row  = 0;
    ctx->active_menu_skip = 0;

    glm::uvec4& m = ctx->menu_state[idx];
    uint32_t& current_row = m.y;
    uint32_t& num_rows    = m.z;
    uint32_t& skip_mask   = m.w;

    // Clamp focus if num_rows shrank since last frame.
    if ( num_rows > 0 && current_row >= num_rows )
        current_row = vxui_menu_next_row( num_rows, num_rows, skip_mask, -1, false );

    // Initial skip: if focus is on a non-interactive row, advance to first interactive row.
    if ( num_rows > 0 && ( skip_mask & ( 1u << current_row ) ) )
        current_row = vxui_menu_next_row( current_row, num_rows, skip_mask, +1, true );

    // Navigate using last frame's count.
    if ( num_rows > 0 )
    {
        if ( ctx->input & VXUI_INPUT_UP )
            current_row = vxui_menu_next_row( current_row, num_rows, skip_mask, -1, wrap );
        if ( ctx->input & VXUI_INPUT_DOWN )
            current_row = vxui_menu_next_row( current_row, num_rows, skip_mask, +1, wrap );
    }

    return true;
}

static void vxui_menu_open_row( vxui_ctx* ctx, const char* label )
{
    // Composite id: menu hash as seed so same label in different menus doesn't collide.
    Clay_String      cs   = { false, (int32_t) strlen( label ), label };
    Clay_ElementId   eid  = Clay__HashString( cs, ctx->menu_state[ctx->active_menu].x );

    Clay__OpenElementWithId( eid );

    Clay_ElementDeclaration decl        = {};
    decl.layout.sizing.width            = CLAY_SIZING_GROW( 0 );
    decl.layout.sizing.height           = CLAY_SIZING_FIXED( VXUI_ROW_HEIGHT );
    decl.backgroundColor                = { 0, 0, 0, 1 };  // alpha > 0 forces RECTANGLE emit

    Clay__ConfigureOpenElement( decl );
    Clay__CloseElement();
}

bool vxui_menu_action( vxui_ctx* ctx, const char* label )
{
    assert( ctx && ctx->active_menu >= 0 );

    vxui_menu_open_row( ctx, label );

    glm::uvec4& m = ctx->menu_state[ctx->active_menu];
    const uint32_t& current_row = m.y;
    uint32_t row = ctx->active_menu_row++;

    return ( row == current_row ) && ( ctx->input & VXUI_INPUT_CONFIRM );
}

bool vxui_menu_option( vxui_ctx* ctx, const char* label, int* index, const char** options, int count )
{
    assert( ctx && ctx->active_menu >= 0 );
    assert( index && options && count > 0 );

    vxui_menu_open_row( ctx, label );

    glm::uvec4& m = ctx->menu_state[ctx->active_menu];
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

    // TODO: emit rect (same pattern as vxui_menu_action via vxui_menu_open_row)

    glm::uvec4& m = ctx->menu_state[ctx->active_menu];
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
    // TODO: emit rect (same pattern as vxui_menu_action via vxui_menu_open_row)
    ctx->active_menu_skip |= ( 1u << ctx->active_menu_row );
    ctx->active_menu_row++;
}

void vxui_menu_label( vxui_ctx* ctx, const char* text )
{
    assert( ctx && ctx->active_menu >= 0 );
    // TODO: emit rect (same pattern as vxui_menu_action via vxui_menu_open_row)
    ctx->active_menu_skip |= ( 1u << ctx->active_menu_row );
    ctx->active_menu_row++;
}

bool vxui_menu_cancelled( vxui_ctx* ctx )
{
    assert( ctx );
    return ( ctx->input & VXUI_INPUT_CANCEL ) != 0;
}

void vxui_menu_end( vxui_ctx* ctx )
{
    assert( ctx && ctx->active_menu >= 0 );

    glm::uvec4& m = ctx->menu_state[ctx->active_menu];
    m.z = ctx->active_menu_row;   // num_rows
    m.w = ctx->active_menu_skip;  // skip_mask

    ctx->active_menu      = -1;
    ctx->active_menu_row  = 0;
    ctx->active_menu_skip = 0;
}

#endif // #ifdef VXUI_IMPL
