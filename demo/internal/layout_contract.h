#pragma once

#include <algorithm>
#include <cstring>

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

struct vxui_demo_surface_contract
{
    float content_min_width;
    float content_target_width;
    float content_max_width;
    float desktop_min_surface_width;
    float target_fraction;
};

struct vxui_demo_surface_metrics
{
    float surface_width;
    float content_width;
    float label_lane_width;
    float control_lane_width;
};

enum vxui_demo_viewport_class
{
    VXUI_DEMO_VIEWPORT_PREFERRED_DESKTOP = 0,
    VXUI_DEMO_VIEWPORT_DESKTOP_NORMAL,
    VXUI_DEMO_VIEWPORT_DESKTOP_TIGHT,
    VXUI_DEMO_VIEWPORT_DESKTOP_STRESS_TIGHT,
    VXUI_DEMO_VIEWPORT_UNSUPPORTED,
};

struct vxui_demo_main_menu_contract
{
    float hero_to_deck_gap_min;
    float deck_to_footer_gap_min;
    float command_fraction;
    float command_min_width;
    float command_max_width;
    float preview_min_width;
    float deck_gap;
    float footer_reserve;
    float command_menu_viewport_min_height;
    float preview_panel_padding;
    float preview_header_gap;
    float preview_header_min_height;
    float preview_viewport_min_height;
    float preview_viewport_bottom_guard;
};

struct vxui_demo_controls_block_contract
{
    uint16_t padding;
    uint16_t row_gap;
    float title_font_size;
    float line_font_size;
    float line_gap_min;
    float min_height;
    bool compact_copy;
    uint8_t visible_line_count;
};

struct vxui_demo_split_deck_contract
{
    float deck_to_footer_gap_min;
    float primary_fraction;
    float tertiary_fraction;
    float primary_min_width;
    float primary_max_width;
    float secondary_min_width;
    float tertiary_min_width;
    float tertiary_max_width;
    float deck_gap;
    float footer_reserve;
    float menu_viewport_min_height;
};

struct vxui_demo_settings_contract
{
    float header_to_body_gap_min;
    float body_to_footer_gap_min;
    float body_min_height;
    float footer_reserve;
    float menu_viewport_min_height;
    float menu_viewport_min_width_fraction;
};

struct vxui_demo_scroll_contract
{
    const char* container_id;
    const char* viewport_id;
    const char* content_id;
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

inline bool vxui_demo_main_menu_preview_uses_compact_layout( const vxui_demo_main_menu_layout_spec& layout )
{
    return layout.surface_max_height <= 620.0f;
}

struct vxui_demo_settings_layout_spec
{
    vxui_demo_surface_metrics surface;
    float surface_max_height;
    float footer_reserve;
    float menu_viewport_height;
};

inline bool vxui_demo_locale_needs_wide_label_lane( const char* locale )
{
    return locale && ( std::strncmp( locale, "ja", 2 ) == 0 || std::strncmp( locale, "ar", 2 ) == 0 );
}

inline const char* vxui_demo_root_id( vxui_demo_surface_kind kind )
{
    switch ( kind ) {
        case VXUI_DEMO_SURFACE_BOOT:
            return "boot";
        case VXUI_DEMO_SURFACE_TITLE:
            return "title";
        case VXUI_DEMO_SURFACE_MAIN_MENU:
            return "main_menu";
        case VXUI_DEMO_SURFACE_SORTIE:
            return "sortie";
        case VXUI_DEMO_SURFACE_LOADOUT:
            return "loadout";
        case VXUI_DEMO_SURFACE_ARCHIVES:
            return "archives";
        case VXUI_DEMO_SURFACE_SETTINGS:
            return "settings";
        case VXUI_DEMO_SURFACE_RECORDS:
            return "records";
        case VXUI_DEMO_SURFACE_CREDITS:
            return "credits";
        case VXUI_DEMO_SURFACE_LAUNCH_STUB:
            return "launch_stub";
        case VXUI_DEMO_SURFACE_RESULTS_STUB:
        default:
            return "results_stub";
    }
}

inline const char* vxui_demo_surface_id( vxui_demo_surface_kind kind )
{
    switch ( kind ) {
        case VXUI_DEMO_SURFACE_BOOT:
            return "boot.surface";
        case VXUI_DEMO_SURFACE_TITLE:
            return "title.surface";
        case VXUI_DEMO_SURFACE_MAIN_MENU:
            return "main.surface";
        case VXUI_DEMO_SURFACE_SORTIE:
            return "sortie.surface";
        case VXUI_DEMO_SURFACE_LOADOUT:
            return "loadout.surface";
        case VXUI_DEMO_SURFACE_ARCHIVES:
            return "archives.surface";
        case VXUI_DEMO_SURFACE_SETTINGS:
            return "settings.surface";
        case VXUI_DEMO_SURFACE_RECORDS:
            return "records.surface";
        case VXUI_DEMO_SURFACE_CREDITS:
            return "credits.surface";
        case VXUI_DEMO_SURFACE_LAUNCH_STUB:
            return "launch_stub.surface";
        case VXUI_DEMO_SURFACE_RESULTS_STUB:
        default:
            return "results_stub.surface";
    }
}

inline vxui_demo_surface_kind vxui_demo_surface_kind_from_screen_id( const char* screen_name )
{
    if ( !screen_name ) {
        return VXUI_DEMO_SURFACE_MAIN_MENU;
    }
    if ( std::strcmp( screen_name, "boot" ) == 0 ) {
        return VXUI_DEMO_SURFACE_BOOT;
    }
    if ( std::strcmp( screen_name, "title" ) == 0 ) {
        return VXUI_DEMO_SURFACE_TITLE;
    }
    if ( std::strcmp( screen_name, "main_menu" ) == 0 ) {
        return VXUI_DEMO_SURFACE_MAIN_MENU;
    }
    if ( std::strcmp( screen_name, "sortie" ) == 0 ) {
        return VXUI_DEMO_SURFACE_SORTIE;
    }
    if ( std::strcmp( screen_name, "loadout" ) == 0 ) {
        return VXUI_DEMO_SURFACE_LOADOUT;
    }
    if ( std::strcmp( screen_name, "archives" ) == 0 ) {
        return VXUI_DEMO_SURFACE_ARCHIVES;
    }
    if ( std::strcmp( screen_name, "settings" ) == 0 ) {
        return VXUI_DEMO_SURFACE_SETTINGS;
    }
    if ( std::strcmp( screen_name, "records" ) == 0 ) {
        return VXUI_DEMO_SURFACE_RECORDS;
    }
    if ( std::strcmp( screen_name, "credits" ) == 0 ) {
        return VXUI_DEMO_SURFACE_CREDITS;
    }
    if ( std::strcmp( screen_name, "launch_stub" ) == 0 ) {
        return VXUI_DEMO_SURFACE_LAUNCH_STUB;
    }
    if ( std::strcmp( screen_name, "results_stub" ) == 0 ) {
        return VXUI_DEMO_SURFACE_RESULTS_STUB;
    }
    return VXUI_DEMO_SURFACE_MAIN_MENU;
}

inline const char* vxui_demo_surface_id_for_screen( const char* screen_name )
{
    return vxui_demo_surface_id( vxui_demo_surface_kind_from_screen_id( screen_name ) );
}

inline const char* vxui_demo_enter_sequence_name_for_screen( const char* screen_name )
{
    if ( !screen_name ) {
        return nullptr;
    }
    if ( std::strcmp( screen_name, "boot" ) == 0 ) {
        return "boot_enter";
    }
    if ( std::strcmp( screen_name, "title" ) == 0 ) {
        return "title_enter";
    }
    if ( std::strcmp( screen_name, "main_menu" ) == 0 ) {
        return "main_menu_enter";
    }
    if ( std::strcmp( screen_name, "sortie" ) == 0 ) {
        return "sortie_enter";
    }
    if ( std::strcmp( screen_name, "loadout" ) == 0 ) {
        return "loadout_enter";
    }
    if ( std::strcmp( screen_name, "archives" ) == 0 ) {
        return "archives_enter";
    }
    if ( std::strcmp( screen_name, "settings" ) == 0 ) {
        return "settings_enter";
    }
    if ( std::strcmp( screen_name, "records" ) == 0 ) {
        return "records_enter";
    }
    if ( std::strcmp( screen_name, "credits" ) == 0 ) {
        return "credits_enter";
    }
    if ( std::strcmp( screen_name, "launch_stub" ) == 0 ) {
        return "launch_stub_enter";
    }
    if ( std::strcmp( screen_name, "results_stub" ) == 0 ) {
        return "results_stub_enter";
    }
    return nullptr;
}

inline vxui_demo_surface_contract vxui_demo_get_surface_contract( vxui_demo_surface_kind kind )
{
    switch ( kind ) {
        case VXUI_DEMO_SURFACE_SORTIE:
        case VXUI_DEMO_SURFACE_LOADOUT:
        case VXUI_DEMO_SURFACE_ARCHIVES:
        case VXUI_DEMO_SURFACE_SETTINGS:
        case VXUI_DEMO_SURFACE_RECORDS:
        case VXUI_DEMO_SURFACE_CREDITS:
            return {
                820.0f,
                960.0f,
                1080.0f,
                860.0f,
                0.78f,
            };

        case VXUI_DEMO_SURFACE_MAIN_MENU:
            return {
                720.0f,
                800.0f,
                860.0f,
                720.0f,
                0.64f,
            };

        case VXUI_DEMO_SURFACE_BOOT:
        case VXUI_DEMO_SURFACE_TITLE:
        case VXUI_DEMO_SURFACE_LAUNCH_STUB:
        case VXUI_DEMO_SURFACE_RESULTS_STUB:
        default:
            return {
                720.0f,
                840.0f,
                920.0f,
                760.0f,
                0.70f,
            };
    }
}

inline vxui_demo_viewport_class vxui_demo_classify_viewport( float viewport_width, float viewport_height )
{
    if ( viewport_width >= 1600.0f && viewport_height >= 900.0f ) {
        return VXUI_DEMO_VIEWPORT_PREFERRED_DESKTOP;
    }
    if ( viewport_width >= 1280.0f && viewport_height >= 640.0f ) {
        return VXUI_DEMO_VIEWPORT_DESKTOP_NORMAL;
    }
    if ( viewport_width >= 1100.0f && viewport_height >= 640.0f ) {
        return VXUI_DEMO_VIEWPORT_DESKTOP_TIGHT;
    }
    if ( viewport_width >= 960.0f && viewport_height >= 720.0f ) {
        return VXUI_DEMO_VIEWPORT_DESKTOP_STRESS_TIGHT;
    }
    return VXUI_DEMO_VIEWPORT_UNSUPPORTED;
}

inline bool vxui_demo_is_supported_viewport( float viewport_width, float viewport_height )
{
    return vxui_demo_classify_viewport( viewport_width, viewport_height ) != VXUI_DEMO_VIEWPORT_UNSUPPORTED;
}

inline vxui_demo_main_menu_contract vxui_demo_get_main_menu_contract( void )
{
    return {
        8.0f,
        8.0f,
        0.325f,
        248.0f,
        256.0f,
        320.0f,
        16.0f,
        188.0f,
        260.0f,
        10.0f,
        10.0f,
        84.0f,
        132.0f,
        8.0f,
    };
}

inline vxui_demo_split_deck_contract vxui_demo_get_split_deck_contract( vxui_demo_surface_kind kind )
{
    switch ( kind ) {
        case VXUI_DEMO_SURFACE_SORTIE:
            return {
                8.0f,
                0.43f,
                0.09f,
                352.0f,
                420.0f,
                280.0f,
                132.0f,
                164.0f,
                16.0f,
                240.0f,
                240.0f,
            };

        case VXUI_DEMO_SURFACE_LOADOUT:
        case VXUI_DEMO_SURFACE_ARCHIVES:
        case VXUI_DEMO_SURFACE_RECORDS:
        default:
            return {
                8.0f,
                0.38f,
                0.0f,
                300.0f,
                360.0f,
                280.0f,
                0.0f,
                0.0f,
                18.0f,
                250.0f,
                240.0f,
            };
    }
}

inline vxui_demo_settings_contract vxui_demo_get_settings_contract( void )
{
    return {
        8.0f,
        8.0f,
        180.0f,
        250.0f,
        220.0f,
        0.60f,
    };
}

inline vxui_demo_controls_block_contract vxui_demo_get_controls_block_contract( float surface_max_height, float owner_width = 0.0f )
{
    const bool compact_height = surface_max_height <= 650.0f;
    const bool compact_width = owner_width > 0.0f && owner_width <= 520.0f;
    if ( compact_height || compact_width ) {
        return {
            5,
            6,
            12.0f,
            9.4f,
            2.0f,
            104.0f,
            true,
            1,
        };
    }
    return {
        10,
        6,
        15.0f,
        10.8f,
        3.0f,
        132.0f,
        false,
        1,
    };
}

inline vxui_demo_scroll_contract vxui_demo_get_scroll_contract( vxui_demo_surface_kind kind )
{
    switch ( kind ) {
        case VXUI_DEMO_SURFACE_MAIN_MENU:
            return {
                "main.preview_panel",
                "main.preview_body",
                "main.preview_body",
            };

        case VXUI_DEMO_SURFACE_SETTINGS:
            return {
                "settings.body_panel",
                "settings.body_menu",
                "settings.body_menu",
            };

        default:
            return { nullptr, nullptr, nullptr };
    }
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

inline float vxui_demo_compute_surface_width( float viewport_width, vxui_demo_surface_kind kind )
{
    vxui_demo_surface_contract contract = vxui_demo_get_surface_contract( kind );
    float preferred = viewport_width * contract.target_fraction;
    preferred = viewport_width >= contract.content_target_width
        ? std::max( preferred, contract.content_target_width )
        : std::min( preferred, contract.content_target_width );
    preferred = std::clamp( preferred, contract.content_min_width, contract.content_max_width );
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
    const vxui_demo_main_menu_contract contract = vxui_demo_get_main_menu_contract();
    const float surface_max_height = std::max( 0.0f, viewport_height );
    const bool tight_width = surface.content_width <= 560.0f;
    const bool stress_width = surface.content_width <= 500.0f;
    const float command_fraction = tight_width ? 0.36f : contract.command_fraction;
    const float command_min_width = stress_width ? 250.0f : tight_width ? 270.0f : contract.command_min_width;
    const float command_max_width = tight_width ? 320.0f : contract.command_max_width;
    const float deck_gap = tight_width ? 14.0f : contract.deck_gap;
    const float preview_min_width = tight_width ? 340.0f : contract.preview_min_width;
    const float command_panel_width =
        std::clamp( surface.content_width * command_fraction, command_min_width, command_max_width );
    const float preview_panel_width =
        std::max( preview_min_width, surface.content_width - command_panel_width - deck_gap );
    const float footer_reserve_target = surface_max_height <= 620.0f
        ? 250.0f
        : surface_max_height <= 650.0f ? 244.0f
        : std::max( 192.0f, surface_max_height * 0.35f );
    const float footer_reserve = std::min( contract.footer_reserve, footer_reserve_target );
    const float hero_reserve = surface_max_height <= 620.0f ? 68.0f : surface_max_height <= 720.0f ? 56.0f : 92.0f;
    const float deck_height_floor = surface_max_height <= 650.0f ? 280.0f : 420.0f;
    const float deck_height = std::max( deck_height_floor, surface_max_height - footer_reserve - hero_reserve );
    const float preview_panel_padding = tight_width ? 10.0f : contract.preview_panel_padding;
    const float preview_panel_gap = surface_max_height <= 650.0f ? 8.0f : 12.0f;
    const float preview_header_min_height =
        std::clamp( preview_panel_width * 0.18f, tight_width ? 78.0f : contract.preview_header_min_height, 136.0f );
    const float preview_header_gap = tight_width ? 8.0f : contract.preview_header_gap;
    const float preview_viewport_bottom_guard = tight_width ? 4.0f : contract.preview_viewport_bottom_guard;
    const float controls_owner_width = std::max( 0.0f, preview_panel_width - preview_panel_padding * 2.0f );
    const vxui_demo_controls_block_contract controls_contract =
        vxui_demo_get_controls_block_contract( surface_max_height <= 620.0f ? 620.0f : surface_max_height, controls_owner_width );
    const float help_reserve = controls_contract.min_height > 0.0f ? controls_contract.min_height + preview_panel_gap : 0.0f;
    const float preview_viewport_height = std::max(
        contract.preview_viewport_min_height,
        deck_height - preview_panel_padding * 2.0f - help_reserve );
    const float preview_body_viewport_height = std::max(
        contract.preview_viewport_min_height,
        preview_viewport_height - preview_header_min_height - preview_header_gap );

    return {
        surface,
        surface_max_height,
        command_panel_width,
        preview_panel_width,
        deck_gap,
        deck_height,
        footer_reserve,
        std::max( contract.command_menu_viewport_min_height, deck_height ),
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
    const vxui_demo_split_deck_contract contract = vxui_demo_get_split_deck_contract( kind );
    const float surface_max_height = std::max( 0.0f, viewport_height );
    const float primary_lane_width =
        std::clamp( surface.content_width * contract.primary_fraction, contract.primary_min_width, contract.primary_max_width );
    const float tertiary_lane_width = contract.tertiary_fraction > 0.0f
        ? std::clamp( surface.content_width * contract.tertiary_fraction, contract.tertiary_min_width, contract.tertiary_max_width )
        : 0.0f;
    const float secondary_lane_width = std::max(
        contract.secondary_min_width,
        surface.content_width - primary_lane_width - tertiary_lane_width - contract.deck_gap * ( tertiary_lane_width > 0.0f ? 2.0f : 1.0f ) );
    const float footer_reserve_target = surface_max_height <= 650.0f
        ? 236.0f
        : surface_max_height <= 690.0f ? 216.0f
        : std::max( 176.0f, surface_max_height * 0.31f );
    const float footer_reserve = std::min( contract.footer_reserve, footer_reserve_target );
    const float usable_deck_height = std::max( 0.0f, surface_max_height - footer_reserve );

    return {
        surface,
        surface_max_height,
        primary_lane_width,
        secondary_lane_width,
        tertiary_lane_width,
        contract.deck_gap,
        footer_reserve,
        std::max( contract.menu_viewport_min_height, usable_deck_height - 16.0f ),
    };
}

inline vxui_demo_settings_layout_spec vxui_demo_resolve_settings_layout(
    float viewport_width,
    float viewport_height,
    const char* locale )
{
    const vxui_demo_surface_metrics surface = vxui_demo_compute_surface_metrics( viewport_width, locale, VXUI_DEMO_SURFACE_SETTINGS );
    const vxui_demo_settings_contract contract = vxui_demo_get_settings_contract();
    const float surface_max_height = std::max( 0.0f, viewport_height );
    const float footer_reserve = std::min( contract.footer_reserve, std::max( 180.0f, surface_max_height * 0.35f ) );
    const float header_reserve = surface_max_height <= 720.0f ? 86.0f : 102.0f;
    const float menu_viewport_height = std::max( contract.menu_viewport_min_height, surface_max_height - footer_reserve - header_reserve );
    return {
        surface,
        surface_max_height,
        footer_reserve,
        menu_viewport_height,
    };
}
