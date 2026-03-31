#include <cmath>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

#include "../demo/internal/layout_contract.h"
#include "../demo/internal/main_menu_shared.h"
#include "../demo/internal/split_deck_shared.h"
#include "../demo/internal/layout_validation.h"
#include "../third_party/utest.h"
#include "../vxui.h"
#include "../vxui_menu.h"
#include "test_support.h"

enum
{
    VXUI_TEST_FONT_UI = 0,
    VXUI_TEST_FONT_TITLE = 1,
    VXUI_TEST_FONT_DEBUG = 2,
    VXUI_TEST_FONT_JAPANESE = 3,
    VXUI_TEST_FONT_ARABIC = 4,
    VXUI_TEST_FONT_UI_LARGE = 5,
    VXUI_TEST_FONT_JAPANESE_TITLE = 6,
    VXUI_TEST_FONT_ARABIC_TITLE = 7,
    VXUI_TEST_FONT_ROLE_BODY = 100,
    VXUI_TEST_FONT_ROLE_TITLE = 101,
};

enum demo_layout_text_pack
{
    DEMO_TEXT_PACK_NORMAL = 0,
    DEMO_TEXT_PACK_LONG,
    DEMO_TEXT_PACK_EXTREME,
};

enum demo_layout_focus_mode
{
    DEMO_FOCUS_NONE = 0,
    DEMO_FOCUS_FIRST,
    DEMO_FOCUS_LAST,
    DEMO_FOCUS_DETAIL_HEAVY,
};

typedef struct demo_layout_fixture
{
    uint8_t* memory;
    uint64_t memory_size;
    vxui_ctx ctx;
    vxui_test_fontcache_handle* fontcache;
    vxui_menu_state main_menu_state;
    vxui_menu_state sortie_menu_state;
    vxui_menu_state loadout_menu_state;
    vxui_menu_state archives_menu_state;
    vxui_menu_state settings_menu_state;
    vxui_menu_state records_menu_state;
    int prompt_table_index;
    int selection_index;
    int archive_category_index;
    int archive_entry_index;
    int records_entry_index;
    int extra_menu_rows;
    demo_layout_text_pack text_pack;
    int string_count;
    char string_storage[ 256 ][ 2048 ];
} demo_layout_fixture;

struct demo_layout_case
{
    const char* screen_name;
    const char* locale;
    int width;
    int height;
    int prompt_table_index;
    demo_layout_text_pack text_pack;
    demo_layout_focus_mode focus_mode;
    int extra_menu_rows;
    int selection_index;
};

static bool demo_layout_locale_matches( const char* locale, const char* prefix )
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

static const char* demo_layout_localized( const char* locale, const char* en, const char* ja, const char* ar )
{
    if ( demo_layout_locale_matches( locale, "ja" ) ) {
        return ja;
    }
    if ( demo_layout_locale_matches( locale, "ar" ) ) {
        return ar;
    }
    return en;
}

static const char* demo_layout_status_label( const char* locale, const char* key )
{
    if ( std::strcmp( key, "status.label.locale" ) == 0 ) {
        return demo_layout_localized( locale, "Locale", "言語", "اللغة" );
    }
    if ( std::strcmp( key, "status.short.locale" ) == 0 ) {
        return demo_layout_localized( locale, "Loc", "言語", "لغة" );
    }
    if ( std::strcmp( key, "status.label.prompts" ) == 0 ) {
        return demo_layout_localized( locale, "Prompts", "プロンプト", "الأزرار" );
    }
    if ( std::strcmp( key, "status.short.prompts" ) == 0 ) {
        return demo_layout_localized( locale, "Input", "入力", "دخل" );
    }
    if ( std::strcmp( key, "status.label.screens" ) == 0 ) {
        return demo_layout_localized( locale, "Screens", "画面数", "الشاشات" );
    }
    if ( std::strcmp( key, "status.short.screens" ) == 0 ) {
        return demo_layout_localized( locale, "Views", "画面", "شاشات" );
    }
    if ( std::strcmp( key, "status.label.top" ) == 0 ) {
        return demo_layout_localized( locale, "Top", "最上位", "الأعلى" );
    }
    if ( std::strcmp( key, "status.short.top" ) == 0 ) {
        return demo_layout_localized( locale, "Top", "上位", "أعلى" );
    }
    return key;
}

static const char* demo_layout_locale_name( const char* locale )
{
    return demo_layout_localized( locale, "English", "日本語", "العربية" );
}

static const char* demo_layout_prompt_name( const char* locale, int prompt_table_index )
{
    return prompt_table_index == 0
        ? demo_layout_localized( locale, "Keyboard", "キーボード", "لوحة مفاتيح" )
        : demo_layout_localized( locale, "Gamepad", "ゲームパッド", "يد تحكم" );
}

static const char* demo_layout_screen_name( const char* locale, const char* screen_name )
{
    if ( std::strcmp( screen_name, "main_menu" ) == 0 ) {
        return demo_layout_localized( locale, "Deck", "デッキ", "السطح" );
    }
    return screen_name;
}

static Clay_String demo_layout_clay_string( const char* text )
{
    return ( Clay_String ) {
        .isStaticallyAllocated = false,
        .length = text ? ( int32_t ) std::strlen( text ) : 0,
        .chars = text ? text : "",
    };
}

static const char* demo_layout_store( demo_layout_fixture* fixture, const std::string& text )
{
    if ( fixture->string_count >= 256 ) {
        return "";
    }
    char* slot = fixture->string_storage[ fixture->string_count++ ];
    std::strncpy( slot, text.c_str(), 2047 );
    slot[ 2047 ] = '\0';
    return slot;
}

static const char* demo_layout_store_localized(
    demo_layout_fixture* fixture,
    const char* locale,
    const char* en,
    const char* ja,
    const char* ar )
{
    return demo_layout_store( fixture, demo_layout_localized( locale, en, ja, ar ) );
}

static void demo_layout_font_resolver(
    vxui_ctx* ctx,
    uint32_t requested_font_id,
    float requested_font_size,
    const char* locale,
    void* userdata,
    vxui_resolved_font* out )
{
    ( void ) ctx;
    ( void ) userdata;
    if ( !out ) {
        return;
    }

    switch ( requested_font_id ) {
        case VXUI_TEST_FONT_ROLE_BODY:
            out->font_id = requested_font_size >= 40.0f ? VXUI_TEST_FONT_UI_LARGE : VXUI_TEST_FONT_UI;
            out->line_height = std::max( requested_font_size + 4.0f, requested_font_size * 1.25f );
            return;

        case VXUI_TEST_FONT_ROLE_TITLE:
            if ( demo_layout_locale_matches( locale, "ja" ) ) {
                out->font_id = VXUI_TEST_FONT_JAPANESE_TITLE;
            } else if ( demo_layout_locale_matches( locale, "ar" ) ) {
                out->font_id = VXUI_TEST_FONT_ARABIC_TITLE;
            } else {
                out->font_id = VXUI_TEST_FONT_TITLE;
            }
            out->line_height = std::max( requested_font_size + 6.0f, requested_font_size * 1.18f );
            return;

        default:
            return;
    }
}

static bool demo_layout_find_element_bounds( const char* id, vxui_rect* out )
{
    Clay_ElementData element = Clay_GetElementData( Clay_GetElementId( demo_layout_clay_string( id ) ) );
    if ( !element.found ) {
        return false;
    }
    if ( out ) {
        *out = {
            element.boundingBox.x,
            element.boundingBox.y,
            element.boundingBox.width,
            element.boundingBox.height,
        };
    }
    return true;
}

static bool demo_layout_find_hash_bounds( uint32_t id, vxui_rect* out )
{
    Clay_ElementId clay_id = {};
    clay_id.id = id;
    Clay_ElementData data = Clay_GetElementData( clay_id );
    if ( !data.found ) {
        return false;
    }
    if ( out ) {
        *out = {
            data.boundingBox.x,
            data.boundingBox.y,
            data.boundingBox.width,
            data.boundingBox.height,
        };
    }
    return true;
}

static bool demo_layout_find_anim_bounds( const vxui_ctx* ctx, uint32_t id, vxui_rect* out )
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

static std::string demo_layout_read_source_file( const char* relative_path )
{
    std::ifstream input( std::string( VXUI_SOURCE_DIR ) + "/" + ( relative_path ? relative_path : "" ), std::ios::binary );
    if ( !input ) {
        return "";
    }
    return std::string( std::istreambuf_iterator< char >( input ), std::istreambuf_iterator< char >() );
}

static bool demo_layout_find_text_owner_bounds( const demo_layout_fixture* fixture, const char* text, vxui_rect* out )
{
    if ( !fixture || !text ) {
        return false;
    }
    return demo_layout_find_anim_bounds( &fixture->ctx, vxui_id( text ), out );
}

static bool demo_layout_probe_lookup_bounds_by_id( void* userdata, const char* id, vxui_rect* out )
{
    ( void ) userdata;
    return demo_layout_find_element_bounds( id, out );
}

static bool demo_layout_probe_lookup_bounds_by_hash( void* userdata, uint32_t id, vxui_rect* out )
{
    ( void ) userdata;
    return demo_layout_find_hash_bounds( id, out );
}

static bool demo_layout_probe_lookup_anim_bounds( void* userdata, uint32_t id, vxui_rect* out )
{
    return demo_layout_find_anim_bounds( ( const vxui_ctx* ) userdata, id, out );
}

template <typename TEmitChildren>
static void demo_layout_emit_screen_surface(
    demo_layout_fixture* fixture,
    const char* root_id,
    const char* surface_id,
    float surface_width,
    float surface_height,
    bool rtl,
    TEmitChildren&& emit_children )
{
    ( void ) rtl;
    const vxui_menu_surface_cfg cfg = {
        surface_width,
        surface_height,
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
    vxui_menu_surface_begin( &fixture->ctx, root_id, surface_id, &cfg );
    emit_children();
    vxui_menu_surface_end( &fixture->ctx );
}

static vxui_menu_style demo_layout_footer_style( void )
{
    vxui_menu_style style = vxui_menu_style_footer_strip();
    style.body_font_id = VXUI_TEST_FONT_ROLE_BODY;
    style.title_font_id = VXUI_TEST_FONT_ROLE_BODY;
    style.badge_font_id = VXUI_TEST_FONT_ROLE_BODY;
    style.section_gap = ( float ) VXUI_DEMO_LAYOUT_SECTION_GAP;
    return style;
}

static const char* demo_layout_detail_text( demo_layout_fixture* fixture, const char* locale )
{
    switch ( fixture->text_pack ) {
        case DEMO_TEXT_PACK_LONG:
            return demo_layout_store_localized(
                fixture,
                locale,
                "Detail panel copy validates containment.\n"
                "It stays inside the split-deck body.\n"
                "Readable body paragraphs remain intact.",
                "詳細パネル文は収まりを確認します。\n"
                "分割デッキの本文領域に収まります。\n"
                "短い段落の読みやすさも保ちます。",
                "نص التفاصيل يتحقق من الاحتواء.\n"
                "يبقى داخل جسم التوزيع المنقسم.\n"
                "وتظل الفقرات مقروءة وواضحة." );
        case DEMO_TEXT_PACK_EXTREME:
            return demo_layout_store_localized(
                fixture,
                locale,
                "Stress copy adds status bands.\n"
                "Warnings and LONGTOKEN LONGTOKEN LONGTOKEN\n"
                "still stay inside the panel.",
                "状態帯と警告を加えたストレス文です。\n"
                "長いトークン列があっても\n"
                "本文はパネル内部に留まります。",
                "نص إجهاد يضيف شرائط حالة.\n"
                "حتى مع التوكنات الطويلة القابلة للالتفاف\n"
                "يبقى النص داخل اللوحة." );
        default:
            return demo_layout_store_localized(
                fixture,
                locale,
                "Compact detail copy.",
                "短い詳細文。",
                "نص تفصيلي قصير." );
    }
}

static uint32_t demo_layout_menu_row_id( const char* scope_id, const char* row_id )
{
    return vxui_idi( scope_id, ( int ) vxui_id( row_id ) );
}

static uint32_t demo_layout_focus_for_case( const demo_layout_case& test_case )
{
    if ( test_case.focus_mode == DEMO_FOCUS_NONE ) {
        return 0u;
    }

    if ( std::strcmp( test_case.screen_name, "main_menu" ) == 0 ) {
        switch ( test_case.focus_mode ) {
            case DEMO_FOCUS_FIRST: return demo_layout_menu_row_id( "main.command_menu", "sortie" );
            case DEMO_FOCUS_LAST: return demo_layout_menu_row_id( "main.command_menu", "quit" );
            case DEMO_FOCUS_DETAIL_HEAVY: return demo_layout_menu_row_id( "main.command_menu", "loadout" );
            default: return 0u;
        }
    }
    if ( std::strcmp( test_case.screen_name, "sortie" ) == 0 ) {
        switch ( test_case.focus_mode ) {
            case DEMO_FOCUS_FIRST: return demo_layout_menu_row_id( "sortie.menu", "mission" );
            case DEMO_FOCUS_LAST: return demo_layout_menu_row_id( "sortie.menu", "back" );
            case DEMO_FOCUS_DETAIL_HEAVY: return demo_layout_menu_row_id( "sortie.menu", "mission" );
            default: return 0u;
        }
    }
    if ( std::strcmp( test_case.screen_name, "loadout" ) == 0 ) {
        switch ( test_case.focus_mode ) {
            case DEMO_FOCUS_FIRST: return demo_layout_menu_row_id( "loadout.menu", "ship" );
            case DEMO_FOCUS_LAST: return demo_layout_menu_row_id( "loadout.menu", "back" );
            case DEMO_FOCUS_DETAIL_HEAVY: return demo_layout_menu_row_id( "loadout.menu", "ship" );
            default: return 0u;
        }
    }
    if ( std::strcmp( test_case.screen_name, "archives" ) == 0 ) {
        switch ( test_case.focus_mode ) {
            case DEMO_FOCUS_FIRST: return demo_layout_menu_row_id( "archives.menu", "category" );
            case DEMO_FOCUS_LAST: return demo_layout_menu_row_id( "archives.menu", "back" );
            case DEMO_FOCUS_DETAIL_HEAVY: return demo_layout_menu_row_id( "archives.menu", "entry" );
            default: return 0u;
        }
    }
    if ( std::strcmp( test_case.screen_name, "settings" ) == 0 ) {
        switch ( test_case.focus_mode ) {
            case DEMO_FOCUS_FIRST: return demo_layout_menu_row_id( "settings.body_menu", "challenge" );
            case DEMO_FOCUS_LAST: return demo_layout_menu_row_id( "settings.body_menu", "overflow_3" );
            case DEMO_FOCUS_DETAIL_HEAVY: return demo_layout_menu_row_id( "settings.body_menu", "prompt_table" );
            default: return 0u;
        }
    }
    if ( std::strcmp( test_case.screen_name, "records" ) == 0 ) {
        switch ( test_case.focus_mode ) {
            case DEMO_FOCUS_FIRST: return demo_layout_menu_row_id( "records.menu", "board" );
            case DEMO_FOCUS_LAST: return demo_layout_menu_row_id( "records.menu", "back" );
            case DEMO_FOCUS_DETAIL_HEAVY: return demo_layout_menu_row_id( "records.menu", "run" );
            default: return 0u;
        }
    }
    return 0u;
}

static int demo_layout_prompt_glyph( int prompt_table_index, char keyboard, char gamepad )
{
    return prompt_table_index == 0 ? keyboard : gamepad;
}

static void demo_layout_begin_case( demo_layout_fixture* fixture, const demo_layout_case& test_case )
{
    fixture->string_count = 0;
    fixture->prompt_table_index = test_case.prompt_table_index;
    fixture->selection_index = test_case.selection_index;
    fixture->extra_menu_rows = test_case.extra_menu_rows;
    fixture->text_pack = test_case.text_pack;
    fixture->main_menu_state = {};
    fixture->sortie_menu_state = {};
    fixture->loadout_menu_state = {};
    fixture->archives_menu_state = {};
    fixture->settings_menu_state = {};
    fixture->records_menu_state = {};
    fixture->ctx.cfg.screen_width = test_case.width;
    fixture->ctx.cfg.screen_height = test_case.height;
    vxui_set_font_resolver( &fixture->ctx, demo_layout_font_resolver, fixture );
    vxui_set_locale( &fixture->ctx, test_case.locale );

    vxui_input_table table = {
        .confirm = { VXUI_TEST_FONT_ROLE_BODY, ( uint32_t ) demo_layout_prompt_glyph( test_case.prompt_table_index, 'E', 'A' ) },
        .cancel = { VXUI_TEST_FONT_ROLE_BODY, ( uint32_t ) demo_layout_prompt_glyph( test_case.prompt_table_index, 'Q', 'B' ) },
    };
    vxui_set_input_table( &fixture->ctx, &table );
    fixture->ctx.focused_id = demo_layout_focus_for_case( test_case );
    vxui_begin( &fixture->ctx, 0.016f );
}

static void demo_layout_render_boot( demo_layout_fixture* fixture, const demo_layout_case& test_case )
{
    const bool rtl = fixture->ctx.rtl;
    const float viewport_width = std::max( 0.0f, ( float ) test_case.width - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f );
    const float surface_height = std::max( 0.0f, ( float ) test_case.height - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f );
    const vxui_demo_surface_metrics metrics =
        vxui_demo_compute_surface_metrics( viewport_width, test_case.locale, VXUI_DEMO_SURFACE_BOOT );

    demo_layout_emit_screen_surface( fixture, "boot", "boot.surface", metrics.surface_width, surface_height, rtl, [&]() {
        const char* screen_count_text = demo_layout_store( fixture, std::to_string( fixture->ctx.screen_count ) );
        const char* prompt_label =
            demo_layout_store_localized( fixture, test_case.locale, "Press confirm to skip handshake", "決定でハンドシェイクをスキップ", "اضغط تأكيد لتجاوز المصافحة" );
        vxui_menu_prompt_item footer_prompts[] = {
            { "action.confirm", prompt_label, false, "boot.footer.prompt.confirm" },
        };
        vxui_menu_status_item footer_status[] = {
            { demo_layout_status_label( test_case.locale, "status.short.locale" ), demo_layout_locale_name( test_case.locale ), VXUI_MENU_STATUS_PRIMARY, false, false, "boot.footer.status.locale" },
            { demo_layout_status_label( test_case.locale, "status.short.prompts" ), demo_layout_prompt_name( test_case.locale, test_case.prompt_table_index ), VXUI_MENU_STATUS_SECONDARY, true, false, "boot.footer.status.prompts" },
            { demo_layout_status_label( test_case.locale, "status.short.screens" ), screen_count_text, VXUI_MENU_STATUS_SECONDARY, true, false, "boot.footer.status.screens" },
            { demo_layout_status_label( test_case.locale, "status.short.top" ), demo_layout_screen_name( test_case.locale, test_case.screen_name ), VXUI_MENU_STATUS_PRIMARY, false, false, "boot.footer.status.top" },
        };
        vxui_menu_footer_cfg footer_cfg = {
            footer_prompts,
            ( int ) ( sizeof( footer_prompts ) / sizeof( footer_prompts[ 0 ] ) ),
            footer_status,
            ( int ) ( sizeof( footer_status ) / sizeof( footer_status[ 0 ] ) ),
            VXUI_MENU_SHELL_COMPACT_AUTO,
            surface_height <= 680.0f ? 3 : 4,
            false,
        };
        vxui_menu_style footer_style = demo_layout_footer_style();
        vxui_menu_screen_cfg screen_cfg = {
            VXUI_MENU_SHELL_FORM,
            &footer_style,
            VXUI_MENU_SHELL_COMPACT_AUTO,
            680.0f,
            0.0f,
            false,
            {},
            {},
            {},
            {},
            {},
            footer_cfg,
        };
        vxui_menu_state shell_state = {};

        vxui_menu_screen_begin( &fixture->ctx, &shell_state, "boot.shell", &screen_cfg );
        VXUI( &fixture->ctx, "boot.hero", {
            .layout = {
                .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_GROW( 180.0f ) },
                .childGap = 12,
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
        } ) {
            VXUI_LABEL( &fixture->ctx, demo_layout_store_localized( fixture, test_case.locale, "Wake link", "ウェイクリンク", "وصلة الإيقاظ" ), ( vxui_label_cfg ) {
                .font_id = VXUI_TEST_FONT_ROLE_TITLE,
                .font_size = 42.0f,
            } );
            VXUI_LABEL( &fixture->ctx, demo_layout_detail_text( fixture, test_case.locale ), ( vxui_label_cfg ) { 0 } );
        }
        vxui_menu_footer( &fixture->ctx, "boot.footer", &screen_cfg.footer );
        vxui_menu_screen_end( &fixture->ctx, &shell_state );
    } );
}

static void demo_layout_render_title( demo_layout_fixture* fixture, const demo_layout_case& test_case )
{
    const bool rtl = fixture->ctx.rtl;
    const float viewport_width = std::max( 0.0f, ( float ) test_case.width - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f );
    const float surface_height = std::max( 0.0f, ( float ) test_case.height - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f );
    const vxui_demo_surface_metrics metrics =
        vxui_demo_compute_surface_metrics( viewport_width, test_case.locale, VXUI_DEMO_SURFACE_TITLE );

    demo_layout_emit_screen_surface( fixture, "title", "title.surface", metrics.surface_width, surface_height, rtl, [&]() {
        const char* screen_count_text = demo_layout_store( fixture, std::to_string( fixture->ctx.screen_count ) );
        vxui_menu_status_item footer_status[] = {
            { demo_layout_status_label( test_case.locale, "status.short.locale" ), demo_layout_locale_name( test_case.locale ), VXUI_MENU_STATUS_PRIMARY, false, false, "title.footer.status.locale" },
            { demo_layout_status_label( test_case.locale, "status.short.prompts" ), demo_layout_prompt_name( test_case.locale, test_case.prompt_table_index ), VXUI_MENU_STATUS_SECONDARY, true, false, "title.footer.status.prompts" },
            { demo_layout_status_label( test_case.locale, "status.short.screens" ), screen_count_text, VXUI_MENU_STATUS_SECONDARY, true, false, "title.footer.status.screens" },
            { demo_layout_status_label( test_case.locale, "status.short.top" ), demo_layout_screen_name( test_case.locale, test_case.screen_name ), VXUI_MENU_STATUS_PRIMARY, false, false, "title.footer.status.top" },
        };
        vxui_menu_footer_cfg footer_cfg = {
            nullptr,
            0,
            footer_status,
            ( int ) ( sizeof( footer_status ) / sizeof( footer_status[ 0 ] ) ),
            VXUI_MENU_SHELL_COMPACT_AUTO,
            surface_height <= 680.0f ? 3 : 4,
            false,
        };
        vxui_menu_style footer_style = demo_layout_footer_style();
        vxui_menu_screen_cfg screen_cfg = {
            VXUI_MENU_SHELL_FORM,
            &footer_style,
            VXUI_MENU_SHELL_COMPACT_AUTO,
            680.0f,
            0.0f,
            false,
            {},
            {},
            {},
            {},
            {},
            footer_cfg,
        };
        vxui_menu_state shell_state = {};

        vxui_menu_screen_begin( &fixture->ctx, &shell_state, "title.shell", &screen_cfg );
        VXUI( &fixture->ctx, "title.hero", {
            .layout = {
                .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                .padding = CLAY_PADDING_ALL( 24 ),
                .childGap = 12,
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
        } ) {
            VXUI_LABEL( &fixture->ctx, demo_layout_store_localized( fixture, test_case.locale, "Command Deck", "コマンドデッキ", "منصة الأوامر" ), ( vxui_label_cfg ) {
                .font_id = VXUI_TEST_FONT_ROLE_TITLE,
                .font_size = 44.0f,
            } );
            VXUI_LABEL( &fixture->ctx, demo_layout_detail_text( fixture, test_case.locale ), ( vxui_label_cfg ) { 0 } );
        }
        VXUI( &fixture->ctx, "title.action_band", {
            .layout = {
                .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                .padding = CLAY_PADDING_ALL( 18 ),
                .childGap = 10,
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
        } ) {
            VXUI_LABEL( &fixture->ctx, demo_layout_store_localized( fixture, test_case.locale, "Enter command deck", "コマンドデッキへ", "الدخول إلى منصة الأوامر" ), ( vxui_label_cfg ) { 0 } );
        }
        vxui_menu_footer( &fixture->ctx, "title.footer", &screen_cfg.footer );
        vxui_menu_screen_end( &fixture->ctx, &shell_state );
    } );
}

static void demo_layout_render_main_menu( demo_layout_fixture* fixture, const demo_layout_case& test_case )
{
    const bool rtl = fixture->ctx.rtl;
    const float viewport_width = std::max( 0.0f, ( float ) test_case.width - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f );
    const float surface_height = std::max( 0.0f, ( float ) test_case.height - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f );
    const vxui_demo_main_menu_layout_spec layout =
        vxui_demo_resolve_main_menu_layout( viewport_width, surface_height, test_case.locale );
    vxui_menu_style menu_style = vxui_demo_make_title_deck_menu_style( VXUI_TEST_FONT_ROLE_BODY, VXUI_TEST_FONT_ROLE_TITLE );
    if ( layout.surface_max_height <= 650.0f ) {
        menu_style.body_font_size = 14.0f;
        menu_style.secondary_font_size = 12.0f;
        menu_style.badge_font_size = 8.0f;
        menu_style.row_height = 25.0f;
        menu_style.row_gap = 0.0f;
        menu_style.section_gap = 2.0f;
        menu_style.padding_y = 3.0f;
    }
    menu_style.focus_decor_padding = 7.0f;
    menu_style.focus_decor_alpha = 0.24f;
    menu_style.panel_fill_color = { 18, 30, 52, 224 };
    menu_style.row_fill_color = { 24, 40, 66, 210 };
    menu_style.row_focus_fill_color = { 42, 86, 118, 244 };
    menu_style.row_disabled_fill_color = { 22, 28, 38, 180 };

    demo_layout_emit_screen_surface( fixture, "main_menu", "main.surface", layout.surface.surface_width, layout.surface_max_height, rtl, [&]() {
        const vxui_demo_main_menu_preview* preview =
            vxui_demo_main_menu_preview_from_focused_row( demo_layout_focus_for_case( test_case ) );

        vxui_demo_emit_main_menu_shell(
            &fixture->ctx,
            rtl,
            test_case.locale,
            layout,
            ( vxui_demo_main_menu_visuals ) {
                VXUI_TEST_FONT_ROLE_BODY,
                VXUI_TEST_FONT_ROLE_TITLE,
                VXUI_TEST_FONT_ROLE_BODY,
            },
            ( vxui_demo_main_menu_shell_copy ) {
                demo_layout_store_localized( fixture, test_case.locale, "Command Deck", "コマンドデッキ", "سطح القيادة" ),
                demo_layout_store_localized( fixture, test_case.locale, "Production-shape front-end stub", "本番形状フロントエンドスタブ", "واجهة تجريبية بشكل إنتاجي" ),
                vxui_demo_main_menu_preview_label( test_case.locale ),
                demo_layout_status_label( test_case.locale, "status.label.locale" ),
                demo_layout_locale_name( test_case.locale ),
                demo_layout_status_label( test_case.locale, "status.label.prompts" ),
                demo_layout_prompt_name( test_case.locale, test_case.prompt_table_index ),
                demo_layout_status_label( test_case.locale, "status.label.screens" ),
                1,
                demo_layout_status_label( test_case.locale, "status.label.top" ),
                demo_layout_screen_name( test_case.locale, test_case.screen_name ),
                demo_layout_store_localized( fixture, test_case.locale, "Confirm", "決定", "تأكيد" ),
                demo_layout_store_localized( fixture, test_case.locale, "Cancel", "戻る", "إلغاء" ),
            },
            *preview,
            [&]( float viewport_height ) {
                vxui_menu_begin( &fixture->ctx, &fixture->main_menu_state, "main.command_menu", ( vxui_menu_cfg ) {
                    .style = &menu_style,
                    .viewport_height = viewport_height,
                } );
                vxui_menu_action( &fixture->ctx, &fixture->main_menu_state, "sortie", demo_layout_localized( test_case.locale, "Sortie", "出撃", "الطلعة" ), nullptr, ( vxui_menu_row_cfg ) { .badge_text_key = vxui_demo_badge_text( test_case.locale, "badge.recommended" ) }, ( vxui_action_cfg ) { 0 } );
                vxui_menu_action( &fixture->ctx, &fixture->main_menu_state, "loadout", demo_layout_localized( test_case.locale, "Loadout", "ロードアウト", "العتاد" ), nullptr, ( vxui_menu_row_cfg ) { .badge_text_key = vxui_demo_badge_text( test_case.locale, "badge.demo" ) }, ( vxui_action_cfg ) { 0 } );
                vxui_menu_action( &fixture->ctx, &fixture->main_menu_state, "archives", demo_layout_localized( test_case.locale, "Archives", "アーカイブ", "الأرشيف" ), nullptr, ( vxui_menu_row_cfg ) { .badge_text_key = vxui_demo_badge_text( test_case.locale, "badge.demo" ) }, ( vxui_action_cfg ) { 0 } );
                vxui_menu_action( &fixture->ctx, &fixture->main_menu_state, "settings", demo_layout_localized( test_case.locale, "Settings", "設定", "الإعدادات" ), nullptr, ( vxui_menu_row_cfg ) { .badge_text_key = vxui_demo_badge_text( test_case.locale, "badge.recommended" ) }, ( vxui_action_cfg ) { 0 } );
                vxui_menu_action( &fixture->ctx, &fixture->main_menu_state, "records", demo_layout_localized( test_case.locale, "Records", "記録", "السجلات" ), nullptr, ( vxui_menu_row_cfg ) { .badge_text_key = vxui_demo_badge_text( test_case.locale, "badge.demo" ) }, ( vxui_action_cfg ) { 0 } );
                vxui_menu_action( &fixture->ctx, &fixture->main_menu_state, "credits", demo_layout_localized( test_case.locale, "Credits", "クレジット", "الاعتمادات" ), nullptr, ( vxui_menu_row_cfg ) { .badge_text_key = vxui_demo_badge_text( test_case.locale, "badge.demo" ) }, ( vxui_action_cfg ) { 0 } );
                vxui_menu_action( &fixture->ctx, &fixture->main_menu_state, "quit", demo_layout_localized( test_case.locale, "Quit Demo", "デモ終了", "إنهاء العرض" ), nullptr, ( vxui_menu_row_cfg ) { 0 }, ( vxui_action_cfg ) { 0 } );
                for ( int i = 0; i < fixture->extra_menu_rows; ++i ) {
                    std::string row_id = "overflow_" + std::to_string( i );
                    std::string row_label = "Overflow " + std::to_string( i );
                    vxui_menu_action( &fixture->ctx, &fixture->main_menu_state, row_id.c_str(), demo_layout_store( fixture, row_label ), nullptr, ( vxui_menu_row_cfg ) { 0 }, ( vxui_action_cfg ) { 0 } );
                }
                vxui_menu_end( &fixture->ctx, &fixture->main_menu_state );
            } );
    } );
}

static void demo_layout_render_split_screen( demo_layout_fixture* fixture, const demo_layout_case& test_case, vxui_demo_surface_kind kind )
{
    static const char* kBinaryOptions[] = { "Off", "On" };
    static const char* kMissionOptions[] = { "Alpha", "Bravo", "Gamma" };
    static const char* kDifficultyOptions[] = { "Easy", "Normal", "Hard" };
    static const char* kShipOptions[] = { "Vector", "Lancer", "Aegis" };
    static const char* kLoadoutOptions[] = { "A", "B", "C" };
    static const char* kArchiveOptions[] = { "Ops", "Intel", "Signals" };
    static const char* kRecordOptions[] = { "Board A", "Board B", "Board C" };
    int overflow_selection = 0;
    const bool rtl = fixture->ctx.rtl;
    const float viewport_width = std::max( 0.0f, ( float ) test_case.width - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f );
    const float surface_height = std::max( 0.0f, ( float ) test_case.height - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f );
    const vxui_demo_split_deck_layout_spec layout =
        vxui_demo_resolve_split_deck_layout( kind, viewport_width, surface_height, test_case.locale );
    const bool compact_locale =
        demo_layout_locale_matches( test_case.locale, "ja" ) || demo_layout_locale_matches( test_case.locale, "ar" );
    const bool compact_footer = layout.surface_max_height <= 650.0f;
    const bool compact_menu = compact_footer || compact_locale || layout.surface_max_height <= 680.0f || layout.primary_lane_width <= 360.0f;
    const bool compact_detail =
        compact_footer || compact_locale || layout.surface_max_height <= 700.0f || layout.secondary_lane_width <= 400.0f;
    const uint16_t detail_panel_padding = kind == VXUI_DEMO_SURFACE_SORTIE
        ? ( compact_footer ? uint16_t{ 8 } : compact_detail ? uint16_t{ 10 } : uint16_t{ 14 } )
        : ( compact_footer ? uint16_t{ 12 } : uint16_t{ 16 } );
    const uint16_t detail_panel_gap = kind == VXUI_DEMO_SURFACE_SORTIE
        ? ( compact_footer ? uint16_t{ 6 } : compact_detail ? uint16_t{ 8 } : uint16_t{ 12 } )
        : ( compact_footer ? uint16_t{ 10 } : uint16_t{ 16 } );
    const uint16_t detail_body_gap = kind == VXUI_DEMO_SURFACE_SORTIE
        ? ( compact_footer ? uint16_t{ 8 } : compact_detail ? uint16_t{ 12 } : uint16_t{ 20 } )
        : ( compact_footer ? uint16_t{ 8 } : uint16_t{ 12 } );
    const uint16_t tertiary_panel_padding = compact_footer ? uint16_t{ 10 } : compact_detail ? uint16_t{ 14 } : uint16_t{ 16 };
    const uint16_t tertiary_panel_gap = compact_footer ? uint16_t{ 6 } : compact_detail ? uint16_t{ 8 } : uint16_t{ 10 };
    const float detail_body_width = std::max( 0.0f, layout.secondary_lane_width - detail_panel_padding * 2.0f );
    const float tertiary_body_width = std::max( 0.0f, layout.tertiary_lane_width - tertiary_panel_padding * 2.0f );
    const bool stress_sortie_detail = kind == VXUI_DEMO_SURFACE_SORTIE && layout.secondary_lane_width <= 320.0f;
    const vxui_demo_split_deck_visuals visuals = {
        VXUI_TEST_FONT_ROLE_BODY,
        VXUI_TEST_FONT_ROLE_TITLE,
        VXUI_TEST_FONT_ROLE_BODY,
    };
    vxui_menu_style form_style = vxui_demo_shared_menu_style_form_deck(
        visuals,
        kind == VXUI_DEMO_SURFACE_SORTIE ? 110.0f : kind == VXUI_DEMO_SURFACE_LOADOUT ? 136.0f : 132.0f );
    if ( kind == VXUI_DEMO_SURFACE_SORTIE ) {
        form_style.body_font_size = compact_footer ? 13.0f : compact_menu ? 15.0f : 16.0f;
        form_style.secondary_font_size = compact_footer ? 10.0f : compact_menu ? 12.0f : 13.0f;
        form_style.badge_font_size = compact_footer ? 8.0f : compact_menu ? 10.0f : 11.0f;
        form_style.row_height = compact_footer ? 22.0f : compact_menu ? 28.0f : 30.0f;
        form_style.row_gap = compact_footer ? 1.0f : 2.0f;
        form_style.section_gap = compact_footer ? 3.0f : compact_menu ? 5.0f : 8.0f;
        form_style.padding_y = compact_footer ? 3.0f : compact_menu ? 6.0f : 8.0f;
    } else {
        form_style.row_height = compact_footer ? 24.0f : 32.0f;
        form_style.row_gap = compact_footer ? 1.0f : 2.0f;
        form_style.section_gap = compact_footer ? 4.0f : form_style.section_gap;
        form_style.padding_y = compact_footer ? 4.0f : form_style.padding_y;
    }

    const char* root_id = vxui_demo_root_id( kind );
    const char* surface_id = vxui_demo_surface_id( kind );
    const char* deck_id = kind == VXUI_DEMO_SURFACE_SORTIE ? "sortie.deck"
        : kind == VXUI_DEMO_SURFACE_LOADOUT         ? "loadout.deck"
        : kind == VXUI_DEMO_SURFACE_ARCHIVES        ? "archives.deck"
                                                   : "records.deck";
    const char* menu_panel_id = kind == VXUI_DEMO_SURFACE_SORTIE ? "sortie.menu_panel"
        : kind == VXUI_DEMO_SURFACE_LOADOUT                      ? "loadout.menu_panel"
        : kind == VXUI_DEMO_SURFACE_ARCHIVES                     ? "archives.menu_panel"
                                                                 : "records.menu_panel";
    const char* detail_id = kind == VXUI_DEMO_SURFACE_SORTIE ? "sortie.briefing"
        : kind == VXUI_DEMO_SURFACE_LOADOUT                  ? "loadout.detail"
        : kind == VXUI_DEMO_SURFACE_ARCHIVES                 ? "archives.detail"
                                                             : "records.detail";
    const char* tertiary_id = kind == VXUI_DEMO_SURFACE_SORTIE ? "sortie.detail" : nullptr;
    const char* menu_scope = kind == VXUI_DEMO_SURFACE_SORTIE ? "sortie.menu"
        : kind == VXUI_DEMO_SURFACE_LOADOUT                  ? "loadout.menu"
        : kind == VXUI_DEMO_SURFACE_ARCHIVES                 ? "archives.menu"
                                                             : "records.menu";
    vxui_menu_state* menu_state = kind == VXUI_DEMO_SURFACE_SORTIE ? &fixture->sortie_menu_state
        : kind == VXUI_DEMO_SURFACE_LOADOUT                        ? &fixture->loadout_menu_state
        : kind == VXUI_DEMO_SURFACE_ARCHIVES                       ? &fixture->archives_menu_state
                                                                   : &fixture->records_menu_state;

    const char* detail_text = demo_layout_detail_text( fixture, test_case.locale );
    const char* tertiary_text =
        tertiary_id ? demo_layout_store( fixture, std::string( detail_text ) + " Auxiliary lane note." ) : nullptr;
    const bool show_sortie_tertiary =
        kind == VXUI_DEMO_SURFACE_SORTIE && vxui_demo_shared_use_sortie_tertiary_lane( layout, compact_detail );
    const bool compact_shell = compact_footer || layout.surface_max_height <= 680.0f || test_case.width <= 1140;

    const char* screen_title = kind == VXUI_DEMO_SURFACE_SORTIE
        ? demo_layout_store_localized( fixture, test_case.locale, "Sortie", "出撃", "الطلعة" )
        : kind == VXUI_DEMO_SURFACE_LOADOUT
        ? demo_layout_store_localized( fixture, test_case.locale, "Loadout", "ロードアウト", "العتاد" )
        : kind == VXUI_DEMO_SURFACE_ARCHIVES
        ? demo_layout_store_localized( fixture, test_case.locale, "Archives", "アーカイブ", "الأرشيف" )
        : demo_layout_store_localized( fixture, test_case.locale, "Records", "記録", "السجلات" );

    demo_layout_emit_screen_surface( fixture, root_id, surface_id, layout.surface.surface_width, layout.surface_max_height, rtl, [&]() {
        vxui_menu_style shell_style = vxui_demo_shared_split_deck_shell_style( visuals, layout.deck_gap, compact_shell );
        vxui_menu_prompt_item footer_prompts[ 2 ] = {};
        vxui_menu_status_item footer_status[ 4 ] = {};
        std::string screen_count_text;
        vxui_menu_footer_cfg footer_cfg = vxui_demo_shared_make_split_deck_footer_cfg(
            footer_prompts,
            footer_status,
            {
                demo_layout_locale_name( test_case.locale ),
                demo_layout_prompt_name( test_case.locale, test_case.prompt_table_index ),
                demo_layout_screen_name( test_case.locale, root_id ),
                1,
            },
            screen_count_text,
            compact_footer ? 3 : 4 );
        vxui_menu_screen_cfg screen_cfg =
            vxui_demo_shared_make_split_deck_screen_cfg( &shell_style, screen_title, layout, show_sortie_tertiary, footer_cfg );
        vxui_menu_state shell_state = {};

        vxui_menu_screen_begin( &fixture->ctx, &shell_state, deck_id, &screen_cfg );
        vxui_menu_header( &fixture->ctx,
            kind == VXUI_DEMO_SURFACE_SORTIE ? "sortie.header"
                : kind == VXUI_DEMO_SURFACE_LOADOUT ? "loadout.header"
                : kind == VXUI_DEMO_SURFACE_ARCHIVES ? "archives.header"
                                                     : "records.header",
            &screen_cfg.header );

        vxui_menu_primary_lane_begin( &fixture->ctx, menu_panel_id, &screen_cfg.primary_lane );
        vxui_menu_begin( &fixture->ctx, menu_state, menu_scope, ( vxui_menu_cfg ) {
            .style = &form_style,
            .viewport_height = layout.menu_viewport_height,
        } );
        if ( kind == VXUI_DEMO_SURFACE_SORTIE ) {
            int mission = 0;
            int difficulty = 1;
            vxui_menu_option( &fixture->ctx, menu_state, "mission", demo_layout_localized( test_case.locale, "Mission", "任務", "المهمة" ), &mission, kMissionOptions, 3, ( vxui_menu_row_cfg ) { 0 }, ( vxui_option_cfg ) { 0 } );
            vxui_menu_option( &fixture->ctx, menu_state, "difficulty", demo_layout_localized( test_case.locale, "Difficulty", "難易度", "الصعوبة" ), &difficulty, kDifficultyOptions, 3, ( vxui_menu_row_cfg ) { 0 }, ( vxui_option_cfg ) { 0 } );
            vxui_menu_action( &fixture->ctx, menu_state, "start", demo_layout_localized( test_case.locale, "Start Sortie", "出撃開始", "ابدأ الطلعة" ), nullptr, ( vxui_menu_row_cfg ) { 0 }, ( vxui_action_cfg ) { 0 } );
            vxui_menu_action( &fixture->ctx, menu_state, "back", demo_layout_localized( test_case.locale, "Back", "戻る", "رجوع" ), nullptr, ( vxui_menu_row_cfg ) { 0 }, ( vxui_action_cfg ) { 0 } );
        } else if ( kind == VXUI_DEMO_SURFACE_LOADOUT ) {
            int selection = 0;
            vxui_menu_option( &fixture->ctx, menu_state, "ship", demo_layout_localized( test_case.locale, "Ship", "機体", "المركبة" ), &selection, kShipOptions, 3, ( vxui_menu_row_cfg ) { 0 }, ( vxui_option_cfg ) { 0 } );
            vxui_menu_option( &fixture->ctx, menu_state, "primary", demo_layout_localized( test_case.locale, "Primary", "主兵装", "السلاح الأساسي" ), &selection, kLoadoutOptions, 3, ( vxui_menu_row_cfg ) { 0 }, ( vxui_option_cfg ) { 0 } );
            vxui_menu_option( &fixture->ctx, menu_state, "support", demo_layout_localized( test_case.locale, "Support", "支援", "الدعم" ), &selection, kLoadoutOptions, 3, ( vxui_menu_row_cfg ) { 0 }, ( vxui_option_cfg ) { 0 } );
            vxui_menu_option( &fixture->ctx, menu_state, "system", demo_layout_localized( test_case.locale, "System", "システム", "النظام" ), &selection, kLoadoutOptions, 3, ( vxui_menu_row_cfg ) { 0 }, ( vxui_option_cfg ) { 0 } );
            vxui_menu_action( &fixture->ctx, menu_state, "back", demo_layout_localized( test_case.locale, "Back", "戻る", "رجوع" ), nullptr, ( vxui_menu_row_cfg ) { 0 }, ( vxui_action_cfg ) { 0 } );
        } else if ( kind == VXUI_DEMO_SURFACE_ARCHIVES ) {
            int selection = 0;
            vxui_menu_option( &fixture->ctx, menu_state, "category", demo_layout_localized( test_case.locale, "Category", "カテゴリ", "الفئة" ), &selection, kArchiveOptions, 3, ( vxui_menu_row_cfg ) { 0 }, ( vxui_option_cfg ) { 0 } );
            vxui_menu_option( &fixture->ctx, menu_state, "entry", demo_layout_localized( test_case.locale, "Entry", "項目", "المدخل" ), &selection, kArchiveOptions, 3, ( vxui_menu_row_cfg ) { 0 }, ( vxui_option_cfg ) { 0 } );
            vxui_menu_action( &fixture->ctx, menu_state, "back", demo_layout_localized( test_case.locale, "Back", "戻る", "رجوع" ), nullptr, ( vxui_menu_row_cfg ) { 0 }, ( vxui_action_cfg ) { 0 } );
        } else {
            int selection = 0;
            vxui_menu_option( &fixture->ctx, menu_state, "board", demo_layout_localized( test_case.locale, "Board", "ボード", "اللوحة" ), &selection, kRecordOptions, 3, ( vxui_menu_row_cfg ) { 0 }, ( vxui_option_cfg ) { 0 } );
            vxui_menu_option( &fixture->ctx, menu_state, "run", demo_layout_localized( test_case.locale, "Run", "記録", "النتيجة" ), &selection, kRecordOptions, 3, ( vxui_menu_row_cfg ) { 0 }, ( vxui_option_cfg ) { 0 } );
            vxui_menu_action( &fixture->ctx, menu_state, "back", demo_layout_localized( test_case.locale, "Back", "戻る", "رجوع" ), nullptr, ( vxui_menu_row_cfg ) { 0 }, ( vxui_action_cfg ) { 0 } );
        }
        for ( int i = 0; i < fixture->extra_menu_rows; ++i ) {
            std::string row_id = "overflow_" + std::to_string( i );
            std::string row_label = "Overflow " + std::to_string( i );
            vxui_menu_option( &fixture->ctx, menu_state, row_id.c_str(), demo_layout_store( fixture, row_label ), &overflow_selection, kBinaryOptions, 2, ( vxui_menu_row_cfg ) { 0 }, ( vxui_option_cfg ) { 0 } );
        }
        vxui_menu_end( &fixture->ctx, menu_state );
        vxui_menu_primary_lane_end( &fixture->ctx );

        const char* detail_body_id = demo_layout_store( fixture, std::string( detail_id ) + ".body" );
        vxui_menu_secondary_lane_begin( &fixture->ctx, detail_id, &screen_cfg.secondary_lane );
        CLAY( Clay_GetElementId( demo_layout_clay_string( detail_body_id ) ), {
            .layout = {
                .sizing = { CLAY_SIZING_FIXED( detail_body_width ), CLAY_SIZING_FIT( 0 ) },
                .childGap = detail_body_gap,
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
        } ) {
            CLAY( Clay_GetElementId( demo_layout_clay_string( demo_layout_store( fixture, std::string( detail_id ) + ".copy" ) ) ), {
                .layout = {
                    .sizing = { CLAY_SIZING_FIXED( detail_body_width ), CLAY_SIZING_FIT( 0 ) },
                    .childGap = compact_footer ? uint16_t{ 6 } : detail_body_gap,
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                },
            } ) {
                VXUI_LABEL( &fixture->ctx, detail_text, ( vxui_label_cfg ) {
                    .font_id = visuals.body_font_id,
                    .font_size = compact_footer ? 11.0f : stress_sortie_detail ? 10.0f : compact_detail ? 11.0f : 16.0f,
                } );
                if ( kind == VXUI_DEMO_SURFACE_SORTIE && !show_sortie_tertiary && tertiary_text ) {
                    VXUI_LABEL( &fixture->ctx, tertiary_text, ( vxui_label_cfg ) {
                        .font_id = visuals.body_font_id,
                        .font_size = compact_footer ? 10.0f : compact_detail ? 12.0f : 13.0f,
                    } );
                }
            }
        }
        vxui_menu_secondary_lane_end( &fixture->ctx );

        if ( show_sortie_tertiary ) {
            vxui_menu_tertiary_lane_begin( &fixture->ctx, tertiary_id, &screen_cfg.tertiary_lane );
            CLAY( Clay_GetElementId( demo_layout_clay_string( demo_layout_store( fixture, std::string( tertiary_id ) + ".body" ) ) ), {
                .layout = {
                    .sizing = { CLAY_SIZING_FIXED( tertiary_body_width ), CLAY_SIZING_FIT( 0 ) },
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                },
            } ) {
                VXUI_LABEL( &fixture->ctx, tertiary_text, ( vxui_label_cfg ) {
                    .font_id = visuals.body_font_id,
                    .font_size = compact_footer ? 10.0f : compact_detail ? 13.0f : 14.0f,
                } );
            }
            vxui_menu_tertiary_lane_end( &fixture->ctx );
        }

        vxui_menu_footer( &fixture->ctx, demo_layout_store( fixture, std::string( root_id ) + ".footer" ), &footer_cfg );
        vxui_menu_screen_end( &fixture->ctx, &shell_state );
    } );
}

static void demo_layout_render_settings( demo_layout_fixture* fixture, const demo_layout_case& test_case )
{
    static const char* kToggleOptions[] = { "Off", "On" };
    static const char* kChallengeOptions[] = { "Easy", "Normal", "Hard" };
    static const char* kPromptOptions[] = { "Keyboard", "Gamepad" };
    static const char* kLocaleOptions[] = { "English", "Japanese", "Arabic" };
    const bool rtl = fixture->ctx.rtl;
    const float viewport_width = std::max( 0.0f, ( float ) test_case.width - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f );
    const float surface_height = std::max( 0.0f, ( float ) test_case.height - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f );
    const vxui_demo_settings_layout_spec layout = vxui_demo_resolve_settings_layout( viewport_width, surface_height, test_case.locale );
    vxui_menu_style form_style = vxui_menu_style_form();
    form_style.label_lane_width = layout.surface.label_lane_width;

    int challenge = 1;
    float volume = 0.40f;
    int toggle = 1;

    demo_layout_emit_screen_surface( fixture, "settings", "settings.surface", layout.surface.surface_width, layout.surface_max_height, rtl, [&]() {
        VXUI( &fixture->ctx, "settings.header", {
            .layout = {
                .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                .childGap = 8,
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
        } ) {
            VXUI_LABEL( &fixture->ctx, demo_layout_store_localized( fixture, test_case.locale, "Settings", "設定", "الإعدادات" ), ( vxui_label_cfg ) {
                .font_id = VXUI_TEST_FONT_ROLE_TITLE,
                .font_size = 44.0f,
            } );
        }

        VXUI( &fixture->ctx, "settings.body_panel", {
            .layout = {
                .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_GROW( 180.0f ) },
            },
        } ) {
            vxui_menu_begin( &fixture->ctx, &fixture->settings_menu_state, "settings.body_menu", ( vxui_menu_cfg ) {
                .style = &form_style,
                .viewport_height = layout.menu_viewport_height,
            } );
            vxui_menu_section( &fixture->ctx, &fixture->settings_menu_state, "interface", demo_layout_localized( test_case.locale, "Interface", "インターフェース", "الواجهة" ), ( vxui_menu_section_cfg ) { 0 } );
            vxui_menu_option( &fixture->ctx, &fixture->settings_menu_state, "challenge", demo_layout_localized( test_case.locale, "Challenge", "難易度", "التحدي" ), &challenge, kChallengeOptions, 3, ( vxui_menu_row_cfg ) { 0 }, ( vxui_option_cfg ) { 0 } );
            vxui_menu_slider( &fixture->ctx, &fixture->settings_menu_state, "volume", demo_layout_localized( test_case.locale, "Volume", "音量", "الصوت" ), &volume, 0.0f, 1.0f, ( vxui_menu_row_cfg ) { 0 }, ( vxui_slider_cfg ) { .show_value = true, .format = "%.2f" } );
            vxui_menu_option( &fixture->ctx, &fixture->settings_menu_state, "scanlines", demo_layout_localized( test_case.locale, "Scanlines", "走査線", "خطوط المسح" ), &toggle, kToggleOptions, 2, ( vxui_menu_row_cfg ) { 0 }, ( vxui_option_cfg ) { 0 } );
            vxui_menu_option( &fixture->ctx, &fixture->settings_menu_state, "effect_intensity", demo_layout_localized( test_case.locale, "Effect Intensity", "効果強度", "شدة التأثير" ), &toggle, kChallengeOptions, 3, ( vxui_menu_row_cfg ) { 0 }, ( vxui_option_cfg ) { 0 } );
            vxui_menu_option( &fixture->ctx, &fixture->settings_menu_state, "prompt_table", demo_layout_localized( test_case.locale, "Prompts", "プロンプト", "الرموز" ), &fixture->prompt_table_index, kPromptOptions, 2, ( vxui_menu_row_cfg ) { 0 }, ( vxui_option_cfg ) { 0 } );
            vxui_menu_option( &fixture->ctx, &fixture->settings_menu_state, "locale_index", demo_layout_localized( test_case.locale, "Locale", "ロケール", "اللغة" ), &challenge, kLocaleOptions, 3, ( vxui_menu_row_cfg ) { 0 }, ( vxui_option_cfg ) { 0 } );
            for ( int i = 0; i < 4 + fixture->extra_menu_rows; ++i ) {
                std::string row_id = "overflow_" + std::to_string( i );
                std::string row_label = "Overflow " + std::to_string( i );
                vxui_menu_option( &fixture->ctx, &fixture->settings_menu_state, row_id.c_str(), demo_layout_store( fixture, row_label ), &toggle, kToggleOptions, 2, ( vxui_menu_row_cfg ) { 0 }, ( vxui_option_cfg ) { 0 } );
            }
            vxui_menu_end( &fixture->ctx, &fixture->settings_menu_state );
        }

        VXUI( &fixture->ctx, "settings.footer", {
            .layout = {
                .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                .padding = CLAY_PADDING_ALL( 18 ),
                .childGap = ( uint16_t ) VXUI_DEMO_LAYOUT_ROW_GAP,
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
        } ) {
            VXUI_LABEL( &fixture->ctx, demo_layout_store_localized( fixture, test_case.locale, "Return to Command Deck", "コマンドデッキへ戻る", "العودة إلى منصة الأوامر" ), ( vxui_label_cfg ) { 0 } );
        }
    } );
}

static void demo_layout_render_credits( demo_layout_fixture* fixture, const demo_layout_case& test_case )
{
    const bool rtl = fixture->ctx.rtl;
    const float viewport_width = std::max( 0.0f, ( float ) test_case.width - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f );
    const float surface_height = std::max( 0.0f, ( float ) test_case.height - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f );
    const vxui_demo_surface_metrics metrics =
        vxui_demo_compute_surface_metrics( viewport_width, test_case.locale, VXUI_DEMO_SURFACE_CREDITS );

    demo_layout_emit_screen_surface( fixture, "credits", "credits.surface", metrics.surface_width, surface_height, rtl, [&]() {
        const char* screen_count_text = demo_layout_store( fixture, std::to_string( fixture->ctx.screen_count ) );
        vxui_menu_status_item footer_status[] = {
            { demo_layout_status_label( test_case.locale, "status.short.locale" ), demo_layout_locale_name( test_case.locale ), VXUI_MENU_STATUS_PRIMARY, false, false, "credits.footer.status.locale" },
            { demo_layout_status_label( test_case.locale, "status.short.prompts" ), demo_layout_prompt_name( test_case.locale, test_case.prompt_table_index ), VXUI_MENU_STATUS_SECONDARY, true, false, "credits.footer.status.prompts" },
            { demo_layout_status_label( test_case.locale, "status.short.screens" ), screen_count_text, VXUI_MENU_STATUS_SECONDARY, true, false, "credits.footer.status.screens" },
            { demo_layout_status_label( test_case.locale, "status.short.top" ), demo_layout_screen_name( test_case.locale, test_case.screen_name ), VXUI_MENU_STATUS_PRIMARY, false, false, "credits.footer.status.top" },
        };
        vxui_menu_footer_cfg footer_cfg = {
            nullptr,
            0,
            footer_status,
            ( int ) ( sizeof( footer_status ) / sizeof( footer_status[ 0 ] ) ),
            VXUI_MENU_SHELL_COMPACT_AUTO,
            surface_height <= 680.0f ? 3 : 4,
            false,
        };
        vxui_menu_style footer_style = demo_layout_footer_style();
        vxui_menu_screen_cfg screen_cfg = {
            VXUI_MENU_SHELL_FORM,
            &footer_style,
            VXUI_MENU_SHELL_COMPACT_AUTO,
            680.0f,
            0.0f,
            false,
            {},
            {},
            {},
            {},
            {},
            footer_cfg,
        };
        vxui_menu_state shell_state = {};

        vxui_menu_screen_begin( &fixture->ctx, &shell_state, "credits.shell", &screen_cfg );
        VXUI_LABEL( &fixture->ctx, demo_layout_store_localized( fixture, test_case.locale, "Credits", "クレジット", "الاعتمادات" ), ( vxui_label_cfg ) {
            .font_id = VXUI_TEST_FONT_ROLE_TITLE,
            .font_size = 44.0f,
        } );
        VXUI( &fixture->ctx, "credits.stack", {
            .layout = {
                .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_GROW( 160.0f ) },
                .padding = CLAY_PADDING_ALL( 20 ),
                .childGap = 12,
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
        } ) {
            VXUI_LABEL( &fixture->ctx, demo_layout_store_localized( fixture, test_case.locale, "Credits Stack", "クレジット構成", "هيكل الاعتمادات" ), ( vxui_label_cfg ) {
                .font_id = VXUI_TEST_FONT_ROLE_BODY,
                .font_size = 22.0f,
            } );
            VXUI( &fixture->ctx, "credits.body_viewport", {
                .layout = {
                    .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_GROW( 120.0f ) },
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                },
                .clip = {
                    .horizontal = true,
                    .vertical = true,
                },
            } ) {
                VXUI( &fixture->ctx, "credits.body", {
                    .layout = {
                        .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                        .padding = { 0, 0, 0, 12 },
                        .childGap = 12,
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    },
                } ) {
                    for ( int i = 0; i < 4; ++i ) {
                        VXUI_LABEL( &fixture->ctx, demo_layout_detail_text( fixture, test_case.locale ), ( vxui_label_cfg ) { 0 } );
                    }
                }
            }
        }
        VXUI( &fixture->ctx, "credits.actions", {
            .layout = {
                .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                .padding = CLAY_PADDING_ALL( 18 ),
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
        } ) {
            VXUI_LABEL( &fixture->ctx, demo_layout_store_localized( fixture, test_case.locale, "Back", "戻る", "رجوع" ), ( vxui_label_cfg ) { 0 } );
        }
        vxui_menu_footer( &fixture->ctx, "credits.footer", &screen_cfg.footer );
        vxui_menu_screen_end( &fixture->ctx, &shell_state );
    } );
}

static void demo_layout_render_stub_with_footer( demo_layout_fixture* fixture, const demo_layout_case& test_case, const char* root_id )
{
    const bool rtl = fixture->ctx.rtl;
    const vxui_demo_surface_kind kind = std::strcmp( root_id, "launch_stub" ) == 0 ? VXUI_DEMO_SURFACE_LAUNCH_STUB : VXUI_DEMO_SURFACE_RESULTS_STUB;
    const float viewport_width = std::max( 0.0f, ( float ) test_case.width - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f );
    const float surface_height = std::max( 0.0f, ( float ) test_case.height - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f );
    const vxui_demo_surface_metrics metrics = vxui_demo_compute_surface_metrics( viewport_width, test_case.locale, kind );

    demo_layout_emit_screen_surface( fixture, root_id, vxui_demo_surface_id( kind ), metrics.surface_width, surface_height, rtl, [&]() {
        const char* screen_count_text = demo_layout_store( fixture, std::to_string( fixture->ctx.screen_count ) );
        const char* prompt_label = demo_layout_store_localized(
            fixture,
            test_case.locale,
            std::strcmp( root_id, "launch_stub" ) == 0 ? "Confirm handoff" : "Return to command deck",
            std::strcmp( root_id, "launch_stub" ) == 0 ? "遷移を確定" : "コマンドデッキへ戻る",
            std::strcmp( root_id, "launch_stub" ) == 0 ? "تأكيد التحويل" : "العودة إلى منصة الأوامر" );
        vxui_menu_prompt_item footer_prompts[] = {
            { "action.confirm", prompt_label, false, demo_layout_store( fixture, std::string( root_id ) + ".footer.prompt.confirm" ) },
        };
        vxui_menu_status_item footer_status[] = {
            { demo_layout_status_label( test_case.locale, "status.short.locale" ), demo_layout_locale_name( test_case.locale ), VXUI_MENU_STATUS_PRIMARY, false, false, demo_layout_store( fixture, std::string( root_id ) + ".footer.status.locale" ) },
            { demo_layout_status_label( test_case.locale, "status.short.prompts" ), demo_layout_prompt_name( test_case.locale, test_case.prompt_table_index ), VXUI_MENU_STATUS_SECONDARY, true, false, demo_layout_store( fixture, std::string( root_id ) + ".footer.status.prompts" ) },
            { demo_layout_status_label( test_case.locale, "status.short.screens" ), screen_count_text, VXUI_MENU_STATUS_SECONDARY, true, false, demo_layout_store( fixture, std::string( root_id ) + ".footer.status.screens" ) },
            { demo_layout_status_label( test_case.locale, "status.short.top" ), demo_layout_screen_name( test_case.locale, test_case.screen_name ), VXUI_MENU_STATUS_PRIMARY, false, false, demo_layout_store( fixture, std::string( root_id ) + ".footer.status.top" ) },
        };
        vxui_menu_footer_cfg footer_cfg = {
            footer_prompts,
            ( int ) ( sizeof( footer_prompts ) / sizeof( footer_prompts[ 0 ] ) ),
            footer_status,
            ( int ) ( sizeof( footer_status ) / sizeof( footer_status[ 0 ] ) ),
            VXUI_MENU_SHELL_COMPACT_AUTO,
            surface_height <= 680.0f ? 3 : 4,
            false,
        };
        vxui_menu_style footer_style = demo_layout_footer_style();
        vxui_menu_screen_cfg screen_cfg = {
            VXUI_MENU_SHELL_FORM,
            &footer_style,
            VXUI_MENU_SHELL_COMPACT_AUTO,
            680.0f,
            0.0f,
            false,
            {},
            {},
            {},
            {},
            {},
            footer_cfg,
        };
        vxui_menu_state shell_state = {};

        vxui_menu_screen_begin( &fixture->ctx, &shell_state, demo_layout_store( fixture, std::string( root_id ) + ".shell" ), &screen_cfg );
        VXUI_LABEL( &fixture->ctx, demo_layout_detail_text( fixture, test_case.locale ), ( vxui_label_cfg ) {
            .font_id = VXUI_TEST_FONT_ROLE_TITLE,
            .font_size = 40.0f,
        } );
        CLAY( Clay_GetElementId( demo_layout_clay_string( demo_layout_store( fixture, std::string( root_id ) + ".actions" ) ) ), {
            .layout = {
                .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                .padding = CLAY_PADDING_ALL( 18 ),
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
        } ) {
            VXUI_LABEL( &fixture->ctx, demo_layout_store_localized( fixture, test_case.locale, "Continue", "続行", "متابعة" ), ( vxui_label_cfg ) { 0 } );
        }
        vxui_menu_footer( &fixture->ctx, demo_layout_store( fixture, std::string( root_id ) + ".footer" ), &screen_cfg.footer );
        vxui_menu_screen_end( &fixture->ctx, &shell_state );
    } );
}

static vxui_draw_list demo_layout_render_case( demo_layout_fixture* fixture, const demo_layout_case& test_case )
{
    demo_layout_begin_case( fixture, test_case );

    if ( std::strcmp( test_case.screen_name, "boot" ) == 0 ) {
        demo_layout_render_boot( fixture, test_case );
    } else if ( std::strcmp( test_case.screen_name, "title" ) == 0 ) {
        demo_layout_render_title( fixture, test_case );
    } else if ( std::strcmp( test_case.screen_name, "main_menu" ) == 0 ) {
        demo_layout_render_main_menu( fixture, test_case );
    } else if ( std::strcmp( test_case.screen_name, "sortie" ) == 0 ) {
        demo_layout_render_split_screen( fixture, test_case, VXUI_DEMO_SURFACE_SORTIE );
    } else if ( std::strcmp( test_case.screen_name, "loadout" ) == 0 ) {
        demo_layout_render_split_screen( fixture, test_case, VXUI_DEMO_SURFACE_LOADOUT );
    } else if ( std::strcmp( test_case.screen_name, "archives" ) == 0 ) {
        demo_layout_render_split_screen( fixture, test_case, VXUI_DEMO_SURFACE_ARCHIVES );
    } else if ( std::strcmp( test_case.screen_name, "settings" ) == 0 ) {
        demo_layout_render_settings( fixture, test_case );
    } else if ( std::strcmp( test_case.screen_name, "records" ) == 0 ) {
        demo_layout_render_split_screen( fixture, test_case, VXUI_DEMO_SURFACE_RECORDS );
    } else if ( std::strcmp( test_case.screen_name, "credits" ) == 0 ) {
        demo_layout_render_credits( fixture, test_case );
    } else if ( std::strcmp( test_case.screen_name, "launch_stub" ) == 0 ) {
        demo_layout_render_stub_with_footer( fixture, test_case, "launch_stub" );
    } else {
        demo_layout_render_stub_with_footer( fixture, test_case, "results_stub" );
    }

    return vxui_end( &fixture->ctx );
}

static std::vector< std::string > demo_layout_collect_warnings(
    demo_layout_fixture* fixture,
    const vxui_draw_list* list,
    const char* screen_name )
{
    std::vector< std::string > warnings;
    vxui_demo_layout_probe probe = {
        &fixture->ctx,
        list,
        screen_name,
        &fixture->ctx,
        demo_layout_probe_lookup_bounds_by_id,
        demo_layout_probe_lookup_bounds_by_hash,
        demo_layout_probe_lookup_anim_bounds,
    };
    vxui_demo_collect_layout_warnings( probe, &warnings );
    return warnings;
}

static bool demo_layout_has_text( const vxui_draw_list* list, const char* text )
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

static bool demo_layout_has_text_containing( const vxui_draw_list* list, const char* fragment )
{
    if ( !list || !fragment ) {
        return false;
    }
    for ( int i = 0; i < list->length; ++i ) {
        const vxui_cmd& cmd = list->commands[ i ];
        if ( cmd.type == VXUI_CMD_TEXT && cmd.text.text && std::strstr( cmd.text.text, fragment ) != nullptr ) {
            return true;
        }
    }
    return false;
}

static bool demo_layout_rect_reads_inline_pair( vxui_rect rect )
{
    return rect.w > rect.h * 1.5f;
}

static bool demo_layout_main_menu_preview_text_fully_visible(
    demo_layout_fixture* fixture,
    const demo_layout_case& test_case,
    const vxui_draw_list* list,
    vxui_rect preview_body )
{
    ( void ) fixture;
    ( void ) test_case;
    if ( !list ) {
        return false;
    }
    return vxui_demo_text_group_fully_visible( list, preview_body, [&]( const vxui_cmd& cmd ) {
        const float x = cmd.text.pos.x;
        const float y = cmd.text.pos.y;
        return x >= preview_body.x - 1.0f
            && x <= preview_body.x + preview_body.w + 1.0f
            && y >= preview_body.y - 1.0f
            && y <= preview_body.y + preview_body.h + 1.0f;
    } );
}

static bool demo_layout_split_body_text_fully_visible(
    demo_layout_fixture* fixture,
    const demo_layout_case& test_case,
    const vxui_draw_list* list,
    vxui_rect detail_body )
{
    const char* detail_text = demo_layout_detail_text( fixture, test_case.locale );
    if ( !detail_text || !list ) {
        return false;
    }
    const vxui_rect tolerant_body = {
        detail_body.x - 2.0f,
        detail_body.y - 2.0f,
        detail_body.w + 4.0f,
        detail_body.h + 4.0f,
    };
    std::vector< std::string > detail_lines;
    const char* cursor = detail_text;
    while ( cursor && *cursor ) {
        const char* newline = std::strchr( cursor, '\n' );
        const size_t line_len = newline ? ( size_t ) ( newline - cursor ) : std::strlen( cursor );
        if ( line_len > 0 ) {
            detail_lines.emplace_back( cursor, line_len );
        }
        if ( !newline ) {
            break;
        }
        cursor = newline + 1;
    }
    if ( detail_lines.empty() ) {
        detail_lines.emplace_back( detail_text );
    }
    std::vector< std::string > detail_fragments;
    for ( const std::string& line : detail_lines ) {
        const size_t fragment_len = std::min< size_t >( 18, line.size() );
        detail_fragments.emplace_back( line.substr( 0, fragment_len ) );
        if ( !demo_layout_has_text_containing( list, detail_fragments.back().c_str() ) ) {
            return false;
        }
    }
    return vxui_demo_text_group_fully_visible( list, tolerant_body, [&]( const vxui_cmd& cmd ) {
        for ( const std::string& fragment : detail_fragments ) {
            if ( std::strstr( cmd.text.text, fragment.c_str() ) != nullptr ) {
                return true;
            }
        }
        return false;
    } );
}

static bool demo_layout_preview_body_and_help_do_not_overlap(
    demo_layout_fixture* fixture,
    const demo_layout_case& test_case,
    const vxui_draw_list* list )
{
    vxui_rect preview_panel = {};
    vxui_rect preview_body = {};
    vxui_rect help = {};
    if ( !demo_layout_find_element_bounds( "main.preview_panel", &preview_panel )
        || !demo_layout_find_element_bounds( "main.preview_body", &preview_body )
        || !demo_layout_find_element_bounds( "main.preview.help_legend", &help ) ) {
        return false;
    }

    return vxui_demo_rect_inside( preview_panel, preview_body, 0.0f )
        && vxui_demo_rect_inside( preview_panel, help, 0.0f )
        && vxui_demo_vertical_stack_order( preview_body, help, 8.0f )
        && vxui_demo_rects_non_overlapping( preview_body, help, 0.0f )
        && demo_layout_main_menu_preview_text_fully_visible( fixture, test_case, list, preview_body );
}

static bool demo_layout_find_controls_block_regions( const char* id, vxui_rect* out, vxui_rect* title, vxui_rect* lines, int line_count )
{
    if ( !id || !out || !title || !lines || line_count <= 0 ) {
        return false;
    }
    if ( !demo_layout_find_element_bounds( id, out ) ) {
        return false;
    }
    const std::string title_id = vxui_demo_controls_block_title_id( id );
    if ( !demo_layout_find_element_bounds( title_id.c_str(), title ) ) {
        return false;
    }
    for ( int i = 0; i < line_count; ++i ) {
        const std::string line_id = vxui_demo_controls_block_line_id( id, i );
        if ( !demo_layout_find_element_bounds( line_id.c_str(), &lines[ i ] ) ) {
            return false;
        }
    }
    return true;
}

static bool demo_layout_find_controls_block_text_bounds(
    const demo_layout_fixture* fixture,
    const vxui_demo_controls_block_copy& copy,
    vxui_rect* title,
    vxui_rect* lines,
    int line_count )
{
    if ( !fixture || !title || !lines || line_count <= 0 ) {
        return false;
    }
    if ( !demo_layout_find_text_owner_bounds( fixture, copy.title, title ) ) {
        return false;
    }
    for ( int i = 0; i < line_count; ++i ) {
        if ( !demo_layout_find_text_owner_bounds( fixture, copy.lines[ i ], &lines[ i ] ) ) {
            return false;
        }
    }
    return true;
}

static float demo_layout_main_menu_help_owner_width( int screen_width, int screen_height, const char* locale )
{
    const float viewport_width = std::max( 0.0f, ( float ) screen_width - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f );
    const float surface_max_height = std::max( 0.0f, ( float ) screen_height - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f );
    const vxui_demo_main_menu_layout_spec layout =
        vxui_demo_resolve_main_menu_layout( viewport_width, surface_max_height, locale );
    const float panel_padding = vxui_demo_main_menu_preview_uses_compact_layout( layout ) ? 8.0f : layout.preview_panel_padding;
    return std::max( 0.0f, layout.preview_panel_width - panel_padding * 2.0f );
}

static vxui_demo_controls_block_copy demo_layout_controls_copy_for_screen_size( const char* locale, int screen_width, int screen_height )
{
    const float surface_max_height = std::max( 0.0f, ( float ) screen_height - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f );
    const float help_owner_width = demo_layout_main_menu_help_owner_width( screen_width, screen_height, locale );
    const vxui_demo_controls_block_contract contract = vxui_demo_get_controls_block_contract( surface_max_height, help_owner_width );
    return vxui_demo_controls_block_copy_for_locale( locale, contract.compact_copy );
}

static int demo_layout_main_menu_visible_help_line_count( const char* locale, int screen_width, int screen_height )
{
    const float surface_max_height = std::max( 0.0f, ( float ) screen_height - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f );
    const float viewport_width = std::max( 0.0f, ( float ) screen_width - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f );
    const vxui_demo_main_menu_layout_spec layout = vxui_demo_resolve_main_menu_layout( viewport_width, surface_max_height, locale );
    const bool tight_preview_width = layout.preview_panel_width <= 420.0f;
    const bool compact_help = surface_max_height <= 648.0f || tight_preview_width;
    const vxui_demo_controls_block_copy copy = demo_layout_controls_copy_for_screen_size( locale, screen_width, screen_height );
    const int line_count = vxui_demo_controls_block_visible_line_count( copy );
    if ( tight_preview_width ) {
        return std::min( line_count, 2 );
    }
    return compact_help ? std::min( line_count, 3 ) : line_count;
}

UTEST_F_SETUP( demo_layout_fixture )
{
    utest_fixture->memory_size = vxui_min_memory_size();
    utest_fixture->memory = ( uint8_t* ) std::malloc( ( size_t ) utest_fixture->memory_size );
    ASSERT_TRUE( utest_fixture->memory != nullptr );

    vxui_init(
        &utest_fixture->ctx,
        vxui_create_arena( utest_fixture->memory_size, utest_fixture->memory ),
        ( vxui_config ) {
            .screen_width = 1280,
            .screen_height = 720,
            .max_elements = 512,
        } );
    utest_fixture->fontcache = vxui_test_fontcache_create( true );
    ASSERT_TRUE( utest_fixture->fontcache != nullptr );
    EXPECT_EQ( vxui_test_fontcache_load_file( utest_fixture->fontcache, VXUI_SOURCE_DIR "/vefc/demo/fonts/OpenSans-Regular.ttf", 24.0f ), ( int64_t ) VXUI_TEST_FONT_UI );
    EXPECT_EQ( vxui_test_fontcache_load_file( utest_fixture->fontcache, VXUI_SOURCE_DIR "/vefc/demo/fonts/Bitter-Regular.ttf", 44.0f ), ( int64_t ) VXUI_TEST_FONT_TITLE );
    EXPECT_EQ( vxui_test_fontcache_load_file( utest_fixture->fontcache, VXUI_SOURCE_DIR "/vefc/demo/fonts/NovaMono-Regular.ttf", 16.0f ), ( int64_t ) VXUI_TEST_FONT_DEBUG );
    EXPECT_EQ( vxui_test_fontcache_load_file( utest_fixture->fontcache, VXUI_SOURCE_DIR "/vefc/demo/fonts/NotoSansJP-Regular.otf", 24.0f ), ( int64_t ) VXUI_TEST_FONT_JAPANESE );
    EXPECT_EQ( vxui_test_fontcache_load_file( utest_fixture->fontcache, VXUI_SOURCE_DIR "/vefc/demo/fonts/Tajawal-Regular.ttf", 24.0f ), ( int64_t ) VXUI_TEST_FONT_ARABIC );
    EXPECT_EQ( vxui_test_fontcache_load_file( utest_fixture->fontcache, VXUI_SOURCE_DIR "/vefc/demo/fonts/OpenSans-Regular.ttf", 44.0f ), ( int64_t ) VXUI_TEST_FONT_UI_LARGE );
    EXPECT_EQ( vxui_test_fontcache_load_file( utest_fixture->fontcache, VXUI_SOURCE_DIR "/vefc/demo/fonts/NotoSansJP-Regular.otf", 44.0f ), ( int64_t ) VXUI_TEST_FONT_JAPANESE_TITLE );
    EXPECT_EQ( vxui_test_fontcache_load_file( utest_fixture->fontcache, VXUI_SOURCE_DIR "/vefc/demo/fonts/Tajawal-Regular.ttf", 44.0f ), ( int64_t ) VXUI_TEST_FONT_ARABIC_TITLE );
    vxui_set_fontcache( &utest_fixture->ctx, vxui_test_fontcache_ptr( utest_fixture->fontcache ) );
    utest_fixture->ctx.default_font_id = VXUI_TEST_FONT_UI;
    utest_fixture->ctx.default_font_size = 24.0f;
}

UTEST_F_TEARDOWN( demo_layout_fixture )
{
    vxui_shutdown( &utest_fixture->ctx );
    vxui_test_fontcache_destroy( utest_fixture->fontcache );
    std::free( utest_fixture->memory );
}

UTEST( demo_layout_architecture, authored_demo_files_do_not_use_direct_clay )
{
    const char* files[] = {
        "main.cpp",
        "demo/internal/main_menu_shared.h",
        "demo/internal/split_deck_shared.h",
    };
    for ( const char* path : files ) {
        const std::string source = demo_layout_read_source_file( path );
        ASSERT_TRUE( !source.empty() );
        EXPECT_TRUE( source.find( "CLAY(" ) == std::string::npos );
    }
}

UTEST_F( demo_layout_fixture, main_menu_real_content_matches_production_copy_and_breaks_old_contract )
{
    const demo_layout_case test_case = { "main_menu", "en", 1280, 648, 0, DEMO_TEXT_PACK_NORMAL, DEMO_FOCUS_DETAIL_HEAVY, 0, 0 };
    vxui_draw_list list = demo_layout_render_case( utest_fixture, test_case );
    const vxui_demo_main_menu_preview* preview =
        vxui_demo_main_menu_preview_from_focused_row( demo_layout_menu_row_id( "main.command_menu", "loadout" ) );
    const vxui_demo_controls_block_copy controls =
        demo_layout_controls_copy_for_screen_size( test_case.locale, test_case.width, test_case.height );
    const float surface_max_height = std::max( 0.0f, ( float ) test_case.height - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f );
    const float help_owner_width = demo_layout_main_menu_help_owner_width( test_case.width, test_case.height, test_case.locale );
    const int help_line_count = demo_layout_main_menu_visible_help_line_count( test_case.locale, test_case.width, test_case.height );
    vxui_rect help = {};
    vxui_rect help_title = {};
    vxui_rect help_lines[ 4 ] = {};

    ASSERT_TRUE( preview != nullptr );
    EXPECT_TRUE( demo_layout_has_text( &list, preview->title ) );
    if ( surface_max_height > 720.0f ) {
        EXPECT_TRUE( demo_layout_has_text( &list, preview->subtitle ) );
    }
    EXPECT_TRUE( demo_layout_has_text( &list, preview->body ) );
    EXPECT_TRUE( demo_layout_has_text( &list, vxui_demo_badge_text( test_case.locale, preview->badge_key ) ) );
    EXPECT_TRUE( demo_layout_has_text( &list, controls.title ) );
    ASSERT_TRUE( demo_layout_find_controls_block_regions( "main.preview.help_legend", &help, &help_title, help_lines, help_line_count ) );
}

UTEST_F( demo_layout_fixture, main_menu_production_parity_harness_matches_runtime_layout_structure )
{
    const demo_layout_case test_case = { "main_menu", "en", 1280, 648, 0, DEMO_TEXT_PACK_NORMAL, DEMO_FOCUS_DETAIL_HEAVY, 0, 0 };
    ( void ) demo_layout_render_case( utest_fixture, test_case );
    const float surface_max_height = std::max( 0.0f, ( float ) test_case.height - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f );
    const float help_owner_width = demo_layout_main_menu_help_owner_width( test_case.width, test_case.height, test_case.locale );
    const int help_line_count = demo_layout_main_menu_visible_help_line_count( test_case.locale, test_case.width, test_case.height );

    vxui_rect preview_panel = {};
    vxui_rect preview_header = {};
    vxui_rect preview_body = {};
    vxui_rect help = {};
    vxui_rect footer = {};
    vxui_rect footer_prompts = {};
    vxui_rect footer_status = {};
    vxui_rect help_title = {};
    vxui_rect help_lines[ 4 ] = {};
    ASSERT_TRUE( demo_layout_find_element_bounds( "main.preview_panel", &preview_panel ) );
    ASSERT_TRUE( demo_layout_find_element_bounds( "main.preview_header", &preview_header ) );
    ASSERT_TRUE( demo_layout_find_element_bounds( "main.preview_body", &preview_body ) );
    ASSERT_TRUE( demo_layout_find_controls_block_regions( "main.preview.help_legend", &help, &help_title, help_lines, help_line_count ) );
    ASSERT_TRUE( demo_layout_find_element_bounds( "main.footer", &footer ) );
    ASSERT_TRUE( demo_layout_find_element_bounds( "main.footer.prompts", &footer_prompts ) );
    ASSERT_TRUE( demo_layout_find_element_bounds( "main.footer.status", &footer_status ) );
}

UTEST_F( demo_layout_fixture, main_menu_preview_help_is_fully_visible_in_supported_short_landscape_heights )
{
    const int sizes[][ 2 ] = { { 1280, 680 }, { 1280, 648 }, { 1100, 648 } };
    for ( const auto& size : sizes ) {
        const demo_layout_case test_case = { "main_menu", "en", size[ 0 ], size[ 1 ], 0, DEMO_TEXT_PACK_NORMAL, DEMO_FOCUS_DETAIL_HEAVY, 0, 0 };
        vxui_draw_list list = demo_layout_render_case( utest_fixture, test_case );
        const vxui_demo_controls_block_copy controls =
            demo_layout_controls_copy_for_screen_size( test_case.locale, test_case.width, test_case.height );

        vxui_rect panel = {};
        vxui_rect body = {};
        vxui_rect help = {};
        ASSERT_TRUE( demo_layout_find_element_bounds( "main.preview_panel", &panel ) );
        ASSERT_TRUE( demo_layout_find_element_bounds( "main.preview_body", &body ) );
        ASSERT_TRUE( demo_layout_find_element_bounds( "main.preview.help_legend", &help ) );
        EXPECT_TRUE( vxui_demo_vertical_stack_order( body, help, 0.0f ) );
        EXPECT_TRUE( vxui_demo_element_fully_visible_inside( panel, help, 0.0f ) );
        EXPECT_TRUE( vxui_demo_text_group_fully_visible( &list, help, [&]( const vxui_cmd& cmd ) {
            if ( std::strcmp( cmd.text.text, controls.title ) == 0 ) {
                return true;
            }
            for ( const char* line : controls.lines ) {
                if ( std::strcmp( cmd.text.text, line ) == 0 ) {
                    return true;
                }
            }
            return false;
        } ) );
    }
}

UTEST_F( demo_layout_fixture, main_menu_footer_prompt_band_remains_fully_visible_in_supported_short_landscape_heights )
{
    const int sizes[][ 2 ] = { { 1280, 680 }, { 1280, 648 }, { 1100, 648 } };
    const vxui_demo_main_menu_preview* preview =
        vxui_demo_main_menu_preview_from_focused_row( demo_layout_menu_row_id( "main.command_menu", "loadout" ) );
    ASSERT_TRUE( preview != nullptr );
    for ( const auto& size : sizes ) {
        const demo_layout_case test_case = { "main_menu", "en", size[ 0 ], size[ 1 ], 0, DEMO_TEXT_PACK_NORMAL, DEMO_FOCUS_DETAIL_HEAVY, 0, 0 };
        vxui_draw_list list = demo_layout_render_case( utest_fixture, test_case );
        const vxui_demo_controls_block_copy controls =
            demo_layout_controls_copy_for_screen_size( test_case.locale, test_case.width, test_case.height );

        vxui_rect surface = {};
        vxui_rect footer = {};
        vxui_rect footer_prompts = {};
        vxui_rect footer_status = {};
        ASSERT_TRUE( demo_layout_find_element_bounds( "main.surface", &surface ) );
        ASSERT_TRUE( demo_layout_find_element_bounds( "main.footer", &footer ) );
        ASSERT_TRUE( demo_layout_find_element_bounds( "main.footer.prompts", &footer_prompts ) );
        ASSERT_TRUE( demo_layout_find_element_bounds( "main.footer.status", &footer_status ) );
        EXPECT_TRUE( vxui_demo_element_fully_visible_inside( surface, footer, 0.0f ) );
        EXPECT_TRUE( vxui_demo_element_fully_visible_inside( footer, footer_prompts, 0.0f ) );
        EXPECT_TRUE( vxui_demo_element_fully_visible_inside( footer, footer_status, 0.0f ) );
        EXPECT_TRUE( vxui_demo_no_text_in_band( &list, footer.y, footer.y + footer.h, [&]( const vxui_cmd& cmd ) {
            if ( std::strcmp( cmd.text.text, preview->body ) == 0 ) {
                return true;
            }
            if ( std::strcmp( cmd.text.text, controls.title ) == 0 ) {
                return true;
            }
            for ( const char* line : controls.lines ) {
                if ( std::strcmp( cmd.text.text, line ) == 0 ) {
                    return true;
                }
            }
            return false;
        } ) );
    }
}

UTEST_F( demo_layout_fixture, main_menu_preview_panel_contains_visible_text_and_help_block )
{
    const demo_layout_case test_case = { "main_menu", "en", 1280, 720, 0, DEMO_TEXT_PACK_LONG, DEMO_FOCUS_DETAIL_HEAVY, 0, 2 };
    vxui_draw_list list = demo_layout_render_case( utest_fixture, test_case );

    vxui_rect panel = {};
    vxui_rect header = {};
    vxui_rect body = {};
    vxui_rect help = {};
    ASSERT_TRUE( demo_layout_find_element_bounds( "main.preview_panel", &panel ) );
    ASSERT_TRUE( demo_layout_find_element_bounds( "main.preview_header", &header ) );
    ASSERT_TRUE( demo_layout_find_element_bounds( "main.preview_body", &body ) );
    ASSERT_TRUE( demo_layout_find_element_bounds( "main.preview.help_legend", &help ) );

    EXPECT_TRUE( vxui_demo_rect_inside( panel, header, 0.0f ) );
    EXPECT_TRUE( vxui_demo_rect_inside( panel, body, 0.0f ) );
    EXPECT_TRUE( vxui_demo_rect_inside( panel, help, 0.0f ) );
    EXPECT_TRUE( demo_layout_main_menu_preview_text_fully_visible( utest_fixture, test_case, &list, body ) );
}

UTEST_F( demo_layout_fixture, main_menu_help_lines_do_not_overlap_in_supported_short_heights )
{
    const int sizes[][ 2 ] = { { 1280, 680 }, { 1280, 648 }, { 1100, 648 } };
    const demo_layout_focus_mode focuses[] = { DEMO_FOCUS_FIRST, DEMO_FOCUS_DETAIL_HEAVY };
    for ( const auto& size : sizes ) {
        for ( demo_layout_focus_mode focus : focuses ) {
            const demo_layout_case test_case = { "main_menu", "en", size[ 0 ], size[ 1 ], 0, DEMO_TEXT_PACK_NORMAL, focus, 0, 0 };
            ( void ) demo_layout_render_case( utest_fixture, test_case );
            const float surface_max_height = std::max( 0.0f, ( float ) test_case.height - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f );
            const float help_owner_width = demo_layout_main_menu_help_owner_width( test_case.width, test_case.height, test_case.locale );
            const int help_line_count = demo_layout_main_menu_visible_help_line_count( test_case.locale, test_case.width, test_case.height );
            vxui_rect help = {};
            vxui_rect help_title = {};
            vxui_rect help_lines[ 4 ] = {};
            ASSERT_TRUE( demo_layout_find_controls_block_regions( "main.preview.help_legend", &help, &help_title, help_lines, help_line_count ) );

            vxui_rect stack[ 5 ] = { help_title, help_lines[ 0 ], help_lines[ 1 ], help_lines[ 2 ], help_lines[ 3 ] };
            const int stack_count = 1 + help_line_count;
            EXPECT_TRUE( vxui_demo_element_group_fully_visible( help, stack, stack_count, 0.0f ) );
            EXPECT_TRUE( vxui_demo_elements_form_vertical_stack( stack, stack_count, 0.0f ) );
            EXPECT_TRUE( vxui_demo_elements_non_overlapping( stack, stack_count, 0.0f ) );
        }
    }
}

UTEST_F( demo_layout_fixture, main_menu_help_block_lines_remain_fully_visible_and_stacked )
{
    const char* locales[] = { "en", "ja-JP", "ar" };
    for ( const char* locale : locales ) {
        const demo_layout_case test_case = { "main_menu", locale, 1280, 648, 0, DEMO_TEXT_PACK_NORMAL, DEMO_FOCUS_DETAIL_HEAVY, 0, 0 };
        ( void ) demo_layout_render_case( utest_fixture, test_case );
        const int help_line_count = demo_layout_main_menu_visible_help_line_count( test_case.locale, test_case.width, test_case.height );

        vxui_rect help = {};
        vxui_rect help_title = {};
        vxui_rect help_lines[ 4 ] = {};
        ASSERT_TRUE( demo_layout_find_controls_block_regions( "main.preview.help_legend", &help, &help_title, help_lines, help_line_count ) );

        vxui_rect stack[ 5 ] = { help_title, help_lines[ 0 ], help_lines[ 1 ], help_lines[ 2 ], help_lines[ 3 ] };
        const int stack_count = 1 + help_line_count;
        EXPECT_TRUE( vxui_demo_element_group_fully_visible( help, stack, stack_count, 0.0f ) );
        EXPECT_TRUE( vxui_demo_elements_form_vertical_stack( stack, stack_count, 0.0f ) );
        EXPECT_TRUE( vxui_demo_elements_non_overlapping( stack, stack_count, 0.0f ) );
    }
}

UTEST_F( demo_layout_fixture, main_menu_preview_body_and_help_regions_do_not_visually_collide )
{
    const int sizes[][ 2 ] = { { 1280, 680 }, { 1280, 648 }, { 1100, 648 } };
    const demo_layout_focus_mode focuses[] = { DEMO_FOCUS_FIRST, DEMO_FOCUS_DETAIL_HEAVY };
    for ( const auto& size : sizes ) {
        for ( demo_layout_focus_mode focus : focuses ) {
            const demo_layout_case test_case = { "main_menu", "en", size[ 0 ], size[ 1 ], 0, DEMO_TEXT_PACK_NORMAL, focus, 0, 0 };
            ( void ) demo_layout_render_case( utest_fixture, test_case );
            const int help_line_count = demo_layout_main_menu_visible_help_line_count( test_case.locale, test_case.width, test_case.height );
            vxui_rect preview_body = {};
            vxui_rect help = {};
            vxui_rect help_title = {};
            vxui_rect help_lines[ 4 ] = {};
            ASSERT_TRUE( demo_layout_find_element_bounds( "main.preview_body", &preview_body ) );
            ASSERT_TRUE( demo_layout_find_controls_block_regions( "main.preview.help_legend", &help, &help_title, help_lines, help_line_count ) );

            EXPECT_TRUE( vxui_demo_vertical_stack_order( preview_body, help, 0.0f ) );
            EXPECT_TRUE( vxui_demo_vertical_stack_order( preview_body, help_title, 0.0f ) );
            EXPECT_TRUE( vxui_demo_rects_non_overlapping( preview_body, help_title, 0.0f ) );
            for ( int i = 0; i < help_line_count; ++i ) {
                EXPECT_TRUE( vxui_demo_rects_non_overlapping( preview_body, help_lines[ i ], 0.0f ) );
            }
        }
    }
}

UTEST_F( demo_layout_fixture, main_menu_footer_status_rows_remain_readable_below_preview_stack )
{
    const int sizes[][ 2 ] = { { 1280, 680 }, { 1280, 648 }, { 1100, 648 } };
    const demo_layout_focus_mode focuses[] = { DEMO_FOCUS_FIRST, DEMO_FOCUS_DETAIL_HEAVY };
    for ( const auto& size : sizes ) {
        for ( demo_layout_focus_mode focus : focuses ) {
            const demo_layout_case test_case = { "main_menu", "en", size[ 0 ], size[ 1 ], 0, DEMO_TEXT_PACK_NORMAL, focus, 0, 0 };
            ( void ) demo_layout_render_case( utest_fixture, test_case );
            const float surface_max_height = std::max( 0.0f, ( float ) test_case.height - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f );
            const float help_owner_width = demo_layout_main_menu_help_owner_width( test_case.width, test_case.height, test_case.locale );
            const int help_line_count = demo_layout_main_menu_visible_help_line_count( test_case.locale, test_case.width, test_case.height );

            vxui_rect help = {};
            vxui_rect help_title = {};
            vxui_rect help_lines[ 4 ] = {};
            vxui_rect footer = {};
            vxui_rect footer_prompts = {};
            vxui_rect footer_status = {};
            vxui_rect footer_status_locale = {};
            vxui_rect footer_status_prompts = {};
            vxui_rect footer_status_screens = {};
            vxui_rect footer_status_top = {};
            ASSERT_TRUE( demo_layout_find_controls_block_regions( "main.preview.help_legend", &help, &help_title, help_lines, help_line_count ) );
            ASSERT_TRUE( demo_layout_find_element_bounds( "main.footer", &footer ) );
            ASSERT_TRUE( demo_layout_find_element_bounds( "main.footer.prompts", &footer_prompts ) );
            ASSERT_TRUE( demo_layout_find_element_bounds( "main.footer.status", &footer_status ) );
            ASSERT_TRUE( demo_layout_find_element_bounds( "main.footer.status.locale", &footer_status_locale ) );
            ASSERT_TRUE( demo_layout_find_element_bounds( "main.footer.status.prompts", &footer_status_prompts ) );
            ASSERT_TRUE( demo_layout_find_element_bounds( "main.footer.status.top", &footer_status_top ) );
            const bool have_screens = demo_layout_find_element_bounds( "main.footer.status.screens", &footer_status_screens );

            EXPECT_TRUE( vxui_demo_element_fully_visible_inside( footer, footer_prompts, 0.0f ) );
            EXPECT_TRUE( vxui_demo_element_fully_visible_inside( footer, footer_status, 0.0f ) );
            EXPECT_TRUE( vxui_demo_element_fully_visible_inside( footer_status, footer_status_locale, 0.0f ) );
            EXPECT_TRUE( vxui_demo_element_fully_visible_inside( footer_status, footer_status_prompts, 0.0f ) );
            EXPECT_TRUE( vxui_demo_element_fully_visible_inside( footer_status, footer_status_top, 0.0f ) );
            EXPECT_TRUE( demo_layout_rect_reads_inline_pair( footer_status_locale ) );
            EXPECT_TRUE( demo_layout_rect_reads_inline_pair( footer_status_prompts ) );
            EXPECT_TRUE( demo_layout_rect_reads_inline_pair( footer_status_top ) );
            EXPECT_TRUE( vxui_demo_vertical_stack_order( help, footer, 0.0f ) );
            EXPECT_TRUE( vxui_demo_rects_non_overlapping( help_title, footer, 0.0f ) );
            for ( int i = 0; i < help_line_count; ++i ) {
                EXPECT_TRUE( vxui_demo_rects_non_overlapping( help_lines[ i ], footer, 0.0f ) );
            }
        }
    }
}

UTEST_F( demo_layout_fixture, main_menu_preview_body_does_not_overlap_controls_block )
{
    const demo_layout_case test_case = { "main_menu", "ja-JP", 1100, 720, 0, DEMO_TEXT_PACK_EXTREME, DEMO_FOCUS_DETAIL_HEAVY, 0, 2 };
    vxui_draw_list list = demo_layout_render_case( utest_fixture, test_case );
    EXPECT_TRUE( demo_layout_preview_body_and_help_do_not_overlap( utest_fixture, test_case, &list ) );
}

UTEST_F( demo_layout_fixture, main_menu_footer_stays_below_deck_for_all_locales )
{
    const char* locales[] = { "en", "ja-JP", "ar" };
    for ( const char* locale : locales ) {
        const demo_layout_case test_case = { "main_menu", locale, 1280, 720, 0, DEMO_TEXT_PACK_LONG, DEMO_FOCUS_FIRST, 0, 0 };
        ( void ) demo_layout_render_case( utest_fixture, test_case );

        vxui_rect command_panel = {};
        vxui_rect preview_panel = {};
        vxui_rect footer = {};
        ASSERT_TRUE( demo_layout_find_element_bounds( "main.command_panel", &command_panel ) );
        ASSERT_TRUE( demo_layout_find_element_bounds( "main.preview_panel", &preview_panel ) );
        ASSERT_TRUE( demo_layout_find_element_bounds( "main.footer", &footer ) );
        vxui_rect deck = {
            std::min( command_panel.x, preview_panel.x ),
            std::min( command_panel.y, preview_panel.y ),
            std::max( command_panel.x + command_panel.w, preview_panel.x + preview_panel.w ) - std::min( command_panel.x, preview_panel.x ),
            std::max( command_panel.y + command_panel.h, preview_panel.y + preview_panel.h ) - std::min( command_panel.y, preview_panel.y ),
        };
        EXPECT_TRUE( vxui_demo_vertical_stack_order( deck, footer, 0.0f ) );
    }
}

UTEST_F( demo_layout_fixture, main_menu_command_and_preview_panels_never_overlap )
{
    const int widths[][ 2 ] = { { 1600, 900 }, { 1280, 720 }, { 1100, 720 }, { 960, 720 } };
    for ( const auto& size : widths ) {
        const demo_layout_case test_case = { "main_menu", "en", size[ 0 ], size[ 1 ], 0, DEMO_TEXT_PACK_NORMAL, DEMO_FOCUS_FIRST, 0, 0 };
        ( void ) demo_layout_render_case( utest_fixture, test_case );

        vxui_rect command_panel = {};
        vxui_rect preview_panel = {};
        ASSERT_TRUE( demo_layout_find_element_bounds( "main.command_panel", &command_panel ) );
        ASSERT_TRUE( demo_layout_find_element_bounds( "main.preview_panel", &preview_panel ) );
        EXPECT_TRUE( vxui_demo_horizontal_split_order( command_panel, preview_panel, vxui_demo_get_main_menu_contract().deck_gap * 0.5f, false ) );
    }
}

UTEST_F( demo_layout_fixture, split_deck_screens_preserve_lane_minimums_for_all_locales )
{
    const char* screens[] = { "sortie", "loadout", "archives", "records" };
    const char* locales[] = { "en", "ja-JP", "ar" };
    for ( const char* screen : screens ) {
        for ( const char* locale : locales ) {
            const demo_layout_case test_case = { screen, locale, 1280, 720, 0, DEMO_TEXT_PACK_LONG, DEMO_FOCUS_DETAIL_HEAVY, 0, 0 };
            ( void ) demo_layout_render_case( utest_fixture, test_case );

            const vxui_demo_surface_kind kind = vxui_demo_surface_kind_from_screen_id( screen );
            const vxui_demo_split_deck_contract contract = vxui_demo_get_split_deck_contract( kind );
            const char* menu_panel_id = std::strcmp( screen, "sortie" ) == 0 ? "sortie.menu_panel"
                : std::strcmp( screen, "loadout" ) == 0                    ? "loadout.menu_panel"
                : std::strcmp( screen, "archives" ) == 0                   ? "archives.menu_panel"
                                                                            : "records.menu_panel";
            const char* detail_id = std::strcmp( screen, "sortie" ) == 0 ? "sortie.briefing"
                : std::strcmp( screen, "loadout" ) == 0                  ? "loadout.detail"
                : std::strcmp( screen, "archives" ) == 0                 ? "archives.detail"
                                                                          : "records.detail";
            vxui_rect menu_panel = {};
            vxui_rect detail = {};
            ASSERT_TRUE( demo_layout_find_element_bounds( menu_panel_id, &menu_panel ) );
            ASSERT_TRUE( demo_layout_find_element_bounds( detail_id, &detail ) );
            EXPECT_GE( menu_panel.w, contract.primary_min_width );
            EXPECT_GE( detail.w, contract.secondary_min_width );
        }
    }
}

UTEST_F( demo_layout_fixture, split_deck_detail_panels_keep_visible_text_inside_panel )
{
    const char* screens[] = { "sortie", "loadout", "archives", "records" };
    for ( const char* screen : screens ) {
        const demo_layout_case test_case = { screen, "en", 1100, 720, 0, DEMO_TEXT_PACK_EXTREME, DEMO_FOCUS_DETAIL_HEAVY, 0, 0 };
        vxui_draw_list list = demo_layout_render_case( utest_fixture, test_case );

        const char* detail_id = std::strcmp( screen, "sortie" ) == 0 ? "sortie.briefing"
            : std::strcmp( screen, "loadout" ) == 0                  ? "loadout.detail"
            : std::strcmp( screen, "archives" ) == 0                 ? "archives.detail"
                                                                      : "records.detail";
        vxui_rect detail = {};
        std::string detail_body_id = std::string( detail_id ) + ".body";
        vxui_rect detail_body = {};
        ASSERT_TRUE( demo_layout_find_element_bounds( detail_id, &detail ) );
        ASSERT_TRUE( demo_layout_find_element_bounds( detail_body_id.c_str(), &detail_body ) );
        EXPECT_TRUE( vxui_demo_rect_inside( detail, detail_body, 0.0f ) );
        EXPECT_TRUE( demo_layout_split_body_text_fully_visible( utest_fixture, test_case, &list, detail_body ) );
    }
}

UTEST_F( demo_layout_fixture, settings_has_exactly_one_vertical_overflow_owner )
{
    const demo_layout_case test_case = { "settings", "en", 1280, 720, 0, DEMO_TEXT_PACK_LONG, DEMO_FOCUS_LAST, 8, 0 };
    ( void ) demo_layout_render_case( utest_fixture, test_case );

    vxui_rect body_panel = {};
    vxui_rect menu_viewport = {};
    vxui_rect menu_content = {};
    ASSERT_TRUE( demo_layout_find_element_bounds( "settings.body_panel", &body_panel ) );
    ASSERT_TRUE( demo_layout_find_hash_bounds( vxui_idi( "settings.body_menu", 1 ), &menu_viewport ) );
    ASSERT_TRUE( demo_layout_find_hash_bounds( vxui_idi( "settings.body_menu", 2 ), &menu_content ) );
    EXPECT_TRUE( vxui_demo_region_has_single_overflow_owner( body_panel, menu_viewport, menu_content ) );
}

UTEST_F( demo_layout_fixture, settings_footer_never_intersects_scrolled_content )
{
    const demo_layout_case test_case = { "settings", "ja-JP", 960, 720, 0, DEMO_TEXT_PACK_LONG, DEMO_FOCUS_LAST, 12, 0 };
    ( void ) demo_layout_render_case( utest_fixture, test_case );
    vxui_rect menu_viewport = {};
    vxui_rect footer = {};
    ASSERT_TRUE( demo_layout_find_hash_bounds( vxui_idi( "settings.body_menu", 1 ), &menu_viewport ) );
    ASSERT_TRUE( demo_layout_find_element_bounds( "settings.footer", &footer ) );
    EXPECT_LE( menu_viewport.y + menu_viewport.h, footer.y + 1.0f );
}

UTEST_F( demo_layout_fixture, credits_stack_and_footer_do_not_collide )
{
    const demo_layout_case test_case = { "credits", "en", 1280, 720, 0, DEMO_TEXT_PACK_LONG, DEMO_FOCUS_NONE, 0, 0 };
    ( void ) demo_layout_render_case( utest_fixture, test_case );
    vxui_rect stack = {};
    vxui_rect footer = {};
    ASSERT_TRUE( demo_layout_find_element_bounds( "credits.stack", &stack ) );
    ASSERT_TRUE( demo_layout_find_element_bounds( "credits.footer", &footer ) );
    EXPECT_TRUE( vxui_demo_vertical_stack_order( stack, footer, 8.0f ) );
}

UTEST_F( demo_layout_fixture, all_supported_screens_keep_surface_inside_viewport )
{
    const char* screens[] = { "boot", "title", "main_menu", "sortie", "loadout", "archives", "settings", "records", "credits", "launch_stub", "results_stub" };
    const int widths[][ 2 ] = { { 1600, 900 }, { 1280, 720 }, { 1100, 720 }, { 960, 720 } };
    for ( const char* screen : screens ) {
        for ( const auto& size : widths ) {
            const demo_layout_case test_case = { screen, "en", size[ 0 ], size[ 1 ], 0, DEMO_TEXT_PACK_NORMAL, DEMO_FOCUS_NONE, 0, 0 };
            ( void ) demo_layout_render_case( utest_fixture, test_case );
            vxui_rect root = {};
            vxui_rect surface = {};
            ASSERT_TRUE( demo_layout_find_element_bounds( screen, &root ) );
            ASSERT_TRUE( demo_layout_find_element_bounds( vxui_demo_surface_id_for_screen( screen ), &surface ) );
            EXPECT_TRUE( vxui_demo_rect_inside( root, surface, 0.0f ) );
        }
    }
}

UTEST_F( demo_layout_fixture, all_supported_screens_keep_focus_ring_inside_surface )
{
    const char* screens[] = { "main_menu", "sortie", "loadout", "archives", "settings", "records" };
    for ( const char* screen : screens ) {
        const demo_layout_case test_case = {
            screen,
            "en",
            1280,
            720,
            0,
            DEMO_TEXT_PACK_NORMAL,
            std::strcmp( screen, "main_menu" ) == 0 ? DEMO_FOCUS_FIRST : DEMO_FOCUS_LAST,
            std::strcmp( screen, "main_menu" ) == 0 ? 0 : 4,
            0
        };
        vxui_draw_list list = demo_layout_render_case( utest_fixture, test_case );
        ( void ) list;
        std::vector< std::string > warnings = demo_layout_collect_warnings( utest_fixture, &list, screen );
        bool focus_warning = false;
        for ( const std::string& warning : warnings ) {
            if ( warning.find( "focused element drifted" ) != std::string::npos ) {
                focus_warning = true;
                break;
            }
        }
        EXPECT_FALSE( focus_warning );
    }
}

UTEST_F( demo_layout_fixture, all_supported_screens_pass_runtime_contract_engine_without_warnings )
{
    const char* screens[] = { "title", "main_menu", "sortie", "loadout", "archives", "settings", "records", "credits", "launch_stub", "results_stub" };
    const char* locales[] = { "en", "ja-JP", "ar" };
    const int widths[][ 2 ] = { { 1280, 720 }, { 1100, 720 }, { 960, 720 } };
    for ( const char* screen : screens ) {
        for ( const char* locale : locales ) {
            for ( const auto& size : widths ) {
                const demo_layout_case test_case = { screen, locale, size[ 0 ], size[ 1 ], 0, DEMO_TEXT_PACK_LONG, DEMO_FOCUS_NONE, 2, 0 };
                vxui_draw_list list = demo_layout_render_case( utest_fixture, test_case );
                std::vector< std::string > warnings = demo_layout_collect_warnings( utest_fixture, &list, screen );
                warnings.erase(
                    std::remove_if(
                        warnings.begin(),
                        warnings.end(),
                        []( const std::string& warning ) { return warning == "focused element drifted outside the bounded surface."; } ),
                    warnings.end() );
                if ( !warnings.empty() ) {
                    std::fprintf( stderr, "layout warning %s %s %dx%d: %s\n", screen, locale, size[ 0 ], size[ 1 ], warnings[ 0 ].c_str() );
                }
                EXPECT_TRUE( warnings.empty() );
            }
        }
    }
}

UTEST_F( demo_layout_fixture, long_string_matrix_produces_no_visible_text_outside_surface )
{
    const char* screens[] = { "main_menu", "sortie", "loadout", "archives", "settings", "records", "credits" };
    const char* locales[] = { "en", "ja-JP", "ar" };
    for ( const char* screen : screens ) {
        for ( const char* locale : locales ) {
            const demo_layout_case test_case = { screen, locale, 960, 720, 1, DEMO_TEXT_PACK_EXTREME, DEMO_FOCUS_DETAIL_HEAVY, 8, 2 };
            vxui_draw_list list = demo_layout_render_case( utest_fixture, test_case );
            std::vector< std::string > warnings = demo_layout_collect_warnings( utest_fixture, &list, screen );
            warnings.erase(
                std::remove_if(
                    warnings.begin(),
                    warnings.end(),
                    []( const std::string& warning ) { return warning == "focused element drifted outside the bounded surface."; } ),
                warnings.end() );
            EXPECT_TRUE( warnings.empty() );

            if ( std::strcmp( screen, "main_menu" ) == 0 ) {
                vxui_rect preview_body = {};
                ASSERT_TRUE( demo_layout_find_element_bounds( "main.preview_body", &preview_body ) );
                EXPECT_TRUE( demo_layout_main_menu_preview_text_fully_visible( utest_fixture, test_case, &list, preview_body ) );
            } else if ( std::strcmp( screen, "settings" ) == 0 ) {
                continue;
            } else if ( std::strcmp( screen, "credits" ) == 0 ) {
                vxui_rect viewport = {};
                ASSERT_TRUE( demo_layout_find_element_bounds( "credits.body_viewport", &viewport ) );
                EXPECT_TRUE( vxui_demo_clipped_text_stays_inside_viewport( &list, viewport, [&]( const vxui_cmd& cmd ) {
                    return std::fabs( cmd.clip_rect.x - viewport.x ) <= 2.0f
                        && std::fabs( cmd.clip_rect.y - viewport.y ) <= 2.0f
                        && std::fabs( cmd.clip_rect.w - viewport.w ) <= 2.0f
                        && std::fabs( cmd.clip_rect.h - viewport.h ) <= 2.0f;
                } ) );
            } else {
                if ( std::strcmp( screen, "sortie" ) == 0 && test_case.width == 960 ) {
                    continue;
                }
                const char* detail_id = std::strcmp( screen, "sortie" ) == 0 ? "sortie.briefing"
                    : std::strcmp( screen, "loadout" ) == 0                  ? "loadout.detail"
                    : std::strcmp( screen, "archives" ) == 0                 ? "archives.detail"
                                                                              : "records.detail";
                std::string body_id = std::string( detail_id ) + ".body";
                vxui_rect body = {};
                ASSERT_TRUE( demo_layout_find_element_bounds( body_id.c_str(), &body ) );
                EXPECT_TRUE( demo_layout_split_body_text_fully_visible( utest_fixture, test_case, &list, body ) );
            }
        }
    }
}

UTEST_F( demo_layout_fixture, arabic_matrix_preserves_rtl_lane_order_and_containment )
{
    const char* screens[] = { "main_menu", "sortie", "loadout", "archives", "records", "settings" };
    for ( const char* screen : screens ) {
        const demo_layout_case test_case = { screen, "ar", 1280, 720, 1, DEMO_TEXT_PACK_LONG, DEMO_FOCUS_DETAIL_HEAVY, 4, 2 };
        ( void ) demo_layout_render_case( utest_fixture, test_case );

        if ( std::strcmp( screen, "main_menu" ) == 0 ) {
            vxui_rect command = {};
            vxui_rect preview = {};
            ASSERT_TRUE( demo_layout_find_element_bounds( "main.command_panel", &command ) );
            ASSERT_TRUE( demo_layout_find_element_bounds( "main.preview_panel", &preview ) );
            EXPECT_TRUE( vxui_demo_horizontal_split_order( command, preview, vxui_demo_get_main_menu_contract().deck_gap, false ) );
        } else if ( std::strcmp( screen, "settings" ) == 0 ) {
            vxui_rect body_panel = {};
            vxui_rect menu_viewport = {};
            ASSERT_TRUE( demo_layout_find_element_bounds( "settings.body_panel", &body_panel ) );
            ASSERT_TRUE( demo_layout_find_hash_bounds( vxui_idi( "settings.body_menu", 1 ), &menu_viewport ) );
            EXPECT_TRUE( vxui_demo_rect_inside( body_panel, menu_viewport, 0.0f ) );
        }
    }
}

UTEST_F( demo_layout_fixture, tight_supported_desktop_matrix_preserves_footer_and_panel_separation )
{
    const char* screens[] = { "main_menu", "sortie", "loadout", "archives", "settings", "records", "credits" };
    for ( const char* screen : screens ) {
        const demo_layout_case test_case = { screen, "en", 960, 720, 0, DEMO_TEXT_PACK_LONG, DEMO_FOCUS_LAST, 8, 0 };
        vxui_draw_list list = demo_layout_render_case( utest_fixture, test_case );
        std::vector< std::string > warnings = demo_layout_collect_warnings( utest_fixture, &list, screen );
        warnings.erase(
            std::remove_if(
                warnings.begin(),
                warnings.end(),
                []( const std::string& warning ) { return warning == "focused element drifted outside the bounded surface."; } ),
            warnings.end() );
        EXPECT_TRUE( warnings.empty() );
    }
}

UTEST_F( demo_layout_fixture, generated_content_stress_never_creates_unowned_overflow )
{
    const char* screens[] = { "main_menu", "settings", "sortie", "archives", "records" };
    const char* locales[] = { "en", "ja-JP", "ar" };
    for ( int pack = DEMO_TEXT_PACK_NORMAL; pack <= DEMO_TEXT_PACK_EXTREME; ++pack ) {
        for ( int extra_rows = 0; extra_rows <= 12; extra_rows += 4 ) {
            for ( const char* screen : screens ) {
                for ( const char* locale : locales ) {
                    const demo_layout_case test_case = {
                        screen,
                        locale,
                        1100,
                        720,
                        extra_rows % 2,
                        ( demo_layout_text_pack ) pack,
                        DEMO_FOCUS_DETAIL_HEAVY,
                        extra_rows,
                        extra_rows % 3,
                    };
                    vxui_draw_list list = demo_layout_render_case( utest_fixture, test_case );
                    std::vector< std::string > warnings = demo_layout_collect_warnings( utest_fixture, &list, screen );
                    warnings.erase(
                        std::remove_if(
                            warnings.begin(),
                            warnings.end(),
                            []( const std::string& warning ) { return warning == "focused element drifted outside the bounded surface."; } ),
                        warnings.end() );
                    EXPECT_TRUE( warnings.empty() );
                }
            }
        }
    }
}
