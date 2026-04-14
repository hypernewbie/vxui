/*
	-- VXUI Motion UI --

	Copyright 2026 Xi Chen

	Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
	associated documentation files (the "Software"), to deal in the Software without restriction,
	including without limitation the rights to use, copy, modify, merge, publish, distribute,
	sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all copies or substantial
	portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
	NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
	NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES
   OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include <cstdint>
#include <cstddef>
#include <cassert>
#include <cmath>
#include <glm/glm.hpp>

// ============================================== UTILITY ==============================================

#ifndef VXUI_ROUND_UP
	#define VXUI_ROUND_UP(N, S) ((((N) + (S) - 1) / (S)) * (S))
#endif
#ifndef VXUI_ROUND_DOWN
	#define VXUI_ROUND_DOWN(N, S) ((N) & ~((S) - 1))
#endif

constexpr uint32_t vxui_hash( const char* s )
{
    uint32_t h = 5381;
    while ( *s ) h = ( ( h << 5 ) + h ) ^ ( uint8_t ) *s++;
    return h;
}

struct vxui_alloc_state
{
    uint8_t* base = nullptr;
    uint64_t size = 0;
    uint64_t offset = 0;
};

static inline vxui_alloc_state vxui_alloc_create( uint64_t size, void* memory )
{
    assert( memory );
    vxui_alloc_state out = { 0 };
    out.base = ( uint8_t* ) memory;
    out.size = size;
    out.offset = 0;
    return out;
}

static inline void* vxui_alloc( vxui_alloc_state* a, uint64_t size, uint64_t align )
{
    assert( a && a->base );
    if ( !size ) return nullptr;
    
    uint64_t nptr = ( uint64_t ) a->base + a->offset;
    nptr = VXUI_ROUND_UP( nptr, align );
    nptr -= ( uint64_t ) a->base;

    if ( nptr + size > a->size )
    {
        // Can't allocate, arena is full.
        return nullptr;
    }
    
    a->offset = nptr + size;
    return ( void* ) ( a->base + nptr );
}

static inline void vxui_alloc_reset( vxui_alloc_state* a )
{
    assert( a );
    a->offset = 0;
}

// ============================================== SPRING ===============================================

// s = { value, velocity }, xt = target, freq = omega (higher = snappier)
static inline void vxui_spring_update( glm::vec2& s, float xt, float freq = 12.0f, float dT = 1.0f / 60.0f )
{
    // Exact analytical solution for critically damped spring (zeta = 1)(Juckett formulation):
    //      x(t) = xt + (A + B*t) * e^(-omega*t)
    //      v(t) = (B - omega*(A + B*t)) * e^(-omega*t)
    //          where A = x0 - xt (displacement), B = v0 + omega*A

    const float e  = glm::exp( -freq * dT );
    const float A  = s.x - xt;
    const float B  = s.y + freq * A;

    s.x = xt + ( A + B * dT ) * e;
    s.y = ( B - freq * ( A + B * dT ) ) * e;
}
