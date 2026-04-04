#pragma once

#include "../../vxui.h"
#include "../../vxui_menu.h"

struct vxui_demo_command_deck_theme
{
    vxui_color app_background_base;
    vxui_color app_background_accent;
    vxui_color hero_surface_fill;
    vxui_color hero_surface_border;
    vxui_color primary_panel_fill;
    vxui_color primary_panel_border;
    vxui_color secondary_panel_fill;
    vxui_color secondary_panel_border;
    vxui_color utility_fill;
    vxui_color utility_border;
    vxui_color focused_row_fill;
    vxui_color focused_row_border;
    vxui_color passive_row_fill;
    vxui_color passive_row_border;
    vxui_color badge_fill;
    vxui_color badge_text;
    vxui_color title_text;
    vxui_color section_text;
    vxui_color body_text;
    vxui_color muted_text;
    vxui_color utility_text;
    vxui_color accent_cool;
    vxui_color warning_text;
    vxui_color success_text;
    vxui_color alert_text;
    vxui_color action_fill;
    vxui_color action_border;
    vxui_color action_text;
    vxui_color stat_track;
    vxui_color stat_fill;
    vxui_color disabled_fill;
    vxui_color disabled_text;
    float surface_scanline_alpha;
    float title_focus_decor_alpha;
    float form_focus_decor_alpha;
};

inline const vxui_demo_command_deck_theme& vxui_demo_command_deck_theme_tokens()
{
    static constexpr vxui_demo_command_deck_theme theme = {
        .app_background_base = { 3, 7, 18, 255 },
        .app_background_accent = { 0, 240, 255, 18 },
        .hero_surface_fill = { 15, 23, 42, 96 },
        .hero_surface_border = { 8, 145, 178, 110 },
        .primary_panel_fill = { 10, 16, 28, 126 },
        .primary_panel_border = { 8, 145, 178, 128 },
        .secondary_panel_fill = { 10, 17, 30, 134 },
        .secondary_panel_border = { 8, 145, 178, 110 },
        .utility_fill = { 8, 16, 30, 150 },
        .utility_border = { 8, 145, 178, 110 },
        .focused_row_fill = { 8, 63, 78, 196 },
        .focused_row_border = { 34, 211, 238, 255 },
        .passive_row_fill = { 0, 0, 0, 0 },
        .passive_row_border = { 0, 0, 0, 0 },
        .badge_fill = { 120, 53, 15, 128 },
        .badge_text = { 251, 191, 36, 255 },
        .title_text = { 255, 255, 255, 255 },
        .section_text = { 34, 211, 238, 255 },
        .body_text = { 210, 222, 238, 255 },
        .muted_text = { 110, 140, 170, 255 },
        .utility_text = { 240, 248, 255, 255 },
        .accent_cool = { 34, 211, 238, 255 },
        .warning_text = { 255, 200, 80, 255 },
        .success_text = { 100, 230, 180, 255 },
        .alert_text = { 255, 80, 80, 255 },
        .action_fill = { 0, 20, 40, 220 },
        .action_border = { 0, 200, 240, 200 },
        .action_text = { 0, 220, 255, 255 },
        .stat_track = { 10, 20, 36, 255 },
        .stat_fill = { 0, 220, 180, 255 },
        .disabled_fill = { 10, 15, 25, 180 },
        .disabled_text = { 80, 100, 125, 255 },
        .surface_scanline_alpha = 0.020f,
        .title_focus_decor_alpha = 0.0f,
        .form_focus_decor_alpha = 0.0f,
    };
    return theme;
}

inline Clay_Color vxui_demo_clay_color( vxui_color color )
{
    return ( Clay_Color ) {
        ( float ) color.r,
        ( float ) color.g,
        ( float ) color.b,
        ( float ) color.a,
    };
}

inline Clay_BorderElementConfig vxui_demo_panel_border( vxui_color color, uint16_t width = 1 )
{
    return ( Clay_BorderElementConfig ) {
        .color = vxui_demo_clay_color( color ),
        .width = CLAY_BORDER_ALL( width ),
    };
}

inline void vxui_demo_make_surface_cfg_transparent( vxui_menu_surface_cfg& surface_cfg )
{
    surface_cfg.corner_radius = 0.1f;
    surface_cfg.border_width = 1.0f;
    surface_cfg.background_fill_color = { 0, 0, 0, 0 };
    surface_cfg.surface_fill_color = { 0, 0, 0, 0 };
    surface_cfg.surface_border_color = { 0, 0, 0, 0 };
}

inline vxui_label_cfg vxui_demo_text_style( uint32_t font_id, float font_size, vxui_color color )
{
    vxui_label_cfg cfg = {};
    cfg.font_id = font_id;
    cfg.font_size = font_size;
    cfg.color = color;
    return cfg;
}

inline vxui_value_cfg vxui_demo_value_style( uint32_t font_id, float font_size, vxui_color color, const char* format = nullptr )
{
    vxui_value_cfg cfg = {};
    cfg.font_id = font_id;
    cfg.font_size = font_size;
    cfg.color = color;
    cfg.format = format;
    return cfg;
}

inline void vxui_demo_apply_title_menu_theme( vxui_menu_style& style )
{
    const vxui_demo_command_deck_theme& theme = vxui_demo_command_deck_theme_tokens();
    style.corner_radius = 0.1f;
    style.border_width = 1.0f;
    style.badge_padding_x = 6.0f;
    style.badge_padding_y = 2.0f;
    style.body_letter_spacing = 1;
    style.title_letter_spacing = 2;
    style.focus_decor = VXUI_MENU_FOCUS_DECOR_NONE;
    style.focus_decor_padding = 0.0f;
    style.focus_decor_alpha = 0.0f;
    style.panel_fill_color = theme.primary_panel_fill;
    style.row_fill_color = theme.passive_row_fill;
    style.row_focus_fill_color = theme.focused_row_fill;
    style.row_disabled_fill_color = theme.disabled_fill;
    style.row_border_color = theme.passive_row_border;
    style.row_focus_border_color = theme.focused_row_border;
    style.text_color = theme.body_text;
    style.focused_text_color = theme.title_text;
    style.disabled_text_color = theme.disabled_text;
    style.secondary_text_color = theme.muted_text;
    style.section_text_color = theme.section_text;
    style.badge_fill_color = theme.badge_fill;
    style.badge_text_color = theme.badge_text;
    style.prompt_fill_color = theme.action_fill;
    style.prompt_text_color = theme.action_text;
    style.arrow_color = theme.focused_row_border;
    style.slider_track_color = theme.stat_track;
    style.slider_fill_color = theme.stat_fill;
}

inline void vxui_demo_apply_form_menu_theme( vxui_menu_style& style )
{
    const vxui_demo_command_deck_theme& theme = vxui_demo_command_deck_theme_tokens();
    style.corner_radius = 0.1f;
    style.border_width = 1.0f;
    style.badge_padding_x = 6.0f;
    style.badge_padding_y = 2.0f;
    style.body_letter_spacing = 1;
    style.title_letter_spacing = 2;
    style.focus_decor = VXUI_MENU_FOCUS_DECOR_NONE;
    style.focus_decor_padding = 0.0f;
    style.focus_decor_alpha = 0.0f;
    style.panel_fill_color = theme.primary_panel_fill;
    style.row_fill_color = theme.passive_row_fill;
    style.row_focus_fill_color = theme.focused_row_fill;
    style.row_disabled_fill_color = theme.disabled_fill;
    style.row_border_color = theme.passive_row_border;
    style.row_focus_border_color = theme.focused_row_border;
    style.text_color = theme.body_text;
    style.focused_text_color = theme.utility_text;
    style.disabled_text_color = theme.disabled_text;
    style.secondary_text_color = theme.muted_text;
    style.section_text_color = theme.section_text;
    style.badge_fill_color = theme.badge_fill;
    style.badge_text_color = theme.badge_text;
    style.prompt_fill_color = theme.action_fill;
    style.prompt_text_color = theme.action_text;
    style.arrow_color = theme.focused_row_border;
    style.slider_track_color = theme.stat_track;
    style.slider_fill_color = theme.stat_fill;
}
