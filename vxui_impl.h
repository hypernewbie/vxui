#ifdef VXUI_IMPL

#define CLAY_IMPLEMENTATION
#include "clay/clay.h"
#include "vxui_impl_util.h"
#include <cstdio>
#include <cstring>
#include <vector>
#include <hb.h>
#define VE_FONTCACHE_IMPL
#include "ve_fontcache.h"

static void vxui_clay_error( Clay_ErrorData e )
{
    fprintf( stderr, "clay error: type=%d %.*s\n", (int) e.errorType, (int) e.errorText.length, e.errorText.chars );
    assert( !"clay error" );
}

static Clay_Dimensions vxui_measure_text( Clay_StringSlice s, Clay_TextElementConfig* cfg, void* userData );

static const char* vxui_text_push( vxui_ctx* ctx, const char* src, int len )
{
    assert( ctx && src && len >= 0 );
    assert( ctx->text_offset + len <= VXUI_MAX_TEXT_BYTES );
    char* dst = ctx->text_buf + ctx->text_offset;
    memcpy( dst, src, len );
    ctx->text_offset += len;
    return dst;
}

void vxui_init( vxui_ctx* ctx, float w, float h, void* clay_memory, size_t clay_size )
{
    assert( ctx && clay_memory && clay_size >= Clay_MinMemorySize() );
    Clay_Arena arena = Clay_CreateArenaWithCapacityAndMemory( clay_size, clay_memory );
    ctx->clay = Clay_Initialize( arena, { w, h }, { vxui_clay_error, nullptr } );
    Clay_SetMeasureTextFunction( vxui_measure_text, ctx );
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

void vxui_frame( vxui_ctx* ctx, float dt, float w, float h )
{
    assert( ctx );
    assert( ctx->active_menu == -1 );   // mismatched menu_begin/end from last frame
    assert( !ctx->frame_active && "vxui_frame without intervening vxui_render" );
    ctx->prev_input             = ctx->input;
    ctx->prev_active_mask       = ctx->menu_active_mask;
    ctx->prev_mouse_buttons     = ctx->mouse_buttons;
    ctx->input                  = 0;
    ctx->menu_active_mask       = 0;
    ctx->mouse_buttons          = 0;
    ctx->dt                     = dt;
    ctx->frame_active           = true;
    ctx->inputs_committed       = false;
    ctx->text_offset            = 0;
    ctx->pressed_row_count      = 0;
    ctx->frame_row_count        = 0;
    if ( ctx->clay )
    {
        Clay_SetCurrentContext( (Clay_Context*) ctx->clay );
        if ( w > 0 && h > 0 )
            Clay_SetLayoutDimensions( { w, h } );
        Clay_BeginLayout();
    }
}

static void vxui_commit_inputs( vxui_ctx* ctx );

// RECT hit test: inside the rect and inside its scroll clip, if any.
static bool vxui_hit_rect( const vxui_draw_cmd& c, glm::vec2 p )
{
    bool inside = p.x >= c.rect.x && p.x <= c.rect.x + c.rect.z
               && p.y >= c.rect.y && p.y <= c.rect.y + c.rect.w;
    if ( !inside || !c.clipped ) return inside;
    if ( c.clip.z <= 0 || c.clip.w <= 0 ) return false;
    return p.x >= c.clip.x && p.x <= c.clip.x + c.clip.z
        && p.y >= c.clip.y && p.y <= c.clip.y + c.clip.w;
}

vxui_draw_list vxui_render( vxui_ctx* ctx )
{
    assert( ctx );
    assert( ctx->active_menu == -1 );   // unclosed menu
    assert( ctx->frame_active && "vxui_render without vxui_frame" );

    vxui_commit_inputs( ctx );
    ctx->frame_active = false;

    if ( !ctx->clay )
    {
        ctx->draw_list = {};
        return ctx->draw_list;
    }

    Clay_RenderCommandArray cmds = Clay_EndLayout();

    // Clay emits scissor start/end around clipped subtrees. Carry the active
    // intersection onto each command so hit tests and renderers share it.
    glm::vec4 clip_stack[VXUI_MAX_CLIP_STACK] = {};
    int       clip_depth                     = 0;

    int count = 0;
    for ( int i = 0; i < cmds.length; i++ )
    {
        Clay_RenderCommand* cmd = &cmds.internalArray[i];

        if ( cmd->commandType == CLAY_RENDER_COMMAND_TYPE_SCISSOR_START )
        {
            assert( clip_depth < VXUI_MAX_CLIP_STACK );
            glm::vec4 next = { cmd->boundingBox.x, cmd->boundingBox.y, cmd->boundingBox.width, cmd->boundingBox.height };
            if ( clip_depth > 0 )
            {
                glm::vec4& cur = clip_stack[clip_depth - 1];
                float x0 = cur.x > next.x ? cur.x : next.x;
                float y0 = cur.y > next.y ? cur.y : next.y;
                float x1 = cur.x + cur.z < next.x + next.z ? cur.x + cur.z : next.x + next.z;
                float y1 = cur.y + cur.w < next.y + next.w ? cur.y + cur.w : next.y + next.w;
                next = { x0, y0, x1 - x0, y1 - y0 };
            }
            clip_stack[clip_depth++] = next;
            continue;
        }
        if ( cmd->commandType == CLAY_RENDER_COMMAND_TYPE_SCISSOR_END )
        {
            assert( clip_depth > 0 );
            clip_depth--;
            continue;
        }

        if ( cmd->commandType != CLAY_RENDER_COMMAND_TYPE_RECTANGLE && cmd->commandType != CLAY_RENDER_COMMAND_TYPE_TEXT ) continue;
        assert( count < VXUI_MAX_DRAW_CMDS && "draw command capacity exceeded" );
        if ( count >= VXUI_MAX_DRAW_CMDS ) break;

        glm::vec4 clip    = clip_depth > 0 ? clip_stack[clip_depth - 1] : glm::vec4{};
        bool      clipped = clip_depth > 0;

        vxui_draw_cmd& out = ctx->draw_buf[count++];
        out.id                = cmd->id;
        out.type              = cmd->commandType == CLAY_RENDER_COMMAND_TYPE_RECTANGLE ? VXUI_DRAW_RECT : VXUI_DRAW_TEXT;
        out.state             = 0;
        out.clipped           = clipped;
        out.rect              = { cmd->boundingBox.x, cmd->boundingBox.y, cmd->boundingBox.width, cmd->boundingBox.height };
        out.clip              = clip;
        out.focus_offset_y    = 0;
        out.text              = nullptr;
        out.userdata          = cmd->userData;
        out.text_len          = 0;
        out.font              = 0;
        out.font_px           = 0;
        out.row_index         = -1;
        out.focused_row_index = -1;

        if ( out.type == VXUI_DRAW_TEXT )
        {
            const Clay_TextRenderData& t = cmd->renderData.text;
            out.text     = t.stringContents.chars;     // points into ctx->text_buf, valid until next vxui_frame
            out.text_len = t.stringContents.length;
            out.font     = t.fontId;
            out.font_px  = t.fontSize;
        }
    }

    ctx->mouse_click_row_id = 0;
    uint32_t under_cursor_id = 0;
    for ( int i = 0; i < count; i++ )
    {
        vxui_draw_cmd& c = ctx->draw_buf[i];
        if ( c.type != VXUI_DRAW_RECT ) continue;
        for ( int j = 0; j < ctx->pressed_row_count; j++ )
        {
            if ( c.id == ctx->pressed_row_ids[j] ) { c.state |= VXUI_DRAW_PRESSED; break; }
        }
        for ( int j = 0; j < ctx->frame_row_count; j++ )
        {
            const glm::uvec3& row = ctx->frame_rows[j];
            if ( c.id != row.x ) continue;
            const glm::uvec4& menu = ctx->menu_state[row.z];
            c.row_index         = (int) row.y;
            c.focused_row_index = (int) menu.y;
            if ( row.y == menu.y && !( menu.w & ( 1u << row.y ) ) )
            {
                c.state          |= VXUI_DRAW_FOCUSED;
                c.focus_offset_y  = ctx->menu_focus_spring[row.z].x;
            }
            break;
        }
        if ( vxui_hit_rect( c, ctx->mouse_pos ) ) under_cursor_id = c.id;     // last (topmost) wins for click target
    }

    // Settle the mouse edge before stamping HOVERED/PRESSED on containing rects.
    bool btn_now  = ( ctx->mouse_buttons      & VXUI_MOUSE_LEFT ) != 0;
    bool btn_prev = ( ctx->prev_mouse_buttons & VXUI_MOUSE_LEFT ) != 0;
    if ( btn_now && !btn_prev )
        ctx->mouse_press_row_id = under_cursor_id;
    if ( !btn_now && btn_prev )
    {
        if ( ctx->mouse_press_row_id != 0 && under_cursor_id == ctx->mouse_press_row_id )
            ctx->mouse_click_row_id = ctx->mouse_press_row_id;
        ctx->mouse_press_row_id = 0;
    }

    for ( int i = 0; i < count; i++ )
    {
        vxui_draw_cmd& c = ctx->draw_buf[i];
        if ( c.type != VXUI_DRAW_RECT ) continue;
        if ( !vxui_hit_rect( c, ctx->mouse_pos ) ) continue;
        c.state |= VXUI_DRAW_HOVERED;
        if ( c.id == ctx->mouse_press_row_id ) c.state |= VXUI_DRAW_PRESSED;
    }

    for ( int i = 0; i < count; i++ )
    {
        vxui_draw_cmd& c = ctx->draw_buf[i];
        memset( &c.render, 0, sizeof( c.render ) );
        if ( ctx->render_data_fn )
            ctx->render_data_fn( &c, &c.render, ctx->render_data_userdata );
    }

    ctx->draw_list.cmds  = ctx->draw_buf;
    ctx->draw_list.count = count;
    return ctx->draw_list;
}

void vxui_set_render_data_fn( vxui_ctx* ctx, vxui_render_data_fn fn, void* userdata )
{
    assert( ctx );
    ctx->render_data_fn       = fn;
    ctx->render_data_userdata = userdata;
}

int vxui_draw_count( const vxui_draw_list& dl, uint8_t type )
{
    int n = 0;
    for ( int i = 0; i < dl.count; i++ )
        if ( dl.cmds[i].type == type ) n++;
    return n;
}

const vxui_draw_cmd* vxui_draw_nth( const vxui_draw_list& dl, uint8_t type, int n )
{
    int seen = 0;
    for ( int i = 0; i < dl.count; i++ )
    {
        if ( dl.cmds[i].type != type ) continue;
        if ( seen == n ) return &dl.cmds[i];
        seen++;
    }
    return nullptr;
}

const vxui_draw_cmd* vxui_draw_find( const vxui_draw_list& dl, uint8_t type, uint32_t id )
{
    for ( int i = 0; i < dl.count; i++ )
        if ( dl.cmds[i].type == type && dl.cmds[i].id == id ) return &dl.cmds[i];
    return nullptr;
}

static Clay_ElementId vxui_clay_id( const char* id )
{
    assert( id );
    Clay_String clay_id = { false, (int32_t) strlen( id ), id };
    return Clay__HashString( clay_id, 0 );
}

void vxui_div( vxui_ctx* ctx, const char* id, vxui_div_cfg cfg )
{
    assert( ctx );

    Clay__OpenElementWithId( vxui_clay_id( id ) );

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

void vxui_rect( vxui_ctx* ctx, const char* id, vxui_div_cfg cfg )
{
    assert( ctx );

    Clay__OpenElementWithId( vxui_clay_id( id ) );

    Clay_ElementDeclaration decl = {};
    decl.layout.sizing.width     = vxui_sizing_to_clay( cfg.width );
    decl.layout.sizing.height    = vxui_sizing_to_clay( cfg.height );
    decl.layout.layoutDirection  = cfg.col ? CLAY_TOP_TO_BOTTOM : CLAY_LEFT_TO_RIGHT;
    decl.layout.padding          = { cfg.padding[0], cfg.padding[1], cfg.padding[2], cfg.padding[3] };
    decl.layout.childGap         = cfg.gap;
    decl.layout.childAlignment.x = vxui_align_x_to_clay( cfg.align_x );
    decl.layout.childAlignment.y = vxui_align_y_to_clay( cfg.align_y );
    decl.backgroundColor         = { 0, 0, 0, 1 };  // alpha > 0 forces RECTANGLE emit

    Clay__ConfigureOpenElement( decl );
}

void vxui_text( vxui_ctx* ctx, const char* id, const char* label, uint16_t font_px )
{
    assert( ctx );
    if ( font_px == 0 ) font_px = (uint16_t) VXUI_FONT_SIZE_DEFAULT;

    Clay__OpenElementWithId( vxui_clay_id( id ) );

    Clay_ElementDeclaration decl = {};
    decl.layout.sizing.width     = CLAY_SIZING_FIT( 0 );
    decl.layout.sizing.height    = CLAY_SIZING_FIT( 0 );

    Clay__ConfigureOpenElement( decl );

    int         label_len = (int) strlen( label );
    const char* stable    = vxui_text_push( ctx, label, label_len );
    Clay_String text_str  = { false, (int32_t) label_len, stable };
    CLAY_TEXT( text_str, CLAY_TEXT_CONFIG( { .fontSize = font_px } ) );

    Clay__CloseElement();
}

void vxui_root( vxui_ctx* ctx, const char* id, float x, float y )
{
    assert( ctx );

    Clay__OpenElementWithId( vxui_clay_id( id ) );

    Clay_ElementDeclaration decl       = {};
    decl.layout.sizing.width           = CLAY_SIZING_FIT( 0 );
    decl.layout.sizing.height          = CLAY_SIZING_FIT( 0 );
    decl.floating.attachTo             = CLAY_ATTACH_TO_ROOT;
    decl.floating.attachPoints.element = CLAY_ATTACH_POINT_LEFT_TOP;
    decl.floating.attachPoints.parent  = CLAY_ATTACH_POINT_LEFT_TOP;
    decl.floating.offset               = { x, y };

    Clay__ConfigureOpenElement( decl );
}

void vxui_root_end( vxui_ctx* ctx )
{
    assert( ctx );
    Clay__CloseElement();
}

uint32_t vxui_id( const char* id )
{
    return vxui_clay_id( id ).id;
}

// Clay double-hashes text: text element = HashNumber( child offset, parent ),
// line cmd = HashNumber( line index, text element ). This is the first line
// of a single vxui_text/vxui_hud_text element; both offsets zero.
uint32_t vxui_text_id( const char* id )
{
    return Clay__HashNumber( 0, Clay__HashNumber( 0, vxui_clay_id( id ).id ).id ).id;
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

bool vxui_input_just_pressed( vxui_ctx* ctx, const char* action )
{
    uint32_t hash = vxui_hash( action );
    for ( int i = 0; i < (int) s_vxui_inputs_n; i++ )
    {
        if ( hash != s_vxui_inputs[i] ) continue;
        return ( ctx->input & ~ctx->prev_input & ( 1 << i ) ) != 0;
    }
    assert( !"invalid input action" );
    return false;
}

static void vxui_commit_inputs( vxui_ctx* ctx )
{
    if ( ctx->inputs_committed ) return;
    ctx->inputs_committed = true;
    ctx->input_repeated   = 0;
    for ( int i = 0; i < (int) s_vxui_inputs_n; i++ )
    {
        uint32_t bit = 1u << i;
        bool now  = ( ctx->input      & bit ) != 0;
        bool prev = ( ctx->prev_input & bit ) != 0;
        if ( !now )
        {
            ctx->input_held_time[i] = 0;
            continue;
        }
        if ( !prev )
        {
            ctx->input_held_time[i] = 0;
            ctx->input_next_fire[i] = VXUI_INPUT_DELAY;
            ctx->input_repeated    |= bit;
        }
        else
        {
            ctx->input_held_time[i] += ctx->dt;
            if ( ctx->input_held_time[i] + 1e-5f >= ctx->input_next_fire[i] )
            {
                ctx->input_repeated    |= bit;
                ctx->input_next_fire[i] += VXUI_INPUT_REPEAT;
            }
        }
    }
}

bool vxui_input_repeated( vxui_ctx* ctx, const char* action )
{
    vxui_commit_inputs( ctx );
    uint32_t hash = vxui_hash( action );
    for ( int i = 0; i < (int) s_vxui_inputs_n; i++ )
    {
        if ( hash != s_vxui_inputs[i] ) continue;
        return ( ctx->input_repeated & ( 1 << i ) ) != 0;
    }
    assert( !"invalid input action" );
    return false;
}

void vxui_mouse( vxui_ctx* ctx, float x, float y, uint32_t buttons )
{
    assert( ctx );
    ctx->mouse_pos     = { x, y };
    ctx->mouse_buttons = buttons;
}

static int vxui_menu_get( vxui_ctx* ctx, const char* name )
{
    uint32_t id = vxui_hash( name );
    for ( int i = 0; i < ctx->menu_count; i++ )
    {
        if ( ctx->menu_state[i].x != id ) continue;
        assert( strcmp( ctx->menu_names[i], name ) == 0 && "menu hash collision" );
        return i;
    }

    assert( ctx->menu_count < VXUI_MAX_MENUS );
    int idx = ctx->menu_count++;
    ctx->menu_state[idx]        = { id, 0, 0, 0 };
    ctx->menu_focus_spring[idx] = { 0.0f, 0.0f, -1.0f, 0.0f };  // prev_row -1 = unset, snap on first focus
    ctx->menu_names[idx]        = name;

    return idx;
}

static uint32_t vxui_menu_next_row( uint32_t from, uint32_t num_rows, uint32_t skip_mask, int step, bool wrap )
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

bool vxui_menu( vxui_ctx* ctx, const char* id, bool wrap, int max_visible, bool fill )
{
    assert( ctx );
    assert( ctx->active_menu == -1 );   // no nested menus

    vxui_commit_inputs( ctx );

    int idx = vxui_menu_get( ctx, id );
    ctx->active_menu          = idx;
    ctx->active_menu_row      = 0;
    ctx->active_menu_skip     = 0;
    ctx->active_menu_focus_id = 0;
    ctx->active_menu_fill     = fill;

    // Snap focus spring on remount so reappearing menus don't animate from a stale offset.
    bool is_remount = ( ctx->prev_active_mask & ( 1u << idx ) ) == 0;
    if ( is_remount ) ctx->menu_focus_spring[idx] = { 0.0f, 0.0f, -1.0f, 0.0f };
    ctx->menu_active_mask |= ( 1u << idx );

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

    // Navigate using last frame's count. DAS-paced (initial press fires, then
    // VXUI_INPUT_DELAY/REPEAT). Holding nav at 60Hz roll is never the right UX.
    if ( num_rows > 0 )
    {
        if ( ctx->input_repeated & VXUI_INPUT_UP )
            current_row = vxui_menu_next_row( current_row, num_rows, skip_mask, -1, wrap );
        if ( ctx->input_repeated & VXUI_INPUT_DOWN )
            current_row = vxui_menu_next_row( current_row, num_rows, skip_mask, +1, wrap );
    }

    // Scroll: keep focused row inside [scroll_top, scroll_top + max_visible).
    int& scroll_top = ctx->menu_scroll_top[idx];
    if ( max_visible > 0 )
    {
        if ( (int) current_row < scroll_top )
            scroll_top = (int) current_row;
        if ( (int) current_row >= scroll_top + max_visible )
            scroll_top = (int) current_row - max_visible + 1;
        if ( scroll_top < 0 ) scroll_top = 0;
    }
    else
    {
        scroll_top = 0;
    }

    // Open a column container so rows stack vertically.
    Clay_String      cs  = { false, (int32_t) strlen( id ), id };
    Clay_ElementId   eid = Clay__HashString( cs, 0 );
    Clay__OpenElementWithId( eid );

    Clay_ElementDeclaration decl = {};
    decl.layout.layoutDirection  = CLAY_TOP_TO_BOTTOM;
    decl.layout.sizing.width     = fill ? CLAY_SIZING_GROW( 0 ) : CLAY_SIZING_FIT( 0 );
    decl.layout.sizing.height    = max_visible > 0 ? CLAY_SIZING_FIXED( (float) ( max_visible * VXUI_ROW_HEIGHT ) ) : CLAY_SIZING_FIT( 0 );
    if ( max_visible > 0 )
    {
        decl.clip.vertical    = true;
        decl.clip.childOffset = { 0.0f, -(float) ( scroll_top * VXUI_ROW_HEIGHT ) };
    }

    Clay__ConfigureOpenElement( decl );

    return true;
}

static uint32_t vxui_menu_open_row( vxui_ctx* ctx, const char* label, bool has_value = false )
{
    // Composite id: menu hash as seed so same label in different menus doesn't collide.
    // Identity uses label only; value text can change every frame without remounting focus.
    int              label_len = (int) strlen( label );
    Clay_String      cs        = { false, (int32_t) label_len, label };
    Clay_ElementId   eid       = Clay__HashString( cs, ctx->menu_state[ctx->active_menu].x );

    assert( ctx->active_menu_row < VXUI_MAX_MENU_ROWS );
    for ( int i = 0; i < ctx->active_menu_row; i++ )
        assert( ctx->active_menu_row_ids[i] != eid.id && "duplicate label in same menu" );
    ctx->active_menu_row_ids[ctx->active_menu_row] = eid.id;

    assert( ctx->frame_row_count < VXUI_MAX_MENUS * VXUI_MAX_MENU_ROWS );
    for ( int i = 0; i < ctx->frame_row_count; i++ )
        assert( ctx->frame_rows[i].x != eid.id && "menu row hash collision" );
    ctx->frame_rows[ctx->frame_row_count++] = { eid.id, (uint32_t) ctx->active_menu_row, (uint32_t) ctx->active_menu };

    bool fill = ctx->active_menu_fill;

    Clay__OpenElementWithId( eid );

    Clay_ElementDeclaration decl = {};
    decl.layout.sizing.width     = fill ? CLAY_SIZING_GROW( 0 ) : CLAY_SIZING_FIT( 0 );
    decl.layout.sizing.height    = CLAY_SIZING_FIXED( VXUI_ROW_HEIGHT );
    decl.layout.layoutDirection  = CLAY_LEFT_TO_RIGHT;
    decl.layout.childAlignment.y = CLAY_ALIGN_Y_CENTER;
    decl.layout.childGap         = has_value ? 8 : 0;
    // Fill rows get padding so the renderer has room to overlay a focus marker
    // on the left and the value text breathes on the right. FIT rows keep zero
    // padding so tests that compare rect.z to text width stay valid.
    if ( fill )
        decl.layout.padding = { 36, 12, 0, 0 };
    decl.backgroundColor         = { 0, 0, 0, 1 };  // alpha > 0 forces RECTANGLE emit

    Clay__ConfigureOpenElement( decl );

    // Copy label into per-frame buffer so the Clay_String pointer stays valid through Clay_EndLayout.
    const char* stable_label = vxui_text_push( ctx, label, label_len );
    Clay_String text_str     = { false, (int32_t) label_len, stable_label };
    CLAY_TEXT( text_str, CLAY_TEXT_CONFIG( { .fontSize = (uint16_t) VXUI_FONT_SIZE_DEFAULT } ) );

    return eid.id;
}

static void vxui_menu_close_row( vxui_ctx* ctx, const char* value = nullptr )
{
    bool fill      = ctx->active_menu_fill;
    bool has_value = value && *value;
    int  value_len = has_value ? (int) strlen( value ) : 0;

    // Optional value text. In fill rows a grow-spacer pushes it to the right
    // edge; in FIT rows it sits next to the label with childGap of 8.
    if ( has_value )
    {
        if ( fill )
        {
            Clay_ElementId spacer_eid = Clay__HashString( CLAY_STRING( "spacer" ), Clay__GetOpenLayoutElement()->id );
            Clay__OpenElementWithId( spacer_eid );

            Clay_ElementDeclaration spacer = {};
            spacer.layout.sizing.width     = CLAY_SIZING_GROW( 0 );
            spacer.layout.sizing.height    = CLAY_SIZING_FIXED( VXUI_ROW_HEIGHT );

            Clay__ConfigureOpenElement( spacer );
            Clay__CloseElement();
        }

        const char* stable_value = vxui_text_push( ctx, value, value_len );
        Clay_String text_str     = { false, (int32_t) value_len, stable_value };
        CLAY_TEXT( text_str, CLAY_TEXT_CONFIG( { .fontSize = (uint16_t) VXUI_FONT_SIZE_DEFAULT } ) );
    }

    Clay__CloseElement();
}

static bool vxui_menu_row_interactive( vxui_ctx* ctx, const char* label, bool has_value = false )
{
    uint32_t row_id = vxui_menu_open_row( ctx, label, has_value );

    glm::uvec4& m       = ctx->menu_state[ctx->active_menu];
    uint32_t&   current = m.y;
    uint32_t    row     = ctx->active_menu_row++;

    // Mouse click on this row jumps focus before any other resolution.
    // Consumption of mouse_click_row_id happens in vxui_menu_action so the fire
    // signal can also be derived from the same id without a second compute.
    if ( ctx->mouse_click_row_id == row_id )
        current = row;

    // Promote focus to first interactive row when current is stuck on a leading skip row.
    if ( current < row
         && ( ctx->active_menu_skip & ( 1u << current ) )
         && ctx->active_menu_focus_id == 0 )
        current = row;

    if ( row != current ) return false;

    ctx->active_menu_focus_id = row_id;
    return true;
}

bool vxui_menu_action( vxui_ctx* ctx, const char* label )
{
    assert( ctx && ctx->active_menu >= 0 );
    bool focused = vxui_menu_row_interactive( ctx, label );
    vxui_menu_close_row( ctx );
    if ( !focused ) return false;

    bool kbd_held   = ( ctx->input & VXUI_INPUT_CONFIRM ) != 0;
    bool kbd_fire   = ( ctx->input & ~ctx->prev_input & VXUI_INPUT_CONFIRM ) != 0;
    bool mouse_fire = ( ctx->mouse_click_row_id == ctx->active_menu_focus_id );
    if ( mouse_fire ) ctx->mouse_click_row_id = 0;

    if ( ( kbd_held || mouse_fire ) && ctx->pressed_row_count < VXUI_MAX_MENUS )
        ctx->pressed_row_ids[ctx->pressed_row_count++] = ctx->active_menu_focus_id;

    // Edge-triggered: holding confirm or holding the mouse button fires once, not every frame.
    return kbd_fire || mouse_fire;
}

bool vxui_menu_option( vxui_ctx* ctx, const char* label, int* index, const char** options, int count )
{
    assert( ctx && ctx->active_menu >= 0 );
    assert( index && options && count > 0 );
    assert( *index >= 0 && *index < count );

    bool focused = vxui_menu_row_interactive( ctx, label, true );
    // TODO: PRESSED state on confirm-held mirror, see vxui_menu_action.

    int prev = *index;
    if ( focused )
    {
        // DAS-repeat: hold cycles at VXUI_INPUT_DELAY/REPEAT, not every frame.
        if ( ctx->input_repeated & VXUI_INPUT_RIGHT )
            *index = ( *index + 1 ) % count;

        if ( ctx->input_repeated & VXUI_INPUT_LEFT )
            *index = ( *index - 1 + count ) % count;
    }

    vxui_menu_close_row( ctx, options[*index] );

    return *index != prev;
}

bool vxui_menu_slider( vxui_ctx* ctx, const char* label, float* value, float mn, float mx, float step )
{
    assert( ctx && ctx->active_menu >= 0 );
    assert( value && mn < mx && step > 0.0f );

    bool focused = vxui_menu_row_interactive( ctx, label );
    vxui_menu_close_row( ctx );
    if ( !focused ) return false;
    // TODO: PRESSED state on confirm-held mirror, see vxui_menu_action.

    float prev = *value;

    // DAS-repeat: hold scrubs at VXUI_INPUT_DELAY/REPEAT, not every frame.
    if ( ctx->input_repeated & VXUI_INPUT_RIGHT )
        *value = glm::min( *value + step, mx );

    if ( ctx->input_repeated & VXUI_INPUT_LEFT )
        *value = glm::max( *value - step, mn );

    return *value != prev;
}

void vxui_menu_section( vxui_ctx* ctx, const char* title )
{
    assert( ctx && ctx->active_menu >= 0 );
    vxui_menu_open_row( ctx, title );
    vxui_menu_close_row( ctx );
    ctx->active_menu_skip |= ( 1u << ctx->active_menu_row );
    ctx->active_menu_row++;
}

void vxui_menu_label( vxui_ctx* ctx, const char* text )
{
    assert( ctx && ctx->active_menu >= 0 );
    vxui_menu_open_row( ctx, text );
    vxui_menu_close_row( ctx );
    ctx->active_menu_skip |= ( 1u << ctx->active_menu_row );
    ctx->active_menu_row++;
}

// Edge-triggered: holding cancel fires once, not every frame.
bool vxui_menu_cancelled( vxui_ctx* ctx )
{
    assert( ctx );
    return ( ctx->input & ~ctx->prev_input & VXUI_INPUT_CANCEL ) != 0;
}

void vxui_menu_end( vxui_ctx* ctx )
{
    assert( ctx && ctx->active_menu >= 0 );

    if ( ctx->active_menu_focus_id != 0 )
    {
        glm::uvec4& m       = ctx->menu_state[ctx->active_menu];
        glm::vec4&  spring  = ctx->menu_focus_spring[ctx->active_menu];
        float& offset_y     = spring.x;
        float& velocity_y   = spring.y;
        float& prev_row_f   = spring.z;

        // On focus change, snap offset to keep the visual displacement at the
        // previous row, then spring it back to 0.
        float curr_row_f = (float) m.y;
        if ( prev_row_f >= 0.0f && prev_row_f != curr_row_f )
            offset_y += ( prev_row_f - curr_row_f ) * (float) VXUI_ROW_HEIGHT;
        prev_row_f = curr_row_f;

        glm::vec2 s = { offset_y, velocity_y };
        vxui_spring_update( s, 0.0f, 12.0f, ctx->dt > 0.0f ? ctx->dt : 1.0f / 60.0f );
        offset_y   = s.x;
        velocity_y = s.y;
    }

    Clay__CloseElement();

    glm::uvec4& m = ctx->menu_state[ctx->active_menu];
    m.z = ctx->active_menu_row;   // num_rows
    m.w = ctx->active_menu_skip;  // skip_mask

    ctx->active_menu          = -1;
    ctx->active_menu_row      = 0;
    ctx->active_menu_skip     = 0;
    ctx->active_menu_focus_id = 0;
}

struct vxui_text_state
{
    ve_fontcache cache;
    std::vector< std::vector< uint8_t > > font_bytes; // VXUI owns the bytes; VEFC keeps weak pointers
    int          default_font = -1;                   // -1 = none loaded yet
};

static Clay_Dimensions vxui_measure_text( Clay_StringSlice s, Clay_TextElementConfig* cfg, void* userData )
{
    vxui_ctx* ctx = (vxui_ctx*) userData;
    if ( !ctx || !ctx->text || s.length <= 0 ) return { 0, 0 };

    vxui_text_state* st = (vxui_text_state*) ctx->text;
    int font_id = ( cfg->fontId != 0 ) ? (int) cfg->fontId : st->default_font;
    if ( font_id < 0 ) return { 0, 0 };

    ve_fontcache_set_font_size( &st->cache, font_id, (float) cfg->fontSize );

    std::u8string text( (const char8_t*) s.chars, (size_t) s.length );
    ve_fontcache_vec2 m = ve_fontcache_measure_text( &st->cache, font_id, text, 1.0f, 1.0f, true );
    return { m.x, (float) cfg->fontSize };
}

vxui_font_id vxui_load_font( vxui_ctx* ctx, const void* data, size_t size, float size_px )
{
    assert( ctx && data && size > 0 );

    if ( !ctx->text )
    {
        vxui_text_state* st = new vxui_text_state();
        ve_fontcache_init( &st->cache, false );   // no FreeType: shaping/measure path only
        ctx->text = st;
    }

    vxui_text_state* st = (vxui_text_state*) ctx->text;

    st->font_bytes.emplace_back( (const uint8_t*) data, (const uint8_t*) data + size );
    auto& bytes = st->font_bytes.back();

    ve_font_id id = ve_fontcache_load( &st->cache, bytes.data(), bytes.size(), size_px );
    if ( id < 0 )
    {
        st->font_bytes.pop_back();
        return VXUI_FONT_INVALID;
    }
    assert( id < (ve_font_id) VXUI_FONT_INVALID );

    if ( st->default_font < 0 ) st->default_font = (int) id;

    return (vxui_font_id) id;
}

void vxui_shutdown( vxui_ctx* ctx )
{
    assert( ctx );
    if ( !ctx->text ) return;
    vxui_text_state* st = (vxui_text_state*) ctx->text;
    ve_fontcache_shutdown( &st->cache );
    delete st;
    ctx->text = nullptr;
}

#endif // #ifdef VXUI_IMPL
