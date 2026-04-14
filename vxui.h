
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
#include <glm/glm.hpp>

#define VXUI_MAX_MENUS 32

// ============================================== API ==================================================

enum vxui_input_action : uint32_t
{
    VXUI_INPUT_UP      = 1 << 0,
    VXUI_INPUT_DOWN    = 1 << 1,
    VXUI_INPUT_LEFT    = 1 << 2,
    VXUI_INPUT_RIGHT   = 1 << 3,
    VXUI_INPUT_CONFIRM = 1 << 4,
    VXUI_INPUT_CANCEL  = 1 << 5,
};

struct vxui_ctx
{
    uint32_t active_page = 0;
    uint32_t input       = 0;   // bitfield of vxui_input_action

    glm::uvec3 menu_state[VXUI_MAX_MENUS] = {}; // { hash_id, current_row, num_rows }
    int menu_count  = 0;
    int active_menu = -1;    // index into menus[], -1 = none
    int active_menu_row = 0; // current row being declared
};

bool vxui_page           ( vxui_ctx* ctx, const char* name );
void vxui_switch         ( vxui_ctx* ctx, const char* name );
void vxui_input          ( vxui_ctx* ctx, const char* action, bool pressed = true );
bool vxui_input_pressed  ( vxui_ctx* ctx, const char* action );

bool vxui_menu           ( vxui_ctx* ctx, const char* id, bool wrap = true );
bool vxui_menu_action    ( vxui_ctx* ctx, const char* label );
bool vxui_menu_cancelled ( vxui_ctx* ctx );
void vxui_menu_end       ( vxui_ctx* ctx );
