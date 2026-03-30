#pragma once

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <string>

#include "layout_contract.h"
#include "theme.h"
#include "vxui.h"
#include "vxui_menu.h"

struct vxui_demo_main_menu_preview
{
    const char* row_id;
    const char* title;
    const char* subtitle;
    const char* body;
    const char* badge_key;
};

struct vxui_demo_controls_block_copy
{
    const char* title;
    const char* lines[ 4 ];
};

struct vxui_demo_main_menu_visuals
{
    uint32_t body_font_id;
    uint32_t title_font_id;
    uint32_t section_font_id;
};

struct vxui_demo_main_menu_shell_copy
{
    const char* hero_title;
    const char* hero_banner;
    const char* preview_label_text;
    const char* locale_label;
    const char* locale_value;
    const char* prompts_label;
    const char* prompts_value;
    const char* screens_label;
    int screen_count;
    const char* top_label;
    const char* top_value;
    const char* confirm_label;
    const char* cancel_label;
};

struct vxui_demo_controls_block_visuals
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

inline Clay_String vxui_demo_shared_clay_string( const char* text )
{
    return ( Clay_String ) {
        .isStaticallyAllocated = false,
        .length = text ? ( int32_t ) std::strlen( text ) : 0,
        .chars = text ? text : "",
    };
}

inline constexpr vxui_demo_main_menu_preview VXUI_DEMO_MAIN_MENU_PREVIEWS[] = {
    { "sortie", "Sortie Command", "Deploy a live mission.", "Choose a mission rail and launch.", "badge.recommended" },
    { "loadout", "Loadout Deck", "Tune the frame.", "Swap frame and systems.", "badge.demo" },
    { "archives", "Archives Vault", "Inspect logs.", "Read intel and locked archive entries.", "badge.demo" },
    { "settings", "Settings Grid", "System config.", "Adjust locale, prompts, and rendering.", "badge.recommended" },
    { "records", "Records Relay", "Run debriefs.", "Review past mission summaries.", "badge.demo" },
    { "credits", "Credits Channel", "Stack callouts.", "Read acknowledgements and stack notes.", "badge.demo" },
    { "quit", "Quit Demo", "Exit application.", "Leave the command deck.", nullptr },
};

inline std::string vxui_demo_controls_block_title_id( const char* id )
{
    return std::string( id ) + ".title";
}

inline std::string vxui_demo_controls_block_line_id( const char* id, int index )
{
    return std::string( id ) + ".line." + std::to_string( index );
}

inline bool vxui_demo_shared_locale_matches( const char* locale, const char* prefix )
{
    if ( !locale || !prefix ) {
        return false;
    }
    size_t prefix_length = std::strlen( prefix );
    if ( std::strncmp( locale, prefix, prefix_length ) != 0 ) {
        return false;
    }
    return locale[ prefix_length ] == '\0' || locale[ prefix_length ] == '-' || locale[ prefix_length ] == '_';
}

inline const char* vxui_demo_main_menu_preview_label( const char* locale )
{
    if ( vxui_demo_shared_locale_matches( locale, "ja" ) ) {
        return "注目セクター";
    }
    if ( vxui_demo_shared_locale_matches( locale, "ar" ) ) {
        return "القطاع المحدد";
    }
    return "Focused Sector";
}

inline const char* vxui_demo_badge_text( const char* locale, const char* badge_key )
{
    if ( !badge_key ) {
        return nullptr;
    }
    if ( std::strcmp( badge_key, "badge.recommended" ) == 0 ) {
        if ( vxui_demo_shared_locale_matches( locale, "ja" ) ) {
            return "推奨";
        }
        if ( vxui_demo_shared_locale_matches( locale, "ar" ) ) {
            return "موصى به";
        }
        return "Recommended";
    }
    if ( std::strcmp( badge_key, "badge.demo" ) == 0 ) {
        if ( vxui_demo_shared_locale_matches( locale, "ja" ) ) {
            return "デモ";
        }
        if ( vxui_demo_shared_locale_matches( locale, "ar" ) ) {
            return "عرض";
        }
        return "Demo";
    }
    if ( std::strcmp( badge_key, "badge.selected" ) == 0 ) {
        if ( vxui_demo_shared_locale_matches( locale, "ja" ) ) {
            return "選択中";
        }
        if ( vxui_demo_shared_locale_matches( locale, "ar" ) ) {
            return "محدد";
        }
        return "Selected";
    }
    if ( std::strcmp( badge_key, "badge.locked" ) == 0 ) {
        if ( vxui_demo_shared_locale_matches( locale, "ja" ) ) {
            return "ロック";
        }
        if ( vxui_demo_shared_locale_matches( locale, "ar" ) ) {
            return "مغلق";
        }
        return "Locked";
    }
    return badge_key;
}

inline vxui_demo_controls_block_copy vxui_demo_controls_block_copy_for_locale( const char* locale, bool compact_copy = false )
{
    if ( vxui_demo_shared_locale_matches( locale, "ja" ) ) {
        if ( compact_copy ) {
            return {
                "操作",
                {
                    "移動: 矢印",
                    "決定: Enter",
                    "戻る: Esc",
                    "",
                },
            };
        }
        return {
            "操作",
            {
                "移動: 矢印 / D-pad",
                "決定: Enter / A",
                "戻る: Esc / B",
                "列: [ ]   設定: 1-5",
            },
        };
    }
    if ( vxui_demo_shared_locale_matches( locale, "ar" ) ) {
        if ( compact_copy ) {
            return {
                "التحكم",
                {
                    "الحركة: الأسهم",
                    "التأكيد: Enter",
                    "الرجوع: Esc",
                    "",
                },
            };
        }
        return {
            "التحكم",
            {
                "الحركة: الأسهم",
                "التأكيد: Enter",
                "الرجوع: Esc",
                "الأعمدة: [ ]   الإعداد: 1-5",
            },
        };
    }
    if ( compact_copy ) {
        return {
            "Controls",
            {
                "Move: arrows",
                "Confirm: Enter",
                "Back: Esc",
                "",
            },
        };
    }
    return {
        "Controls",
        {
            "Move: arrows / D-pad",
            "Confirm: Enter / Space / A",
            "Back: Esc / Backspace / B",
            "Lanes: [ ]   Setup: 1-5",
        },
    };
}

inline vxui_menu_style vxui_demo_make_title_deck_menu_style( uint32_t body_font_id, uint32_t title_font_id )
{
    vxui_menu_style style = vxui_menu_style_br_title();
    style.option_wrap_by_default = true;
    style.body_font_id = body_font_id;
    style.title_font_id = title_font_id;
    style.badge_font_id = body_font_id;
    style.body_font_size = 18.0f;
    style.title_font_size = 24.0f;
    style.badge_font_size = 9.0f;
    style.row_height = 30.0f;
    style.row_gap = 1.0f;
    style.section_gap = 8.0f;
    style.padding_x = 16.0f;
    style.padding_y = 8.0f;
    vxui_demo_apply_title_menu_theme( style );
    return style;
}

inline const vxui_demo_main_menu_preview* vxui_demo_main_menu_preview_from_focused_row( uint32_t focused_row_id )
{
    for ( const vxui_demo_main_menu_preview& preview : VXUI_DEMO_MAIN_MENU_PREVIEWS ) {
        if ( focused_row_id == vxui_idi( "main.command_menu", ( int ) vxui_id( preview.row_id ) ) ) {
            return &preview;
        }
    }
    return &VXUI_DEMO_MAIN_MENU_PREVIEWS[ 0 ];
}

inline int vxui_demo_controls_block_visible_line_count( const vxui_demo_controls_block_copy& copy )
{
    int line_count = 0;
    for ( const char* line : copy.lines ) {
        if ( line && line[ 0 ] != '\0' ) {
            line_count += 1;
        }
    }
    return line_count;
}

template <typename TEmitCommandPanel>
inline void vxui_demo_emit_main_menu_shell(
    vxui_ctx* ctx,
    bool rtl,
    const char* locale,
    const vxui_demo_main_menu_layout_spec& layout,
    const vxui_demo_main_menu_visuals& visuals,
    const vxui_demo_main_menu_shell_copy& copy,
    const vxui_demo_main_menu_preview& preview,
    TEmitCommandPanel&& emit_command_panel )
{
    ( void ) rtl;

    const bool compact_vertical = layout.surface_max_height <= 650.0f;
    const bool preview_compact = vxui_demo_main_menu_preview_uses_compact_layout( layout );
    const bool tight_preview_width = layout.preview_panel_width <= 420.0f;
    const bool compact_help = layout.surface_max_height <= 648.0f || tight_preview_width;
    const vxui_demo_controls_block_copy help_copy = vxui_demo_controls_block_copy_for_locale( locale, compact_help );
    const int help_line_count = vxui_demo_controls_block_visible_line_count( help_copy );
    const int compact_help_line_count = tight_preview_width ? std::min( help_line_count, 2 ) : ( compact_help ? std::min( help_line_count, 3 ) : help_line_count );

    vxui_menu_style shell_style = vxui_demo_make_title_deck_menu_style( visuals.body_font_id, visuals.title_font_id );
    shell_style.title_font_id = visuals.title_font_id;
    shell_style.body_font_id = visuals.body_font_id;
    shell_style.badge_font_id = visuals.body_font_id;
    shell_style.row_gap = compact_vertical ? 6.0f : 8.0f;
    shell_style.section_gap = compact_vertical ? 8.0f : 10.0f;
    shell_style.lane_gap = layout.deck_gap;
    shell_style.padding_x = compact_vertical ? 10.0f : 12.0f;
    shell_style.padding_y = compact_vertical ? 6.0f : 8.0f;

    vxui_menu_prompt_item prompt_items[] = {
        { "action.confirm", copy.confirm_label, false, "main.prompt.confirm" },
        { "action.cancel", copy.cancel_label, false, "main.prompt.cancel" },
    };

    const std::string screen_count_text = std::to_string( std::max( 0, copy.screen_count ) );
    vxui_menu_status_item status_items[] = {
        { copy.locale_label, copy.locale_value, VXUI_MENU_STATUS_PRIMARY, false, false, "main.footer.status.locale" },
        { copy.prompts_label, copy.prompts_value, VXUI_MENU_STATUS_SECONDARY, true, false, "main.footer.status.prompts" },
        { copy.screens_label, screen_count_text.c_str(), VXUI_MENU_STATUS_SECONDARY, true, false, "main.footer.status.screens" },
        { copy.top_label, copy.top_value, VXUI_MENU_STATUS_PRIMARY, false, false, "main.footer.status.top" },
    };

    vxui_menu_help_cfg help_cfg = {
        help_copy.title,
        help_copy.lines,
        help_line_count,
        compact_help_line_count,
        false,
    };
    vxui_menu_preview_cfg preview_cfg = {
        copy.preview_label_text,
        preview.title,
        preview.subtitle,
        vxui_demo_badge_text( locale, preview.badge_key ),
        preview.body,
        preview.body,
        preview_compact ? 2 : 3,
        2,
        &help_cfg,
        false,
        "main.preview_header",
        "main.preview_body",
    };
    vxui_menu_footer_cfg footer_cfg = {
        prompt_items,
        ( int ) ( sizeof( prompt_items ) / sizeof( prompt_items[ 0 ] ) ),
        status_items,
        ( int ) ( sizeof( status_items ) / sizeof( status_items[ 0 ] ) ),
        VXUI_MENU_SHELL_COMPACT_AUTO,
        compact_vertical ? 3 : 4,
        false,
    };
    vxui_menu_screen_cfg screen_cfg = {
        VXUI_MENU_SHELL_TITLE_MENU,
        &shell_style,
        VXUI_MENU_SHELL_COMPACT_AUTO,
        648.0f,
        1100.0f,
        false,
        { copy.hero_title, compact_vertical ? nullptr : copy.hero_banner, false },
        { layout.command_panel_width, layout.command_panel_width, false, false, false },
        { layout.preview_panel_width, layout.preview_panel_width, true, false, false },
        { 0.0f, 0.0f, false, true, true },
        preview_cfg,
        footer_cfg,
    };

    vxui_menu_state shell_state = {};
    vxui_menu_screen_begin( ctx, &shell_state, "main.shell", &screen_cfg );
    vxui_menu_header( ctx, "main.hero", &screen_cfg.header );

    vxui_menu_primary_lane_begin( ctx, "main.command_panel", &screen_cfg.primary_lane );
    emit_command_panel( 0.0f );
    vxui_menu_primary_lane_end( ctx );

    vxui_menu_secondary_lane_begin( ctx, "main.preview_panel", &screen_cfg.secondary_lane );
    vxui_menu_preview( ctx, "main.preview", &preview_cfg );
    vxui_menu_secondary_lane_end( ctx );

    vxui_menu_footer( ctx, "main.footer", &footer_cfg );
    vxui_menu_screen_end( ctx, &shell_state );
}
