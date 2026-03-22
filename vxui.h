#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string>

#include "clay/clay.h"
#include "vefc/ve_fontcache.h"

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

uint64_t vxui_min_memory_size( void );
vxui_arena vxui_create_arena( uint64_t size, void* memory );

#ifdef VXUI_IMPL
/* implementation lands in later phases */
#endif // VXUI_IMPL
