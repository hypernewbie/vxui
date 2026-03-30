#pragma once

#include <algorithm>
#include <cstring>
#include <string>

#include "theme.h"
#include "../../vxui.h"
#include "../../vxui_menu.h"
#include "layout_contract.h"

struct vxui_demo_split_deck_visuals
{
    uint32_t body_font_id;
    uint32_t title_font_id;
    uint32_t section_font_id;
};

struct vxui_demo_shared_scanline
{
    float spacing;
    float alpha;
};

struct vxui_demo_status_summary_cfg
{
    const char* locale_name_key;
    const char* prompt_name_key;
    const char* top_name_key;
    int screen_count;
};

struct vxui_demo_sortie_screen_cfg
{
    vxui_menu_state* menu_state;
    int* selected_mission_index;
    int* difficulty_index;
    float layout_surface_max_height_override;
    bool background_scanline;
    vxui_action_fn start_fn;
    vxui_action_cfg start_cfg;
    vxui_action_fn back_fn;
    vxui_action_cfg back_cfg;
    vxui_demo_status_summary_cfg status;
};

struct vxui_demo_loadout_screen_cfg
{
    vxui_menu_state* menu_state;
    int* selected_ship_index;
    int* selected_primary_index;
    int* selected_support_index;
    int* selected_system_index;
    float layout_surface_max_height_override;
    bool background_scanline;
    vxui_action_fn back_fn;
    vxui_action_cfg back_cfg;
    vxui_demo_status_summary_cfg status;
};

struct vxui_demo_archives_screen_cfg
{
    vxui_menu_state* menu_state;
    int* archive_category_index;
    int* archive_entry_index;
    float layout_surface_max_height_override;
    bool background_scanline;
    vxui_action_fn back_fn;
    vxui_action_cfg back_cfg;
    vxui_demo_status_summary_cfg status;
};

struct vxui_demo_records_screen_cfg
{
    vxui_menu_state* menu_state;
    int* records_board_index;
    int* records_entry_index;
    float layout_surface_max_height_override;
    bool background_scanline;
    vxui_action_fn back_fn;
    vxui_action_cfg back_cfg;
    vxui_demo_status_summary_cfg status;
};

struct vxui_demo_shared_mission
{
    const char* name;
    const char* region;
    const char* briefing;
    const char* threat;
    const char* reward;
    const char* warning;
};

struct vxui_demo_shared_ship
{
    const char* name;
    const char* class_name;
    const char* summary;
    float speed;
    float shield;
    float output;
    float sync;
};

struct vxui_demo_shared_archive_entry
{
    const char* title;
    const char* subtitle;
    const char* detail;
    const char* meta;
    bool unlocked;
};

struct vxui_demo_shared_record
{
    const char* run_name;
    const char* pilot_name;
    const char* ship_name;
    const char* stage_name;
    const char* score_text;
    const char* clear_text;
    const char* note;
};

inline const vxui_demo_shared_mission VXUI_DEMO_SHARED_MISSIONS[] = {
    { "OP-01 Aegis Break", "Stratos Belt", "Breach the carrier screen, cut a corridor through interceptor rings, and keep the convoy beacon alive long enough to escape.", "Threat Tier 02 / Interceptor bloom", "Reward: Burst rail prototype", "Warning: Dense crossfire near the aft gate." },
    { "OP-02 Silent Meridian", "Aster Reach", "Thread a low-visibility debris lane and destroy the relay pair feeding hostile target locks into the sector grid.", "Threat Tier 03 / Sensor fog", "Reward: Shield lattice shard", "Warning: Heavy drift and delayed visual reads." },
    { "OP-03 Pale Crown", "Helios Verge", "Crack the defense lattice around the crown platform and extract the command archive before the perimeter resets.", "Threat Tier 04 / Shield bloom", "Reward: Archive clearance key", "Warning: Tight windows on the centerline." },
    { "OP-04 Last Metric", "Greywater Rim", "Stage a final stress sortie against an unstable fleet pocket built to show late-loop records and results states.", "Threat Tier 05 / Attrition field", "Reward: Demo clear marker", "Warning: Sustained pressure from every lane." },
};

inline const char* vxui_demo_shared_mission_briefing_text( int mission_index )
{
    return VXUI_DEMO_SHARED_MISSIONS[ std::clamp( mission_index, 0, 3 ) ].briefing;
}

inline void vxui_demo_emit_compact_mission_briefing_lines(
    vxui_ctx* ctx,
    int mission_index,
    uint32_t font_id,
    vxui_color color )
{
    const char* lines[5] = {};
    int line_count = 0;
    switch ( std::clamp( mission_index, 0, 3 ) ) {
        case 0:
            lines[0] = "Breach the carrier screen,";
            lines[1] = "cut through interceptor rings,";
            lines[2] = "keep the convoy beacon alive,";
            lines[3] = "and open the escape lane.";
            line_count = 4;
            break;
        case 1:
            lines[0] = "Thread a low-visibility debris lane,";
            lines[1] = "destroy the relay pair, and break";
            lines[2] = "the hostile target-lock feed";
            lines[3] = "into the sector grid.";
            line_count = 4;
            break;
        case 2:
            lines[0] = "Crack the defense lattice around";
            lines[1] = "the crown platform, then extract";
            lines[2] = "the command archive before";
            lines[3] = "the perimeter resets.";
            line_count = 4;
            break;
        case 3:
            lines[0] = "Stage a final stress sortie against";
            lines[1] = "an unstable fleet pocket and push";
            lines[2] = "through the late-loop pressure";
            lines[3] = "state cleanly.";
            line_count = 4;
            break;
    }

    for ( int i = 0; i < line_count; ++i ) {
        VXUI_LABEL( ctx, lines[i], vxui_demo_text_style( font_id, 10.0f, color ) );
    }
}

inline void vxui_demo_emit_compact_mission_title_lines(
    vxui_ctx* ctx,
    int mission_index,
    uint32_t font_id,
    vxui_color color )
{
    const char* lines[2] = {};
    int line_count = 0;
    switch ( std::clamp( mission_index, 0, 3 ) ) {
        case 0:
            lines[0] = "OP-01 Aegis";
            lines[1] = "Break";
            line_count = 2;
            break;
        case 1:
            lines[0] = "OP-02 Silent";
            lines[1] = "Meridian";
            line_count = 2;
            break;
        case 2:
            lines[0] = "OP-03 Pale";
            lines[1] = "Crown";
            line_count = 2;
            break;
        case 3:
            lines[0] = "OP-04 Last";
            lines[1] = "Metric";
            line_count = 2;
            break;
    }

    for ( int i = 0; i < line_count; ++i ) {
        VXUI_LABEL( ctx, lines[i], vxui_demo_text_style( font_id, 17.0f, color ) );
    }
}

inline const vxui_demo_shared_ship VXUI_DEMO_SHARED_SHIPS[] = {
    { "VF-17 Kestrel", "Balanced Interceptor", "Fast response frame with forgiving shields and an easy rhythm for menu testing.", 0.72f, 0.66f, 0.58f, 0.70f },
    { "VX-4 Halberd", "Heavy Breaker", "Slower but decisive burst output with broad lock-on windows and strong shield retention.", 0.42f, 0.88f, 0.86f, 0.51f },
    { "AR-9 Mistral", "Sweep Specialist", "High-mobility frame built around lane control, support uptime, and aggressive repositioning.", 0.91f, 0.46f, 0.62f, 0.80f },
    { "NX-0 Aurora", "Prototype Lattice", "Experimental chassis with volatile output curves and elevated system sync for flashy demo bars.", 0.65f, 0.55f, 0.95f, 0.93f },
};

inline const vxui_demo_shared_archive_entry VXUI_DEMO_SHARED_ARCHIVE_INTEL[] = {
    { "Intercept Packet 04", "Fleet route fragment", "Recovered fleet routing packet with enough integrity left to expose the next relay corridor and justify a second sortie pass.", "Status: decrypted", true },
    { "Signal Burn Log", "Telemetry noise capture", "A dense telemetry scrape used to justify the scanline toggle, warning accents, and command-deck radar vocabulary.", "Status: cleaned for archive", true },
};

inline const vxui_demo_shared_archive_entry VXUI_DEMO_SHARED_ARCHIVE_FRAMES[] = {
    { "Halberd Field Notes", "Maintenance digest", "Crew notes on thermal pacing, burst discipline, and why the heavy frame deserves the recommended badge in demo copy.", "Status: available", true },
    { "Aurora Lattice Memo", "Prototype clearance", "Experimental notes remain partially hidden to show locked-state visuals and mixed unlocked/locked archive lists.", "Status: restricted", false },
};

inline const vxui_demo_shared_archive_entry VXUI_DEMO_SHARED_ARCHIVE_SIGNALS[] = {
    { "Wake Sweep 7C", "Deep-space beacon trace", "A quiet signal layer used as the fiction excuse for the boot handshake and title ambience treatment.", "Status: tagged for sortie", true },
    { "Null Carrier Echo", "Redacted relay shard", "Still locked in the stub build, but visible so the archives screen demonstrates disabled rows without hiding content.", "Status: locked", false },
};

inline const vxui_demo_shared_record VXUI_DEMO_SHARED_RECORDS[] = {
    { "Run 01 / Kestrel", "Iris Vale", "VF-17 Kestrel", "Aegis Break", "3,284,120", "1CC / Normal", "Balanced clear used as the reference board row." },
    { "Run 02 / Halberd", "Rho Mercer", "VX-4 Halberd", "Silent Meridian", "4,012,770", "No-miss / Hard", "Heavy burst route with slower movement but higher relay damage." },
    { "Run 03 / Mistral", "Naomi Crest", "AR-9 Mistral", "Pale Crown", "3,776,980", "Normal / Fast clear", "Mobility route showcasing quicker stage times and lower shield padding." },
    { "Run 04 / Aurora", "Seta Noor", "NX-0 Aurora", "Last Metric", "4,488,400", "Prototype / Demo clear", "High-sync prototype route built to justify the command-deck warning colors." },
};

inline const char* const VXUI_DEMO_SHARED_DIFFICULTY_KEYS[] = { "difficulty.easy", "difficulty.normal", "difficulty.hard" };
inline const char* const VXUI_DEMO_SHARED_MISSION_NAMES[] = { "OP-01 Aegis Break", "OP-02 Silent Meridian", "OP-03 Pale Crown", "OP-04 Last Metric" };
inline const char* const VXUI_DEMO_SHARED_SHIP_NAMES[] = { "VF-17 Kestrel", "VX-4 Halberd", "AR-9 Mistral", "NX-0 Aurora" };
inline const char* const VXUI_DEMO_SHARED_PRIMARY_NAMES[] = { "Burst Rail", "Spread Lance", "Pulse Needler", "Vector Beam" };
inline const char* const VXUI_DEMO_SHARED_SUPPORT_NAMES[] = { "Shield Drone", "Mine Curtain", "Target Decoy", "Repair Pulse" };
inline const char* const VXUI_DEMO_SHARED_SYSTEM_NAMES[] = { "Afterburn Sync", "Mag Shield", "Trace Assist", "Risk Core" };
inline const char* const VXUI_DEMO_SHARED_ARCHIVE_CATEGORY_NAMES[] = { "Intel Bank", "Frame Logs", "Signal Vault" };
inline const char* const VXUI_DEMO_SHARED_ARCHIVE_INTEL_NAMES[] = { "Intercept Packet 04", "Signal Burn Log" };
inline const char* const VXUI_DEMO_SHARED_ARCHIVE_FRAME_NAMES[] = { "Halberd Field Notes", "Aurora Lattice Memo" };
inline const char* const VXUI_DEMO_SHARED_ARCHIVE_SIGNAL_NAMES[] = { "Wake Sweep 7C", "Null Carrier Echo" };
inline const char* const VXUI_DEMO_SHARED_RECORD_BOARD_NAMES[] = { "Global Board", "Ship Board", "Prototype Board" };
inline const char* const VXUI_DEMO_SHARED_RECORD_NAMES[] = { "Run 01 / Kestrel", "Run 02 / Halberd", "Run 03 / Mistral", "Run 04 / Aurora" };

static inline Clay_ElementId vxui_demo_split_deck_hashed_id( uint32_t id )
{
    return ( Clay_ElementId ) { .id = id, .offset = 0, .baseId = 0, .stringId = { 0 } };
}

inline bool vxui_demo_split_deck_locale_matches( const char* locale, const char* prefix )
{
    if ( !locale || !prefix ) {
        return false;
    }
    const size_t prefix_len = std::strlen( prefix );
    if ( std::strncmp( locale, prefix, prefix_len ) != 0 ) {
        return false;
    }
    return locale[ prefix_len ] == '\0' || locale[ prefix_len ] == '-' || locale[ prefix_len ] == '_';
}

inline const char* vxui_demo_shared_locale_name_key_from_locale( const char* locale )
{
    if ( vxui_demo_split_deck_locale_matches( locale, "ja" ) ) {
        return "locale.name.ja";
    }
    if ( vxui_demo_split_deck_locale_matches( locale, "ar" ) ) {
        return "locale.name.ar";
    }
    return "locale.name.en";
}

inline const char* vxui_demo_shared_footer_locale_key_from_locale( const char* locale )
{
    if ( vxui_demo_split_deck_locale_matches( locale, "ja" ) ) {
        return "locale.short.ja";
    }
    if ( vxui_demo_split_deck_locale_matches( locale, "ar" ) ) {
        return "locale.short.ar";
    }
    return "locale.short.en";
}

inline const char* vxui_demo_shared_prompt_name_key( int prompt_table_index )
{
    return prompt_table_index == 0 ? "prompt.name.keyboard" : "prompt.name.gamepad";
}

inline const char* vxui_demo_shared_footer_prompt_name_key( int prompt_table_index )
{
    return prompt_table_index == 0 ? "prompt.short.keyboard" : "prompt.short.gamepad";
}

inline vxui_menu_style vxui_demo_shared_split_deck_shell_style(
    const vxui_demo_split_deck_visuals& visuals,
    float lane_gap,
    bool compact )
{
    const vxui_demo_command_deck_theme& theme = vxui_demo_command_deck_theme_tokens();
    vxui_menu_style style = vxui_menu_style_split_deck();
    vxui_demo_apply_title_menu_theme( style );
    style.option_wrap_by_default = true;
    style.body_font_id = visuals.body_font_id;
    style.title_font_id = visuals.title_font_id;
    style.badge_font_id = visuals.body_font_id;
    style.body_font_size = compact ? 14.0f : 16.0f;
    style.secondary_font_size = compact ? 11.0f : 13.0f;
    style.title_font_size = compact ? 24.0f : 30.0f;
    style.badge_font_size = compact ? 9.0f : 10.0f;
    style.row_gap = compact ? 6.0f : 8.0f;
    style.section_gap = compact ? 8.0f : 10.0f;
    style.padding_x = compact ? 8.0f : 10.0f;
    style.padding_y = compact ? 6.0f : 7.0f;
    style.lane_gap = lane_gap;
    style.corner_radius = 12.0f;
    style.panel_fill_color = theme.secondary_panel_fill;
    style.row_fill_color = theme.primary_panel_fill;
    style.row_focus_fill_color = theme.focused_row_fill;
    style.row_border_color = theme.primary_panel_border;
    style.row_focus_border_color = theme.secondary_panel_border;
    style.section_text_color = theme.title_text;
    style.secondary_text_color = theme.muted_text;
    style.prompt_fill_color = theme.action_fill;
    style.prompt_text_color = theme.action_text;
    return style;
}

inline void vxui_demo_shared_emit_surface_scanline( vxui_ctx* ctx, const char* root_id, float alpha_scale = 0.45f )
{
    const vxui_demo_command_deck_theme& theme = vxui_demo_command_deck_theme_tokens();
    const uint32_t previous_decl_id = ctx ? ctx->current_decl_id : 0u;
    if ( ctx ) {
        ctx->current_decl_id = vxui_id( root_id );
    }
    VXUI_TRAIT( VXUI_TRAIT_SCANLINE, ( vxui_demo_shared_scanline ) { .spacing = 10.0f, .alpha = theme.surface_scanline_alpha * alpha_scale } );
    if ( ctx ) {
        ctx->current_decl_id = previous_decl_id;
    }
}

inline bool vxui_demo_shared_use_sortie_tertiary_lane( const vxui_demo_split_deck_layout_spec& layout, bool compact_detail )
{
    return !compact_detail && layout.tertiary_lane_width >= 220.0f && layout.secondary_lane_width >= 430.0f;
}

template <size_t PromptCount, size_t StatusCount>
inline vxui_menu_footer_cfg vxui_demo_shared_make_split_deck_footer_cfg(
    vxui_menu_prompt_item ( &prompt_items )[ PromptCount ],
    vxui_menu_status_item ( &status_items )[ StatusCount ],
    const vxui_demo_status_summary_cfg& status,
    std::string& screen_count_text,
    int compact_max_status_items )
{
    screen_count_text = std::to_string( std::max( 0, status.screen_count ) );
    prompt_items[ 0 ] = { "action.confirm", "menu.confirm", false };
    prompt_items[ 1 ] = { "action.cancel", "menu.cancel", false };
    status_items[ 0 ] = { "status.short.locale", status.locale_name_key, VXUI_MENU_STATUS_PRIMARY, false, false };
    status_items[ 1 ] = { "status.short.prompts", status.prompt_name_key, VXUI_MENU_STATUS_SECONDARY, true, false };
    status_items[ 2 ] = { "status.short.screens", screen_count_text.c_str(), VXUI_MENU_STATUS_SECONDARY, true, false };
    status_items[ 3 ] = { "status.short.top", status.top_name_key, VXUI_MENU_STATUS_PRIMARY, false, false };
    return ( vxui_menu_footer_cfg ) {
        prompt_items,
        ( int ) PromptCount,
        status_items,
        ( int ) StatusCount,
        VXUI_MENU_SHELL_COMPACT_AUTO,
        compact_max_status_items,
        false,
    };
}

inline vxui_menu_screen_cfg vxui_demo_shared_make_split_deck_screen_cfg(
    const vxui_menu_style* shell_style,
    const char* title_key,
    const vxui_demo_split_deck_layout_spec& layout,
    bool tertiary_enabled,
    const vxui_menu_footer_cfg& footer_cfg )
{
    return ( vxui_menu_screen_cfg ) {
        VXUI_MENU_SHELL_SPLIT_DECK,
        shell_style,
        VXUI_MENU_SHELL_COMPACT_AUTO,
        680.0f,
        1140.0f,
        tertiary_enabled,
        { title_key, nullptr, false },
        { layout.primary_lane_width, layout.primary_lane_width, false, false, false },
        { layout.secondary_lane_width, layout.secondary_lane_width, false, false, false },
        { layout.tertiary_lane_width, layout.tertiary_lane_width, false, true, !tertiary_enabled },
        {},
        footer_cfg,
    };
}

inline vxui_menu_style vxui_demo_shared_menu_style_form_deck( const vxui_demo_split_deck_visuals& visuals, float label_lane_width )
{
    vxui_menu_style style = vxui_menu_style_form();
    style.option_wrap_by_default = true;
    style.value_lane_mode = VXUI_MENU_VALUE_LANE_GROW;
    style.body_font_id = visuals.body_font_id;
    style.title_font_id = visuals.section_font_id;
    style.badge_font_id = visuals.body_font_id;
    style.label_lane_width = label_lane_width;
    style.body_font_size = 18.0f;
    style.secondary_font_size = 15.0f;
    style.title_font_size = 24.0f;
    style.row_height = 40.0f;
    style.row_gap = 4.0f;
    style.section_gap = 10.0f;
    style.padding_x = 14.0f;
    style.padding_y = 10.0f;
    vxui_demo_apply_form_menu_theme( style );
    style.panel_fill_color = { 0, 0, 0, 0 };
    return style;
}

inline void vxui_demo_shared_emit_stat_bar( vxui_ctx* ctx, const vxui_demo_split_deck_visuals& visuals, const char* id, const char* label, float value )
{
    const vxui_demo_command_deck_theme& theme = vxui_demo_command_deck_theme_tokens();
    value = std::clamp( value, 0.0f, 1.0f );
    const std::string row_id = std::string( id ) + ".row";
    const std::string fill_id = std::string( id ) + ".fill";
    CLAY( vxui_demo_split_deck_hashed_id( vxui_id( row_id.c_str() ) ), {
        .layout = {
            .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
            .childGap = 8,
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
    } ) {
        VXUI_LABEL( ctx, label, ( vxui_label_cfg ) {
            .font_id = visuals.body_font_id,
            .font_size = 18.0f,
            .color = theme.muted_text,
        } );
        CLAY( vxui_demo_split_deck_hashed_id( vxui_id( id ) ), {
            .layout = {
                .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIXED( 12 ) },
            },
            .backgroundColor = vxui_demo_clay_color( theme.stat_track ),
            .cornerRadius = CLAY_CORNER_RADIUS( 6 ),
        } ) {
            CLAY( vxui_demo_split_deck_hashed_id( vxui_id( fill_id.c_str() ) ), {
                .layout = {
                    .sizing = { CLAY_SIZING_FIXED( value * 220.0f ), CLAY_SIZING_GROW( 0 ) },
                },
                .backgroundColor = vxui_demo_clay_color( theme.stat_fill ),
                .cornerRadius = CLAY_CORNER_RADIUS( 6 ),
            } ) {}
        }
    }
}

inline const vxui_demo_shared_archive_entry* vxui_demo_shared_archive_entries_for_category( int category_index, int* out_count, const char* const** out_names )
{
    switch ( std::clamp( category_index, 0, 2 ) ) {
        case 1:
            if ( out_count ) *out_count = ( int ) ( sizeof( VXUI_DEMO_SHARED_ARCHIVE_FRAMES ) / sizeof( VXUI_DEMO_SHARED_ARCHIVE_FRAMES[ 0 ] ) );
            if ( out_names ) *out_names = VXUI_DEMO_SHARED_ARCHIVE_FRAME_NAMES;
            return VXUI_DEMO_SHARED_ARCHIVE_FRAMES;
        case 2:
            if ( out_count ) *out_count = ( int ) ( sizeof( VXUI_DEMO_SHARED_ARCHIVE_SIGNALS ) / sizeof( VXUI_DEMO_SHARED_ARCHIVE_SIGNALS[ 0 ] ) );
            if ( out_names ) *out_names = VXUI_DEMO_SHARED_ARCHIVE_SIGNAL_NAMES;
            return VXUI_DEMO_SHARED_ARCHIVE_SIGNALS;
        case 0:
        default:
            if ( out_count ) *out_count = ( int ) ( sizeof( VXUI_DEMO_SHARED_ARCHIVE_INTEL ) / sizeof( VXUI_DEMO_SHARED_ARCHIVE_INTEL[ 0 ] ) );
            if ( out_names ) *out_names = VXUI_DEMO_SHARED_ARCHIVE_INTEL_NAMES;
            return VXUI_DEMO_SHARED_ARCHIVE_INTEL;
    }
}

inline void vxui_demo_render_sortie_screen_shared( vxui_ctx* ctx, const vxui_demo_split_deck_visuals& visuals, const vxui_demo_sortie_screen_cfg& cfg )
{
    const vxui_demo_command_deck_theme& theme = vxui_demo_command_deck_theme_tokens();
    const bool rtl = ctx->rtl;
    const float viewport_width = std::max( 0.0f, ( float ) ctx->cfg.screen_width - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f );
    const float surface_max_height = std::max( 0.0f, ( float ) ctx->cfg.screen_height - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f );
    const float layout_surface_max_height =
        cfg.layout_surface_max_height_override > 0.0f
        ? std::min( surface_max_height, cfg.layout_surface_max_height_override )
        : surface_max_height;
    const vxui_demo_split_deck_layout_spec layout =
        vxui_demo_resolve_split_deck_layout( VXUI_DEMO_SURFACE_SORTIE, viewport_width, layout_surface_max_height, ctx->locale );
    const bool compact_locale =
        vxui_demo_split_deck_locale_matches( ctx->locale, "ja" ) || vxui_demo_split_deck_locale_matches( ctx->locale, "ar" );
    const bool compact_footer = layout.surface_max_height <= 650.0f;
    const bool compact_shell = compact_footer || layout.surface_max_height <= 680.0f || ctx->cfg.screen_width <= 1140;
    const bool compact_menu = compact_footer || compact_locale || layout.surface_max_height <= 680.0f || layout.primary_lane_width <= 360.0f;
    const bool compact_detail = compact_footer || compact_locale || layout.surface_max_height <= 700.0f || layout.secondary_lane_width <= 400.0f;
    const bool tertiary_enabled = vxui_demo_shared_use_sortie_tertiary_lane( layout, compact_detail );
    vxui_menu_style shell_style = vxui_demo_shared_split_deck_shell_style( visuals, layout.deck_gap, compact_shell );
    vxui_menu_style form_style = vxui_demo_shared_menu_style_form_deck( visuals, 110.0f );
    form_style.body_font_size = compact_footer ? 13.0f : compact_menu ? 15.0f : 16.0f;
    form_style.secondary_font_size = compact_footer ? 10.0f : compact_menu ? 12.0f : 13.0f;
    form_style.badge_font_size = compact_footer ? 8.0f : compact_menu ? 10.0f : 11.0f;
    form_style.row_height = compact_footer ? 22.0f : compact_menu ? 28.0f : 30.0f;
    form_style.row_gap = compact_footer ? 1.0f : 2.0f;
    form_style.section_gap = compact_footer ? 3.0f : compact_menu ? 5.0f : 8.0f;
    form_style.padding_y = compact_footer ? 3.0f : compact_menu ? 6.0f : 8.0f;
    vxui_menu_surface_cfg surface_cfg = {
        layout.surface.surface_width,
        layout.surface_max_height,
        ( float ) VXUI_DEMO_LAYOUT_OUTER_PADDING,
        ( float ) VXUI_DEMO_LAYOUT_SURFACE_PADDING_X,
        ( float ) VXUI_DEMO_LAYOUT_SURFACE_PADDING_Y,
        ( float ) VXUI_DEMO_LAYOUT_SECTION_GAP,
        18.0f,
        1.0f,
        theme.app_background_base,
        theme.primary_panel_fill,
        theme.primary_panel_border,
    };
    if ( cfg.background_scanline ) {
        vxui_demo_shared_emit_surface_scanline( ctx, "sortie" );
    }

    vxui_menu_surface_begin( ctx, "sortie", "sortie.surface", &surface_cfg );
    {
        vxui_menu_prompt_item footer_prompts[ 2 ] = {};
        vxui_menu_status_item footer_status[ 4 ] = {};
        std::string screen_count_text;
        vxui_menu_footer_cfg footer_cfg = vxui_demo_shared_make_split_deck_footer_cfg(
            footer_prompts,
            footer_status,
            cfg.status,
            screen_count_text,
            compact_footer ? 3 : 4 );
        vxui_menu_screen_cfg screen_cfg =
            vxui_demo_shared_make_split_deck_screen_cfg( &shell_style, "menu.sortie", layout, tertiary_enabled, footer_cfg );
        vxui_menu_state shell_state = {};

        vxui_menu_screen_begin( ctx, &shell_state, "sortie.deck", &screen_cfg );
        vxui_menu_header( ctx, "sortie.header", &screen_cfg.header );

        vxui_menu_primary_lane_begin( ctx, "sortie.menu_panel", &screen_cfg.primary_lane );
        vxui_menu_begin( ctx, cfg.menu_state, "sortie.menu", ( vxui_menu_cfg ) {
            .style = &form_style,
            .viewport_height = layout.menu_viewport_height,
        } );
        vxui_menu_section( ctx, cfg.menu_state, "operations", "sortie.section.operations", ( vxui_menu_section_cfg ) { 0 } );
        vxui_menu_option( ctx, cfg.menu_state, "mission", "Mission", cfg.selected_mission_index, const_cast< const char** >( VXUI_DEMO_SHARED_MISSION_NAMES ), 4, ( vxui_menu_row_cfg ) { 0 }, ( vxui_option_cfg ) { 0 } );
        vxui_menu_section( ctx, cfg.menu_state, "setup", "sortie.section.setup", ( vxui_menu_section_cfg ) { 0 } );
        vxui_menu_option( ctx, cfg.menu_state, "difficulty", "menu.difficulty", cfg.difficulty_index, const_cast< const char** >( VXUI_DEMO_SHARED_DIFFICULTY_KEYS ), 3, ( vxui_menu_row_cfg ) { 0 }, ( vxui_option_cfg ) { 0 } );
        vxui_menu_action( ctx, cfg.menu_state, "start", "menu.start_sortie", cfg.start_fn, ( vxui_menu_row_cfg ) { 0 }, cfg.start_cfg );
        vxui_menu_action( ctx, cfg.menu_state, "back", "menu.back", cfg.back_fn, ( vxui_menu_row_cfg ) { 0 }, cfg.back_cfg );
        vxui_menu_end( ctx, cfg.menu_state );
        vxui_menu_primary_lane_end( ctx );

        const int mission_index = std::clamp( cfg.selected_mission_index ? *cfg.selected_mission_index : 0, 0, 3 );
        const vxui_demo_shared_mission& mission = VXUI_DEMO_SHARED_MISSIONS[ mission_index ];
        const char* briefing_text = vxui_demo_shared_mission_briefing_text( mission_index );
        const uint16_t detail_panel_gap = compact_footer ? uint16_t{ 6 } : compact_detail ? uint16_t{ 8 } : uint16_t{ 12 };
        const uint16_t detail_body_gap = compact_footer ? uint16_t{ 8 } : compact_detail ? uint16_t{ 12 } : uint16_t{ 20 };
        const uint16_t tertiary_panel_gap = compact_detail ? uint16_t{ 8 } : uint16_t{ 10 };

        vxui_menu_secondary_lane_begin( ctx, "sortie.briefing", &screen_cfg.secondary_lane );
        VXUI( ctx, "sortie.briefing.header", {
            .layout = {
                .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                .childGap = compact_detail ? uint16_t{ 2 } : uint16_t{ 4 },
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
        } ) {
            if ( compact_detail ) {
                vxui_demo_emit_compact_mission_title_lines( ctx, mission_index, visuals.title_font_id, theme.title_text );
            } else {
                VXUI_LABEL( ctx, mission.name, vxui_demo_text_style( visuals.title_font_id, 26.0f, theme.title_text ) );
            }
            VXUI_LABEL( ctx, mission.region, vxui_demo_text_style( visuals.section_font_id, compact_footer ? 12.0f : compact_detail ? 13.0f : 15.0f, theme.accent_cool ) );
        }

        VXUI( ctx, "sortie.briefing.body", {
            .layout = {
                .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                .childGap = detail_body_gap,
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
        } ) {
            VXUI( ctx, "sortie.briefing.copy", {
                .layout = {
                    .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                    .padding = { 0, 0, 0, compact_footer ? uint16_t{ 8 } : compact_detail ? uint16_t{ 18 } : uint16_t{ 24 } },
                    .childGap = compact_detail ? uint16_t{ 2 } : uint16_t{ 0 },
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                },
            } ) {
                if ( compact_detail ) {
                    vxui_demo_emit_compact_mission_briefing_lines( ctx, mission_index, visuals.body_font_id, theme.body_text );
                } else {
                    VXUI_LABEL( ctx, briefing_text, vxui_demo_text_style( visuals.body_font_id, 12.0f, theme.body_text ) );
                }
            }
            if ( !compact_footer ) {
                VXUI( ctx, "sortie.briefing.warning_gap", {
                    .layout = {
                        .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIXED( compact_detail ? 10.0f : 16.0f ) },
                    },
                } ) {}
                VXUI_LABEL( ctx, mission.warning, vxui_demo_text_style( visuals.body_font_id, compact_detail ? 10.0f : 11.0f, theme.warning_text ) );
            }
            if ( !tertiary_enabled ) {
                VXUI( ctx, "sortie.briefing.meta", {
                    .layout = {
                        .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                        .childGap = compact_footer ? uint16_t{ 4 } : uint16_t{ 8 },
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    },
                } ) {
                    VXUI_LABEL( ctx, mission.threat, vxui_demo_text_style( visuals.section_font_id, compact_footer ? 12.0f : compact_detail ? 13.0f : 15.0f, theme.section_text ) );
                    VXUI_LABEL( ctx, mission.reward, vxui_demo_text_style( visuals.body_font_id, compact_footer ? 11.0f : compact_detail ? 12.0f : 13.0f, theme.body_text ) );
                    if ( !compact_footer ) {
                        VXUI_LABEL( ctx, "Challenge window stable", vxui_demo_text_style( visuals.body_font_id, compact_detail ? 11.0f : 12.0f, theme.success_text ) );
                    }
                }
            }
        }
        vxui_menu_secondary_lane_end( ctx );

        if ( tertiary_enabled ) {
            vxui_menu_tertiary_lane_begin( ctx, "sortie.detail", &screen_cfg.tertiary_lane );
            VXUI( ctx, "sortie.detail.body", {
                .layout = {
                    .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                    .childGap = tertiary_panel_gap,
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                },
            } ) {
                VXUI_LABEL( ctx, mission.threat, vxui_demo_text_style( visuals.section_font_id, compact_detail ? 15.0f : 16.0f, theme.section_text ) );
                VXUI_LABEL( ctx, mission.reward, vxui_demo_text_style( visuals.body_font_id, compact_detail ? 13.0f : 14.0f, theme.body_text ) );
                VXUI_LABEL( ctx, "Challenge window stable", vxui_demo_text_style( visuals.body_font_id, compact_detail ? 12.0f : 13.0f, theme.success_text ) );
            }
            vxui_menu_tertiary_lane_end( ctx );
        }

        vxui_menu_footer( ctx, "sortie.footer", &screen_cfg.footer );
        vxui_menu_screen_end( ctx, &shell_state );
    }
    vxui_menu_surface_end( ctx );
}

inline void vxui_demo_render_loadout_screen_shared( vxui_ctx* ctx, const vxui_demo_split_deck_visuals& visuals, const vxui_demo_loadout_screen_cfg& cfg )
{
    const vxui_demo_command_deck_theme& theme = vxui_demo_command_deck_theme_tokens();
    const bool rtl = ctx->rtl;
    const float viewport_width = std::max( 0.0f, ( float ) ctx->cfg.screen_width - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f );
    const float surface_max_height = std::max( 0.0f, ( float ) ctx->cfg.screen_height - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f );
    const float layout_surface_max_height =
        cfg.layout_surface_max_height_override > 0.0f
        ? std::min( surface_max_height, cfg.layout_surface_max_height_override )
        : surface_max_height;
    const vxui_demo_split_deck_layout_spec layout =
        vxui_demo_resolve_split_deck_layout( VXUI_DEMO_SURFACE_LOADOUT, viewport_width, layout_surface_max_height, ctx->locale );
    const bool compact_footer = layout.surface_max_height <= 620.0f;
    const bool compact_shell = compact_footer || layout.surface_max_height <= 680.0f || ctx->cfg.screen_width <= 1140;
    vxui_menu_style shell_style = vxui_demo_shared_split_deck_shell_style( visuals, layout.deck_gap, compact_shell );
    vxui_menu_style form_style = vxui_demo_shared_menu_style_form_deck( visuals, 136.0f );
    form_style.row_height = compact_footer ? 24.0f : 32.0f;
    form_style.row_gap = compact_footer ? 1.0f : 2.0f;
    form_style.section_gap = compact_footer ? 4.0f : form_style.section_gap;
    form_style.padding_y = compact_footer ? 4.0f : form_style.padding_y;
    vxui_menu_surface_cfg surface_cfg = {
        layout.surface.surface_width,
        layout.surface_max_height,
        ( float ) VXUI_DEMO_LAYOUT_OUTER_PADDING,
        ( float ) VXUI_DEMO_LAYOUT_SURFACE_PADDING_X,
        ( float ) VXUI_DEMO_LAYOUT_SURFACE_PADDING_Y,
        ( float ) VXUI_DEMO_LAYOUT_SECTION_GAP,
        18.0f,
        1.0f,
        theme.app_background_base,
        theme.primary_panel_fill,
        theme.primary_panel_border,
    };
    if ( cfg.background_scanline ) {
        vxui_demo_shared_emit_surface_scanline( ctx, "loadout" );
    }

    vxui_menu_surface_begin( ctx, "loadout", "loadout.surface", &surface_cfg );
    {
        vxui_menu_prompt_item footer_prompts[ 2 ] = {};
        vxui_menu_status_item footer_status[ 4 ] = {};
        std::string screen_count_text;
        vxui_menu_footer_cfg footer_cfg = vxui_demo_shared_make_split_deck_footer_cfg(
            footer_prompts,
            footer_status,
            cfg.status,
            screen_count_text,
            compact_footer ? 3 : 4 );
        vxui_menu_screen_cfg screen_cfg =
            vxui_demo_shared_make_split_deck_screen_cfg( &shell_style, "menu.loadout", layout, false, footer_cfg );
        vxui_menu_state shell_state = {};

        vxui_menu_screen_begin( ctx, &shell_state, "loadout.deck", &screen_cfg );
        vxui_menu_header( ctx, "loadout.header", &screen_cfg.header );

        vxui_menu_primary_lane_begin( ctx, "loadout.menu_panel", &screen_cfg.primary_lane );
        vxui_menu_begin( ctx, cfg.menu_state, "loadout.menu", ( vxui_menu_cfg ) {
            .style = &form_style,
            .viewport_height = layout.menu_viewport_height,
        } );
        vxui_menu_section( ctx, cfg.menu_state, "frames", "loadout.section.ships", ( vxui_menu_section_cfg ) { 0 } );
        vxui_menu_option( ctx, cfg.menu_state, "ship", "Frame", cfg.selected_ship_index, const_cast< const char** >( VXUI_DEMO_SHARED_SHIP_NAMES ), 4, ( vxui_menu_row_cfg ) { 0 }, ( vxui_option_cfg ) { 0 } );
        vxui_menu_section( ctx, cfg.menu_state, "systems", "loadout.section.weapons", ( vxui_menu_section_cfg ) { 0 } );
        vxui_menu_option( ctx, cfg.menu_state, "primary", "Primary", cfg.selected_primary_index, const_cast< const char** >( VXUI_DEMO_SHARED_PRIMARY_NAMES ), 4, ( vxui_menu_row_cfg ) { 0 }, ( vxui_option_cfg ) { 0 } );
        vxui_menu_option( ctx, cfg.menu_state, "support", "Support", cfg.selected_support_index, const_cast< const char** >( VXUI_DEMO_SHARED_SUPPORT_NAMES ), 4, ( vxui_menu_row_cfg ) { 0 }, ( vxui_option_cfg ) { 0 } );
        vxui_menu_option( ctx, cfg.menu_state, "system", "System", cfg.selected_system_index, const_cast< const char** >( VXUI_DEMO_SHARED_SYSTEM_NAMES ), 4, ( vxui_menu_row_cfg ) { 0 }, ( vxui_option_cfg ) { 0 } );
        vxui_menu_action( ctx, cfg.menu_state, "back", "menu.back", cfg.back_fn, ( vxui_menu_row_cfg ) { 0 }, cfg.back_cfg );
        vxui_menu_end( ctx, cfg.menu_state );
        vxui_menu_primary_lane_end( ctx );

        const vxui_demo_shared_ship& ship = VXUI_DEMO_SHARED_SHIPS[ std::clamp( cfg.selected_ship_index ? *cfg.selected_ship_index : 0, 0, 3 ) ];
        vxui_menu_secondary_lane_begin( ctx, "loadout.detail", &screen_cfg.secondary_lane );
        VXUI( ctx, "loadout.detail.body", {
            .layout = {
                .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                .childGap = compact_footer ? uint16_t{ 8 } : uint16_t{ 12 },
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
        } ) {
            VXUI( ctx, "loadout.frame_card", {
                .layout = {
                    .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                    .childGap = compact_footer ? uint16_t{ 4 } : uint16_t{ 8 },
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                },
            } ) {
                VXUI_LABEL( ctx, ship.name, vxui_demo_text_style( visuals.title_font_id, compact_footer ? 30.0f : 34.0f, theme.title_text ) );
                VXUI_LABEL( ctx, ship.class_name, vxui_demo_text_style( visuals.section_font_id, compact_footer ? 18.0f : 20.0f, theme.accent_cool ) );
                VXUI_LABEL( ctx, ship.summary, vxui_demo_text_style( visuals.body_font_id, compact_footer ? 15.0f : 17.0f, theme.body_text ) );
            }

            VXUI( ctx, "loadout.stat_card", {
                .layout = {
                    .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                    .childGap = compact_footer ? uint16_t{ 6 } : uint16_t{ 8 },
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                },
            } ) {
                vxui_demo_shared_emit_stat_bar( ctx, visuals, "loadout.stat.speed", "Speed", ship.speed );
                vxui_demo_shared_emit_stat_bar( ctx, visuals, "loadout.stat.shield", "Shield", ship.shield );
                vxui_demo_shared_emit_stat_bar( ctx, visuals, "loadout.stat.output", "Output", ship.output );
                vxui_demo_shared_emit_stat_bar( ctx, visuals, "loadout.stat.sync", "Sync", ship.sync );
                VXUI_LABEL( ctx, VXUI_DEMO_SHARED_PRIMARY_NAMES[ std::clamp( cfg.selected_primary_index ? *cfg.selected_primary_index : 0, 0, 3 ) ], vxui_demo_text_style( visuals.body_font_id, 15.0f, theme.section_text ) );
                VXUI_LABEL( ctx, VXUI_DEMO_SHARED_SUPPORT_NAMES[ std::clamp( cfg.selected_support_index ? *cfg.selected_support_index : 0, 0, 3 ) ], vxui_demo_text_style( visuals.body_font_id, 15.0f, theme.body_text ) );
                VXUI_LABEL( ctx, VXUI_DEMO_SHARED_SYSTEM_NAMES[ std::clamp( cfg.selected_system_index ? *cfg.selected_system_index : 0, 0, 3 ) ], vxui_demo_text_style( visuals.body_font_id, 15.0f, theme.success_text ) );
            }
        }
        vxui_menu_secondary_lane_end( ctx );

        vxui_menu_footer( ctx, "loadout.footer", &screen_cfg.footer );
        vxui_menu_screen_end( ctx, &shell_state );
    }
    vxui_menu_surface_end( ctx );
}

inline void vxui_demo_render_archives_screen_shared( vxui_ctx* ctx, const vxui_demo_split_deck_visuals& visuals, const vxui_demo_archives_screen_cfg& cfg )
{
    const vxui_demo_command_deck_theme& theme = vxui_demo_command_deck_theme_tokens();
    const bool rtl = ctx->rtl;
    const float viewport_width = std::max( 0.0f, ( float ) ctx->cfg.screen_width - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f );
    const float surface_max_height = std::max( 0.0f, ( float ) ctx->cfg.screen_height - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f );
    const float layout_surface_max_height =
        cfg.layout_surface_max_height_override > 0.0f
        ? std::min( surface_max_height, cfg.layout_surface_max_height_override )
        : surface_max_height;
    const vxui_demo_split_deck_layout_spec layout =
        vxui_demo_resolve_split_deck_layout( VXUI_DEMO_SURFACE_ARCHIVES, viewport_width, layout_surface_max_height, ctx->locale );
    const bool compact_footer = layout.surface_max_height <= 620.0f;
    const bool compact_shell = compact_footer || layout.surface_max_height <= 680.0f || ctx->cfg.screen_width <= 1140;
    vxui_menu_style shell_style = vxui_demo_shared_split_deck_shell_style( visuals, layout.deck_gap, compact_shell );
    vxui_menu_style form_style = vxui_demo_shared_menu_style_form_deck( visuals, 132.0f );
    form_style.row_height = compact_footer ? 24.0f : 32.0f;
    form_style.row_gap = compact_footer ? 1.0f : 2.0f;
    form_style.section_gap = compact_footer ? 4.0f : form_style.section_gap;
    form_style.padding_y = compact_footer ? 4.0f : form_style.padding_y;

    vxui_menu_surface_cfg surface_cfg = {
        layout.surface.surface_width,
        layout.surface_max_height,
        ( float ) VXUI_DEMO_LAYOUT_OUTER_PADDING,
        ( float ) VXUI_DEMO_LAYOUT_SURFACE_PADDING_X,
        ( float ) VXUI_DEMO_LAYOUT_SURFACE_PADDING_Y,
        ( float ) VXUI_DEMO_LAYOUT_SECTION_GAP,
        18.0f,
        1.0f,
        theme.app_background_base,
        theme.primary_panel_fill,
        theme.primary_panel_border,
    };
    if ( cfg.background_scanline ) {
        vxui_demo_shared_emit_surface_scanline( ctx, "archives" );
    }

    vxui_menu_surface_begin( ctx, "archives", "archives.surface", &surface_cfg );
    {
        vxui_menu_prompt_item footer_prompts[ 2 ] = {};
        vxui_menu_status_item footer_status[ 4 ] = {};
        std::string screen_count_text;
        vxui_menu_footer_cfg footer_cfg = vxui_demo_shared_make_split_deck_footer_cfg(
            footer_prompts,
            footer_status,
            cfg.status,
            screen_count_text,
            compact_footer ? 3 : 4 );
        vxui_menu_screen_cfg screen_cfg =
            vxui_demo_shared_make_split_deck_screen_cfg( &shell_style, "menu.archives", layout, false, footer_cfg );
        vxui_menu_state shell_state = {};

        vxui_menu_screen_begin( ctx, &shell_state, "archives.deck", &screen_cfg );
        vxui_menu_header( ctx, "archives.header", &screen_cfg.header );

        int entry_count = 0;
        const char* const* entry_names = nullptr;
        vxui_demo_shared_archive_entries_for_category( cfg.archive_category_index ? *cfg.archive_category_index : 0, &entry_count, &entry_names );
        if ( cfg.archive_entry_index ) {
            *cfg.archive_entry_index = std::clamp( *cfg.archive_entry_index, 0, std::max( 0, entry_count - 1 ) );
        }

        vxui_menu_primary_lane_begin( ctx, "archives.menu_panel", &screen_cfg.primary_lane );
        vxui_menu_begin( ctx, cfg.menu_state, "archives.menu", ( vxui_menu_cfg ) {
            .style = &form_style,
            .viewport_height = layout.menu_viewport_height,
        } );
        vxui_menu_option( ctx, cfg.menu_state, "category", "menu.category", cfg.archive_category_index, const_cast< const char** >( VXUI_DEMO_SHARED_ARCHIVE_CATEGORY_NAMES ), 3, ( vxui_menu_row_cfg ) { 0 }, ( vxui_option_cfg ) { 0 } );
        vxui_menu_option( ctx, cfg.menu_state, "entry", "Entry", cfg.archive_entry_index, const_cast< const char** >( entry_names ), entry_count, ( vxui_menu_row_cfg ) { 0 }, ( vxui_option_cfg ) { 0 } );
        vxui_menu_action( ctx, cfg.menu_state, "back", "menu.back", cfg.back_fn, ( vxui_menu_row_cfg ) { 0 }, cfg.back_cfg );
        vxui_menu_end( ctx, cfg.menu_state );
        vxui_menu_primary_lane_end( ctx );

        const vxui_demo_shared_archive_entry* entries =
            vxui_demo_shared_archive_entries_for_category( cfg.archive_category_index ? *cfg.archive_category_index : 0, &entry_count, &entry_names );
        const int entry_index = std::clamp( cfg.archive_entry_index ? *cfg.archive_entry_index : 0, 0, std::max( 0, entry_count - 1 ) );
        const vxui_demo_shared_archive_entry& entry = entries[ entry_index ];

        vxui_menu_secondary_lane_begin( ctx, "archives.detail", &screen_cfg.secondary_lane );
        VXUI( ctx, "archives.detail.header", {
            .layout = {
                .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                .childGap = 4,
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
        } ) {
            VXUI_LABEL( ctx, entry.title, vxui_demo_text_style( visuals.title_font_id, compact_footer ? 30.0f : 34.0f, theme.title_text ) );
            VXUI_LABEL( ctx, entry.subtitle, vxui_demo_text_style( visuals.section_font_id, compact_footer ? 18.0f : 20.0f, theme.accent_cool ) );
        }
        VXUI( ctx, "archives.detail.body", {
            .layout = {
                .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                .childGap = compact_footer ? uint16_t{ 10 } : uint16_t{ 12 },
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
        } ) {
            VXUI_LABEL( ctx, entry.detail, vxui_demo_text_style( visuals.body_font_id, compact_footer ? 15.0f : 17.0f, theme.body_text ) );
            VXUI_LABEL( ctx, entry.meta, vxui_demo_text_style( visuals.body_font_id, 15.0f, entry.unlocked ? theme.section_text : theme.alert_text ) );
        }
        vxui_menu_secondary_lane_end( ctx );

        vxui_menu_footer( ctx, "archives.footer", &screen_cfg.footer );
        vxui_menu_screen_end( ctx, &shell_state );
    }
    vxui_menu_surface_end( ctx );
}

inline void vxui_demo_render_records_screen_shared( vxui_ctx* ctx, const vxui_demo_split_deck_visuals& visuals, const vxui_demo_records_screen_cfg& cfg )
{
    const vxui_demo_command_deck_theme& theme = vxui_demo_command_deck_theme_tokens();
    const bool rtl = ctx->rtl;
    const float viewport_width = std::max( 0.0f, ( float ) ctx->cfg.screen_width - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f );
    const float surface_max_height = std::max( 0.0f, ( float ) ctx->cfg.screen_height - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f );
    const float layout_surface_max_height =
        cfg.layout_surface_max_height_override > 0.0f
        ? std::min( surface_max_height, cfg.layout_surface_max_height_override )
        : surface_max_height;
    const vxui_demo_split_deck_layout_spec layout =
        vxui_demo_resolve_split_deck_layout( VXUI_DEMO_SURFACE_RECORDS, viewport_width, layout_surface_max_height, ctx->locale );
    const bool compact_footer = layout.surface_max_height <= 620.0f;
    if ( cfg.records_entry_index ) {
        *cfg.records_entry_index = std::clamp( *cfg.records_entry_index, 0, 3 );
    }
    const bool compact_shell = compact_footer || layout.surface_max_height <= 680.0f || ctx->cfg.screen_width <= 1140;
    vxui_menu_style shell_style = vxui_demo_shared_split_deck_shell_style( visuals, layout.deck_gap, compact_shell );
    vxui_menu_style form_style = vxui_demo_shared_menu_style_form_deck( visuals, 132.0f );
    form_style.row_height = compact_footer ? 24.0f : 32.0f;
    form_style.row_gap = compact_footer ? 1.0f : 2.0f;
    form_style.section_gap = compact_footer ? 4.0f : form_style.section_gap;
    form_style.padding_y = compact_footer ? 4.0f : form_style.padding_y;

    vxui_menu_surface_cfg surface_cfg = {
        layout.surface.surface_width,
        layout.surface_max_height,
        ( float ) VXUI_DEMO_LAYOUT_OUTER_PADDING,
        ( float ) VXUI_DEMO_LAYOUT_SURFACE_PADDING_X,
        ( float ) VXUI_DEMO_LAYOUT_SURFACE_PADDING_Y,
        ( float ) VXUI_DEMO_LAYOUT_SECTION_GAP,
        18.0f,
        1.0f,
        theme.app_background_base,
        theme.primary_panel_fill,
        theme.primary_panel_border,
    };
    if ( cfg.background_scanline ) {
        vxui_demo_shared_emit_surface_scanline( ctx, "records" );
    }

    vxui_menu_surface_begin( ctx, "records", "records.surface", &surface_cfg );
    {
        vxui_menu_prompt_item footer_prompts[ 2 ] = {};
        vxui_menu_status_item footer_status[ 4 ] = {};
        std::string screen_count_text;
        vxui_menu_footer_cfg footer_cfg = vxui_demo_shared_make_split_deck_footer_cfg(
            footer_prompts,
            footer_status,
            cfg.status,
            screen_count_text,
            compact_footer ? 3 : 4 );
        vxui_menu_screen_cfg screen_cfg =
            vxui_demo_shared_make_split_deck_screen_cfg( &shell_style, "menu.records", layout, false, footer_cfg );
        vxui_menu_state shell_state = {};

        vxui_menu_screen_begin( ctx, &shell_state, "records.deck", &screen_cfg );
        vxui_menu_header( ctx, "records.header", &screen_cfg.header );

        vxui_menu_primary_lane_begin( ctx, "records.menu_panel", &screen_cfg.primary_lane );
        vxui_menu_begin( ctx, cfg.menu_state, "records.menu", ( vxui_menu_cfg ) {
            .style = &form_style,
            .viewport_height = layout.menu_viewport_height,
        } );
        vxui_menu_option( ctx, cfg.menu_state, "board", "menu.board", cfg.records_board_index, const_cast< const char** >( VXUI_DEMO_SHARED_RECORD_BOARD_NAMES ), 3, ( vxui_menu_row_cfg ) { 0 }, ( vxui_option_cfg ) { 0 } );
        vxui_menu_option( ctx, cfg.menu_state, "run", "Run", cfg.records_entry_index, const_cast< const char** >( VXUI_DEMO_SHARED_RECORD_NAMES ), 4, ( vxui_menu_row_cfg ) { 0 }, ( vxui_option_cfg ) { 0 } );
        vxui_menu_action( ctx, cfg.menu_state, "back", "menu.back", cfg.back_fn, ( vxui_menu_row_cfg ) { 0 }, cfg.back_cfg );
        vxui_menu_end( ctx, cfg.menu_state );
        vxui_menu_primary_lane_end( ctx );

        const vxui_demo_shared_record& record = VXUI_DEMO_SHARED_RECORDS[ std::clamp( cfg.records_entry_index ? *cfg.records_entry_index : 0, 0, 3 ) ];
        vxui_menu_secondary_lane_begin( ctx, "records.detail", &screen_cfg.secondary_lane );
        VXUI( ctx, "records.detail.header", {
            .layout = {
                .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                .childGap = 4,
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
        } ) {
            VXUI_LABEL( ctx, record.run_name, vxui_demo_text_style( visuals.title_font_id, compact_footer ? 30.0f : 34.0f, theme.title_text ) );
            VXUI_LABEL( ctx, record.pilot_name, vxui_demo_text_style( visuals.section_font_id, compact_footer ? 18.0f : 20.0f, theme.accent_cool ) );
        }

        VXUI( ctx, "records.detail.body", {
            .layout = {
                .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                .childGap = compact_footer ? uint16_t{ 10 } : uint16_t{ 12 },
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
        } ) {
            VXUI_LABEL( ctx, record.ship_name, vxui_demo_text_style( visuals.body_font_id, compact_footer ? 15.0f : 17.0f, theme.body_text ) );
            VXUI_LABEL( ctx, record.stage_name, vxui_demo_text_style( visuals.body_font_id, compact_footer ? 15.0f : 17.0f, theme.section_text ) );
            VXUI_LABEL( ctx, record.score_text, vxui_demo_text_style( visuals.section_font_id, compact_footer ? 20.0f : 22.0f, theme.title_text ) );
            VXUI_LABEL( ctx, record.clear_text, vxui_demo_text_style( visuals.body_font_id, 16.0f, theme.success_text ) );
            VXUI_LABEL( ctx, record.note, vxui_demo_text_style( visuals.body_font_id, 15.0f, theme.muted_text ) );
        }
        vxui_menu_secondary_lane_end( ctx );

        vxui_menu_footer( ctx, "records.footer", &screen_cfg.footer );
        vxui_menu_screen_end( ctx, &shell_state );
    }
    vxui_menu_surface_end( ctx );
}
