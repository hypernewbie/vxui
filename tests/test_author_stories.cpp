// User-story layout tests for VXUI authored screens.

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include <io.h>

#include "../demo/internal/layout_contract.h"
#include "../demo/internal/layout_validation.h"
#include "../demo/internal/main_menu_shared.h"
#include "../demo/internal/split_deck_shared.h"
#include "../third_party/utest.h"
#include "../vxui.h"
#include "../vxui_menu.h"
#include "test_story_helpers.h"
#include "test_support.h"

enum
{
    STORY_FONT_UI = 0,
    STORY_FONT_TITLE = 1,
    STORY_FONT_DEBUG = 2,
    STORY_FONT_JAPANESE = 3,
    STORY_FONT_ARABIC = 4,
    STORY_FONT_UI_LARGE = 5,
    STORY_FONT_JAPANESE_TITLE = 6,
    STORY_FONT_ARABIC_TITLE = 7,
    STORY_FONT_ROLE_BODY = 100,
    STORY_FONT_ROLE_TITLE = 101,
};

typedef struct author_story_fixture
{
    uint8_t* memory;
    uint64_t memory_size;
    vxui_ctx ctx;
    vxui_test_fontcache_handle* fontcache;
    vxui_input_table keyboard_table;
    vxui_menu_state main_menu_state;
    vxui_menu_state sortie_menu_state;
    vxui_menu_state loadout_menu_state;
    vxui_menu_state archives_menu_state;
    vxui_menu_state records_menu_state;
    int difficulty;
    int selected_mission_index;
    int selected_ship_index;
    int selected_primary_index;
    int selected_support_index;
    int selected_system_index;
    int archive_category_index;
    int archive_entry_index;
    int records_board_index;
    int records_entry_index;
    int string_count;
    char string_storage[ 256 ][ 2048 ];
} author_story_fixture;

static const vxui_demo_split_deck_visuals STORY_SPLIT_DECK_VISUALS = {
    STORY_FONT_ROLE_BODY,
    STORY_FONT_ROLE_TITLE,
    STORY_FONT_ROLE_BODY,
};

static const char* story_store( author_story_fixture* f, const char* text )
{
    if ( !text || f->string_count >= 256 ) {
        return "";
    }
    char* slot = f->string_storage[ f->string_count++ ];
    std::strncpy( slot, text, 2047 );
    slot[ 2047 ] = '\0';
    return slot;
}

static const char* story_store( author_story_fixture* f, const std::string& text )
{
    return story_store( f, text.c_str() );
}

static const char* story_localized( const char* locale, const char* en, const char* ja, const char* ar )
{
    if ( locale && std::strncmp( locale, "ja", 2 ) == 0 ) {
        return ja;
    }
    if ( locale && std::strncmp( locale, "ar", 2 ) == 0 ) {
        return ar;
    }
    return en;
}

static void story_font_resolver(
    vxui_ctx* ctx, uint32_t requested_font_id, float requested_font_size,
    const char* locale, void* userdata, vxui_resolved_font* out )
{
    ( void ) ctx;
    ( void ) userdata;
    if ( !out ) {
        return;
    }

    switch ( requested_font_id ) {
        case STORY_FONT_ROLE_BODY:
            out->font_id = requested_font_size >= 40.0f ? STORY_FONT_UI_LARGE : STORY_FONT_UI;
            out->line_height = std::max( requested_font_size + 4.0f, requested_font_size * 1.25f );
            return;
        case STORY_FONT_ROLE_TITLE:
            if ( locale && std::strncmp( locale, "ja", 2 ) == 0 ) {
                out->font_id = STORY_FONT_JAPANESE_TITLE;
            } else if ( locale && std::strncmp( locale, "ar", 2 ) == 0 ) {
                out->font_id = STORY_FONT_ARABIC_TITLE;
            } else {
                out->font_id = STORY_FONT_TITLE;
            }
            out->line_height = std::max( requested_font_size + 6.0f, requested_font_size * 1.18f );
            return;
        default:
            return;
    }
}

static Clay_String story_clay_string( const char* text )
{
    return ( Clay_String ) {
        .isStaticallyAllocated = false,
        .length = text ? ( int32_t ) std::strlen( text ) : 0,
        .chars = text ? text : "",
    };
}

static bool story_find_element( const char* id, vxui_rect* out )
{
    Clay_ElementData data = Clay_GetElementData( Clay_GetElementId( story_clay_string( id ) ) );
    if ( !data.found ) {
        return false;
    }
    if ( out ) {
        *out = { data.boundingBox.x, data.boundingBox.y, data.boundingBox.width, data.boundingBox.height };
    }
    return true;
}

static bool story_find_clay_hash( uint32_t id, vxui_rect* out )
{
    Clay_ElementData data = Clay_GetElementData( ( Clay_ElementId ) { .id = id, .offset = 0, .baseId = 0, .stringId = { 0 } } );
    if ( !data.found ) {
        return false;
    }
    if ( out ) {
        *out = { data.boundingBox.x, data.boundingBox.y, data.boundingBox.width, data.boundingBox.height };
    }
    return true;
}

static bool story_find_anim( const vxui_ctx* ctx, uint32_t id, vxui_rect* out )
{
    if ( !ctx || !ctx->anim_slots || ctx->anim_capacity <= 0 ) {
        return false;
    }
    for ( int i = 0; i < ctx->anim_capacity; ++i ) {
        const vxui_anim_slot* slot = &ctx->anim_slots[ i ];
        if ( !slot->occupied || slot->state.id != id ) {
            continue;
        }
        if ( out ) {
            *out = slot->state.bounds;
        }
        return true;
    }
    return false;
}

static bool story_has_text( const vxui_draw_list* list, const char* text )
{
    if ( !list || !text ) {
        return false;
    }
    for ( int i = 0; i < list->length; ++i ) {
        const vxui_cmd& cmd = list->commands[ i ];
        if ( cmd.type == VXUI_CMD_TEXT && cmd.text.text && std::strcmp( cmd.text.text, text ) == 0 ) {
            return true;
        }
    }
    return false;
}

static bool story_has_text_in_region( const vxui_draw_list* list, vxui_rect region )
{
    if ( !list ) {
        return false;
    }
    return vxui_demo_text_group_fully_visible( list, region, [&]( const vxui_cmd& cmd ) {
        const float x = cmd.text.pos.x;
        const float y = cmd.text.pos.y;
        return x >= region.x - 1.0f
            && x <= region.x + region.w + 1.0f
            && y >= region.y - 1.0f
            && y <= region.y + region.h + 1.0f;
    } );
}

static bool story_rect_reads_inline_pair( vxui_rect rect )
{
    return story_rect_is_readable( rect ) && rect.w > rect.h * 1.5f;
}

static uint32_t story_menu_row_id( const char* menu_id, const char* row_id )
{
    return vxui_idi( menu_id ? menu_id : "", ( int ) vxui_id( row_id ? row_id : "" ) );
}

static uint32_t story_menu_mix_id( uint32_t base, uint32_t salt )
{
    uint32_t value = base ^ ( salt * 0x9E3779B9u ) ^ 0x7F4A7C15u;
    if ( value == 0 ) {
        value = salt ? salt : 1u;
    }
    return value;
}

static bool story_find_menu_row( const author_story_fixture* f, const char* menu_id, const char* row_id, vxui_rect* out )
{
    return story_find_anim( &f->ctx, story_menu_row_id( menu_id, row_id ), out );
}

static bool story_find_menu_label_lane( const char* menu_id, const char* row_id, vxui_rect* out )
{
    return story_find_clay_hash( story_menu_mix_id( story_menu_row_id( menu_id, row_id ), 2u ), out );
}

static bool story_find_menu_value_lane( const char* menu_id, const char* row_id, vxui_rect* out )
{
    return story_find_clay_hash( story_menu_mix_id( story_menu_row_id( menu_id, row_id ), 3u ), out );
}

static bool story_find_menu_value_group( const char* menu_id, const char* row_id, vxui_rect* out )
{
    return story_find_clay_hash( story_menu_mix_id( story_menu_row_id( menu_id, row_id ), 11u ), out );
}

static bool story_find_menu_badge( const char* menu_id, const char* row_id, vxui_rect* out )
{
    return story_find_clay_hash( story_menu_mix_id( story_menu_row_id( menu_id, row_id ), 17u ), out );
}

template <typename TEmit>
static std::string story_capture_stderr( TEmit&& emit )
{
    FILE* tmp = nullptr;
    if ( tmpfile_s( &tmp ) != 0 || !tmp ) {
        emit();
        return "";
    }

    std::fflush( stderr );
    const int stderr_fd = _dup( _fileno( stderr ) );
    if ( stderr_fd < 0 ) {
        std::fclose( tmp );
        emit();
        return "";
    }

    _dup2( _fileno( tmp ), _fileno( stderr ) );
    emit();
    std::fflush( stderr );
    _dup2( stderr_fd, _fileno( stderr ) );
    _close( stderr_fd );

    std::fseek( tmp, 0, SEEK_SET );
    std::string output;
    char buffer[ 256 ] = {};
    while ( std::fgets( buffer, sizeof( buffer ), tmp ) ) {
        output += buffer;
    }
    std::fclose( tmp );
    return output;
}

template <typename TEmit>
static void story_emit_screen_surface(
    author_story_fixture* f, const char* root_id, const char* surface_id,
    float w, float h, bool rtl, TEmit&& emit )
{
    ( void ) rtl;
    const vxui_menu_surface_cfg cfg = {
        w,
        h,
        ( float ) VXUI_DEMO_LAYOUT_OUTER_PADDING,
        ( float ) VXUI_DEMO_LAYOUT_SURFACE_PADDING_X,
        ( float ) VXUI_DEMO_LAYOUT_SURFACE_PADDING_Y,
        ( float ) VXUI_DEMO_LAYOUT_SECTION_GAP,
        18.0f,
        1.0f,
        { 0, 0, 0, 0 },
        { 0, 0, 0, 0 },
        { 0, 0, 0, 0 },
    };
    vxui_menu_surface_begin( &f->ctx, root_id, surface_id, &cfg );
    emit();
    vxui_menu_surface_end( &f->ctx );
}

static void story_reset_state( author_story_fixture* f )
{
    f->main_menu_state = {};
    f->sortie_menu_state = {};
    f->loadout_menu_state = {};
    f->archives_menu_state = {};
    f->records_menu_state = {};
    f->difficulty = 1;
    f->selected_mission_index = 0;
    f->selected_ship_index = 0;
    f->selected_primary_index = 0;
    f->selected_support_index = 0;
    f->selected_system_index = 0;
    f->archive_category_index = 0;
    f->archive_entry_index = 0;
    f->records_board_index = 0;
    f->records_entry_index = 0;
}

static void story_begin( author_story_fixture* f, int width, int height, const char* locale, uint32_t focused_id = 0 )
{
    f->string_count = 0;
    f->ctx.cfg.screen_width = width;
    f->ctx.cfg.screen_height = height;
    vxui_set_font_resolver( &f->ctx, story_font_resolver, f );
    vxui_set_locale( &f->ctx, locale );
    vxui_set_input_table( &f->ctx, &f->keyboard_table );
    f->ctx.focused_id = focused_id;
    vxui_begin( &f->ctx, 0.016f );
}

static vxui_draw_list story_end( author_story_fixture* f )
{
    return vxui_end( &f->ctx );
}

static vxui_demo_status_summary_cfg story_status_cfg( const char* locale, const char* top_name_key )
{
    return ( vxui_demo_status_summary_cfg ) {
        .locale_name_key = vxui_demo_shared_footer_locale_key_from_locale( locale ),
        .prompt_name_key = vxui_demo_shared_footer_prompt_name_key( 0 ),
        .top_name_key = top_name_key,
        .screen_count = 1,
    };
}

static vxui_draw_list story_render_main_menu(
    author_story_fixture* f, int width, int height, const char* locale,
    const story_content_pack& pack, uint32_t focused_row_id = 0 )
{
    story_begin( f, width, height, locale, focused_row_id );
    const bool rtl = f->ctx.rtl;
    const float vp_width = std::max( 0.0f, ( float ) width - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f );
    const float surf_height = std::max( 0.0f, ( float ) height - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f );
    const vxui_demo_main_menu_layout_spec layout =
        vxui_demo_resolve_main_menu_layout( vp_width, surf_height, locale );
    vxui_menu_style menu_style = vxui_demo_make_title_deck_menu_style( STORY_FONT_ROLE_BODY, STORY_FONT_ROLE_TITLE );
    if ( layout.surface_max_height <= 650.0f ) {
        menu_style.body_font_size = 14.0f;
        menu_style.secondary_font_size = 12.0f;
        menu_style.badge_font_size = 8.0f;
        menu_style.row_height = 25.0f;
        menu_style.row_gap = 0.0f;
        menu_style.section_gap = 2.0f;
        menu_style.padding_y = 3.0f;
    }

    const vxui_demo_main_menu_preview* preview =
        focused_row_id ? vxui_demo_main_menu_preview_from_focused_row( focused_row_id ) : &VXUI_DEMO_MAIN_MENU_PREVIEWS[ 0 ];
    if ( !preview ) {
        preview = &VXUI_DEMO_MAIN_MENU_PREVIEWS[ 0 ];
    }

    f->ctx.focused_id = focused_row_id;
    story_emit_screen_surface( f, "main_menu", "main.surface", layout.surface.surface_width, layout.surface_max_height, rtl, [&]() {
        vxui_demo_emit_main_menu_shell(
            &f->ctx, rtl, locale, layout,
            ( vxui_demo_main_menu_visuals ) { STORY_FONT_ROLE_BODY, STORY_FONT_ROLE_TITLE, STORY_FONT_ROLE_BODY },
            ( vxui_demo_main_menu_shell_copy ) {
                pack.title, pack.subtitle,
                vxui_demo_main_menu_preview_label( locale ),
                story_store( f, story_localized( locale, "Loc", "言語", "لغة" ) ),
                story_store( f, story_localized( locale, "EN", "JA", "AR" ) ),
                story_store( f, story_localized( locale, "Input", "入力", "دخل" ) ),
                story_store( f, story_localized( locale, "KB", "KB", "KB" ) ),
                story_store( f, story_localized( locale, "Views", "画面", "شاشات" ) ),
                1,
                story_store( f, story_localized( locale, "Top", "上位", "أعلى" ) ),
                story_store( f, story_localized( locale, "Deck", "デッキ", "السطح" ) ),
                pack.footer_prompt, "Cancel",
            },
            *preview,
            [&]( float viewport_height ) {
                vxui_menu_begin( &f->ctx, &f->main_menu_state, "main.command_menu", ( vxui_menu_cfg ) {
                    .style = &menu_style,
                    .viewport_height = viewport_height,
                } );
                for ( int i = 0; i < pack.row_count; ++i ) {
                    const story_menu_row_content& row = pack.rows[ i ];
                    std::string row_id = "row_" + std::to_string( i );
                    vxui_menu_action( &f->ctx, &f->main_menu_state,
                        story_store( f, row_id ), row.label, nullptr,
                        ( vxui_menu_row_cfg ) {
                            .value_key = row.value,
                            .secondary_key = row.secondary,
                            .badge_text_key = row.badge,
                        },
                        ( vxui_action_cfg ) { 0 } );
                }
                vxui_menu_end( &f->ctx, &f->main_menu_state );
            } );
    } );
    return story_end( f );
}

static vxui_draw_list story_render_sortie( author_story_fixture* f, int width, int height, const char* locale, uint32_t focused_id = 0 )
{
    story_begin( f, width, height, locale, focused_id );
    vxui_demo_render_sortie_screen_shared( &f->ctx, STORY_SPLIT_DECK_VISUALS,
        ( vxui_demo_sortie_screen_cfg ) {
            .menu_state = &f->sortie_menu_state,
            .selected_mission_index = &f->selected_mission_index,
            .difficulty_index = &f->difficulty,
            .start_fn = nullptr,
            .start_cfg = ( vxui_action_cfg ) { 0 },
            .back_fn = nullptr,
            .back_cfg = ( vxui_action_cfg ) { 0 },
            .status = story_status_cfg( locale, "screen.sortie" ),
        } );
    return story_end( f );
}

static vxui_draw_list story_render_loadout( author_story_fixture* f, int width, int height, const char* locale )
{
    story_begin( f, width, height, locale );
    vxui_demo_render_loadout_screen_shared( &f->ctx, STORY_SPLIT_DECK_VISUALS,
        ( vxui_demo_loadout_screen_cfg ) {
            .menu_state = &f->loadout_menu_state,
            .selected_ship_index = &f->selected_ship_index,
            .selected_primary_index = &f->selected_primary_index,
            .selected_support_index = &f->selected_support_index,
            .selected_system_index = &f->selected_system_index,
            .back_fn = nullptr,
            .back_cfg = ( vxui_action_cfg ) { 0 },
            .status = story_status_cfg( locale, "screen.loadout" ),
        } );
    return story_end( f );
}

static vxui_draw_list story_render_archives( author_story_fixture* f, int width, int height, const char* locale )
{
    story_begin( f, width, height, locale );
    vxui_demo_render_archives_screen_shared( &f->ctx, STORY_SPLIT_DECK_VISUALS,
        ( vxui_demo_archives_screen_cfg ) {
            .menu_state = &f->archives_menu_state,
            .archive_category_index = &f->archive_category_index,
            .archive_entry_index = &f->archive_entry_index,
            .back_fn = nullptr,
            .back_cfg = ( vxui_action_cfg ) { 0 },
            .status = story_status_cfg( locale, "screen.archives" ),
        } );
    return story_end( f );
}

static vxui_draw_list story_render_records( author_story_fixture* f, int width, int height, const char* locale )
{
    story_begin( f, width, height, locale );
    vxui_demo_render_records_screen_shared( &f->ctx, STORY_SPLIT_DECK_VISUALS,
        ( vxui_demo_records_screen_cfg ) {
            .menu_state = &f->records_menu_state,
            .records_board_index = &f->records_board_index,
            .records_entry_index = &f->records_entry_index,
            .back_fn = nullptr,
            .back_cfg = ( vxui_action_cfg ) { 0 },
            .status = story_status_cfg( locale, "screen.records" ),
        } );
    return story_end( f );
}

static int story_main_menu_visible_help_line_count( int width, int height, const char* locale )
{
    const float surf_h = std::max( 0.0f, ( float ) height - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f );
    const float vp_w = std::max( 0.0f, ( float ) width - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f );
    const vxui_demo_main_menu_layout_spec layout = vxui_demo_resolve_main_menu_layout( vp_w, surf_h, locale );
    const bool tight_preview_width = layout.preview_panel_width <= 420.0f;
    const bool compact_help = surf_h <= 648.0f || tight_preview_width;
    const vxui_demo_controls_block_copy copy = vxui_demo_controls_block_copy_for_locale( locale, compact_help );
    const int line_count = vxui_demo_controls_block_visible_line_count( copy );
    if ( tight_preview_width ) {
        return std::min( line_count, 2 );
    }
    return compact_help ? std::min( line_count, 3 ) : line_count;
}

UTEST_F_SETUP( author_story_fixture )
{
    utest_fixture->memory_size = vxui_min_memory_size();
    utest_fixture->memory = ( uint8_t* ) std::malloc( ( size_t ) utest_fixture->memory_size );
    ASSERT_TRUE( utest_fixture->memory != nullptr );

    vxui_init(
        &utest_fixture->ctx,
        vxui_create_arena( utest_fixture->memory_size, utest_fixture->memory ),
        ( vxui_config ) { .screen_width = 1280, .screen_height = 720, .max_elements = 1024 } );

    utest_fixture->fontcache = vxui_test_fontcache_create( true );
    ASSERT_TRUE( utest_fixture->fontcache != nullptr );
    EXPECT_EQ( vxui_test_fontcache_load_file( utest_fixture->fontcache, VXUI_SOURCE_DIR "/vefc/demo/fonts/OpenSans-Regular.ttf", 24.0f ), ( int64_t ) STORY_FONT_UI );
    EXPECT_EQ( vxui_test_fontcache_load_file( utest_fixture->fontcache, VXUI_SOURCE_DIR "/vefc/demo/fonts/Bitter-Regular.ttf", 44.0f ), ( int64_t ) STORY_FONT_TITLE );
    EXPECT_EQ( vxui_test_fontcache_load_file( utest_fixture->fontcache, VXUI_SOURCE_DIR "/vefc/demo/fonts/NovaMono-Regular.ttf", 16.0f ), ( int64_t ) STORY_FONT_DEBUG );
    EXPECT_EQ( vxui_test_fontcache_load_file( utest_fixture->fontcache, VXUI_SOURCE_DIR "/vefc/demo/fonts/NotoSansJP-Regular.otf", 24.0f ), ( int64_t ) STORY_FONT_JAPANESE );
    EXPECT_EQ( vxui_test_fontcache_load_file( utest_fixture->fontcache, VXUI_SOURCE_DIR "/vefc/demo/fonts/Tajawal-Regular.ttf", 24.0f ), ( int64_t ) STORY_FONT_ARABIC );
    EXPECT_EQ( vxui_test_fontcache_load_file( utest_fixture->fontcache, VXUI_SOURCE_DIR "/vefc/demo/fonts/OpenSans-Regular.ttf", 44.0f ), ( int64_t ) STORY_FONT_UI_LARGE );
    EXPECT_EQ( vxui_test_fontcache_load_file( utest_fixture->fontcache, VXUI_SOURCE_DIR "/vefc/demo/fonts/NotoSansJP-Regular.otf", 44.0f ), ( int64_t ) STORY_FONT_JAPANESE_TITLE );
    EXPECT_EQ( vxui_test_fontcache_load_file( utest_fixture->fontcache, VXUI_SOURCE_DIR "/vefc/demo/fonts/Tajawal-Regular.ttf", 44.0f ), ( int64_t ) STORY_FONT_ARABIC_TITLE );
    vxui_set_fontcache( &utest_fixture->ctx, vxui_test_fontcache_ptr( utest_fixture->fontcache ) );
    utest_fixture->ctx.default_font_id = STORY_FONT_UI;
    utest_fixture->ctx.default_font_size = 24.0f;

    utest_fixture->keyboard_table = {
        .confirm = { STORY_FONT_ROLE_BODY, ( uint32_t ) 'E' },
        .cancel = { STORY_FONT_ROLE_BODY, ( uint32_t ) 'Q' },
        .tab_left = { STORY_FONT_ROLE_BODY, ( uint32_t ) '[' },
        .tab_right = { STORY_FONT_ROLE_BODY, ( uint32_t ) ']' },
        .up = { STORY_FONT_ROLE_BODY, ( uint32_t ) '^' },
        .down = { STORY_FONT_ROLE_BODY, ( uint32_t ) 'v' },
        .left = { STORY_FONT_ROLE_BODY, ( uint32_t ) '<' },
        .right = { STORY_FONT_ROLE_BODY, ( uint32_t ) '>' },
    };

    story_reset_state( utest_fixture );
}

UTEST_F_TEARDOWN( author_story_fixture )
{
    vxui_shutdown( &utest_fixture->ctx );
    vxui_test_fontcache_destroy( utest_fixture->fontcache );
    std::free( utest_fixture->memory );
}

UTEST_F( author_story_fixture, story_prompts_render_with_valid_input_bindings )
{
    const story_content_pack pack = story_content_pack_en_normal();
    story_reset_state( utest_fixture );
    std::string warnings = story_capture_stderr( [&]() {
        ( void ) story_render_main_menu( utest_fixture, 1280, 720, "en", pack );
        story_reset_state( utest_fixture );
        ( void ) story_render_sortie( utest_fixture, 1280, 720, "en" );
    } );

    EXPECT_EQ( warnings.find( "missing prompt binding" ), std::string::npos );

    vxui_rect prompts = {};
    vxui_rect footer = {};
    ASSERT_TRUE( story_find_element( "sortie.footer", &footer ) );
    ASSERT_TRUE( story_find_element( "sortie.footer.prompts", &prompts ) );
    EXPECT_TRUE( story_element_fully_inside( footer, prompts, 1.0f ) );
}

UTEST_F( author_story_fixture, main_menu_required_help_elements_exist )
{
    const story_content_pack pack = story_content_pack_en_normal();
    const int compact_sizes[][ 2 ] = { { 1280, 680 }, { 1280, 648 }, { 1100, 648 } };

    for ( const auto& size : compact_sizes ) {
        story_reset_state( utest_fixture );
        ( void ) story_render_main_menu( utest_fixture, size[ 0 ], size[ 1 ], "en", pack );

        const float surf_h = std::max( 0.0f, ( float ) size[ 1 ] - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f );
        const float vp_w = std::max( 0.0f, ( float ) size[ 0 ] - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f );
        const vxui_demo_main_menu_layout_spec layout = vxui_demo_resolve_main_menu_layout( vp_w, surf_h, "en" );
        const float panel_pad = vxui_demo_main_menu_preview_uses_compact_layout( layout ) ? 8.0f : layout.preview_panel_padding;
        const float owner_w = std::max( 0.0f, layout.preview_panel_width - panel_pad * 2.0f );
        const int help_line_count = story_main_menu_visible_help_line_count( size[ 0 ], size[ 1 ], "en" );

        vxui_rect help = {}, title = {}, footer = {}, prompts = {}, status = {}, body = {};
        vxui_rect locale = {}, prompt_group = {}, screens = {}, top = {};
        ASSERT_TRUE( story_find_element( "main.preview.help_legend", &help ) );
        ASSERT_TRUE( story_find_element( vxui_demo_controls_block_title_id( "main.preview.help_legend" ).c_str(), &title ) );
        ASSERT_TRUE( story_find_element( "main.footer", &footer ) );
        ASSERT_TRUE( story_find_element( "main.footer.prompts", &prompts ) );
        ASSERT_TRUE( story_find_element( "main.footer.status", &status ) );
        ASSERT_TRUE( story_find_element( "main.preview_body", &body ) );
        ASSERT_TRUE( story_find_element( "main.footer.status.locale", &locale ) );
        ASSERT_TRUE( story_find_element( "main.footer.status.prompts", &prompt_group ) );
        ASSERT_TRUE( story_find_element( "main.footer.status.top", &top ) );
        const bool have_screens = story_find_element( "main.footer.status.screens", &screens );

        for ( int i = 0; i < help_line_count; ++i ) {
            vxui_rect line = {};
            ASSERT_TRUE( story_find_element( vxui_demo_controls_block_line_id( "main.preview.help_legend", i ).c_str(), &line ) );
        }
        ( void ) have_screens;
    }
}

UTEST_F( author_story_fixture, compact_help_lines_readable_all_locales )
{
    const char* locales[] = { "en", "ja-JP", "ar" };
    const int compact_sizes[][ 2 ] = { { 1280, 680 }, { 1280, 648 }, { 1100, 648 } };

    for ( const char* locale : locales ) {
        const story_content_pack pack =
            std::strcmp( locale, "ja-JP" ) == 0 ? story_content_pack_ja_normal() :
            std::strcmp( locale, "ar" ) == 0 ? story_content_pack_ar_normal() :
            story_content_pack_en_normal();

        for ( const auto& size : compact_sizes ) {
            story_reset_state( utest_fixture );
            ( void ) story_render_main_menu( utest_fixture, size[ 0 ], size[ 1 ], locale, pack );

            const float surf_h = std::max( 0.0f, ( float ) size[ 1 ] - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f );
            const float vp_w = std::max( 0.0f, ( float ) size[ 0 ] - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f );
            const vxui_demo_main_menu_layout_spec layout = vxui_demo_resolve_main_menu_layout( vp_w, surf_h, locale );
            const float panel_pad = vxui_demo_main_menu_preview_uses_compact_layout( layout ) ? 8.0f : layout.preview_panel_padding;
            const float owner_w = std::max( 0.0f, layout.preview_panel_width - panel_pad * 2.0f );
            const int help_line_count = story_main_menu_visible_help_line_count( size[ 0 ], size[ 1 ], locale );

            vxui_rect help = {}, title = {}, lines[ 4 ] = {};
            ASSERT_TRUE( story_find_element( "main.preview.help_legend", &help ) );
            ASSERT_TRUE( story_find_element( vxui_demo_controls_block_title_id( "main.preview.help_legend" ).c_str(), &title ) );
            for ( int i = 0; i < help_line_count; ++i ) {
                ASSERT_TRUE( story_find_element( vxui_demo_controls_block_line_id( "main.preview.help_legend", i ).c_str(), &lines[ i ] ) );
            }

            vxui_rect stack[ 5 ] = { title, lines[ 0 ], lines[ 1 ], lines[ 2 ], lines[ 3 ] };
            const int stack_count = 1 + help_line_count;
            EXPECT_TRUE( story_group_fully_visible( help, stack, stack_count, 1.0f ) );
            EXPECT_TRUE( story_group_no_overlaps( stack, stack_count ) );
            EXPECT_TRUE( story_readable_vertical_stack( stack, stack_count ) );
        }
    }
}

UTEST_F( author_story_fixture, regression_main_menu_preview_help_overlap )
{
    const story_content_pack pack = story_content_pack_en_normal();
    story_reset_state( utest_fixture );
    ( void ) story_render_main_menu( utest_fixture, 1280, 648, "en", pack );

    vxui_rect viewport = {}, help = {};
    ASSERT_TRUE( story_find_element( "main.preview_body", &viewport ) );
    ASSERT_TRUE( story_find_element( "main.preview.help_legend", &help ) );
    EXPECT_TRUE( story_vertical_order( viewport, help, 0.0f ) );
    EXPECT_TRUE( story_no_vertical_overlap( viewport, help ) );
}

UTEST_F( author_story_fixture, regression_main_menu_help_line_collision )
{
    const story_content_pack pack = story_content_pack_en_normal();
    const int compact_sizes[][ 2 ] = { { 1280, 680 }, { 1280, 648 }, { 1100, 648 } };

    for ( const auto& size : compact_sizes ) {
        story_reset_state( utest_fixture );
        ( void ) story_render_main_menu( utest_fixture, size[ 0 ], size[ 1 ], "en", pack );

        const float surf_h = std::max( 0.0f, ( float ) size[ 1 ] - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f );
        const float vp_w = std::max( 0.0f, ( float ) size[ 0 ] - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f );
        const vxui_demo_main_menu_layout_spec layout = vxui_demo_resolve_main_menu_layout( vp_w, surf_h, "en" );
        const float panel_pad = vxui_demo_main_menu_preview_uses_compact_layout( layout ) ? 8.0f : layout.preview_panel_padding;
        const float owner_w = std::max( 0.0f, layout.preview_panel_width - panel_pad * 2.0f );
        const int help_line_count = story_main_menu_visible_help_line_count( size[ 0 ], size[ 1 ], "en" );

        vxui_rect help = {}, title = {}, lines[ 4 ] = {};
        ASSERT_TRUE( story_find_element( "main.preview.help_legend", &help ) );
        ASSERT_TRUE( story_find_element( vxui_demo_controls_block_title_id( "main.preview.help_legend" ).c_str(), &title ) );
        for ( int i = 0; i < help_line_count; ++i ) {
            ASSERT_TRUE( story_find_element( vxui_demo_controls_block_line_id( "main.preview.help_legend", i ).c_str(), &lines[ i ] ) );
        }

        vxui_rect stack[ 5 ] = { title, lines[ 0 ], lines[ 1 ], lines[ 2 ], lines[ 3 ] };
        const int count = 1 + help_line_count;
        EXPECT_TRUE( story_group_no_overlaps( stack, count ) );
        EXPECT_TRUE( story_readable_vertical_stack( stack, count ) );
    }
}

UTEST_F( author_story_fixture, regression_footer_status_crowding )
{
    const story_content_pack pack = story_content_pack_en_normal();
    const int compact_sizes[][ 2 ] = { { 1280, 680 }, { 1280, 648 }, { 1100, 648 } };

    for ( const auto& size : compact_sizes ) {
        story_reset_state( utest_fixture );
        ( void ) story_render_main_menu( utest_fixture, size[ 0 ], size[ 1 ], "en", pack );

        vxui_rect footer = {}, prompts = {}, status = {};
        vxui_rect locale = {}, prompt_group = {}, screens = {}, top = {};
        ASSERT_TRUE( story_find_element( "main.footer", &footer ) );
        ASSERT_TRUE( story_find_element( "main.footer.prompts", &prompts ) );
        ASSERT_TRUE( story_find_element( "main.footer.status", &status ) );
        ASSERT_TRUE( story_find_element( "main.footer.status.locale", &locale ) );
        ASSERT_TRUE( story_find_element( "main.footer.status.prompts", &prompt_group ) );
        ASSERT_TRUE( story_find_element( "main.footer.status.top", &top ) );
        const bool have_screens = story_find_element( "main.footer.status.screens", &screens );
        EXPECT_TRUE( story_element_fully_inside( footer, prompts, 1.0f ) );
        EXPECT_TRUE( story_element_fully_inside( footer, status, 1.0f ) );
        EXPECT_TRUE( vxui_demo_rects_non_overlapping( prompts, status, 0.0f ) );
        EXPECT_TRUE( story_rect_is_readable( prompts ) );
        EXPECT_TRUE( story_rect_is_readable( status ) );
        EXPECT_TRUE( story_element_fully_inside( status, locale, 1.0f ) );
        EXPECT_TRUE( story_element_fully_inside( status, prompt_group, 1.0f ) );
        EXPECT_TRUE( story_element_fully_inside( status, top, 1.0f ) );
        EXPECT_TRUE( story_rect_reads_inline_pair( locale ) );
        EXPECT_TRUE( story_rect_reads_inline_pair( prompt_group ) );
        EXPECT_TRUE( story_rect_reads_inline_pair( top ) );
        ( void ) have_screens;
    }
}

UTEST_F( author_story_fixture, split_deck_story_uses_production_sortie_path )
{
    story_reset_state( utest_fixture );
    vxui_draw_list list = story_render_sortie( utest_fixture, 1280, 720, "en" );

    EXPECT_TRUE( story_has_text( &list, "OP-01 Aegis Break" ) );

    vxui_rect menu_panel = {}, briefing = {}, briefing_body = {}, detail = {}, footer = {};
    ASSERT_TRUE( story_find_element( "sortie.menu_panel", &menu_panel ) );
    ASSERT_TRUE( story_find_element( "sortie.briefing", &briefing ) );
    ASSERT_TRUE( story_find_element( "sortie.briefing.body", &briefing_body ) );
    EXPECT_TRUE( story_find_element( "sortie.detail", &detail ) || story_find_element( "sortie.briefing.meta", &detail ) );
    ASSERT_TRUE( story_find_element( "sortie.footer", &footer ) );
}

UTEST_F( author_story_fixture, split_deck_production_screens_readable_across_viewports )
{
    const int sizes[][ 2 ] = { { 1280, 720 }, { 1280, 648 }, { 1100, 648 } };

    for ( const auto& size : sizes ) {
        story_reset_state( utest_fixture );
        ( void ) story_render_sortie( utest_fixture, size[ 0 ], size[ 1 ], "en" );
        vxui_rect deck = {}, footer = {};
        ASSERT_TRUE( story_find_element( "sortie.menu_panel", &deck ) );
        ASSERT_TRUE( story_find_element( "sortie.footer", &footer ) );
        EXPECT_TRUE( story_vertical_order( deck, footer, 0.0f ) );

        story_reset_state( utest_fixture );
        ( void ) story_render_loadout( utest_fixture, size[ 0 ], size[ 1 ], "en" );
        ASSERT_TRUE( story_find_element( "loadout.menu_panel", &deck ) );
        ASSERT_TRUE( story_find_element( "loadout.detail.body", &deck ) );
        ASSERT_TRUE( story_find_element( "loadout.footer", &footer ) );
        EXPECT_TRUE( story_rect_is_readable( footer ) );

        story_reset_state( utest_fixture );
        ( void ) story_render_archives( utest_fixture, size[ 0 ], size[ 1 ], "en" );
        ASSERT_TRUE( story_find_element( "archives.menu_panel", &deck ) );
        ASSERT_TRUE( story_find_element( "archives.detail.body", &deck ) );
        ASSERT_TRUE( story_find_element( "archives.footer", &footer ) );
        EXPECT_TRUE( story_rect_is_readable( footer ) );

        story_reset_state( utest_fixture );
        ( void ) story_render_records( utest_fixture, size[ 0 ], size[ 1 ], "en" );
        ASSERT_TRUE( story_find_element( "records.menu_panel", &deck ) );
        ASSERT_TRUE( story_find_element( "records.detail.body", &deck ) );
        ASSERT_TRUE( story_find_element( "records.footer", &footer ) );
        EXPECT_TRUE( story_rect_is_readable( footer ) );
    }
}

UTEST_F( author_story_fixture, sortie_required_lane_and_footer_elements_exist )
{
    const int sizes[][ 2 ] = { { 1280, 720 }, { 1280, 680 }, { 1280, 648 }, { 1100, 720 }, { 1100, 648 } };
    const uint32_t focused_mission = story_menu_row_id( "sortie.menu", "mission" );

    for ( const auto& size : sizes ) {
        story_reset_state( utest_fixture );
        utest_fixture->selected_mission_index = 3;
        ( void ) story_render_sortie( utest_fixture, size[ 0 ], size[ 1 ], "en", focused_mission );

        vxui_rect menu_panel = {}, briefing = {}, briefing_vp = {}, detail = {}, footer = {}, prompts = {}, status = {};
        ASSERT_TRUE( story_find_element( "sortie.menu_panel", &menu_panel ) );
        ASSERT_TRUE( story_find_element( "sortie.briefing", &briefing ) );
        ASSERT_TRUE( story_find_element( "sortie.briefing.body", &briefing_vp ) );
        EXPECT_TRUE( story_find_element( "sortie.detail", &detail ) || story_find_element( "sortie.briefing.meta", &detail ) );
        ASSERT_TRUE( story_find_element( "sortie.footer", &footer ) );
        ASSERT_TRUE( story_find_element( "sortie.footer.prompts", &prompts ) );
        ASSERT_TRUE( story_find_element( "sortie.footer.status", &status ) );

        vxui_rect row = {}, label_lane = {}, value_lane = {}, value_group = {}, badge = {};
        ASSERT_TRUE( story_find_menu_row( utest_fixture, "sortie.menu", "mission", &row ) );
        ASSERT_TRUE( story_find_menu_label_lane( "sortie.menu", "mission", &label_lane ) );
        ASSERT_TRUE( story_find_menu_value_lane( "sortie.menu", "mission", &value_lane ) );
        ASSERT_TRUE( story_find_menu_value_group( "sortie.menu", "mission", &value_group ) );
        EXPECT_FALSE( story_find_menu_badge( "sortie.menu", "mission", &badge ) );

        ASSERT_TRUE( story_find_menu_row( utest_fixture, "sortie.menu", "difficulty", &row ) );
        ASSERT_TRUE( story_find_menu_label_lane( "sortie.menu", "difficulty", &label_lane ) );
        ASSERT_TRUE( story_find_menu_value_lane( "sortie.menu", "difficulty", &value_lane ) );
        ASSERT_TRUE( story_find_menu_value_group( "sortie.menu", "difficulty", &value_group ) );
        EXPECT_FALSE( story_find_menu_badge( "sortie.menu", "difficulty", &badge ) );

        ASSERT_TRUE( story_find_menu_row( utest_fixture, "sortie.menu", "start", &row ) );
        ASSERT_TRUE( story_find_menu_label_lane( "sortie.menu", "start", &label_lane ) );
        EXPECT_FALSE( story_find_menu_badge( "sortie.menu", "start", &badge ) );

        ASSERT_TRUE( story_find_menu_row( utest_fixture, "sortie.menu", "back", &row ) );
        ASSERT_TRUE( story_find_menu_label_lane( "sortie.menu", "back", &label_lane ) );
    }
}

UTEST_F( author_story_fixture, regression_sortie_option_value_overflows_menu_lane )
{
    const int sizes[][ 2 ] = { { 1280, 720 }, { 1280, 680 }, { 1280, 648 }, { 1100, 720 }, { 1100, 648 } };
    const uint32_t focused_mission = story_menu_row_id( "sortie.menu", "mission" );

    for ( const auto& size : sizes ) {
        story_reset_state( utest_fixture );
        utest_fixture->selected_mission_index = 3;
        ( void ) story_render_sortie( utest_fixture, size[ 0 ], size[ 1 ], "en", focused_mission );

        vxui_rect menu_panel = {}, briefing = {}, label_lane = {}, value_lane = {}, value_group = {};
        ASSERT_TRUE( story_find_element( "sortie.menu_panel", &menu_panel ) );
        ASSERT_TRUE( story_find_element( "sortie.briefing", &briefing ) );
        ASSERT_TRUE( story_find_menu_label_lane( "sortie.menu", "mission", &label_lane ) );
        ASSERT_TRUE( story_find_menu_value_lane( "sortie.menu", "mission", &value_lane ) );
        ASSERT_TRUE( story_find_menu_value_group( "sortie.menu", "mission", &value_group ) );

        EXPECT_TRUE( story_rect_is_readable( label_lane ) );
        EXPECT_TRUE( story_rect_is_readable( value_lane ) );
        EXPECT_TRUE( story_element_fully_inside( menu_panel, label_lane, 1.0f ) || story_no_horizontal_overlap( label_lane, briefing ) );
        EXPECT_TRUE( story_element_fully_inside( menu_panel, value_lane, 1.0f ) || story_no_horizontal_overlap( value_lane, briefing ) );
        EXPECT_TRUE( story_element_fully_inside( value_lane, value_group, 1.0f ) );
    }
}

UTEST_F( author_story_fixture, regression_sortie_badge_and_value_do_not_collide )
{
    const int sizes[][ 2 ] = { { 1280, 720 }, { 1280, 680 }, { 1280, 648 }, { 1100, 720 }, { 1100, 648 } };
    const uint32_t focused_difficulty = story_menu_row_id( "sortie.menu", "difficulty" );

    for ( const auto& size : sizes ) {
        story_reset_state( utest_fixture );
        utest_fixture->selected_mission_index = 3;
        utest_fixture->difficulty = 2;
        ( void ) story_render_sortie( utest_fixture, size[ 0 ], size[ 1 ], "en", focused_difficulty );

        vxui_rect mission_value = {}, difficulty_value = {}, badge = {};
        ASSERT_TRUE( story_find_menu_value_group( "sortie.menu", "mission", &mission_value ) );
        ASSERT_TRUE( story_find_menu_value_group( "sortie.menu", "difficulty", &difficulty_value ) );
        EXPECT_FALSE( story_find_menu_badge( "sortie.menu", "mission", &badge ) );
        EXPECT_FALSE( story_find_menu_badge( "sortie.menu", "difficulty", &badge ) );
        EXPECT_TRUE( story_rect_is_readable( mission_value ) );
        EXPECT_TRUE( story_rect_is_readable( difficulty_value ) );
    }
}

UTEST_F( author_story_fixture, regression_sortie_menu_rows_remain_readable_with_real_option_values )
{
    const int sizes[][ 2 ] = { { 1280, 720 }, { 1280, 680 }, { 1280, 648 }, { 1100, 720 }, { 1100, 648 } };
    const char* rows[] = { "mission", "difficulty", "start", "back" };

    for ( const auto& size : sizes ) {
        story_reset_state( utest_fixture );
        utest_fixture->selected_mission_index = 3;
        utest_fixture->difficulty = 2;
        ( void ) story_render_sortie( utest_fixture, size[ 0 ], size[ 1 ], "en", story_menu_row_id( "sortie.menu", "mission" ) );

        vxui_rect row_rects[ 4 ] = {};
        for ( int i = 0; i < 4; ++i ) {
            vxui_rect label_lane = {};
            ASSERT_TRUE( story_find_menu_row( utest_fixture, "sortie.menu", rows[ i ], &row_rects[ i ] ) );
            ASSERT_TRUE( story_find_menu_label_lane( "sortie.menu", rows[ i ], &label_lane ) );
            EXPECT_TRUE( story_rect_is_readable( row_rects[ i ] ) );
            EXPECT_TRUE( story_rect_is_readable( label_lane ) );
        }

        vxui_rect mission_value = {}, difficulty_value = {};
        ASSERT_TRUE( story_find_menu_value_group( "sortie.menu", "mission", &mission_value ) );
        ASSERT_TRUE( story_find_menu_value_group( "sortie.menu", "difficulty", &difficulty_value ) );
        EXPECT_TRUE( story_rect_is_readable( mission_value ) );
        EXPECT_TRUE( story_rect_is_readable( difficulty_value ) );
        EXPECT_TRUE( story_group_no_overlaps( row_rects, 4 ) );
        EXPECT_TRUE( story_readable_vertical_stack( row_rects, 4, 1.0f ) );
    }
}

UTEST_F( author_story_fixture, regression_sortie_footer_prompts_and_status_remain_readable )
{
    const int sizes[][ 2 ] = { { 1280, 720 }, { 1280, 680 }, { 1280, 648 }, { 1100, 720 }, { 1100, 648 } };

    for ( const auto& size : sizes ) {
        story_reset_state( utest_fixture );
        ( void ) story_render_sortie( utest_fixture, size[ 0 ], size[ 1 ], "en" );

        vxui_rect footer = {}, prompts = {}, status = {};
        ASSERT_TRUE( story_find_element( "sortie.footer", &footer ) );
        ASSERT_TRUE( story_find_element( "sortie.footer.prompts", &prompts ) );
        ASSERT_TRUE( story_find_element( "sortie.footer.status", &status ) );
        EXPECT_TRUE( story_element_fully_inside( footer, prompts, 1.0f ) );
        EXPECT_TRUE( story_element_fully_inside( footer, status, 1.0f ) );
        EXPECT_TRUE( vxui_demo_rects_non_overlapping( prompts, status, 0.0f ) );
        EXPECT_TRUE( story_rect_is_readable( prompts ) );
        EXPECT_TRUE( story_rect_is_readable( status ) );
    }
}

UTEST_F( author_story_fixture, regression_sortie_three_lane_split_remains_non_overlapping )
{
    const int sizes[][ 2 ] = { { 1280, 720 }, { 1280, 680 }, { 1280, 648 }, { 1100, 720 }, { 1100, 648 } };
    const char* locales[] = { "en", "ja-JP", "ar" };

    for ( const char* locale : locales ) {
        for ( const auto& size : sizes ) {
            story_reset_state( utest_fixture );
            utest_fixture->selected_mission_index = 3;
            vxui_draw_list list = story_render_sortie( utest_fixture, size[ 0 ], size[ 1 ], locale );

            vxui_rect menu_panel = {}, briefing = {}, detail = {}, footer = {}, deck = {}, body = {}, meta = {};
            vxui_rect warning_text = {};
            ASSERT_TRUE( story_find_element( "sortie.menu_panel", &menu_panel ) );
            ASSERT_TRUE( story_find_element( "sortie.briefing", &briefing ) );
            ASSERT_TRUE( story_find_element( "sortie.briefing.body", &body ) );
            ASSERT_TRUE( story_find_element( "sortie.deck", &deck ) );
            ASSERT_TRUE( story_find_element( "sortie.footer", &footer ) );
            ASSERT_TRUE( story_find_anim( &utest_fixture->ctx, vxui_id( VXUI_DEMO_SHARED_MISSIONS[ 3 ].warning ), &warning_text ) );

            EXPECT_TRUE( story_no_horizontal_overlap( menu_panel, briefing ) );
            EXPECT_TRUE( story_element_fully_inside( briefing, body, 1.0f ) );
            EXPECT_TRUE( story_has_text_in_region( &list, body ) );
            EXPECT_TRUE( story_rect_is_readable( footer ) );
            if ( story_find_element( "sortie.detail", &detail ) ) {
                EXPECT_TRUE( story_no_horizontal_overlap( briefing, detail ) );
                EXPECT_TRUE( story_no_horizontal_overlap( menu_panel, detail ) );
            } else {
                vxui_rect copy = {};
                ASSERT_TRUE( story_find_element( "sortie.briefing.meta", &meta ) );
                ASSERT_TRUE( story_find_element( "sortie.briefing.copy", &copy ) );
                EXPECT_TRUE( story_element_fully_inside( briefing, meta, 1.0f ) );
                EXPECT_TRUE( story_vertical_order( copy, meta, 0.0f ) );
            }
        }
    }
}
