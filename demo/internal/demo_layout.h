#pragma once


#include <algorithm>
#include <cstdint>
#include <cstring>
#include "../../vxui.h"
#include "../../vxui_menu.h"

inline constexpr float VXUI_DEMO_LAYOUT_OUTER_PADDING = 16.0f;
inline constexpr float VXUI_DEMO_LAYOUT_SURFACE_PADDING_X = 24.0f;
inline constexpr float VXUI_DEMO_LAYOUT_SURFACE_PADDING_Y = 18.0f;
inline constexpr float VXUI_DEMO_LAYOUT_SECTION_GAP = 14.0f;
inline constexpr float VXUI_DEMO_LAYOUT_ROW_GAP = 10.0f;
inline constexpr float VXUI_DEMO_LAYOUT_INLINE_GAP = 8.0f;

enum vxui_demo_surface_kind
{
    VXUI_DEMO_SURFACE_BOOT = 0,
    VXUI_DEMO_SURFACE_TITLE,
    VXUI_DEMO_SURFACE_MAIN_MENU,
    VXUI_DEMO_SURFACE_SORTIE,
    VXUI_DEMO_SURFACE_LOADOUT,
    VXUI_DEMO_SURFACE_ARCHIVES,
    VXUI_DEMO_SURFACE_SETTINGS,
    VXUI_DEMO_SURFACE_RECORDS,
    VXUI_DEMO_SURFACE_CREDITS,
    VXUI_DEMO_SURFACE_LAUNCH_STUB,
    VXUI_DEMO_SURFACE_RESULTS_STUB,
};

struct vxui_demo_screen_table_entry
{
    const char* screen_name;
    const char* surface_id;
    const char* enter_sequence_name;
};

inline constexpr vxui_demo_screen_table_entry VXUI_DEMO_SCREEN_TABLE[] = {
    { "boot", "boot.surface", "boot_enter" },
    { "title", "title.surface", "title_enter" },
    { "main_menu", "main.surface", "main_menu_enter" },
    { "sortie", "sortie.surface", "sortie_enter" },
    { "loadout", "loadout.surface", "loadout_enter" },
    { "archives", "archives.surface", "archives_enter" },
    { "settings", "settings.surface", "settings_enter" },
    { "records", "records.surface", "records_enter" },
    { "credits", "credits.surface", "credits_enter" },
    { "launch_stub", "launch_stub.surface", "launch_stub_enter" },
    { "results_stub", "results_stub.surface", "results_stub_enter" },
};

inline constexpr int VXUI_DEMO_SCREEN_TABLE_COUNT =
    ( int ) ( sizeof( VXUI_DEMO_SCREEN_TABLE ) / sizeof( VXUI_DEMO_SCREEN_TABLE[ 0 ] ) );

inline const vxui_demo_screen_table_entry& vxui_demo_screen_table_entry_for_kind( vxui_demo_surface_kind kind )
{
    const int index = ( kind >= VXUI_DEMO_SURFACE_BOOT && kind <= VXUI_DEMO_SURFACE_RESULTS_STUB )
        ? ( int ) kind
        : ( int ) VXUI_DEMO_SURFACE_MAIN_MENU;
    return VXUI_DEMO_SCREEN_TABLE[ index ];
}

inline const vxui_demo_screen_table_entry* vxui_demo_screen_table_entry_for_name( const char* screen_name )
{
    if ( !screen_name ) {
        return nullptr;
    }
    for ( int i = 0; i < VXUI_DEMO_SCREEN_TABLE_COUNT; ++i ) {
        if ( std::strcmp( VXUI_DEMO_SCREEN_TABLE[ i ].screen_name, screen_name ) == 0 ) {
            return &VXUI_DEMO_SCREEN_TABLE[ i ];
        }
    }
    return nullptr;
}

struct vxui_demo_surface_metrics
{
    float surface_width;
    float content_width;
    float label_lane_width;
    float control_lane_width;
};

struct vxui_demo_main_menu_layout_spec
{
    vxui_demo_surface_metrics surface;
    float surface_max_height;
    float command_panel_width;
    float preview_panel_width;
    float deck_gap;
    float deck_height;
    float footer_reserve;
    float command_menu_viewport_height;
    float preview_panel_padding;
    float preview_viewport_height;
    float preview_header_min_height;
    float preview_header_gap;
    float preview_body_viewport_height;
    float preview_viewport_bottom_guard;
};

struct vxui_demo_main_menu_type_scale
{
    float hero_uplink_size;
    float hero_title_size;
    float hero_banner_size;
    float hero_clock_size;
    float hero_sync_size;
    float command_label_size;
    float command_badge_size;
    float command_row_height;
    uint16_t command_row_gap;
    uint16_t command_panel_gap;
    float preview_eyebrow_size;
    float preview_title_size;
    float preview_badge_size;
    float preview_bloom_size;
    float preview_subtitle_size;
    float preview_body_size;
    float preview_warning_size;
    float preview_stat_heading_size;
    float preview_stat_title_size;
    float preview_stat_label_size;
    float preview_stat_value_size;
    float footer_key_size;
    float footer_action_size;
    float footer_meta_size;
};

struct vxui_demo_split_deck_layout_spec
{
    vxui_demo_surface_metrics surface;
    float surface_max_height;
    float primary_lane_width;
    float secondary_lane_width;
    float tertiary_lane_width;
    float deck_gap;
    float footer_reserve;
    float menu_viewport_height;
};

struct vxui_demo_settings_layout_spec
{
    vxui_demo_surface_metrics surface;
    float surface_max_height;
    float footer_reserve;
    float menu_viewport_height;
};

inline bool vxui_demo_main_menu_preview_uses_compact_layout( const vxui_demo_main_menu_layout_spec& layout )
{
    return layout.surface_max_height <= 620.0f;
}

inline vxui_demo_main_menu_type_scale vxui_demo_get_main_menu_type_scale( const vxui_demo_main_menu_layout_spec& layout )
{
    const bool compact_vertical = layout.surface_max_height <= 650.0f;
    const bool tight_preview_width = layout.preview_panel_width <= 420.0f;
    return {
        compact_vertical ? 11.0f : 14.0f,
        compact_vertical ? 38.0f : 45.0f,
        compact_vertical ? 11.0f : 14.0f,
        compact_vertical ? 18.0f : 22.0f,
        compact_vertical ? 9.0f : 11.0f,
        compact_vertical ? 13.0f : 15.0f,
        compact_vertical ? 7.0f : 9.0f,
        compact_vertical ? 27.0f : 36.0f,
        compact_vertical ? uint16_t{ 2 } : uint16_t{ 4 },
        compact_vertical ? uint16_t{ 5 } : uint16_t{ 4 },
        compact_vertical ? 11.0f : 14.0f,
        compact_vertical ? 32.0f : tight_preview_width ? 38.0f : 42.0f,
        compact_vertical ? 9.0f : 11.0f,
        compact_vertical ? 8.0f : 10.0f,
        compact_vertical ? 13.0f : 15.0f,
        compact_vertical ? 12.0f : 14.0f,
        compact_vertical ? 9.0f : 11.0f,
        compact_vertical ? 9.0f : 11.0f,
        compact_vertical ? 13.0f : 15.0f,
        compact_vertical ? 8.0f : 10.0f,
        compact_vertical ? 9.0f : 11.0f,
        compact_vertical ? 9.0f : 11.0f,
        compact_vertical ? 8.0f : 10.0f,
        compact_vertical ? 7.0f : 9.0f,
    };
}

inline float vxui_demo_main_menu_preview_void_height( const vxui_demo_main_menu_layout_spec& layout )
{
    return layout.surface_max_height <= 650.0f ? 8.0f : 36.0f;
}

inline bool vxui_demo_locale_needs_wide_label_lane( const char* locale )
{
    return locale && ( std::strncmp( locale, "ja", 2 ) == 0 || std::strncmp( locale, "ar", 2 ) == 0 );
}

inline const char* vxui_demo_root_id( vxui_demo_surface_kind kind )
{
    return vxui_demo_screen_table_entry_for_kind( kind ).screen_name;
}

inline const char* vxui_demo_surface_id( vxui_demo_surface_kind kind )
{
    return vxui_demo_screen_table_entry_for_kind( kind ).surface_id;
}

inline vxui_demo_surface_kind vxui_demo_surface_kind_from_screen_id( const char* screen_name )
{
    const vxui_demo_screen_table_entry* entry = vxui_demo_screen_table_entry_for_name( screen_name );
    if ( !entry ) {
        return VXUI_DEMO_SURFACE_MAIN_MENU;
    }
    return ( vxui_demo_surface_kind ) ( entry - VXUI_DEMO_SCREEN_TABLE );
}

inline const char* vxui_demo_surface_id_for_screen( const char* screen_name )
{
    const vxui_demo_screen_table_entry* entry = vxui_demo_screen_table_entry_for_name( screen_name );
    return entry ? entry->surface_id : vxui_demo_surface_id( VXUI_DEMO_SURFACE_MAIN_MENU );
}

inline const char* vxui_demo_enter_sequence_name_for_screen( const char* screen_name )
{
    const vxui_demo_screen_table_entry* entry = vxui_demo_screen_table_entry_for_name( screen_name );
    return entry ? entry->enter_sequence_name : nullptr;
}

inline constexpr const char* VXUI_DEMO_DEFAULT_WATCHED_SEQUENCE_NAME = "main_menu_enter";

inline constexpr const char* VXUI_DEMO_FRONTEND_ENTER_SEQUENCE_NAMES[] = {
    "boot_enter",
    "title_enter",
    "main_menu_enter",
    "sortie_enter",
    "loadout_enter",
    "archives_enter",
    "settings_enter",
    "records_enter",
    "credits_enter",
    "launch_stub_enter",
    "results_stub_enter",
};

inline constexpr int VXUI_DEMO_FRONTEND_ENTER_SEQUENCE_COUNT =
    ( int ) ( sizeof( VXUI_DEMO_FRONTEND_ENTER_SEQUENCE_NAMES ) / sizeof( VXUI_DEMO_FRONTEND_ENTER_SEQUENCE_NAMES[ 0 ] ) );

inline float vxui_demo_surface_desktop_min_width( vxui_demo_surface_kind kind )
{
    switch ( kind ) {
        case VXUI_DEMO_SURFACE_SORTIE:
        case VXUI_DEMO_SURFACE_LOADOUT:
        case VXUI_DEMO_SURFACE_ARCHIVES:
        case VXUI_DEMO_SURFACE_SETTINGS:
        case VXUI_DEMO_SURFACE_RECORDS:
        case VXUI_DEMO_SURFACE_CREDITS:
            return 860.0f;

        case VXUI_DEMO_SURFACE_MAIN_MENU:
            return 720.0f;

        case VXUI_DEMO_SURFACE_BOOT:
        case VXUI_DEMO_SURFACE_TITLE:
        case VXUI_DEMO_SURFACE_LAUNCH_STUB:
        case VXUI_DEMO_SURFACE_RESULTS_STUB:
        default:
            return 760.0f;
    }
}

inline constexpr float VXUI_DEMO_SETTINGS_MENU_VIEWPORT_MIN_WIDTH_FRACTION = 0.60f;

inline float vxui_demo_compute_surface_width( float viewport_width, vxui_demo_surface_kind kind )
{
    float content_min_width = 720.0f;
    float content_target_width = 840.0f;
    float content_max_width = 920.0f;
    float target_fraction = 0.70f;

    switch ( kind ) {
        case VXUI_DEMO_SURFACE_SORTIE:
        case VXUI_DEMO_SURFACE_LOADOUT:
        case VXUI_DEMO_SURFACE_ARCHIVES:
        case VXUI_DEMO_SURFACE_SETTINGS:
        case VXUI_DEMO_SURFACE_RECORDS:
        case VXUI_DEMO_SURFACE_CREDITS:
            content_min_width = 820.0f;
            content_target_width = 960.0f;
            content_max_width = 1080.0f;
            target_fraction = 0.78f;
            break;

        case VXUI_DEMO_SURFACE_MAIN_MENU:
            content_min_width = 720.0f;
            content_target_width = 880.0f;
            content_max_width = 940.0f;
            target_fraction = 0.70f;
            break;

        case VXUI_DEMO_SURFACE_BOOT:
        case VXUI_DEMO_SURFACE_TITLE:
        case VXUI_DEMO_SURFACE_LAUNCH_STUB:
        case VXUI_DEMO_SURFACE_RESULTS_STUB:
        default:
            break;
    }

    float preferred = viewport_width * target_fraction;
    preferred = viewport_width >= content_target_width
        ? std::max( preferred, content_target_width )
        : std::min( preferred, content_target_width );
    preferred = std::clamp( preferred, content_min_width, content_max_width );
    return std::min( preferred, viewport_width );
}

inline vxui_demo_surface_metrics vxui_demo_compute_surface_metrics(
    float viewport_width,
    const char* locale,
    vxui_demo_surface_kind kind,
    float surface_padding_x = VXUI_DEMO_LAYOUT_SURFACE_PADDING_X,
    float control_gap = VXUI_DEMO_LAYOUT_ROW_GAP )
{
    float surface_width = vxui_demo_compute_surface_width( viewport_width, kind );
    float content_width = std::max( 0.0f, surface_width - surface_padding_x * 2.0f );
    float label_min_width = vxui_demo_locale_needs_wide_label_lane( locale ) ? 180.0f : 160.0f;
    float label_lane_width = std::clamp( content_width * 0.24f, label_min_width, 220.0f );
    float control_lane_width = std::max( 0.0f, content_width - label_lane_width - control_gap );
    return {
        surface_width,
        content_width,
        label_lane_width,
        control_lane_width,
    };
}

inline vxui_demo_main_menu_layout_spec vxui_demo_resolve_main_menu_layout(
    float viewport_width,
    float viewport_height,
    const char* locale )
{
    const vxui_demo_surface_metrics surface = vxui_demo_compute_surface_metrics( viewport_width, locale, VXUI_DEMO_SURFACE_MAIN_MENU );
    const float surface_max_height = std::max( 0.0f, viewport_height );
    const bool tight_width = surface.content_width <= 560.0f;
    const bool stress_width = surface.content_width <= 500.0f;
    const float command_fraction = tight_width ? 0.36f : 0.325f;
    const float command_min_width = stress_width ? 250.0f : tight_width ? 270.0f : 270.0f;
    const float command_max_width = tight_width ? 320.0f : 280.0f;
    const float deck_gap = tight_width ? 14.0f : 16.0f;
    const float preview_min_width = tight_width ? 340.0f : 360.0f;
    const float command_panel_width =
        std::clamp( surface.content_width * command_fraction, command_min_width, command_max_width );
    const float preview_panel_width =
        std::max( preview_min_width, surface.content_width - command_panel_width - deck_gap );
    const float footer_reserve_target = surface_max_height <= 620.0f
        ? 250.0f
        : surface_max_height <= 650.0f ? 244.0f
        : std::max( 184.0f, surface_max_height * 0.33f );
    const float footer_reserve = std::min( 208.0f, footer_reserve_target );
    const float hero_reserve = surface_max_height <= 620.0f ? 68.0f : surface_max_height <= 720.0f ? 52.0f : 86.0f;
    const float deck_height_floor = surface_max_height <= 650.0f ? 280.0f : 420.0f;
    const float deck_height = std::max( deck_height_floor, surface_max_height - footer_reserve - hero_reserve );
    const float preview_panel_padding = tight_width ? 10.0f : 12.0f;
    const float preview_panel_gap = surface_max_height <= 650.0f ? 8.0f : 12.0f;
    const float preview_header_min_height =
        std::clamp( preview_panel_width * 0.18f, tight_width ? 78.0f : 84.0f, 136.0f );
    const float preview_header_gap = tight_width ? 8.0f : 10.0f;
    const float preview_viewport_bottom_guard = tight_width ? 4.0f : 8.0f;
    const float controls_owner_width = std::max( 0.0f, preview_panel_width - preview_panel_padding * 2.0f );
    const bool compact_controls = surface_max_height <= 650.0f
        || ( controls_owner_width > 0.0f && controls_owner_width <= 520.0f );
    const float controls_min_height = compact_controls ? 104.0f : 176.0f;
    const float help_reserve = controls_min_height > 0.0f ? controls_min_height + preview_panel_gap : 0.0f;
    const float preview_viewport_height = std::max( 132.0f, deck_height - preview_panel_padding * 2.0f - help_reserve );
    const float preview_body_viewport_height = std::max(
        132.0f,
        preview_viewport_height - preview_header_min_height - preview_header_gap );

    return {
        surface,
        surface_max_height,
        command_panel_width,
        preview_panel_width,
        deck_gap,
        deck_height,
        footer_reserve,
        std::max( 260.0f, deck_height ),
        preview_panel_padding,
        preview_viewport_height,
        preview_header_min_height,
        preview_header_gap,
        preview_body_viewport_height,
        preview_viewport_bottom_guard,
    };
}

inline vxui_demo_split_deck_layout_spec vxui_demo_resolve_split_deck_layout(
    vxui_demo_surface_kind kind,
    float viewport_width,
    float viewport_height,
    const char* locale )
{
    const vxui_demo_surface_metrics surface = vxui_demo_compute_surface_metrics( viewport_width, locale, kind );
    const float surface_max_height = std::max( 0.0f, viewport_height );
    float primary_fraction = 0.38f;
    float tertiary_fraction = 0.0f;
    float primary_min_width = 300.0f;
    float primary_max_width = 360.0f;
    float secondary_min_width = 280.0f;
    float tertiary_min_width = 0.0f;
    float tertiary_max_width = 0.0f;
    float deck_gap = 18.0f;
    float footer_reserve_limit = 250.0f;
    float menu_viewport_min_height = 240.0f;

    if ( kind == VXUI_DEMO_SURFACE_SORTIE ) {
        primary_fraction = 0.43f;
        tertiary_fraction = 0.09f;
        primary_min_width = 352.0f;
        primary_max_width = 420.0f;
        tertiary_min_width = 132.0f;
        tertiary_max_width = 164.0f;
        deck_gap = 16.0f;
        footer_reserve_limit = 240.0f;
    }

    const float primary_lane_width =
        std::clamp( surface.content_width * primary_fraction, primary_min_width, primary_max_width );
    const float tertiary_lane_width = tertiary_fraction > 0.0f
        ? std::clamp( surface.content_width * tertiary_fraction, tertiary_min_width, tertiary_max_width )
        : 0.0f;
    const float secondary_lane_width = std::max(
        secondary_min_width,
        surface.content_width - primary_lane_width - tertiary_lane_width - deck_gap * ( tertiary_lane_width > 0.0f ? 2.0f : 1.0f ) );
    const float footer_reserve_target = surface_max_height <= 650.0f
        ? 236.0f
        : surface_max_height <= 690.0f ? 216.0f
        : std::max( 176.0f, surface_max_height * 0.31f );
    const float footer_reserve = std::min( footer_reserve_limit, footer_reserve_target );
    const float usable_deck_height = std::max( 0.0f, surface_max_height - footer_reserve );

    return {
        surface,
        surface_max_height,
        primary_lane_width,
        secondary_lane_width,
        tertiary_lane_width,
        deck_gap,
        footer_reserve,
        std::max( menu_viewport_min_height, usable_deck_height - 16.0f ),
    };
}

inline vxui_demo_settings_layout_spec vxui_demo_resolve_settings_layout(
    float viewport_width,
    float viewport_height,
    const char* locale )
{
    const vxui_demo_surface_metrics surface = vxui_demo_compute_surface_metrics( viewport_width, locale, VXUI_DEMO_SURFACE_SETTINGS );
    const float surface_max_height = std::max( 0.0f, viewport_height );
    const float footer_reserve = std::min( 250.0f, std::max( 180.0f, surface_max_height * 0.35f ) );
    const float header_reserve = surface_max_height <= 720.0f ? 86.0f : 102.0f;
    const float menu_viewport_height = std::max( 220.0f, surface_max_height - footer_reserve - header_reserve );
    return {
        surface,
        surface_max_height,
        footer_reserve,
        menu_viewport_height,
    };
}

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

inline bool vxui_demo_locale_prefix_matches( const char* locale, const char* prefix )
{
    if ( !locale || !prefix ) {
        return false;
    }
    const size_t prefix_length = std::strlen( prefix );
    if ( std::strncmp( locale, prefix, prefix_length ) != 0 ) {
        return false;
    }
    return locale[ prefix_length ] == '\0' || locale[ prefix_length ] == '-' || locale[ prefix_length ] == '_';
}

inline const char* vxui_demo_locale_text( const char* locale, const char* en_text, const char* ja_text, const char* ar_text )
{
    if ( vxui_demo_locale_prefix_matches( locale, "ja" ) ) {
        return ja_text;
    }
    if ( vxui_demo_locale_prefix_matches( locale, "ar" ) ) {
        return ar_text;
    }
    return en_text;
}
