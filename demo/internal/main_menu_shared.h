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
    const char* eyebrow;
    const char* title;
    const char* subtitle;
    const char* body;
    const char* body_line_2;
    const char* badge_key;
    const char* warning;
    const char* stat_heading;
    const char* stat_title;
    const char* stat_labels[ 3 ];
    const char* stat_values[ 3 ];
};

struct vxui_demo_controls_block_copy
{
    const char* title;
    const char* lines[ 5 ];
};

struct vxui_demo_main_menu_visuals
{
    uint32_t body_font_id;
    uint32_t title_font_id;
    uint32_t mono_font_id;
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

inline vxui_demo_main_menu_type_scale vxui_demo_get_main_menu_type_scale( const vxui_demo_main_menu_layout_spec& layout )
{
    const bool compact_vertical = layout.surface_max_height <= 650.0f;
    const bool tight_preview_width = layout.preview_panel_width <= 420.0f;
    return {
        compact_vertical ? 11.0f : 13.0f,
        compact_vertical ? 38.0f : 43.0f,
        compact_vertical ? 11.0f : 13.0f,
        compact_vertical ? 18.0f : 20.0f,
        compact_vertical ? 9.0f : 10.0f,
        compact_vertical ? 13.0f : 14.0f,
        compact_vertical ? 7.0f : 8.0f,
        compact_vertical ? 27.0f : 32.0f,
        compact_vertical ? uint16_t{ 2 } : uint16_t{ 4 },
        compact_vertical ? uint16_t{ 5 } : uint16_t{ 4 },
        compact_vertical ? 11.0f : 13.0f,
        compact_vertical ? 32.0f : tight_preview_width ? 36.0f : 40.0f,
        compact_vertical ? 9.0f : 10.0f,
        compact_vertical ? 8.0f : 9.0f,
        compact_vertical ? 13.0f : 14.0f,
        compact_vertical ? 12.0f : 13.0f,
        compact_vertical ? 9.0f : 10.0f,
        compact_vertical ? 9.0f : 10.0f,
        compact_vertical ? 13.0f : 14.0f,
        compact_vertical ? 8.0f : 9.0f,
        compact_vertical ? 9.0f : 10.0f,
        compact_vertical ? 9.0f : 10.0f,
        compact_vertical ? 8.0f : 9.0f,
        compact_vertical ? 7.0f : 8.0f,
    };
}

inline constexpr vxui_demo_main_menu_preview VXUI_DEMO_MAIN_MENU_PREVIEWS[] = {
    { "sortie", "OPERATIONAL THEATER", "AEGIS BREAK", "OBJECTIVE: BREACH THE CARRIER SCREEN.", "Cut through interceptor rings, keep the convoy beacon alive,", "then open the escape lane.", "THREAT TIER 02", "WARNING: DENSE CROSSFIRE NEAR THE AFT GATE.", "ACTIVE CHASSIS", "VF-17 KESTREL", { "MAIN SHOT", "SUB SHOT", "BOMB / SURGE" }, { "BURST RAIL", "HOMING SWARM", "GRAVITY WELL" } },
    { "loadout", "FRAME CONFIGURATION", "LOADOUT DECK", "TUNE THE NEXT PASS.", "Swap frame, primary, support, and system load before launch.", nullptr, "badge.demo", nullptr, nullptr, nullptr, { nullptr, nullptr, nullptr }, { nullptr, nullptr, nullptr } },
    { "archives", "SIGNAL ARCHIVE", "ARCHIVES VAULT", "INSPECT LOCKED INTEL.", "Review comms fragments, frame notes, and recovered operation logs.", nullptr, "badge.demo", nullptr, nullptr, nullptr, { nullptr, nullptr, nullptr }, { nullptr, nullptr, nullptr } },
    { "settings", "SYSTEM GRID", "SETTINGS GRID", "PATCH LOCAL UI.", "Adjust locale, input prompts, scanline treatment, and renderer behaviour.", nullptr, "badge.recommended", nullptr, nullptr, nullptr, { nullptr, nullptr, nullptr }, { nullptr, nullptr, nullptr } },
    { "records", "RUN HISTORY", "RECORDS RELAY", "REVIEW PREVIOUS SORTIES.", "Check scoreboards, route notes, and debrief summaries.", nullptr, "badge.demo", nullptr, nullptr, nullptr, { nullptr, nullptr, nullptr }, { nullptr, nullptr, nullptr } },
    { "credits", "STACK NOTES", "CREDITS CHANNEL", "TRACE THE BUILD.", "Read runtime acknowledgements and front-end implementation notes.", nullptr, "badge.demo", nullptr, nullptr, nullptr, { nullptr, nullptr, nullptr }, { nullptr, nullptr, nullptr } },
    { "quit", "SESSION EXIT", "QUIT DEMO", "DROP TO DESKTOP.", "Leave the command deck and close the sample front end.", nullptr, nullptr, nullptr, nullptr, nullptr, { nullptr, nullptr, nullptr }, { nullptr, nullptr, nullptr } },
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
        return "REC";
    }
    if ( std::strcmp( badge_key, "badge.demo" ) == 0 ) {
        if ( vxui_demo_shared_locale_matches( locale, "ja" ) ) {
            return "デモ";
        }
        if ( vxui_demo_shared_locale_matches( locale, "ar" ) ) {
            return "عرض";
        }
        return "DEMO";
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
                    nullptr,
                    nullptr,
                },
            };
        }
        return {
            "操作",
            {
                "移動: 矢印 / D-pad",
                "列: [ ]",
                "決定: Enter / A",
                "設定: 1-5",
                "戻る: Esc / B",
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
                    nullptr,
                    nullptr,
                },
            };
        }
        return {
            "التحكم",
            {
                "الحركة: الأسهم",
                "المسارات: [ ]",
                "التأكيد: Enter",
                "الإعداد: 1-5",
                "الرجوع: Esc",
            },
        };
    }
    if ( compact_copy ) {
        return {
            "Controls",
            {
                "Navigation: arrows",
                "Confirm: Enter",
                "Back/Cancel: Esc",
                nullptr,
                nullptr,
            },
        };
    }
    return {
        "Controls",
        {
            "Navigation: arrows / D-pad",
            "Lanes: [ ]",
            "Confirm: Enter / Space / A",
            "Setup: 1-5",
            "Back/Cancel: Esc / Backspace / B",
        },
    };
}

inline vxui_menu_style vxui_demo_make_title_deck_menu_style( uint32_t body_font_id, uint32_t title_font_id )
{
    vxui_menu_style style = vxui_menu_style_title_menu();
    style.option_wrap_by_default = true;
    style.body_font_id = body_font_id;
    style.title_font_id = title_font_id;
    style.badge_font_id = body_font_id;
    vxui_demo_apply_title_menu_theme( style );
    style.badge_padding_x = 4.0f;
    style.badge_padding_y = 2.0f;
    return style;
}

inline int vxui_demo_main_menu_preview_stat_count( const vxui_demo_main_menu_preview& preview )
{
    int count = 0;
    for ( int i = 0; i < 3; ++i ) {
        if ( preview.stat_labels[ i ] && preview.stat_values[ i ] ) {
            count += 1;
        }
    }
    return count;
}

inline void vxui_demo_emit_main_menu_preview_badge(
    vxui_ctx* ctx,
    const char* id,
    uint32_t font_id,
    float font_size,
    const vxui_demo_command_deck_theme& theme,
    const char* text )
{
    if ( !text || text[ 0 ] == '\0' ) {
        return;
    }

    const vxui_color threat_fill = { 66, 18, 24, 72 };
    const vxui_color threat_border = { 220, 38, 38, 110 };
    const vxui_color threat_text = { 248, 113, 113, 255 };

    VXUI_HASH( ctx, vxui_id( id ), {
        .layout = {
            .sizing = { CLAY_SIZING_FIT( 0 ), CLAY_SIZING_FIT( 0 ) },
            .padding = { 5, 6, 2, 2 },
            .childGap = 5,
            .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER },
            .layoutDirection = CLAY_LEFT_TO_RIGHT,
        },
        .backgroundColor = vxui_demo_clay_color( threat_fill ),
        .cornerRadius = CLAY_CORNER_RADIUS( 0 ),
        .border = vxui_demo_panel_border( threat_border, 1 ),
    } ) {
        VXUI_HASH( ctx, vxui_idi( id, 1 ), {
            .layout = {
                .sizing = { CLAY_SIZING_FIXED( 4.0f ), CLAY_SIZING_FIXED( 4.0f ) },
            },
            .backgroundColor = vxui_demo_clay_color( threat_text ),
            .cornerRadius = CLAY_CORNER_RADIUS( 0 ),
        } ) {}

        vxui_label_cfg badge_style = vxui_demo_text_style( font_id, font_size, threat_text );
        badge_style.letter_spacing = 1;
        VXUI_LABEL( ctx, text, badge_style );
    }
}

inline void vxui_demo_emit_main_menu_divider( vxui_ctx* ctx, const char* id, vxui_color color )
{
    VXUI_HASH( ctx, vxui_id( id ), {
        .layout = {
            .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIXED( 1.0f ) },
        },
        .backgroundColor = vxui_demo_clay_color( color ),
    } ) {}
}

inline uint32_t vxui_demo_main_menu_row_id( const char* row_id )
{
    return row_id ? vxui_idi( "main.command_menu", ( int ) vxui_id( row_id ) ) : 0u;
}

inline const vxui_demo_main_menu_preview* vxui_demo_try_main_menu_preview_from_focused_row( uint32_t focused_row_id )
{
    for ( const vxui_demo_main_menu_preview& preview : VXUI_DEMO_MAIN_MENU_PREVIEWS ) {
        if ( focused_row_id == vxui_demo_main_menu_row_id( preview.row_id ) ) {
            return &preview;
        }
    }
    return nullptr;
}

inline void vxui_demo_emit_main_menu_command_row(
    vxui_ctx* ctx,
    const char* row_key,
    const char* label_key,
    const char* badge_key,
    vxui_action_fn fn,
    vxui_action_cfg action_cfg,
    uint32_t label_font_id,
    uint32_t badge_font_id,
    float font_size,
    float badge_font_size,
    float row_height,
    const vxui_demo_command_deck_theme& theme )
{
    if ( !ctx || !row_key || !label_key ) {
        return;
    }

    const uint32_t row_id = vxui_demo_main_menu_row_id( row_key );
    action_cfg.no_focus_ring = true;
    vxui__register_action( ctx, row_id, fn, action_cfg );
    vxui__get_anim_state( ctx, row_id, true );
    ctx->current_decl_id = row_id;

    const bool focused = ctx->focused_id == row_id && !action_cfg.disabled;
    vxui_color row_fill = focused ? theme.focused_row_fill : theme.secondary_panel_fill;
    vxui_color label_color = focused ? theme.title_text : theme.muted_text;
    Clay_BorderElementConfig row_border = {
        .color = vxui_demo_clay_color( { 0, 0, 0, 0 } ),
        .width = { 0, 0, 0, 0, 0 },
    };
    vxui_color badge_fill = theme.badge_fill;
    vxui_color badge_text = theme.badge_text;
    Clay_BorderElementConfig badge_border = {};
    if ( !focused ) {
        row_fill = theme.secondary_panel_fill;
        row_fill.a = 158;
        label_color.a = 220;
        row_border.color = vxui_demo_clay_color( theme.secondary_panel_border );
        row_border.width = { 1, 1, 1, 1, 0 };
    } else {
        row_fill.a = 214;
        row_border.color = vxui_demo_clay_color( theme.focused_row_border );
        row_border.width = { 4, 1, 1, 1, 0 };
    }
    if ( badge_key && std::strcmp( badge_key, "badge.demo" ) == 0 ) {
        badge_fill = { 30, 41, 59, 176 };
        badge_text = { 115, 131, 156, 255 };
        badge_border = vxui_demo_panel_border( { 55, 65, 84, 110 }, 1 );
    } else if ( badge_key && std::strcmp( badge_key, "badge.recommended" ) == 0 ) {
        badge_fill = { 120, 53, 15, 136 };
        badge_text = theme.badge_text;
        badge_border = vxui_demo_panel_border( { 146, 64, 14, 120 }, 1 );
    }
    const char* resolved_label = vxui__resolve_text( ctx, label_key );

    VXUI_HASH( ctx, row_id, {
        .layout = {
            .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIXED( row_height ) },
            .padding = { 8, 14, 0, 0 },
            .childGap = 6,
            .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
            .layoutDirection = CLAY_LEFT_TO_RIGHT,
        },
        .backgroundColor = vxui_demo_clay_color( row_fill ),
        .cornerRadius = CLAY_CORNER_RADIUS( 0 ),
        .border = row_border,
    } ) {
        VXUI_HASH( ctx, vxui_idi( row_key, 101 ), {
            .layout = {
                .sizing = { CLAY_SIZING_FIXED( 8.0f ), CLAY_SIZING_FIT( 0 ) },
                .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER },
            },
        } ) {
            if ( focused ) {
                vxui_label_cfg chevron_style = vxui_demo_text_style( label_font_id, font_size * 0.82f, theme.section_text );
                chevron_style.letter_spacing = 1;
                VXUI_LABEL( ctx, ">", chevron_style );
            }
        }

        VXUI_HASH( ctx, vxui_idi( row_key, 102 ), {
            .layout = {
                .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
            },
        } ) {
            vxui_label_cfg label_style = vxui_demo_text_style( label_font_id, font_size, label_color );
            label_style.letter_spacing = 1;
            VXUI_LABEL( ctx, resolved_label ? resolved_label : label_key, label_style );
        }

        if ( badge_key && badge_key[ 0 ] != '\0' ) {
            VXUI_HASH( ctx, vxui_idi( row_key, 103 ), {
                .layout = {
                    .sizing = { CLAY_SIZING_FIT( 0 ), CLAY_SIZING_FIT( 0 ) },
                    .padding = { 3, 6, 1, 1 },
                    .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER },
                },
                .backgroundColor = vxui_demo_clay_color( badge_fill ),
                .cornerRadius = CLAY_CORNER_RADIUS( 0 ),
                .border = badge_border,
            } ) {
                vxui_label_cfg badge_style = vxui_demo_text_style( badge_font_id, badge_font_size, badge_text );
                badge_style.letter_spacing = 1;
                VXUI_LABEL( ctx, vxui_demo_badge_text( ctx->locale, badge_key ), badge_style );
            }
        }
    }
}

inline void vxui_demo_emit_main_menu_controls_block(
    vxui_ctx* ctx,
    const char* id,
    const vxui_demo_controls_block_copy& copy,
    uint32_t micro_font_id,
    uint32_t title_font_id,
    const vxui_demo_command_deck_theme& theme,
    float title_font_size,
    float line_font_size,
    uint16_t padding,
    uint16_t row_gap,
    float min_height )
{
    const char* title = copy.title;
    if ( title && std::strcmp( title, "Controls" ) == 0 ) {
        title = "INPUT TELEMETRY";
    }
    const vxui_color panel_fill = { 1, 4, 10, 236 };
    const vxui_color panel_border = { 32, 42, 58, 178 };
    const vxui_color row_divider = { 36, 46, 58, 144 };
    const vxui_color chip_fill = { 30, 41, 59, 188 };
    const vxui_color chip_border = { 55, 65, 84, 104 };
    const vxui_color label_text = { 102, 112, 128, 255 };
    const vxui_color ghost_text = { 58, 71, 88, 40 };
    const vxui_color accent_chip_fill = { 69, 26, 3, 220 };
    const vxui_color accent_chip_border = { 146, 64, 14, 110 };

    auto parse_line = []( const char* line, std::string& label, std::string& value ) {
        label.clear();
        value.clear();
        if ( !line || line[ 0 ] == '\0' ) {
            return false;
        }
        if ( const char* colon = std::strchr( line, ':' ) ) {
            label.assign( line, ( size_t ) ( colon - line ) );
            const char* value_start = colon + 1;
            while ( *value_start == ' ' ) {
                value_start += 1;
            }
            value = value_start;
        } else {
            value = line;
        }
        return !label.empty() || !value.empty();
    };

    auto emit_item = [&]( const std::string& item_id, const std::string& label, const std::string& value, bool accent_value, bool grow ) {
        VXUI_HASH( ctx, vxui_id( item_id.c_str() ), {
            .layout = {
                .sizing = { grow ? CLAY_SIZING_GROW( 0 ) : CLAY_SIZING_FIT( 0 ), CLAY_SIZING_FIT( 0 ) },
                .padding = { 0, 0, 0, 3 },
                .childGap = 8,
                .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                .layoutDirection = CLAY_LEFT_TO_RIGHT,
            },
            .border = {
                .color = vxui_demo_clay_color( row_divider ),
                .width = { 0, 0, 0, 1, 0 },
            },
        } ) {
            if ( !label.empty() ) {
                VXUI_LABEL( ctx, label.c_str(), vxui_demo_text_style( micro_font_id, line_font_size * 0.76f, label_text ) );
            }

            VXUI_HASH( ctx, vxui_id( ( item_id + ".spacer" ).c_str() ), {
                .layout = {
                    .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_GROW( 0 ) },
                },
            } ) {}

            if ( !value.empty() ) {
                VXUI_HASH( ctx, vxui_id( ( item_id + ".value" ).c_str() ), {
                    .layout = {
                        .sizing = { CLAY_SIZING_FIT( 0 ), CLAY_SIZING_FIT( 0 ) },
                        .padding = { 3, 3, 1, 1 },
                        .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER },
                    },
                    .backgroundColor = vxui_demo_clay_color( accent_value ? accent_chip_fill : chip_fill ),
                    .cornerRadius = CLAY_CORNER_RADIUS( 0 ),
                    .border = vxui_demo_panel_border( accent_value ? accent_chip_border : chip_border, 1 ),
                } ) {
                    VXUI_LABEL( ctx, value.c_str(), vxui_demo_text_style( micro_font_id, line_font_size * ( accent_value ? 0.82f : 0.78f ), accent_value ? theme.badge_text : theme.body_text ) );
                }
            }
        }
    };
    auto is_confirm_label = []( const std::string& label ) {
        return label == "Confirm" || label == "決定" || label == "التأكيد";
    };

    VXUI_HASH( ctx, vxui_id( id ), {
        .layout = {
            .sizing = { CLAY_SIZING_GROW( 0 ), min_height > 0.0f ? CLAY_SIZING_FIT( min_height ) : CLAY_SIZING_FIT( 0 ) },
            .padding = { padding, padding, padding, padding },
            .childGap = row_gap,
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
        .backgroundColor = vxui_demo_clay_color( panel_fill ),
        .cornerRadius = CLAY_CORNER_RADIUS( 0 ),
        .border = vxui_demo_panel_border( panel_border, 1 ),
    } ) {
        VXUI_HASH( ctx, vxui_id( ( std::string( id ) + ".header" ).c_str() ), {
            .layout = {
                .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                .childGap = 8,
                .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                .layoutDirection = CLAY_LEFT_TO_RIGHT,
            },
        } ) {
            if ( title && title[ 0 ] != '\0' ) {
                vxui_label_cfg title_style = vxui_demo_text_style( title_font_id, title_font_size, theme.section_text );
                title_style.letter_spacing = 1;
                VXUI_LABEL( ctx, title, title_style );
            }

            VXUI_HASH( ctx, vxui_id( ( std::string( id ) + ".header.spacer" ).c_str() ), {
                .layout = {
                    .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_GROW( 0 ) },
                },
            } ) {}

            vxui_label_cfg ghost_style = vxui_demo_text_style( title_font_id, title_font_size * ( min_height >= 132.0f ? 3.0f : 1.8f ), ghost_text );
            ghost_style.letter_spacing = 0;
            VXUI_LABEL( ctx, "LAUNCH", ghost_style );
        }

        std::string labels[ 5 ];
        std::string values[ 5 ];
        bool visible[ 5 ] = {};
        bool accent[ 5 ] = {};
        int visible_count = 0;
        for ( int i = 0; i < 5; ++i ) {
            visible[ i ] = parse_line( copy.lines[ i ], labels[ i ], values[ i ] );
            accent[ i ] = visible[ i ] && is_confirm_label( labels[ i ] );
            visible_count += visible[ i ] ? 1 : 0;
        }

        const bool compact_layout = visible_count <= 3 || values[ 3 ].empty();
        if ( compact_layout ) {
            for ( int i = 0; i < 5; ++i ) {
                if ( !visible[ i ] ) {
                    continue;
                }
                const std::string compact_row_id = std::string( id ) + ".compact." + std::to_string( i );
                VXUI_HASH( ctx, vxui_id( compact_row_id.c_str() ), {
                    .layout = {
                        .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                        .layoutDirection = CLAY_LEFT_TO_RIGHT,
                    },
                } ) {
                    emit_item( compact_row_id + ".item", labels[ i ], values[ i ], accent[ i ], true );
                }
            }
        } else {
            for ( int row = 0; row < 2; ++row ) {
                const int left_index = row * 2;
                const int right_index = left_index + 1;
                const std::string row_id = std::string( id ) + ".grid." + std::to_string( row );
                VXUI_HASH( ctx, vxui_id( row_id.c_str() ), {
                    .layout = {
                        .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                        .childGap = 12,
                        .layoutDirection = CLAY_LEFT_TO_RIGHT,
                    },
                } ) {
                    if ( visible[ left_index ] ) {
                        emit_item( row_id + ".left", labels[ left_index ], values[ left_index ], accent[ left_index ], true );
                    }
                    if ( visible[ right_index ] ) {
                        emit_item( row_id + ".right", labels[ right_index ], values[ right_index ], accent[ right_index ], true );
                    }
                }
            }

            if ( visible[ 4 ] ) {
                emit_item( std::string( id ) + ".full", labels[ 4 ], values[ 4 ], false, true );
            }
        }
    }
}

inline const char* vxui_demo_main_menu_clock_text( void )
{
    return "17:48:54.65";
}

inline void vxui_demo_emit_main_menu_header( vxui_ctx* ctx, const vxui_demo_main_menu_visuals& visuals, const vxui_demo_command_deck_theme& theme, const vxui_demo_main_menu_shell_copy& copy )
{
    const vxui_demo_main_menu_type_scale scale = vxui_demo_get_main_menu_type_scale(
        vxui_demo_resolve_main_menu_layout(
            std::max( 0.0f, ( float ) ctx->cfg.screen_width - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f ),
            std::max( 0.0f, ( float ) ctx->cfg.screen_height - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f ),
            ctx ? ctx->locale : nullptr ) );
    VXUI( ctx, "main.hero.custom", {
        .layout = {
            .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
            .padding = { 6, 0, 0, 0 },
            .childGap = 6,
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
    } ) {
        VXUI( ctx, "main.hero.row", {
            .layout = {
                .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                .childGap = 16,
                .childAlignment = { .y = CLAY_ALIGN_Y_BOTTOM },
                .layoutDirection = CLAY_LEFT_TO_RIGHT,
            },
        } ) {
            VXUI( ctx, "main.hero.left", {
                .layout = {
                    .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                    .childGap = 4,
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                },
            } ) {
                VXUI( ctx, "main.hero.eyebrow", {
                    .layout = {
                        .sizing = { CLAY_SIZING_FIT( 0 ), CLAY_SIZING_FIT( 0 ) },
                        .childGap = 6,
                        .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                        .layoutDirection = CLAY_LEFT_TO_RIGHT,
                    },
                    } ) {
                    CLAY_AUTO_ID( {
                        .layout = {
                            .sizing = { CLAY_SIZING_FIXED( 6.0f ), CLAY_SIZING_FIXED( 6.0f ) },
                        },
                        .backgroundColor = vxui_demo_clay_color( theme.badge_text ),
                    } ) {}
                    VXUI_LABEL( ctx, "SYS.UPLINK.SECURE", vxui_demo_text_style( visuals.mono_font_id, scale.hero_uplink_size, theme.badge_text ) );
                }

                VXUI_LABEL( ctx, copy.hero_title, vxui_demo_text_style( visuals.title_font_id, scale.hero_title_size, theme.title_text ) );
                VXUI_LABEL( ctx, "Production-shape front-end stub // v2.0.4", vxui_demo_text_style( visuals.mono_font_id, scale.hero_banner_size, theme.accent_cool ) );
            }

            VXUI( ctx, "main.hero.spacer", {
                .layout = {
                    .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_GROW( 0 ) },
                },
            } ) {}

            VXUI( ctx, "main.hero.right", {
                .layout = {
                    .sizing = { CLAY_SIZING_FIT( 0 ), CLAY_SIZING_FIT( 0 ) },
                    .childGap = 2,
                    .childAlignment = { .x = CLAY_ALIGN_X_RIGHT },
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                },
            } ) {
                VXUI_LABEL( ctx, vxui_demo_main_menu_clock_text(), vxui_demo_text_style( visuals.mono_font_id, scale.hero_clock_size, theme.section_text ) );
                VXUI_LABEL( ctx, "[ TACTICAL FRAME SYNC ]", vxui_demo_text_style( visuals.mono_font_id, scale.hero_sync_size, theme.muted_text ) );
            }
        }

        vxui_demo_emit_main_menu_divider( ctx, "main.hero.divider", theme.primary_panel_border );
    }
}

inline void vxui_demo_emit_main_menu_footer( vxui_ctx* ctx, const vxui_demo_main_menu_visuals& visuals, const vxui_demo_command_deck_theme& theme, float command_panel_width, float deck_gap )
{
    const vxui_demo_main_menu_type_scale scale = vxui_demo_get_main_menu_type_scale(
        vxui_demo_resolve_main_menu_layout(
            std::max( 0.0f, ( float ) ctx->cfg.screen_width - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f ),
            std::max( 0.0f, ( float ) ctx->cfg.screen_height - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f ),
            ctx ? ctx->locale : nullptr ) );
    VXUI( ctx, "main.footer.custom", {
        .layout = {
            .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
            .padding = { 2, 0, 0, 0 },
            .childGap = 3,
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
    } ) {
        vxui_demo_emit_main_menu_divider( ctx, "main.footer.divider", theme.primary_panel_border );

        VXUI( ctx, "main.footer.row", {
            .layout = {
                .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                .padding = { 1, 1, 2, 0 },
                .childGap = 0,
                .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                .layoutDirection = CLAY_LEFT_TO_RIGHT,
            },
        } ) {
            VXUI( ctx, "main.footer.left_lane", {
                .layout = {
                    .sizing = { CLAY_SIZING_FIXED( command_panel_width ), CLAY_SIZING_FIT( 0 ) },
                },
            } ) {
                VXUI( ctx, "main.footer.actions", {
                    .layout = {
                        .sizing = { CLAY_SIZING_FIT( 0 ), CLAY_SIZING_FIT( 0 ) },
                        .childGap = 14,
                        .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                        .layoutDirection = CLAY_LEFT_TO_RIGHT,
                    },
                } ) {
                    VXUI( ctx, "main.footer.deploy", {
                        .layout = {
                            .sizing = { CLAY_SIZING_FIT( 0 ), CLAY_SIZING_FIT( 0 ) },
                            .childGap = 6,
                            .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                            .layoutDirection = CLAY_LEFT_TO_RIGHT,
                        },
                    } ) {
                        VXUI( ctx, "main.footer.deploy.key", {
                            .layout = {
                                .sizing = { CLAY_SIZING_FIT( 0 ), CLAY_SIZING_FIT( 0 ) },
                                .padding = { 4, 5, 2, 2 },
                            },
                            .backgroundColor = vxui_demo_clay_color( theme.action_fill ),
                            .cornerRadius = CLAY_CORNER_RADIUS( 0 ),
                            .border = vxui_demo_panel_border( theme.primary_panel_border, 1 ),
                        } ) {
                            VXUI_LABEL( ctx, "E", vxui_demo_text_style( visuals.mono_font_id, scale.footer_key_size, theme.section_text ) );
                        }
                        VXUI_LABEL( ctx, "DEPLOY", vxui_demo_text_style( visuals.mono_font_id, scale.footer_action_size, theme.body_text ) );
                    }

                    VXUI( ctx, "main.footer.abort", {
                        .layout = {
                            .sizing = { CLAY_SIZING_FIT( 0 ), CLAY_SIZING_FIT( 0 ) },
                            .childGap = 6,
                            .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                            .layoutDirection = CLAY_LEFT_TO_RIGHT,
                        },
                    } ) {
                        VXUI( ctx, "main.footer.abort.key", {
                            .layout = {
                                .sizing = { CLAY_SIZING_FIT( 0 ), CLAY_SIZING_FIT( 0 ) },
                                .padding = { 4, 5, 2, 2 },
                            },
                            .backgroundColor = vxui_demo_clay_color( theme.utility_fill ),
                            .cornerRadius = CLAY_CORNER_RADIUS( 0 ),
                            .border = vxui_demo_panel_border( theme.primary_panel_border, 1 ),
                        } ) {
                            VXUI_LABEL( ctx, "Q", vxui_demo_text_style( visuals.mono_font_id, scale.footer_key_size, theme.muted_text ) );
                        }
                        VXUI_LABEL( ctx, "ABORT", vxui_demo_text_style( visuals.mono_font_id, scale.footer_action_size, theme.muted_text ) );
                    }
                }
            }

            VXUI( ctx, "main.footer.gap", {
                .layout = {
                    .sizing = { CLAY_SIZING_FIXED( deck_gap ), CLAY_SIZING_GROW( 0 ) },
                },
            } ) {}

            VXUI( ctx, "main.footer.meta_lane", {
                .layout = {
                    .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                    .padding = { 0, 4, 0, 0 },
                    .childAlignment = { .x = CLAY_ALIGN_X_RIGHT },
                },
            } ) {
                VXUI( ctx, "main.footer.meta", {
                    .layout = {
                        .sizing = { CLAY_SIZING_FIT( 0 ), CLAY_SIZING_FIT( 0 ) },
                        .childGap = 6,
                        .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                        .layoutDirection = CLAY_LEFT_TO_RIGHT,
                    },
                } ) {
                    VXUI_LABEL( ctx, "CREDIT(S) 04", vxui_demo_text_style( visuals.mono_font_id, scale.footer_meta_size, theme.badge_text ) );
                    VXUI_LABEL( ctx, "|", vxui_demo_text_style( visuals.mono_font_id, scale.footer_meta_size, theme.muted_text ) );
                    VXUI_LABEL( ctx, "RANK: 1CC / NORMAL", vxui_demo_text_style( visuals.mono_font_id, scale.footer_meta_size, theme.body_text ) );
                    VXUI_LABEL( ctx, "|", vxui_demo_text_style( visuals.mono_font_id, scale.footer_meta_size, theme.muted_text ) );
                    VXUI_LABEL( ctx, "INPUT: KB/M", vxui_demo_text_style( visuals.mono_font_id, scale.footer_meta_size, theme.muted_text ) );
                }
            }
        }
    }
}

inline const vxui_demo_main_menu_preview* vxui_demo_main_menu_preview_from_focused_row( uint32_t focused_row_id )
{
    if ( const vxui_demo_main_menu_preview* preview = vxui_demo_try_main_menu_preview_from_focused_row( focused_row_id ) ) {
        return preview;
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
    const bool tight_preview_width = layout.preview_panel_width <= 420.0f;
    const bool compact_help = layout.surface_max_height <= 648.0f || tight_preview_width;
    const vxui_demo_controls_block_copy help_copy = vxui_demo_controls_block_copy_for_locale( locale, compact_help );
    const vxui_demo_command_deck_theme& theme = vxui_demo_command_deck_theme_tokens();
    const vxui_demo_controls_block_contract controls_contract =
        vxui_demo_get_controls_block_contract( layout.surface_max_height, std::max( 0.0f, layout.preview_panel_width - layout.preview_panel_padding * 2.0f ) );
    const vxui_demo_main_menu_type_scale scale = vxui_demo_get_main_menu_type_scale( layout );
    const char* badge_text = vxui_demo_badge_text( locale, preview.badge_key );

    const float deck_height = layout.deck_height;
    const uint16_t shell_gap = compact_vertical ? uint16_t{ 8 } : uint16_t{ 12 };
    const uint16_t deck_gap = ( uint16_t ) layout.deck_gap;
    const uint16_t preview_padding = ( uint16_t ) layout.preview_panel_padding;
    const uint16_t preview_gap = compact_vertical ? uint16_t{ 8 } : uint16_t{ 12 };
    const char* eyebrow = preview.eyebrow ? preview.eyebrow : copy.preview_label_text;
    vxui_color objective_text = theme.body_text;
    objective_text.a = 190;

    VXUI( ctx, "main.shell", {
        .layout = {
            .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_GROW( 0 ) },
            .padding = { 0, 0, 0, 8 },
            .childGap = shell_gap,
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
    } ) {
        vxui_demo_emit_main_menu_header( ctx, visuals, theme, copy );

        VXUI( ctx, "main.deck", {
            .layout = {
                .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIXED( deck_height ) },
                .childGap = deck_gap,
                .layoutDirection = CLAY_LEFT_TO_RIGHT,
            },
        } ) {
            VXUI( ctx, "main.command_panel", {
                .layout = {
                    .sizing = { CLAY_SIZING_FIXED( layout.command_panel_width ), CLAY_SIZING_GROW( 0 ) },
                },
            } ) {
                emit_command_panel( deck_height );
            }

            VXUI( ctx, "main.preview_panel", {
                .layout = {
                    .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_GROW( 0 ) },
                    .padding = { preview_padding, preview_padding, preview_padding, preview_padding },
                    .childGap = preview_gap,
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                },
                .backgroundColor = vxui_demo_clay_color( theme.primary_panel_fill ),
                .cornerRadius = CLAY_CORNER_RADIUS( 0 ),
                .border = vxui_demo_panel_border( theme.primary_panel_border, 1 ),
            } ) {
                VXUI( ctx, "main.preview.top", {
                    .layout = {
                        .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                        .childGap = preview_gap,
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    },
                } ) {
                    if ( eyebrow && eyebrow[ 0 ] != '\0' ) {
                        VXUI( ctx, "main.preview.eyebrow", {
                            .layout = {
                                .sizing = { CLAY_SIZING_FIT( 0 ), CLAY_SIZING_FIT( 0 ) },
                                .childGap = 8,
                                .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                                .layoutDirection = CLAY_LEFT_TO_RIGHT,
                            },
                            } ) {
                                VXUI( ctx, "main.preview.eyebrow.line", {
                                    .layout = {
                                        .sizing = { CLAY_SIZING_FIXED( 14.0f ), CLAY_SIZING_FIXED( 1.0f ) },
                                    },
                                    .backgroundColor = vxui_demo_clay_color( theme.section_text ),
                                } ) {}
                            vxui_label_cfg eyebrow_style = vxui_demo_text_style( visuals.mono_font_id, scale.preview_eyebrow_size, theme.section_text );
                            eyebrow_style.letter_spacing = 1;
                            VXUI_LABEL( ctx, eyebrow, eyebrow_style );
                        }
                    }

                    VXUI( ctx, "main.preview_header", {
                        .layout = {
                            .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                            .childGap = compact_vertical ? uint16_t{ 8 } : uint16_t{ 12 },
                            .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                            .layoutDirection = CLAY_LEFT_TO_RIGHT,
                        },
                    } ) {
                        VXUI( ctx, "main.preview_header.text", {
                            .layout = {
                                .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                                .childGap = compact_vertical ? uint16_t{ 4 } : uint16_t{ 6 },
                                .layoutDirection = CLAY_TOP_TO_BOTTOM,
                            },
                        } ) {
                            VXUI_LABEL( ctx, preview.title, vxui_demo_text_style( visuals.title_font_id, scale.preview_title_size, theme.title_text ) );
                        }

                        VXUI( ctx, "main.preview_header.spacer", {
                            .layout = {
                                .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_GROW( 0 ) },
                            },
                        } ) {}

                        VXUI( ctx, "main.preview.badge_stack", {
                            .layout = {
                                .sizing = { CLAY_SIZING_FIT( 0 ), CLAY_SIZING_FIT( 0 ) },
                                .childGap = 2,
                                .childAlignment = { .x = CLAY_ALIGN_X_RIGHT },
                                .layoutDirection = CLAY_TOP_TO_BOTTOM,
                            },
                        } ) {
                            vxui_demo_emit_main_menu_preview_badge( ctx, "main.preview.badge", visuals.mono_font_id, scale.preview_badge_size, theme, badge_text );
                            if ( preview.row_id && std::strcmp( preview.row_id, "sortie" ) == 0 ) {
                                VXUI_LABEL( ctx, "INTERCEPTOR BLOOM", vxui_demo_text_style( visuals.mono_font_id, scale.preview_bloom_size, theme.warning_text ) );
                            }
                        }
                    }

                    vxui_demo_emit_main_menu_divider( ctx, "main.preview.divider", theme.primary_panel_border );

                    VXUI( ctx, "main.preview_body", {
                        .layout = {
                            .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                            .childGap = compact_vertical ? uint16_t{ 4 } : uint16_t{ 5 },
                            .layoutDirection = CLAY_TOP_TO_BOTTOM,
                        },
                    } ) {
                        if ( preview.subtitle && preview.subtitle[ 0 ] != '\0' ) {
                            vxui_label_cfg subtitle_style = vxui_demo_text_style( visuals.title_font_id, scale.preview_subtitle_size, objective_text );
                            subtitle_style.letter_spacing = 1;
                            VXUI_LABEL( ctx, preview.subtitle, subtitle_style );
                        }

                        VXUI_LABEL( ctx, preview.body, vxui_demo_text_style( visuals.body_font_id, scale.preview_body_size, theme.body_text ) );
                        if ( preview.body_line_2 && preview.body_line_2[ 0 ] != '\0' ) {
                            VXUI_LABEL( ctx, preview.body_line_2, vxui_demo_text_style( visuals.body_font_id, scale.preview_body_size, theme.body_text ) );
                        }

                        if ( preview.warning && preview.warning[ 0 ] != '\0' ) {
                            VXUI_LABEL( ctx, preview.warning, vxui_demo_text_style( visuals.mono_font_id, scale.preview_warning_size, theme.warning_text ) );
                        }

                        if ( vxui_demo_main_menu_preview_stat_count( preview ) > 0 ) {
                            VXUI( ctx, "main.preview.stats", {
                                .layout = {
                                    .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                                    .padding = { compact_vertical ? uint16_t{ 12 } : uint16_t{ 10 }, compact_vertical ? uint16_t{ 12 } : uint16_t{ 10 }, compact_vertical ? uint16_t{ 10 } : uint16_t{ 8 }, compact_vertical ? uint16_t{ 10 } : uint16_t{ 8 } },
                                    .childGap = compact_vertical ? uint16_t{ 6 } : uint16_t{ 8 },
                                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                                },
                                .backgroundColor = vxui_demo_clay_color( theme.secondary_panel_fill ),
                                .cornerRadius = CLAY_CORNER_RADIUS( 0 ),
                                .border = vxui_demo_panel_border( theme.primary_panel_border, 1 ),
                            } ) {
                                VXUI( ctx, "main.preview.stats.header", {
                                    .layout = {
                                        .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                                        .childGap = 8,
                                        .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                                        .layoutDirection = CLAY_LEFT_TO_RIGHT,
                                    },
                                } ) {
                                    if ( preview.stat_heading && preview.stat_heading[ 0 ] != '\0' ) {
                                        VXUI_LABEL( ctx, preview.stat_heading, vxui_demo_text_style( visuals.mono_font_id, scale.preview_stat_heading_size, theme.section_text ) );
                                    }
                                    VXUI( ctx, "main.preview.stats.header.spacer", {
                                        .layout = {
                                            .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_GROW( 0 ) },
                                        },
                                    } ) {}
                                    if ( preview.stat_title && preview.stat_title[ 0 ] != '\0' ) {
                                        vxui_label_cfg stat_title_style = vxui_demo_text_style( visuals.title_font_id, scale.preview_stat_title_size, theme.title_text );
                                        stat_title_style.letter_spacing = 1;
                                        VXUI_LABEL( ctx, preview.stat_title, stat_title_style );
                                    }
                                }

                                VXUI( ctx, "main.preview.stats.grid", {
                                    .layout = {
                                        .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                                        .childGap = compact_vertical ? uint16_t{ 10 } : uint16_t{ 12 },
                                        .layoutDirection = CLAY_LEFT_TO_RIGHT,
                                    },
                                } ) {
                                    for ( int i = 0; i < 3; ++i ) {
                                        if ( !preview.stat_labels[ i ] || !preview.stat_values[ i ] ) {
                                            continue;
                                        }

                                        const std::string stat_id = std::string( "main.preview.stats.col." ) + std::to_string( i );
                                        VXUI_HASH( ctx, vxui_id( stat_id.c_str() ), {
                                            .layout = {
                                                .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                                                .childGap = 4,
                                                .layoutDirection = CLAY_TOP_TO_BOTTOM,
                                            },
                                        } ) {
                                            VXUI_LABEL( ctx, preview.stat_labels[ i ], vxui_demo_text_style( visuals.mono_font_id, scale.preview_stat_label_size, theme.muted_text ) );
                                            VXUI_LABEL( ctx, preview.stat_values[ i ], vxui_demo_text_style( visuals.mono_font_id, scale.preview_stat_value_size, i == 2 ? theme.warning_text : theme.section_text ) );
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                VXUI( ctx, "main.preview.body_spacer", {
                    .layout = {
                        .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_GROW( 0 ) },
                    },
                } ) {}

                vxui_demo_emit_main_menu_controls_block(
                    ctx,
                    "main.preview.controls",
                    help_copy,
                    visuals.mono_font_id,
                    visuals.title_font_id,
                    theme,
                    controls_contract.title_font_size,
                    controls_contract.line_font_size,
                    controls_contract.padding,
                    controls_contract.row_gap,
                    controls_contract.min_height );

            }
        }

        vxui_demo_emit_main_menu_footer( ctx, visuals, theme, layout.command_panel_width, ( float ) deck_gap );
    }
}
