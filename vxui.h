#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>

#include "clay/clay.h"
typedef struct ve_fontcache ve_fontcache;

#define VXUI( ctx, id_str, ... ) \
    CLAY( CLAY_ID( id_str ), __VA_ARGS__ )

#define VXUI_DEFAULT_STIFFNESS 200.0f
#define VXUI_DEFAULT_DAMPING 20.0f
#define VXUI_SETTLE_EPSILON 0.001f

typedef struct vxui_vec2
{
    float x;
    float y;
} vxui_vec2;

typedef struct vxui_rect
{
    float x;
    float y;
    float w;
    float h;
} vxui_rect;

typedef struct vxui_color
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} vxui_color;

typedef struct vxui_arena
{
    uint8_t* memory;
    uint64_t capacity;
    uint64_t offset;
} vxui_arena;

typedef struct vxui_ctx vxui_ctx;

typedef void ( *vxui_action_fn )( vxui_ctx* ctx, uint32_t id, void* userdata );

typedef struct vxui_label_cfg
{
    uint32_t font_id;
    float font_size;
    vxui_color color;
} vxui_label_cfg;

typedef struct vxui_value_cfg
{
    uint32_t font_id;
    float font_size;
    vxui_color color;
    const char* format;
    float spring_speed;
} vxui_value_cfg;

typedef struct vxui_action_cfg
{
    uint32_t nav_up;
    uint32_t nav_down;
    uint32_t nav_left;
    uint32_t nav_right;
    bool no_focus_ring;
    bool disabled;
    void* userdata;
} vxui_action_cfg;

typedef enum vxui_cmd_type
{
    VXUI_CMD_RECT,
    VXUI_CMD_RECT_ROUNDED,
    VXUI_CMD_IMAGE,
    VXUI_CMD_TEXT,
    VXUI_CMD_CLIP_PUSH,
    VXUI_CMD_CLIP_POP,
} vxui_cmd_type;

typedef struct vxui_draw_cmd_rect
{
    vxui_rect bounds;
    vxui_color color;
} vxui_draw_cmd_rect;

typedef struct vxui_draw_cmd_rect_rounded
{
    vxui_rect bounds;
    vxui_color color;
    float radius;
} vxui_draw_cmd_rect_rounded;

typedef struct vxui_draw_cmd_image
{
    vxui_rect bounds;
    void* image_data;
} vxui_draw_cmd_image;

typedef struct vxui_draw_cmd_text
{
    vxui_vec2 pos;
    uint32_t font_id;
    float size;
    vxui_color color;
    const char* text;
} vxui_draw_cmd_text;

typedef struct vxui_cmd
{
    vxui_cmd_type type;
    vxui_rect clip_rect;
    union
    {
        vxui_draw_cmd_rect rect;
        vxui_draw_cmd_rect_rounded rect_rounded;
        vxui_draw_cmd_image image;
        vxui_draw_cmd_text text;
        struct
        {
            vxui_rect rect;
        } clip;
    };
} vxui_cmd;

typedef struct vxui_focus_ring_cfg
{
    vxui_color color;
    float border_width;
    float corner_radius;
    float spring_stiffness;
    float spring_damping;
} vxui_focus_ring_cfg;

typedef struct vxui_config
{
    int screen_width;
    int screen_height;
    int exit_anim_max_time;
    vxui_focus_ring_cfg focus_ring;
    int max_elements;
    int max_anim_states;
    int max_sequences;
    int max_seq_steps;
} vxui_config;

typedef struct vxui_draw_list
{
    vxui_cmd* commands;
    int length;
} vxui_draw_list;

typedef enum vxui_prop
{
    VXUI_PROP_OPACITY,
    VXUI_PROP_SCALE,
    VXUI_PROP_SLIDE_X,
    VXUI_PROP_SLIDE_Y,
} vxui_prop;

typedef struct vxui_anim_state
{
    uint32_t id;
    uint64_t last_seen_frame;
    vxui_rect bounds;

    float opacity_current;
    float opacity_target;
    float opacity_velocity;

    float scale_current;
    float scale_target;
    float scale_velocity;

    float slide_x_current;
    float slide_x_target;
    float slide_x_velocity;

    float slide_y_current;
    float slide_y_target;
    float slide_y_velocity;

    float color_r_current;
    float color_r_target;
    float color_r_velocity;

    float color_g_current;
    float color_g_target;
    float color_g_velocity;

    float color_b_current;
    float color_b_target;
    float color_b_velocity;

    float color_a_current;
    float color_a_target;
    float color_a_velocity;
} vxui_anim_state;

typedef struct vxui_anim_slot
{
    bool occupied;
    vxui_anim_state state;
} vxui_anim_slot;

typedef enum vxui_decl_kind
{
    VXUI_DECL_LABEL,
    VXUI_DECL_VALUE,
    VXUI_DECL_ACTION,
} vxui_decl_kind;

typedef struct vxui_decl
{
    vxui_decl_kind kind;
    uint32_t id;
    uint32_t nav_up;
    uint32_t nav_down;
    uint32_t nav_left;
    uint32_t nav_right;
    bool focusable;
    bool disabled;
    bool no_focus_ring;
    vxui_action_fn on_confirm;
    void* userdata;
} vxui_decl;

typedef struct vxui_ctx
{
    vxui_config cfg;
    vxui_arena arena;

    uint64_t frame_index;
    float delta_time;

    ve_fontcache* fontcache;

    vxui_cmd* commands;
    int command_count;
    int command_capacity;

    vxui_draw_cmd_text* text_queue;
    int text_queue_count;
    int text_queue_capacity;

    uint32_t pending_nav_mask;
    bool pending_confirm;
    bool pending_cancel;
    int pending_tab;

    Clay_Context* clay_ctx;
    Clay_Arena clay_arena;
    Clay_ErrorHandler clay_error_handler;
    const char* ( *text_fn )( const char* key, void* userdata );
    void* text_fn_userdata;
    Clay_RenderCommandArray clay_render_commands;
    vxui_rect current_clip_rect;
    bool clip_active;

    vxui_rect* clip_stack;
    int clip_stack_count;
    int clip_stack_capacity;

    char* frame_string_buffer;
    int frame_string_count;
    int frame_string_capacity;

    vxui_decl* decls;
    int decl_count;
    int decl_capacity;

    uint32_t default_font_id;
    float default_font_size;
    vxui_color default_text_color;

    vxui_anim_slot* anim_slots;
    int anim_capacity;
    vxui_cmd* anim_retained_commands;
    bool* anim_retained_valid;
    char* anim_retained_text;
    int anim_retained_text_stride;

    /* uint32_t focused_id; */
} vxui_ctx;

uint64_t vxui_min_memory_size( void );
vxui_arena vxui_create_arena( uint64_t size, void* memory );
void vxui_init( vxui_ctx* ctx, vxui_arena arena, vxui_config cfg );
void vxui_begin( vxui_ctx* ctx, float delta_time );
vxui_draw_list vxui_end( vxui_ctx* ctx );
void vxui_flush_text( vxui_ctx* ctx );
void vxui_set_fontcache( vxui_ctx* ctx, ve_fontcache* cache );
void vxui_set_text_fn( vxui_ctx* ctx, const char* ( *fn )( const char* key, void* userdata ), void* userdata );
void VXUI_LABEL( vxui_ctx* ctx, const char* l10n_key, vxui_label_cfg cfg );
void VXUI_VALUE( vxui_ctx* ctx, const char* l10n_key, float value, vxui_value_cfg cfg );
void VXUI_ACTION( vxui_ctx* ctx, const char* id, const char* l10n_key, vxui_action_fn fn, vxui_action_cfg cfg );
uint32_t vxui_id( const char* label );
uint32_t vxui_idi( const char* label, int index );

#ifdef VXUI_IMPL

#include "vefc/ve_fontcache.h"

enum
{
    VXUI__DEFAULT_SCREEN_WIDTH = 1920,
    VXUI__DEFAULT_SCREEN_HEIGHT = 1080,
    VXUI__DEFAULT_EXIT_ANIM_MAX_TIME = 500,
    VXUI__DEFAULT_MAX_ELEMENTS = 8192,
    VXUI__DEFAULT_MAX_ANIM_STATES = 1024,
    VXUI__DEFAULT_MAX_SEQUENCES = 64,
    VXUI__DEFAULT_MAX_SEQ_STEPS = 1024,
    VXUI__DEFAULT_FRAME_STRING_BYTES = VXUI__DEFAULT_MAX_ELEMENTS * 128,
    VXUI__DEFAULT_RETAINED_TEXT_BYTES = 256,
};

static void* vxui__arena_alloc( vxui_arena* arena, uint64_t size, uint64_t align );
static uint32_t vxui__hash_bytes( const void* data, size_t size, uint32_t seed );
static uint32_t vxui__next_pow2( uint32_t value );
static Clay_String vxui__clay_string_from_cstr( const char* text );
static Clay_ElementId vxui__clay_id_from_hash( uint32_t id );
static vxui_rect vxui__rect_from_clay_box( Clay_BoundingBox box );
static vxui_rect vxui__rect_union( vxui_rect a, vxui_rect b );
static vxui_rect vxui__transform_rect( vxui_rect rect, const vxui_anim_state* st );
static vxui_color vxui__color_from_clay( Clay_Color color );
static vxui_color vxui__apply_anim_color( vxui_color color, const vxui_anim_state* st );
static void vxui__handle_clay_error( Clay_ErrorData errorData );
static Clay_Dimensions vxui__measure_text( Clay_StringSlice text, Clay_TextElementConfig* cfg, void* userdata );
static void vxui__reset_frame_buffers( vxui_ctx* ctx );
static vxui_cmd* vxui__push_cmd( vxui_ctx* ctx, vxui_cmd_type type );
static vxui_draw_cmd_text* vxui__push_text( vxui_ctx* ctx );
static vxui_decl* vxui__push_decl( vxui_ctx* ctx );
static const char* vxui__push_frame_string( vxui_ctx* ctx, const char* text, size_t length );
static const char* vxui__copy_slice_text( vxui_ctx* ctx, Clay_StringSlice text );
static const char* vxui__resolve_text( vxui_ctx* ctx, const char* key );
static uint32_t vxui__effective_font_id( vxui_ctx* ctx, uint32_t font_id );
static float vxui__effective_font_size( vxui_ctx* ctx, float font_size );
static vxui_color vxui__effective_text_color( vxui_ctx* ctx, vxui_color color );
static void vxui__emit_text( vxui_ctx* ctx, const char* text, uint32_t font_id, float font_size, vxui_color color, uintptr_t owner_id = 0 );
static void vxui__register_action( vxui_ctx* ctx, uint32_t id, vxui_action_fn fn, vxui_action_cfg cfg );
static vxui_anim_slot* vxui__get_anim_slot( vxui_ctx* ctx, uint32_t id, bool create );
static vxui_anim_state* vxui__get_anim_state( vxui_ctx* ctx, uint32_t id, bool create );
static void vxui__spring_step( float target, float* current, float* velocity, float stiffness, float damping, float dt );
static bool vxui__anim_settled( const vxui_anim_state* st );
static void vxui__mark_seen( vxui_anim_state* st, uint64_t frame_index, vxui_rect bounds );
static void vxui__apply_anim_to_cmd( vxui_cmd* cmd, const vxui_anim_state* st );
static void vxui__scan_decl_anim_bounds( vxui_ctx* ctx );
static void vxui__scan_clay_anim_states( vxui_ctx* ctx );
static void vxui__update_anim_store( vxui_ctx* ctx );
static void vxui__capture_retained_cmd( vxui_ctx* ctx, uint32_t id, const vxui_cmd* cmd );
static void vxui__emit_retained_anim_commands( vxui_ctx* ctx );
static void vxui__evict_dead_anim_states( vxui_ctx* ctx );
static void vxui__translate_clay_commands( vxui_ctx* ctx );
static vxui_config vxui__sanitize_config( vxui_config cfg );

static void* vxui__arena_alloc( vxui_arena* arena, uint64_t size, uint64_t align )
{
    if ( align == 0 ) {
        align = 1;
    }

    uint64_t mask = align - 1;
    uint64_t base = ( uint64_t ) ( uintptr_t ) arena->memory;
    uint64_t at = ( ( base + arena->offset + mask ) & ~mask ) - base;
    if ( at + size > arena->capacity ) {
        return nullptr;
    }

    void* ret = arena->memory + at;
    arena->offset = at + size;
    return ret;
}

static uint32_t vxui__hash_bytes( const void* data, size_t size, uint32_t seed )
{
    const uint8_t* bytes = ( const uint8_t* ) data;
    uint32_t hash = 2166136261u ^ seed;

    for ( size_t i = 0; i < size; ++i ) {
        hash ^= bytes[ i ];
        hash *= 16777619u;
    }

    return hash;
}

static uint32_t vxui__next_pow2( uint32_t value )
{
    if ( value <= 1u ) {
        return 1u;
    }

    value -= 1u;
    value |= value >> 1u;
    value |= value >> 2u;
    value |= value >> 4u;
    value |= value >> 8u;
    value |= value >> 16u;
    return value + 1u;
}

static Clay_String vxui__clay_string_from_cstr( const char* text )
{
    Clay_String ret = {};
    ret.isStaticallyAllocated = false;
    ret.length = text ? ( int32_t ) std::strlen( text ) : 0;
    ret.chars = text ? text : "";
    return ret;
}

static Clay_ElementId vxui__clay_id_from_hash( uint32_t id )
{
    Clay_ElementId ret = {};
    ret.id = id;
    return ret;
}

static vxui_rect vxui__rect_from_clay_box( Clay_BoundingBox box )
{
    vxui_rect rect = {};
    rect.x = box.x;
    rect.y = box.y;
    rect.w = box.width;
    rect.h = box.height;
    return rect;
}

static vxui_rect vxui__rect_union( vxui_rect a, vxui_rect b )
{
    if ( a.w <= 0.0f || a.h <= 0.0f ) {
        return b;
    }
    if ( b.w <= 0.0f || b.h <= 0.0f ) {
        return a;
    }

    float left = a.x < b.x ? a.x : b.x;
    float top = a.y < b.y ? a.y : b.y;
    float right = ( a.x + a.w ) > ( b.x + b.w ) ? ( a.x + a.w ) : ( b.x + b.w );
    float bottom = ( a.y + a.h ) > ( b.y + b.h ) ? ( a.y + a.h ) : ( b.y + b.h );
    return ( vxui_rect ) { left, top, right - left, bottom - top };
}

static vxui_rect vxui__transform_rect( vxui_rect rect, const vxui_anim_state* st )
{
    float cx = rect.x + rect.w * 0.5f;
    float cy = rect.y + rect.h * 0.5f;
    float scale = st ? st->scale_current : 1.0f;
    float half_w = rect.w * scale * 0.5f;
    float half_h = rect.h * scale * 0.5f;

    vxui_rect ret = rect;
    ret.x = cx - half_w + ( st ? st->slide_x_current : 0.0f );
    ret.y = cy - half_h + ( st ? st->slide_y_current : 0.0f );
    ret.w = half_w * 2.0f;
    ret.h = half_h * 2.0f;
    return ret;
}

static vxui_color vxui__color_from_clay( Clay_Color color )
{
    vxui_color ret = {};

    float r = color.r < 0.0f ? 0.0f : ( color.r > 255.0f ? 255.0f : color.r );
    float g = color.g < 0.0f ? 0.0f : ( color.g > 255.0f ? 255.0f : color.g );
    float b = color.b < 0.0f ? 0.0f : ( color.b > 255.0f ? 255.0f : color.b );
    float a = color.a < 0.0f ? 0.0f : ( color.a > 255.0f ? 255.0f : color.a );

    ret.r = ( uint8_t ) r;
    ret.g = ( uint8_t ) g;
    ret.b = ( uint8_t ) b;
    ret.a = ( uint8_t ) a;
    return ret;
}

static vxui_color vxui__apply_anim_color( vxui_color color, const vxui_anim_state* st )
{
    if ( !st ) {
        return color;
    }

    auto clamp_channel = []( float value ) -> uint8_t {
        float rounded = std::round( value );
        if ( rounded < 0.0f ) {
            rounded = 0.0f;
        }
        if ( rounded > 255.0f ) {
            rounded = 255.0f;
        }
        return ( uint8_t ) rounded;
    };

    float opacity = st->opacity_current * st->color_a_current;
    return ( vxui_color ) {
        clamp_channel( ( float ) color.r * st->color_r_current ),
        clamp_channel( ( float ) color.g * st->color_g_current ),
        clamp_channel( ( float ) color.b * st->color_b_current ),
        clamp_channel( ( float ) color.a * opacity ),
    };
}

static void vxui__handle_clay_error( Clay_ErrorData errorData )
{
    std::fprintf(
        stderr,
        "vxui clay error: %.*s\n",
        errorData.errorText.length,
        errorData.errorText.chars ? errorData.errorText.chars : "" );
}

static Clay_Dimensions vxui__measure_text( Clay_StringSlice text, Clay_TextElementConfig* cfg, void* userdata )
{
    vxui_ctx* ctx = ( vxui_ctx* ) userdata;
    Clay_Dimensions dims = {};
    dims.height = cfg ? ( float ) cfg->fontSize : 0.0f;

    if ( !ctx || !ctx->fontcache || !cfg ) {
        return dims;
    }

    ve_fontcache* cache = ctx->fontcache;
    if ( cfg->fontId >= cache->entry.size() ) {
        return dims;
    }

    size_t verts = cache->drawlist.vertices.size();
    size_t indices = cache->drawlist.indices.size();
    size_t dcalls = cache->drawlist.dcalls.size();
    ve_fontcache_vec2 cursor = cache->cursor_pos;

    std::u8string temp( ( const char8_t* ) text.chars, ( size_t ) text.length );
    ve_fontcache_draw_text( cache, ( ve_font_id ) cfg->fontId, temp, 0.0f, 0.0f, 1.0f, 1.0f, true );
    ve_fontcache_vec2 end = ve_fontcache_get_cursor_pos( cache );

    cache->drawlist.vertices.resize( verts );
    cache->drawlist.indices.resize( indices );
    cache->drawlist.dcalls.resize( dcalls );
    cache->cursor_pos = cursor;

    dims.width = end.x;
    return dims;
}

static void vxui__reset_frame_buffers( vxui_ctx* ctx )
{
    ctx->command_count = 0;
    ctx->text_queue_count = 0;
    ctx->pending_nav_mask = 0;
    ctx->pending_confirm = false;
    ctx->pending_cancel = false;
    ctx->pending_tab = 0;
    ctx->clay_render_commands = Clay_RenderCommandArray {};
    ctx->current_clip_rect = vxui_rect {};
    ctx->clip_active = false;
    ctx->clip_stack_count = 0;
    ctx->frame_string_count = 0;
    ctx->decl_count = 0;
}

static vxui_cmd* vxui__push_cmd( vxui_ctx* ctx, vxui_cmd_type type )
{
    if ( ctx->command_count >= ctx->command_capacity ) {
        return nullptr;
    }

    vxui_cmd* cmd = &ctx->commands[ ctx->command_count++ ];
    *cmd = vxui_cmd {};
    cmd->type = type;
    return cmd;
}

static vxui_draw_cmd_text* vxui__push_text( vxui_ctx* ctx )
{
    if ( ctx->text_queue_count >= ctx->text_queue_capacity ) {
        return nullptr;
    }

    vxui_draw_cmd_text* text = &ctx->text_queue[ ctx->text_queue_count++ ];
    *text = vxui_draw_cmd_text {};
    return text;
}

static vxui_decl* vxui__push_decl( vxui_ctx* ctx )
{
    if ( ctx->decl_count >= ctx->decl_capacity ) {
        return nullptr;
    }

    vxui_decl* decl = &ctx->decls[ ctx->decl_count++ ];
    *decl = vxui_decl {};
    return decl;
}

static const char* vxui__push_frame_string( vxui_ctx* ctx, const char* text, size_t length )
{
    if ( !ctx->frame_string_buffer || ctx->frame_string_count + ( int ) length + 1 > ctx->frame_string_capacity ) {
        assert( false && "vxui frame string buffer exhausted" );
        return "";
    }

    char* dest = ctx->frame_string_buffer + ctx->frame_string_count;
    if ( length > 0 ) {
        std::memcpy( dest, text, length );
    }
    dest[ length ] = '\0';
    ctx->frame_string_count += ( int ) length + 1;
    return dest;
}

static const char* vxui__copy_slice_text( vxui_ctx* ctx, Clay_StringSlice text )
{
    if ( !text.chars || text.length <= 0 ) {
        return vxui__push_frame_string( ctx, "", 0 );
    }
    return vxui__push_frame_string( ctx, text.chars, ( size_t ) text.length );
}

static const char* vxui__resolve_text( vxui_ctx* ctx, const char* key )
{
    if ( ctx->text_fn ) {
        const char* ret = ctx->text_fn( key, ctx->text_fn_userdata );
        if ( ret ) {
            return ret;
        }
    }
    return key;
}

static uint32_t vxui__effective_font_id( vxui_ctx* ctx, uint32_t font_id )
{
    return font_id != 0 ? font_id : ctx->default_font_id;
}

static float vxui__effective_font_size( vxui_ctx* ctx, float font_size )
{
    return font_size > 0.0f ? font_size : ctx->default_font_size;
}

static vxui_color vxui__effective_text_color( vxui_ctx* ctx, vxui_color color )
{
    if ( color.r == 0 && color.g == 0 && color.b == 0 && color.a == 0 ) {
        return ctx->default_text_color;
    }
    return color;
}

static void vxui__emit_text( vxui_ctx* ctx, const char* text, uint32_t font_id, float font_size, vxui_color color, uintptr_t owner_id )
{
    const char* copied = vxui__push_frame_string( ctx, text ? text : "", text ? std::strlen( text ) : 0 );
    CLAY_TEXT(
        vxui__clay_string_from_cstr( copied ),
        CLAY_TEXT_CONFIG( {
            .userData = ( void* ) owner_id,
            .textColor = { ( float ) color.r, ( float ) color.g, ( float ) color.b, ( float ) color.a },
            .fontId = ( uint16_t ) font_id,
            .fontSize = ( uint16_t ) font_size,
        } ) );
}

static void vxui__register_action( vxui_ctx* ctx, uint32_t id, vxui_action_fn fn, vxui_action_cfg cfg )
{
    vxui_decl* decl = vxui__push_decl( ctx );
    if ( !decl ) {
        return;
    }

    decl->kind = VXUI_DECL_ACTION;
    decl->id = id;
    decl->focusable = !cfg.disabled;
    decl->disabled = cfg.disabled;
    decl->no_focus_ring = cfg.no_focus_ring;
    decl->nav_up = cfg.nav_up;
    decl->nav_down = cfg.nav_down;
    decl->nav_left = cfg.nav_left;
    decl->nav_right = cfg.nav_right;
    decl->on_confirm = fn;
    decl->userdata = cfg.userdata;
}

static vxui_anim_slot* vxui__get_anim_slot( vxui_ctx* ctx, uint32_t id, bool create )
{
    if ( !ctx->anim_slots || ctx->anim_capacity <= 0 ) {
        return nullptr;
    }

    uint32_t mask = ( uint32_t ) ctx->anim_capacity - 1u;
    uint32_t slot_index = id & mask;
    for ( int i = 0; i < ctx->anim_capacity; ++i ) {
        vxui_anim_slot* slot = &ctx->anim_slots[ slot_index ];
        if ( !slot->occupied ) {
            if ( !create ) {
                return nullptr;
            }

            std::memset( &slot->state, 0, sizeof( slot->state ) );
            slot->occupied = true;
            slot->state.id = id;
            slot->state.opacity_current = 1.0f;
            slot->state.opacity_target = 1.0f;
            slot->state.scale_current = 1.0f;
            slot->state.scale_target = 1.0f;
            slot->state.color_r_current = 1.0f;
            slot->state.color_r_target = 1.0f;
            slot->state.color_g_current = 1.0f;
            slot->state.color_g_target = 1.0f;
            slot->state.color_b_current = 1.0f;
            slot->state.color_b_target = 1.0f;
            slot->state.color_a_current = 1.0f;
            slot->state.color_a_target = 1.0f;

            if ( ctx->anim_retained_valid ) {
                ctx->anim_retained_valid[ slot_index ] = false;
            }
            return slot;
        }

        if ( slot->state.id == id ) {
            return slot;
        }

        slot_index = ( slot_index + 1u ) & mask;
    }

    assert( false && "vxui animation table exhausted" );
    return nullptr;
}

static vxui_anim_state* vxui__get_anim_state( vxui_ctx* ctx, uint32_t id, bool create )
{
    vxui_anim_slot* slot = vxui__get_anim_slot( ctx, id, create );
    return slot ? &slot->state : nullptr;
}

static void vxui__spring_step(
    float target,
    float* current,
    float* velocity,
    float stiffness,
    float damping,
    float dt )
{
    if ( !current || !velocity || dt <= 0.0f ) {
        return;
    }

    float safe_stiffness = stiffness > 0.0f ? stiffness : VXUI_DEFAULT_STIFFNESS;
    float safe_damping = damping > 0.0f ? damping : VXUI_DEFAULT_DAMPING;
    float max_step = 1.0f / 240.0f;
    int steps = ( int ) std::ceil( dt / max_step );
    if ( steps < 1 ) {
        steps = 1;
    }

    float step_dt = dt / ( float ) steps;
    for ( int i = 0; i < steps; ++i ) {
        float accel = safe_stiffness * ( target - *current ) - safe_damping * *velocity;
        *velocity += accel * step_dt;
        *current += *velocity * step_dt;
    }

    if ( std::fabs( target - *current ) <= VXUI_SETTLE_EPSILON && std::fabs( *velocity ) <= VXUI_SETTLE_EPSILON ) {
        *current = target;
        *velocity = 0.0f;
    }
}

static bool vxui__anim_settled( const vxui_anim_state* st )
{
    if ( !st ) {
        return true;
    }

    auto near = []( float current, float target, float velocity ) -> bool {
        return std::fabs( current - target ) <= VXUI_SETTLE_EPSILON && std::fabs( velocity ) <= VXUI_SETTLE_EPSILON;
    };

    return near( st->opacity_current, st->opacity_target, st->opacity_velocity ) &&
           near( st->scale_current, st->scale_target, st->scale_velocity ) &&
           near( st->slide_x_current, st->slide_x_target, st->slide_x_velocity ) &&
           near( st->slide_y_current, st->slide_y_target, st->slide_y_velocity ) &&
           near( st->color_r_current, st->color_r_target, st->color_r_velocity ) &&
           near( st->color_g_current, st->color_g_target, st->color_g_velocity ) &&
           near( st->color_b_current, st->color_b_target, st->color_b_velocity ) &&
           near( st->color_a_current, st->color_a_target, st->color_a_velocity );
}

static void vxui__mark_seen( vxui_anim_state* st, uint64_t frame_index, vxui_rect bounds )
{
    if ( !st ) {
        return;
    }

    if ( st->last_seen_frame == frame_index ) {
        st->bounds = vxui__rect_union( st->bounds, bounds );
    } else {
        st->bounds = bounds;
    }
    st->last_seen_frame = frame_index;
}

static void vxui__apply_anim_to_cmd( vxui_cmd* cmd, const vxui_anim_state* st )
{
    if ( !cmd || !st ) {
        return;
    }

    switch ( cmd->type ) {
        case VXUI_CMD_RECT:
            cmd->rect.bounds = vxui__transform_rect( cmd->rect.bounds, st );
            cmd->rect.color = vxui__apply_anim_color( cmd->rect.color, st );
            break;

        case VXUI_CMD_RECT_ROUNDED:
            cmd->rect_rounded.bounds = vxui__transform_rect( cmd->rect_rounded.bounds, st );
            cmd->rect_rounded.color = vxui__apply_anim_color( cmd->rect_rounded.color, st );
            cmd->rect_rounded.radius *= st->scale_current;
            break;

        case VXUI_CMD_IMAGE:
            cmd->image.bounds = vxui__transform_rect( cmd->image.bounds, st );
            break;

        case VXUI_CMD_TEXT: {
            vxui_rect text_bounds = st->bounds;
            if ( text_bounds.w <= 0.0f ) {
                text_bounds.w = cmd->text.size;
            }
            if ( text_bounds.h <= 0.0f ) {
                text_bounds.h = cmd->text.size;
            }

            text_bounds = vxui__transform_rect( text_bounds, st );
            cmd->text.pos = ( vxui_vec2 ) { text_bounds.x, text_bounds.y };
            cmd->text.size *= st->scale_current;
            cmd->text.color = vxui__apply_anim_color( cmd->text.color, st );
            break;
        }

        case VXUI_CMD_CLIP_PUSH:
        case VXUI_CMD_CLIP_POP:
            cmd->clip.rect = vxui__transform_rect( cmd->clip.rect, st );
            break;
    }
}

static void vxui__scan_decl_anim_bounds( vxui_ctx* ctx )
{
    if ( !ctx->clay_ctx ) {
        return;
    }

    for ( int i = 0; i < ctx->decl_count; ++i ) {
        vxui_decl* decl = &ctx->decls[ i ];
        vxui_anim_state* st = vxui__get_anim_state( ctx, decl->id, true );
        Clay_ElementData data = Clay_GetElementData( vxui__clay_id_from_hash( decl->id ) );
        if ( data.found ) {
            vxui__mark_seen( st, ctx->frame_index, vxui__rect_from_clay_box( data.boundingBox ) );
        }
    }
}

static void vxui__scan_clay_anim_states( vxui_ctx* ctx )
{
    for ( int32_t i = 0; i < ctx->clay_render_commands.length; ++i ) {
        Clay_RenderCommand* src = Clay_RenderCommandArray_Get( &ctx->clay_render_commands, i );
        if ( !src ) {
            continue;
        }

        switch ( src->commandType ) {
            case CLAY_RENDER_COMMAND_TYPE_RECTANGLE:
            case CLAY_RENDER_COMMAND_TYPE_TEXT:
            case CLAY_RENDER_COMMAND_TYPE_IMAGE:
            case CLAY_RENDER_COMMAND_TYPE_SCISSOR_START:
            case CLAY_RENDER_COMMAND_TYPE_SCISSOR_END: {
                vxui_anim_state* st = vxui__get_anim_state( ctx, src->id, true );
                vxui__mark_seen( st, ctx->frame_index, vxui__rect_from_clay_box( src->boundingBox ) );
                if ( src->commandType == CLAY_RENDER_COMMAND_TYPE_TEXT && src->userData ) {
                    vxui_anim_state* owner = vxui__get_anim_state( ctx, ( uint32_t ) ( uintptr_t ) src->userData, true );
                    vxui__mark_seen( owner, ctx->frame_index, vxui__rect_from_clay_box( src->boundingBox ) );
                }
                break;
            }

            case CLAY_RENDER_COMMAND_TYPE_NONE:
            case CLAY_RENDER_COMMAND_TYPE_BORDER:
            case CLAY_RENDER_COMMAND_TYPE_CUSTOM:
            default:
                break;
        }
    }
}

static void vxui__update_anim_store( vxui_ctx* ctx )
{
    for ( int i = 0; i < ctx->anim_capacity; ++i ) {
        vxui_anim_slot* slot = &ctx->anim_slots[ i ];
        if ( !slot->occupied ) {
            continue;
        }

        vxui_anim_state* st = &slot->state;
        bool seen = st->last_seen_frame == ctx->frame_index;
        st->opacity_target = seen ? 1.0f : 0.0f;
        st->scale_target = 1.0f;
        st->slide_x_target = 0.0f;
        st->slide_y_target = 0.0f;
        st->color_r_target = 1.0f;
        st->color_g_target = 1.0f;
        st->color_b_target = 1.0f;
        st->color_a_target = 1.0f;

        vxui__spring_step( st->opacity_target, &st->opacity_current, &st->opacity_velocity, VXUI_DEFAULT_STIFFNESS, VXUI_DEFAULT_DAMPING, ctx->delta_time );
        vxui__spring_step( st->scale_target, &st->scale_current, &st->scale_velocity, VXUI_DEFAULT_STIFFNESS, VXUI_DEFAULT_DAMPING, ctx->delta_time );
        vxui__spring_step( st->slide_x_target, &st->slide_x_current, &st->slide_x_velocity, VXUI_DEFAULT_STIFFNESS, VXUI_DEFAULT_DAMPING, ctx->delta_time );
        vxui__spring_step( st->slide_y_target, &st->slide_y_current, &st->slide_y_velocity, VXUI_DEFAULT_STIFFNESS, VXUI_DEFAULT_DAMPING, ctx->delta_time );
        vxui__spring_step( st->color_r_target, &st->color_r_current, &st->color_r_velocity, VXUI_DEFAULT_STIFFNESS, VXUI_DEFAULT_DAMPING, ctx->delta_time );
        vxui__spring_step( st->color_g_target, &st->color_g_current, &st->color_g_velocity, VXUI_DEFAULT_STIFFNESS, VXUI_DEFAULT_DAMPING, ctx->delta_time );
        vxui__spring_step( st->color_b_target, &st->color_b_current, &st->color_b_velocity, VXUI_DEFAULT_STIFFNESS, VXUI_DEFAULT_DAMPING, ctx->delta_time );
        vxui__spring_step( st->color_a_target, &st->color_a_current, &st->color_a_velocity, VXUI_DEFAULT_STIFFNESS, VXUI_DEFAULT_DAMPING, ctx->delta_time );
    }
}

static void vxui__capture_retained_cmd( vxui_ctx* ctx, uint32_t id, const vxui_cmd* cmd )
{
    if ( !cmd || !ctx->anim_retained_commands || !ctx->anim_retained_valid ) {
        return;
    }

    vxui_anim_slot* slot = vxui__get_anim_slot( ctx, id, false );
    if ( !slot ) {
        return;
    }

    int index = ( int ) ( slot - ctx->anim_slots );
    ctx->anim_retained_commands[ index ] = *cmd;
    if ( cmd->type == VXUI_CMD_TEXT && ctx->anim_retained_text && ctx->anim_retained_text_stride > 1 ) {
        char* dest = ctx->anim_retained_text + index * ctx->anim_retained_text_stride;
        size_t max_copy = ( size_t ) ctx->anim_retained_text_stride - 1u;
        size_t length = cmd->text.text ? std::strlen( cmd->text.text ) : 0u;
        if ( length > max_copy ) {
            length = max_copy;
        }
        if ( length > 0 ) {
            std::memcpy( dest, cmd->text.text, length );
        }
        dest[ length ] = '\0';
        ctx->anim_retained_commands[ index ].text.text = dest;
    }
    ctx->anim_retained_valid[ index ] = true;
}

static void vxui__emit_retained_anim_commands( vxui_ctx* ctx )
{
    if ( !ctx->anim_retained_commands || !ctx->anim_retained_valid ) {
        return;
    }

    for ( int i = 0; i < ctx->anim_capacity; ++i ) {
        vxui_anim_slot* slot = &ctx->anim_slots[ i ];
        if ( !slot->occupied || !ctx->anim_retained_valid[ i ] ) {
            continue;
        }

        vxui_anim_state* st = &slot->state;
        if ( st->last_seen_frame == ctx->frame_index ) {
            continue;
        }

        if ( st->opacity_current <= VXUI_SETTLE_EPSILON && st->color_a_current <= VXUI_SETTLE_EPSILON ) {
            continue;
        }

        if ( ctx->anim_retained_commands[ i ].type == VXUI_CMD_CLIP_PUSH || ctx->anim_retained_commands[ i ].type == VXUI_CMD_CLIP_POP ) {
            continue;
        }

        vxui_cmd* cmd = vxui__push_cmd( ctx, ctx->anim_retained_commands[ i ].type );
        if ( !cmd ) {
            continue;
        }

        *cmd = ctx->anim_retained_commands[ i ];
        vxui__apply_anim_to_cmd( cmd, st );
        if ( cmd->type == VXUI_CMD_TEXT ) {
            vxui_draw_cmd_text* queued = vxui__push_text( ctx );
            if ( queued ) {
                *queued = cmd->text;
            }
        }
    }
}

static void vxui__evict_dead_anim_states( vxui_ctx* ctx )
{
    for ( int i = 0; i < ctx->anim_capacity; ++i ) {
        vxui_anim_slot* slot = &ctx->anim_slots[ i ];
        if ( !slot->occupied ) {
            continue;
        }

        vxui_anim_state* st = &slot->state;
        if ( st->last_seen_frame == ctx->frame_index ) {
            continue;
        }

        float age_ms = ( float ) ( ctx->frame_index - st->last_seen_frame ) * ctx->delta_time * 1000.0f;
        if ( vxui__anim_settled( st ) || age_ms >= ( float ) ctx->cfg.exit_anim_max_time ) {
            slot->occupied = false;
            std::memset( &slot->state, 0, sizeof( slot->state ) );
            if ( ctx->anim_retained_valid ) {
                ctx->anim_retained_valid[ i ] = false;
            }
            if ( ctx->anim_retained_commands ) {
                ctx->anim_retained_commands[ i ] = vxui_cmd {};
            }
            if ( ctx->anim_retained_text && ctx->anim_retained_text_stride > 0 ) {
                ctx->anim_retained_text[ i * ctx->anim_retained_text_stride ] = '\0';
            }
        }
    }
}

static void vxui__translate_clay_commands( vxui_ctx* ctx )
{
    ctx->current_clip_rect = vxui_rect {};
    ctx->clip_active = false;
    ctx->clip_stack_count = 0;

    vxui__scan_decl_anim_bounds( ctx );
    vxui__scan_clay_anim_states( ctx );
    vxui__update_anim_store( ctx );

    for ( int32_t i = 0; i < ctx->clay_render_commands.length; ++i ) {
        Clay_RenderCommand* src = Clay_RenderCommandArray_Get( &ctx->clay_render_commands, i );
        if ( !src ) {
            continue;
        }

        vxui_anim_state* st = vxui__get_anim_state( ctx, src->id, false );

        switch ( src->commandType ) {
            case CLAY_RENDER_COMMAND_TYPE_RECTANGLE: {
                const Clay_RectangleRenderData* rect = &src->renderData.rectangle;
                float radius = rect->cornerRadius.topLeft;
                vxui_cmd* cmd = vxui__push_cmd( ctx, radius > 0.0f ? VXUI_CMD_RECT_ROUNDED : VXUI_CMD_RECT );
                if ( !cmd ) {
                    break;
                }

                cmd->clip_rect = ctx->current_clip_rect;
                if ( radius > 0.0f ) {
                    cmd->rect_rounded.bounds = vxui__rect_from_clay_box( src->boundingBox );
                    cmd->rect_rounded.color = vxui__color_from_clay( rect->backgroundColor );
                    cmd->rect_rounded.radius = radius;
                } else {
                    cmd->rect.bounds = vxui__rect_from_clay_box( src->boundingBox );
                    cmd->rect.color = vxui__color_from_clay( rect->backgroundColor );
                }
                vxui__capture_retained_cmd( ctx, src->id, cmd );
                vxui__apply_anim_to_cmd( cmd, st );
                break;
            }

            case CLAY_RENDER_COMMAND_TYPE_TEXT: {
                const Clay_TextRenderData* text = &src->renderData.text;
                const char* copied = vxui__copy_slice_text( ctx, text->stringContents );
                vxui_draw_cmd_text* queued = vxui__push_text( ctx );
                vxui_cmd* cmd = vxui__push_cmd( ctx, VXUI_CMD_TEXT );
                if ( !queued || !cmd ) {
                    break;
                }

                queued->pos = ( vxui_vec2 ) { src->boundingBox.x, src->boundingBox.y };
                queued->font_id = text->fontId;
                queued->size = ( float ) text->fontSize;
                queued->color = vxui__color_from_clay( text->textColor );
                queued->text = copied;

                cmd->clip_rect = ctx->current_clip_rect;
                cmd->text = *queued;
                vxui__capture_retained_cmd( ctx, src->id, cmd );
                vxui__apply_anim_to_cmd( cmd, st );
                *queued = cmd->text;
                break;
            }

            case CLAY_RENDER_COMMAND_TYPE_IMAGE: {
                const Clay_ImageRenderData* image = &src->renderData.image;
                vxui_cmd* cmd = vxui__push_cmd( ctx, VXUI_CMD_IMAGE );
                if ( !cmd ) {
                    break;
                }

                cmd->clip_rect = ctx->current_clip_rect;
                cmd->image.bounds = vxui__rect_from_clay_box( src->boundingBox );
                cmd->image.image_data = image->imageData;
                vxui__capture_retained_cmd( ctx, src->id, cmd );
                vxui__apply_anim_to_cmd( cmd, st );
                break;
            }

            case CLAY_RENDER_COMMAND_TYPE_SCISSOR_START: {
                if ( ctx->clip_active ) {
                    assert( ctx->clip_stack_count < ctx->clip_stack_capacity );
                    if ( ctx->clip_stack_count < ctx->clip_stack_capacity ) {
                        ctx->clip_stack[ ctx->clip_stack_count++ ] = ctx->current_clip_rect;
                    }
                }

                ctx->current_clip_rect = vxui__rect_from_clay_box( src->boundingBox );
                ctx->clip_active = true;

                vxui_cmd* cmd = vxui__push_cmd( ctx, VXUI_CMD_CLIP_PUSH );
                if ( !cmd ) {
                    break;
                }

                cmd->clip_rect = ctx->current_clip_rect;
                cmd->clip.rect = ctx->current_clip_rect;
                vxui__apply_anim_to_cmd( cmd, st );
                break;
            }

            case CLAY_RENDER_COMMAND_TYPE_SCISSOR_END: {
                vxui_cmd* cmd = vxui__push_cmd( ctx, VXUI_CMD_CLIP_POP );
                if ( cmd ) {
                    cmd->clip_rect = ctx->current_clip_rect;
                    cmd->clip.rect = ctx->current_clip_rect;
                    vxui__apply_anim_to_cmd( cmd, st );
                }

                if ( ctx->clip_stack_count > 0 ) {
                    ctx->current_clip_rect = ctx->clip_stack[ --ctx->clip_stack_count ];
                    ctx->clip_active = true;
                } else {
                    ctx->current_clip_rect = vxui_rect {};
                    ctx->clip_active = false;
                }
                break;
            }

            case CLAY_RENDER_COMMAND_TYPE_NONE:
            case CLAY_RENDER_COMMAND_TYPE_BORDER:
            case CLAY_RENDER_COMMAND_TYPE_CUSTOM:
            default:
                assert( false && "unsupported clay render command" );
                break;
        }
    }

    vxui__emit_retained_anim_commands( ctx );
    vxui__evict_dead_anim_states( ctx );
}

static vxui_config vxui__sanitize_config( vxui_config cfg )
{
    if ( cfg.screen_width <= 0 ) {
        cfg.screen_width = VXUI__DEFAULT_SCREEN_WIDTH;
    }
    if ( cfg.screen_height <= 0 ) {
        cfg.screen_height = VXUI__DEFAULT_SCREEN_HEIGHT;
    }
    if ( cfg.exit_anim_max_time <= 0 ) {
        cfg.exit_anim_max_time = VXUI__DEFAULT_EXIT_ANIM_MAX_TIME;
    }

    if ( cfg.focus_ring.color.a == 0 ) {
        cfg.focus_ring.color = ( vxui_color ) { 0, 255, 200, 255 };
    }
    if ( cfg.focus_ring.border_width <= 0.0f ) {
        cfg.focus_ring.border_width = 2.0f;
    }
    if ( cfg.focus_ring.corner_radius <= 0.0f ) {
        cfg.focus_ring.corner_radius = 4.0f;
    }
    if ( cfg.focus_ring.spring_stiffness <= 0.0f ) {
        cfg.focus_ring.spring_stiffness = 200.0f;
    }
    if ( cfg.focus_ring.spring_damping <= 0.0f ) {
        cfg.focus_ring.spring_damping = 20.0f;
    }

    if ( cfg.max_elements <= 0 ) {
        cfg.max_elements = VXUI__DEFAULT_MAX_ELEMENTS;
    }
    if ( cfg.max_anim_states <= 0 ) {
        cfg.max_anim_states = VXUI__DEFAULT_MAX_ANIM_STATES;
    }
    if ( cfg.max_sequences <= 0 ) {
        cfg.max_sequences = VXUI__DEFAULT_MAX_SEQUENCES;
    }
    if ( cfg.max_seq_steps <= 0 ) {
        cfg.max_seq_steps = VXUI__DEFAULT_MAX_SEQ_STEPS;
    }

    return cfg;
}

uint64_t vxui_min_memory_size( void )
{
    uint64_t clay_bytes = ( uint64_t ) Clay_MinMemorySize();
    uint64_t command_capacity = ( uint64_t ) VXUI__DEFAULT_MAX_ELEMENTS + ( uint64_t ) VXUI__DEFAULT_MAX_ANIM_STATES;
    uint64_t command_bytes = command_capacity * ( uint64_t ) sizeof( vxui_cmd );
    uint64_t text_bytes = command_capacity * ( uint64_t ) sizeof( vxui_draw_cmd_text );
    uint64_t clip_bytes = ( uint64_t ) VXUI__DEFAULT_MAX_ELEMENTS * ( uint64_t ) sizeof( vxui_rect );
    uint64_t decl_bytes = ( uint64_t ) VXUI__DEFAULT_MAX_ELEMENTS * ( uint64_t ) sizeof( vxui_decl );
    uint64_t anim_bytes = ( uint64_t ) VXUI__DEFAULT_MAX_ANIM_STATES * ( uint64_t ) sizeof( vxui_anim_slot );
    uint64_t retained_cmd_bytes = ( uint64_t ) VXUI__DEFAULT_MAX_ANIM_STATES * ( uint64_t ) sizeof( vxui_cmd );
    uint64_t retained_valid_bytes = ( uint64_t ) VXUI__DEFAULT_MAX_ANIM_STATES * ( uint64_t ) sizeof( bool );
    uint64_t retained_text_bytes = ( uint64_t ) VXUI__DEFAULT_MAX_ANIM_STATES * ( uint64_t ) VXUI__DEFAULT_RETAINED_TEXT_BYTES;
    uint64_t frame_bytes = ( uint64_t ) VXUI__DEFAULT_FRAME_STRING_BYTES;
    uint64_t slack_bytes = 64u * 1024u;
    return clay_bytes + command_bytes + text_bytes + clip_bytes + decl_bytes + anim_bytes + retained_cmd_bytes + retained_valid_bytes + retained_text_bytes + frame_bytes + slack_bytes;
}

vxui_arena vxui_create_arena( uint64_t size, void* memory )
{
    vxui_arena arena = {};
    arena.memory = ( uint8_t* ) memory;
    arena.capacity = size;
    arena.offset = 0;
    return arena;
}

void vxui_init( vxui_ctx* ctx, vxui_arena arena, vxui_config cfg )
{
    *ctx = vxui_ctx {};
    ctx->cfg = vxui__sanitize_config( cfg );
    ctx->arena = arena;

    void* clay_memory = vxui__arena_alloc(
        &ctx->arena,
        ( uint64_t ) Clay_MinMemorySize(),
        64 );
    if ( clay_memory ) {
        Clay_SetCurrentContext( nullptr );
        ctx->clay_error_handler.errorHandlerFunction = vxui__handle_clay_error;
        ctx->clay_error_handler.userData = ctx;
        ctx->clay_arena = Clay_CreateArenaWithCapacityAndMemory( Clay_MinMemorySize(), clay_memory );
        ctx->clay_ctx = Clay_Initialize(
            ctx->clay_arena,
            Clay_Dimensions { ( float ) ctx->cfg.screen_width, ( float ) ctx->cfg.screen_height },
            ctx->clay_error_handler );
        Clay_SetMeasureTextFunction( vxui__measure_text, ctx );
    }

    ctx->commands = ( vxui_cmd* ) vxui__arena_alloc(
        &ctx->arena,
        ( uint64_t ) ( ctx->cfg.max_elements + ctx->cfg.max_anim_states ) * ( uint64_t ) sizeof( vxui_cmd ),
        ( uint64_t ) alignof( vxui_cmd ) );
    if ( ctx->commands ) {
        ctx->command_capacity = ctx->cfg.max_elements + ctx->cfg.max_anim_states;
    }

    ctx->text_queue = ( vxui_draw_cmd_text* ) vxui__arena_alloc(
        &ctx->arena,
        ( uint64_t ) ( ctx->cfg.max_elements + ctx->cfg.max_anim_states ) * ( uint64_t ) sizeof( vxui_draw_cmd_text ),
        ( uint64_t ) alignof( vxui_draw_cmd_text ) );
    if ( ctx->text_queue ) {
        ctx->text_queue_capacity = ctx->cfg.max_elements + ctx->cfg.max_anim_states;
    }

    ctx->clip_stack = ( vxui_rect* ) vxui__arena_alloc(
        &ctx->arena,
        ( uint64_t ) ctx->cfg.max_elements * ( uint64_t ) sizeof( vxui_rect ),
        ( uint64_t ) alignof( vxui_rect ) );
    if ( ctx->clip_stack ) {
        ctx->clip_stack_capacity = ctx->cfg.max_elements;
    }

    ctx->frame_string_capacity = ctx->cfg.max_elements * 128;
    ctx->frame_string_buffer = ( char* ) vxui__arena_alloc(
        &ctx->arena,
        ( uint64_t ) ctx->frame_string_capacity,
        1 );
    if ( !ctx->frame_string_buffer ) {
        ctx->frame_string_capacity = 0;
    }

    ctx->decls = ( vxui_decl* ) vxui__arena_alloc(
        &ctx->arena,
        ( uint64_t ) ctx->cfg.max_elements * ( uint64_t ) sizeof( vxui_decl ),
        ( uint64_t ) alignof( vxui_decl ) );
    if ( ctx->decls ) {
        ctx->decl_capacity = ctx->cfg.max_elements;
    }

    ctx->anim_capacity = ( int ) vxui__next_pow2( ( uint32_t ) ctx->cfg.max_anim_states );
    ctx->anim_slots = ( vxui_anim_slot* ) vxui__arena_alloc(
        &ctx->arena,
        ( uint64_t ) ctx->anim_capacity * ( uint64_t ) sizeof( vxui_anim_slot ),
        ( uint64_t ) alignof( vxui_anim_slot ) );
    if ( ctx->anim_slots ) {
        std::memset( ctx->anim_slots, 0, ( size_t ) ctx->anim_capacity * sizeof( vxui_anim_slot ) );
    } else {
        ctx->anim_capacity = 0;
    }

    if ( ctx->anim_capacity > 0 ) {
        ctx->anim_retained_commands = ( vxui_cmd* ) vxui__arena_alloc(
            &ctx->arena,
            ( uint64_t ) ctx->anim_capacity * ( uint64_t ) sizeof( vxui_cmd ),
            ( uint64_t ) alignof( vxui_cmd ) );
        ctx->anim_retained_valid = ( bool* ) vxui__arena_alloc(
            &ctx->arena,
            ( uint64_t ) ctx->anim_capacity * ( uint64_t ) sizeof( bool ),
            ( uint64_t ) alignof( bool ) );
        ctx->anim_retained_text_stride = VXUI__DEFAULT_RETAINED_TEXT_BYTES;
        ctx->anim_retained_text = ( char* ) vxui__arena_alloc(
            &ctx->arena,
            ( uint64_t ) ctx->anim_capacity * ( uint64_t ) ctx->anim_retained_text_stride,
            1 );

        if ( ctx->anim_retained_commands ) {
            std::memset( ctx->anim_retained_commands, 0, ( size_t ) ctx->anim_capacity * sizeof( vxui_cmd ) );
        }
        if ( ctx->anim_retained_valid ) {
            std::memset( ctx->anim_retained_valid, 0, ( size_t ) ctx->anim_capacity * sizeof( bool ) );
        }
        if ( ctx->anim_retained_text ) {
            std::memset( ctx->anim_retained_text, 0, ( size_t ) ctx->anim_capacity * ( size_t ) ctx->anim_retained_text_stride );
        } else {
            ctx->anim_retained_text_stride = 0;
        }
    }

    ctx->default_font_id = 0;
    ctx->default_font_size = 24.0f;
    ctx->default_text_color = ( vxui_color ) { 255, 255, 255, 255 };

    vxui__reset_frame_buffers( ctx );
}

void vxui_begin( vxui_ctx* ctx, float delta_time )
{
    ctx->frame_index += 1;
    ctx->delta_time = delta_time;
    vxui__reset_frame_buffers( ctx );

    if ( ctx->clay_ctx ) {
        Clay_SetCurrentContext( ctx->clay_ctx );
        Clay_SetLayoutDimensions( Clay_Dimensions { ( float ) ctx->cfg.screen_width, ( float ) ctx->cfg.screen_height } );
        Clay_BeginLayout();
    }
}

vxui_draw_list vxui_end( vxui_ctx* ctx )
{
    vxui_draw_list list = {};

    if ( ctx->clay_ctx ) {
        Clay_SetCurrentContext( ctx->clay_ctx );
        ctx->clay_render_commands = Clay_EndLayout();
        vxui__translate_clay_commands( ctx );
    }

    list.commands = ctx->commands;
    list.length = ctx->command_count;
    return list;
}

void vxui_flush_text( vxui_ctx* ctx )
{
    ( void ) ctx;
}

void vxui_set_fontcache( vxui_ctx* ctx, ve_fontcache* cache )
{
    ctx->fontcache = cache;
}

void vxui_set_text_fn( vxui_ctx* ctx, const char* ( *fn )( const char* key, void* userdata ), void* userdata )
{
    ctx->text_fn = fn;
    ctx->text_fn_userdata = userdata;
}

void VXUI_LABEL( vxui_ctx* ctx, const char* l10n_key, vxui_label_cfg cfg )
{
    const char* resolved = vxui__resolve_text( ctx, l10n_key );
    uint32_t font_id = vxui__effective_font_id( ctx, cfg.font_id );
    float font_size = vxui__effective_font_size( ctx, cfg.font_size );
    vxui_color color = vxui__effective_text_color( ctx, cfg.color );

    vxui_decl* decl = vxui__push_decl( ctx );
    uint32_t decl_id = vxui_id( l10n_key );
    if ( decl ) {
        decl->kind = VXUI_DECL_LABEL;
        decl->id = decl_id;
    }
    vxui__get_anim_state( ctx, decl_id, true );

    CLAY( vxui__clay_id_from_hash( decl_id ), {} ) {
        vxui__emit_text( ctx, resolved, font_id, font_size, color, decl_id );
    }
}

void VXUI_VALUE( vxui_ctx* ctx, const char* l10n_key, float value, vxui_value_cfg cfg )
{
    const char* resolved = vxui__resolve_text( ctx, l10n_key );
    const char* format = cfg.format ? cfg.format : "%g";
    uint32_t font_id = vxui__effective_font_id( ctx, cfg.font_id );
    float font_size = vxui__effective_font_size( ctx, cfg.font_size );
    vxui_color color = vxui__effective_text_color( ctx, cfg.color );

    int needed = std::snprintf( nullptr, 0, format, value );
    if ( needed < 0 ) {
        needed = 0;
    }

    std::string formatted( ( size_t ) needed, '\0' );
    if ( needed > 0 ) {
        std::snprintf( formatted.data(), formatted.size() + 1, format, value );
    }

    vxui_decl* decl = vxui__push_decl( ctx );
    uint32_t decl_id = vxui_id( l10n_key );
    if ( decl ) {
        decl->kind = VXUI_DECL_VALUE;
        decl->id = decl_id;
    }
    vxui__get_anim_state( ctx, decl_id, true );

    CLAY( vxui__clay_id_from_hash( decl_id ), {
        .layout = {
            .childGap = 8,
            .layoutDirection = CLAY_LEFT_TO_RIGHT,
        },
    } ) {
        vxui__emit_text( ctx, resolved, font_id, font_size, color, decl_id );
        vxui__emit_text( ctx, formatted.c_str(), font_id, font_size, color, decl_id );
    }
}

void VXUI_ACTION( vxui_ctx* ctx, const char* id, const char* l10n_key, vxui_action_fn fn, vxui_action_cfg cfg )
{
    const char* resolved = vxui__resolve_text( ctx, l10n_key );
    uint32_t action_id = vxui_id( id );

    vxui__register_action( ctx, action_id, fn, cfg );
    vxui__get_anim_state( ctx, action_id, true );

    CLAY( vxui__clay_id_from_hash( action_id ), {
        .userData = cfg.userdata,
    } ) {
        vxui__emit_text(
            ctx,
            resolved,
            ctx->default_font_id,
            ctx->default_font_size,
            ctx->default_text_color,
            action_id );
    }
}

uint32_t vxui_id( const char* label )
{
    Clay_ElementId id = Clay_GetElementId( vxui__clay_string_from_cstr( label ) );
    return id.id;
}

uint32_t vxui_idi( const char* label, int index )
{
    Clay_ElementId id = Clay_GetElementIdWithIndex( vxui__clay_string_from_cstr( label ), ( uint32_t ) index );
    return id.id;
}

#endif // VXUI_IMPL
