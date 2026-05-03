
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

#define VXUI_MAX_MENUS      32
#define VXUI_MAX_MENU_ROWS  32
#define VXUI_ROW_HEIGHT     32
#define VXUI_INPUT_DELAY    0.4f
#define VXUI_INPUT_REPEAT   0.1f

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

#define VXUI_FIT     0
#define VXUI_GROW    1
#define VXUI_FIXED   2
#define VXUI_PERCENT 3

struct vxui_sizing { uint8_t type; float value; };

struct vxui_div_cfg
{
    vxui_sizing  width      = {};
    vxui_sizing  height     = {};
    bool         col        = false;
    uint16_t     padding[4] = {};
    uint16_t     gap        = 0;
    uint8_t      align_x    = 0;   // 0=start  1=center  2=end
    uint8_t      align_y    = 0;
};

#define VXUI_MAX_DRAW_CMDS 512

#define VXUI_DRAW_RECT 0
#define VXUI_DRAW_TEXT 1

struct vxui_draw_cmd
{
    uint32_t    id;
    uint8_t     type;       // VXUI_DRAW_RECT | VXUI_DRAW_TEXT
    glm::vec4   rect;       // x, y, w, h
    const char* text;       // null when type != TEXT; valid until next vxui_frame
    int32_t     text_len;
    uint16_t    font;
    uint16_t    font_px;
};

struct vxui_draw_list
{
    vxui_draw_cmd* cmds  = nullptr;
    int            count = 0;
};

struct vxui_ctx
{
    uint32_t active_page      = 0;
    uint32_t input            = 0;    // bitfield of vxui_input_action, cleared each vxui_frame
    uint32_t prev_input       = 0;    // input bitfield from the previous frame
    uint32_t input_repeated   = 0;    // bits firing this frame (just-pressed | repeat)
    uint32_t menu_active_mask = 0;    // bit i set if menu i was active this frame
    uint32_t prev_active_mask = 0;    // menu_active_mask from the previous frame
    bool     frame_active     = false;// true between vxui_frame and vxui_render
    bool     inputs_committed = false;// input_repeated computed this frame
    float    input_held_time  [6] = {};// seconds each input has been held
    float    input_next_fire  [6] = {};// next held_time threshold to fire repeat
    float    dt               = 0;
    void*    clay             = nullptr;

    glm::uvec4  menu_state       [VXUI_MAX_MENUS] = {}; // { hash_id, current_row, num_rows, skip_mask }
    glm::vec4   menu_focus_spring[VXUI_MAX_MENUS] = {}; // { offset_y, velocity_y, prev_row, _ }, prev_row -1 = unset
    int         menu_scroll_top  [VXUI_MAX_MENUS] = {}; // topmost visible row index when max_visible > 0
    const char* menu_names       [VXUI_MAX_MENUS] = {}; // literal pointer for hash-collision detection
    uint32_t active_menu_row_ids[VXUI_MAX_MENU_ROWS] = {}; // Clay ids of rows declared this frame, for dup detection
    int      menu_count           = 0;
    int      active_menu          = -1;  // index into menus[], -1 = none
    int      active_menu_row      = 0;   // current row being declared
    uint32_t active_menu_skip     = 0;   // bitmask, accumulated during declaration
    uint32_t active_menu_focus_id = 0;   // Clay id of focused row this frame, 0 = none

    vxui_draw_cmd  draw_buf[VXUI_MAX_DRAW_CMDS] = {};
    vxui_draw_list draw_list                     = {};
};

void           vxui_init    ( vxui_ctx* ctx, float w, float h, void* clay_memory, size_t clay_size );
void           vxui_frame   ( vxui_ctx* ctx, float dt, float w = 0, float h = 0 );
vxui_draw_list vxui_render  ( vxui_ctx* ctx );
void           vxui_div     ( vxui_ctx* ctx, const char* id, vxui_div_cfg cfg = {} );
void           vxui_div_end ( vxui_ctx* ctx );
void           vxui_root    ( vxui_ctx* ctx, const char* id, float x, float y );
void           vxui_root_end( vxui_ctx* ctx );

bool vxui_page             ( vxui_ctx* ctx, const char* name );
void vxui_switch           ( vxui_ctx* ctx, const char* name );
void vxui_input            ( vxui_ctx* ctx, const char* action, bool pressed = true );
bool vxui_input_pressed    ( vxui_ctx* ctx, const char* action );
bool vxui_input_just_pressed( vxui_ctx* ctx, const char* action );
bool vxui_input_repeated    ( vxui_ctx* ctx, const char* action );

bool vxui_menu           ( vxui_ctx* ctx, const char* id, bool wrap = true, int max_visible = 0, bool auto_repeat = false );

int                  vxui_draw_count ( const vxui_draw_list& dl, uint8_t type );
const vxui_draw_cmd* vxui_draw_nth   ( const vxui_draw_list& dl, uint8_t type, int n );
const vxui_draw_cmd* vxui_draw_find  ( const vxui_draw_list& dl, uint8_t type, uint32_t id );
bool vxui_menu_action    ( vxui_ctx* ctx, const char* label );
bool vxui_menu_option    ( vxui_ctx* ctx, const char* label, int* index, const char** options, int count );
bool vxui_menu_slider    ( vxui_ctx* ctx, const char* label, float* value, float mn = 0.0f, float mx = 1.0f, float step = 0.1f );
void vxui_menu_section   ( vxui_ctx* ctx, const char* title );
void vxui_menu_label     ( vxui_ctx* ctx, const char* text );
bool vxui_menu_cancelled ( vxui_ctx* ctx );
void vxui_menu_end       ( vxui_ctx* ctx );
