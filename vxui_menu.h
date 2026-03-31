#pragma once

#include "vxui.h"

#define VXUI_MENU_STYLE_FOOTER_PROMPT_BAR ( 1u << 0 )
#define VXUI_MENU_STYLE_SECONDARY_TEXT ( 1u << 1 )
#define VXUI_MENU_STYLE_AUTO_SCROLL_TO_FOCUS ( 1u << 2 )
#define VXUI_MENU_STYLE_VALUE_ARROWS ( 1u << 3 )
#define VXUI_MENU_STYLE_FOCUS_FRAME ( 1u << 4 )

typedef enum vxui_menu_layout_mode
{
    VXUI_MENU_LAYOUT_FORM = 0,
    VXUI_MENU_LAYOUT_CENTERED = 1,
} vxui_menu_layout_mode;

typedef enum vxui_menu_value_lane_mode
{
    VXUI_MENU_VALUE_LANE_GROW = 0,
    VXUI_MENU_VALUE_LANE_FIXED = 1,
    VXUI_MENU_VALUE_LANE_FIT = 2,
} vxui_menu_value_lane_mode;

typedef enum vxui_menu_wrap_mode
{
    VXUI_MENU_WRAP_DEFAULT = 0,
    VXUI_MENU_WRAP_FORCE_OFF = 1,
    VXUI_MENU_WRAP_FORCE_ON = 2,
} vxui_menu_wrap_mode;

typedef enum vxui_menu_focus_decor
{
    VXUI_MENU_FOCUS_DECOR_NONE = 0,
    VXUI_MENU_FOCUS_DECOR_GLOW = 1,
    VXUI_MENU_FOCUS_DECOR_PULSE = 2,
} vxui_menu_focus_decor;

typedef enum vxui_menu_shell_kind
{
    VXUI_MENU_SHELL_TITLE_MENU = 0,
    VXUI_MENU_SHELL_SPLIT_DECK = 1,
    VXUI_MENU_SHELL_FORM = 2,
} vxui_menu_shell_kind;

typedef enum vxui_menu_shell_compact_mode
{
    VXUI_MENU_SHELL_COMPACT_AUTO = 0,
    VXUI_MENU_SHELL_COMPACT_FORCE_OFF = 1,
    VXUI_MENU_SHELL_COMPACT_FORCE_ON = 2,
} vxui_menu_shell_compact_mode;

typedef enum vxui_menu_status_importance
{
    VXUI_MENU_STATUS_PRIMARY = 0,
    VXUI_MENU_STATUS_SECONDARY = 1,
    VXUI_MENU_STATUS_OPTIONAL = 2,
} vxui_menu_status_importance;

typedef struct vxui_menu_state
{
    float scroll_current;
    float scroll_target;
    float scroll_velocity;
    uint32_t last_focused_row_id;
    int last_emitted_row_count;
    int selected_section_index;
} vxui_menu_state;

typedef struct vxui_menu_badge_cfg
{
    uint32_t font_id;
    float font_size;
    float padding_x;
    float padding_y;
    float corner_radius;
    float min_width;
    vxui_color fill_color;
    vxui_color text_color;
    bool hidden;
} vxui_menu_badge_cfg;

typedef struct vxui_menu_row_cfg
{
    const char* value_key;
    const char* secondary_key;
    const char* badge_text_key;
    vxui_menu_badge_cfg badge;
    uint32_t font_id;
    float font_size;
    vxui_color text_color;
    vxui_menu_wrap_mode wrap_mode;
    bool hidden;
    bool disabled;
} vxui_menu_row_cfg;

typedef struct vxui_menu_section_cfg
{
    const char* secondary_key;
    uint32_t font_id;
    float font_size;
    vxui_color text_color;
    bool hidden;
    bool centered;
} vxui_menu_section_cfg;

typedef struct vxui_menu_prompt_bar_cfg
{
    const char* const* action_names;
    const char* const* label_keys;
    int count;
    float gap;
    bool hidden;
} vxui_menu_prompt_bar_cfg;

typedef struct vxui_menu_header_cfg
{
    const char* title_key;
    const char* subtitle_key;
    bool hidden;
} vxui_menu_header_cfg;

typedef struct vxui_menu_help_cfg
{
    const char* title_key;
    const char* const* line_keys;
    int line_count;
    int compact_line_count;
    bool hidden;
} vxui_menu_help_cfg;

typedef struct vxui_menu_preview_cfg
{
    const char* section_key;
    const char* title_key;
    const char* subtitle_key;
    const char* badge_text_key;
    const char* body_key;
    const char* compact_body_key;
    int body_max_lines;
    int compact_body_max_lines;
    const vxui_menu_help_cfg* help;
    bool hidden;
    const char* header_id;
    const char* body_id;
} vxui_menu_preview_cfg;

typedef struct vxui_menu_prompt_item
{
    const char* action_name;
    const char* label_key;
    bool hidden;
    const char* id;
} vxui_menu_prompt_item;

typedef struct vxui_menu_status_item
{
    const char* label_key;
    const char* value_key;
    vxui_menu_status_importance importance;
    bool collapse_in_compact;
    bool hidden;
    const char* id;
} vxui_menu_status_item;

typedef struct vxui_menu_footer_cfg
{
    const vxui_menu_prompt_item* prompt_items;
    int prompt_item_count;
    const vxui_menu_status_item* status_items;
    int status_item_count;
    vxui_menu_shell_compact_mode compact_mode;
    int compact_max_status_items;
    bool hidden;
} vxui_menu_footer_cfg;

typedef struct vxui_menu_lane_cfg
{
    float width;
    float min_width;
    bool grow;
    bool weak;
    bool hidden;
} vxui_menu_lane_cfg;

typedef struct vxui_menu_style
{
    uint32_t flags;
    vxui_menu_layout_mode layout_mode;
    vxui_menu_value_lane_mode value_lane_mode;
    float row_height;
    float row_gap;
    float section_gap;
    float label_lane_width;
    float value_lane_width;
    float padding_x;
    float padding_y;
    float lane_gap;
    float secondary_gap;
    float corner_radius;
    float border_width;
    float badge_padding_x;
    float badge_padding_y;
    float prompt_gap;
    float scroll_stiffness;
    float scroll_damping;
    bool option_wrap_by_default;
    uint32_t body_font_id;
    uint32_t title_font_id;
    uint32_t badge_font_id;
    float body_font_size;
    float secondary_font_size;
    float title_font_size;
    float badge_font_size;
    vxui_menu_focus_decor focus_decor;
    float focus_decor_padding;
    float focus_decor_alpha;
    vxui_color panel_fill_color;
    vxui_color row_fill_color;
    vxui_color row_focus_fill_color;
    vxui_color row_disabled_fill_color;
    vxui_color row_border_color;
    vxui_color row_focus_border_color;
    vxui_color text_color;
    vxui_color focused_text_color;
    vxui_color disabled_text_color;
    vxui_color secondary_text_color;
    vxui_color section_text_color;
    vxui_color badge_fill_color;
    vxui_color badge_text_color;
    vxui_color prompt_fill_color;
    vxui_color prompt_text_color;
    vxui_color arrow_color;
    vxui_color slider_track_color;
    vxui_color slider_fill_color;
} vxui_menu_style;

typedef struct vxui_menu_cfg
{
    const vxui_menu_style* style;
    const char* title_key;
    const char* subtitle_key;
    float viewport_height;
} vxui_menu_cfg;

typedef struct vxui_menu_screen_cfg
{
    vxui_menu_shell_kind shell_kind;
    const vxui_menu_style* style;
    vxui_menu_shell_compact_mode compact_mode;
    float compact_height_threshold;
    float compact_width_threshold;
    bool tertiary_enabled;
    vxui_menu_header_cfg header;
    vxui_menu_lane_cfg primary_lane;
    vxui_menu_lane_cfg secondary_lane;
    vxui_menu_lane_cfg tertiary_lane;
    vxui_menu_preview_cfg preview;
    vxui_menu_footer_cfg footer;
} vxui_menu_screen_cfg;

typedef struct vxui_menu_surface_cfg
{
    float surface_width;
    float surface_max_height;
    float outer_padding;
    float surface_padding_x;
    float surface_padding_y;
    float section_gap;
    float corner_radius;
    float border_width;
    vxui_color background_fill_color;
    vxui_color surface_fill_color;
    vxui_color surface_border_color;
} vxui_menu_surface_cfg;

vxui_menu_surface_cfg vxui_menu_surface_cfg_default(
    float surface_width,
    float surface_max_height,
    vxui_color background_fill_color,
    vxui_color surface_fill_color,
    vxui_color surface_border_color );

vxui_menu_style vxui_menu_style_br_title( void );
vxui_menu_style vxui_menu_style_br_panel( void );
vxui_menu_style vxui_menu_style_form( void );
vxui_menu_style vxui_menu_style_compact( void );
vxui_menu_style vxui_menu_style_title_menu( void );
vxui_menu_style vxui_menu_style_split_deck( void );
vxui_menu_style vxui_menu_style_form_shell( void );
vxui_menu_style vxui_menu_style_footer_strip( void );

void vxui_menu_begin( vxui_ctx* ctx, vxui_menu_state* state, const char* id, vxui_menu_cfg cfg );
void vxui_menu_end( vxui_ctx* ctx, vxui_menu_state* state );
void vxui_menu_section( vxui_ctx* ctx, vxui_menu_state* state, const char* id, const char* title_key, vxui_menu_section_cfg cfg );
void vxui_menu_label( vxui_ctx* ctx, vxui_menu_state* state, const char* id, const char* label_key, vxui_menu_row_cfg cfg );
void vxui_menu_action( vxui_ctx* ctx, vxui_menu_state* state, const char* id, const char* label_key, vxui_action_fn fn, vxui_menu_row_cfg row_cfg, vxui_action_cfg action_cfg );
void vxui_menu_option( vxui_ctx* ctx, vxui_menu_state* state, const char* id, const char* label_key, int* index, const char** option_keys, int count, vxui_menu_row_cfg row_cfg, vxui_option_cfg option_cfg );
void vxui_menu_slider( vxui_ctx* ctx, vxui_menu_state* state, const char* id, const char* label_key, float* value, float min_value, float max_value, vxui_menu_row_cfg row_cfg, vxui_slider_cfg slider_cfg );
void vxui_menu_badge( vxui_ctx* ctx, vxui_menu_state* state, const char* id, const char* text_key, vxui_menu_badge_cfg cfg );
void vxui_menu_prompt_bar( vxui_ctx* ctx, vxui_menu_state* state, const char* id, const vxui_menu_prompt_bar_cfg* cfg );
void vxui_menu_surface_begin( vxui_ctx* ctx, const char* root_id, const char* surface_id, const vxui_menu_surface_cfg* cfg );
void vxui_menu_surface_end( vxui_ctx* ctx );
void vxui_menu_screen_begin( vxui_ctx* ctx, vxui_menu_state* state, const char* id, const vxui_menu_screen_cfg* cfg );
void vxui_menu_screen_end( vxui_ctx* ctx, vxui_menu_state* state );
void vxui_menu_header( vxui_ctx* ctx, const char* id, const vxui_menu_header_cfg* cfg );
void vxui_menu_primary_lane_begin( vxui_ctx* ctx, const char* id, const vxui_menu_lane_cfg* cfg );
void vxui_menu_primary_lane_end( vxui_ctx* ctx );
void vxui_menu_secondary_lane_begin( vxui_ctx* ctx, const char* id, const vxui_menu_lane_cfg* cfg );
void vxui_menu_secondary_lane_end( vxui_ctx* ctx );
void vxui_menu_tertiary_lane_begin( vxui_ctx* ctx, const char* id, const vxui_menu_lane_cfg* cfg );
void vxui_menu_tertiary_lane_end( vxui_ctx* ctx );
void vxui_menu_preview( vxui_ctx* ctx, const char* id, const vxui_menu_preview_cfg* cfg );
void vxui_menu_help_legend( vxui_ctx* ctx, const char* id, const vxui_menu_help_cfg* cfg );
void vxui_menu_footer( vxui_ctx* ctx, const char* id, const vxui_menu_footer_cfg* cfg );
bool vxui_menu_screen_is_compact( vxui_ctx* ctx );

#ifdef VXUI_MENU_IMPL

#ifndef VXUI_IMPL
#error "VXUI_MENU_IMPL requires VXUI_IMPL in the same translation unit."
#endif

#include <cmath>
#include <cstdio>
#include <cstring>

enum
{
    VXUI_MENU__MAX_SCOPE_DEPTH = 8,
    VXUI_MENU__MAX_SCREEN_SCOPE_DEPTH = 4,
    VXUI_MENU__MAX_SURFACE_SCOPE_DEPTH = 4,
    VXUI_MENU__MAX_FOOTER_STATUS_ITEMS = 16,
};

typedef struct vxui_menu__scope
{
    vxui_ctx* ctx;
    vxui_menu_state* state;
    const char* id;
    vxui_menu_style style;
    vxui_menu_cfg cfg;
    uint32_t viewport_id;
    uint32_t content_id;
    uint32_t focused_row_id;
    int row_count;
    int focused_row_index;
} vxui_menu__scope;

typedef enum vxui_menu__lane_role
{
    VXUI_MENU__LANE_ROLE_NONE = 0,
    VXUI_MENU__LANE_ROLE_PRIMARY = 1,
    VXUI_MENU__LANE_ROLE_SECONDARY = 2,
    VXUI_MENU__LANE_ROLE_TERTIARY = 3,
} vxui_menu__lane_role;

typedef struct vxui_menu__screen_scope
{
    vxui_ctx* ctx;
    vxui_menu_state* state;
    const char* id;
    vxui_menu_style style;
    vxui_menu_screen_cfg cfg;
    bool compact;
    bool body_open;
    bool lane_open;
    vxui_menu__lane_role active_lane;
} vxui_menu__screen_scope;

typedef struct vxui_menu__surface_scope
{
    vxui_ctx* ctx;
    const char* root_id;
    const char* surface_id;
} vxui_menu__surface_scope;

static thread_local vxui_menu__scope vxui_menu__scopes[ VXUI_MENU__MAX_SCOPE_DEPTH ];
static thread_local int vxui_menu__scope_count = 0;
static thread_local vxui_menu__screen_scope vxui_menu__screen_scopes[ VXUI_MENU__MAX_SCREEN_SCOPE_DEPTH ];
static thread_local int vxui_menu__screen_scope_count = 0;
static thread_local vxui_menu__surface_scope vxui_menu__surface_scopes[ VXUI_MENU__MAX_SURFACE_SCOPE_DEPTH ];
static thread_local int vxui_menu__surface_scope_count = 0;

static Clay_Color vxui_menu__to_clay_color( vxui_color color )
{
    return Clay_Color { ( float ) color.r, ( float ) color.g, ( float ) color.b, ( float ) color.a };
}

static uint16_t vxui_menu__u16( float value )
{
    if ( value <= 0.0f ) {
        return 0;
    }
    if ( value >= 65535.0f ) {
        return 65535u;
    }
    return ( uint16_t ) std::lround( ( double ) value );
}

static uint32_t vxui_menu__mix_id( uint32_t base, uint32_t salt )
{
    uint32_t value = base ^ ( salt * 0x9E3779B9u ) ^ 0x7F4A7C15u;
    if ( value == 0 ) {
        value = salt ? salt : 1u;
    }
    return value;
}

static vxui_menu__scope* vxui_menu__current_scope( void )
{
    if ( vxui_menu__scope_count <= 0 ) {
        return nullptr;
    }
    return &vxui_menu__scopes[ vxui_menu__scope_count - 1 ];
}

static vxui_menu__screen_scope* vxui_menu__current_screen_scope( void )
{
    if ( vxui_menu__screen_scope_count <= 0 ) {
        return nullptr;
    }
    return &vxui_menu__screen_scopes[ vxui_menu__screen_scope_count - 1 ];
}

static const char* vxui_menu__push_child_id( vxui_ctx* ctx, const char* base, const char* suffix )
{
    if ( !ctx ) {
        return "";
    }

    char buffer[ 256 ] = {};
    int wrote = std::snprintf( buffer, sizeof( buffer ), "%s.%s", base ? base : "", suffix ? suffix : "" );
    if ( wrote < 0 ) {
        return vxui__push_frame_string( ctx, "", 0 );
    }
    size_t length = ( size_t ) wrote;
    if ( length >= sizeof( buffer ) ) {
        length = sizeof( buffer ) - 1u;
    }
    return vxui__push_frame_string( ctx, buffer, length );
}

static const char* vxui_menu__push_child_index_id( vxui_ctx* ctx, const char* base, const char* suffix, int index )
{
    if ( !ctx ) {
        return "";
    }

    char buffer[ 256 ] = {};
    int wrote = std::snprintf( buffer, sizeof( buffer ), "%s.%s.%d", base ? base : "", suffix ? suffix : "", index );
    if ( wrote < 0 ) {
        return vxui__push_frame_string( ctx, "", 0 );
    }
    size_t length = ( size_t ) wrote;
    if ( length >= sizeof( buffer ) ) {
        length = sizeof( buffer ) - 1u;
    }
    return vxui__push_frame_string( ctx, buffer, length );
}

static vxui_menu_surface_cfg vxui_menu__sanitize_surface_cfg( vxui_menu_surface_cfg cfg )
{
    if ( cfg.surface_width < 0.0f ) {
        cfg.surface_width = 0.0f;
    }
    if ( cfg.surface_max_height < 0.0f ) {
        cfg.surface_max_height = 0.0f;
    }
    if ( cfg.outer_padding < 0.0f ) {
        cfg.outer_padding = 0.0f;
    }
    if ( cfg.surface_padding_x < 0.0f ) {
        cfg.surface_padding_x = 0.0f;
    }
    if ( cfg.surface_padding_y < 0.0f ) {
        cfg.surface_padding_y = 0.0f;
    }
    if ( cfg.section_gap < 0.0f ) {
        cfg.section_gap = 0.0f;
    }
    if ( cfg.corner_radius < 0.0f ) {
        cfg.corner_radius = 0.0f;
    }
    if ( cfg.border_width < 0.0f ) {
        cfg.border_width = 0.0f;
    }
    return cfg;
}

vxui_menu_surface_cfg vxui_menu_surface_cfg_default(
    float surface_width,
    float surface_max_height,
    vxui_color background_fill_color,
    vxui_color surface_fill_color,
    vxui_color surface_border_color )
{
    return ( vxui_menu_surface_cfg ) {
        surface_width,
        surface_max_height,
        16.0f,
        24.0f,
        18.0f,
        14.0f,
        18.0f,
        1.0f,
        background_fill_color,
        surface_fill_color,
        surface_border_color,
    };
}

static vxui_menu_style vxui_menu__sanitize_style( vxui_menu_style style )
{
    vxui_menu_style defaults = vxui_menu_style_form();

    if ( style.flags == 0 ) {
        style.flags = defaults.flags;
    }
    if ( style.row_height <= 0.0f ) {
        style.row_height = defaults.row_height;
    }
    if ( style.row_gap <= 0.0f ) {
        style.row_gap = defaults.row_gap;
    }
    if ( style.section_gap <= 0.0f ) {
        style.section_gap = defaults.section_gap;
    }
    if ( style.label_lane_width <= 0.0f && style.layout_mode == VXUI_MENU_LAYOUT_FORM ) {
        style.label_lane_width = defaults.label_lane_width;
    }
    if ( style.value_lane_width <= 0.0f && style.value_lane_mode == VXUI_MENU_VALUE_LANE_FIXED ) {
        style.value_lane_width = defaults.value_lane_width;
    }
    if ( style.padding_x <= 0.0f ) {
        style.padding_x = defaults.padding_x;
    }
    if ( style.padding_y <= 0.0f ) {
        style.padding_y = defaults.padding_y;
    }
    if ( style.lane_gap <= 0.0f ) {
        style.lane_gap = defaults.lane_gap;
    }
    if ( style.secondary_gap <= 0.0f ) {
        style.secondary_gap = defaults.secondary_gap;
    }
    if ( style.corner_radius <= 0.0f ) {
        style.corner_radius = defaults.corner_radius;
    }
    if ( style.border_width <= 0.0f ) {
        style.border_width = defaults.border_width;
    }
    if ( style.badge_padding_x <= 0.0f ) {
        style.badge_padding_x = defaults.badge_padding_x;
    }
    if ( style.badge_padding_y <= 0.0f ) {
        style.badge_padding_y = defaults.badge_padding_y;
    }
    if ( style.prompt_gap <= 0.0f ) {
        style.prompt_gap = defaults.prompt_gap;
    }
    if ( style.scroll_stiffness <= 0.0f ) {
        style.scroll_stiffness = defaults.scroll_stiffness;
    }
    if ( style.scroll_damping <= 0.0f ) {
        style.scroll_damping = defaults.scroll_damping;
    }
    if ( style.body_font_size <= 0.0f ) {
        style.body_font_size = defaults.body_font_size;
    }
    if ( style.secondary_font_size <= 0.0f ) {
        style.secondary_font_size = defaults.secondary_font_size;
    }
    if ( style.title_font_size <= 0.0f ) {
        style.title_font_size = defaults.title_font_size;
    }
    if ( style.badge_font_size <= 0.0f ) {
        style.badge_font_size = defaults.badge_font_size;
    }
    if ( style.focus_decor_padding <= 0.0f ) {
        style.focus_decor_padding = defaults.focus_decor_padding;
    }
    if ( style.focus_decor_alpha <= 0.0f ) {
        style.focus_decor_alpha = defaults.focus_decor_alpha;
    }

    if ( style.panel_fill_color.a == 0 ) style.panel_fill_color = defaults.panel_fill_color;
    if ( style.row_fill_color.a == 0 ) style.row_fill_color = defaults.row_fill_color;
    if ( style.row_focus_fill_color.a == 0 ) style.row_focus_fill_color = defaults.row_focus_fill_color;
    if ( style.row_disabled_fill_color.a == 0 ) style.row_disabled_fill_color = defaults.row_disabled_fill_color;
    if ( style.row_border_color.a == 0 ) style.row_border_color = defaults.row_border_color;
    if ( style.row_focus_border_color.a == 0 ) style.row_focus_border_color = defaults.row_focus_border_color;
    if ( style.text_color.a == 0 ) style.text_color = defaults.text_color;
    if ( style.focused_text_color.a == 0 ) style.focused_text_color = defaults.focused_text_color;
    if ( style.disabled_text_color.a == 0 ) style.disabled_text_color = defaults.disabled_text_color;
    if ( style.secondary_text_color.a == 0 ) style.secondary_text_color = defaults.secondary_text_color;
    if ( style.section_text_color.a == 0 ) style.section_text_color = defaults.section_text_color;
    if ( style.badge_fill_color.a == 0 ) style.badge_fill_color = defaults.badge_fill_color;
    if ( style.badge_text_color.a == 0 ) style.badge_text_color = defaults.badge_text_color;
    if ( style.prompt_fill_color.a == 0 ) style.prompt_fill_color = defaults.prompt_fill_color;
    if ( style.prompt_text_color.a == 0 ) style.prompt_text_color = defaults.prompt_text_color;
    if ( style.arrow_color.a == 0 ) style.arrow_color = defaults.arrow_color;
    if ( style.slider_track_color.a == 0 ) style.slider_track_color = defaults.slider_track_color;
    if ( style.slider_fill_color.a == 0 ) style.slider_fill_color = defaults.slider_fill_color;

    return style;
}

static vxui_color vxui_menu__scale_alpha( vxui_color color, float scale )
{
    int alpha = ( int ) std::lround( ( double ) color.a * ( double ) scale );
    if ( alpha < 0 ) {
        alpha = 0;
    }
    if ( alpha > 255 ) {
        alpha = 255;
    }
    color.a = ( uint8_t ) alpha;
    return color;
}

static vxui_menu_style vxui_menu__default_screen_style( vxui_menu_shell_kind kind )
{
    switch ( kind ) {
        case VXUI_MENU_SHELL_TITLE_MENU:
            return vxui_menu_style_title_menu();

        case VXUI_MENU_SHELL_SPLIT_DECK:
            return vxui_menu_style_split_deck();

        case VXUI_MENU_SHELL_FORM:
        default:
            return vxui_menu_style_form_shell();
    }
}

static float vxui_menu__default_compact_height_threshold( vxui_menu_shell_kind kind )
{
    switch ( kind ) {
        case VXUI_MENU_SHELL_TITLE_MENU:
            return 668.0f;

        case VXUI_MENU_SHELL_SPLIT_DECK:
            return 680.0f;

        case VXUI_MENU_SHELL_FORM:
        default:
            return 700.0f;
    }
}

static float vxui_menu__default_compact_width_threshold( vxui_menu_shell_kind kind )
{
    switch ( kind ) {
        case VXUI_MENU_SHELL_SPLIT_DECK:
            return 1140.0f;

        case VXUI_MENU_SHELL_TITLE_MENU:
            return 1120.0f;

        case VXUI_MENU_SHELL_FORM:
        default:
            return 0.0f;
    }
}

static bool vxui_menu__resolve_screen_compact( vxui_ctx* ctx, const vxui_menu_screen_cfg* cfg )
{
    if ( !ctx ) {
        return false;
    }

    vxui_menu_shell_kind kind = cfg ? cfg->shell_kind : VXUI_MENU_SHELL_FORM;
    vxui_menu_shell_compact_mode mode = cfg ? cfg->compact_mode : VXUI_MENU_SHELL_COMPACT_AUTO;
    if ( mode == VXUI_MENU_SHELL_COMPACT_FORCE_ON ) {
        return true;
    }
    if ( mode == VXUI_MENU_SHELL_COMPACT_FORCE_OFF ) {
        return false;
    }

    float height_threshold = cfg && cfg->compact_height_threshold > 0.0f
        ? cfg->compact_height_threshold
        : vxui_menu__default_compact_height_threshold( kind );
    float width_threshold = cfg && cfg->compact_width_threshold > 0.0f
        ? cfg->compact_width_threshold
        : vxui_menu__default_compact_width_threshold( kind );
    bool height_compact = height_threshold > 0.0f && ctx->cfg.screen_height <= ( int ) std::lround( ( double ) height_threshold );
    bool width_compact = width_threshold > 0.0f && ctx->cfg.screen_width <= ( int ) std::lround( ( double ) width_threshold );
    return height_compact || width_compact;
}

static vxui_menu_lane_cfg vxui_menu__default_lane_cfg( vxui_menu_shell_kind kind, vxui_menu__lane_role role, bool compact )
{
    switch ( kind ) {
        case VXUI_MENU_SHELL_TITLE_MENU:
            switch ( role ) {
                case VXUI_MENU__LANE_ROLE_PRIMARY:
                    return ( vxui_menu_lane_cfg ) { compact ? 320.0f : 360.0f, compact ? 280.0f : 320.0f, false, false, false };

                case VXUI_MENU__LANE_ROLE_SECONDARY:
                    return ( vxui_menu_lane_cfg ) { 0.0f, compact ? 0.0f : 420.0f, true, false, false };

                case VXUI_MENU__LANE_ROLE_TERTIARY:
                default:
                    return ( vxui_menu_lane_cfg ) { 0.0f, 0.0f, false, true, true };
            }

        case VXUI_MENU_SHELL_SPLIT_DECK:
            switch ( role ) {
                case VXUI_MENU__LANE_ROLE_PRIMARY:
                    return ( vxui_menu_lane_cfg ) { compact ? 300.0f : 332.0f, compact ? 280.0f : 300.0f, false, false, false };

                case VXUI_MENU__LANE_ROLE_SECONDARY:
                    return ( vxui_menu_lane_cfg ) { 0.0f, compact ? 360.0f : 420.0f, true, false, false };

                case VXUI_MENU__LANE_ROLE_TERTIARY:
                default:
                    return ( vxui_menu_lane_cfg ) { compact ? 220.0f : 252.0f, compact ? 200.0f : 220.0f, false, true, false };
            }

        case VXUI_MENU_SHELL_FORM:
        default:
            return ( vxui_menu_lane_cfg ) { 0.0f, 0.0f, true, false, role != VXUI_MENU__LANE_ROLE_PRIMARY };
    }
}

static vxui_menu_lane_cfg vxui_menu__resolve_lane_cfg( const vxui_menu__screen_scope* scope, vxui_menu__lane_role role, const vxui_menu_lane_cfg* override_cfg )
{
    vxui_menu_lane_cfg lane = vxui_menu__default_lane_cfg( scope ? scope->cfg.shell_kind : VXUI_MENU_SHELL_FORM, role, scope ? scope->compact : false );
    if ( scope ) {
        switch ( role ) {
            case VXUI_MENU__LANE_ROLE_PRIMARY:
                if ( scope->cfg.primary_lane.width > 0.0f ) lane.width = scope->cfg.primary_lane.width;
                if ( scope->cfg.primary_lane.min_width > 0.0f ) lane.min_width = scope->cfg.primary_lane.min_width;
                lane.grow = scope->cfg.primary_lane.grow || lane.grow;
                lane.weak = scope->cfg.primary_lane.weak;
                lane.hidden = scope->cfg.primary_lane.hidden;
                break;

            case VXUI_MENU__LANE_ROLE_SECONDARY:
                if ( scope->cfg.secondary_lane.width > 0.0f ) lane.width = scope->cfg.secondary_lane.width;
                if ( scope->cfg.secondary_lane.min_width > 0.0f ) lane.min_width = scope->cfg.secondary_lane.min_width;
                lane.grow = scope->cfg.secondary_lane.grow || lane.grow;
                lane.weak = scope->cfg.secondary_lane.weak;
                lane.hidden = scope->cfg.secondary_lane.hidden;
                break;

            case VXUI_MENU__LANE_ROLE_TERTIARY:
                if ( scope->cfg.tertiary_lane.width > 0.0f ) lane.width = scope->cfg.tertiary_lane.width;
                if ( scope->cfg.tertiary_lane.min_width > 0.0f ) lane.min_width = scope->cfg.tertiary_lane.min_width;
                lane.grow = scope->cfg.tertiary_lane.grow || lane.grow;
                lane.weak = scope->cfg.tertiary_lane.weak || lane.weak;
                lane.hidden = scope->cfg.tertiary_lane.hidden || !scope->cfg.tertiary_enabled;
                break;

            case VXUI_MENU__LANE_ROLE_NONE:
            default:
                break;
        }
    }

    if ( override_cfg ) {
        if ( override_cfg->width > 0.0f ) lane.width = override_cfg->width;
        if ( override_cfg->min_width > 0.0f ) lane.min_width = override_cfg->min_width;
        lane.grow = override_cfg->grow || lane.grow;
        lane.weak = override_cfg->weak || lane.weak;
        lane.hidden = override_cfg->hidden;
    }

    return lane;
}

static void vxui_menu__ensure_screen_body_open( vxui_menu__screen_scope* scope )
{
    if ( !scope || scope->body_open ) {
        return;
    }

    vxui_ctx* ctx = scope->ctx;
    float lane_gap = scope->compact ? scope->style.lane_gap * 0.75f : scope->style.lane_gap;
    Clay__OpenElementWithId( vxui__clay_id_from_hash( vxui_id( vxui_menu__push_child_id( ctx, scope->id, "body" ) ) ) );
    Clay__ConfigureOpenElement( ( Clay_ElementDeclaration ) {
        .layout = {
            .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_GROW( 0 ) },
            .childGap = vxui_menu__u16( lane_gap ),
            .childAlignment = { .y = CLAY_ALIGN_Y_TOP },
            .layoutDirection = CLAY_LEFT_TO_RIGHT,
        },
    } );
    scope->body_open = true;
}

static void vxui_menu__close_screen_body( vxui_menu__screen_scope* scope )
{
    if ( !scope ) {
        return;
    }
    if ( scope->lane_open ) {
        Clay__CloseElement();
        scope->lane_open = false;
        scope->active_lane = VXUI_MENU__LANE_ROLE_NONE;
    }
    if ( scope->body_open ) {
        Clay__CloseElement();
        scope->body_open = false;
    }
}

static uint32_t vxui_menu__row_id( const vxui_menu__scope* scope, const char* id )
{
    return vxui_idi( scope && scope->id ? scope->id : "", ( int ) vxui_id( id ? id : "" ) );
}

static bool vxui_menu__visible( const vxui_menu_row_cfg* cfg )
{
    return !cfg || !cfg->hidden;
}

static vxui_color vxui_menu__row_text_color( const vxui_menu_style* style, bool focused, bool disabled, bool secondary )
{
    if ( disabled ) {
        return secondary ? style->secondary_text_color : style->disabled_text_color;
    }
    if ( secondary ) {
        return style->secondary_text_color;
    }
    return focused ? style->focused_text_color : style->text_color;
}

static uint32_t vxui_menu__row_font_id( const vxui_menu_style* style, const vxui_menu_row_cfg* cfg )
{
    if ( cfg && cfg->font_id != 0 ) {
        return cfg->font_id;
    }
    return style->body_font_id;
}

static float vxui_menu__row_font_size( const vxui_menu_style* style, const vxui_menu_row_cfg* cfg )
{
    if ( cfg && cfg->font_size > 0.0f ) {
        return cfg->font_size;
    }
    return style->body_font_size;
}

static void vxui_menu__emit_text_leaf( vxui_ctx* ctx, uint32_t owner_id, const char* text, uint32_t font_id, float font_size, vxui_color color )
{
    const char* resolved = text ? vxui__resolve_text( ctx, text ) : "";
    CLAY_AUTO_ID( vxui__text_leaf_decl( Clay_ElementDeclaration {} ) ) {
        vxui__emit_text(
            ctx,
            resolved,
            vxui__effective_font_id( ctx, font_id ),
            vxui__effective_font_size( ctx, font_size ),
            color,
            owner_id );
    }
}

static void vxui_menu__attach_focus_decor( vxui_ctx* ctx, uint32_t row_id, const vxui_menu_style* style )
{
    if ( !ctx || !style ) {
        return;
    }

    uint32_t prev_decl_id = ctx->current_decl_id;
    ctx->current_decl_id = row_id;

    switch ( style->focus_decor ) {
        case VXUI_MENU_FOCUS_DECOR_NONE:
            break;

        case VXUI_MENU_FOCUS_DECOR_GLOW: {
            vxui__trait_glow_params params = {
                .padding = style->focus_decor_padding,
                .alpha = style->focus_decor_alpha,
            };
            vxui__attach_trait( ctx, VXUI_TRAIT_GLOW, &params, sizeof( params ) );
        } break;

        case VXUI_MENU_FOCUS_DECOR_PULSE: {
            vxui__trait_pulse_params params = {
                .speed = 2.0f,
                .scale = 0.05f,
                .alpha = style->focus_decor_alpha,
            };
            vxui__attach_trait( ctx, VXUI_TRAIT_PULSE, &params, sizeof( params ) );
        } break;
    }

    ctx->current_decl_id = prev_decl_id;
}

static void vxui_menu__begin_common_row( vxui_menu__scope* scope, uint32_t row_id, bool focused, bool disabled, bool centered )
{
    const vxui_menu_style* style = &scope->style;
    vxui_color fill = disabled ? style->row_disabled_fill_color : ( focused ? style->row_focus_fill_color : style->row_fill_color );
    vxui_color border = focused ? style->row_focus_border_color : style->row_border_color;
    Clay_BorderWidth border_width = {};
    if ( ( style->flags & VXUI_MENU_STYLE_FOCUS_FRAME ) != 0u ) {
        uint16_t width = vxui_menu__u16( style->border_width );
        border_width = { width, width, width, width, width };
    }

    Clay__OpenElementWithId( vxui__clay_id_from_hash( row_id ) );
    Clay__ConfigureOpenElement( ( Clay_ElementDeclaration ) {
        .layout = {
            .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
            .padding = {
                vxui_menu__u16( style->padding_x ),
                vxui_menu__u16( style->padding_x ),
                vxui_menu__u16( style->padding_y ),
                vxui_menu__u16( style->padding_y ),
            },
            .childGap = vxui_menu__u16( style->secondary_gap ),
            .childAlignment = {
                .x = centered ? CLAY_ALIGN_X_CENTER : CLAY_ALIGN_X_LEFT,
            },
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
        .backgroundColor = vxui_menu__to_clay_color( fill ),
        .cornerRadius = CLAY_CORNER_RADIUS( style->corner_radius ),
        .border = {
            .color = vxui_menu__to_clay_color( border ),
            .width = border_width,
        },
    } );

    if ( focused ) {
        vxui_menu__attach_focus_decor( scope->ctx, row_id, style );
    }
}

static void vxui_menu__end_common_row( void )
{
    Clay__CloseElement();
}

static void vxui_menu__begin_top_lane( const vxui_menu__scope* scope, uint32_t row_id, bool centered )
{
    Clay__OpenElementWithId( vxui__clay_id_from_hash( vxui_menu__mix_id( row_id, 1u ) ) );
    Clay__ConfigureOpenElement( ( Clay_ElementDeclaration ) {
        .layout = {
            .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
            .childGap = vxui_menu__u16( scope->style.lane_gap ),
            .childAlignment = {
                .x = centered ? CLAY_ALIGN_X_CENTER : CLAY_ALIGN_X_LEFT,
                .y = CLAY_ALIGN_Y_CENTER,
            },
            .layoutDirection = CLAY_LEFT_TO_RIGHT,
        },
    } );
}

static void vxui_menu__end_top_lane( void )
{
    Clay__CloseElement();
}

static void vxui_menu__begin_lane( const vxui_menu__scope* scope, uint32_t row_id, uint32_t salt, bool label_lane )
{
    const vxui_menu_style* style = &scope->style;
    Clay_SizingAxis width = CLAY_SIZING_FIT( 0 );
    if ( style->layout_mode == VXUI_MENU_LAYOUT_FORM ) {
        if ( label_lane ) {
            width = CLAY_SIZING_FIXED( style->label_lane_width );
        } else {
            switch ( style->value_lane_mode ) {
                case VXUI_MENU_VALUE_LANE_GROW:
                    width = CLAY_SIZING_GROW( 0 );
                    break;
                case VXUI_MENU_VALUE_LANE_FIXED:
                    width = CLAY_SIZING_FIXED( style->value_lane_width );
                    break;
                case VXUI_MENU_VALUE_LANE_FIT:
                default:
                    width = CLAY_SIZING_FIT( 0 );
                    break;
            }
        }
    } else if ( !label_lane ) {
        width = CLAY_SIZING_FIT( 0 );
    } else {
        width = CLAY_SIZING_GROW( 0 );
    }

    Clay__OpenElementWithId( vxui__clay_id_from_hash( vxui_menu__mix_id( row_id, salt ) ) );
    Clay__ConfigureOpenElement( ( Clay_ElementDeclaration ) {
        .layout = {
            .sizing = { width, CLAY_SIZING_FIT( 0 ) },
            .childAlignment = {
                .x = scope->ctx->rtl ? CLAY_ALIGN_X_RIGHT : ( scope->style.layout_mode == VXUI_MENU_LAYOUT_CENTERED ? CLAY_ALIGN_X_CENTER : CLAY_ALIGN_X_LEFT ),
            },
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
    } );
}

static void vxui_menu__end_lane( void )
{
    Clay__CloseElement();
}

static void vxui_menu__emit_badge_inline( const vxui_menu__scope* scope, uint32_t row_id, const char* text_key, const vxui_menu_badge_cfg* cfg )
{
    if ( !text_key || ( cfg && cfg->hidden ) ) {
        return;
    }

    const vxui_menu_style* style = &scope->style;
    float padding_x = cfg && cfg->padding_x > 0.0f ? cfg->padding_x : style->badge_padding_x;
    float padding_y = cfg && cfg->padding_y > 0.0f ? cfg->padding_y : style->badge_padding_y;
    float radius = cfg && cfg->corner_radius > 0.0f ? cfg->corner_radius : style->corner_radius;
    float min_width = cfg && cfg->min_width > 0.0f ? cfg->min_width : 0.0f;
    uint32_t font_id = cfg && cfg->font_id != 0 ? cfg->font_id : style->badge_font_id;
    float font_size = cfg && cfg->font_size > 0.0f ? cfg->font_size : style->badge_font_size;
    vxui_color fill = cfg && cfg->fill_color.a != 0 ? cfg->fill_color : style->badge_fill_color;
    vxui_color text = cfg && cfg->text_color.a != 0 ? cfg->text_color : style->badge_text_color;

    Clay__OpenElementWithId( vxui__clay_id_from_hash( vxui_menu__mix_id( row_id, 17u ) ) );
    Clay__ConfigureOpenElement( ( Clay_ElementDeclaration ) {
        .layout = {
            .sizing = { min_width > 0.0f ? CLAY_SIZING_FIXED( min_width ) : CLAY_SIZING_FIT( 0 ), CLAY_SIZING_FIT( 0 ) },
            .padding = {
                vxui_menu__u16( padding_x ),
                vxui_menu__u16( padding_x ),
                vxui_menu__u16( padding_y ),
                vxui_menu__u16( padding_y ),
            },
            .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER },
        },
        .backgroundColor = vxui_menu__to_clay_color( fill ),
        .cornerRadius = CLAY_CORNER_RADIUS( radius ),
    } );
    vxui_menu__emit_text_leaf( scope->ctx, row_id, text_key, font_id, font_size, text );
    Clay__CloseElement();
}

static void vxui_menu__emit_label_and_value(
    const vxui_menu__scope* scope,
    uint32_t row_id,
    const char* label_key,
    const char* value_text,
    const vxui_menu_row_cfg* row_cfg,
    bool focused,
    bool disabled,
    bool interactive_value )
{
    vxui_color label_color = row_cfg && row_cfg->text_color.a != 0 ? row_cfg->text_color : vxui_menu__row_text_color( &scope->style, focused, disabled, false );
    vxui_color value_color = vxui_menu__row_text_color( &scope->style, focused, disabled, false );
    uint32_t font_id = vxui_menu__row_font_id( &scope->style, row_cfg );
    float font_size = vxui_menu__row_font_size( &scope->style, row_cfg );
    bool centered = scope->style.layout_mode == VXUI_MENU_LAYOUT_CENTERED;
    bool has_value = value_text && value_text[ 0 ] != '\0';

    vxui_menu__begin_top_lane( scope, row_id, centered );
    auto emit_label_lane = [&]() {
        vxui_menu__begin_lane( scope, row_id, 2u, true );
        vxui_menu__emit_text_leaf( scope->ctx, row_id, label_key, font_id, font_size, label_color );
        vxui_menu__end_lane();
    };
    auto emit_value_lane = [&]() {
        if ( !has_value && !( row_cfg && row_cfg->badge_text_key ) ) {
            return;
        }
        vxui_menu__begin_lane( scope, row_id, 3u, false );
        if ( has_value ) {
            Clay__OpenElementWithId( vxui__clay_id_from_hash( vxui_menu__mix_id( row_id, 11u ) ) );
            Clay__ConfigureOpenElement( ( Clay_ElementDeclaration ) {
                .layout = {
                    .sizing = { CLAY_SIZING_FIT( 0 ), CLAY_SIZING_FIT( 0 ) },
                    .childGap = vxui_menu__u16( scope->style.prompt_gap ),
                    .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                    .layoutDirection = CLAY_LEFT_TO_RIGHT,
                },
            } );
            if ( interactive_value && ( scope->style.flags & VXUI_MENU_STYLE_VALUE_ARROWS ) != 0u && focused && !disabled ) {
                vxui_menu__emit_text_leaf( scope->ctx, row_id, scope->ctx->rtl ? ">" : "<", font_id, font_size, scope->style.arrow_color );
            }
            vxui_menu__emit_text_leaf( scope->ctx, row_id, value_text, font_id, font_size, value_color );
            if ( interactive_value && ( scope->style.flags & VXUI_MENU_STYLE_VALUE_ARROWS ) != 0u && focused && !disabled ) {
                vxui_menu__emit_text_leaf( scope->ctx, row_id, scope->ctx->rtl ? "<" : ">", font_id, font_size, scope->style.arrow_color );
            }
            Clay__CloseElement();
        }
        if ( row_cfg && row_cfg->badge_text_key ) {
            vxui_menu__emit_badge_inline( scope, row_id, row_cfg->badge_text_key, &row_cfg->badge );
        }
        vxui_menu__end_lane();
    };

    if ( scope->ctx->rtl && has_value ) {
        emit_value_lane();
        emit_label_lane();
    } else {
        emit_label_lane();
        emit_value_lane();
    }
    vxui_menu__end_top_lane();

    if ( row_cfg && row_cfg->secondary_key && ( scope->style.flags & VXUI_MENU_STYLE_SECONDARY_TEXT ) != 0u ) {
        vxui_color secondary = vxui_menu__row_text_color( &scope->style, focused, disabled, true );
        vxui_menu__emit_text_leaf( scope->ctx, row_id, row_cfg->secondary_key, font_id, scope->style.secondary_font_size, secondary );
    }
}

static void vxui_menu__register_non_focusable( vxui_ctx* ctx, vxui_decl_kind kind, uint32_t row_id )
{
    vxui__register_decl( ctx, kind, row_id, 0, 0, 0, 0, false, false, true, nullptr, nullptr );
    vxui__get_anim_state( ctx, row_id, true );
    ctx->current_decl_id = row_id;
}

static void vxui_menu__track_row( vxui_menu__scope* scope, uint32_t row_id )
{
    int row_index = scope->row_count;
    scope->row_count += 1;
    if ( scope->state ) {
        scope->state->last_emitted_row_count = scope->row_count;
        if ( scope->ctx->focused_id == row_id ) {
            scope->focused_row_id = row_id;
            scope->focused_row_index = row_index;
        }
    }
}

static bool vxui_menu__option_wrap_enabled( const vxui_menu_style* style, const vxui_menu_row_cfg* row_cfg )
{
    if ( row_cfg ) {
        switch ( row_cfg->wrap_mode ) {
            case VXUI_MENU_WRAP_FORCE_OFF:
                return false;

            case VXUI_MENU_WRAP_FORCE_ON:
                return true;

            case VXUI_MENU_WRAP_DEFAULT:
            default:
                break;
        }
    }
    return style ? style->option_wrap_by_default : true;
}

vxui_menu_style vxui_menu_style_form( void )
{
    return ( vxui_menu_style ) {
        .flags = VXUI_MENU_STYLE_SECONDARY_TEXT | VXUI_MENU_STYLE_AUTO_SCROLL_TO_FOCUS | VXUI_MENU_STYLE_VALUE_ARROWS | VXUI_MENU_STYLE_FOCUS_FRAME,
        .layout_mode = VXUI_MENU_LAYOUT_FORM,
        .value_lane_mode = VXUI_MENU_VALUE_LANE_FIT,
        .row_height = 34.0f,
        .row_gap = 8.0f,
        .section_gap = 16.0f,
        .label_lane_width = 220.0f,
        .value_lane_width = 160.0f,
        .padding_x = 12.0f,
        .padding_y = 8.0f,
        .lane_gap = 12.0f,
        .secondary_gap = 4.0f,
        .corner_radius = 10.0f,
        .border_width = 1.0f,
        .badge_padding_x = 8.0f,
        .badge_padding_y = 4.0f,
        .prompt_gap = 8.0f,
        .scroll_stiffness = VXUI_DEFAULT_STIFFNESS,
        .scroll_damping = VXUI_DEFAULT_DAMPING,
        .option_wrap_by_default = true,
        .body_font_size = 24.0f,
        .secondary_font_size = 18.0f,
        .title_font_size = 30.0f,
        .badge_font_size = 16.0f,
        .focus_decor = VXUI_MENU_FOCUS_DECOR_GLOW,
        .focus_decor_padding = 4.0f,
        .focus_decor_alpha = 0.24f,
        .panel_fill_color = { 12, 16, 24, 192 },
        .row_fill_color = { 22, 28, 40, 220 },
        .row_focus_fill_color = { 32, 46, 68, 240 },
        .row_disabled_fill_color = { 18, 22, 28, 160 },
        .row_border_color = { 60, 86, 114, 120 },
        .row_focus_border_color = { 110, 180, 255, 255 },
        .text_color = { 230, 236, 244, 255 },
        .focused_text_color = { 255, 255, 255, 255 },
        .disabled_text_color = { 132, 140, 150, 255 },
        .secondary_text_color = { 164, 172, 182, 255 },
        .section_text_color = { 164, 198, 255, 255 },
        .badge_fill_color = { 70, 96, 132, 255 },
        .badge_text_color = { 245, 250, 255, 255 },
        .prompt_fill_color = { 20, 26, 34, 220 },
        .prompt_text_color = { 235, 240, 248, 255 },
        .arrow_color = { 148, 208, 255, 255 },
        .slider_track_color = { 32, 40, 56, 255 },
        .slider_fill_color = { 96, 176, 255, 255 },
    };
}

vxui_menu_style vxui_menu_style_br_panel( void )
{
    vxui_menu_style style = vxui_menu_style_form();
    style.flags |= VXUI_MENU_STYLE_FOOTER_PROMPT_BAR;
    style.row_height = 36.0f;
    style.row_gap = 10.0f;
    style.label_lane_width = 240.0f;
    style.corner_radius = 12.0f;
    style.row_fill_color = { 18, 24, 36, 224 };
    style.row_focus_fill_color = { 34, 52, 76, 244 };
    style.row_border_color = { 76, 108, 144, 144 };
    style.row_focus_border_color = { 156, 214, 255, 255 };
    style.section_text_color = { 196, 220, 255, 255 };
    style.focus_decor_alpha = 0.30f;
    return style;
}

vxui_menu_style vxui_menu_style_br_title( void )
{
    vxui_menu_style style = vxui_menu_style_form();
    style.flags |= VXUI_MENU_STYLE_FOOTER_PROMPT_BAR;
    style.layout_mode = VXUI_MENU_LAYOUT_CENTERED;
    style.value_lane_mode = VXUI_MENU_VALUE_LANE_FIT;
    style.row_height = 40.0f;
    style.row_gap = 14.0f;
    style.section_gap = 20.0f;
    style.label_lane_width = 0.0f;
    style.padding_x = 20.0f;
    style.padding_y = 10.0f;
    style.corner_radius = 14.0f;
    style.title_font_size = 40.0f;
    style.body_font_size = 28.0f;
    style.secondary_font_size = 20.0f;
    style.focus_decor = VXUI_MENU_FOCUS_DECOR_PULSE;
    style.focus_decor_alpha = 0.16f;
    style.row_fill_color = { 18, 22, 34, 150 };
    style.row_focus_fill_color = { 42, 60, 88, 220 };
    style.row_border_color = { 64, 90, 128, 96 };
    style.row_focus_border_color = { 178, 228, 255, 255 };
    style.text_color = { 228, 234, 244, 255 };
    style.focused_text_color = { 255, 255, 255, 255 };
    style.section_text_color = { 210, 228, 255, 255 };
    return style;
}

vxui_menu_style vxui_menu_style_compact( void )
{
    vxui_menu_style style = vxui_menu_style_form();
    style.row_height = 28.0f;
    style.row_gap = 6.0f;
    style.section_gap = 10.0f;
    style.label_lane_width = 180.0f;
    style.padding_x = 8.0f;
    style.padding_y = 6.0f;
    style.corner_radius = 8.0f;
    style.body_font_size = 20.0f;
    style.secondary_font_size = 15.0f;
    style.badge_font_size = 14.0f;
    return style;
}

vxui_menu_style vxui_menu_style_title_menu( void )
{
    vxui_menu_style style = vxui_menu_style_br_title();
    style.body_font_size = 18.0f;
    style.title_font_size = 24.0f;
    style.badge_font_size = 9.0f;
    style.row_height = 30.0f;
    style.row_gap = 8.0f;
    style.section_gap = 10.0f;
    style.padding_x = 12.0f;
    style.padding_y = 8.0f;
    style.panel_fill_color = { 14, 18, 28, 196 };
    style.row_fill_color = { 20, 26, 38, 172 };
    style.row_focus_fill_color = { 38, 54, 78, 228 };
    return style;
}

vxui_menu_style vxui_menu_style_split_deck( void )
{
    vxui_menu_style style = vxui_menu_style_br_panel();
    style.body_font_size = 16.0f;
    style.secondary_font_size = 13.0f;
    style.badge_font_size = 10.0f;
    style.row_gap = 8.0f;
    style.section_gap = 10.0f;
    style.padding_x = 10.0f;
    style.padding_y = 7.0f;
    style.panel_fill_color = { 12, 18, 28, 208 };
    return style;
}

vxui_menu_style vxui_menu_style_form_shell( void )
{
    vxui_menu_style style = vxui_menu_style_form();
    style.section_gap = 14.0f;
    style.panel_fill_color = { 10, 14, 22, 188 };
    return style;
}

vxui_menu_style vxui_menu_style_footer_strip( void )
{
    vxui_menu_style style = vxui_menu_style_compact();
    style.row_gap = 4.0f;
    style.section_gap = 8.0f;
    style.padding_x = 10.0f;
    style.padding_y = 6.0f;
    style.corner_radius = 10.0f;
    style.panel_fill_color = { 12, 18, 28, 220 };
    style.row_fill_color = { 0, 0, 0, 0 };
    style.row_focus_fill_color = { 0, 0, 0, 0 };
    return style;
}

void vxui_menu_begin( vxui_ctx* ctx, vxui_menu_state* state, const char* id, vxui_menu_cfg cfg )
{
    if ( !ctx || !state || !id || vxui_menu__scope_count >= VXUI_MENU__MAX_SCOPE_DEPTH ) {
        return;
    }

    vxui_menu_style style = cfg.style ? *cfg.style : vxui_menu_style_form();
    style = vxui_menu__sanitize_style( style );

    if ( state->scroll_target < 0.0f ) {
        state->scroll_target = 0.0f;
    }
    vxui__spring_step( state->scroll_target, &state->scroll_current, &state->scroll_velocity, style.scroll_stiffness, style.scroll_damping, ctx->delta_time );

    vxui_menu__scope* scope = &vxui_menu__scopes[ vxui_menu__scope_count++ ];
    *scope = {};
    scope->ctx = ctx;
    scope->state = state;
    scope->id = id;
    scope->style = style;
    scope->cfg = cfg;
    scope->viewport_id = vxui_idi( id, 1 );
    scope->content_id = vxui_idi( id, 2 );
    scope->focused_row_id = 0;
    scope->focused_row_index = -1;

    state->last_emitted_row_count = 0;

    Clay__OpenElementWithId( vxui__clay_id_from_hash( scope->viewport_id ) );
    Clay__ConfigureOpenElement( ( Clay_ElementDeclaration ) {
        .layout = {
            .sizing = { CLAY_SIZING_GROW( 0 ), cfg.viewport_height > 0.0f ? CLAY_SIZING_FIXED( cfg.viewport_height ) : CLAY_SIZING_GROW( 0 ) },
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
        .backgroundColor = vxui_menu__to_clay_color( style.panel_fill_color ),
        .cornerRadius = CLAY_CORNER_RADIUS( style.corner_radius ),
        .clip = {
            .horizontal = true,
            .vertical = true,
            .childOffset = { 0.0f, -state->scroll_current },
        },
    } );

    Clay__OpenElementWithId( vxui__clay_id_from_hash( scope->content_id ) );
    Clay__ConfigureOpenElement( ( Clay_ElementDeclaration ) {
        .layout = {
            .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
            .childGap = vxui_menu__u16( style.row_gap ),
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
    } );

    if ( cfg.title_key && cfg.title_key[ 0 ] != '\0' ) {
        uint32_t title_id = vxui_menu__mix_id( scope->viewport_id, 101u );
        vxui_menu__register_non_focusable( ctx, VXUI_DECL_LABEL, title_id );
        vxui_menu__begin_common_row( scope, title_id, false, false, true );
        vxui_menu__emit_text_leaf( ctx, title_id, cfg.title_key, style.title_font_id, style.title_font_size, style.section_text_color );
        if ( cfg.subtitle_key && cfg.subtitle_key[ 0 ] != '\0' ) {
            vxui_menu__emit_text_leaf( ctx, title_id, cfg.subtitle_key, style.body_font_id, style.secondary_font_size, style.secondary_text_color );
        }
        vxui_menu__end_common_row();
        vxui_menu__track_row( scope, title_id );
    }
}

void vxui_menu_end( vxui_ctx* ctx, vxui_menu_state* state )
{
    vxui_menu__scope* scope = vxui_menu__current_scope();
    if ( !scope || scope->ctx != ctx || scope->state != state ) {
        return;
    }

    Clay__CloseElement();
    Clay__CloseElement();

    if ( ( scope->style.flags & VXUI_MENU_STYLE_AUTO_SCROLL_TO_FOCUS ) != 0u ) {
        vxui_rect focus_bounds = {};
        vxui_rect viewport_bounds = {};
        vxui_rect content_bounds = {};
        bool have_focus_bounds = false;
        if ( scope->focused_row_id != 0u ) {
            have_focus_bounds = vxui_find_anim_bounds( ctx, scope->focused_row_id, &focus_bounds ) && focus_bounds.w > 0.0f && focus_bounds.h > 0.0f;
        }
        if ( !have_focus_bounds && scope->focused_row_id != 0u ) {
            Clay_ElementData focused = Clay_GetElementData( vxui__clay_id_from_hash( scope->focused_row_id ) );
            if ( focused.found ) {
                focus_bounds = vxui__rect_from_clay_box( focused.boundingBox );
                have_focus_bounds = focus_bounds.w > 0.0f && focus_bounds.h > 0.0f;
            }
        }
        if ( have_focus_bounds ) {
            Clay_ElementData viewport = Clay_GetElementData( vxui__clay_id_from_hash( scope->viewport_id ) );
            Clay_ElementData content = Clay_GetElementData( vxui__clay_id_from_hash( scope->content_id ) );
            if ( viewport.found ) {
                viewport_bounds = vxui__rect_from_clay_box( viewport.boundingBox );
                float margin = scope->style.padding_y + scope->style.row_gap;
                if ( focus_bounds.y < viewport_bounds.y + margin ) {
                    state->scroll_target -= ( viewport_bounds.y + margin ) - focus_bounds.y;
                }
                if ( focus_bounds.y + focus_bounds.h > viewport_bounds.y + viewport_bounds.h - margin ) {
                    state->scroll_target += ( focus_bounds.y + focus_bounds.h ) - ( viewport_bounds.y + viewport_bounds.h - margin );
                }
            }
            if ( content.found && viewport.found ) {
                content_bounds = vxui__rect_from_clay_box( content.boundingBox );
                float estimated_content_height = scope->row_count > 0
                    ? ( float ) scope->row_count * scope->style.row_height + ( float ) ( scope->row_count - 1 ) * scope->style.row_gap
                    : 0.0f;
                float max_target = content_bounds.h - viewport_bounds.h;
                if ( estimated_content_height - viewport_bounds.h > max_target ) {
                    max_target = estimated_content_height - viewport_bounds.h;
                }
                if ( max_target < 0.0f ) {
                    max_target = 0.0f;
                }
                if ( state->scroll_target < 0.0f ) {
                    state->scroll_target = 0.0f;
                }
                if ( state->scroll_target > max_target ) {
                    state->scroll_target = max_target;
                }
            }
        } else if ( scope->focused_row_id != 0u && scope->focused_row_index >= 0 ) {
            float viewport_height = scope->cfg.viewport_height > 0.0f ? scope->cfg.viewport_height : 0.0f;
            float margin = scope->style.padding_y + scope->style.row_gap;
            float row_stride = scope->style.row_height + scope->style.row_gap;
            float row_top = ( float ) scope->focused_row_index * row_stride;
            float row_bottom = row_top + scope->style.row_height;
            if ( viewport_height > 0.0f ) {
                if ( row_top < state->scroll_target + margin ) {
                    state->scroll_target = row_top - margin;
                }
                if ( row_bottom > state->scroll_target + viewport_height - margin ) {
                    state->scroll_target = row_bottom - viewport_height + margin;
                }
                float max_target = ( float ) scope->row_count * row_stride - viewport_height;
                if ( max_target < 0.0f ) {
                    max_target = 0.0f;
                }
                if ( state->scroll_target < 0.0f ) {
                    state->scroll_target = 0.0f;
                }
                if ( state->scroll_target > max_target ) {
                    state->scroll_target = max_target;
                }
            }
        }
    }

    state->last_focused_row_id = scope->focused_row_id;
    state->last_emitted_row_count = scope->row_count;
    vxui_menu__scope_count -= 1;
}

void vxui_menu_section( vxui_ctx* ctx, vxui_menu_state* state, const char* id, const char* title_key, vxui_menu_section_cfg cfg )
{
    ( void ) state;
    vxui_menu__scope* scope = vxui_menu__current_scope();
    if ( !scope || scope->ctx != ctx || cfg.hidden ) {
        return;
    }

    uint32_t row_id = vxui_menu__row_id( scope, id );
    vxui_menu__register_non_focusable( ctx, VXUI_DECL_LABEL, row_id );
    vxui_menu__begin_common_row( scope, row_id, false, false, cfg.centered || scope->style.layout_mode == VXUI_MENU_LAYOUT_CENTERED );
    vxui_menu__emit_text_leaf(
        ctx,
        row_id,
        title_key,
        cfg.font_id != 0 ? cfg.font_id : scope->style.title_font_id,
        cfg.font_size > 0.0f ? cfg.font_size : scope->style.title_font_size,
        cfg.text_color.a != 0 ? cfg.text_color : scope->style.section_text_color );
    if ( cfg.secondary_key && ( scope->style.flags & VXUI_MENU_STYLE_SECONDARY_TEXT ) != 0u ) {
        vxui_menu__emit_text_leaf( ctx, row_id, cfg.secondary_key, scope->style.body_font_id, scope->style.secondary_font_size, scope->style.secondary_text_color );
    }
    vxui_menu__end_common_row();
    vxui_menu__track_row( scope, row_id );
}

void vxui_menu_label( vxui_ctx* ctx, vxui_menu_state* state, const char* id, const char* label_key, vxui_menu_row_cfg cfg )
{
    ( void ) state;
    vxui_menu__scope* scope = vxui_menu__current_scope();
    if ( !scope || scope->ctx != ctx || !vxui_menu__visible( &cfg ) ) {
        return;
    }

    uint32_t row_id = vxui_menu__row_id( scope, id );
    vxui_menu__register_non_focusable( ctx, VXUI_DECL_LABEL, row_id );
    vxui_menu__begin_common_row( scope, row_id, false, cfg.disabled, scope->style.layout_mode == VXUI_MENU_LAYOUT_CENTERED );
    vxui_menu__emit_label_and_value( scope, row_id, label_key, cfg.value_key ? cfg.value_key : "", &cfg, false, cfg.disabled, false );
    vxui_menu__end_common_row();
    vxui_menu__track_row( scope, row_id );
}

void vxui_menu_action( vxui_ctx* ctx, vxui_menu_state* state, const char* id, const char* label_key, vxui_action_fn fn, vxui_menu_row_cfg row_cfg, vxui_action_cfg action_cfg )
{
    ( void ) state;
    vxui_menu__scope* scope = vxui_menu__current_scope();
    if ( !scope || scope->ctx != ctx || !vxui_menu__visible( &row_cfg ) ) {
        return;
    }

    uint32_t row_id = vxui_menu__row_id( scope, id );
    action_cfg.disabled = action_cfg.disabled || row_cfg.disabled;
    action_cfg.no_focus_ring = true;
    vxui__register_action( ctx, row_id, fn, action_cfg );
    vxui__get_anim_state( ctx, row_id, true );
    ctx->current_decl_id = row_id;

    bool focused = ctx->focused_id == row_id && !action_cfg.disabled;
    vxui_menu__begin_common_row( scope, row_id, focused, action_cfg.disabled, scope->style.layout_mode == VXUI_MENU_LAYOUT_CENTERED );
    vxui_menu__emit_label_and_value( scope, row_id, label_key, row_cfg.value_key ? row_cfg.value_key : "", &row_cfg, focused, action_cfg.disabled, false );
    vxui_menu__end_common_row();
    vxui_menu__track_row( scope, row_id );
}

void vxui_menu_option( vxui_ctx* ctx, vxui_menu_state* state, const char* id, const char* label_key, int* index, const char** option_keys, int count, vxui_menu_row_cfg row_cfg, vxui_option_cfg option_cfg )
{
    ( void ) state;
    vxui_menu__scope* scope = vxui_menu__current_scope();
    if ( !scope || scope->ctx != ctx || !vxui_menu__visible( &row_cfg ) ) {
        return;
    }

    uint32_t row_id = vxui_menu__row_id( scope, id );
    bool interactive = !row_cfg.disabled;
    bool wrap = vxui_menu__option_wrap_enabled( &scope->style, &row_cfg );

    vxui__register_decl(
        ctx,
        VXUI_DECL_OPTION,
        row_id,
        interactive ? option_cfg.nav_up : 0,
        interactive ? option_cfg.nav_down : 0,
        interactive ? option_cfg.nav_left : 0,
        interactive ? option_cfg.nav_right : 0,
        interactive,
        !interactive,
        true,
        nullptr,
        option_cfg.userdata );
    vxui__get_anim_state( ctx, row_id, true );
    if ( interactive ) {
        vxui__push_input_owner( ctx, row_id, false, false, true, true );
    }
    ctx->current_decl_id = row_id;

    if ( index && count > 0 ) {
        if ( *index < 0 ) {
            *index = 0;
        }
        if ( *index >= count ) {
            *index = count - 1;
        }
    }

    if ( interactive && ctx->focused_id == row_id && index && count > 0 ) {
        bool changed = false;
        if ( ( ctx->pending_nav_mask & ( 1u << VXUI_DIR_LEFT ) ) != 0u ) {
            changed = vxui__option_step( index, count, -1, wrap ) || changed;
            ctx->pending_nav_mask &= ~( 1u << VXUI_DIR_LEFT );
        }
        if ( ( ctx->pending_nav_mask & ( 1u << VXUI_DIR_RIGHT ) ) != 0u ) {
            changed = vxui__option_step( index, count, +1, wrap ) || changed;
            ctx->pending_nav_mask &= ~( 1u << VXUI_DIR_RIGHT );
        }
        if ( changed && option_cfg.on_change ) {
            option_cfg.on_change( ctx, *index, option_cfg.userdata );
        }
    }

    const char* value_key = ( index && option_keys && count > 0 ) ? option_keys[ *index ] : "";
    bool focused = interactive && ctx->focused_id == row_id;
    vxui_menu__begin_common_row( scope, row_id, focused, !interactive, scope->style.layout_mode == VXUI_MENU_LAYOUT_CENTERED );
    vxui_menu__emit_label_and_value( scope, row_id, label_key, value_key, &row_cfg, focused, !interactive, true );
    vxui_menu__end_common_row();
    vxui_menu__track_row( scope, row_id );
}

void vxui_menu_slider( vxui_ctx* ctx, vxui_menu_state* state, const char* id, const char* label_key, float* value, float min_value, float max_value, vxui_menu_row_cfg row_cfg, vxui_slider_cfg slider_cfg )
{
    ( void ) state;
    vxui_menu__scope* scope = vxui_menu__current_scope();
    if ( !scope || scope->ctx != ctx || !vxui_menu__visible( &row_cfg ) ) {
        return;
    }

    uint32_t row_id = vxui_menu__row_id( scope, id );
    bool interactive = !row_cfg.disabled;
    float range = max_value - min_value;
    float step = slider_cfg.step != 0.0f ? slider_cfg.step : ( range != 0.0f ? range / 20.0f : 0.0f );
    if ( step < 0.0f ) {
        step = -step;
    }

    vxui__register_decl( ctx, VXUI_DECL_SLIDER, row_id, 0, 0, 0, 0, interactive, !interactive, true, nullptr, slider_cfg.userdata );
    vxui__get_anim_state( ctx, row_id, true );
    if ( interactive ) {
        vxui__push_input_owner( ctx, row_id, false, false, true, true );
    }
    ctx->current_decl_id = row_id;

    if ( value ) {
        if ( *value < min_value ) {
            *value = min_value;
        }
        if ( *value > max_value ) {
            *value = max_value;
        }
    }

    if ( interactive && ctx->focused_id == row_id && value ) {
        float next = *value;
        if ( ( ctx->pending_nav_mask & ( 1u << VXUI_DIR_LEFT ) ) != 0u ) {
            next -= step;
            ctx->pending_nav_mask &= ~( 1u << VXUI_DIR_LEFT );
        }
        if ( ( ctx->pending_nav_mask & ( 1u << VXUI_DIR_RIGHT ) ) != 0u ) {
            next += step;
            ctx->pending_nav_mask &= ~( 1u << VXUI_DIR_RIGHT );
        }
        if ( next < min_value ) {
            next = min_value;
        }
        if ( next > max_value ) {
            next = max_value;
        }
        if ( next != *value ) {
            *value = next;
            if ( slider_cfg.on_change ) {
                slider_cfg.on_change( ctx, *value, slider_cfg.userdata );
            }
        }
    }

    float fill = 0.0f;
    if ( value && range > 0.0f ) {
        fill = vxui__clamp01( ( *value - min_value ) / range );
    }

    char formatted[ 64 ] = {};
    if ( value ) {
        const char* format = slider_cfg.format ? slider_cfg.format : "%.2f";
        std::snprintf( formatted, sizeof( formatted ), format, *value );
    }

    bool focused = interactive && ctx->focused_id == row_id;
    vxui_menu__begin_common_row( scope, row_id, focused, !interactive, scope->style.layout_mode == VXUI_MENU_LAYOUT_CENTERED );

    vxui_color label_color = row_cfg.text_color.a != 0 ? row_cfg.text_color : vxui_menu__row_text_color( &scope->style, focused, !interactive, false );
    uint32_t font_id = vxui_menu__row_font_id( &scope->style, &row_cfg );
    float font_size = vxui_menu__row_font_size( &scope->style, &row_cfg );
    bool centered = scope->style.layout_mode == VXUI_MENU_LAYOUT_CENTERED;

    vxui_menu__begin_top_lane( scope, row_id, centered );
    auto emit_label_lane = [&]() {
        vxui_menu__begin_lane( scope, row_id, 2u, true );
        vxui_menu__emit_text_leaf( ctx, row_id, label_key, font_id, font_size, label_color );
        vxui_menu__end_lane();
    };
    auto emit_value_lane = [&]() {
        vxui_menu__begin_lane( scope, row_id, 3u, false );
        Clay__OpenElementWithId( vxui__clay_id_from_hash( vxui_menu__mix_id( row_id, 31u ) ) );
        Clay__ConfigureOpenElement( ( Clay_ElementDeclaration ) {
            .layout = {
                .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                .childGap = vxui_menu__u16( scope->style.prompt_gap ),
                .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                .layoutDirection = CLAY_LEFT_TO_RIGHT,
            },
        } );

        if ( focused && interactive && ( scope->style.flags & VXUI_MENU_STYLE_VALUE_ARROWS ) != 0u ) {
            vxui_menu__emit_text_leaf( ctx, row_id, scope->ctx->rtl ? ">" : "<", font_id, font_size, scope->style.arrow_color );
        }

        Clay__OpenElementWithId( vxui__clay_id_from_hash( vxui_menu__mix_id( row_id, 32u ) ) );
        Clay__ConfigureOpenElement( ( Clay_ElementDeclaration ) {
            .layout = {
                .sizing = { scope->style.layout_mode == VXUI_MENU_LAYOUT_CENTERED ? CLAY_SIZING_FIXED( 180.0f ) : CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIXED( 12.0f ) },
            },
            .backgroundColor = vxui_menu__to_clay_color( scope->style.slider_track_color ),
            .cornerRadius = CLAY_CORNER_RADIUS( 6.0f ),
        } );
        CLAY_AUTO_ID( {
            .layout = {
                .sizing = { CLAY_SIZING_PERCENT( fill ), CLAY_SIZING_GROW( 0 ) },
            },
            .backgroundColor = vxui_menu__to_clay_color( scope->style.slider_fill_color ),
            .cornerRadius = CLAY_CORNER_RADIUS( 6.0f ),
        } ) {}
        Clay__CloseElement();

        vxui_menu__emit_text_leaf( ctx, row_id, formatted, font_id, font_size, vxui_menu__row_text_color( &scope->style, focused, !interactive, false ) );
        if ( row_cfg.badge_text_key ) {
            vxui_menu__emit_badge_inline( scope, row_id, row_cfg.badge_text_key, &row_cfg.badge );
        }
        if ( focused && interactive && ( scope->style.flags & VXUI_MENU_STYLE_VALUE_ARROWS ) != 0u ) {
            vxui_menu__emit_text_leaf( ctx, row_id, scope->ctx->rtl ? "<" : ">", font_id, font_size, scope->style.arrow_color );
        }
        Clay__CloseElement();
        vxui_menu__end_lane();
    };

    if ( scope->ctx->rtl ) {
        emit_value_lane();
        emit_label_lane();
    } else {
        emit_label_lane();
        emit_value_lane();
    }
    vxui_menu__end_top_lane();

    if ( row_cfg.secondary_key && ( scope->style.flags & VXUI_MENU_STYLE_SECONDARY_TEXT ) != 0u ) {
        vxui_menu__emit_text_leaf( ctx, row_id, row_cfg.secondary_key, font_id, scope->style.secondary_font_size, vxui_menu__row_text_color( &scope->style, focused, !interactive, true ) );
    }

    vxui_menu__end_common_row();
    vxui_menu__track_row( scope, row_id );
}

void vxui_menu_badge( vxui_ctx* ctx, vxui_menu_state* state, const char* id, const char* text_key, vxui_menu_badge_cfg cfg )
{
    ( void ) state;
    vxui_menu__scope* scope = vxui_menu__current_scope();
    if ( !scope || scope->ctx != ctx || cfg.hidden ) {
        return;
    }

    uint32_t row_id = vxui_menu__row_id( scope, id );
    vxui_menu__register_non_focusable( ctx, VXUI_DECL_LABEL, row_id );
    vxui_menu__begin_common_row( scope, row_id, false, false, scope->style.layout_mode == VXUI_MENU_LAYOUT_CENTERED );
    vxui_menu__emit_badge_inline( scope, row_id, text_key, &cfg );
    vxui_menu__end_common_row();
    vxui_menu__track_row( scope, row_id );
}

void vxui_menu_prompt_bar( vxui_ctx* ctx, vxui_menu_state* state, const char* id, const vxui_menu_prompt_bar_cfg* cfg )
{
    ( void ) state;
    vxui_menu__scope* scope = vxui_menu__current_scope();
    if ( !scope || scope->ctx != ctx || !cfg || cfg->hidden || cfg->count <= 0 ) {
        return;
    }

    uint32_t row_id = vxui_menu__row_id( scope, id );
    vxui_menu__register_non_focusable( ctx, VXUI_DECL_PROMPT, row_id );
    vxui_menu__begin_common_row( scope, row_id, false, false, scope->style.layout_mode == VXUI_MENU_LAYOUT_CENTERED );

    Clay__OpenElementWithId( vxui__clay_id_from_hash( vxui_menu__mix_id( row_id, 51u ) ) );
    Clay__ConfigureOpenElement( ( Clay_ElementDeclaration ) {
        .layout = {
            .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
            .childGap = vxui_menu__u16( cfg->gap > 0.0f ? cfg->gap : scope->style.prompt_gap ),
            .childAlignment = {
                .x = scope->style.layout_mode == VXUI_MENU_LAYOUT_CENTERED ? CLAY_ALIGN_X_CENTER : CLAY_ALIGN_X_LEFT,
                .y = CLAY_ALIGN_Y_CENTER,
            },
            .layoutDirection = CLAY_LEFT_TO_RIGHT,
        },
    } );

    for ( int i = 0; i < cfg->count; ++i ) {
        const char* action_name = cfg->action_names ? cfg->action_names[ i ] : nullptr;
        const char* label_key = cfg->label_keys ? cfg->label_keys[ i ] : nullptr;
        if ( !action_name ) {
            continue;
        }

        CLAY_AUTO_ID( {
            .layout = {
                .sizing = { CLAY_SIZING_FIT( 0 ), CLAY_SIZING_FIT( 0 ) },
                .childGap = vxui_menu__u16( scope->style.prompt_gap * 0.5f ),
                .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                .layoutDirection = CLAY_LEFT_TO_RIGHT,
            },
            .backgroundColor = vxui_menu__to_clay_color( scope->style.prompt_fill_color ),
            .cornerRadius = CLAY_CORNER_RADIUS( scope->style.corner_radius ),
        } ) {
            VXUI_PROMPT( ctx, action_name );
            if ( label_key ) {
                vxui_menu__emit_text_leaf( ctx, row_id, label_key, scope->style.body_font_id, scope->style.secondary_font_size, scope->style.prompt_text_color );
            }
        }
    }

    Clay__CloseElement();
    vxui_menu__end_common_row();
    vxui_menu__track_row( scope, row_id );
}

void vxui_menu_surface_begin( vxui_ctx* ctx, const char* root_id, const char* surface_id, const vxui_menu_surface_cfg* cfg )
{
    if ( !ctx || !root_id || !surface_id || vxui_menu__surface_scope_count >= VXUI_MENU__MAX_SURFACE_SCOPE_DEPTH ) {
        return;
    }

    vxui_menu_surface_cfg resolved = {};
    if ( cfg ) {
        resolved = *cfg;
    }
    resolved = vxui_menu__sanitize_surface_cfg( resolved );

    vxui_menu__surface_scope* scope = &vxui_menu__surface_scopes[ vxui_menu__surface_scope_count++ ];
    scope->ctx = ctx;
    scope->root_id = root_id;
    scope->surface_id = surface_id;

    Clay__OpenElementWithId( vxui__clay_id_from_hash( vxui_id( root_id ) ) );
    Clay__ConfigureOpenElement( ( Clay_ElementDeclaration ) {
        .layout = {
            .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_GROW( 0 ) },
            .padding = CLAY_PADDING_ALL( vxui_menu__u16( resolved.outer_padding ) ),
            .childAlignment = { .x = CLAY_ALIGN_X_CENTER },
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
        .backgroundColor = vxui_menu__to_clay_color( resolved.background_fill_color ),
    } );

    Clay__OpenElementWithId( vxui__clay_id_from_hash( vxui_id( surface_id ) ) );
    Clay__ConfigureOpenElement( ( Clay_ElementDeclaration ) {
        .layout = {
            .sizing = {
                resolved.surface_width > 0.0f ? CLAY_SIZING_FIXED( resolved.surface_width ) : CLAY_SIZING_GROW( 0 ),
                resolved.surface_max_height > 0.0f ? CLAY_SIZING_GROW( 0, resolved.surface_max_height ) : CLAY_SIZING_GROW( 0 ),
            },
            .padding = {
                vxui_menu__u16( resolved.surface_padding_x ),
                vxui_menu__u16( resolved.surface_padding_x ),
                vxui_menu__u16( resolved.surface_padding_y ),
                vxui_menu__u16( resolved.surface_padding_y ),
            },
            .childGap = vxui_menu__u16( resolved.section_gap ),
            .childAlignment = { .x = ctx->rtl ? CLAY_ALIGN_X_RIGHT : CLAY_ALIGN_X_LEFT },
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
        .backgroundColor = vxui_menu__to_clay_color( resolved.surface_fill_color ),
        .cornerRadius = CLAY_CORNER_RADIUS( resolved.corner_radius ),
        .border = {
            .color = vxui_menu__to_clay_color( resolved.surface_border_color ),
            .width = CLAY_BORDER_ALL( vxui_menu__u16( resolved.border_width ) ),
        },
    } );
}

void vxui_menu_surface_end( vxui_ctx* ctx )
{
    if ( vxui_menu__surface_scope_count <= 0 ) {
        return;
    }

    vxui_menu__surface_scope* scope = &vxui_menu__surface_scopes[ vxui_menu__surface_scope_count - 1 ];
    if ( scope->ctx != ctx ) {
        return;
    }

    Clay__CloseElement();
    Clay__CloseElement();
    vxui_menu__surface_scope_count -= 1;
}

void vxui_menu_screen_begin( vxui_ctx* ctx, vxui_menu_state* state, const char* id, const vxui_menu_screen_cfg* cfg )
{
    if ( !ctx || !state || !id || vxui_menu__screen_scope_count >= VXUI_MENU__MAX_SCREEN_SCOPE_DEPTH ) {
        return;
    }

    vxui_menu_screen_cfg resolved = {};
    if ( cfg ) {
        resolved = *cfg;
    }

    vxui_menu_style style = cfg && cfg->style ? *cfg->style : vxui_menu__default_screen_style( resolved.shell_kind );
    style = vxui_menu__sanitize_style( style );

    vxui_menu__screen_scope* scope = &vxui_menu__screen_scopes[ vxui_menu__screen_scope_count++ ];
    *scope = {};
    scope->ctx = ctx;
    scope->state = state;
    scope->id = id;
    scope->style = style;
    scope->cfg = resolved;
    scope->compact = vxui_menu__resolve_screen_compact( ctx, &resolved );

    Clay__OpenElementWithId( vxui__clay_id_from_hash( vxui_id( id ) ) );
    Clay__ConfigureOpenElement( ( Clay_ElementDeclaration ) {
        .layout = {
            .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_GROW( 0 ) },
            .childGap = vxui_menu__u16( scope->compact ? style.section_gap * 0.75f : style.section_gap ),
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
    } );
}

void vxui_menu_screen_end( vxui_ctx* ctx, vxui_menu_state* state )
{
    vxui_menu__screen_scope* scope = vxui_menu__current_screen_scope();
    if ( !scope || scope->ctx != ctx || scope->state != state ) {
        return;
    }

    vxui_menu__close_screen_body( scope );
    Clay__CloseElement();
    vxui_menu__screen_scope_count -= 1;
}

void vxui_menu_header( vxui_ctx* ctx, const char* id, const vxui_menu_header_cfg* cfg )
{
    vxui_menu__screen_scope* scope = vxui_menu__current_screen_scope();
    if ( !scope || scope->ctx != ctx ) {
        return;
    }

    const vxui_menu_header_cfg* header = cfg ? cfg : &scope->cfg.header;
    if ( !header || header->hidden || ( !header->title_key && !header->subtitle_key ) ) {
        return;
    }

    const char* root_id = id ? id : vxui_menu__push_child_id( ctx, scope->id, "header" );
    float pad_x = scope->compact ? scope->style.padding_x * 0.8f : scope->style.padding_x * 0.95f;
    float pad_y = scope->compact ? scope->style.padding_y * 0.55f : scope->style.padding_y * 0.72f;

    Clay__OpenElementWithId( vxui__clay_id_from_hash( vxui_id( root_id ) ) );
    Clay__ConfigureOpenElement( ( Clay_ElementDeclaration ) {
        .layout = {
            .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
            .padding = {
                vxui_menu__u16( pad_x ),
                vxui_menu__u16( pad_x ),
                vxui_menu__u16( pad_y ),
                vxui_menu__u16( pad_y ),
            },
            .childGap = vxui_menu__u16( scope->style.secondary_gap * 0.75f ),
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
        .backgroundColor = vxui_menu__to_clay_color( vxui_menu__scale_alpha( scope->style.panel_fill_color, 0.72f ) ),
        .cornerRadius = CLAY_CORNER_RADIUS( scope->style.corner_radius ),
        .border = {
            .color = vxui_menu__to_clay_color( vxui_menu__scale_alpha( scope->style.row_border_color, 0.85f ) ),
            .width = CLAY_BORDER_ALL( 1 ),
        },
    } );

    if ( header->title_key ) {
        const char* title_id = vxui_menu__push_child_id( ctx, root_id, "title" );
        vxui_menu__emit_text_leaf( ctx, vxui_id( title_id ), header->title_key, scope->style.title_font_id, scope->compact ? scope->style.title_font_size * 0.88f : scope->style.title_font_size, scope->style.section_text_color );
    }
    if ( header->subtitle_key ) {
        const char* subtitle_id = vxui_menu__push_child_id( ctx, root_id, "subtitle" );
        vxui_menu__emit_text_leaf( ctx, vxui_id( subtitle_id ), header->subtitle_key, scope->style.body_font_id, scope->style.secondary_font_size, scope->style.secondary_text_color );
    }

    Clay__CloseElement();
}

static void vxui_menu__lane_begin_impl( vxui_ctx* ctx, const char* id, const vxui_menu_lane_cfg* cfg, vxui_menu__lane_role role )
{
    vxui_menu__screen_scope* scope = vxui_menu__current_screen_scope();
    if ( !scope || scope->ctx != ctx || scope->lane_open ) {
        return;
    }

    vxui_menu_lane_cfg lane = vxui_menu__resolve_lane_cfg( scope, role, cfg );
    if ( lane.hidden ) {
        return;
    }

    vxui_menu__ensure_screen_body_open( scope );

    Clay_SizingAxis width = lane.grow
        ? CLAY_SIZING_GROW( 0 )
        : ( lane.width > 0.0f ? CLAY_SIZING_FIXED( lane.width ) : ( lane.min_width > 0.0f ? CLAY_SIZING_FIXED( lane.min_width ) : CLAY_SIZING_FIT( 0 ) ) );
    float pad_x = scope->compact ? scope->style.padding_x * 0.85f : scope->style.padding_x;
    float pad_y = scope->compact ? scope->style.padding_y * 0.85f : scope->style.padding_y;
    vxui_color fill_color = scope->style.panel_fill_color;
    vxui_color border_color = scope->style.row_border_color;
    if ( role == VXUI_MENU__LANE_ROLE_PRIMARY ) {
        fill_color = vxui_menu__scale_alpha( scope->style.row_fill_color, 0.92f );
    } else if ( role == VXUI_MENU__LANE_ROLE_SECONDARY ) {
        fill_color = vxui_menu__scale_alpha( scope->style.panel_fill_color, 1.06f );
        border_color = scope->style.row_focus_border_color;
    } else if ( role == VXUI_MENU__LANE_ROLE_TERTIARY ) {
        fill_color = vxui_menu__scale_alpha( scope->style.panel_fill_color, 0.7f );
        border_color = vxui_menu__scale_alpha( scope->style.row_border_color, 0.65f );
    }
    const char* lane_id = id
        ? id
        : vxui_menu__push_child_id(
              ctx,
              scope->id,
              role == VXUI_MENU__LANE_ROLE_PRIMARY ? "primary_lane" : ( role == VXUI_MENU__LANE_ROLE_SECONDARY ? "secondary_lane" : "tertiary_lane" ) );

    Clay__OpenElementWithId( vxui__clay_id_from_hash( vxui_id( lane_id ) ) );
    Clay__ConfigureOpenElement( ( Clay_ElementDeclaration ) {
        .layout = {
            .sizing = { width, CLAY_SIZING_GROW( 0 ) },
            .padding = {
                vxui_menu__u16( pad_x ),
                vxui_menu__u16( pad_x ),
                vxui_menu__u16( pad_y ),
                vxui_menu__u16( pad_y ),
            },
            .childGap = vxui_menu__u16( scope->compact ? scope->style.row_gap * 0.75f : scope->style.row_gap ),
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
        .backgroundColor = vxui_menu__to_clay_color( lane.weak ? vxui_menu__scale_alpha( fill_color, 0.8f ) : fill_color ),
        .cornerRadius = CLAY_CORNER_RADIUS( scope->style.corner_radius ),
        .border = {
            .color = vxui_menu__to_clay_color( lane.weak ? vxui_menu__scale_alpha( border_color, 0.7f ) : border_color ),
            .width = { 1, 1, 1, 1, 1 },
        },
    } );

    scope->lane_open = true;
    scope->active_lane = role;
}

static void vxui_menu__lane_end_impl( vxui_ctx* ctx )
{
    vxui_menu__screen_scope* scope = vxui_menu__current_screen_scope();
    if ( !scope || scope->ctx != ctx || !scope->lane_open ) {
        return;
    }

    Clay__CloseElement();
    scope->lane_open = false;
    scope->active_lane = VXUI_MENU__LANE_ROLE_NONE;
}

void vxui_menu_primary_lane_begin( vxui_ctx* ctx, const char* id, const vxui_menu_lane_cfg* cfg )
{
    vxui_menu__lane_begin_impl( ctx, id, cfg, VXUI_MENU__LANE_ROLE_PRIMARY );
}

void vxui_menu_primary_lane_end( vxui_ctx* ctx )
{
    vxui_menu__lane_end_impl( ctx );
}

void vxui_menu_secondary_lane_begin( vxui_ctx* ctx, const char* id, const vxui_menu_lane_cfg* cfg )
{
    vxui_menu__lane_begin_impl( ctx, id, cfg, VXUI_MENU__LANE_ROLE_SECONDARY );
}

void vxui_menu_secondary_lane_end( vxui_ctx* ctx )
{
    vxui_menu__lane_end_impl( ctx );
}

void vxui_menu_tertiary_lane_begin( vxui_ctx* ctx, const char* id, const vxui_menu_lane_cfg* cfg )
{
    vxui_menu__lane_begin_impl( ctx, id, cfg, VXUI_MENU__LANE_ROLE_TERTIARY );
}

void vxui_menu_tertiary_lane_end( vxui_ctx* ctx )
{
    vxui_menu__lane_end_impl( ctx );
}

void vxui_menu_help_legend( vxui_ctx* ctx, const char* id, const vxui_menu_help_cfg* cfg )
{
    vxui_menu__screen_scope* scope = vxui_menu__current_screen_scope();
    if ( !scope || scope->ctx != ctx || !cfg || cfg->hidden ) {
        return;
    }

    const char* root_id = id ? id : vxui_menu__push_child_id( ctx, scope->id, "help_legend" );
    int line_count = cfg->line_count;
    if ( scope->compact && cfg->compact_line_count > 0 && cfg->compact_line_count < line_count ) {
        line_count = cfg->compact_line_count;
    }

    Clay__OpenElementWithId( vxui__clay_id_from_hash( vxui_id( root_id ) ) );
    Clay__ConfigureOpenElement( ( Clay_ElementDeclaration ) {
        .layout = {
            .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
            .childGap = vxui_menu__u16( scope->compact ? scope->style.secondary_gap : scope->style.secondary_gap * 1.25f ),
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
    } );

    if ( cfg->title_key ) {
        const char* title_id = vxui_menu__push_child_id( ctx, root_id, "title" );
        Clay__OpenElementWithId( vxui__clay_id_from_hash( vxui_id( title_id ) ) );
        Clay__ConfigureOpenElement( ( Clay_ElementDeclaration ) {
            .layout = {
                .sizing = { CLAY_SIZING_FIT( 0 ), CLAY_SIZING_FIT( 0 ) },
            },
        } );
        vxui_menu__emit_text_leaf( ctx, vxui_id( vxui_menu__push_child_id( ctx, title_id, "text" ) ), cfg->title_key, scope->style.body_font_id, scope->style.secondary_font_size, scope->style.section_text_color );
        Clay__CloseElement();
    }

    for ( int i = 0; i < line_count; ++i ) {
        const char* line_key = cfg->line_keys ? cfg->line_keys[ i ] : nullptr;
        if ( !line_key ) {
            continue;
        }
        const char* line_id = vxui_menu__push_child_index_id( ctx, root_id, "line", i );
        Clay__OpenElementWithId( vxui__clay_id_from_hash( vxui_id( line_id ) ) );
        Clay__ConfigureOpenElement( ( Clay_ElementDeclaration ) {
            .layout = {
                .sizing = { CLAY_SIZING_FIT( 0 ), CLAY_SIZING_FIT( 0 ) },
            },
        } );
        vxui_menu__emit_text_leaf( ctx, vxui_id( vxui_menu__push_child_id( ctx, line_id, "text" ) ), line_key, scope->style.body_font_id, scope->compact ? scope->style.secondary_font_size : scope->style.body_font_size * 0.78f, scope->style.secondary_text_color );
        Clay__CloseElement();
    }

    Clay__CloseElement();
}

void vxui_menu_preview( vxui_ctx* ctx, const char* id, const vxui_menu_preview_cfg* cfg )
{
    vxui_menu__screen_scope* scope = vxui_menu__current_screen_scope();
    if ( !scope || scope->ctx != ctx ) {
        return;
    }

    const vxui_menu_preview_cfg* preview = cfg ? cfg : &scope->cfg.preview;
    if ( !preview || preview->hidden ) {
        return;
    }

    const char* root_id = id ? id : vxui_menu__push_child_id( ctx, scope->id, "preview" );
    const char* body_key = scope->compact && preview->compact_body_key ? preview->compact_body_key : preview->body_key;

    Clay__OpenElementWithId( vxui__clay_id_from_hash( vxui_id( root_id ) ) );
    Clay__ConfigureOpenElement( ( Clay_ElementDeclaration ) {
        .layout = {
            .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
            .childGap = vxui_menu__u16( scope->compact ? scope->style.row_gap * 0.7f : scope->style.row_gap ),
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
    } );

    if ( preview->section_key ) {
        const char* section_id = vxui_menu__push_child_id( ctx, root_id, "section" );
        vxui_menu__emit_text_leaf( ctx, vxui_id( section_id ), preview->section_key, scope->style.body_font_id, scope->style.secondary_font_size, scope->style.section_text_color );
    }

    if ( preview->title_key || preview->badge_text_key ) {
        const char* heading_id = preview->header_id ? preview->header_id : vxui_menu__push_child_id( ctx, root_id, "heading" );
        Clay__OpenElementWithId( vxui__clay_id_from_hash( vxui_id( heading_id ) ) );
        Clay__ConfigureOpenElement( ( Clay_ElementDeclaration ) {
            .layout = {
                .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                .childGap = vxui_menu__u16( scope->style.prompt_gap ),
                .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                .layoutDirection = CLAY_LEFT_TO_RIGHT,
            },
        } );
        if ( preview->title_key ) {
            const char* title_id = vxui_menu__push_child_id( ctx, root_id, "title" );
            vxui_menu__emit_text_leaf( ctx, vxui_id( title_id ), preview->title_key, scope->style.title_font_id, scope->compact ? scope->style.title_font_size * 0.86f : scope->style.title_font_size, scope->style.text_color );
        }
        if ( preview->badge_text_key ) {
            const char* badge_id = vxui_menu__push_child_id( ctx, root_id, "badge" );
            vxui_menu__scope badge_scope = {};
            badge_scope.ctx = ctx;
            badge_scope.style = scope->style;
            vxui_menu__emit_badge_inline( &badge_scope, vxui_id( badge_id ), preview->badge_text_key, nullptr );
        }
        Clay__CloseElement();
    }

    if ( preview->subtitle_key ) {
        const char* subtitle_id = vxui_menu__push_child_id( ctx, root_id, "subtitle" );
        vxui_menu__emit_text_leaf( ctx, vxui_id( subtitle_id ), preview->subtitle_key, scope->style.body_font_id, scope->style.secondary_font_size, scope->style.secondary_text_color );
    }

    if ( body_key ) {
        const char* body_id = preview->body_id ? preview->body_id : vxui_menu__push_child_id( ctx, root_id, "body" );
        Clay__OpenElementWithId( vxui__clay_id_from_hash( vxui_id( body_id ) ) );
        Clay__ConfigureOpenElement( ( Clay_ElementDeclaration ) {
            .layout = {
                .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
        } );
        vxui_menu__emit_text_leaf( ctx, vxui_id( vxui_menu__push_child_id( ctx, body_id, "text" ) ), body_key, scope->style.body_font_id, scope->compact ? scope->style.body_font_size * 0.82f : scope->style.body_font_size * 0.9f, scope->style.text_color );
        Clay__CloseElement();
    }

    if ( preview->help && !preview->help->hidden ) {
        if ( body_key ) {
            CLAY_AUTO_ID( vxui__text_leaf_decl( ( Clay_ElementDeclaration ) {
                .layout = {
                    .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIXED( scope->compact ? 10.0f : 12.0f ) },
                },
            } ) ) {}
        }
        const char* help_id = vxui_menu__push_child_id( ctx, root_id, "help_legend" );
        vxui_menu_help_legend( ctx, help_id, preview->help );
    }

    Clay__CloseElement();
}

void vxui_menu_footer( vxui_ctx* ctx, const char* id, const vxui_menu_footer_cfg* cfg )
{
    vxui_menu__screen_scope* scope = vxui_menu__current_screen_scope();
    if ( !scope || scope->ctx != ctx ) {
        return;
    }

    const vxui_menu_footer_cfg* footer = cfg ? cfg : &scope->cfg.footer;
    if ( !footer || footer->hidden ) {
        return;
    }

    vxui_menu__close_screen_body( scope );

    bool compact = scope->compact;
    if ( footer->compact_mode == VXUI_MENU_SHELL_COMPACT_FORCE_ON ) {
        compact = true;
    } else if ( footer->compact_mode == VXUI_MENU_SHELL_COMPACT_FORCE_OFF ) {
        compact = false;
    }

    int visible_prompt_count = 0;
    for ( int i = 0; i < footer->prompt_item_count; ++i ) {
        const vxui_menu_prompt_item* item = &footer->prompt_items[ i ];
        if ( item->hidden || !item->action_name ) {
            continue;
        }
        visible_prompt_count += 1;
    }

    bool use_status[ VXUI_MENU__MAX_FOOTER_STATUS_ITEMS ] = {};
    int visible_status_count = 0;
    int capped_status_count = footer->status_item_count;
    if ( capped_status_count > VXUI_MENU__MAX_FOOTER_STATUS_ITEMS ) {
        capped_status_count = VXUI_MENU__MAX_FOOTER_STATUS_ITEMS;
    }
    for ( int i = 0; i < capped_status_count; ++i ) {
        const vxui_menu_status_item* item = &footer->status_items[ i ];
        use_status[ i ] = !item->hidden && ( item->label_key || item->value_key );
        visible_status_count += use_status[ i ] ? 1 : 0;
    }

    if ( compact ) {
        int max_visible = footer->compact_max_status_items > 0 ? footer->compact_max_status_items : 2;
        auto drop_matching = [&]( vxui_menu_status_importance importance, bool require_collapse_flag ) {
            for ( int i = capped_status_count - 1; i >= 0 && visible_status_count > max_visible; --i ) {
                const vxui_menu_status_item* item = &footer->status_items[ i ];
                if ( !use_status[ i ] || item->importance != importance ) {
                    continue;
                }
                if ( require_collapse_flag && !item->collapse_in_compact ) {
                    continue;
                }
                use_status[ i ] = false;
                visible_status_count -= 1;
            }
        };
        drop_matching( VXUI_MENU_STATUS_OPTIONAL, false );
        drop_matching( VXUI_MENU_STATUS_SECONDARY, true );
        drop_matching( VXUI_MENU_STATUS_SECONDARY, false );
        drop_matching( VXUI_MENU_STATUS_PRIMARY, true );
    }

    if ( visible_prompt_count <= 0 && visible_status_count <= 0 ) {
        return;
    }

    const char* root_id = id ? id : vxui_menu__push_child_id( ctx, scope->id, "footer" );
    float pad_x = compact ? scope->style.padding_x * 0.8f : scope->style.padding_x;
    float pad_y = compact ? scope->style.padding_y * 0.75f : scope->style.padding_y;

    Clay__OpenElementWithId( vxui__clay_id_from_hash( vxui_id( root_id ) ) );
    Clay__ConfigureOpenElement( ( Clay_ElementDeclaration ) {
        .layout = {
            .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
            .padding = {
                vxui_menu__u16( pad_x ),
                vxui_menu__u16( pad_x ),
                vxui_menu__u16( pad_y ),
                vxui_menu__u16( pad_y ),
            },
            .childGap = vxui_menu__u16( compact ? scope->style.prompt_gap * 0.75f : scope->style.prompt_gap ),
            .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
            .layoutDirection = CLAY_LEFT_TO_RIGHT,
        },
        .backgroundColor = vxui_menu__to_clay_color( vxui_menu__scale_alpha( scope->style.panel_fill_color, compact ? 0.92f : 0.98f ) ),
        .cornerRadius = CLAY_CORNER_RADIUS( scope->style.corner_radius ),
        .border = {
            .color = vxui_menu__to_clay_color( vxui_menu__scale_alpha( scope->style.row_border_color, 0.78f ) ),
            .width = CLAY_BORDER_ALL( 1 ),
        },
    } );

    if ( visible_prompt_count > 0 ) {
        const char* prompts_id = vxui_menu__push_child_id( ctx, root_id, "prompts" );
        Clay__OpenElementWithId( vxui__clay_id_from_hash( vxui_id( prompts_id ) ) );
        Clay__ConfigureOpenElement( ( Clay_ElementDeclaration ) {
            .layout = {
                .sizing = { CLAY_SIZING_FIT( 0 ), CLAY_SIZING_FIT( 0 ) },
                .childGap = vxui_menu__u16( compact ? scope->style.prompt_gap * 0.85f : scope->style.prompt_gap ),
                .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                .layoutDirection = CLAY_LEFT_TO_RIGHT,
            },
        } );
        for ( int i = 0; i < footer->prompt_item_count; ++i ) {
            const vxui_menu_prompt_item* item = &footer->prompt_items[ i ];
            if ( item->hidden || !item->action_name ) {
                continue;
            }
            const char* prompt_id = item->id ? item->id : vxui_menu__push_child_index_id( ctx, prompts_id, "item", i );
            Clay__OpenElementWithId( vxui__clay_id_from_hash( vxui_id( prompt_id ) ) );
            Clay__ConfigureOpenElement( ( Clay_ElementDeclaration ) {
                .layout = {
                    .sizing = { CLAY_SIZING_FIT( 0 ), CLAY_SIZING_FIT( 0 ) },
                    .childGap = vxui_menu__u16( compact ? scope->style.prompt_gap * 0.45f : scope->style.prompt_gap * 0.55f ),
                    .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                    .layoutDirection = CLAY_LEFT_TO_RIGHT,
                },
            } );
            const float previous_prompt_size = ctx->default_font_size;
            const vxui_color previous_prompt_color = ctx->default_text_color;
            ctx->default_font_size = compact ? scope->style.secondary_font_size * 0.95f : scope->style.secondary_font_size;
            ctx->default_text_color = scope->style.prompt_text_color;
            VXUI_PROMPT( ctx, item->action_name );
            ctx->default_font_size = previous_prompt_size;
            ctx->default_text_color = previous_prompt_color;
            if ( item->label_key ) {
                const char* label_id = vxui_menu__push_child_id( ctx, prompt_id, "label" );
                vxui_menu__emit_text_leaf( ctx, vxui_id( label_id ), item->label_key, scope->style.body_font_id, scope->style.secondary_font_size, scope->style.prompt_text_color );
            }
            Clay__CloseElement();
        }
        Clay__CloseElement();
    }

    if ( visible_prompt_count > 0 && visible_status_count > 0 ) {
        const char* spacer_id = vxui_menu__push_child_id( ctx, root_id, "spacer" );
        Clay__OpenElementWithId( vxui__clay_id_from_hash( vxui_id( spacer_id ) ) );
        Clay__ConfigureOpenElement( ( Clay_ElementDeclaration ) {
            .layout = {
                .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
            },
        } );
        Clay__CloseElement();
    }

    if ( visible_status_count > 0 ) {
        const char* status_id = vxui_menu__push_child_id( ctx, root_id, "status" );
        Clay__OpenElementWithId( vxui__clay_id_from_hash( vxui_id( status_id ) ) );
        Clay__ConfigureOpenElement( ( Clay_ElementDeclaration ) {
            .layout = {
                .sizing = { CLAY_SIZING_FIT( 0 ), CLAY_SIZING_FIT( 0 ) },
                .childGap = vxui_menu__u16( compact ? scope->style.prompt_gap : scope->style.prompt_gap * 1.2f ),
                .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                .layoutDirection = CLAY_LEFT_TO_RIGHT,
            },
        } );
        for ( int i = 0; i < capped_status_count; ++i ) {
            const vxui_menu_status_item* item = &footer->status_items[ i ];
            if ( !use_status[ i ] ) {
                continue;
            }
            const char* item_id = item->id ? item->id : vxui_menu__push_child_index_id( ctx, status_id, "item", i );
            Clay__OpenElementWithId( vxui__clay_id_from_hash( vxui_id( item_id ) ) );
            Clay__ConfigureOpenElement( ( Clay_ElementDeclaration ) {
                .layout = {
                    .sizing = { CLAY_SIZING_FIT( 0 ), CLAY_SIZING_FIT( 0 ) },
                    .childGap = vxui_menu__u16( scope->style.secondary_gap ),
                    .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                    .layoutDirection = CLAY_LEFT_TO_RIGHT,
                },
            } );
            if ( item->label_key ) {
                const char* label_id = vxui_menu__push_child_id( ctx, item_id, "label" );
                vxui_menu__emit_text_leaf( ctx, vxui_id( label_id ), item->label_key, scope->style.body_font_id, scope->style.secondary_font_size, scope->style.secondary_text_color );
            }
            if ( item->value_key ) {
                const char* value_id = vxui_menu__push_child_id( ctx, item_id, "value" );
                vxui_menu__emit_text_leaf( ctx, vxui_id( value_id ), item->value_key, scope->style.body_font_id, scope->style.secondary_font_size, scope->style.text_color );
            }
            Clay__CloseElement();
        }
        Clay__CloseElement();
    }

    Clay__CloseElement();
}

bool vxui_menu_screen_is_compact( vxui_ctx* ctx )
{
    vxui_menu__screen_scope* scope = vxui_menu__current_screen_scope();
    return scope && scope->ctx == ctx && scope->compact;
}

#endif
