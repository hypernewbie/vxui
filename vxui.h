
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

#include <cfloat>
#include <cstdint>
#include <glm/glm.hpp>

#define VXUI_MAX_MENUS         32
#define VXUI_MAX_MENU_ROWS     32
#define VXUI_ROW_HEIGHT        32
#define VXUI_FONT_SIZE_DEFAULT ( VXUI_ROW_HEIGHT * 3 / 4 )
#define VXUI_INPUT_DELAY       0.4f
#define VXUI_INPUT_REPEAT      0.1f

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

#define VXUI_MOUSE_LEFT  0x01
#define VXUI_MOUSE_RIGHT 0x02

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

#define VXUI_MAX_DRAW_CMDS  512
#define VXUI_MAX_TEXT_BYTES 8192

#define VXUI_DRAW_RECT 0
#define VXUI_DRAW_TEXT 1

#define VXUI_DRAW_FOCUSED  0x01
#define VXUI_DRAW_PRESSED  0x02
#define VXUI_DRAW_DISABLED 0x04
#define VXUI_DRAW_HOVERED  0x08

struct vxui_render_data
{
    uint32_t  material_id;
    uint32_t  texture_id;
    uint32_t  flags;
    float     outline_thickness;

    glm::vec4 colour;
    glm::vec4 outline_colour;
    glm::vec4 uv;
    float     params[ 8 ];
};

struct vxui_draw_cmd
{
    uint32_t         id;
    uint8_t          type;            // VXUI_DRAW_RECT | VXUI_DRAW_TEXT
    uint8_t          state;           // VXUI_DRAW_FOCUSED | _PRESSED | _DISABLED | _HOVERED
    glm::vec4        rect;            // x, y, w, h
    float            focus_offset_y;  // VXUI-owned spring; nonzero only on FOCUSED RECT cmds
    const char*      text;            // null when type != TEXT; valid until next vxui_frame
    int32_t          text_len;
    uint16_t         font;
    uint16_t         font_px;
    vxui_render_data render;
};

struct vxui_draw_list
{
    vxui_draw_cmd* cmds  = nullptr;
    int            count = 0;
};

typedef void ( *vxui_render_data_fn )( const vxui_draw_cmd* cmd, vxui_render_data* out, void* userdata );

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
    void*    text             = nullptr;// vxui_text_state*, lazy on first vxui_load_font
    void*    renderer         = nullptr;// owned by sibling render module

    glm::vec2 mouse_pos          = { FLT_MAX, FLT_MAX };  // off-screen sentinel = no mouse
    uint32_t  mouse_buttons      = 0;    // bitfield of VXUI_MOUSE_*, cleared each vxui_frame
    uint32_t  prev_mouse_buttons = 0;    // mouse_buttons from the previous frame
    uint32_t  mouse_press_row_id = 0;    // RECT id pressed under cursor, persists until release
    uint32_t  mouse_click_row_id = 0;    // RECT id activated by button-up over same rect, one-frame carry

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

    uint32_t focused_row_ids[VXUI_MAX_MENUS] = {};   // per-frame, count = focused_row_count
    float    focus_offsets  [VXUI_MAX_MENUS] = {};   // parallel to focused_row_ids
    int      focused_row_count               = 0;
    uint32_t pressed_row_ids[VXUI_MAX_MENUS] = {};   // per-frame, count = pressed_row_count
    int      pressed_row_count               = 0;

    vxui_draw_cmd  draw_buf[VXUI_MAX_DRAW_CMDS] = {};
    vxui_draw_list draw_list                     = {};

    char text_buf[VXUI_MAX_TEXT_BYTES] = {};  // per-frame label storage; reset in vxui_frame
    int  text_offset                    = 0;

    vxui_render_data_fn render_data_fn       = nullptr;
    void*               render_data_userdata = nullptr;
};

#define VXUI_FONT_INVALID 0xffff
typedef uint16_t vxui_font_id;

void           vxui_init    ( vxui_ctx* ctx, float w, float h, void* clay_memory, size_t clay_size );
void           vxui_shutdown( vxui_ctx* ctx );
vxui_font_id   vxui_load_font( vxui_ctx* ctx, const void* data, size_t size, float size_px );
void           vxui_frame   ( vxui_ctx* ctx, float dt, float w = 0, float h = 0 );
vxui_draw_list vxui_render  ( vxui_ctx* ctx );
void           vxui_div     ( vxui_ctx* ctx, const char* id, vxui_div_cfg cfg = {} );
void           vxui_div_end ( vxui_ctx* ctx );
void           vxui_rect    ( vxui_ctx* ctx, const char* id, vxui_div_cfg cfg = {} );
void           vxui_root    ( vxui_ctx* ctx, const char* id, float x, float y );
void           vxui_root_end( vxui_ctx* ctx );

bool vxui_page             ( vxui_ctx* ctx, const char* name );
void vxui_switch           ( vxui_ctx* ctx, const char* name );
void vxui_input            ( vxui_ctx* ctx, const char* action, bool pressed = true );
bool vxui_input_pressed    ( vxui_ctx* ctx, const char* action );
bool vxui_input_just_pressed( vxui_ctx* ctx, const char* action );
bool vxui_input_repeated    ( vxui_ctx* ctx, const char* action );
void vxui_mouse            ( vxui_ctx* ctx, float x, float y, uint32_t buttons );

bool vxui_menu           ( vxui_ctx* ctx, const char* id, bool wrap = true, int max_visible = 0 );

int                  vxui_draw_count ( const vxui_draw_list& dl, uint8_t type );
const vxui_draw_cmd* vxui_draw_nth   ( const vxui_draw_list& dl, uint8_t type, int n );
const vxui_draw_cmd* vxui_draw_find  ( const vxui_draw_list& dl, uint8_t type, uint32_t id );
void vxui_set_render_data_fn( vxui_ctx* ctx, vxui_render_data_fn fn, void* userdata );
bool vxui_menu_action    ( vxui_ctx* ctx, const char* label );
bool vxui_menu_option    ( vxui_ctx* ctx, const char* label, int* index, const char** options, int count );
bool vxui_menu_slider    ( vxui_ctx* ctx, const char* label, float* value, float mn = 0.0f, float mx = 1.0f, float step = 0.1f );
void vxui_menu_section   ( vxui_ctx* ctx, const char* title );
void vxui_menu_label     ( vxui_ctx* ctx, const char* text );
bool vxui_menu_cancelled ( vxui_ctx* ctx );
void vxui_menu_end       ( vxui_ctx* ctx );
