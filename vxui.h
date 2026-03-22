#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <cstring>
#include <string>

#include "clay/clay.h"
typedef struct ve_fontcache ve_fontcache;

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

    /* vxui_anim_state* anim_states; */
    /* int anim_state_count; */
    /* int anim_state_capacity; */

    /* uint32_t focused_id; */
} vxui_ctx;

uint64_t vxui_min_memory_size( void );
vxui_arena vxui_create_arena( uint64_t size, void* memory );
void vxui_init( vxui_ctx* ctx, vxui_arena arena, vxui_config cfg );
void vxui_begin( vxui_ctx* ctx, float delta_time );
vxui_draw_list vxui_end( vxui_ctx* ctx );
void vxui_flush_text( vxui_ctx* ctx );
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
};

static void* vxui__arena_alloc( vxui_arena* arena, uint64_t size, uint64_t align );
static uint32_t vxui__hash_bytes( const void* data, size_t size, uint32_t seed );
static Clay_String vxui__clay_string_from_cstr( const char* text );
static void vxui__reset_frame_buffers( vxui_ctx* ctx );
static vxui_cmd* vxui__push_cmd( vxui_ctx* ctx, vxui_cmd_type type );
static vxui_draw_cmd_text* vxui__push_text( vxui_ctx* ctx );
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

static Clay_String vxui__clay_string_from_cstr( const char* text )
{
    Clay_String ret = {};
    ret.isStaticallyAllocated = false;
    ret.length = text ? ( int32_t ) std::strlen( text ) : 0;
    ret.chars = text ? text : "";
    return ret;
}

static void vxui__reset_frame_buffers( vxui_ctx* ctx )
{
    ctx->command_count = 0;
    ctx->text_queue_count = 0;
    ctx->pending_nav_mask = 0;
    ctx->pending_confirm = false;
    ctx->pending_cancel = false;
    ctx->pending_tab = 0;
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
    uint64_t command_bytes = ( uint64_t ) VXUI__DEFAULT_MAX_ELEMENTS * ( uint64_t ) sizeof( vxui_cmd );
    uint64_t text_bytes = ( uint64_t ) VXUI__DEFAULT_MAX_ELEMENTS * ( uint64_t ) sizeof( vxui_draw_cmd_text );
    uint64_t slack_bytes = 64u * 1024u;
    return clay_bytes + command_bytes + text_bytes + slack_bytes;
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

    ctx->commands = ( vxui_cmd* ) vxui__arena_alloc(
        &ctx->arena,
        ( uint64_t ) ctx->cfg.max_elements * ( uint64_t ) sizeof( vxui_cmd ),
        ( uint64_t ) alignof( vxui_cmd ) );
    if ( ctx->commands ) {
        ctx->command_capacity = ctx->cfg.max_elements;
    }

    ctx->text_queue = ( vxui_draw_cmd_text* ) vxui__arena_alloc(
        &ctx->arena,
        ( uint64_t ) ctx->cfg.max_elements * ( uint64_t ) sizeof( vxui_draw_cmd_text ),
        ( uint64_t ) alignof( vxui_draw_cmd_text ) );
    if ( ctx->text_queue ) {
        ctx->text_queue_capacity = ctx->cfg.max_elements;
    }

    vxui__reset_frame_buffers( ctx );
}

void vxui_begin( vxui_ctx* ctx, float delta_time )
{
    ctx->frame_index += 1;
    ctx->delta_time = delta_time;
    vxui__reset_frame_buffers( ctx );
}

vxui_draw_list vxui_end( vxui_ctx* ctx )
{
    vxui_draw_list list = {};
    list.commands = ctx->commands;
    list.length = ctx->command_count;
    return list;
}

void vxui_flush_text( vxui_ctx* ctx )
{
    ( void ) ctx;
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
