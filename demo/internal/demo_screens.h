#pragma once


#include <algorithm>
#include <string>
#include "../../vxui.h"
#include "../../vxui_menu.h"
#include "demo_layout.h"
#include "main_menu_shared.h"

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

inline const char* vxui_demo_shared_locale_name_key_from_locale( const char* locale )
{
    return vxui_demo_locale_text( locale, "locale.name.en", "locale.name.ja", "locale.name.ar" );
}

inline const char* vxui_demo_shared_footer_locale_key_from_locale( const char* locale )
{
    return vxui_demo_locale_text( locale, "locale.short.en", "locale.short.ja", "locale.short.ar" );
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
    style.body_font_size = 15.0f;
    style.secondary_font_size = 11.0f;
    style.title_font_size = 28.0f;
    style.badge_font_size = 10.0f;
    style.row_gap = 6.0f;
    style.section_gap = 12.0f;
    style.padding_x = 12.0f;
    style.padding_y = 10.0f;
    if ( compact ) {
        style.body_font_size = 14.0f;
        style.secondary_font_size = 11.0f;
        style.title_font_size = 24.0f;
        style.badge_font_size = 9.0f;
        style.row_gap = 6.0f;
        style.section_gap = 8.0f;
        style.padding_x = 8.0f;
        style.padding_y = 6.0f;
    }
    style.lane_gap = lane_gap;
    // Split-deck lane colors intentionally differ from apply_title_menu_theme defaults
    style.panel_fill_color = { 0, 0, 0, 0 };
    style.row_fill_color = { 0, 0, 0, 0 };
    style.row_border_color = { 0, 0, 0, 0 };
    style.row_focus_border_color = theme.focused_row_border;
    style.section_text_color = theme.title_text;
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
        nullptr,
        0,
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
    style.body_font_size = 15.0f;
    style.secondary_font_size = 11.0f;
    style.title_font_size = 12.0f;
    style.row_height = 32.0f;
    style.row_gap = 4.0f;
    style.section_gap = 12.0f;
    style.padding_x = 14.0f;
    style.padding_y = 10.0f;
    vxui_demo_apply_form_menu_theme( style );
    style.panel_fill_color = { 0, 0, 0, 0 };
    return style;
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

struct vxui_demo_ops_layout_spec
{
    vxui_demo_surface_metrics surface;
    float surface_max_height;
    float rail_width;
    float detail_width;
    float deck_gap;
    float deck_height;
    bool compact;
    bool tall_portrait;
};

struct vxui_demo_ops_type_scale
{
    float eyebrow_size;
    float title_size;
    float section_size;
    float body_size;
    float micro_size;
    float footer_key_size;
    float footer_action_size;
    float footer_meta_size;
};

struct vxui_demo_ops_screen_setup
{
    vxui_demo_ops_layout_spec layout;
    vxui_demo_ops_type_scale scale;
    vxui_menu_style menu_style;
};

inline const char* vxui_demo_ops_resolve_text( vxui_ctx* ctx, const char* key )
{
    const char* resolved = vxui__resolve_text( ctx, key );
    return resolved ? resolved : key;
}

inline vxui_demo_ops_layout_spec vxui_demo_resolve_ops_layout(
    vxui_demo_surface_kind kind,
    float viewport_width,
    float viewport_height,
    const char* locale )
{
    float rail_fraction = 0.29f;
    float rail_min_width = 242.0f;
    float rail_max_width = 266.0f;
    float detail_min_width = 420.0f;
    switch ( kind ) {
        case VXUI_DEMO_SURFACE_SORTIE:
            rail_fraction = 0.33f;
            rail_min_width = 272.0f;
            rail_max_width = 300.0f;
            detail_min_width = 404.0f;
            break;
        case VXUI_DEMO_SURFACE_LOADOUT:
            rail_fraction = 0.31f;
            rail_min_width = 252.0f;
            rail_max_width = 282.0f;
            detail_min_width = 408.0f;
            break;
        case VXUI_DEMO_SURFACE_ARCHIVES:
            rail_fraction = 0.29f;
            rail_min_width = 244.0f;
            rail_max_width = 268.0f;
            detail_min_width = 420.0f;
            break;
        case VXUI_DEMO_SURFACE_RECORDS:
        default:
            break;
    }
    const vxui_demo_surface_metrics surface = vxui_demo_compute_surface_metrics( viewport_width, locale, kind );
    const float surface_max_height = std::max( 0.0f, viewport_height );
    const bool tall_portrait = viewport_width <= 980.0f && surface_max_height >= 900.0f;
    const bool compact = surface_max_height <= 660.0f || ( viewport_width <= 980.0f && !tall_portrait );
    const float deck_gap = compact ? 12.0f : 16.0f;
    const float header_reserve = compact ? 104.0f : tall_portrait ? 108.0f : 126.0f;
    const float footer_reserve = compact ? 86.0f : tall_portrait ? 92.0f : 144.0f;
    const float deck_height = std::max( compact ? 360.0f : 420.0f, surface_max_height - header_reserve - footer_reserve - ( tall_portrait ? 60.0f : 0.0f ) );
    const float rail_width = std::clamp( surface.content_width * rail_fraction, rail_min_width, rail_max_width );
    const float detail_width = std::max( detail_min_width, surface.content_width - rail_width - deck_gap );
    return {
        surface,
        surface_max_height,
        rail_width,
        detail_width,
        deck_gap,
        deck_height,
        compact,
        tall_portrait,
    };
}

inline vxui_demo_ops_type_scale vxui_demo_get_ops_scale( vxui_demo_surface_kind kind, const vxui_demo_ops_layout_spec& layout )
{
    float tall_title_size = 42.0f;
    float compact_title_size = 30.0f;
    float regular_title_size = 34.0f;
    switch ( kind ) {
        case VXUI_DEMO_SURFACE_SORTIE:
            tall_title_size = 48.0f;
            compact_title_size = 34.0f;
            regular_title_size = 40.0f;
            break;
        case VXUI_DEMO_SURFACE_LOADOUT:
            tall_title_size = 46.0f;
            compact_title_size = 33.0f;
            regular_title_size = 38.0f;
            break;
        case VXUI_DEMO_SURFACE_ARCHIVES:
            tall_title_size = 44.0f;
            compact_title_size = 32.0f;
            regular_title_size = 36.0f;
            break;
        case VXUI_DEMO_SURFACE_RECORDS:
        default:
            break;
    }
    if ( layout.tall_portrait ) {
        return ( vxui_demo_ops_type_scale ) { 13.0f, tall_title_size, 17.0f, 16.0f, 12.0f, 11.0f, 12.0f, 11.0f };
    }
    return layout.compact
        ? ( vxui_demo_ops_type_scale ) { 11.0f, compact_title_size, 14.0f, 13.0f, 10.0f, 10.0f, 11.0f, 10.0f }
        : ( vxui_demo_ops_type_scale ) { 12.0f, regular_title_size, 15.0f, 14.0f, 11.0f, 11.0f, 12.0f, 11.0f };
}

inline vxui_menu_style vxui_demo_ops_menu_style(
    const vxui_demo_split_deck_visuals& visuals,
    float label_lane_width,
    bool compact,
    bool tall_portrait )
{
    vxui_menu_style style = vxui_demo_shared_menu_style_form_deck( visuals, label_lane_width );
    style.body_font_id = visuals.body_font_id;
    style.title_font_id = visuals.section_font_id;
    style.badge_font_id = visuals.section_font_id;
    style.body_font_size = compact ? 13.0f : tall_portrait ? 15.0f : 14.0f;
    style.secondary_font_size = compact ? 10.0f : 11.0f;
    style.title_font_size = compact ? 11.0f : 12.0f;
    style.badge_font_size = compact ? 9.0f : 10.0f;
    style.row_height = compact ? 28.0f : tall_portrait ? 36.0f : 32.0f;
    style.row_gap = compact ? 2.0f : tall_portrait ? 4.0f : 3.0f;
    style.section_gap = compact ? 8.0f : 10.0f;
    style.padding_x = compact ? 10.0f : tall_portrait ? 14.0f : 12.0f;
    style.padding_y = compact ? 6.0f : tall_portrait ? 9.0f : 8.0f;
    return style;
}

inline vxui_demo_ops_screen_setup vxui_demo_ops_init_screen(
    vxui_ctx* ctx,
    const vxui_demo_split_deck_visuals& visuals,
    vxui_demo_surface_kind kind,
    float layout_surface_max_height_override,
    float label_lane_width,
    float tall_portrait_label_lane_width = 0.0f )
{
    const float viewport_width = std::max( 0.0f, ( float ) ctx->cfg.screen_width - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f );
    const float surface_max_height = std::max( 0.0f, ( float ) ctx->cfg.screen_height - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f );
    const float layout_surface_max_height = layout_surface_max_height_override > 0.0f
        ? std::min( surface_max_height, layout_surface_max_height_override )
        : surface_max_height;
    const vxui_demo_ops_layout_spec layout = vxui_demo_resolve_ops_layout( kind, viewport_width, layout_surface_max_height, ctx->locale );
    const float resolved_label_lane_width = layout.tall_portrait && tall_portrait_label_lane_width > 0.0f
        ? tall_portrait_label_lane_width
        : label_lane_width;
    return {
        layout,
        vxui_demo_get_ops_scale( kind, layout ),
        vxui_demo_ops_menu_style( visuals, resolved_label_lane_width, layout.compact, layout.tall_portrait ),
    };
}

inline void vxui_demo_ops_emit_chip(
    vxui_ctx* ctx,
    const char* id,
    const char* text,
    uint32_t font_id,
    float font_size,
    vxui_color text_color,
    vxui_color fill_color,
    vxui_color border_color )
{
    VXUI_HASH( ctx, vxui_id( id ), {
        .layout = {
            .sizing = { CLAY_SIZING_FIT( 0 ), CLAY_SIZING_FIT( 0 ) },
            .padding = { 3, 6, 1, 1 },
            .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
        },
        .backgroundColor = vxui_demo_clay_color( fill_color ),
        .cornerRadius = CLAY_CORNER_RADIUS( 0 ),
        .border = vxui_demo_panel_border( border_color, 1 ),
    } ) {
        VXUI_LABEL( ctx, text, vxui_demo_text_style( font_id, font_size, text_color ) );
    }
}

inline void vxui_demo_ops_emit_header(
    vxui_ctx* ctx,
    const char* id,
    const char* eyebrow,
    const char* title,
    const char* subtitle,
    uint32_t mono_font_id,
    uint32_t title_font_id,
    uint32_t body_font_id,
    const vxui_demo_ops_type_scale& scale,
    const vxui_demo_command_deck_theme& theme )
{
    const std::string base( id );
    const std::string header_id = base + ".header";
    const std::string divider_id = base + ".divider";
    VXUI_HASH( ctx, vxui_id( header_id.c_str() ), {
        .layout = {
            .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
            .childGap = 4,
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
    } ) {
        VXUI_LABEL( ctx, eyebrow, vxui_demo_text_style( mono_font_id, scale.eyebrow_size, theme.badge_text ) );
        VXUI_LABEL( ctx, title, vxui_demo_text_style( title_font_id, scale.title_size, theme.title_text ) );
        if ( subtitle && subtitle[ 0 ] != '\0' ) {
            VXUI_LABEL( ctx, subtitle, vxui_demo_text_style( body_font_id, scale.body_size, theme.body_text ) );
        }
        vxui_demo_emit_main_menu_divider( ctx, divider_id.c_str(), theme.primary_panel_border );
    }
}

inline void vxui_demo_ops_emit_footer(
    vxui_ctx* ctx,
    const char* id,
    const vxui_demo_status_summary_cfg& status,
    uint32_t mono_font_id,
    const vxui_demo_ops_type_scale& scale,
    const vxui_demo_command_deck_theme& theme )
{
    const std::string base( id );
    const std::string footer_id = base + ".footer";
    const std::string left_id = footer_id + ".left";
    const std::string right_id = footer_id + ".right";
    const std::string deploy_id = footer_id + ".deploy";
    const std::string cancel_id = footer_id + ".cancel";
    const std::string locale_text = std::string( vxui_demo_ops_resolve_text( ctx, status.locale_name_key ) );
    const std::string prompt_text = std::string( vxui_demo_ops_resolve_text( ctx, status.prompt_name_key ) );
    const std::string top_text = std::string( status.top_name_key ? status.top_name_key : "Demo" );
    const std::string count_text = std::to_string( std::max( 0, status.screen_count ) );

    VXUI_HASH( ctx, vxui_id( footer_id.c_str() ), {
        .layout = {
            .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
            .padding = { 8, 0, 0, 0 },
            .childGap = 10,
            .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
            .layoutDirection = CLAY_LEFT_TO_RIGHT,
        },
    } ) {
        VXUI_HASH( ctx, vxui_id( left_id.c_str() ), {
            .layout = {
                .sizing = { CLAY_SIZING_FIT( 0 ), CLAY_SIZING_FIT( 0 ) },
                .childGap = 10,
                .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                .layoutDirection = CLAY_LEFT_TO_RIGHT,
            },
        } ) {
            VXUI_HASH( ctx, vxui_id( deploy_id.c_str() ), {
                .layout = {
                    .sizing = { CLAY_SIZING_FIT( 0 ), CLAY_SIZING_FIT( 0 ) },
                    .childGap = 6,
                    .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                    .layoutDirection = CLAY_LEFT_TO_RIGHT,
                },
            } ) {
                vxui_demo_ops_emit_chip( ctx, ( deploy_id + ".key" ).c_str(), "E", mono_font_id, scale.footer_key_size, theme.section_text, theme.focused_row_fill, theme.focused_row_border );
                VXUI_LABEL( ctx, "CONFIRM", vxui_demo_text_style( mono_font_id, scale.footer_action_size, theme.body_text ) );
            }
            VXUI_HASH( ctx, vxui_id( cancel_id.c_str() ), {
                .layout = {
                    .sizing = { CLAY_SIZING_FIT( 0 ), CLAY_SIZING_FIT( 0 ) },
                    .childGap = 6,
                    .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                    .layoutDirection = CLAY_LEFT_TO_RIGHT,
                },
            } ) {
                vxui_demo_ops_emit_chip( ctx, ( cancel_id + ".key" ).c_str(), "Q", mono_font_id, scale.footer_key_size, theme.muted_text, theme.secondary_panel_fill, theme.primary_panel_border );
                VXUI_LABEL( ctx, "CANCEL", vxui_demo_text_style( mono_font_id, scale.footer_action_size, theme.muted_text ) );
            }
        }
        VXUI_HASH( ctx, vxui_id( right_id.c_str() ), {
            .layout = {
                .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                .childGap = 6,
                .childAlignment = { .x = CLAY_ALIGN_X_RIGHT, .y = CLAY_ALIGN_Y_CENTER },
                .layoutDirection = CLAY_LEFT_TO_RIGHT,
            },
        } ) {
            VXUI_LABEL( ctx, locale_text.c_str(), vxui_demo_text_style( mono_font_id, scale.footer_meta_size, theme.badge_text ) );
            VXUI_LABEL( ctx, "|", vxui_demo_text_style( mono_font_id, scale.footer_meta_size, theme.muted_text ) );
            VXUI_LABEL( ctx, prompt_text.c_str(), vxui_demo_text_style( mono_font_id, scale.footer_meta_size, theme.body_text ) );
            VXUI_LABEL( ctx, "|", vxui_demo_text_style( mono_font_id, scale.footer_meta_size, theme.muted_text ) );
            VXUI_LABEL( ctx, count_text.c_str(), vxui_demo_text_style( mono_font_id, scale.footer_meta_size, theme.muted_text ) );
            VXUI_LABEL( ctx, "|", vxui_demo_text_style( mono_font_id, scale.footer_meta_size, theme.muted_text ) );
            VXUI_LABEL( ctx, top_text.c_str(), vxui_demo_text_style( mono_font_id, scale.footer_meta_size, theme.body_text ) );
        }
    }
}

inline void vxui_demo_render_sortie_screen_ops( vxui_ctx* ctx, const vxui_demo_split_deck_visuals& visuals, const vxui_demo_sortie_screen_cfg& cfg );
inline void vxui_demo_render_loadout_screen_ops( vxui_ctx* ctx, const vxui_demo_split_deck_visuals& visuals, const vxui_demo_loadout_screen_cfg& cfg );
inline void vxui_demo_render_archives_screen_ops( vxui_ctx* ctx, const vxui_demo_split_deck_visuals& visuals, const vxui_demo_archives_screen_cfg& cfg );
inline void vxui_demo_render_records_screen_ops( vxui_ctx* ctx, const vxui_demo_split_deck_visuals& visuals, const vxui_demo_records_screen_cfg& cfg );

inline void vxui_demo_render_sortie_screen_ops( vxui_ctx* ctx, const vxui_demo_split_deck_visuals& visuals, const vxui_demo_sortie_screen_cfg& cfg )
{
    const vxui_demo_command_deck_theme& theme = vxui_demo_command_deck_theme_tokens();
    const vxui_demo_ops_screen_setup setup = vxui_demo_ops_init_screen( ctx, visuals, VXUI_DEMO_SURFACE_SORTIE, cfg.layout_surface_max_height_override, 104.0f, 92.0f );
    const vxui_demo_ops_layout_spec& layout = setup.layout;
    const vxui_demo_ops_type_scale& scale = setup.scale;
    vxui_menu_style menu_style = setup.menu_style;
    menu_style.row_height = layout.compact ? 28.0f : layout.tall_portrait ? 31.0f : 30.0f;
    menu_style.padding_x = layout.compact ? 8.0f : 10.0f;
    menu_style.padding_y = layout.compact ? 5.0f : 6.0f;
    menu_style.body_font_size = layout.compact ? 12.0f : 13.0f;
    menu_style.secondary_font_size = 10.0f;
    const int mission_index = std::clamp( cfg.selected_mission_index ? *cfg.selected_mission_index : 0, 0, 3 );
    const vxui_demo_shared_mission& mission = VXUI_DEMO_SHARED_MISSIONS[ mission_index ];
    const char* title_text = vxui_demo_ops_resolve_text( ctx, "screen.sortie" );
    const char* recommended_badge = mission_index == 0 ? "REC" : mission_index == 3 ? "FINAL" : "LIVE";

    if ( cfg.background_scanline ) {
        vxui_demo_shared_emit_surface_scanline( ctx, "sortie" );
    }

    vxui_menu_surface_cfg surface_cfg = vxui_menu_surface_cfg_default( layout.surface.surface_width, layout.surface_max_height, theme.app_background_base, theme.app_background_base, { 0, 0, 0, 0 } );
    vxui_demo_make_surface_cfg_transparent( surface_cfg );
    vxui_menu_surface_begin( ctx, "sortie", "sortie.surface", &surface_cfg );
    {
        VXUI( ctx, "sortie.shell", {
            .layout = {
                .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_GROW( 0 ) },
                .padding = { 10, 10, 8, 8 },
                .childGap = 10,
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
        } ) {
            vxui_demo_ops_emit_header( ctx, "sortie", "TACTICAL SORTIE BOARD", title_text, "Mission routing, lock checks, and launch-state prep.", visuals.section_font_id, visuals.title_font_id, visuals.body_font_id, scale, theme );

            VXUI( ctx, "sortie.board", {
                .layout = {
                    .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_GROW( 0 ) },
                    .childGap = ( uint16_t ) layout.deck_gap,
                    .padding = { 10, 10, 8, 8 },
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                },
                .backgroundColor = vxui_demo_clay_color( theme.primary_panel_fill ),
                .cornerRadius = CLAY_CORNER_RADIUS( 0 ),
                .border = vxui_demo_panel_border( theme.primary_panel_border, 1 ),
            } ) {
                VXUI( ctx, "sortie.summary.row", {
                    .layout = {
                        .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                        .childGap = 10,
                        .layoutDirection = CLAY_LEFT_TO_RIGHT,
                    },
                } ) {
                    VXUI( ctx, "sortie.summary.hero", {
                        .layout = {
                            .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                            .padding = { 10, 12, 8, 8 },
                            .childGap = 4,
                            .layoutDirection = CLAY_TOP_TO_BOTTOM,
                        },
                        .backgroundColor = vxui_demo_clay_color( theme.primary_panel_fill ),
                        .cornerRadius = CLAY_CORNER_RADIUS( 0 ),
                        .border = vxui_demo_panel_border( theme.focused_row_border, 1 ),
                    } ) {
                        VXUI_LABEL( ctx, "ACTIVE SORTIE", vxui_demo_text_style( visuals.section_font_id, scale.eyebrow_size, theme.section_text ) );
                        VXUI_LABEL( ctx, mission.name, vxui_demo_text_style( visuals.title_font_id, scale.title_size, theme.title_text ) );
                        VXUI_LABEL( ctx, mission.region, vxui_demo_text_style( visuals.section_font_id, scale.section_size, theme.accent_cool ) );
                        VXUI_LABEL( ctx, mission.briefing, vxui_demo_text_style( visuals.body_font_id, scale.body_size, theme.body_text ) );
                    }
                    VXUI( ctx, "sortie.summary.state", {
                        .layout = {
                            .sizing = { CLAY_SIZING_FIXED( layout.tall_portrait ? 236.0f : 228.0f ), CLAY_SIZING_FIT( 0 ) },
                            .padding = { 10, 12, 8, 8 },
                            .childGap = 6,
                            .layoutDirection = CLAY_TOP_TO_BOTTOM,
                        },
                        .backgroundColor = vxui_demo_clay_color( theme.secondary_panel_fill ),
                        .cornerRadius = CLAY_CORNER_RADIUS( 0 ),
                        .border = vxui_demo_panel_border( theme.primary_panel_border, 1 ),
                    } ) {
                        VXUI_LABEL( ctx, "THREAT", vxui_demo_text_style( visuals.section_font_id, scale.micro_size, theme.muted_text ) );
                        VXUI_LABEL( ctx, mission.threat, vxui_demo_text_style( visuals.section_font_id, scale.body_size, theme.alert_text ) );
                        vxui_demo_ops_emit_chip( ctx, "sortie.summary.state.badge", recommended_badge, visuals.section_font_id, scale.micro_size, theme.badge_text, theme.badge_fill, theme.primary_panel_border );
                        VXUI_LABEL( ctx, mission.warning, vxui_demo_text_style( visuals.section_font_id, scale.micro_size, theme.warning_text ) );
                    }
                }

                VXUI( ctx, "sortie.control.card", {
                    .layout = {
                        .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                        .padding = { 6, 8, 4, 4 },
                        .childGap = 4,
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    },
                    .backgroundColor = vxui_demo_clay_color( theme.secondary_panel_fill ),
                    .cornerRadius = CLAY_CORNER_RADIUS( 0 ),
                    .border = vxui_demo_panel_border( theme.primary_panel_border, 1 ),
                } ) {
                    VXUI( ctx, "sortie.control.meta", {
                        .layout = {
                            .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                            .childGap = 6,
                            .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                            .layoutDirection = CLAY_LEFT_TO_RIGHT,
                        },
                    } ) {
                        VXUI_LABEL( ctx, "LAUNCH SETUP", vxui_demo_text_style( visuals.section_font_id, scale.micro_size, theme.section_text ) );
                        VXUI_LABEL( ctx, "CATAPULT / ARMED", vxui_demo_text_style( visuals.section_font_id, scale.micro_size, theme.success_text ) );
                        VXUI( ctx, "sortie.control.meta.spacer", {
                            .layout = { .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_GROW( 0 ) } },
                        } ) {}
                        VXUI_LABEL( ctx, "VECTOR LOCK", vxui_demo_text_style( visuals.section_font_id, scale.micro_size, theme.accent_cool ) );
                    }
                    vxui_menu_begin( ctx, cfg.menu_state, "sortie.menu", ( vxui_menu_cfg ) { .style = &menu_style, .viewport_height = layout.tall_portrait ? 70.0f : 64.0f } );
                    vxui_menu_option( ctx, cfg.menu_state, "difficulty", "menu.difficulty", cfg.difficulty_index, const_cast< const char** >( VXUI_DEMO_SHARED_DIFFICULTY_KEYS ), 3, ( vxui_menu_row_cfg ) { 0 }, ( vxui_option_cfg ) { 0 } );
                    vxui_menu_action( ctx, cfg.menu_state, "start", "menu.start_sortie", cfg.start_fn, ( vxui_menu_row_cfg ) { .secondary_key = "CATAPULT ARM" }, cfg.start_cfg );
                    vxui_menu_end( ctx, cfg.menu_state );
                }

                VXUI( ctx, "sortie.return.band", {
                    .layout = {
                        .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                        .padding = { 4, 0, 0, 0 },
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    },
                } ) {
                    vxui_menu_begin( ctx, cfg.menu_state, "sortie.return.menu", ( vxui_menu_cfg ) { .style = &menu_style, .viewport_height = layout.compact ? 28.0f : 30.0f } );
                    vxui_menu_action( ctx, cfg.menu_state, "back", "menu.back", cfg.back_fn, ( vxui_menu_row_cfg ) { .secondary_key = "RETURN COMMAND DECK" }, cfg.back_cfg );
                    vxui_menu_end( ctx, cfg.menu_state );
                }
            }
            vxui_demo_ops_emit_footer( ctx, "sortie", cfg.status, visuals.section_font_id, scale, theme );
        }
    }
    vxui_menu_surface_end( ctx );
}

inline void vxui_demo_render_loadout_screen_ops( vxui_ctx* ctx, const vxui_demo_split_deck_visuals& visuals, const vxui_demo_loadout_screen_cfg& cfg )
{
    const vxui_demo_command_deck_theme& theme = vxui_demo_command_deck_theme_tokens();
    static const char* const ship_short_names[] = { "VF-17", "VX-4", "AR-9", "NX-0" };
    const vxui_demo_ops_screen_setup setup = vxui_demo_ops_init_screen( ctx, visuals, VXUI_DEMO_SURFACE_LOADOUT, cfg.layout_surface_max_height_override, 112.0f );
    const vxui_demo_ops_layout_spec& layout = setup.layout;
    const vxui_demo_ops_type_scale& scale = setup.scale;
    vxui_menu_style menu_style = setup.menu_style;
    menu_style.row_height = layout.compact ? 28.0f : layout.tall_portrait ? 31.0f : 30.0f;
    menu_style.padding_x = layout.compact ? 8.0f : 10.0f;
    menu_style.padding_y = layout.compact ? 5.0f : 6.0f;
    menu_style.body_font_size = layout.compact ? 12.0f : 13.0f;
    const int ship_index = std::clamp( cfg.selected_ship_index ? *cfg.selected_ship_index : 0, 0, 3 );
    const int primary_index = std::clamp( cfg.selected_primary_index ? *cfg.selected_primary_index : 0, 0, 3 );
    const int support_index = std::clamp( cfg.selected_support_index ? *cfg.selected_support_index : 0, 0, 3 );
    const int system_index = std::clamp( cfg.selected_system_index ? *cfg.selected_system_index : 0, 0, 3 );
    const vxui_demo_shared_ship& ship = VXUI_DEMO_SHARED_SHIPS[ ship_index ];
    const char* primary_name = VXUI_DEMO_SHARED_PRIMARY_NAMES[ primary_index ];
    const char* support_name = VXUI_DEMO_SHARED_SUPPORT_NAMES[ support_index ];
    const char* system_name = VXUI_DEMO_SHARED_SYSTEM_NAMES[ system_index ];
    const char* title_text = vxui_demo_ops_resolve_text( ctx, "screen.loadout" );

    if ( cfg.background_scanline ) {
        vxui_demo_shared_emit_surface_scanline( ctx, "loadout" );
    }

    vxui_menu_surface_cfg surface_cfg = vxui_menu_surface_cfg_default( layout.surface.surface_width, layout.surface_max_height, theme.app_background_base, theme.app_background_base, { 0, 0, 0, 0 } );
    vxui_demo_make_surface_cfg_transparent( surface_cfg );
    vxui_menu_surface_begin( ctx, "loadout", "loadout.surface", &surface_cfg );
    {
        VXUI( ctx, "loadout.shell", {
            .layout = {
                .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_GROW( 0 ) },
                .padding = { 10, 10, 8, 8 },
                .childGap = 10,
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
        } ) {
            vxui_demo_ops_emit_header( ctx, "loadout", "FRAME BAY / ACTIVE FIT", title_text, "Chassis selection, weapon fit, and readiness telemetry.", visuals.section_font_id, visuals.title_font_id, visuals.body_font_id, scale, theme );

            VXUI( ctx, "loadout.board", {
                .layout = {
                    .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_GROW( 0 ) },
                    .childGap = ( uint16_t ) layout.deck_gap,
                    .padding = { 10, 10, 8, 8 },
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                },
                .backgroundColor = vxui_demo_clay_color( theme.primary_panel_fill ),
                .cornerRadius = CLAY_CORNER_RADIUS( 0 ),
                .border = vxui_demo_panel_border( theme.primary_panel_border, 1 ),
            } ) {
                VXUI( ctx, "loadout.summary.row", {
                    .layout = {
                        .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                        .childGap = 10,
                        .layoutDirection = CLAY_LEFT_TO_RIGHT,
                    },
                } ) {
                    VXUI( ctx, "loadout.summary.hero", {
                        .layout = {
                            .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                            .padding = { 10, 12, 8, 8 },
                            .childGap = 4,
                            .layoutDirection = CLAY_TOP_TO_BOTTOM,
                        },
                        .backgroundColor = vxui_demo_clay_color( theme.primary_panel_fill ),
                        .cornerRadius = CLAY_CORNER_RADIUS( 0 ),
                        .border = vxui_demo_panel_border( theme.focused_row_border, 1 ),
                    } ) {
                        VXUI_LABEL( ctx, "ACTIVE CHASSIS", vxui_demo_text_style( visuals.section_font_id, scale.eyebrow_size, theme.section_text ) );
                        VXUI_LABEL( ctx, ship.name, vxui_demo_text_style( visuals.title_font_id, scale.title_size, theme.title_text ) );
                        VXUI_LABEL( ctx, ship.class_name, vxui_demo_text_style( visuals.section_font_id, scale.section_size, theme.accent_cool ) );
                        VXUI_LABEL( ctx, ship.summary, vxui_demo_text_style( visuals.body_font_id, scale.body_size, theme.body_text ) );
                    }
                    VXUI( ctx, "loadout.summary.state", {
                        .layout = {
                            .sizing = { CLAY_SIZING_FIXED( layout.tall_portrait ? 236.0f : 228.0f ), CLAY_SIZING_FIT( 0 ) },
                            .padding = { 10, 12, 8, 8 },
                            .childGap = 6,
                            .layoutDirection = CLAY_TOP_TO_BOTTOM,
                        },
                        .backgroundColor = vxui_demo_clay_color( theme.secondary_panel_fill ),
                        .cornerRadius = CLAY_CORNER_RADIUS( 0 ),
                        .border = vxui_demo_panel_border( theme.primary_panel_border, 1 ),
                    } ) {
                        VXUI_LABEL( ctx, "FIT STATE", vxui_demo_text_style( visuals.section_font_id, scale.micro_size, theme.muted_text ) );
                        vxui_demo_ops_emit_chip( ctx, "loadout.summary.state.badge", "SYNC GREEN", visuals.section_font_id, scale.micro_size, theme.success_text, theme.secondary_panel_fill, theme.primary_panel_border );
                        VXUI_LABEL( ctx, primary_name, vxui_demo_text_style( visuals.body_font_id, scale.body_size, theme.section_text ) );
                        VXUI_LABEL( ctx, support_name, vxui_demo_text_style( visuals.body_font_id, scale.body_size, theme.body_text ) );
                        VXUI_LABEL( ctx, system_name, vxui_demo_text_style( visuals.body_font_id, scale.body_size, theme.success_text ) );
                    }
                }

                VXUI( ctx, "loadout.selector.card", {
                    .layout = {
                        .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                        .padding = { 6, 8, 4, 4 },
                        .childGap = 4,
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    },
                    .backgroundColor = vxui_demo_clay_color( theme.secondary_panel_fill ),
                    .cornerRadius = CLAY_CORNER_RADIUS( 0 ),
                    .border = vxui_demo_panel_border( theme.primary_panel_border, 1 ),
                } ) {
                    VXUI( ctx, "loadout.selector.meta", {
                        .layout = {
                            .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                            .childGap = 6,
                            .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                            .layoutDirection = CLAY_LEFT_TO_RIGHT,
                        },
                    } ) {
                        VXUI_LABEL( ctx, "EQUIPMENT BANDS", vxui_demo_text_style( visuals.section_font_id, scale.micro_size, theme.section_text ) );
                        VXUI_LABEL( ctx, ship.name, vxui_demo_text_style( visuals.body_font_id, scale.micro_size, theme.body_text ) );
                        VXUI( ctx, "loadout.selector.meta.spacer", {
                            .layout = { .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_GROW( 0 ) } },
                        } ) {}
                        VXUI_LABEL( ctx, "READY", vxui_demo_text_style( visuals.section_font_id, scale.micro_size, theme.success_text ) );
                    }
                    vxui_menu_begin( ctx, cfg.menu_state, "loadout.menu", ( vxui_menu_cfg ) { .style = &menu_style, .viewport_height = layout.tall_portrait ? 126.0f : 116.0f } );
                    vxui_menu_option( ctx, cfg.menu_state, "ship", "Frame", cfg.selected_ship_index, const_cast< const char** >( ship_short_names ), 4, ( vxui_menu_row_cfg ) { .badge_text_key = "badge.recommended" }, ( vxui_option_cfg ) { 0 } );
                    vxui_menu_option( ctx, cfg.menu_state, "primary", "Primary", cfg.selected_primary_index, const_cast< const char** >( VXUI_DEMO_SHARED_PRIMARY_NAMES ), 4, ( vxui_menu_row_cfg ) { 0 }, ( vxui_option_cfg ) { 0 } );
                    vxui_menu_option( ctx, cfg.menu_state, "support", "Support", cfg.selected_support_index, const_cast< const char** >( VXUI_DEMO_SHARED_SUPPORT_NAMES ), 4, ( vxui_menu_row_cfg ) { 0 }, ( vxui_option_cfg ) { 0 } );
                    vxui_menu_option( ctx, cfg.menu_state, "system", "System", cfg.selected_system_index, const_cast< const char** >( VXUI_DEMO_SHARED_SYSTEM_NAMES ), 4, ( vxui_menu_row_cfg ) { 0 }, ( vxui_option_cfg ) { 0 } );
                    vxui_menu_end( ctx, cfg.menu_state );
                }

                VXUI( ctx, "loadout.stats.card", {
                    .layout = {
                        .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                        .padding = { 8, 10, 6, 6 },
                        .childGap = 6,
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    },
                    .backgroundColor = vxui_demo_clay_color( theme.secondary_panel_fill ),
                    .cornerRadius = CLAY_CORNER_RADIUS( 0 ),
                    .border = vxui_demo_panel_border( theme.primary_panel_border, 1 ),
                } ) {
                    VXUI_LABEL( ctx, "FRAME TELEMETRY", vxui_demo_text_style( visuals.section_font_id, scale.section_size, theme.section_text ) );
                    const vxui_menu_stat_bar_cfg stat_cfg = { visuals.body_font_id, scale.body_size, theme.muted_text, theme.stat_track, theme.stat_fill, layout.tall_portrait ? 180.0f : 160.0f };
                    vxui_menu_stat_bar( ctx, "loadout.stat.speed", "SPEED", ship.speed, &stat_cfg );
                    vxui_menu_stat_bar( ctx, "loadout.stat.shield", "SHIELD", ship.shield, &stat_cfg );
                    vxui_menu_stat_bar( ctx, "loadout.stat.output", "OUTPUT", ship.output, &stat_cfg );
                    vxui_menu_stat_bar( ctx, "loadout.stat.sync", "SYNC", ship.sync, &stat_cfg );
                }

                VXUI( ctx, "loadout.return.band", {
                    .layout = {
                        .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                        .padding = { 4, 0, 0, 0 },
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    },
                } ) {
                    vxui_menu_begin( ctx, cfg.menu_state, "loadout.return.menu", ( vxui_menu_cfg ) { .style = &menu_style, .viewport_height = layout.compact ? 28.0f : 30.0f } );
                    vxui_menu_action( ctx, cfg.menu_state, "back", "menu.back", cfg.back_fn, ( vxui_menu_row_cfg ) { .secondary_key = "RETURN FRAME BAY" }, cfg.back_cfg );
                    vxui_menu_end( ctx, cfg.menu_state );
                }
            }
            vxui_demo_ops_emit_footer( ctx, "loadout", cfg.status, visuals.section_font_id, scale, theme );
        }
    }
    vxui_menu_surface_end( ctx );
}

inline void vxui_demo_render_archives_screen_ops( vxui_ctx* ctx, const vxui_demo_split_deck_visuals& visuals, const vxui_demo_archives_screen_cfg& cfg )
{
    const vxui_demo_command_deck_theme& theme = vxui_demo_command_deck_theme_tokens();
    static const char* const archive_category_short_names[] = { "INTEL", "FRAME", "SIGNAL" };
    const vxui_demo_ops_screen_setup setup = vxui_demo_ops_init_screen( ctx, visuals, VXUI_DEMO_SURFACE_ARCHIVES, cfg.layout_surface_max_height_override, 84.0f );
    const vxui_demo_ops_layout_spec& layout = setup.layout;
    const vxui_demo_ops_type_scale& scale = setup.scale;
    vxui_menu_style menu_style = setup.menu_style;
    menu_style.row_height = layout.compact ? 28.0f : layout.tall_portrait ? 30.0f : 29.0f;
    menu_style.padding_x = layout.compact ? 8.0f : 10.0f;
    menu_style.padding_y = layout.compact ? 5.0f : 6.0f;
    menu_style.body_font_size = layout.compact ? 12.0f : 13.0f;
    menu_style.secondary_font_size = layout.compact ? 10.0f : 10.0f;
    const int category_index = std::clamp( cfg.archive_category_index ? *cfg.archive_category_index : 0, 0, 2 );
    int entry_count = 0;
    const char* const* entry_names = nullptr;
    const vxui_demo_shared_archive_entry* entries = vxui_demo_shared_archive_entries_for_category( category_index, &entry_count, &entry_names );
    const int entry_index = std::clamp( cfg.archive_entry_index ? *cfg.archive_entry_index : 0, 0, std::max( 0, entry_count - 1 ) );
    const vxui_demo_shared_archive_entry& entry = entries[ entry_index ];
    const char* category_name = VXUI_DEMO_SHARED_ARCHIVE_CATEGORY_NAMES[ category_index ];
    const char* title_text = vxui_demo_ops_resolve_text( ctx, "screen.archives" );
    const char* access_text = entry.unlocked ? "ACCESS GRANTED" : "RESTRICTED";

    if ( cfg.archive_entry_index ) {
        *cfg.archive_entry_index = entry_index;
    }

    if ( cfg.background_scanline ) {
        vxui_demo_shared_emit_surface_scanline( ctx, "archives" );
    }

    vxui_menu_surface_cfg surface_cfg = vxui_menu_surface_cfg_default( layout.surface.surface_width, layout.surface_max_height, theme.app_background_base, theme.app_background_base, { 0, 0, 0, 0 } );
    vxui_demo_make_surface_cfg_transparent( surface_cfg );
    vxui_menu_surface_begin( ctx, "archives", "archives.surface", &surface_cfg );
    {
        VXUI( ctx, "archives.shell", {
            .layout = {
                .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_GROW( 0 ) },
                .padding = { 10, 10, 8, 8 },
                .childGap = 10,
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
        } ) {
            vxui_demo_ops_emit_header( ctx, "archives", "ARCHIVE CHANNEL / DOSSIER", title_text, "Recovered documents, signal fragments, and locked-clearance trails.", visuals.section_font_id, visuals.title_font_id, visuals.body_font_id, scale, theme );

            VXUI( ctx, "archives.board", {
                .layout = {
                    .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_GROW( 0 ) },
                    .padding = { 10, 10, 8, 8 },
                    .childGap = ( uint16_t ) layout.deck_gap,
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                },
                .backgroundColor = vxui_demo_clay_color( theme.primary_panel_fill ),
                .cornerRadius = CLAY_CORNER_RADIUS( 0 ),
                .border = vxui_demo_panel_border( theme.primary_panel_border, 1 ),
            } ) {
                VXUI( ctx, "archives.summary.row", {
                    .layout = {
                        .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                        .childGap = 10,
                        .layoutDirection = CLAY_LEFT_TO_RIGHT,
                    },
                } ) {
                    VXUI( ctx, "archives.summary.hero", {
                        .layout = {
                            .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                            .padding = { 12, 14, 10, 10 },
                            .childGap = 5,
                            .layoutDirection = CLAY_TOP_TO_BOTTOM,
                        },
                        .backgroundColor = vxui_demo_clay_color( theme.primary_panel_fill ),
                        .cornerRadius = CLAY_CORNER_RADIUS( 0 ),
                        .border = vxui_demo_panel_border( theme.focused_row_border, 1 ),
                    } ) {
                        VXUI_LABEL( ctx, "ACTIVE DOSSIER", vxui_demo_text_style( visuals.section_font_id, scale.eyebrow_size, theme.section_text ) );
                        VXUI_LABEL( ctx, entry.title, vxui_demo_text_style( visuals.title_font_id, scale.title_size, theme.title_text ) );
                        VXUI_LABEL( ctx, entry.subtitle, vxui_demo_text_style( visuals.section_font_id, scale.section_size, theme.accent_cool ) );
                    }
                    VXUI( ctx, "archives.summary.state", {
                        .layout = {
                            .sizing = { CLAY_SIZING_FIXED( layout.tall_portrait ? 240.0f : 220.0f ), CLAY_SIZING_FIT( 0 ) },
                            .padding = { 10, 12, 8, 8 },
                            .childGap = 5,
                            .layoutDirection = CLAY_TOP_TO_BOTTOM,
                        },
                        .backgroundColor = vxui_demo_clay_color( theme.secondary_panel_fill ),
                        .cornerRadius = CLAY_CORNER_RADIUS( 0 ),
                        .border = vxui_demo_panel_border( theme.primary_panel_border, 1 ),
                    } ) {
                        VXUI_LABEL( ctx, "CLEARANCE", vxui_demo_text_style( visuals.section_font_id, scale.micro_size, theme.muted_text ) );
                        vxui_demo_ops_emit_chip( ctx, "archives.summary.state.badge", access_text, visuals.section_font_id, scale.micro_size, entry.unlocked ? theme.success_text : theme.alert_text, theme.secondary_panel_fill, theme.primary_panel_border );
                        VXUI_LABEL( ctx, category_name, vxui_demo_text_style( visuals.body_font_id, scale.body_size, theme.body_text ) );
                        VXUI_LABEL( ctx, entry.meta, vxui_demo_text_style( visuals.section_font_id, scale.micro_size, entry.unlocked ? theme.section_text : theme.warning_text ) );
                    }
                }

                VXUI( ctx, "archives.selector.card", {
                    .layout = {
                        .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                        .padding = { 6, 8, 4, 4 },
                        .childGap = 4,
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    },
                    .backgroundColor = vxui_demo_clay_color( theme.secondary_panel_fill ),
                    .cornerRadius = CLAY_CORNER_RADIUS( 0 ),
                    .border = vxui_demo_panel_border( theme.primary_panel_border, 1 ),
                } ) {
                    VXUI( ctx, "archives.selector.meta", {
                        .layout = {
                            .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                            .childGap = 6,
                            .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                            .layoutDirection = CLAY_LEFT_TO_RIGHT,
                        },
                    } ) {
                        VXUI_LABEL( ctx, "ARCHIVE INDEX", vxui_demo_text_style( visuals.section_font_id, scale.micro_size, theme.section_text ) );
                        VXUI_LABEL( ctx, category_name, vxui_demo_text_style( visuals.body_font_id, scale.micro_size, theme.body_text ) );
                        VXUI( ctx, "archives.selector.meta.spacer", {
                            .layout = { .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_GROW( 0 ) } },
                        } ) {}
                        VXUI_LABEL( ctx, entry.unlocked ? "LIVE FILE" : "SEALED FILE", vxui_demo_text_style( visuals.section_font_id, scale.micro_size, entry.unlocked ? theme.success_text : theme.alert_text ) );
                    }
                    vxui_menu_begin( ctx, cfg.menu_state, "archives.menu", ( vxui_menu_cfg ) { .style = &menu_style, .viewport_height = layout.tall_portrait ? 76.0f : 70.0f } );
                    vxui_menu_option( ctx, cfg.menu_state, "category", "Channel", cfg.archive_category_index, const_cast< const char** >( archive_category_short_names ), 3, ( vxui_menu_row_cfg ) { 0 }, ( vxui_option_cfg ) { 0 } );
                    vxui_menu_option( ctx, cfg.menu_state, "entry", "File", cfg.archive_entry_index, const_cast< const char** >( entry_names ), entry_count, ( vxui_menu_row_cfg ) { 0 }, ( vxui_option_cfg ) { 0 } );
                    vxui_menu_end( ctx, cfg.menu_state );
                }

                VXUI( ctx, "archives.document.card", {
                    .layout = {
                        .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                        .padding = { 10, 12, 8, 8 },
                        .childGap = 6,
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    },
                    .backgroundColor = vxui_demo_clay_color( theme.primary_panel_fill ),
                    .cornerRadius = CLAY_CORNER_RADIUS( 0 ),
                    .border = vxui_demo_panel_border( theme.primary_panel_border, 1 ),
                } ) {
                    VXUI_LABEL( ctx, "DOCUMENT BODY", vxui_demo_text_style( visuals.section_font_id, scale.section_size, theme.section_text ) );
                    vxui_demo_emit_main_menu_divider( ctx, "archives.document.divider", theme.primary_panel_border );
                    VXUI_LABEL( ctx, entry.unlocked ? entry.detail : "Clearance gate still holds. Partial metadata is exposed, but the full body remains sealed behind archive permissions.", vxui_demo_text_style( visuals.body_font_id, scale.body_size, entry.unlocked ? theme.body_text : theme.muted_text ) );
                    VXUI_LABEL( ctx, entry.unlocked ? "Trace indicates the packet was attached to the next sortie corridor and stamped for relay justification." : "Archive body unavailable. Metadata shell only.", vxui_demo_text_style( visuals.section_font_id, scale.micro_size, entry.unlocked ? theme.section_text : theme.warning_text ) );
                }

                VXUI( ctx, "archives.return.band", {
                    .layout = {
                        .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                        .padding = { 4, 0, 0, 0 },
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    },
                } ) {
                    vxui_menu_begin( ctx, cfg.menu_state, "archives.return.menu", ( vxui_menu_cfg ) { .style = &menu_style, .viewport_height = layout.compact ? 28.0f : 30.0f } );
                    vxui_menu_action( ctx, cfg.menu_state, "back", "menu.back", cfg.back_fn, ( vxui_menu_row_cfg ) { .secondary_key = "RETURN ARCHIVE BANK" }, cfg.back_cfg );
                    vxui_menu_end( ctx, cfg.menu_state );
                }
            }
            vxui_demo_ops_emit_footer( ctx, "archives", cfg.status, visuals.section_font_id, scale, theme );
        }
    }
    vxui_menu_surface_end( ctx );
}

inline void vxui_demo_render_records_screen_ops( vxui_ctx* ctx, const vxui_demo_split_deck_visuals& visuals, const vxui_demo_records_screen_cfg& cfg )
{
    const vxui_demo_command_deck_theme& theme = vxui_demo_command_deck_theme_tokens();
    const vxui_demo_ops_screen_setup setup = vxui_demo_ops_init_screen( ctx, visuals, VXUI_DEMO_SURFACE_RECORDS, cfg.layout_surface_max_height_override, 88.0f, 80.0f );
    const vxui_demo_ops_layout_spec& layout = setup.layout;
    const vxui_demo_ops_type_scale& scale = setup.scale;
    vxui_menu_style menu_style = setup.menu_style;
    menu_style.row_height = layout.compact ? 28.0f : layout.tall_portrait ? 30.0f : 29.0f;
    menu_style.padding_x = layout.compact ? 8.0f : 10.0f;
    menu_style.padding_y = layout.compact ? 5.0f : 6.0f;
    menu_style.body_font_size = layout.compact ? 12.0f : 13.0f;
    const int board_index = std::clamp( cfg.records_board_index ? *cfg.records_board_index : 0, 0, 2 );
    const int record_index = std::clamp( cfg.records_entry_index ? *cfg.records_entry_index : 0, 0, 3 );
    const vxui_demo_shared_record& record = VXUI_DEMO_SHARED_RECORDS[ record_index ];
    const char* board_name = VXUI_DEMO_SHARED_RECORD_BOARD_NAMES[ board_index ];
    const char* title_text = vxui_demo_ops_resolve_text( ctx, "screen.records" );

    if ( cfg.records_board_index ) {
        *cfg.records_board_index = board_index;
    }
    if ( cfg.records_entry_index ) {
        *cfg.records_entry_index = record_index;
    }

    if ( cfg.background_scanline ) {
        vxui_demo_shared_emit_surface_scanline( ctx, "records" );
    }

    vxui_menu_surface_cfg surface_cfg = vxui_menu_surface_cfg_default( layout.surface.surface_width, layout.surface_max_height, theme.app_background_base, theme.app_background_base, { 0, 0, 0, 0 } );
    vxui_demo_make_surface_cfg_transparent( surface_cfg );
    vxui_menu_surface_begin( ctx, "records", "records.surface", &surface_cfg );
    {
        VXUI( ctx, "records.shell", {
            .layout = {
                .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_GROW( 0 ) },
                .padding = { 10, 10, 8, 8 },
                .childGap = 10,
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
        } ) {
            vxui_demo_ops_emit_header( ctx, "records", "AFTER ACTION / BOARD TRACE", title_text, "Scored clears, prototype runs, and debrief-state snapshots.", visuals.section_font_id, visuals.title_font_id, visuals.body_font_id, scale, theme );

            VXUI( ctx, "records.board", {
                .layout = {
                    .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_GROW( 0 ) },
                    .padding = { 10, 10, 8, 8 },
                    .childGap = ( uint16_t ) layout.deck_gap,
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                },
                .backgroundColor = vxui_demo_clay_color( theme.primary_panel_fill ),
                .cornerRadius = CLAY_CORNER_RADIUS( 0 ),
                .border = vxui_demo_panel_border( theme.primary_panel_border, 1 ),
            } ) {
                VXUI( ctx, "records.summary.row", {
                    .layout = {
                        .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                        .childGap = 10,
                        .layoutDirection = CLAY_LEFT_TO_RIGHT,
                    },
                } ) {
                    VXUI( ctx, "records.summary.hero", {
                        .layout = {
                            .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                            .padding = { 12, 14, 10, 10 },
                            .childGap = 4,
                            .layoutDirection = CLAY_TOP_TO_BOTTOM,
                        },
                        .backgroundColor = vxui_demo_clay_color( theme.primary_panel_fill ),
                        .cornerRadius = CLAY_CORNER_RADIUS( 0 ),
                        .border = vxui_demo_panel_border( theme.focused_row_border, 1 ),
                    } ) {
                        VXUI_LABEL( ctx, "RANKED CLEAR", vxui_demo_text_style( visuals.section_font_id, scale.eyebrow_size, theme.section_text ) );
                        VXUI_LABEL( ctx, record.score_text, vxui_demo_text_style( visuals.title_font_id, scale.title_size + 8.0f, theme.title_text ) );
                        VXUI_LABEL( ctx, record.run_name, vxui_demo_text_style( visuals.section_font_id, scale.section_size, theme.accent_cool ) );
                    }
                    VXUI( ctx, "records.summary.state", {
                        .layout = {
                            .sizing = { CLAY_SIZING_FIXED( layout.tall_portrait ? 228.0f : 212.0f ), CLAY_SIZING_FIT( 0 ) },
                            .padding = { 10, 12, 8, 8 },
                            .childGap = 5,
                            .layoutDirection = CLAY_TOP_TO_BOTTOM,
                        },
                        .backgroundColor = vxui_demo_clay_color( theme.secondary_panel_fill ),
                        .cornerRadius = CLAY_CORNER_RADIUS( 0 ),
                        .border = vxui_demo_panel_border( theme.primary_panel_border, 1 ),
                    } ) {
                        VXUI_LABEL( ctx, "BOARD STATE", vxui_demo_text_style( visuals.section_font_id, scale.micro_size, theme.muted_text ) );
                        vxui_demo_ops_emit_chip( ctx, "records.summary.state.badge", record.clear_text, visuals.section_font_id, scale.micro_size, theme.success_text, theme.secondary_panel_fill, theme.primary_panel_border );
                        VXUI_LABEL( ctx, board_name, vxui_demo_text_style( visuals.body_font_id, scale.body_size, theme.body_text ) );
                        VXUI_LABEL( ctx, "TRACE LOCKED", vxui_demo_text_style( visuals.section_font_id, scale.micro_size, theme.badge_text ) );
                    }
                }

                VXUI( ctx, "records.selector.card", {
                    .layout = {
                        .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                        .padding = { 6, 8, 4, 4 },
                        .childGap = 4,
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    },
                    .backgroundColor = vxui_demo_clay_color( theme.secondary_panel_fill ),
                    .cornerRadius = CLAY_CORNER_RADIUS( 0 ),
                    .border = vxui_demo_panel_border( theme.primary_panel_border, 1 ),
                } ) {
                    VXUI( ctx, "records.selector.meta", {
                        .layout = {
                            .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                            .childGap = 6,
                            .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                            .layoutDirection = CLAY_LEFT_TO_RIGHT,
                        },
                    } ) {
                        VXUI_LABEL( ctx, "BOARD SELECTOR", vxui_demo_text_style( visuals.section_font_id, scale.micro_size, theme.section_text ) );
                        VXUI_LABEL( ctx, board_name, vxui_demo_text_style( visuals.body_font_id, scale.micro_size, theme.body_text ) );
                        VXUI( ctx, "records.selector.meta.spacer", {
                            .layout = { .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_GROW( 0 ) } },
                        } ) {}
                        VXUI_LABEL( ctx, record.run_name, vxui_demo_text_style( visuals.section_font_id, scale.micro_size, theme.section_text ) );
                    }
                    vxui_menu_begin( ctx, cfg.menu_state, "records.menu", ( vxui_menu_cfg ) { .style = &menu_style, .viewport_height = layout.tall_portrait ? 76.0f : 70.0f } );
                    vxui_menu_option( ctx, cfg.menu_state, "board", "Board", cfg.records_board_index, const_cast< const char** >( VXUI_DEMO_SHARED_RECORD_BOARD_NAMES ), 3, ( vxui_menu_row_cfg ) { 0 }, ( vxui_option_cfg ) { 0 } );
                    vxui_menu_option( ctx, cfg.menu_state, "run", "Run", cfg.records_entry_index, const_cast< const char** >( VXUI_DEMO_SHARED_RECORD_NAMES ), 4, ( vxui_menu_row_cfg ) { 0 }, ( vxui_option_cfg ) { 0 } );
                    vxui_menu_end( ctx, cfg.menu_state );
                }

                VXUI_HASH( ctx, vxui_id( "records.identity.strip" ), {
                    .layout = {
                        .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                        .childGap = 8,
                        .layoutDirection = CLAY_LEFT_TO_RIGHT,
                    },
                } ) {
                    const struct { const char* label; const char* value; } identity_items[] = {
                        { "PILOT", record.pilot_name },
                        { "FRAME", record.ship_name },
                        { "STAGE", record.stage_name },
                    };
                    for ( int i = 0; i < 3; ++i ) {
                        VXUI_HASH( ctx, vxui_idi( "records.identity.strip", i ), {
                            .layout = {
                                .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                                .padding = { 8, 10, 6, 6 },
                                .childGap = 4,
                                .layoutDirection = CLAY_TOP_TO_BOTTOM,
                            },
                            .backgroundColor = vxui_demo_clay_color( theme.secondary_panel_fill ),
                            .cornerRadius = CLAY_CORNER_RADIUS( 0 ),
                            .border = vxui_demo_panel_border( theme.primary_panel_border, 1 ),
                        } ) {
                            VXUI_LABEL( ctx, identity_items[ i ].label, vxui_demo_text_style( visuals.section_font_id, scale.micro_size, theme.muted_text ) );
                            VXUI_LABEL( ctx, identity_items[ i ].value, vxui_demo_text_style( visuals.body_font_id, scale.body_size, theme.body_text ) );
                        }
                    }
                }

                VXUI( ctx, "records.performance.card", {
                    .layout = {
                        .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                        .padding = { 10, 12, 8, 8 },
                        .childGap = 6,
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    },
                    .backgroundColor = vxui_demo_clay_color( theme.secondary_panel_fill ),
                    .cornerRadius = CLAY_CORNER_RADIUS( 0 ),
                    .border = vxui_demo_panel_border( theme.primary_panel_border, 1 ),
                } ) {
                    VXUI_LABEL( ctx, "PERFORMANCE DIGEST", vxui_demo_text_style( visuals.section_font_id, scale.section_size, theme.section_text ) );
                    VXUI_LABEL( ctx, "Rank holds because the route stays clean through the mid-board pressure spike and never drops rhythm on the exit lane.", vxui_demo_text_style( visuals.body_font_id, scale.body_size, theme.body_text ) );
                    VXUI_LABEL( ctx, record.note, vxui_demo_text_style( visuals.section_font_id, scale.micro_size, theme.badge_text ) );
                }

                VXUI( ctx, "records.return.band", {
                    .layout = {
                        .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                        .padding = { 4, 0, 0, 0 },
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    },
                } ) {
                    vxui_menu_begin( ctx, cfg.menu_state, "records.return.menu", ( vxui_menu_cfg ) { .style = &menu_style, .viewport_height = layout.compact ? 28.0f : 30.0f } );
                    vxui_menu_action( ctx, cfg.menu_state, "back", "menu.back", cfg.back_fn, ( vxui_menu_row_cfg ) { .secondary_key = "RETURN SCORE BOARD" }, cfg.back_cfg );
                    vxui_menu_end( ctx, cfg.menu_state );
                }
            }
            vxui_demo_ops_emit_footer( ctx, "records", cfg.status, visuals.section_font_id, scale, theme );
        }
    }
    vxui_menu_surface_end( ctx );
}
