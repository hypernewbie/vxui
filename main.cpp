#include <array>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <filesystem>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <objbase.h>
#include <wincodec.h>

#include "demo/internal/shot.h"
#include "demo/internal/layout_contract.h"
#include <glad/glad.h>
#include "TinyWindow.h"

#ifdef near
#undef near
#endif
#ifdef far
#undef far
#endif

#if defined( VE_FONTCACHE_FREETYPE_RASTERISATION )
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#endif

#if defined( VE_FONTCACHE_HARFBUZZ )
#include <hb.h>
#endif

#define CLAY_IMPLEMENTATION
#include "clay/clay.h"

#define VE_FONTCACHE_IMPL
#include "vefc/ve_fontcache.h"

#define VXUI_IMPL
#include "vxui.h"

#define VXUI_MENU_IMPL
#include "vxui_menu.h"

#include "demo/internal/layout_validation.h"
#include "demo/internal/main_menu_shared.h"
#include "demo/internal/split_deck_shared.h"
#include "demo/internal/theme.h"

#include "vefc/ve_fontcache_backend_test.h"

#ifndef VXUI_SOURCE_DIR
#define VXUI_SOURCE_DIR "."
#endif

enum vxui_demo_font_id
{
    VXUI_DEMO_FONT_UI = 0,
    VXUI_DEMO_FONT_TITLE,
    VXUI_DEMO_FONT_DEBUG,
    VXUI_DEMO_FONT_JAPANESE,
    VXUI_DEMO_FONT_ARABIC,
    VXUI_DEMO_FONT_SECTION_TITLE,
    VXUI_DEMO_FONT_JAPANESE_TITLE,
    VXUI_DEMO_FONT_JAPANESE_SECTION,
    VXUI_DEMO_FONT_ARABIC_TITLE,
    VXUI_DEMO_FONT_ARABIC_SECTION,
    VXUI_DEMO_FONT_FACE_COUNT,
};

enum vxui_demo_font_role
{
    VXUI_DEMO_FONT_ROLE_BODY = 100,
    VXUI_DEMO_FONT_ROLE_TITLE,
    VXUI_DEMO_FONT_ROLE_SECTION,
    VXUI_DEMO_FONT_ROLE_CODE,
};

enum vxui_demo_locale
{
    VXUI_DEMO_LOCALE_ENGLISH = 0,
    VXUI_DEMO_LOCALE_JAPANESE,
    VXUI_DEMO_LOCALE_ARABIC,
};

enum vxui_demo_screen_kind
{
    VXUI_DEMO_SCREEN_BOOT = 0,
    VXUI_DEMO_SCREEN_TITLE,
    VXUI_DEMO_SCREEN_MAIN_MENU,
    VXUI_DEMO_SCREEN_SORTIE,
    VXUI_DEMO_SCREEN_LOADOUT,
    VXUI_DEMO_SCREEN_ARCHIVES,
    VXUI_DEMO_SCREEN_SETTINGS,
    VXUI_DEMO_SCREEN_RECORDS,
    VXUI_DEMO_SCREEN_CREDITS,
    VXUI_DEMO_SCREEN_LAUNCH_STUB,
    VXUI_DEMO_SCREEN_RESULTS_STUB,
    VXUI_DEMO_SCREEN_UNKNOWN,
};

enum
{
    VXUI_DEMO_BODY_SIZE = 24,
    VXUI_DEMO_TITLE_SIZE = 44,
    VXUI_DEMO_SECTION_SIZE = 28,
    VXUI_DEMO_CODE_SIZE = 16,
};

static constexpr uint16_t VXUI_DEMO_BUTTON_PADDING_X = 10;
static constexpr uint16_t VXUI_DEMO_BUTTON_PADDING_Y = 4;
static constexpr uint16_t VXUI_DEMO_INLINE_GAP = ( uint16_t ) VXUI_DEMO_LAYOUT_INLINE_GAP;
static constexpr uint16_t VXUI_DEMO_ROW_GAP = ( uint16_t ) VXUI_DEMO_LAYOUT_ROW_GAP;
static constexpr uint16_t VXUI_DEMO_SCREEN_GAP = ( uint16_t ) VXUI_DEMO_LAYOUT_SECTION_GAP;
static constexpr uint16_t VXUI_DEMO_OUTER_PADDING = ( uint16_t ) VXUI_DEMO_LAYOUT_OUTER_PADDING;
static constexpr uint16_t VXUI_DEMO_SURFACE_PADDING_X = ( uint16_t ) VXUI_DEMO_LAYOUT_SURFACE_PADDING_X;
static constexpr uint16_t VXUI_DEMO_SURFACE_PADDING_Y = ( uint16_t ) VXUI_DEMO_LAYOUT_SURFACE_PADDING_Y;

struct vxui_demo_font_metrics
{
    ve_font_id font_id;
    float line_height;
};

enum vxui_demo_button
{
    VXUI_DEMO_BTN_NAV_UP,
    VXUI_DEMO_BTN_NAV_DOWN,
    VXUI_DEMO_BTN_NAV_LEFT,
    VXUI_DEMO_BTN_NAV_RIGHT,
    VXUI_DEMO_BTN_CONFIRM,
    VXUI_DEMO_BTN_CANCEL,
    VXUI_DEMO_BTN_TAB_LEFT,
    VXUI_DEMO_BTN_TAB_RIGHT,
    VXUI_DEMO_BTN_LOCALE_EN,
    VXUI_DEMO_BTN_LOCALE_JA,
    VXUI_DEMO_BTN_LOCALE_AR,
    VXUI_DEMO_BTN_PROMPT_KEYBOARD,
    VXUI_DEMO_BTN_PROMPT_GAMEPAD,
#ifdef VXUI_DEBUG
    VXUI_DEMO_BTN_DEBUG_TOGGLE,
    VXUI_DEMO_BTN_DEBUG_RELOAD,
    VXUI_DEMO_BTN_DEBUG_COPY,
#endif
    VXUI_DEMO_BTN_COUNT,
};

typedef struct vxui_demo_app
{
    bool show_settings;
    int difficulty;
    float volume;
    float settings_body_scroll_current;
    float settings_body_scroll_target;
    float settings_body_scroll_velocity;
    int locale_index;
    int prompt_table_index;
    int selected_mission_index;
    int selected_ship_index;
    int selected_primary_index;
    int selected_support_index;
    int selected_system_index;
    int archive_category_index;
    int archive_entry_index;
    int scanline_index;
    int effect_intensity_index;
    int records_board_index;
    int records_entry_index;
    float boot_timer;
    float title_timer;
    float launch_timer;
    float results_timer;
    float shot_layout_surface_max_height_override;
    bool request_quit;
    bool shot_mode;
    bool shot_force_compact_layout;
    TinyWindow::tWindow* window;
    vxui_input_table keyboard_table;
    vxui_input_table gamepad_table;
    vxui_menu_state main_menu_state;
    vxui_menu_state sortie_menu_state;
    vxui_menu_state loadout_menu_state;
    vxui_menu_state archives_menu_state;
    vxui_menu_state settings_menu_state;
    vxui_menu_state records_menu_state;
    vxui_menu_state credits_menu_state;
    vxui_ctx* ctx;
    std::string watched_seq_path;
    bool copied_to_clipboard;
    int debug_prop_index;
    float debug_delay_value;
    float debug_target_value;
    int last_selected_seq;
    bool button_prev[ VXUI_DEMO_BTN_COUNT ];
    char preview_status[ 96 ];
    char clipboard_status[ 96 ];
    char reload_status[ 256 ];
} vxui_demo_app;

typedef struct vxui_demo_pulse
{
    float speed;
    float scale;
    float alpha;
} vxui_demo_pulse;

typedef struct vxui_demo_glow
{
    float padding;
    float alpha;
} vxui_demo_glow;

typedef struct vxui_demo_scanline
{
    float spacing;
    float alpha;
} vxui_demo_scanline;

typedef struct vxui_demo_spin
{
    float speed;
    float padding;
} vxui_demo_spin;

typedef struct vxui_demo_renderer
{
    GLuint primitive_program;
    GLuint fontcache_shader_render_glyph;
    GLuint fontcache_shader_blit_atlas;
    GLuint fontcache_shader_draw_text;
    GLuint vao;
    GLuint text_vbo;
    GLuint text_ibo;
    size_t text_vbo_capacity;
    size_t text_ibo_capacity;
    GLuint fontcache_fbo[ 2 ];
    GLuint fontcache_fbo_texture[ 2 ];
    std::vector< GLuint > cpu_atlas_textures;
    TinyWindow::vec2_t< unsigned int > window_size;
    TinyWindow::vec2_t< unsigned int > snap_configured_window_size {};
    bool snap_configured = false;
    ve_fontcache cache;
    std::array< std::vector< uint8_t >, VXUI_DEMO_FONT_FACE_COUNT > demo_fonts;
    std::array< float, VXUI_DEMO_FONT_FACE_COUNT > demo_line_heights {};
    std::vector< std::vector< uint8_t > > backend_test_dynamic_font_buffers;
    ve_font_id backend_test_primary_font = -1;
    ve_font_id backend_test_secondary_font = -1;
    ve_font_id backend_test_small_font = -1;
    ve_font_id backend_test_latin_font = -1;
    ve_font_id backend_test_cjk_font = -1;
    ve_font_id backend_test_huge_font = -1;
    bool backend_test_mode;
    GLuint backend_target_fbo;
    GLuint backend_target_texture;
    GLuint backend_present_fbo;
    GLuint backend_present_texture;
    bool shot_capture_mode;
    GLuint shot_capture_fbo;
    GLuint shot_capture_texture;
    struct vxui_demo_uniform_cache
    {
        struct primitive_uniforms
        {
            GLint bounds = -1;
            GLint viewport = -1;
            GLint size = -1;
            GLint color = -1;
            GLint radius = -1;
            GLint border = -1;
        } primitive;
        struct fontcache_blit_atlas_uniforms
        {
            GLint src_texture = -1;
            GLint region = -1;
        } blit_atlas;
        struct fontcache_draw_text_uniforms
        {
            GLint src_texture = -1;
            GLint downsample = -1;
            GLint colour = -1;
        } draw_text;
    } uniforms;
    struct vxui_demo_gl_debug
    {
        enum backend
        {
            backend_none,
            backend_khr,
            backend_ext,
        } backend;
        bool debug_context;
        bool callback_installed;
        typedef void ( APIENTRYP fn_PushDebugGroup )( GLenum source, GLuint id, GLsizei length, const GLchar* message );
        typedef void ( APIENTRYP fn_PopDebugGroup )( void );
        typedef void ( APIENTRYP fn_ObjectLabel )( GLenum identifier, GLuint name, GLsizei length, const GLchar* label );
        typedef void ( APIENTRYP fn_DebugMessageCallback )( GLDEBUGPROC callback, const void* userParam );
        typedef void ( APIENTRYP fn_DebugMessageControl )( GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled );
        typedef void ( APIENTRYP fn_PushGroupMarkerEXT )( GLsizei length, const GLchar* marker );
        typedef void ( APIENTRYP fn_PopGroupMarkerEXT )( void );
        typedef void ( APIENTRYP fn_InsertEventMarkerEXT )( GLsizei length, const GLchar* marker );
        fn_PushDebugGroup glPushDebugGroup;
        fn_PopDebugGroup glPopDebugGroup;
        fn_ObjectLabel glObjectLabel;
        fn_DebugMessageCallback glDebugMessageCallback;
        fn_DebugMessageControl glDebugMessageControl;
        fn_PushGroupMarkerEXT glPushGroupMarkerEXT;
        fn_PopGroupMarkerEXT glPopGroupMarkerEXT;
        fn_InsertEventMarkerEXT glInsertEventMarkerEXT;
    } gl_debug;
} vxui_demo_renderer;

struct vxui_demo_decl_scope
{
    vxui_ctx* ctx;
    uint32_t previous_decl_id;

    vxui_demo_decl_scope( vxui_ctx* in_ctx, uint32_t decl_id )
        : ctx( in_ctx )
        , previous_decl_id( in_ctx ? in_ctx->current_decl_id : 0u )
    {
        if ( ctx ) {
            ctx->current_decl_id = decl_id;
        }
    }

    ~vxui_demo_decl_scope()
    {
        if ( ctx ) {
            ctx->current_decl_id = previous_decl_id;
        }
    }
};

// Global pointer to renderer for VEFC backend test callbacks (set during --vefc-backend-test)
static vxui_demo_renderer* g_vxui_demo_backend_test_renderer = nullptr;

static const char* vxui_demo_text( const char* key, void* userdata );
static FILE* vxui_demo_fopen( const char* path, const char* mode );
static bool vxui_demo_write_file( const char* path, const char* text );
static std::string vxui_demo_make_temp_path( const char* filename );
static std::u8string vxui_demo_to_u8( const char* text );
static bool vxui_demo_copy_to_clipboard( const char* text );
static int vxui_demo_find_seq_index( const vxui_ctx* ctx, const char* name );
static void vxui_demo_sync_step_editor( vxui_demo_app* app );
static void vxui_demo_apply_step_editor( vxui_demo_app* app );
static void vxui_demo_refresh_status( vxui_demo_app* app );
static const char* vxui_demo_locale_code( int locale_index );
static const char* vxui_demo_locale_name_key( int locale_index );
static const char* vxui_demo_prompt_name_key( int prompt_table_index );
static const char* vxui_demo_footer_locale_key( int locale_index );
static const char* vxui_demo_footer_prompt_key( int prompt_table_index );
static const char* vxui_demo_screen_name_key( const char* screen_name );
static const char* vxui_demo_footer_top_name( const vxui_demo_app* app, const vxui_ctx* ctx );
static bool vxui_demo_locale_matches( const char* locale, const char* prefix );
static vxui_demo_font_metrics vxui_demo_resolve_font_metrics( const vxui_demo_renderer* renderer, uint32_t requested_font_id, float requested_font_size, const char* locale );
static void vxui_demo_font_resolver( vxui_ctx* ctx, uint32_t requested_font_id, float requested_font_size, const char* locale, void* userdata, vxui_resolved_font* out );
static float vxui_demo_control_height( const vxui_demo_renderer* renderer, const char* locale );
static void vxui_demo_emit_save_slot_row( vxui_ctx* ctx, const char* id, const char* label_key, float width, float height, bool rtl );
static void vxui_demo_emit_action_button( vxui_ctx* ctx, const char* id, const char* l10n_key, vxui_action_fn fn, vxui_action_cfg cfg, float control_height );
static bool vxui_demo_get_anim_bounds( const vxui_ctx* ctx, uint32_t id, vxui_rect* out );
static void vxui_demo_step_settings_body_scroll( vxui_demo_app* app, float dt );
static void vxui_demo_sync_settings_body_scroll( vxui_demo_app* app, const vxui_ctx* ctx );
#ifdef VXUI_DEBUG
static void vxui_demo_debug_validate_demo_layout( vxui_ctx* ctx, const vxui_draw_list* list );
#endif
static void vxui_demo_open_title( vxui_ctx* ctx, uint32_t id, void* userdata );
static void vxui_demo_open_main_menu( vxui_ctx* ctx, uint32_t id, void* userdata );
static void vxui_demo_open_sortie( vxui_ctx* ctx, uint32_t id, void* userdata );
static void vxui_demo_open_loadout( vxui_ctx* ctx, uint32_t id, void* userdata );
static void vxui_demo_open_archives( vxui_ctx* ctx, uint32_t id, void* userdata );
static void vxui_demo_open_settings( vxui_ctx* ctx, uint32_t id, void* userdata );
static void vxui_demo_open_records( vxui_ctx* ctx, uint32_t id, void* userdata );
static void vxui_demo_open_credits( vxui_ctx* ctx, uint32_t id, void* userdata );
static void vxui_demo_launch_stub( vxui_ctx* ctx, uint32_t id, void* userdata );
static void vxui_demo_close_settings( vxui_ctx* ctx, uint32_t id, void* userdata );
static void vxui_demo_open_results_stub( vxui_ctx* ctx, uint32_t id, void* userdata );
static void vxui_demo_request_quit( vxui_ctx* ctx, uint32_t id, void* userdata );
static void vxui_demo_restore_defaults_action( vxui_ctx* ctx, uint32_t id, void* userdata );
static void vxui_demo_locale_option_changed( vxui_ctx* ctx, int value, void* userdata );
static void vxui_demo_prompt_table_changed( vxui_ctx* ctx, int value, void* userdata );
static void vxui_demo_render_frontend( vxui_demo_app* app, vxui_demo_renderer* renderer, vxui_ctx* ctx );
static void vxui_demo_handle_auto_transitions( vxui_demo_app* app, vxui_ctx* ctx, float dt, bool confirm_pressed );
static void vxui_demo_handle_cancel_navigation( vxui_demo_app* app, vxui_ctx* ctx );
static GLuint vxui_demo_compile_program( const char* vs_source, const char* fs_source, const char* label );
static void vxui_demo_gl_debug_init( vxui_demo_renderer* renderer );
static void vxui_demo_gl_debug_label( vxui_demo_renderer* renderer, GLenum identifier, GLuint name, const char* label );
static void vxui_demo_gl_debug_begin( vxui_demo_renderer* renderer, const char* label );
static void vxui_demo_gl_debug_end( vxui_demo_renderer* renderer );
static void vxui_demo_gl_debug_event( vxui_demo_renderer* renderer, const char* label );
static void vxui_demo_set_window_size( vxui_demo_renderer* renderer, TinyWindow::vec2_t< unsigned int > window_size );
static void vxui_demo_configure_snap_if_needed( vxui_demo_renderer* renderer, bool force );
static bool vxui_demo_cache_shader_uniforms( vxui_demo_renderer* renderer );
static bool vxui_demo_init_renderer( vxui_demo_renderer* renderer );
static void vxui_demo_shutdown_renderer( vxui_demo_renderer* renderer );
static bool vxui_demo_load_fonts( vxui_demo_renderer* renderer );
static void vxui_demo_render_draw_list( vxui_demo_renderer* renderer, const vxui_draw_list* list );
static void vxui_demo_draw_primitive( vxui_demo_renderer* renderer, vxui_rect bounds, vxui_color color, float radius, float border_width );
static void vxui_demo_draw_placeholder_image( vxui_demo_renderer* renderer, vxui_rect bounds );
static void vxui_demo_render_text_cmd( vxui_demo_renderer* renderer, const vxui_draw_cmd_text* text );
static void vxui_demo_render_fontcache_drawlist( vxui_demo_renderer* renderer, const vxui_rect* clip_rect );
static void vxui_demo_ensure_fontcache_upload_buffers( vxui_demo_renderer* renderer, size_t vertex_bytes, size_t index_bytes );
static void vxui_demo_setup_fontcache_fbo( vxui_demo_renderer* renderer );
static void vxui_demo_flip_fontcache_upload( ve_fontcache_drawlist* drawlist, ve_fontcache_draw& dcall );
static void vxui_demo_clear_backend_test_surfaces( vxui_demo_renderer* renderer, bool clear_cpu_atlas_pages );
static bool vxui_demo_setup_backend_test_fbo( vxui_demo_renderer* renderer );
static void vxui_demo_shutdown_backend_test_fbo( vxui_demo_renderer* renderer );
static bool vxui_demo_setup_shot_capture_fbo( vxui_demo_renderer* renderer );
static void vxui_demo_shutdown_shot_capture_fbo( vxui_demo_renderer* renderer );
static bool vxui_demo_readback_r8_texture( GLuint texture, int x, int y, int w, int h, uint8_t* out_pixels );
static bool vxui_demo_capture_rgba_texture_png( GLuint texture, int width, int height, const std::filesystem::path& path, char* error, size_t error_size );
static bool vxui_demo_write_png_rgba( const std::filesystem::path& path, int width, int height, const uint8_t* pixels, char* error, size_t error_size );
static uint32_t vxui_demo_focus_id_for_screen( vxui_demo_screen_kind screen_kind, const char* focus_name );
static bool vxui_demo_apply_shot_request( vxui_demo_app* app, vxui_ctx* ctx, const vxui_demo_shot_request& request, char* error, size_t error_size );
static void vxui_demo_present_draw_list( vxui_demo_renderer* renderer, vxui_ctx* ctx, const vxui_draw_list* list );
static bool vxui_demo_button_edge( vxui_demo_app* app, vxui_demo_button button, bool down );
static bool vxui_demo_key_down( const TinyWindow::tWindow* window, int key );
static bool vxui_demo_char_down( const TinyWindow::tWindow* window, char ch );
static const TinyWindow::gamepad_t* vxui_demo_primary_gamepad( TinyWindow::windowManager* manager );
static bool vxui_demo_gamepad_down( const TinyWindow::gamepad_t* gamepad, int button );

struct vxui_demo_localized_text
{
    const char* key;
    const char* en;
    const char* ja;
    const char* ar;
};

struct vxui_demo_mission
{
    const char* name;
    const char* region;
    const char* briefing;
    const char* threat;
    const char* reward;
    const char* warning;
};

struct vxui_demo_ship
{
    const char* name;
    const char* class_name;
    const char* summary;
    float speed;
    float shield;
    float output;
    float sync;
};

struct vxui_demo_archive_entry
{
    const char* title;
    const char* subtitle;
    const char* detail;
    const char* meta;
    bool unlocked;
};

struct vxui_demo_record
{
    const char* run_name;
    const char* pilot_name;
    const char* ship_name;
    const char* stage_name;
    const char* score_text;
    const char* clear_text;
    const char* note;
};

struct vxui_demo_credit_entry
{
    const char* role;
    const char* name;
    const char* detail;
};

static const vxui_demo_localized_text VXUI_DEMO_COMMAND_DECK_TEXT[] = {
    { "menu.main", "Command Deck", "コマンドデッキ", "سطح القيادة" },
    { "menu.sortie", "Sortie", "出撃", "الطلعة" },
    { "menu.loadout", "Loadout", "ロードアウト", "العتاد" },
    { "menu.archives", "Archives", "アーカイブ", "الأرشيف" },
    { "menu.records", "Records", "記録", "السجلات" },
    { "menu.credits", "Credits", "クレジット", "الاعتمادات" },
    { "menu.quit_demo", "Quit Demo", "デモ終了", "إنهاء العرض" },
    { "menu.start_sortie", "Start Sortie", "出撃開始", "ابدأ الطلعة" },
    { "menu.return_command_deck", "Return to Command Deck", "デッキへ戻る", "العودة إلى سطح القيادة" },
    { "menu.restore_defaults", "Restore Defaults", "初期化", "استعادة الافتراضي" },
    { "menu.interface", "Interface", "インターフェース", "الواجهة" },
    { "menu.audio", "Audio", "オーディオ", "الصوت" },
    { "menu.visual_fx", "Visual FX", "視覚効果", "المؤثرات" },
    { "menu.input_prompts", "Input Prompts", "入力プロンプト", "رموز الإدخال" },
    { "menu.challenge", "Challenge Mode", "チャレンジモード", "طور التحدي" },
    { "menu.scanlines", "Scanlines", "走査線", "خطوط المسح" },
    { "menu.effect_intensity", "Effect Intensity", "効果強度", "شدة التأثير" },
    { "menu.board", "Board", "ボード", "اللوحة" },
    { "menu.category", "Category", "カテゴリ", "الفئة" },
    { "option.off", "Off", "オフ", "إيقاف" },
    { "option.on", "On", "オン", "تشغيل" },
    { "effect.low", "Low", "低", "منخفض" },
    { "effect.medium", "Medium", "中", "متوسط" },
    { "effect.high", "High", "高", "عال" },
    { "screen.boot", "Boot", "起動", "الإقلاع" },
    { "screen.title", "Title", "タイトル", "العنوان" },
    { "screen.sortie", "Sortie", "出撃", "الطلعة" },
    { "screen.loadout", "Loadout", "ロードアウト", "العتاد" },
    { "screen.archives", "Archives", "アーカイブ", "الأرشيف" },
    { "screen.records", "Records", "記録", "السجلات" },
    { "screen.credits", "Credits", "クレジット", "الاعتمادات" },
    { "screen.launch_stub", "Launch", "発進", "الإطلاق" },
    { "screen.results_stub", "Debrief", "戦果報告", "التقرير" },
    { "boot.line.0", "VXUI Tactical Interface Uplink", "VXUI 戦術インターフェース接続", "ربط واجهة VXUI التكتيكية" },
    { "boot.line.1", "Command Deck bus synchronizing", "コマンドデッキバス同期中", "مزامنة ناقل سطح القيادة" },
    { "boot.line.2", "Press confirm to skip handshake", "決定でハンドシェイクをスキップ", "اضغط تأكيد لتجاوز المصافحة" },
    { "title.subtitle", "Playable stub front-end showcase", "プレイ可能なスタブフロントエンド", "عرض لواجهة لعب تجريبية" },
    { "title.prompt", "Press confirm to enter the deck", "決定でデッキへ", "اضغط تأكيد للدخول إلى السطح" },
    { "title.status", "Operational Theater / Simulation Build", "作戦域 / シミュレーションビルド", "مسرح العمليات / بناء محاكاة" },
    { "main.preview", "Focused Sector", "注目セクター", "القطاع المحدد" },
    { "main.status_banner", "Production-shape front-end stub", "本番形状フロントエンドスタブ", "واجهة تجريبية بشكل إنتاجي" },
    { "sortie.section.operations", "Operations", "作戦一覧", "العمليات" },
    { "sortie.section.setup", "Launch Setup", "出撃設定", "إعداد الإطلاق" },
    { "loadout.section.ships", "Frames", "機体", "الهياكل" },
    { "loadout.section.weapons", "Systems", "装備", "الأنظمة" },
    { "archives.section.entries", "Entries", "項目", "العناصر" },
    { "records.section.runs", "Runs", "ラン", "الجولات" },
    { "credits.section.stack", "Stack", "スタック", "المكدس" },
    { "launch.prompt", "Proceed to debrief", "戦果報告へ", "إلى التقرير" },
    { "results.prompt", "Confirm to return", "決定で戻る", "تأكيد للعودة" },
    { "badge.selected", "Sel", "選択中", "محدد" },
    { "badge.locked", "Locked", "ロック", "مغلق" },
    { "badge.demo", "Demo", "デモ", "عرض" },
    { "badge.recommended", "Rec", "推奨", "موصى به" },
};

static const vxui_demo_mission VXUI_DEMO_MISSIONS[] = {
    { "OP-01 Aegis Break", "Stratos Belt", "Breach the carrier screen, cut a corridor through interceptor rings, and keep the convoy beacon alive long enough to escape.", "Threat Tier 02 / Interceptor bloom", "Reward: Burst rail prototype", "Warning: Dense crossfire near the aft gate." },
    { "OP-02 Silent Meridian", "Aster Reach", "Thread a low-visibility debris lane and destroy the relay pair feeding hostile target locks into the sector grid.", "Threat Tier 03 / Sensor fog", "Reward: Shield lattice shard", "Warning: Heavy drift and delayed visual reads." },
    { "OP-03 Pale Crown", "Helios Verge", "Crack the defense lattice around the crown platform and extract the command archive before the perimeter resets.", "Threat Tier 04 / Shield bloom", "Reward: Archive clearance key", "Warning: Tight windows on the centerline." },
    { "OP-04 Last Metric", "Greywater Rim", "Stage a final stress sortie against an unstable fleet pocket built to show late-loop records and results states.", "Threat Tier 05 / Attrition field", "Reward: Demo clear marker", "Warning: Sustained pressure from every lane." },
};

static const vxui_demo_ship VXUI_DEMO_SHIPS[] = {
    { "VF-17 Kestrel", "Balanced Interceptor", "Fast response frame with forgiving shields and an easy rhythm for menu testing.", 0.72f, 0.66f, 0.58f, 0.70f },
    { "VX-4 Halberd", "Heavy Breaker", "Slower but decisive burst output with broad lock-on windows and strong shield retention.", 0.42f, 0.88f, 0.86f, 0.51f },
    { "AR-9 Mistral", "Sweep Specialist", "High-mobility frame built around lane control, support uptime, and aggressive repositioning.", 0.91f, 0.46f, 0.62f, 0.80f },
    { "NX-0 Aurora", "Prototype Lattice", "Experimental chassis with volatile output curves and elevated system sync for flashy demo bars.", 0.65f, 0.55f, 0.95f, 0.93f },
};

static const vxui_demo_archive_entry VXUI_DEMO_ARCHIVE_INTEL[] = {
    { "Intercept Packet 04", "Fleet route fragment", "Recovered fleet routing packet with enough integrity left to expose the next relay corridor and justify a second sortie pass.", "Status: decrypted", true },
    { "Signal Burn Log", "Telemetry noise capture", "A dense telemetry scrape used to justify the scanline toggle, warning accents, and command-deck radar vocabulary.", "Status: cleaned for archive", true },
};

static const vxui_demo_archive_entry VXUI_DEMO_ARCHIVE_FRAMES[] = {
    { "Halberd Field Notes", "Maintenance digest", "Crew notes on thermal pacing, burst discipline, and why the heavy frame deserves the recommended badge in demo copy.", "Status: available", true },
    { "Aurora Lattice Memo", "Prototype clearance", "Experimental notes remain partially hidden to show locked-state visuals and mixed unlocked/locked archive lists.", "Status: restricted", false },
};

static const vxui_demo_archive_entry VXUI_DEMO_ARCHIVE_SIGNALS[] = {
    { "Wake Sweep 7C", "Deep-space beacon trace", "A quiet signal layer used as the fiction excuse for the boot handshake and title ambience treatment.", "Status: tagged for sortie", true },
    { "Null Carrier Echo", "Redacted relay shard", "Still locked in the stub build, but visible so the archives screen demonstrates disabled rows without hiding content.", "Status: locked", false },
};

static const vxui_demo_record VXUI_DEMO_RECORDS[] = {
    { "Run 01 / Kestrel", "Iris Vale", "VF-17 Kestrel", "Aegis Break", "3,284,120", "1CC / Normal", "Balanced clear used as the reference board row." },
    { "Run 02 / Halberd", "Rho Mercer", "VX-4 Halberd", "Silent Meridian", "4,012,770", "No-miss / Hard", "Heavy burst route with slower movement but higher relay damage." },
    { "Run 03 / Mistral", "Naomi Crest", "AR-9 Mistral", "Pale Crown", "3,776,980", "Normal / Fast clear", "Mobility route showcasing quicker stage times and lower shield padding." },
    { "Run 04 / Aurora", "Seta Noor", "NX-0 Aurora", "Last Metric", "4,488,400", "Prototype / Demo clear", "High-sync prototype route built to justify the command-deck warning colors." },
};

static const vxui_demo_credit_entry VXUI_DEMO_CREDITS[] = {
    { "Runtime", "VXUI", "Single-header game UI runtime for focus, motion, prompts, and screen flow." },
    { "Layout", "Clay", "Immediate-authored retained layout system used under the VXUI bridge." },
    { "Typography", "VEFontCache", "Signed-distance and raster-backed text path for the demo renderer." },
    { "Demo Stack", "TinyWindow + OpenGL + GLAD", "Windows demo shell and renderer scaffolding for the command deck sample." },
};

static const char* const VXUI_DEMO_DIFFICULTY_KEYS[] = { "difficulty.easy", "difficulty.normal", "difficulty.hard" };
static const char* const VXUI_DEMO_BOOL_KEYS[] = { "option.off", "option.on" };
static const char* const VXUI_DEMO_EFFECT_KEYS[] = { "effect.low", "effect.medium", "effect.high" };
static const char* const VXUI_DEMO_MISSION_NAMES[] = { "OP-01 Aegis Break", "OP-02 Silent Meridian", "OP-03 Pale Crown", "OP-04 Last Metric" };
static const char* const VXUI_DEMO_SHIP_NAMES[] = { "VF-17 Kestrel", "VX-4 Halberd", "AR-9 Mistral", "NX-0 Aurora" };
static const char* const VXUI_DEMO_PRIMARY_NAMES[] = { "Burst Rail", "Spread Lance", "Pulse Needler", "Vector Beam" };
static const char* const VXUI_DEMO_SUPPORT_NAMES[] = { "Shield Drone", "Mine Curtain", "Target Decoy", "Repair Pulse" };
static const char* const VXUI_DEMO_SYSTEM_NAMES[] = { "Afterburn Sync", "Mag Shield", "Trace Assist", "Risk Core" };
static const char* const VXUI_DEMO_ARCHIVE_CATEGORY_NAMES[] = { "Intel Bank", "Frame Logs", "Signal Vault" };
static const char* const VXUI_DEMO_ARCHIVE_INTEL_NAMES[] = { "Intercept Packet 04", "Signal Burn Log" };
static const char* const VXUI_DEMO_ARCHIVE_FRAME_NAMES[] = { "Halberd Field Notes", "Aurora Lattice Memo" };
static const char* const VXUI_DEMO_ARCHIVE_SIGNAL_NAMES[] = { "Wake Sweep 7C", "Null Carrier Echo" };
static const char* const VXUI_DEMO_RECORD_BOARD_NAMES[] = { "Global Board", "Ship Board", "Prototype Board" };
static const char* const VXUI_DEMO_RECORD_NAMES[] = { "Run 01 / Kestrel", "Run 02 / Halberd", "Run 03 / Mistral", "Run 04 / Aurora" };
static const char* const VXUI_DEMO_PROMPT_TABLE_KEYS[] = { "prompt.name.keyboard", "prompt.name.gamepad" };
static const char* const VXUI_DEMO_LOCALE_OPTION_KEYS[] = { "locale.name.en", "locale.name.ja", "locale.name.ar" };

static const char* vxui_demo_lookup_command_deck_text( int locale_index, const char* key )
{
    if ( !key ) {
        return nullptr;
    }

    for ( const vxui_demo_localized_text& entry : VXUI_DEMO_COMMAND_DECK_TEXT ) {
        if ( std::strcmp( entry.key, key ) != 0 ) {
            continue;
        }
        switch ( locale_index ) {
            case VXUI_DEMO_LOCALE_JAPANESE:
                return entry.ja;
            case VXUI_DEMO_LOCALE_ARABIC:
                return entry.ar;
            default:
                return entry.en;
        }
    }

    return nullptr;
}

static vxui_demo_screen_kind vxui_demo_screen_kind_from_name( const char* screen_name )
{
    if ( !screen_name ) return VXUI_DEMO_SCREEN_UNKNOWN;
    if ( std::strcmp( screen_name, "boot" ) == 0 ) return VXUI_DEMO_SCREEN_BOOT;
    if ( std::strcmp( screen_name, "title" ) == 0 ) return VXUI_DEMO_SCREEN_TITLE;
    if ( std::strcmp( screen_name, "main_menu" ) == 0 ) return VXUI_DEMO_SCREEN_MAIN_MENU;
    if ( std::strcmp( screen_name, "sortie" ) == 0 ) return VXUI_DEMO_SCREEN_SORTIE;
    if ( std::strcmp( screen_name, "loadout" ) == 0 ) return VXUI_DEMO_SCREEN_LOADOUT;
    if ( std::strcmp( screen_name, "archives" ) == 0 ) return VXUI_DEMO_SCREEN_ARCHIVES;
    if ( std::strcmp( screen_name, "settings" ) == 0 ) return VXUI_DEMO_SCREEN_SETTINGS;
    if ( std::strcmp( screen_name, "records" ) == 0 ) return VXUI_DEMO_SCREEN_RECORDS;
    if ( std::strcmp( screen_name, "credits" ) == 0 ) return VXUI_DEMO_SCREEN_CREDITS;
    if ( std::strcmp( screen_name, "launch_stub" ) == 0 ) return VXUI_DEMO_SCREEN_LAUNCH_STUB;
    if ( std::strcmp( screen_name, "results_stub" ) == 0 ) return VXUI_DEMO_SCREEN_RESULTS_STUB;
    return VXUI_DEMO_SCREEN_UNKNOWN;
}

static const char* vxui_demo_top_screen_name( const vxui_ctx* ctx )
{
    if ( !ctx || ctx->screen_count <= 0 ) {
        return nullptr;
    }
    return ctx->screens[ ctx->screen_count - 1 ].name;
}

static void vxui_demo_apply_locale_index( vxui_ctx* ctx, vxui_demo_app* app, int locale_index )
{
    if ( !ctx || !app ) {
        return;
    }
    app->locale_index = std::clamp( locale_index, 0, 2 );
    vxui_set_locale( ctx, vxui_demo_locale_code( app->locale_index ) );
}

static void vxui_demo_apply_prompt_table_index( vxui_ctx* ctx, vxui_demo_app* app, int prompt_table_index )
{
    if ( !ctx || !app ) {
        return;
    }
    app->prompt_table_index = std::clamp( prompt_table_index, 0, 1 );
    vxui_set_input_table( ctx, app->prompt_table_index == 0 ? &app->keyboard_table : &app->gamepad_table );
}

static void vxui_demo_restore_defaults( vxui_ctx* ctx, vxui_demo_app* app )
{
    if ( !ctx || !app ) {
        return;
    }

    app->difficulty = 1;
    app->volume = 0.40f;
    app->scanline_index = 1;
    app->effect_intensity_index = 1;
    vxui_demo_apply_locale_index( ctx, app, VXUI_DEMO_LOCALE_ENGLISH );
    vxui_demo_apply_prompt_table_index( ctx, app, 0 );
}

static const char* vxui_demo_text( const char* key, void* userdata )
{
    vxui_demo_app* app = ( vxui_demo_app* ) userdata;

    if ( app && app->ctx ) {
#ifdef VXUI_DEBUG
        if ( std::strcmp( key, "debug.selected_seq" ) == 0 ) {
            if ( app->ctx->debug_seq_editor.selected_seq >= 0 && app->ctx->debug_seq_editor.selected_seq < app->ctx->registered_seq_count ) {
                return app->ctx->registered_seqs[ app->ctx->debug_seq_editor.selected_seq ].name;
            }
            return vxui_demo_text( "common.none", userdata );
        }
        if ( std::strcmp( key, "debug.step.id" ) == 0 ) {
            if ( app->ctx->debug_seq_editor.selected_seq >= 0 && app->ctx->debug_seq_editor.selected_seq < app->ctx->registered_seq_count ) {
                const vxui_registered_seq* seq = &app->ctx->registered_seqs[ app->ctx->debug_seq_editor.selected_seq ];
                if ( seq->count > 0 ) {
                    static char fallback[ 32 ];
                    return vxui__get_seq_step_name( seq, 0, fallback, sizeof( fallback ) );
                }
            }
            return vxui_demo_text( "common.none", userdata );
        }
        if ( std::strcmp( key, "debug.generated_c" ) == 0 ) {
            return app->ctx->debug_seq_editor.generated_c[ 0 ] ? app->ctx->debug_seq_editor.generated_c : "(no generated c output)";
        }
        if ( std::strcmp( key, "debug.generated_toml" ) == 0 ) {
            return app->ctx->debug_seq_editor.generated_toml[ 0 ] ? app->ctx->debug_seq_editor.generated_toml : "(no generated toml output)";
        }
        if ( std::strcmp( key, "debug.preview_status" ) == 0 ) {
            return app->preview_status;
        }
        if ( std::strcmp( key, "debug.clipboard" ) == 0 ) {
            return app->clipboard_status;
        }
        if ( std::strcmp( key, "debug.reload" ) == 0 ) {
            return app->reload_status;
        }
#endif
    }

    if ( const char* command_deck_text = vxui_demo_lookup_command_deck_text( app ? app->locale_index : VXUI_DEMO_LOCALE_ENGLISH, key ) ) {
        return command_deck_text;
    }

    if ( app && app->locale_index == VXUI_DEMO_LOCALE_JAPANESE ) {
        if ( std::strcmp( key, "menu.main" ) == 0 ) return "メインメニュー";
        if ( std::strcmp( key, "menu.settings" ) == 0 ) return "設定";
        if ( std::strcmp( key, "menu.open_settings" ) == 0 ) return "設定を開く";
        if ( std::strcmp( key, "menu.back" ) == 0 ) return "戻る";
        if ( std::strcmp( key, "menu.difficulty" ) == 0 ) return "難易度";
        if ( std::strcmp( key, "menu.volume" ) == 0 ) return "音量";
        if ( std::strcmp( key, "menu.saves" ) == 0 ) return "セーブ";
        if ( std::strcmp( key, "menu.confirm" ) == 0 ) return "決定";
        if ( std::strcmp( key, "menu.cancel" ) == 0 ) return "戻る";
        if ( std::strcmp( key, "menu.locale" ) == 0 ) return "言語";
        if ( std::strcmp( key, "menu.prompts" ) == 0 ) return "プロンプト";
        if ( std::strcmp( key, "menu.controls" ) == 0 ) return "操作";
        if ( std::strcmp( key, "menu.debug" ) == 0 ) return "デバッグ";
        if ( std::strcmp( key, "debug.overlay.title" ) == 0 ) return "シーケンスデバッガー";
        if ( std::strcmp( key, "debug.label.preview" ) == 0 ) return "プレビュー";
        if ( std::strcmp( key, "debug.preview.captured" ) == 0 ) return "取得中";
        if ( std::strcmp( key, "debug.preview.idle" ) == 0 ) return "待機";
        if ( std::strcmp( key, "debug.preview.unavailable" ) == 0 ) return "プレビュー取得はデバッグビルド専用です。";
        if ( std::strcmp( key, "debug.clipboard.copied" ) == 0 ) return "現在の TOML 書き出しをクリップボードにコピーしました。";
        if ( std::strcmp( key, "debug.clipboard.ready" ) == 0 ) return "C で現在の書き出しをコピーまたは表示します。";
        if ( std::strcmp( key, "debug.clipboard.unavailable" ) == 0 ) return "シーケンス書き出しツールはデバッグビルド専用です。";
        if ( std::strcmp( key, "debug.reload.ready" ) == 0 ) return "R で監視中のシーケンスファイルを確認します。";
        if ( std::strcmp( key, "debug.reload.reloaded" ) == 0 ) return "監視中のシーケンスファイルを再読み込みしました。";
        if ( std::strcmp( key, "debug.reload.unchanged" ) == 0 ) return "監視中のシーケンスファイルに変更はありません。";
        if ( std::strcmp( key, "debug.reload.failed_prefix" ) == 0 ) return "再読み込みに失敗しました";
        if ( std::strcmp( key, "debug.reload.unavailable" ) == 0 ) return "ホットリロードはデバッグビルド専用です。";
        if ( std::strcmp( key, "difficulty.easy" ) == 0 ) return "かんたん";
        if ( std::strcmp( key, "difficulty.normal" ) == 0 ) return "ふつう";
        if ( std::strcmp( key, "difficulty.hard" ) == 0 ) return "むずかしい";
        if ( std::strcmp( key, "status.label.locale" ) == 0 ) return "言語";
        if ( std::strcmp( key, "status.label.prompts" ) == 0 ) return "プロンプト";
        if ( std::strcmp( key, "status.label.screens" ) == 0 ) return "画面数";
        if ( std::strcmp( key, "status.label.top" ) == 0 ) return "最上位";
        if ( std::strcmp( key, "status.short.locale" ) == 0 ) return "言語";
        if ( std::strcmp( key, "status.short.prompts" ) == 0 ) return "入力";
        if ( std::strcmp( key, "status.short.screens" ) == 0 ) return "画面";
        if ( std::strcmp( key, "status.short.top" ) == 0 ) return "上位";
        if ( std::strcmp( key, "locale.name.en" ) == 0 ) return "英語";
        if ( std::strcmp( key, "locale.name.ja" ) == 0 ) return "日本語";
        if ( std::strcmp( key, "locale.name.ar" ) == 0 ) return "アラビア語";
        if ( std::strcmp( key, "locale.short.en" ) == 0 ) return "EN";
        if ( std::strcmp( key, "locale.short.ja" ) == 0 ) return "JA";
        if ( std::strcmp( key, "locale.short.ar" ) == 0 ) return "AR";
        if ( std::strcmp( key, "prompt.name.keyboard" ) == 0 ) return "キーボード";
        if ( std::strcmp( key, "prompt.name.gamepad" ) == 0 ) return "ゲームパッド";
        if ( std::strcmp( key, "prompt.short.keyboard" ) == 0 ) return "KB";
        if ( std::strcmp( key, "prompt.short.gamepad" ) == 0 ) return "Pad";
        if ( std::strcmp( key, "screen.none" ) == 0 ) return "なし";
        if ( std::strcmp( key, "common.none" ) == 0 ) return "なし";
        if ( std::strcmp( key, "slot.0" ) == 0 ) return "オートセーブ";
        if ( std::strcmp( key, "slot.1" ) == 0 ) return "プロフィール 1";
        if ( std::strcmp( key, "slot.2" ) == 0 ) return "プロフィール 2";
        if ( std::strcmp( key, "slot.3" ) == 0 ) return "プロフィール 3";
        if ( std::strcmp( key, "slot.4" ) == 0 ) return "プロフィール 4";
        if ( std::strcmp( key, "slot.5" ) == 0 ) return "プロフィール 5";
        if ( std::strcmp( key, "slot.6" ) == 0 ) return "プロフィール 6";
        if ( std::strcmp( key, "slot.7" ) == 0 ) return "プロフィール 7";
        if ( std::strcmp( key, "hint.controls.0" ) == 0 ) return "矢印 / Dパッドで移動、Enter / Space / A で決定。";
        if ( std::strcmp( key, "hint.controls.1" ) == 0 ) return "Escape / Backspace / B で戻る、[ と ] でタブ切替。";
        if ( std::strcmp( key, "hint.controls.2" ) == 0 ) return "1 2 3 で言語、4 5 でプロンプト表を切替。";
#ifdef VXUI_DEBUG
        if ( std::strcmp( key, "hint.controls.3" ) == 0 ) return "F1 でデバッガー、R でホットリロード確認、C で書き出しをコピー。";
#else
        if ( std::strcmp( key, "hint.controls.3" ) == 0 ) return "デバッグビルドでシーケンスデバッガーを有効にできます。";
#endif
    } else if ( app && app->locale_index == VXUI_DEMO_LOCALE_ARABIC ) {
        if ( std::strcmp( key, "menu.main" ) == 0 ) return "القائمة الرئيسية";
        if ( std::strcmp( key, "menu.settings" ) == 0 ) return "الإعدادات";
        if ( std::strcmp( key, "menu.open_settings" ) == 0 ) return "افتح الإعدادات";
        if ( std::strcmp( key, "menu.back" ) == 0 ) return "رجوع";
        if ( std::strcmp( key, "menu.difficulty" ) == 0 ) return "الصعوبة";
        if ( std::strcmp( key, "menu.volume" ) == 0 ) return "الصوت";
        if ( std::strcmp( key, "menu.saves" ) == 0 ) return "الحفظ";
        if ( std::strcmp( key, "menu.confirm" ) == 0 ) return "تأكيد";
        if ( std::strcmp( key, "menu.cancel" ) == 0 ) return "إلغاء";
        if ( std::strcmp( key, "menu.locale" ) == 0 ) return "اللغة";
        if ( std::strcmp( key, "menu.prompts" ) == 0 ) return "جدول الأزرار";
        if ( std::strcmp( key, "menu.controls" ) == 0 ) return "التحكم";
        if ( std::strcmp( key, "menu.debug" ) == 0 ) return "التصحيح";
        if ( std::strcmp( key, "debug.overlay.title" ) == 0 ) return "مصحح التسلسلات";
        if ( std::strcmp( key, "debug.label.preview" ) == 0 ) return "المعاينة";
        if ( std::strcmp( key, "debug.preview.captured" ) == 0 ) return "قيد الالتقاط";
        if ( std::strcmp( key, "debug.preview.idle" ) == 0 ) return "خامل";
        if ( std::strcmp( key, "debug.preview.unavailable" ) == 0 ) return "التقاط المعاينة متاح فقط في بناء التصحيح.";
        if ( std::strcmp( key, "debug.clipboard.copied" ) == 0 ) return "تم نسخ تصدير TOML الحالي إلى الحافظة.";
        if ( std::strcmp( key, "debug.clipboard.ready" ) == 0 ) return "اضغط C لنسخ التصدير الحالي أو طباعته.";
        if ( std::strcmp( key, "debug.clipboard.unavailable" ) == 0 ) return "أدوات تصدير التسلسلات متاحة فقط في بناء التصحيح.";
        if ( std::strcmp( key, "debug.reload.ready" ) == 0 ) return "اضغط R للتحقق من ملف التسلسل المراقب.";
        if ( std::strcmp( key, "debug.reload.reloaded" ) == 0 ) return "تمت إعادة تحميل ملف التسلسل المراقب.";
        if ( std::strcmp( key, "debug.reload.unchanged" ) == 0 ) return "لم يتم العثور على تغيير في ملف التسلسل المراقب.";
        if ( std::strcmp( key, "debug.reload.failed_prefix" ) == 0 ) return "فشل إعادة التحميل";
        if ( std::strcmp( key, "debug.reload.unavailable" ) == 0 ) return "إعادة التحميل الفوري متاحة فقط في بناء التصحيح.";
        if ( std::strcmp( key, "difficulty.easy" ) == 0 ) return "سهل";
        if ( std::strcmp( key, "difficulty.normal" ) == 0 ) return "عادي";
        if ( std::strcmp( key, "difficulty.hard" ) == 0 ) return "صعب";
        if ( std::strcmp( key, "status.label.locale" ) == 0 ) return "اللغة";
        if ( std::strcmp( key, "status.label.prompts" ) == 0 ) return "الأزرار";
        if ( std::strcmp( key, "status.label.screens" ) == 0 ) return "الشاشات";
        if ( std::strcmp( key, "status.label.top" ) == 0 ) return "الأعلى";
        if ( std::strcmp( key, "status.short.locale" ) == 0 ) return "لغة";
        if ( std::strcmp( key, "status.short.prompts" ) == 0 ) return "دخل";
        if ( std::strcmp( key, "status.short.screens" ) == 0 ) return "شاشات";
        if ( std::strcmp( key, "status.short.top" ) == 0 ) return "أعلى";
        if ( std::strcmp( key, "locale.name.en" ) == 0 ) return "الإنجليزية";
        if ( std::strcmp( key, "locale.name.ja" ) == 0 ) return "اليابانية";
        if ( std::strcmp( key, "locale.name.ar" ) == 0 ) return "العربية";
        if ( std::strcmp( key, "locale.short.en" ) == 0 ) return "EN";
        if ( std::strcmp( key, "locale.short.ja" ) == 0 ) return "JA";
        if ( std::strcmp( key, "locale.short.ar" ) == 0 ) return "AR";
        if ( std::strcmp( key, "prompt.name.keyboard" ) == 0 ) return "لوحة المفاتيح";
        if ( std::strcmp( key, "prompt.name.gamepad" ) == 0 ) return "يد التحكم";
        if ( std::strcmp( key, "prompt.short.keyboard" ) == 0 ) return "KB";
        if ( std::strcmp( key, "prompt.short.gamepad" ) == 0 ) return "Pad";
        if ( std::strcmp( key, "screen.none" ) == 0 ) return "لا شيء";
        if ( std::strcmp( key, "common.none" ) == 0 ) return "لا شيء";
        if ( std::strcmp( key, "slot.0" ) == 0 ) return "حفظ تلقائي";
        if ( std::strcmp( key, "slot.1" ) == 0 ) return "الملف 1";
        if ( std::strcmp( key, "slot.2" ) == 0 ) return "الملف 2";
        if ( std::strcmp( key, "slot.3" ) == 0 ) return "الملف 3";
        if ( std::strcmp( key, "slot.4" ) == 0 ) return "الملف 4";
        if ( std::strcmp( key, "slot.5" ) == 0 ) return "الملف 5";
        if ( std::strcmp( key, "slot.6" ) == 0 ) return "الملف 6";
        if ( std::strcmp( key, "slot.7" ) == 0 ) return "الملف 7";
        if ( std::strcmp( key, "hint.controls.0" ) == 0 ) return "الأسهم / لوحة الاتجاهات للتنقل، و Enter / Space / A للتأكيد.";
        if ( std::strcmp( key, "hint.controls.1" ) == 0 ) return "Escape / Backspace / B للرجوع، و [ و ] للتبديل بين الأعمدة.";
        if ( std::strcmp( key, "hint.controls.2" ) == 0 ) return "1 2 3 لتبديل اللغة، و 4 5 لتبديل جدول الأزرار.";
#ifdef VXUI_DEBUG
        if ( std::strcmp( key, "hint.controls.3" ) == 0 ) return "F1 لفتح المصحح، و R للتحقق من إعادة التحميل، و C لنسخ المخرجات.";
#else
        if ( std::strcmp( key, "hint.controls.3" ) == 0 ) return "شغّل بناء التصحيح لتفعيل مصحح التسلسلات.";
#endif
    } else {
        if ( std::strcmp( key, "menu.main" ) == 0 ) return "Main Menu";
        if ( std::strcmp( key, "menu.settings" ) == 0 ) return "Settings";
        if ( std::strcmp( key, "menu.open_settings" ) == 0 ) return "Open Settings";
        if ( std::strcmp( key, "menu.back" ) == 0 ) return "Back";
        if ( std::strcmp( key, "menu.difficulty" ) == 0 ) return "Difficulty";
        if ( std::strcmp( key, "menu.volume" ) == 0 ) return "Volume";
        if ( std::strcmp( key, "menu.saves" ) == 0 ) return "Save Slots";
        if ( std::strcmp( key, "menu.confirm" ) == 0 ) return "Confirm";
        if ( std::strcmp( key, "menu.cancel" ) == 0 ) return "Cancel";
        if ( std::strcmp( key, "menu.locale" ) == 0 ) return "Locale";
        if ( std::strcmp( key, "menu.prompts" ) == 0 ) return "Prompt Table";
        if ( std::strcmp( key, "menu.controls" ) == 0 ) return "Controls";
        if ( std::strcmp( key, "menu.debug" ) == 0 ) return "Debug Overlay";
        if ( std::strcmp( key, "debug.overlay.title" ) == 0 ) return "Sequence Debugger";
        if ( std::strcmp( key, "debug.label.preview" ) == 0 ) return "Preview";
        if ( std::strcmp( key, "debug.preview.captured" ) == 0 ) return "captured";
        if ( std::strcmp( key, "debug.preview.idle" ) == 0 ) return "idle";
        if ( std::strcmp( key, "debug.preview.unavailable" ) == 0 ) return "Preview capture is only available in Debug builds.";
        if ( std::strcmp( key, "debug.clipboard.copied" ) == 0 ) return "Copied current TOML export to the clipboard.";
        if ( std::strcmp( key, "debug.clipboard.ready" ) == 0 ) return "Press C to copy or print the current export.";
        if ( std::strcmp( key, "debug.clipboard.unavailable" ) == 0 ) return "Sequence export tools are only available in Debug builds.";
        if ( std::strcmp( key, "debug.reload.ready" ) == 0 ) return "Press R to poll the watched sequence file.";
        if ( std::strcmp( key, "debug.reload.reloaded" ) == 0 ) return "Reloaded the watched sequence file.";
        if ( std::strcmp( key, "debug.reload.unchanged" ) == 0 ) return "No watched sequence change was detected.";
        if ( std::strcmp( key, "debug.reload.failed_prefix" ) == 0 ) return "Reload failed";
        if ( std::strcmp( key, "debug.reload.unavailable" ) == 0 ) return "Hot reload is only available in Debug builds.";
        if ( std::strcmp( key, "difficulty.easy" ) == 0 ) return "Easy";
        if ( std::strcmp( key, "difficulty.normal" ) == 0 ) return "Normal";
        if ( std::strcmp( key, "difficulty.hard" ) == 0 ) return "Hard";
        if ( std::strcmp( key, "status.label.locale" ) == 0 ) return "Locale";
        if ( std::strcmp( key, "status.label.prompts" ) == 0 ) return "Prompts";
        if ( std::strcmp( key, "status.label.screens" ) == 0 ) return "Screens";
        if ( std::strcmp( key, "status.label.top" ) == 0 ) return "Top";
        if ( std::strcmp( key, "status.short.locale" ) == 0 ) return "Loc";
        if ( std::strcmp( key, "status.short.prompts" ) == 0 ) return "Input";
        if ( std::strcmp( key, "status.short.screens" ) == 0 ) return "Views";
        if ( std::strcmp( key, "status.short.top" ) == 0 ) return "Top";
        if ( std::strcmp( key, "locale.name.en" ) == 0 ) return "English";
        if ( std::strcmp( key, "locale.name.ja" ) == 0 ) return "Japanese";
        if ( std::strcmp( key, "locale.name.ar" ) == 0 ) return "Arabic";
        if ( std::strcmp( key, "locale.short.en" ) == 0 ) return "EN";
        if ( std::strcmp( key, "locale.short.ja" ) == 0 ) return "JA";
        if ( std::strcmp( key, "locale.short.ar" ) == 0 ) return "AR";
        if ( std::strcmp( key, "prompt.name.keyboard" ) == 0 ) return "Keyboard";
        if ( std::strcmp( key, "prompt.name.gamepad" ) == 0 ) return "Gamepad";
        if ( std::strcmp( key, "prompt.short.keyboard" ) == 0 ) return "KB";
        if ( std::strcmp( key, "prompt.short.gamepad" ) == 0 ) return "Pad";
        if ( std::strcmp( key, "screen.none" ) == 0 ) return "none";
        if ( std::strcmp( key, "common.none" ) == 0 ) return "none";
        if ( std::strcmp( key, "slot.0" ) == 0 ) return "Autosave";
        if ( std::strcmp( key, "slot.1" ) == 0 ) return "Profile 1";
        if ( std::strcmp( key, "slot.2" ) == 0 ) return "Profile 2";
        if ( std::strcmp( key, "slot.3" ) == 0 ) return "Profile 3";
        if ( std::strcmp( key, "slot.4" ) == 0 ) return "Profile 4";
        if ( std::strcmp( key, "slot.5" ) == 0 ) return "Profile 5";
        if ( std::strcmp( key, "slot.6" ) == 0 ) return "Profile 6";
        if ( std::strcmp( key, "slot.7" ) == 0 ) return "Profile 7";
        if ( std::strcmp( key, "hint.controls.0" ) == 0 ) return "Arrows / D-pad navigate, Enter / Space / A confirm.";
        if ( std::strcmp( key, "hint.controls.1" ) == 0 ) return "Escape / Backspace / B cancels, [ and ] tab between lanes.";
        if ( std::strcmp( key, "hint.controls.2" ) == 0 ) return "1 2 3 switch locale, 4 5 switch prompt table.";
#ifdef VXUI_DEBUG
        if ( std::strcmp( key, "hint.controls.3" ) == 0 ) return "F1 toggles the debugger, R polls hot reload, C copies or prints exports.";
#else
        if ( std::strcmp( key, "hint.controls.3" ) == 0 ) return "Build a Debug configuration to enable the sequence debugger overlay.";
#endif
    }
    return key;
}

static const char* vxui_demo_locale_code( int locale_index )
{
    switch ( locale_index ) {
        case VXUI_DEMO_LOCALE_JAPANESE:
            return "ja-JP";
        case VXUI_DEMO_LOCALE_ARABIC:
            return "ar";
        default:
            return "en";
    }
}

static const char* vxui_demo_locale_name_key( int locale_index )
{
    switch ( locale_index ) {
        case VXUI_DEMO_LOCALE_JAPANESE:
            return "locale.name.ja";
        case VXUI_DEMO_LOCALE_ARABIC:
            return "locale.name.ar";
        default:
            return "locale.name.en";
    }
}

static const char* vxui_demo_footer_locale_key( int locale_index )
{
    switch ( locale_index ) {
        case VXUI_DEMO_LOCALE_JAPANESE:
            return "locale.short.ja";
        case VXUI_DEMO_LOCALE_ARABIC:
            return "locale.short.ar";
        default:
            return "locale.short.en";
    }
}

static const char* vxui_demo_prompt_name_key( int prompt_table_index )
{
    return prompt_table_index == 0 ? "prompt.name.keyboard" : "prompt.name.gamepad";
}

static const char* vxui_demo_footer_prompt_key( int prompt_table_index )
{
    return prompt_table_index == 0 ? "prompt.short.keyboard" : "prompt.short.gamepad";
}

static const char* vxui_demo_screen_name_key( const char* screen_name )
{
    switch ( vxui_demo_screen_kind_from_name( screen_name ) ) {
        case VXUI_DEMO_SCREEN_BOOT:
            return "screen.boot";
        case VXUI_DEMO_SCREEN_TITLE:
            return "screen.title";
        case VXUI_DEMO_SCREEN_MAIN_MENU:
            return "menu.main";
        case VXUI_DEMO_SCREEN_SORTIE:
            return "screen.sortie";
        case VXUI_DEMO_SCREEN_LOADOUT:
            return "screen.loadout";
        case VXUI_DEMO_SCREEN_ARCHIVES:
            return "screen.archives";
        case VXUI_DEMO_SCREEN_SETTINGS:
            return "menu.settings";
        case VXUI_DEMO_SCREEN_RECORDS:
            return "screen.records";
        case VXUI_DEMO_SCREEN_CREDITS:
            return "screen.credits";
        case VXUI_DEMO_SCREEN_LAUNCH_STUB:
            return "screen.launch_stub";
        case VXUI_DEMO_SCREEN_RESULTS_STUB:
            return "screen.results_stub";
        case VXUI_DEMO_SCREEN_UNKNOWN:
        default:
            return "screen.none";
    }
}

static const char* vxui_demo_footer_top_name( const vxui_demo_app* app, const vxui_ctx* ctx )
{
    const char* screen_name = vxui_demo_top_screen_name( ctx );
    switch ( vxui_demo_screen_kind_from_name( screen_name ) ) {
        case VXUI_DEMO_SCREEN_MAIN_MENU:
            if ( app && app->locale_index == VXUI_DEMO_LOCALE_JAPANESE ) return "デッキ";
            if ( app && app->locale_index == VXUI_DEMO_LOCALE_ARABIC ) return "السطح";
            return "Deck";
        case VXUI_DEMO_SCREEN_SORTIE:
            if ( app && app->locale_index == VXUI_DEMO_LOCALE_JAPANESE ) return "出撃";
            if ( app && app->locale_index == VXUI_DEMO_LOCALE_ARABIC ) return "الطلعة";
            return "Sortie";
        case VXUI_DEMO_SCREEN_LOADOUT:
            if ( app && app->locale_index == VXUI_DEMO_LOCALE_JAPANESE ) return "装備";
            if ( app && app->locale_index == VXUI_DEMO_LOCALE_ARABIC ) return "العتاد";
            return "Loadout";
        case VXUI_DEMO_SCREEN_ARCHIVES:
            if ( app && app->locale_index == VXUI_DEMO_LOCALE_JAPANESE ) return "記録庫";
            if ( app && app->locale_index == VXUI_DEMO_LOCALE_ARABIC ) return "الأرشيف";
            return "Archives";
        case VXUI_DEMO_SCREEN_SETTINGS:
            if ( app && app->locale_index == VXUI_DEMO_LOCALE_JAPANESE ) return "設定";
            if ( app && app->locale_index == VXUI_DEMO_LOCALE_ARABIC ) return "الإعدادات";
            return "Settings";
        case VXUI_DEMO_SCREEN_RECORDS:
            if ( app && app->locale_index == VXUI_DEMO_LOCALE_JAPANESE ) return "記録";
            if ( app && app->locale_index == VXUI_DEMO_LOCALE_ARABIC ) return "السجلات";
            return "Records";
        case VXUI_DEMO_SCREEN_CREDITS:
            if ( app && app->locale_index == VXUI_DEMO_LOCALE_JAPANESE ) return "制作";
            if ( app && app->locale_index == VXUI_DEMO_LOCALE_ARABIC ) return "الاعتمادات";
            return "Credits";
        case VXUI_DEMO_SCREEN_TITLE:
            if ( app && app->locale_index == VXUI_DEMO_LOCALE_JAPANESE ) return "開始";
            if ( app && app->locale_index == VXUI_DEMO_LOCALE_ARABIC ) return "العنوان";
            return "Title";
        case VXUI_DEMO_SCREEN_BOOT:
            if ( app && app->locale_index == VXUI_DEMO_LOCALE_JAPANESE ) return "起動";
            if ( app && app->locale_index == VXUI_DEMO_LOCALE_ARABIC ) return "التمهيد";
            return "Boot";
        default:
            return "Demo";
    }
}

static bool vxui_demo_locale_matches( const char* locale, const char* prefix )
{
    if ( !locale || !prefix ) {
        return false;
    }
    size_t prefix_len = std::strlen( prefix );
    if ( std::strncmp( locale, prefix, prefix_len ) != 0 ) {
        return false;
    }
    return locale[ prefix_len ] == '\0' || locale[ prefix_len ] == '-' || locale[ prefix_len ] == '_';
}

static float vxui_demo_font_line_height( const vxui_demo_renderer* renderer, ve_font_id font_id, float fallback )
{
    if ( !renderer || !ve_fontcache_is_valid_font_id( const_cast< ve_fontcache* >( &renderer->cache ), font_id ) ) {
        return fallback;
    }

    size_t index = ( size_t ) font_id;
    if ( index < renderer->demo_line_heights.size() && renderer->demo_line_heights[ index ] > 0.0f ) {
        return renderer->demo_line_heights[ index ];
    }

    const ve_fontcache_entry& entry = renderer->cache.entry[ index ];
    int ascent = 0;
    int descent = 0;
    int line_gap = 0;
    stbtt_GetFontVMetrics( &entry.info, &ascent, &descent, &line_gap );
    return ( float ) ( ascent - descent + line_gap ) * entry.size_scale;
}

static vxui_demo_font_metrics vxui_demo_resolve_font_metrics( const vxui_demo_renderer* renderer, uint32_t requested_font_id, float requested_font_size, const char* locale )
{
    auto locale_body_face = [&]( void ) -> ve_font_id
    {
        if ( vxui_demo_locale_matches( locale, "ja" ) ) {
            return VXUI_DEMO_FONT_JAPANESE;
        }
        if ( vxui_demo_locale_matches( locale, "ar" ) ) {
            return VXUI_DEMO_FONT_ARABIC;
        }
        return VXUI_DEMO_FONT_UI;
    };

    auto locale_title_face = [&]( void ) -> ve_font_id
    {
        if ( vxui_demo_locale_matches( locale, "ja" ) ) {
            return VXUI_DEMO_FONT_JAPANESE_TITLE;
        }
        if ( vxui_demo_locale_matches( locale, "ar" ) ) {
            return VXUI_DEMO_FONT_ARABIC_TITLE;
        }
        return VXUI_DEMO_FONT_TITLE;
    };

    auto locale_section_face = [&]( void ) -> ve_font_id
    {
        if ( vxui_demo_locale_matches( locale, "ja" ) ) {
            return VXUI_DEMO_FONT_JAPANESE_SECTION;
        }
        if ( vxui_demo_locale_matches( locale, "ar" ) ) {
            return VXUI_DEMO_FONT_ARABIC_SECTION;
        }
        return VXUI_DEMO_FONT_SECTION_TITLE;
    };

    ve_font_id face_id = ( ve_font_id ) requested_font_id;
    switch ( requested_font_id ) {
        case VXUI_DEMO_FONT_ROLE_BODY:
            face_id = locale_body_face();
            break;
        case VXUI_DEMO_FONT_ROLE_TITLE:
            face_id = locale_title_face();
            break;
        case VXUI_DEMO_FONT_ROLE_SECTION:
            face_id = locale_section_face();
            break;
        case VXUI_DEMO_FONT_ROLE_CODE:
            face_id = VXUI_DEMO_FONT_DEBUG;
            break;
    }

    float line_height = requested_font_size;
    if ( renderer ) {
        line_height = vxui_demo_font_line_height( renderer, face_id, requested_font_size );
    }

    return { face_id, line_height };
}

static void vxui_demo_font_resolver( vxui_ctx* ctx, uint32_t requested_font_id, float requested_font_size, const char* locale, void* userdata, vxui_resolved_font* out )
{
    ( void ) ctx;
    if ( !out ) {
        return;
    }

    const vxui_demo_renderer* renderer = ( const vxui_demo_renderer* ) userdata;
    vxui_demo_font_metrics metrics = vxui_demo_resolve_font_metrics( renderer, requested_font_id, requested_font_size, locale );
    out->font_id = metrics.font_id >= 0 ? ( uint32_t ) metrics.font_id : requested_font_id;
    out->line_height = metrics.line_height;
}

static float vxui_demo_control_height( const vxui_demo_renderer* renderer, const char* locale )
{
    vxui_demo_font_metrics metrics = vxui_demo_resolve_font_metrics( renderer, VXUI_DEMO_FONT_ROLE_BODY, ( float ) VXUI_DEMO_BODY_SIZE, locale );
    return std::max( 32.0f, metrics.line_height + VXUI_DEMO_BUTTON_PADDING_Y * 2.0f );
}

static void vxui_demo_emit_save_slot_row( vxui_ctx* ctx, const char* id, const char* label_key, float width, float height, bool rtl )
{
    const vxui_demo_command_deck_theme& theme = vxui_demo_command_deck_theme_tokens();
    const uint32_t action_id = vxui_id( id );
    vxui__register_action( ctx, action_id, nullptr, ( vxui_action_cfg ) { 0 } );
    vxui__get_anim_state( ctx, action_id, true );
    ctx->current_decl_id = action_id;

    const bool focused = ctx->focused_id == action_id;
    VXUI_HASH( ctx, action_id, {
        .layout = {
            .sizing = { CLAY_SIZING_FIXED( width ), CLAY_SIZING_FIXED( height ) },
            .padding = { 14, 14, 8, 8 },
            .childAlignment = { .x = rtl ? CLAY_ALIGN_X_RIGHT : CLAY_ALIGN_X_LEFT, .y = CLAY_ALIGN_Y_CENTER },
        },
        .backgroundColor = vxui_demo_clay_color( focused ? theme.focused_row_fill : theme.passive_row_fill ),
        .cornerRadius = CLAY_CORNER_RADIUS( 8 ),
        .border = vxui_demo_panel_border( focused ? theme.focused_row_border : theme.passive_row_border, 1 ),
    } ) {
        VXUI_LABEL( ctx, label_key, ( vxui_label_cfg ) { 0 } );
    }
}

static void vxui_demo_emit_action_button( vxui_ctx* ctx, const char* id, const char* l10n_key, vxui_action_fn fn, vxui_action_cfg cfg, float control_height )
{
    const vxui_demo_command_deck_theme& theme = vxui_demo_command_deck_theme_tokens();
    const char* resolved = vxui__resolve_text( ctx, l10n_key );
    uint32_t action_id = vxui_id( id );

    vxui__register_action( ctx, action_id, fn, cfg );
    vxui__get_anim_state( ctx, action_id, true );
    ctx->current_decl_id = action_id;
    const bool focused = ctx->focused_id == action_id;

    VXUI_HASH( ctx, action_id, {
        .layout = {
            .sizing = { CLAY_SIZING_FIT( 0 ), CLAY_SIZING_FIXED( control_height ) },
            .padding = { ( uint16_t ) VXUI_DEMO_BUTTON_PADDING_X, ( uint16_t ) VXUI_DEMO_BUTTON_PADDING_X, ( uint16_t ) VXUI_DEMO_BUTTON_PADDING_Y, ( uint16_t ) VXUI_DEMO_BUTTON_PADDING_Y },
            .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
        },
        .backgroundColor = vxui_demo_clay_color( focused ? theme.focused_row_fill : theme.action_fill ),
        .cornerRadius = CLAY_CORNER_RADIUS( 8 ),
        .border = vxui_demo_panel_border( focused ? theme.focused_row_border : theme.action_border, 1 ),
    } ) {
        vxui__emit_text(
            ctx,
            resolved,
            VXUI_DEMO_FONT_ROLE_BODY,
            ( float ) VXUI_DEMO_BODY_SIZE,
            focused ? theme.title_text : theme.action_text,
            action_id );
    }
}

#ifdef VXUI_DEBUG
static Clay_String vxui_demo_clay_string( const char* text )
{
    return ( Clay_String ) {
        .isStaticallyAllocated = false,
        .length = text ? ( int32_t ) std::strlen( text ) : 0,
        .chars = text ? text : "",
    };
}

static bool vxui_demo_get_element_bounds( const char* id, vxui_rect* out )
{
    Clay_ElementData element = Clay_GetElementData( Clay_GetElementId( vxui_demo_clay_string( id ) ) );
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

static bool vxui_demo_get_anim_bounds( const vxui_ctx* ctx, uint32_t id, vxui_rect* out )
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

static bool vxui_demo_probe_lookup_bounds_by_id( void* userdata, const char* id, vxui_rect* out )
{
    ( void ) userdata;
    return vxui_demo_get_element_bounds( id, out );
}

static bool vxui_demo_probe_lookup_bounds_by_hash( void* userdata, uint32_t id, vxui_rect* out )
{
    ( void ) userdata;
    Clay_ElementData data = Clay_GetElementData( vxui__clay_id_from_hash( id ) );
    if ( !data.found ) {
        return false;
    }
    if ( out ) {
        *out = vxui__rect_from_clay_box( data.boundingBox );
    }
    return true;
}

static bool vxui_demo_probe_lookup_anim_bounds( void* userdata, uint32_t id, vxui_rect* out )
{
    return vxui_demo_get_anim_bounds( ( const vxui_ctx* ) userdata, id, out );
}

static float vxui_demo_clamp_scroll_value( float value, float max_scroll )
{
    return std::clamp( value, 0.0f, std::max( 0.0f, max_scroll ) );
}

static void vxui_demo_step_settings_body_scroll( vxui_demo_app* app, float dt )
{
    if ( !app ) {
        return;
    }

    vxui__spring_step(
        app->settings_body_scroll_target,
        &app->settings_body_scroll_current,
        &app->settings_body_scroll_velocity,
        VXUI_DEFAULT_STIFFNESS,
        VXUI_DEFAULT_DAMPING,
        dt );
}

static void vxui_demo_sync_settings_body_scroll( vxui_demo_app* app, const vxui_ctx* ctx )
{
    if ( !app || !ctx || !app->show_settings ) {
        if ( app ) {
            app->settings_body_scroll_current = 0.0f;
            app->settings_body_scroll_target = 0.0f;
            app->settings_body_scroll_velocity = 0.0f;
        }
        return;
    }

    vxui_rect body_scroll = {};
    vxui_rect body = {};
    if ( !vxui_demo_get_element_bounds( "settings.body_scroll", &body_scroll )
        || !vxui_demo_get_element_bounds( "settings.body", &body ) ) {
        app->settings_body_scroll_current = 0.0f;
        app->settings_body_scroll_target = 0.0f;
        app->settings_body_scroll_velocity = 0.0f;
        return;
    }

    const float max_scroll = std::max( 0.0f, body.h - body_scroll.h );
    if ( max_scroll <= 1.0f ) {
        app->settings_body_scroll_current = 0.0f;
        app->settings_body_scroll_target = 0.0f;
        app->settings_body_scroll_velocity = 0.0f;
        return;
    }

    app->settings_body_scroll_current = vxui_demo_clamp_scroll_value( app->settings_body_scroll_current, max_scroll );
    app->settings_body_scroll_target = vxui_demo_clamp_scroll_value( app->settings_body_scroll_target, max_scroll );

    if ( ctx->focused_id == 0 ) {
        return;
    }

    vxui_rect focused = {};
    if ( !vxui_demo_get_anim_bounds( ctx, ctx->focused_id, &focused ) ) {
        return;
    }

    const float reveal_padding = 12.0f;
    const float body_top = body_scroll.y + reveal_padding;
    const float body_bottom = body_scroll.y + body_scroll.h - reveal_padding;
    const float focused_bottom = focused.y + focused.h;

    if ( focused.y < body_top ) {
        app->settings_body_scroll_target += focused.y - body_top;
    } else if ( focused_bottom > body_bottom ) {
        app->settings_body_scroll_target += focused_bottom - body_bottom;
    }

    app->settings_body_scroll_target = vxui_demo_clamp_scroll_value( app->settings_body_scroll_target, max_scroll );
}

static uint64_t vxui_demo_digest_warnings( const std::vector<std::string>& warnings )
{
    uint64_t hash = 1469598103934665603ull;
    for ( const std::string& warning : warnings ) {
        for ( unsigned char ch : warning ) {
            hash ^= ( uint64_t ) ch;
            hash *= 1099511628211ull;
        }
        hash ^= 0xFFu;
        hash *= 1099511628211ull;
    }
    return hash;
}

static bool vxui_demo_seq_targets_id( const vxui_registered_seq* seq, uint32_t id )
{
    if ( !seq || !seq->steps || seq->count <= 0 ) {
        return false;
    }

    for ( int i = 0; i < seq->count; ++i ) {
        if ( seq->steps[ i ].id == id ) {
            return true;
        }
    }
    return false;
}

static bool vxui_demo_has_title_border_regression( const vxui_draw_list* list, vxui_rect title_bounds )
{
    if ( !list ) {
        return false;
    }

    for ( int i = 0; i < list->length; ++i ) {
        const vxui_cmd* cmd = &list->commands[ i ];
        if ( cmd->type != VXUI_CMD_BORDER ) {
            continue;
        }

        const vxui_draw_cmd_border& border = cmd->border;
        const bool goldish = border.color.r >= 220 && border.color.g >= 170 && border.color.b <= 120;
        const bool hugs_title = std::fabs( border.bounds.x - title_bounds.x ) <= 10.0f
            && std::fabs( border.bounds.y - title_bounds.y ) <= 10.0f
            && std::fabs( border.bounds.w - title_bounds.w ) <= 24.0f
            && std::fabs( border.bounds.h - title_bounds.h ) <= 24.0f;
        if ( goldish && hugs_title ) {
            return true;
        }
    }

    return false;
}

static void vxui_demo_debug_validate_demo_layout( vxui_ctx* ctx, const vxui_draw_list* list )
{
    if ( !ctx ) {
        return;
    }

    const char* screen_name = vxui_demo_top_screen_name( ctx );
    if ( !screen_name || screen_name[ 0 ] == '\0' ) {
        return;
    }

    std::vector<std::string> warnings;
    vxui_demo_layout_probe probe = {
        ctx,
        list,
        screen_name,
        ctx,
        vxui_demo_probe_lookup_bounds_by_id,
        vxui_demo_probe_lookup_bounds_by_hash,
        vxui_demo_probe_lookup_anim_bounds,
    };
    vxui_demo_collect_layout_warnings( probe, &warnings );

    const float viewport_width = std::max( 0.0f, ( float ) ctx->cfg.screen_width - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f );
    const vxui_demo_surface_kind kind = vxui_demo_surface_kind_from_screen_id( screen_name );
    const vxui_demo_surface_metrics metrics = vxui_demo_compute_surface_metrics( viewport_width, ctx->locale, kind );
    vxui_rect surface = {};
    vxui_demo_get_element_bounds( vxui_demo_surface_id_for_screen( screen_name ), &surface );
    if ( kind == VXUI_DEMO_SURFACE_SETTINGS ) {
        vxui_rect title = {};
        if ( vxui_demo_get_element_bounds( "settings.header", &title ) && vxui_demo_has_title_border_regression( list, title ) ) {
            warnings.emplace_back( "decorative title border regressed; the old gold guide frame is back." );
        }
        if ( surface.w > 0.0f && metrics.control_lane_width < surface.w * 0.40f ) {
            warnings.emplace_back( "settings control lane contract regressed toward a cramped desktop form layout." );
        }
    }

    static uint64_t last_digest = 0;
    static bool had_warnings = false;

    if ( warnings.empty() ) {
        last_digest = 0;
        had_warnings = false;
        return;
    }

    const uint64_t digest = vxui_demo_digest_warnings( warnings );
    if ( had_warnings && digest == last_digest ) {
        return;
    }

    had_warnings = true;
    last_digest = digest;
    std::fprintf( stderr, "vxui demo layout warnings:\n" );
    for ( const std::string& warning : warnings ) {
        std::fprintf( stderr, "  - %s\n", warning.c_str() );
    }
}
#endif

static FILE* vxui_demo_fopen( const char* path, const char* mode )
{
    if ( !path || !mode ) {
        return nullptr;
    }
#if defined( _MSC_VER )
    FILE* fp = nullptr;
    return fopen_s( &fp, path, mode ) == 0 ? fp : nullptr;
#else
    return std::fopen( path, mode );
#endif
}

static bool vxui_demo_write_file( const char* path, const char* text )
{
    FILE* fp = vxui_demo_fopen( path, "wb" );
    if ( !fp ) {
        return false;
    }

    size_t length = std::strlen( text );
    bool ok = std::fwrite( text, 1, length, fp ) == length;
    std::fclose( fp );
    return ok;
}

static std::string vxui_demo_make_temp_path( const char* filename )
{
    std::filesystem::path temp = std::filesystem::temp_directory_path() / "vxui-demo";
    std::error_code ec;
    std::filesystem::create_directories( temp, ec );
    std::filesystem::path path = temp / filename;
    return path.make_preferred().string();
}

static std::u8string vxui_demo_to_u8( const char* text )
{
    if ( !text ) {
        return {};
    }
    const char8_t* begin = reinterpret_cast< const char8_t* >( text );
    return std::u8string( begin, begin + std::strlen( text ) );
}

static bool vxui_demo_copy_to_clipboard( const char* text )
{
#if defined( _WIN32 )
    if ( !text || !OpenClipboard( nullptr ) ) {
        return false;
    }

    EmptyClipboard();
    size_t length = std::strlen( text ) + 1u;
    HGLOBAL handle = GlobalAlloc( GMEM_MOVEABLE, length );
    if ( !handle ) {
        CloseClipboard();
        return false;
    }

    void* memory = GlobalLock( handle );
    if ( !memory ) {
        GlobalFree( handle );
        CloseClipboard();
        return false;
    }
    std::memcpy( memory, text, length );
    GlobalUnlock( handle );

    if ( !SetClipboardData( CF_TEXT, handle ) ) {
        GlobalFree( handle );
        CloseClipboard();
        return false;
    }
    CloseClipboard();
    return true;
#else
    ( void ) text;
    return false;
#endif
}

static int vxui_demo_find_seq_index( const vxui_ctx* ctx, const char* name )
{
    if ( !ctx || !name ) {
        return -1;
    }
    for ( int i = 0; i < ctx->registered_seq_count; ++i ) {
        if ( std::strcmp( ctx->registered_seqs[ i ].name, name ) == 0 ) {
            return i;
        }
    }
    return -1;
}

static void vxui_demo_sync_step_editor( vxui_demo_app* app )
{
#ifdef VXUI_DEBUG
    if ( !app || !app->ctx ) {
        return;
    }

    int selected = app->ctx->debug_seq_editor.selected_seq;
    if ( selected < 0 || selected >= app->ctx->registered_seq_count ) {
        return;
    }

    vxui_registered_seq* seq = &app->ctx->registered_seqs[ selected ];
    if ( seq->count <= 0 ) {
        return;
    }

    app->debug_delay_value = ( float ) seq->steps[ 0 ].delay_ms;
    app->debug_target_value = seq->steps[ 0 ].target;
    switch ( seq->steps[ 0 ].prop ) {
        case VXUI_PROP_OPACITY: app->debug_prop_index = 0; break;
        case VXUI_PROP_SCALE: app->debug_prop_index = 1; break;
        case VXUI_PROP_SLIDE_X: app->debug_prop_index = 2; break;
        case VXUI_PROP_SLIDE_Y: app->debug_prop_index = 3; break;
    }
    app->last_selected_seq = selected;
#else
    ( void ) app;
#endif
}

static void vxui_demo_apply_step_editor( vxui_demo_app* app )
{
#ifdef VXUI_DEBUG
    if ( !app || !app->ctx ) {
        return;
    }

    int selected = app->ctx->debug_seq_editor.selected_seq;
    if ( selected < 0 || selected >= app->ctx->registered_seq_count ) {
        return;
    }

    vxui_registered_seq* seq = &app->ctx->registered_seqs[ selected ];
    if ( seq->count <= 0 ) {
        return;
    }

    seq->steps[ 0 ].delay_ms = ( uint32_t ) std::lround( app->debug_delay_value );
    seq->steps[ 0 ].target = app->debug_target_value;
    seq->steps[ 0 ].prop = app->debug_prop_index == 0 ? VXUI_PROP_OPACITY :
                           app->debug_prop_index == 1 ? VXUI_PROP_SCALE :
                           app->debug_prop_index == 2 ? VXUI_PROP_SLIDE_X : VXUI_PROP_SLIDE_Y;
#else
    ( void ) app;
#endif
}

static void vxui_demo_refresh_status( vxui_demo_app* app )
{
    if ( !app || !app->ctx ) {
        return;
    }

#ifdef VXUI_DEBUG
    const char* preview_state = vxui_demo_text( app->ctx->debug_seq_editor.preview_playing ? "debug.preview.captured" : "debug.preview.idle", app );
    std::snprintf(
        app->preview_status,
        sizeof( app->preview_status ),
        "%s: %s (%d cmds)",
        vxui_demo_text( "debug.label.preview", app ),
        preview_state,
        app->ctx->debug_seq_editor.preview_snapshot.command_count );
    std::snprintf(
        app->clipboard_status,
        sizeof( app->clipboard_status ),
        "%s",
        app->copied_to_clipboard ? vxui_demo_text( "debug.clipboard.copied", app ) : vxui_demo_text( "debug.clipboard.ready", app ) );
    if ( app->reload_status[ 0 ] == '\0' ) {
        std::snprintf( app->reload_status, sizeof( app->reload_status ), "%s", vxui_demo_text( "debug.reload.ready", app ) );
    }
#else
    std::snprintf( app->preview_status, sizeof( app->preview_status ), "%s", vxui_demo_text( "debug.preview.unavailable", app ) );
    std::snprintf( app->clipboard_status, sizeof( app->clipboard_status ), "%s", vxui_demo_text( "debug.clipboard.unavailable", app ) );
    std::snprintf( app->reload_status, sizeof( app->reload_status ), "%s", vxui_demo_text( "debug.reload.unavailable", app ) );
#endif
}

static void vxui_demo_open_settings( vxui_ctx* ctx, uint32_t id, void* userdata )
{
    ( void ) id;
    vxui_demo_app* app = ( vxui_demo_app* ) userdata;
    if ( !ctx || !app ) {
        return;
    }
    app->show_settings = true;
    app->settings_body_scroll_current = 0.0f;
    app->settings_body_scroll_target = 0.0f;
    app->settings_body_scroll_velocity = 0.0f;
    if ( ctx->screen_count > 0 ) {
        vxui_replace_screen( ctx, "settings" );
    } else {
        vxui_push_screen( ctx, "settings" );
    }
}

static void vxui_demo_close_settings( vxui_ctx* ctx, uint32_t id, void* userdata )
{
    ( void ) id;
    vxui_demo_app* app = ( vxui_demo_app* ) userdata;
    if ( !ctx || !app ) {
        return;
    }
    app->show_settings = false;
    app->settings_body_scroll_current = 0.0f;
    app->settings_body_scroll_target = 0.0f;
    app->settings_body_scroll_velocity = 0.0f;
    if ( ctx->screen_count > 0 ) {
        vxui_replace_screen( ctx, "main_menu" );
    } else {
        vxui_push_screen( ctx, "main_menu" );
    }
}

static GLuint vxui_demo_compile_program( const char* vs_source, const char* fs_source, const char* label )
{
    auto print_shader_log = []( GLuint shader ) {
        char log[ 4096 ] = {};
        glGetShaderInfoLog( shader, ( GLsizei ) sizeof( log ), nullptr, log );
        if ( log[ 0 ] ) {
            std::fprintf( stderr, "%s\n", log );
        }
    };
    auto print_program_log = []( GLuint program ) {
        char log[ 4096 ] = {};
        glGetProgramInfoLog( program, ( GLsizei ) sizeof( log ), nullptr, log );
        if ( log[ 0 ] ) {
            std::fprintf( stderr, "%s\n", log );
        }
    };

    GLuint vs = glCreateShader( GL_VERTEX_SHADER );
    GLuint fs = glCreateShader( GL_FRAGMENT_SHADER );
    GLint ok = 0;

    glShaderSource( vs, 1, &vs_source, nullptr );
    glCompileShader( vs );
    glGetShaderiv( vs, GL_COMPILE_STATUS, &ok );
    if ( !ok ) {
        std::fprintf( stderr, "Failed to compile %s vertex shader.\n", label );
        print_shader_log( vs );
        glDeleteShader( vs );
        glDeleteShader( fs );
        return 0;
    }

    glShaderSource( fs, 1, &fs_source, nullptr );
    glCompileShader( fs );
    glGetShaderiv( fs, GL_COMPILE_STATUS, &ok );
    if ( !ok ) {
        std::fprintf( stderr, "Failed to compile %s fragment shader.\n", label );
        print_shader_log( fs );
        glDeleteShader( vs );
        glDeleteShader( fs );
        return 0;
    }

    GLuint program = glCreateProgram();
    glAttachShader( program, vs );
    glAttachShader( program, fs );
    glBindAttribLocation( program, 0, "vpos" );
    glBindAttribLocation( program, 1, "vtex" );
    glLinkProgram( program );
    glGetProgramiv( program, GL_LINK_STATUS, &ok );
    if ( !ok ) {
        std::fprintf( stderr, "Failed to link %s program.\n", label );
        print_program_log( program );
        glDeleteProgram( program );
        program = 0;
    }

    if ( program ) {
        glDetachShader( program, vs );
        glDetachShader( program, fs );
    }
    glDeleteShader( vs );
    glDeleteShader( fs );
    return program;
}

static void vxui_demo_setup_fontcache_fbo( vxui_demo_renderer* renderer )
{
    glGenFramebuffers( 2, renderer->fontcache_fbo );
    glGenTextures( 2, renderer->fontcache_fbo_texture );

    glBindFramebuffer( GL_FRAMEBUFFER, renderer->fontcache_fbo[ 0 ] );
    glBindTexture( GL_TEXTURE_2D, renderer->fontcache_fbo_texture[ 0 ] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_R8, VE_FONTCACHE_GLYPHDRAW_BUFFER_WIDTH, VE_FONTCACHE_GLYPHDRAW_BUFFER_HEIGHT, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderer->fontcache_fbo_texture[ 0 ], 0 );

    glBindFramebuffer( GL_FRAMEBUFFER, renderer->fontcache_fbo[ 1 ] );
    glBindTexture( GL_TEXTURE_2D, renderer->fontcache_fbo_texture[ 1 ] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_R8, VE_FONTCACHE_ATLAS_WIDTH, VE_FONTCACHE_ATLAS_HEIGHT, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderer->fontcache_fbo_texture[ 1 ], 0 );

    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
}

static void vxui_demo_flip_fontcache_upload( ve_fontcache_drawlist* drawlist, ve_fontcache_draw& dcall )
{
#ifdef VE_FONTCACHE_FREETYPE_RASTERISATION
    if ( dcall.pass != VE_FONTCACHE_FRAMEBUFFER_PASS_ATLAS_UPLOAD ) {
        return;
    }

    static std::vector< uint8_t > temp_texels;
    size_t total = ( size_t ) dcall.upload_region_w * ( size_t ) dcall.upload_region_h;
    temp_texels.resize( total );
    uint8_t* src = &drawlist->texels[ dcall.texel_offset ];
    for ( uint32_t y = 0; y < dcall.upload_region_h; ++y ) {
        uint32_t dst_row = dcall.upload_region_h - 1u - y;
        std::memcpy( &temp_texels[ dst_row * dcall.upload_region_w ], &src[ y * dcall.upload_region_w ], dcall.upload_region_w );
    }
    std::memcpy( src, temp_texels.data(), total );
#else
    ( void ) drawlist;
    ( void ) dcall;
#endif
}

static void vxui_demo_clear_framebuffer_colour( vxui_demo_renderer* renderer, GLuint framebuffer )
{
    GLint previous_framebuffer = 0;
    glGetIntegerv( GL_FRAMEBUFFER_BINDING, &previous_framebuffer );
    glBindFramebuffer( GL_FRAMEBUFFER, framebuffer );
    glViewport(
        0,
        0,
        framebuffer == 0 ? static_cast< GLsizei >( renderer->window_size.width ) : ( framebuffer == renderer->fontcache_fbo[ 0 ] ? VE_FONTCACHE_GLYPHDRAW_BUFFER_WIDTH : VE_FONTCACHE_ATLAS_WIDTH ),
        framebuffer == 0 ? static_cast< GLsizei >( renderer->window_size.height ) : ( framebuffer == renderer->fontcache_fbo[ 0 ] ? VE_FONTCACHE_GLYPHDRAW_BUFFER_HEIGHT : VE_FONTCACHE_ATLAS_HEIGHT ) );
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
    glClear( GL_COLOR_BUFFER_BIT );
    glBindFramebuffer( GL_FRAMEBUFFER, static_cast< GLuint >( previous_framebuffer ) );
}

static void vxui_demo_clear_backend_test_surface( vxui_demo_renderer* renderer, GLuint texture, int width, int height )
{
    if ( texture == 0 ) {
        return;
    }
    static GLuint clear_fbo = 0;
    if ( clear_fbo == 0 ) {
        glGenFramebuffers( 1, &clear_fbo );
    }
    GLint previous_read_framebuffer = 0;
    GLint previous_draw_framebuffer = 0;
    glGetIntegerv( GL_READ_FRAMEBUFFER_BINDING, &previous_read_framebuffer );
    glGetIntegerv( GL_DRAW_FRAMEBUFFER_BINDING, &previous_draw_framebuffer );

    glBindFramebuffer( GL_FRAMEBUFFER, clear_fbo );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0 );
    glViewport( 0, 0, width, height );
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
    glClear( GL_COLOR_BUFFER_BIT );

    glBindFramebuffer( GL_READ_FRAMEBUFFER, static_cast< GLuint >( previous_read_framebuffer ) );
    glBindFramebuffer( GL_DRAW_FRAMEBUFFER, static_cast< GLuint >( previous_draw_framebuffer ) );
}

static void vxui_demo_clear_cpu_atlas_pages( vxui_demo_renderer* renderer, bool clear_all )
{
#ifdef VE_FONTCACHE_FREETYPE_RASTERISATION
    if ( renderer->cpu_atlas_textures.empty() ) {
        return;
    }
    static std::vector< uint8_t > zeros( static_cast< size_t >( VE_FONTCACHE_CPU_ATLAS_PAGE_SIZE ) * VE_FONTCACHE_CPU_ATLAS_PAGE_SIZE, 0 );
    GLint previous_texture = 0;
    glGetIntegerv( GL_TEXTURE_BINDING_2D, &previous_texture );
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    glPixelStorei( GL_PACK_ALIGNMENT, 1 );
    size_t count = clear_all ? renderer->cpu_atlas_textures.size() : 1;
    for ( size_t i = 0; i < count; ++i ) {
        if ( renderer->cpu_atlas_textures[ i ] == 0 ) {
            continue;
        }
        glBindTexture( GL_TEXTURE_2D, renderer->cpu_atlas_textures[ i ] );
        glTexSubImage2D(
            GL_TEXTURE_2D,
            0,
            0,
            0,
            VE_FONTCACHE_CPU_ATLAS_PAGE_SIZE,
            VE_FONTCACHE_CPU_ATLAS_PAGE_SIZE,
            GL_RED,
            GL_UNSIGNED_BYTE,
            zeros.data() );
    }
    glBindTexture( GL_TEXTURE_2D, static_cast< GLuint >( previous_texture ) );
#else
    ( void ) renderer;
    ( void ) clear_all;
#endif
}

static void vxui_demo_clear_backend_test_surfaces( vxui_demo_renderer* renderer, bool clear_cpu_atlas_pages )
{
    vxui_demo_clear_framebuffer_colour( renderer, renderer->fontcache_fbo[ 0 ] );
    vxui_demo_clear_framebuffer_colour( renderer, renderer->fontcache_fbo[ 1 ] );
    vxui_demo_clear_backend_test_surface( renderer, renderer->backend_target_texture, static_cast< int >( renderer->window_size.width ), static_cast< int >( renderer->window_size.height ) );
    vxui_demo_clear_backend_test_surface( renderer, renderer->backend_present_texture, static_cast< int >( renderer->window_size.width ), static_cast< int >( renderer->window_size.height ) );
    if ( clear_cpu_atlas_pages ) {
        vxui_demo_clear_cpu_atlas_pages( renderer, true );
    }
}

static bool vxui_demo_setup_backend_test_fbo( vxui_demo_renderer* renderer )
{
    glGenFramebuffers( 1, &renderer->backend_target_fbo );
    glGenTextures( 1, &renderer->backend_target_texture );
    glBindTexture( GL_TEXTURE_2D, renderer->backend_target_texture );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_R8, renderer->window_size.width, renderer->window_size.height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glBindFramebuffer( GL_FRAMEBUFFER, renderer->backend_target_fbo );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderer->backend_target_texture, 0 );

    glGenFramebuffers( 1, &renderer->backend_present_fbo );
    glGenTextures( 1, &renderer->backend_present_texture );
    glBindTexture( GL_TEXTURE_2D, renderer->backend_present_texture );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_R8, renderer->window_size.width, renderer->window_size.height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glBindFramebuffer( GL_FRAMEBUFFER, renderer->backend_present_fbo );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderer->backend_present_texture, 0 );

    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    vxui_demo_gl_debug_label( renderer, GL_FRAMEBUFFER, renderer->backend_target_fbo, "vxui.backend_test.target_fbo" );
    vxui_demo_gl_debug_label( renderer, GL_TEXTURE, renderer->backend_target_texture, "vxui.backend_test.target_texture" );
    vxui_demo_gl_debug_label( renderer, GL_FRAMEBUFFER, renderer->backend_present_fbo, "vxui.backend_test.present_fbo" );
    vxui_demo_gl_debug_label( renderer, GL_TEXTURE, renderer->backend_present_texture, "vxui.backend_test.present_texture" );
    return true;
}

static void vxui_demo_shutdown_backend_test_fbo( vxui_demo_renderer* renderer )
{
    if ( renderer->backend_target_fbo ) {
        glDeleteFramebuffers( 1, &renderer->backend_target_fbo );
        renderer->backend_target_fbo = 0;
    }
    if ( renderer->backend_target_texture ) {
        glDeleteTextures( 1, &renderer->backend_target_texture );
        renderer->backend_target_texture = 0;
    }
    if ( renderer->backend_present_fbo ) {
        glDeleteFramebuffers( 1, &renderer->backend_present_fbo );
        renderer->backend_present_fbo = 0;
    }
    if ( renderer->backend_present_texture ) {
        glDeleteTextures( 1, &renderer->backend_present_texture );
        renderer->backend_present_texture = 0;
    }
}

static bool vxui_demo_setup_shot_capture_fbo( vxui_demo_renderer* renderer )
{
    if ( !renderer || renderer->window_size.width == 0 || renderer->window_size.height == 0 ) {
        return false;
    }

    GLint previous_texture = 0;
    GLint previous_framebuffer = 0;
    glGetIntegerv( GL_TEXTURE_BINDING_2D, &previous_texture );
    glGetIntegerv( GL_FRAMEBUFFER_BINDING, &previous_framebuffer );

    glGenFramebuffers( 1, &renderer->shot_capture_fbo );
    glGenTextures( 1, &renderer->shot_capture_texture );
    glBindTexture( GL_TEXTURE_2D, renderer->shot_capture_texture );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, renderer->window_size.width, renderer->window_size.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

    glBindFramebuffer( GL_FRAMEBUFFER, renderer->shot_capture_fbo );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderer->shot_capture_texture, 0 );
    const bool complete = glCheckFramebufferStatus( GL_FRAMEBUFFER ) == GL_FRAMEBUFFER_COMPLETE;

    glBindTexture( GL_TEXTURE_2D, static_cast< GLuint >( previous_texture ) );
    glBindFramebuffer( GL_FRAMEBUFFER, static_cast< GLuint >( previous_framebuffer ) );

    if ( !complete ) {
        vxui_demo_shutdown_shot_capture_fbo( renderer );
        return false;
    }

    renderer->shot_capture_mode = true;
    vxui_demo_gl_debug_label( renderer, GL_FRAMEBUFFER, renderer->shot_capture_fbo, "vxui.shot_capture.fbo" );
    vxui_demo_gl_debug_label( renderer, GL_TEXTURE, renderer->shot_capture_texture, "vxui.shot_capture.texture" );
    return true;
}

static void vxui_demo_shutdown_shot_capture_fbo( vxui_demo_renderer* renderer )
{
    if ( !renderer ) {
        return;
    }
    if ( renderer->shot_capture_fbo ) {
        glDeleteFramebuffers( 1, &renderer->shot_capture_fbo );
        renderer->shot_capture_fbo = 0;
    }
    if ( renderer->shot_capture_texture ) {
        glDeleteTextures( 1, &renderer->shot_capture_texture );
        renderer->shot_capture_texture = 0;
    }
    renderer->shot_capture_mode = false;
}

static bool vxui_demo_readback_r8_texture( GLuint texture, int x, int y, int w, int h, uint8_t* out_pixels )
{
    if ( texture == 0 || !out_pixels ) {
        return false;
    }
    static GLuint readback_fbo = 0;
    if ( readback_fbo == 0 ) {
        glGenFramebuffers( 1, &readback_fbo );
    }
    GLint previous_read_framebuffer = 0;
    glGetIntegerv( GL_READ_FRAMEBUFFER_BINDING, &previous_read_framebuffer );
    glBindFramebuffer( GL_READ_FRAMEBUFFER, readback_fbo );
    glFramebufferTexture2D( GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0 );
    glReadBuffer( GL_COLOR_ATTACHMENT0 );
    glPixelStorei( GL_PACK_ALIGNMENT, 1 );
    glReadPixels( x, y, w, h, GL_RED, GL_UNSIGNED_BYTE, out_pixels );
    glBindFramebuffer( GL_READ_FRAMEBUFFER, static_cast< GLuint >( previous_read_framebuffer ) );
    return true;
}

static void vxui_demo_backend_test_reset_surfaces()
{
    // Reset surfaces will be called with a valid renderer pointer via lambda capture
}

static bool vxui_demo_backend_test_readback( const char* name, int x, int y, int w, int h, uint8_t* out_pixels )
{
    extern vxui_demo_renderer* g_vxui_demo_backend_test_renderer;
    vxui_demo_renderer* renderer = g_vxui_demo_backend_test_renderer;
    if ( !renderer || !name || !out_pixels ) {
        return false;
    }

    if ( std::strcmp( name, "glyph_buffer" ) == 0 ) {
        GLint previous_read_framebuffer = 0;
        glGetIntegerv( GL_READ_FRAMEBUFFER_BINDING, &previous_read_framebuffer );
        glBindFramebuffer( GL_READ_FRAMEBUFFER, renderer->fontcache_fbo[ 0 ] );
        glReadBuffer( GL_COLOR_ATTACHMENT0 );
        glPixelStorei( GL_PACK_ALIGNMENT, 1 );
        glReadPixels( x, y, w, h, GL_RED, GL_UNSIGNED_BYTE, out_pixels );
        glBindFramebuffer( GL_READ_FRAMEBUFFER, static_cast< GLuint >( previous_read_framebuffer ) );
        return true;
    } else if ( std::strcmp( name, "atlas" ) == 0 ) {
        GLint previous_read_framebuffer = 0;
        glGetIntegerv( GL_READ_FRAMEBUFFER_BINDING, &previous_read_framebuffer );
        glBindFramebuffer( GL_READ_FRAMEBUFFER, renderer->fontcache_fbo[ 1 ] );
        glReadBuffer( GL_COLOR_ATTACHMENT0 );
        glPixelStorei( GL_PACK_ALIGNMENT, 1 );
        glReadPixels( x, y, w, h, GL_RED, GL_UNSIGNED_BYTE, out_pixels );
        glBindFramebuffer( GL_READ_FRAMEBUFFER, static_cast< GLuint >( previous_read_framebuffer ) );
        return true;
    } else if ( std::strcmp( name, "target_linear" ) == 0 || std::strcmp( name, "target" ) == 0 ) {
        return vxui_demo_readback_r8_texture( renderer->backend_target_texture, x, y, w, h, out_pixels );
    } else if ( std::strcmp( name, "presented" ) == 0 ) {
        return vxui_demo_readback_r8_texture( renderer->backend_present_texture, x, y, w, h, out_pixels );
#ifdef VE_FONTCACHE_FREETYPE_RASTERISATION
    } else if ( std::strcmp( name, "cpu_atlas_page_0" ) == 0 ) {
        if ( renderer->cpu_atlas_textures.empty() || renderer->cpu_atlas_textures[ 0 ] == 0 ) {
            return false;
        }
        return vxui_demo_readback_r8_texture( renderer->cpu_atlas_textures[ 0 ], x, y, w, h, out_pixels );
#endif
    }
    return false;
}

static bool vxui_demo_backend_test_write_surface( const char* name, int x, int y, int w, int h, const uint8_t* pixels )
{
    extern vxui_demo_renderer* g_vxui_demo_backend_test_renderer;
    vxui_demo_renderer* renderer = g_vxui_demo_backend_test_renderer;
    if ( !renderer || !name || !pixels || x < 0 || y < 0 || w <= 0 || h <= 0 ) {
        return false;
    }

    GLuint texture = 0;
    int texture_width = 0;
    int texture_height = 0;
    if ( std::strcmp( name, "glyph_buffer" ) == 0 ) {
        texture = renderer->fontcache_fbo_texture[ 0 ];
        texture_width = VE_FONTCACHE_GLYPHDRAW_BUFFER_WIDTH;
        texture_height = VE_FONTCACHE_GLYPHDRAW_BUFFER_HEIGHT;
    } else if ( std::strcmp( name, "atlas" ) == 0 ) {
        texture = renderer->fontcache_fbo_texture[ 1 ];
        texture_width = VE_FONTCACHE_ATLAS_WIDTH;
        texture_height = VE_FONTCACHE_ATLAS_HEIGHT;
    } else if ( std::strcmp( name, "target_linear" ) == 0 ) {
        texture = renderer->backend_target_texture;
        texture_width = static_cast< int >( renderer->window_size.width );
        texture_height = static_cast< int >( renderer->window_size.height );
    } else if ( std::strcmp( name, "presented" ) == 0 ) {
        texture = renderer->backend_present_texture;
        texture_width = static_cast< int >( renderer->window_size.width );
        texture_height = static_cast< int >( renderer->window_size.height );
#ifdef VE_FONTCACHE_FREETYPE_RASTERISATION
    } else if ( std::strcmp( name, "cpu_atlas_page_0" ) == 0 ) {
        renderer->cpu_atlas_textures.resize( 1 );
        if ( renderer->cpu_atlas_textures[ 0 ] == 0 ) {
            glGenTextures( 1, &renderer->cpu_atlas_textures[ 0 ] );
            glBindTexture( GL_TEXTURE_2D, renderer->cpu_atlas_textures[ 0 ] );
            glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
            glPixelStorei( GL_PACK_ALIGNMENT, 1 );
            glTexImage2D( GL_TEXTURE_2D, 0, GL_R8, VE_FONTCACHE_CPU_ATLAS_PAGE_SIZE, VE_FONTCACHE_CPU_ATLAS_PAGE_SIZE, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        }
        texture = renderer->cpu_atlas_textures[ 0 ];
        texture_width = VE_FONTCACHE_CPU_ATLAS_PAGE_SIZE;
        texture_height = VE_FONTCACHE_CPU_ATLAS_PAGE_SIZE;
#endif
    } else {
        return false;
    }

    if ( x + w > texture_width || y + h > texture_height ) {
        return false;
    }

    GLint previous_texture = 0;
    glGetIntegerv( GL_TEXTURE_BINDING_2D, &previous_texture );
    GLint previous_fbo = 0;
    glGetIntegerv( GL_FRAMEBUFFER_BINDING, &previous_fbo );

    // Detach texture from FBO before uploading
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    glBindTexture( GL_TEXTURE_2D, texture );
    // Clear any pending errors before TexSubImage2D
    while ( glGetError() != GL_NO_ERROR ) {}
    glTexSubImage2D( GL_TEXTURE_2D, 0, x, y, w, h, GL_RED, GL_UNSIGNED_BYTE, pixels );
    GLenum gl_err = glGetError();
    if ( gl_err != GL_NO_ERROR ) {
        std::fprintf( stderr, "VEFC backend test write_surface GL error 0x%x for %s\n", gl_err, name );
    }

    glBindTexture( GL_TEXTURE_2D, static_cast< GLuint >( previous_texture ) );
    glBindFramebuffer( GL_FRAMEBUFFER, static_cast< GLuint >( previous_fbo ) );
    return true;
}

static void vxui_demo_backend_test_execute_pipeline()
{
    extern vxui_demo_renderer* g_vxui_demo_backend_test_renderer;
    vxui_demo_renderer* renderer = g_vxui_demo_backend_test_renderer;
    if ( !renderer ) return;
    // Clear target surface, render drawlist, finish — mirrors VEFC demo's execute_pipeline.
    vxui_demo_clear_backend_test_surface( renderer, renderer->backend_target_texture,
        static_cast< int >( renderer->window_size.width ), static_cast< int >( renderer->window_size.height ) );
    renderer->backend_test_mode = true;
    vxui_demo_render_fontcache_drawlist( renderer, nullptr );
    renderer->backend_test_mode = false;
    // Backend conformance tests read back immediately after submission.
    glFinish();
}

static void vxui_demo_backend_test_execute_present()
{
    extern vxui_demo_renderer* g_vxui_demo_backend_test_renderer;
    vxui_demo_renderer* renderer = g_vxui_demo_backend_test_renderer;
    if ( !renderer ) {
        return;
    }
    // Copy target to present surface using two separate FBOs to avoid same-FBO blit issues
    static GLuint copy_read_fbo = 0;
    static GLuint copy_write_fbo = 0;
    if ( copy_read_fbo == 0 ) {
        glGenFramebuffers( 1, &copy_read_fbo );
        glGenFramebuffers( 1, &copy_write_fbo );
    }
    GLint previous_read_framebuffer = 0;
    GLint previous_draw_framebuffer = 0;
    glGetIntegerv( GL_READ_FRAMEBUFFER_BINDING, &previous_read_framebuffer );
    glGetIntegerv( GL_DRAW_FRAMEBUFFER_BINDING, &previous_draw_framebuffer );

    // Read from target texture
    glBindFramebuffer( GL_READ_FRAMEBUFFER, copy_read_fbo );
    glFramebufferTexture2D( GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderer->backend_target_texture, 0 );
    glReadBuffer( GL_COLOR_ATTACHMENT0 );

    // Draw to present texture
    glBindFramebuffer( GL_DRAW_FRAMEBUFFER, copy_write_fbo );
    glFramebufferTexture2D( GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderer->backend_present_texture, 0 );
    glDrawBuffer( GL_COLOR_ATTACHMENT0 );

    glBlitFramebuffer(
        0, 0,
        static_cast< GLint >( renderer->window_size.width ), static_cast< GLint >( renderer->window_size.height ),
        0, 0,
        static_cast< GLint >( renderer->window_size.width ), static_cast< GLint >( renderer->window_size.height ),
        GL_COLOR_BUFFER_BIT,
        GL_NEAREST );

    glBindFramebuffer( GL_READ_FRAMEBUFFER, static_cast< GLuint >( previous_read_framebuffer ) );
    glBindFramebuffer( GL_DRAW_FRAMEBUFFER, static_cast< GLuint >( previous_draw_framebuffer ) );
    // Backend tests require the copied present surface to be fully visible to readback.
    glFinish();
}

static void vxui_demo_backend_test_execute_frame()
{
    extern vxui_demo_renderer* g_vxui_demo_backend_test_renderer;
    vxui_demo_renderer* renderer = g_vxui_demo_backend_test_renderer;
    if ( !renderer ) {
        return;
    }
    vxui_demo_clear_backend_test_surfaces( renderer, renderer->cache.use_freetype ? false : true );
    ve_fontcache_flush_drawlist( &renderer->cache );
    vxui_demo_configure_snap_if_needed( renderer, false );

    const float sx = 1.0f / renderer->window_size.width;
    const float sy = 1.0f / renderer->window_size.height;

    // Draw text in all 4 test regions matching VEFC demo's backend_test_execute_frame.
    // Use available fonts with fallback to UI font for missing ones.
    const ve_font_id ui_font = VXUI_DEMO_FONT_UI;
    const ve_font_id title_font = ve_fontcache_is_valid_font_id( &renderer->cache, VXUI_DEMO_FONT_TITLE ) ? VXUI_DEMO_FONT_TITLE : ui_font;
    const ve_font_id cjk_font = ve_fontcache_is_valid_font_id( &renderer->cache, VXUI_DEMO_FONT_JAPANESE ) ? VXUI_DEMO_FONT_JAPANESE : ui_font;
    const ve_font_id arabic_font = ve_fontcache_is_valid_font_id( &renderer->cache, VXUI_DEMO_FONT_ARABIC ) ? VXUI_DEMO_FONT_ARABIC : ui_font;

    // Top-left region
    ve_fontcache_draw_text( &renderer->cache, cjk_font, u8"ゑ", 0.08f, 0.84f, sx, sy, false );
    ve_fontcache_draw_text( &renderer->cache, title_font, u8"VEFontCache Demo", 0.18f, 0.84f, sx, sy, false );
    ve_fontcache_draw_text( &renderer->cache, ui_font, u8"Backend conformance frame using real demo fonts and strings.", 0.08f, 0.78f, sx, sy, false );
    // Top-right region
    ve_fontcache_draw_text( &renderer->cache, cjk_font, u8"床前明月光 疑是地上霜", 0.58f, 0.78f, sx, sy, false );
    // Bottom-left region
    ve_fontcache_draw_text( &renderer->cache, arabic_font, u8"حب السماء لا تمطر غير الأحلام", 0.08f, 0.42f, sx, sy, false );
    ve_fontcache_draw_text( &renderer->cache, ui_font, u8"The quick brown fox jumps over the lazy dog.", 0.08f, 0.32f, sx, sy, false );
    // Bottom-right region
    ve_fontcache_draw_text( &renderer->cache, ui_font, u8"CODE CODE CODE 0123456789", 0.62f, 0.42f, sx, sy, false );
    ve_fontcache_draw_text( &renderer->cache, cjk_font, u8"漢字キャッシュ圧力", 0.62f, 0.26f, sx, sy, false );

    vxui_demo_backend_test_execute_pipeline();
    vxui_demo_backend_test_execute_present();
}

static void vxui_demo_backend_test_finalise_state()
{
    extern vxui_demo_renderer* g_vxui_demo_backend_test_renderer;
    vxui_demo_renderer* renderer = g_vxui_demo_backend_test_renderer;
    if ( !renderer ) {
        return;
    }
    ve_fontcache_reset_transient_test_state( &renderer->cache );
    vxui_demo_clear_backend_test_surfaces( renderer, true );
    // Backend teardown still relies on a fully quiesced GL state for follow-up checks.
    glFinish();
}

static ve_fontcache_backend_test_font_set vxui_demo_provision_fonts(
    ve_fontcache* target_cache,
    const ve_fontcache_backend_test_font_spec* roles,
    size_t role_count )
{
    extern vxui_demo_renderer* g_vxui_demo_backend_test_renderer;
    vxui_demo_renderer* renderer = g_vxui_demo_backend_test_renderer;
    ve_fontcache_backend_test_font_set font_set;
    if ( !renderer || target_cache != &renderer->cache ) {
        for ( size_t i = 0; i < role_count; ++i ) {
            font_set.mark_unavailable( roles[ i ].role, "backend test runner expected the active renderer cache" );
        }
        return font_set;
    }

    auto load_backend_test_font = [&]( ve_font_id& slot, const char* relative_path, float size_px ) -> ve_font_id {
        if ( slot >= 0 && ve_fontcache_is_valid_font_id( &renderer->cache, slot ) ) {
            return slot;
        }
        const std::filesystem::path source_dir = std::filesystem::path( VXUI_SOURCE_DIR );
        renderer->backend_test_dynamic_font_buffers.emplace_back();
        std::vector< uint8_t >& buffer = renderer->backend_test_dynamic_font_buffers.back();
        slot = ve_fontcache_loadfile(
            &renderer->cache,
            ( source_dir / relative_path ).string().c_str(),
            buffer,
            size_px );
        return slot;
    };

    const ve_font_id primary_font = load_backend_test_font( renderer->backend_test_primary_font, "vefc/demo/fonts/NotoSansJP-Light.otf", 19.0f );
    const ve_font_id secondary_font = load_backend_test_font( renderer->backend_test_secondary_font, "vefc/demo/fonts/OpenSans-Regular.ttf", 42.0f );
    const ve_font_id small_font = load_backend_test_font( renderer->backend_test_small_font, "vefc/demo/fonts/Roboto-Regular.ttf", 10.0f );
    const ve_font_id latin_font = load_backend_test_font( renderer->backend_test_latin_font, "vefc/demo/fonts/Bitter-Regular.ttf", 44.0f );
    const ve_font_id cjk_font = load_backend_test_font( renderer->backend_test_cjk_font, "vefc/demo/fonts/NotoSerifSC-Regular.otf", 54.0f );
    ve_font_id huge_font = load_backend_test_font( renderer->backend_test_huge_font, "vefc/demo/fonts/NotoSansJP-Light.otf", 200.0f );
    if ( huge_font < 0 && !renderer->cache.use_freetype ) {
        huge_font = load_backend_test_font( renderer->backend_test_huge_font, "vefc/demo/fonts/OpenSans-Regular.ttf", 200.0f );
    }

    const auto assign_role = [&]( ve_fontcache_backend_test_font_role role, ve_font_id id, const char* reason ) {
        if ( id >= 0 ) {
            font_set.set( role, id );
        } else {
            font_set.mark_unavailable( role, reason );
        }
    };

    assign_role(
        VE_FONTCACHE_BACKEND_TEST_FONT_ROLE_PRIMARY,
        primary_font >= 0 ? primary_font : VXUI_DEMO_FONT_JAPANESE,
        "primary font not available" );
    assign_role(
        VE_FONTCACHE_BACKEND_TEST_FONT_ROLE_SECONDARY,
        secondary_font >= 0 ? secondary_font : VXUI_DEMO_FONT_UI,
        "secondary font not available" );
    assign_role(
        VE_FONTCACHE_BACKEND_TEST_FONT_ROLE_SMALL,
        small_font >= 0 ? small_font : VXUI_DEMO_FONT_DEBUG,
        "small font not available" );
    assign_role(
        VE_FONTCACHE_BACKEND_TEST_FONT_ROLE_LATIN,
        latin_font >= 0 ? latin_font : ( secondary_font >= 0 ? secondary_font : VXUI_DEMO_FONT_UI ),
        "latin font not available" );
    assign_role(
        VE_FONTCACHE_BACKEND_TEST_FONT_ROLE_CJK,
        cjk_font >= 0 ? cjk_font : ( primary_font >= 0 ? primary_font : VXUI_DEMO_FONT_JAPANESE ),
        "cjk font not available" );
    assign_role(
        VE_FONTCACHE_BACKEND_TEST_FONT_ROLE_HUGE,
        huge_font >= 0 ? huge_font : ( primary_font >= 0 ? primary_font : VXUI_DEMO_FONT_UI ),
        "huge font not available" );
    assign_role( VE_FONTCACHE_BACKEND_TEST_FONT_ROLE_ARABIC, renderer->cache.use_freetype ? VXUI_DEMO_FONT_ARABIC : -1,
        renderer->cache.use_freetype ? "arabic font not available" : "Arabic role is only used in FreeType mode" );
    assign_role( VE_FONTCACHE_BACKEND_TEST_FONT_ROLE_HEBREW, -1, "Hebrew font not available in demo" );
    return font_set;
}

static ve_font_id vxui_demo_reload_font()
{
    extern vxui_demo_renderer* g_vxui_demo_backend_test_renderer;
    vxui_demo_renderer* renderer = g_vxui_demo_backend_test_renderer;
    if ( !renderer ) {
        return -1;
    }
    const std::filesystem::path source_dir = std::filesystem::path( VXUI_SOURCE_DIR );
    renderer->backend_test_dynamic_font_buffers.emplace_back();
    std::vector< uint8_t >& reload_buffer = renderer->backend_test_dynamic_font_buffers.back();
    const std::filesystem::path font_path = source_dir / ( renderer->cache.use_freetype
        ? "vefc/demo/fonts/NotoSansJP-Light.otf"
        : "vefc/demo/fonts/OpenSans-Regular.ttf" );
    return ve_fontcache_loadfile(
        &renderer->cache,
        font_path.string().c_str(),
        reload_buffer,
        19.0f );
}

static void vxui_demo_prepare_real_text()
{
    extern vxui_demo_renderer* g_vxui_demo_backend_test_renderer;
    vxui_demo_renderer* renderer = g_vxui_demo_backend_test_renderer;
    if ( !renderer ) {
        return;
    }
    const bool use_freetype = renderer->cache.use_freetype;
    ve_fontcache_shutdown( &renderer->cache );
    renderer->cache = ve_fontcache();
    renderer->backend_test_dynamic_font_buffers.clear();
    renderer->backend_test_primary_font = -1;
    renderer->backend_test_secondary_font = -1;
    renderer->backend_test_small_font = -1;
    renderer->backend_test_latin_font = -1;
    renderer->backend_test_cjk_font = -1;
    renderer->backend_test_huge_font = -1;
    ve_fontcache_init( &renderer->cache, use_freetype );
    renderer->snap_configured = false;
    vxui_demo_configure_snap_if_needed( renderer, true );
    vxui_demo_load_fonts( renderer );
    vxui_demo_clear_backend_test_surfaces( renderer, true );
}

static void vxui_demo_reinit_cache_for_backend_test( vxui_demo_renderer* renderer, bool use_freetype )
{
    ve_fontcache_shutdown( &renderer->cache );
    renderer->cache = ve_fontcache();
    renderer->backend_test_dynamic_font_buffers.clear();
    renderer->backend_test_primary_font = -1;
    renderer->backend_test_secondary_font = -1;
    renderer->backend_test_small_font = -1;
    renderer->backend_test_latin_font = -1;
    renderer->backend_test_cjk_font = -1;
    renderer->backend_test_huge_font = -1;
    ve_fontcache_init( &renderer->cache, use_freetype );
    renderer->snap_configured = false;
    vxui_demo_configure_snap_if_needed( renderer, true );
    vxui_demo_load_fonts( renderer );
    vxui_demo_clear_backend_test_surfaces( renderer, true );
}

using GLDEBUGPROC = void ( APIENTRYP )( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam );

#define GL_DEBUG_SOURCE_API             0x8246
#define GL_DEBUG_SOURCE_WINDOW_SYSTEM    0x8247
#define GL_DEBUG_SOURCE_SHADER_COMPILER 0x8248
#define GL_DEBUG_SOURCE_THIRD_PARTY     0x8249
#define GL_DEBUG_SOURCE_APPLICATION      0x824A
#define GL_DEBUG_SOURCE_OTHER           0x824B
#define GL_DEBUG_TYPE_ERROR             0x824C
#define GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR 0x824D
#define GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR 0x824E
#define GL_DEBUG_TYPE_PORTABILITY       0x824F
#define GL_DEBUG_TYPE_PERFORMANCE       0x8250
#define GL_DEBUG_TYPE_MARKER            0x8268
#define GL_DEBUG_TYPE_PUSH_GROUP        0x8269
#define GL_DEBUG_TYPE_POP_GROUP         0x826A
#define GL_DEBUG_TYPE_OTHER            0x8251
#define GL_DEBUG_SEVERITY_HIGH         0x9146
#define GL_DEBUG_SEVERITY_MEDIUM       0x9147
#define GL_DEBUG_SEVERITY_LOW          0x9148
#define GL_DEBUG_SEVERITY_NOTIFICATION 0x826B
#define GL_CONTEXT_FLAG_DEBUG_BIT      0x00000002
#define GL_DEBUG_OUTPUT                0x92E0
#define GL_PROGRAM                    0x82E2
#define GL_VERTEX_ARRAY               0x85B5

static void GLAPIENTRY vxui_demo_gl_message_callback( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam )
{
    ( void ) userParam;
    if ( severity != GL_DEBUG_SEVERITY_MEDIUM && severity != GL_DEBUG_SEVERITY_HIGH ) {
        return;
    }
    const char* source_str = "unknown";
    if ( source == GL_DEBUG_SOURCE_API ) source_str = "API";
    else if ( source == GL_DEBUG_SOURCE_WINDOW_SYSTEM ) source_str = "WINDOW_SYSTEM";
    else if ( source == GL_DEBUG_SOURCE_SHADER_COMPILER ) source_str = "SHADER_COMPILER";
    else if ( source == GL_DEBUG_SOURCE_THIRD_PARTY ) source_str = "THIRD_PARTY";
    else if ( source == GL_DEBUG_SOURCE_APPLICATION ) source_str = "APPLICATION";
    else if ( source == GL_DEBUG_SOURCE_OTHER ) source_str = "OTHER";

    const char* type_str = "unknown";
    if ( type == GL_DEBUG_TYPE_ERROR ) type_str = "ERROR";
    else if ( type == GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR ) type_str = "DEPRECATED_BEHAVIOR";
    else if ( type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR ) type_str = "UNDEFINED_BEHAVIOR";
    else if ( type == GL_DEBUG_TYPE_PORTABILITY ) type_str = "PORTABILITY";
    else if ( type == GL_DEBUG_TYPE_PERFORMANCE ) type_str = "PERFORMANCE";
    else if ( type == GL_DEBUG_TYPE_MARKER ) type_str = "MARKER";
    else if ( type == GL_DEBUG_TYPE_PUSH_GROUP ) type_str = "PUSH_GROUP";
    else if ( type == GL_DEBUG_TYPE_POP_GROUP ) type_str = "POP_GROUP";
    else if ( type == GL_DEBUG_TYPE_OTHER ) type_str = "OTHER";

    const char* severity_str = "unknown";
    if ( severity == GL_DEBUG_SEVERITY_HIGH ) severity_str = "HIGH";
    else if ( severity == GL_DEBUG_SEVERITY_MEDIUM ) severity_str = "MEDIUM";
    else if ( severity == GL_DEBUG_SEVERITY_LOW ) severity_str = "LOW";
    else if ( severity == GL_DEBUG_SEVERITY_NOTIFICATION ) severity_str = "NOTIFICATION";

    std::fprintf( stderr, "[GL %s] %s | %s | %u | %s: %.*s\n", severity_str, source_str, type_str, id, severity_str, length, message );
}

static void* vxui_demo_get_proc_address( const char* name )
{
#if defined( _WIN32 )
    void* proc = ( void* ) wglGetProcAddress( name );
    if ( !proc || proc == ( void* ) -1 || proc == ( void* ) 1 || proc == ( void* ) 2 || proc == ( void* ) 3 ) {
        static HMODULE opengl32 = GetModuleHandleW( L"opengl32.dll" );
        if ( opengl32 ) {
            proc = ( void* ) GetProcAddress( opengl32, name );
        }
    }
    return proc;
#else
    return nullptr;
#endif
}

#define VXUI_DEMO_LOAD_PROC( type, name, var ) \
    do { \
        var = ( type ) vxui_demo_get_proc_address( name ); \
    } while ( 0 )

void vxui_demo_gl_debug_init( vxui_demo_renderer* renderer )
{
    renderer->gl_debug.backend = vxui_demo_renderer::vxui_demo_gl_debug::backend_none;
    renderer->gl_debug.debug_context = false;
    renderer->gl_debug.callback_installed = false;
    renderer->gl_debug.glPushDebugGroup = nullptr;
    renderer->gl_debug.glPopDebugGroup = nullptr;
    renderer->gl_debug.glObjectLabel = nullptr;
    renderer->gl_debug.glDebugMessageCallback = nullptr;
    renderer->gl_debug.glDebugMessageControl = nullptr;
    renderer->gl_debug.glPushGroupMarkerEXT = nullptr;
    renderer->gl_debug.glPopGroupMarkerEXT = nullptr;
    renderer->gl_debug.glInsertEventMarkerEXT = nullptr;

    GLint context_flags = 0;
    glGetIntegerv( GL_CONTEXT_FLAGS, &context_flags );
    renderer->gl_debug.debug_context = ( context_flags & GL_CONTEXT_FLAG_DEBUG_BIT ) != 0;

    VXUI_DEMO_LOAD_PROC( vxui_demo_renderer::vxui_demo_gl_debug::fn_PushDebugGroup, "glPushDebugGroup", renderer->gl_debug.glPushDebugGroup );
    VXUI_DEMO_LOAD_PROC( vxui_demo_renderer::vxui_demo_gl_debug::fn_PopDebugGroup, "glPopDebugGroup", renderer->gl_debug.glPopDebugGroup );
    VXUI_DEMO_LOAD_PROC( vxui_demo_renderer::vxui_demo_gl_debug::fn_ObjectLabel, "glObjectLabel", renderer->gl_debug.glObjectLabel );
    VXUI_DEMO_LOAD_PROC( vxui_demo_renderer::vxui_demo_gl_debug::fn_DebugMessageCallback, "glDebugMessageCallback", renderer->gl_debug.glDebugMessageCallback );
    VXUI_DEMO_LOAD_PROC( vxui_demo_renderer::vxui_demo_gl_debug::fn_DebugMessageControl, "glDebugMessageControl", renderer->gl_debug.glDebugMessageControl );

    if ( renderer->gl_debug.glPushDebugGroup && renderer->gl_debug.glPopDebugGroup && renderer->gl_debug.glObjectLabel ) {
        renderer->gl_debug.backend = vxui_demo_renderer::vxui_demo_gl_debug::backend_khr;
        std::printf( "vxui demo markers: KHR_debug\n" );
    } else {
        VXUI_DEMO_LOAD_PROC( vxui_demo_renderer::vxui_demo_gl_debug::fn_PushGroupMarkerEXT, "glPushGroupMarkerEXT", renderer->gl_debug.glPushGroupMarkerEXT );
        VXUI_DEMO_LOAD_PROC( vxui_demo_renderer::vxui_demo_gl_debug::fn_PopGroupMarkerEXT, "glPopGroupMarkerEXT", renderer->gl_debug.glPopGroupMarkerEXT );
        VXUI_DEMO_LOAD_PROC( vxui_demo_renderer::vxui_demo_gl_debug::fn_InsertEventMarkerEXT, "glInsertEventMarkerEXT", renderer->gl_debug.glInsertEventMarkerEXT );

        if ( renderer->gl_debug.glPushGroupMarkerEXT && renderer->gl_debug.glPopGroupMarkerEXT && renderer->gl_debug.glInsertEventMarkerEXT ) {
            renderer->gl_debug.backend = vxui_demo_renderer::vxui_demo_gl_debug::backend_ext;
            std::printf( "vxui demo markers: EXT_debug_marker\n" );
        } else {
            std::printf( "vxui demo markers: unavailable\n" );
        }
    }

    if ( renderer->gl_debug.backend == vxui_demo_renderer::vxui_demo_gl_debug::backend_khr && renderer->gl_debug.glDebugMessageCallback ) {
        if ( renderer->gl_debug.debug_context || renderer->gl_debug.glDebugMessageControl ) {
            if ( renderer->gl_debug.glDebugMessageControl ) {
                renderer->gl_debug.glDebugMessageControl( GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_FALSE );
                renderer->gl_debug.glDebugMessageControl( GL_DEBUG_SOURCE_THIRD_PARTY, GL_DEBUG_TYPE_OTHER, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE );
            }
            renderer->gl_debug.glDebugMessageCallback( vxui_demo_gl_message_callback, nullptr );
            glEnable( GL_DEBUG_OUTPUT );
            renderer->gl_debug.callback_installed = true;
        }
    }
}

void vxui_demo_gl_debug_label( vxui_demo_renderer* renderer, GLenum identifier, GLuint name, const char* label )
{
    if ( !renderer || !label || name == 0 ) {
        return;
    }
    if ( renderer->gl_debug.backend == vxui_demo_renderer::vxui_demo_gl_debug::backend_khr && renderer->gl_debug.glObjectLabel ) {
        renderer->gl_debug.glObjectLabel( identifier, name, -1, label );
    }
}

void vxui_demo_gl_debug_begin( vxui_demo_renderer* renderer, const char* label )
{
    if ( !renderer || !label ) {
        return;
    }
    if ( renderer->gl_debug.backend == vxui_demo_renderer::vxui_demo_gl_debug::backend_khr && renderer->gl_debug.glPushDebugGroup ) {
        renderer->gl_debug.glPushDebugGroup( GL_DEBUG_SOURCE_APPLICATION, 0, -1, label );
    } else if ( renderer->gl_debug.backend == vxui_demo_renderer::vxui_demo_gl_debug::backend_ext && renderer->gl_debug.glPushGroupMarkerEXT ) {
        renderer->gl_debug.glPushGroupMarkerEXT( -1, label );
    }
}

void vxui_demo_gl_debug_end( vxui_demo_renderer* renderer )
{
    if ( !renderer ) {
        return;
    }
    if ( renderer->gl_debug.backend == vxui_demo_renderer::vxui_demo_gl_debug::backend_khr && renderer->gl_debug.glPopDebugGroup ) {
        renderer->gl_debug.glPopDebugGroup();
    } else if ( renderer->gl_debug.backend == vxui_demo_renderer::vxui_demo_gl_debug::backend_ext && renderer->gl_debug.glPopGroupMarkerEXT ) {
        renderer->gl_debug.glPopGroupMarkerEXT();
    }
}

void vxui_demo_gl_debug_event( vxui_demo_renderer* renderer, const char* label )
{
    if ( !renderer || !label ) {
        return;
    }
    if ( renderer->gl_debug.backend == vxui_demo_renderer::vxui_demo_gl_debug::backend_khr && renderer->gl_debug.glPushDebugGroup ) {
        renderer->gl_debug.glPushDebugGroup( GL_DEBUG_SOURCE_APPLICATION, 0, -1, label );
        renderer->gl_debug.glPopDebugGroup();
    } else if ( renderer->gl_debug.backend == vxui_demo_renderer::vxui_demo_gl_debug::backend_ext && renderer->gl_debug.glInsertEventMarkerEXT ) {
        renderer->gl_debug.glInsertEventMarkerEXT( -1, label );
    }
}

static void vxui_demo_set_window_size( vxui_demo_renderer* renderer, TinyWindow::vec2_t< unsigned int > window_size )
{
    if ( !renderer ) {
        return;
    }
    if ( window_size.width == 0 ) {
        window_size.width = 1;
    }
    if ( window_size.height == 0 ) {
        window_size.height = 1;
    }

    const bool changed =
        renderer->window_size.width != window_size.width ||
        renderer->window_size.height != window_size.height;
    renderer->window_size = window_size;
    if ( changed ) {
        vxui_demo_configure_snap_if_needed( renderer, false );
    }
}

static void vxui_demo_configure_snap_if_needed( vxui_demo_renderer* renderer, bool force )
{
    if ( !renderer ) {
        return;
    }
    if ( renderer->window_size.width == 0 ) {
        renderer->window_size.width = 1;
    }
    if ( renderer->window_size.height == 0 ) {
        renderer->window_size.height = 1;
    }

    if ( !force &&
         renderer->snap_configured &&
         renderer->snap_configured_window_size.width == renderer->window_size.width &&
         renderer->snap_configured_window_size.height == renderer->window_size.height ) {
        return;
    }

    ve_fontcache_configure_snap( &renderer->cache, renderer->window_size.width, renderer->window_size.height );
    renderer->snap_configured_window_size = renderer->window_size;
    renderer->snap_configured = true;
}

static bool vxui_demo_cache_shader_uniforms( vxui_demo_renderer* renderer )
{
    if ( !renderer ) {
        return false;
    }

    auto fetch_uniform = []( GLuint program, const char* program_name, const char* uniform_name, GLint* out_location ) -> bool
    {
        *out_location = glGetUniformLocation( program, uniform_name );
        if ( *out_location >= 0 ) {
            return true;
        }
        std::fprintf( stderr, "Failed to resolve uniform '%s' for program '%s'.\n", uniform_name, program_name );
        return false;
    };

    bool ok = true;
    ok &= fetch_uniform( renderer->primitive_program, "primitive", "u_bounds", &renderer->uniforms.primitive.bounds );
    ok &= fetch_uniform( renderer->primitive_program, "primitive", "u_viewport", &renderer->uniforms.primitive.viewport );
    ok &= fetch_uniform( renderer->primitive_program, "primitive", "u_size", &renderer->uniforms.primitive.size );
    ok &= fetch_uniform( renderer->primitive_program, "primitive", "u_color", &renderer->uniforms.primitive.color );
    ok &= fetch_uniform( renderer->primitive_program, "primitive", "u_radius", &renderer->uniforms.primitive.radius );
    ok &= fetch_uniform( renderer->primitive_program, "primitive", "u_border", &renderer->uniforms.primitive.border );
    ok &= fetch_uniform( renderer->fontcache_shader_blit_atlas, "fontcache-blit-atlas", "src_texture", &renderer->uniforms.blit_atlas.src_texture );
    ok &= fetch_uniform( renderer->fontcache_shader_blit_atlas, "fontcache-blit-atlas", "region", &renderer->uniforms.blit_atlas.region );
    ok &= fetch_uniform( renderer->fontcache_shader_draw_text, "fontcache-draw-text", "src_texture", &renderer->uniforms.draw_text.src_texture );
    ok &= fetch_uniform( renderer->fontcache_shader_draw_text, "fontcache-draw-text", "downsample", &renderer->uniforms.draw_text.downsample );
    ok &= fetch_uniform( renderer->fontcache_shader_draw_text, "fontcache-draw-text", "colour", &renderer->uniforms.draw_text.colour );
    if ( !ok ) {
        return false;
    }

    glUseProgram( renderer->fontcache_shader_blit_atlas );
    glUniform1i( renderer->uniforms.blit_atlas.src_texture, 0 );
    glUseProgram( renderer->fontcache_shader_draw_text );
    glUniform1i( renderer->uniforms.draw_text.src_texture, 0 );
    glUseProgram( 0 );
    return true;
}

static bool vxui_demo_init_renderer( vxui_demo_renderer* renderer )
{
    static const char* kPrimitiveVs = R"(#version 330 core
out vec2 local_pos;
uniform vec4 u_bounds;
uniform vec2 u_viewport;
const vec2 kVerts[6] = vec2[6](
    vec2(0.0, 0.0),
    vec2(1.0, 0.0),
    vec2(1.0, 1.0),
    vec2(0.0, 0.0),
    vec2(1.0, 1.0),
    vec2(0.0, 1.0)
);
void main()
{
    vec2 unit = kVerts[ gl_VertexID ];
    vec2 pixel = u_bounds.xy + unit * u_bounds.zw;
    vec2 ndc = vec2(
        pixel.x / u_viewport.x * 2.0 - 1.0,
        1.0 - pixel.y / u_viewport.y * 2.0
    );
    local_pos = unit * u_bounds.zw;
    gl_Position = vec4( ndc, 0.0, 1.0 );
})";
    static const char* kPrimitiveFs = R"(#version 330 core
in vec2 local_pos;
out vec4 fragc;
uniform vec2 u_size;
uniform vec4 u_color;
uniform float u_radius;
uniform float u_border;
float rounded_box_sdf( vec2 p, vec2 half_extents, float radius )
{
    vec2 q = abs( p ) - ( half_extents - vec2( radius ) );
    return length( max( q, 0.0 ) ) + min( max( q.x, q.y ), 0.0 ) - radius;
}
void main()
{
    vec2 half_extents = max( u_size * 0.5, vec2( 0.0 ) );
    float radius = min( u_radius, min( half_extents.x, half_extents.y ) );
    vec2 p = local_pos - half_extents;
    float outer = rounded_box_sdf( p, half_extents, radius );
    float outer_alpha = 1.0 - smoothstep( 0.0, 1.0, outer );
    float alpha = outer_alpha;
    if ( u_border > 0.0 ) {
        vec2 inner_half = max( half_extents - vec2( u_border ), vec2( 0.0 ) );
        float inner_radius = max( radius - u_border, 0.0 );
        float inner = rounded_box_sdf( p, inner_half, inner_radius );
        float inner_alpha = 1.0 - smoothstep( 0.0, 1.0, inner );
        alpha = max( outer_alpha - inner_alpha, 0.0 );
    }
    fragc = vec4( u_color.rgb, u_color.a * alpha );
})";
    static const char* kFontcacheVsShared = R"(#version 330 core
in vec2 vpos;
in vec2 vtex;
out vec2 uv;
void main( void )
{
    uv = vtex;
    gl_Position = vec4( vpos.xy, 0.0, 1.0 );
})";
    static const char* kFontcacheFsRenderGlyph = R"(#version 330 core
out vec4 fragc;
void main( void )
{
    fragc = vec4( 1.0, 1.0, 1.0, 1.0 );
})";
    static const char* kFontcacheFsBlitAtlas = R"(#version 330 core
in vec2 uv;
out vec4 fragc;
uniform uint region;
uniform sampler2D src_texture;
float downsample( vec2 uv, vec2 texsz )
{
    float v =
        texture( src_texture, uv + vec2( 0.0f, 0.0f ) * texsz ).x * 0.25f +
        texture( src_texture, uv + vec2( 0.0f, 1.0f ) * texsz ).x * 0.25f +
        texture( src_texture, uv + vec2( 1.0f, 0.0f ) * texsz ).x * 0.25f +
        texture( src_texture, uv + vec2( 1.0f, 1.0f ) * texsz ).x * 0.25f;
    return v;
}
void main( void )
{
    const vec2 texsz = 1.0f / vec2( 2048, 512 );
    if ( region == 0u || region == 1u || region == 2u ) {
        float v =
            downsample( uv + vec2( -1.5f, -1.5f ) * texsz, texsz ) * 0.25f +
            downsample( uv + vec2(  0.5f, -1.5f ) * texsz, texsz ) * 0.25f +
            downsample( uv + vec2( -1.5f,  0.5f ) * texsz, texsz ) * 0.25f +
            downsample( uv + vec2(  0.5f,  0.5f ) * texsz, texsz ) * 0.25f;
        fragc = vec4( 1, 1, 1, v );
    } else {
        fragc = vec4( 0, 0, 0, 1 );
    }
})";
    static const char* kFontcacheVsDrawText = R"(#version 330 core
in vec2 vpos;
in vec2 vtex;
out vec2 uv;
void main( void )
{
    uv = vtex;
    gl_Position = vec4( vpos.xy * 2.0f - 1.0f, 0.0, 1.0 );
})";
    static const char* kFontcacheFsDrawText = R"(#version 330 core
in vec2 uv;
out vec4 fragc;
uniform sampler2D src_texture;
uniform uint downsample;
uniform vec4 colour;
void main( void )
{
    float v = texture( src_texture, uv ).x;
    if ( downsample == 1u ) {
        const vec2 texsz = 1.0f / vec2( 2048, 512 );
        v =
            texture( src_texture, uv + vec2( -0.5f, -0.5f ) * texsz ).x * 0.25f +
            texture( src_texture, uv + vec2( -0.5f,  0.5f ) * texsz ).x * 0.25f +
            texture( src_texture, uv + vec2(  0.5f, -0.5f ) * texsz ).x * 0.25f +
            texture( src_texture, uv + vec2(  0.5f,  0.5f ) * texsz ).x * 0.25f;
    }
    fragc = vec4( colour.xyz, colour.a * v );
})";

    renderer->primitive_program = vxui_demo_compile_program( kPrimitiveVs, kPrimitiveFs, "primitive" );
    renderer->fontcache_shader_render_glyph = vxui_demo_compile_program( kFontcacheVsShared, kFontcacheFsRenderGlyph, "fontcache-render-glyph" );
    renderer->fontcache_shader_blit_atlas = vxui_demo_compile_program( kFontcacheVsShared, kFontcacheFsBlitAtlas, "fontcache-blit-atlas" );
    renderer->fontcache_shader_draw_text = vxui_demo_compile_program( kFontcacheVsDrawText, kFontcacheFsDrawText, "fontcache-draw-text" );
    if ( !renderer->primitive_program || !renderer->fontcache_shader_render_glyph || !renderer->fontcache_shader_blit_atlas || !renderer->fontcache_shader_draw_text ) {
        return false;
    }
    if ( !vxui_demo_cache_shader_uniforms( renderer ) ) {
        return false;
    }

    glGenVertexArrays( 1, &renderer->vao );
    glBindVertexArray( renderer->vao );
    glGenBuffers( 1, &renderer->text_vbo );
    glGenBuffers( 1, &renderer->text_ibo );
    vxui_demo_setup_fontcache_fbo( renderer );
    glEnable( GL_BLEND );
    glBlendEquation( GL_FUNC_ADD );

    vxui_demo_gl_debug_init( renderer );

    vxui_demo_gl_debug_label( renderer, GL_PROGRAM, renderer->primitive_program, "vxui.primitive_program" );
    vxui_demo_gl_debug_label( renderer, GL_PROGRAM, renderer->fontcache_shader_render_glyph, "vxui.fontcache.render_glyph_program" );
    vxui_demo_gl_debug_label( renderer, GL_PROGRAM, renderer->fontcache_shader_blit_atlas, "vxui.fontcache.blit_atlas_program" );
    vxui_demo_gl_debug_label( renderer, GL_PROGRAM, renderer->fontcache_shader_draw_text, "vxui.fontcache.draw_text_program" );
    vxui_demo_gl_debug_label( renderer, GL_VERTEX_ARRAY, renderer->vao, "vxui.vao" );
    vxui_demo_gl_debug_label( renderer, GL_FRAMEBUFFER, renderer->fontcache_fbo[ 0 ], "vxui.fontcache.glyph_fbo" );
    vxui_demo_gl_debug_label( renderer, GL_FRAMEBUFFER, renderer->fontcache_fbo[ 1 ], "vxui.fontcache.atlas_fbo" );
    vxui_demo_gl_debug_label( renderer, GL_TEXTURE, renderer->fontcache_fbo_texture[ 0 ], "vxui.fontcache.glyph_fbo_texture" );
    vxui_demo_gl_debug_label( renderer, GL_TEXTURE, renderer->fontcache_fbo_texture[ 1 ], "vxui.fontcache.atlas_fbo_texture" );

    return true;
}

static void vxui_demo_shutdown_renderer( vxui_demo_renderer* renderer )
{
    if ( !renderer ) {
        return;
    }
    if ( renderer->primitive_program ) glDeleteProgram( renderer->primitive_program );
    if ( renderer->fontcache_shader_render_glyph ) glDeleteProgram( renderer->fontcache_shader_render_glyph );
    if ( renderer->fontcache_shader_blit_atlas ) glDeleteProgram( renderer->fontcache_shader_blit_atlas );
    if ( renderer->fontcache_shader_draw_text ) glDeleteProgram( renderer->fontcache_shader_draw_text );
    if ( renderer->text_vbo ) glDeleteBuffers( 1, &renderer->text_vbo );
    if ( renderer->text_ibo ) glDeleteBuffers( 1, &renderer->text_ibo );
    if ( renderer->vao ) glDeleteVertexArrays( 1, &renderer->vao );
    if ( renderer->fontcache_fbo[ 0 ] || renderer->fontcache_fbo[ 1 ] ) glDeleteFramebuffers( 2, renderer->fontcache_fbo );
    if ( renderer->fontcache_fbo_texture[ 0 ] || renderer->fontcache_fbo_texture[ 1 ] ) glDeleteTextures( 2, renderer->fontcache_fbo_texture );
    if ( !renderer->cpu_atlas_textures.empty() ) {
        glDeleteTextures( ( GLsizei ) renderer->cpu_atlas_textures.size(), renderer->cpu_atlas_textures.data() );
    }
    vxui_demo_shutdown_shot_capture_fbo( renderer );
    vxui_demo_shutdown_backend_test_fbo( renderer );
    ve_fontcache_shutdown( &renderer->cache );
}

static bool vxui_demo_load_fonts( vxui_demo_renderer* renderer )
{
    struct font_load
    {
        const char* relative_path;
        vxui_demo_font_id font_id;
        float size_px;
    };
    const std::filesystem::path source_dir = std::filesystem::path( VXUI_SOURCE_DIR );
    const font_load fonts[] = {
        { "vefc/demo/fonts/OpenSans-Regular.ttf", VXUI_DEMO_FONT_UI, ( float ) VXUI_DEMO_BODY_SIZE },
        { "vefc/demo/fonts/Bitter-Regular.ttf", VXUI_DEMO_FONT_TITLE, ( float ) VXUI_DEMO_TITLE_SIZE },
        { "vefc/demo/fonts/NovaMono-Regular.ttf", VXUI_DEMO_FONT_DEBUG, ( float ) VXUI_DEMO_CODE_SIZE },
        { "vefc/demo/fonts/NotoSansJP-Regular.otf", VXUI_DEMO_FONT_JAPANESE, ( float ) VXUI_DEMO_BODY_SIZE },
        { "vefc/demo/fonts/Tajawal-Regular.ttf", VXUI_DEMO_FONT_ARABIC, ( float ) VXUI_DEMO_BODY_SIZE },
        { "vefc/demo/fonts/Bitter-Regular.ttf", VXUI_DEMO_FONT_SECTION_TITLE, ( float ) VXUI_DEMO_SECTION_SIZE },
        { "vefc/demo/fonts/NotoSansJP-Regular.otf", VXUI_DEMO_FONT_JAPANESE_TITLE, ( float ) VXUI_DEMO_TITLE_SIZE },
        { "vefc/demo/fonts/NotoSansJP-Regular.otf", VXUI_DEMO_FONT_JAPANESE_SECTION, ( float ) VXUI_DEMO_SECTION_SIZE },
        { "vefc/demo/fonts/Tajawal-Regular.ttf", VXUI_DEMO_FONT_ARABIC_TITLE, ( float ) VXUI_DEMO_TITLE_SIZE },
        { "vefc/demo/fonts/Tajawal-Regular.ttf", VXUI_DEMO_FONT_ARABIC_SECTION, ( float ) VXUI_DEMO_SECTION_SIZE },
    };

    for ( const font_load& font : fonts ) {
        const std::filesystem::path full_path = source_dir / font.relative_path;
        std::vector< uint8_t >& buffer = renderer->demo_fonts[ font.font_id ];
        ve_font_id id = ve_fontcache_loadfile( &renderer->cache, full_path.string().c_str(), buffer, font.size_px );
        if ( id != font.font_id ) {
            std::fprintf( stderr, "Failed to load demo font '%s' with the expected id %d (got %lld).\n", full_path.string().c_str(), font.font_id, ( long long ) id );
            return false;
        }
        renderer->demo_line_heights[ font.font_id ] = vxui_demo_font_line_height( renderer, id, font.size_px );
    }
    return true;
}

static void vxui_demo_draw_primitive( vxui_demo_renderer* renderer, vxui_rect bounds, vxui_color color, float radius, float border_width )
{
    glUseProgram( renderer->primitive_program );
    glBindVertexArray( renderer->vao );
    glUniform4f( renderer->uniforms.primitive.bounds, bounds.x, bounds.y, bounds.w, bounds.h );
    glUniform2f( renderer->uniforms.primitive.viewport, ( float ) renderer->window_size.width, ( float ) renderer->window_size.height );
    glUniform2f( renderer->uniforms.primitive.size, bounds.w, bounds.h );
    glUniform4f(
        renderer->uniforms.primitive.color,
        color.r / 255.0f,
        color.g / 255.0f,
        color.b / 255.0f,
        color.a / 255.0f );
    glUniform1f( renderer->uniforms.primitive.radius, radius );
    glUniform1f( renderer->uniforms.primitive.border, border_width );
    glDrawArrays( GL_TRIANGLES, 0, 6 );
}

static void vxui_demo_draw_placeholder_image( vxui_demo_renderer* renderer, vxui_rect bounds )
{
    vxui_demo_draw_primitive( renderer, bounds, ( vxui_color ) { 40, 48, 68, 255 }, 6.0f, 0.0f );
    vxui_demo_draw_primitive( renderer, bounds, ( vxui_color ) { 255, 128, 200, 255 }, 6.0f, 2.0f );
}

static void vxui_demo_ensure_fontcache_upload_buffers( vxui_demo_renderer* renderer, size_t vertex_bytes, size_t index_bytes )
{
    if ( !renderer ) {
        return;
    }

    auto grow_capacity = []( size_t current, size_t required ) {
        size_t capacity = current ? current : 4096u;
        while ( capacity < required ) {
            capacity *= 2u;
        }
        return capacity;
    };

    if ( vertex_bytes > renderer->text_vbo_capacity ) {
        renderer->text_vbo_capacity = grow_capacity( renderer->text_vbo_capacity, vertex_bytes );
        glBindBuffer( GL_ARRAY_BUFFER, renderer->text_vbo );
        glBufferData( GL_ARRAY_BUFFER, ( GLsizeiptr ) renderer->text_vbo_capacity, nullptr, GL_DYNAMIC_DRAW );
    }

    if ( index_bytes > renderer->text_ibo_capacity ) {
        renderer->text_ibo_capacity = grow_capacity( renderer->text_ibo_capacity, index_bytes );
        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, renderer->text_ibo );
        glBufferData( GL_ELEMENT_ARRAY_BUFFER, ( GLsizeiptr ) renderer->text_ibo_capacity, nullptr, GL_DYNAMIC_DRAW );
    }
}

static void vxui_demo_render_fontcache_drawlist( vxui_demo_renderer* renderer, const vxui_rect* clip_rect )
{
    ve_fontcache_drawlist* drawlist = ve_fontcache_get_drawlist( &renderer->cache );
    if ( !drawlist || drawlist->dcalls.empty() ) {
        ve_fontcache_flush_drawlist( &renderer->cache );
        return;
    }

    ve_fontcache_optimise_drawlist( &renderer->cache );

    const size_t vertex_bytes = drawlist->vertices.size() * sizeof( ve_fontcache_vertex );
    const size_t index_bytes = drawlist->indices.size() * sizeof( uint32_t );
    if ( !drawlist->vertices.empty() && !drawlist->indices.empty() ) {
        vxui_demo_ensure_fontcache_upload_buffers( renderer, vertex_bytes, index_bytes );
        glBindVertexArray( renderer->vao );
        glBindBuffer( GL_ARRAY_BUFFER, renderer->text_vbo );
        glBufferSubData( GL_ARRAY_BUFFER, 0, ( GLsizeiptr ) vertex_bytes, drawlist->vertices.data() );
        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, renderer->text_ibo );
        glBufferSubData( GL_ELEMENT_ARRAY_BUFFER, 0, ( GLsizeiptr ) index_bytes, drawlist->indices.data() );
    }
    glEnableVertexAttribArray( 0 );
    glEnableVertexAttribArray( 1 );
    glVertexAttribPointer( 0, 2, GL_FLOAT, false, sizeof( ve_fontcache_vertex ), nullptr );
    glVertexAttribPointer( 1, 2, GL_FLOAT, false, sizeof( ve_fontcache_vertex ), ( GLvoid* ) ( 2 * sizeof( float ) ) );

    glDisable( GL_CULL_FACE );
    glEnable( GL_BLEND );

    int current_pass_group = -1;
    auto end_pass_group = [&]()
    {
        if ( current_pass_group >= 0 )
        {
            vxui_demo_gl_debug_end( renderer );
            current_pass_group = -1;
        }
    };
    auto begin_pass_group = [&]( int new_group, const char* name )
    {
        if ( current_pass_group != new_group )
        {
            end_pass_group();
            vxui_demo_gl_debug_begin( renderer, name );
            current_pass_group = new_group;
        }
    };

    for ( ve_fontcache_draw& dcall : drawlist->dcalls ) {
        if ( dcall.pass == VE_FONTCACHE_FRAMEBUFFER_PASS_GLYPH ) {
            begin_pass_group( 0, "VEFC Glyph Pass" );
            glUseProgram( renderer->fontcache_shader_render_glyph );
            glBindFramebuffer( GL_FRAMEBUFFER, renderer->fontcache_fbo[ 0 ] );
            glBlendFunc( GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_COLOR );
            glViewport( 0, 0, VE_FONTCACHE_GLYPHDRAW_BUFFER_WIDTH, VE_FONTCACHE_GLYPHDRAW_BUFFER_HEIGHT );
            glEnable( GL_SCISSOR_TEST );
            glScissor( 0, 0, VE_FONTCACHE_GLYPHDRAW_BUFFER_WIDTH, VE_FONTCACHE_GLYPHDRAW_BUFFER_HEIGHT );
            glDisable( GL_FRAMEBUFFER_SRGB );
        } else if ( dcall.pass == VE_FONTCACHE_FRAMEBUFFER_PASS_ATLAS ) {
            begin_pass_group( 1, "VEFC Atlas Blit" );
            glUseProgram( renderer->fontcache_shader_blit_atlas );
            glBindFramebuffer( GL_FRAMEBUFFER, renderer->fontcache_fbo[ 1 ] );
            glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
            glViewport( 0, 0, VE_FONTCACHE_ATLAS_WIDTH, VE_FONTCACHE_ATLAS_HEIGHT );
            glEnable( GL_SCISSOR_TEST );
            glScissor( 0, 0, VE_FONTCACHE_ATLAS_WIDTH, VE_FONTCACHE_ATLAS_HEIGHT );
            glUniform1ui( renderer->uniforms.blit_atlas.region, dcall.region );
            glActiveTexture( GL_TEXTURE0 );
            glBindTexture( GL_TEXTURE_2D, renderer->fontcache_fbo_texture[ 0 ] );
            glDisable( GL_FRAMEBUFFER_SRGB );
        } else if ( dcall.pass == VE_FONTCACHE_FRAMEBUFFER_PASS_TARGET || dcall.pass == VE_FONTCACHE_FRAMEBUFFER_PASS_TARGET_UNCACHED || dcall.pass == VE_FONTCACHE_FRAMEBUFFER_PASS_TARGET_CPU_CACHED ) {
            begin_pass_group( 2, "VEFC Target Text" );
            glUseProgram( renderer->fontcache_shader_draw_text );
            GLuint target_fbo = renderer->backend_test_mode ? renderer->backend_target_fbo : ( renderer->shot_capture_mode ? renderer->shot_capture_fbo : 0u );
            glBindFramebuffer( GL_FRAMEBUFFER, target_fbo );
            glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
            glViewport( 0, 0, renderer->window_size.width, renderer->window_size.height );
            if ( clip_rect && clip_rect->w > 0.0f && clip_rect->h > 0.0f ) {
                GLint x = ( GLint ) std::lround( clip_rect->x );
                GLint y = ( GLint ) std::lround( ( float ) renderer->window_size.height - clip_rect->y - clip_rect->h );
                GLsizei w = ( GLsizei ) std::lround( clip_rect->w );
                GLsizei h = ( GLsizei ) std::lround( clip_rect->h );
                glEnable( GL_SCISSOR_TEST );
                glScissor( x, y, w < 0 ? 0 : w, h < 0 ? 0 : h );
            } else {
                glDisable( GL_SCISSOR_TEST );
            }
            GLuint src_tex = 0;
#ifdef VE_FONTCACHE_FREETYPE_RASTERISATION
            if ( dcall.pass == VE_FONTCACHE_FRAMEBUFFER_PASS_TARGET_CPU_CACHED ) {
                if ( dcall.atlas_page >= ( uint32_t ) renderer->cpu_atlas_textures.size() ) {
                    std::fprintf( stderr, "vxui_demo: TARGET_CPU_CACHED draw arrived but cpu_atlas_textures[%u] not created\n", dcall.atlas_page );
                } else {
                    glUniform1ui( renderer->uniforms.draw_text.downsample, 0 );
                    glActiveTexture( GL_TEXTURE0 );
                    glBindTexture( GL_TEXTURE_2D, renderer->cpu_atlas_textures[ dcall.atlas_page ] );
                    src_tex = renderer->cpu_atlas_textures[ dcall.atlas_page ];
                }
            } else
#endif
            {
                glUniform1ui( renderer->uniforms.draw_text.downsample, dcall.pass == VE_FONTCACHE_FRAMEBUFFER_PASS_TARGET_UNCACHED ? 1u : 0u );
                glActiveTexture( GL_TEXTURE0 );
                src_tex = dcall.pass == VE_FONTCACHE_FRAMEBUFFER_PASS_TARGET_UNCACHED ? renderer->fontcache_fbo_texture[ 0 ] : renderer->fontcache_fbo_texture[ 1 ];
                glBindTexture( GL_TEXTURE_2D, src_tex );
            }
            glUniform4fv( renderer->uniforms.draw_text.colour, 1, dcall.colour );
#ifdef VE_FONTCACHE_FREETYPE_RASTERISATION
        } else if ( dcall.pass == VE_FONTCACHE_FRAMEBUFFER_PASS_ATLAS_PAGE_TEXTURE_CREATE ) {
            begin_pass_group( 3, "VEFC CPU Atlas Create" );
            renderer->cpu_atlas_textures.resize( std::max( ( int ) renderer->cpu_atlas_textures.size(), ( int ) dcall.atlas_page + 1 ) );
            glGenTextures( 1, &renderer->cpu_atlas_textures[ dcall.atlas_page ] );
            glActiveTexture( GL_TEXTURE0 );
            glBindTexture( GL_TEXTURE_2D, renderer->cpu_atlas_textures[ dcall.atlas_page ] );
            glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
            glPixelStorei( GL_PACK_ALIGNMENT, 1 );
            glTexImage2D( GL_TEXTURE_2D, 0, GL_R8, VE_FONTCACHE_CPU_ATLAS_PAGE_SIZE, VE_FONTCACHE_CPU_ATLAS_PAGE_SIZE, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

            char tex_label[ 64 ];
            std::snprintf( tex_label, sizeof( tex_label ), "vxui.fontcache.cpu_atlas_page_%u", dcall.atlas_page );
            vxui_demo_gl_debug_label( renderer, GL_TEXTURE, renderer->cpu_atlas_textures[ dcall.atlas_page ], tex_label );
        } else if ( dcall.pass == VE_FONTCACHE_FRAMEBUFFER_PASS_ATLAS_UPLOAD ) {
            begin_pass_group( 4, "VEFC CPU Atlas Upload" );
            glActiveTexture( GL_TEXTURE0 );
            glBindTexture( GL_TEXTURE_2D, renderer->cpu_atlas_textures[ dcall.atlas_page ] );
            glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
            glPixelStorei( GL_PACK_ALIGNMENT, 1 );
            vxui_demo_flip_fontcache_upload( drawlist, dcall );
            glTexSubImage2D( GL_TEXTURE_2D, 0, dcall.upload_region_x, dcall.upload_region_y, dcall.upload_region_w, dcall.upload_region_h, GL_RED, GL_UNSIGNED_BYTE, &drawlist->texels[ dcall.texel_offset ] );
#endif
        } else {
            end_pass_group();
        }

        if ( dcall.clear_before_draw ) {
            glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
            glClear( GL_COLOR_BUFFER_BIT );
        }
        if ( dcall.end_index > dcall.start_index ) {
            glDrawElements( GL_TRIANGLES, dcall.end_index - dcall.start_index, GL_UNSIGNED_INT, ( GLvoid* ) ( dcall.start_index * sizeof( uint32_t ) ) );
        }
    }

    end_pass_group();
    const GLuint target_fbo = renderer->backend_test_mode ? renderer->backend_target_fbo : ( renderer->shot_capture_mode ? renderer->shot_capture_fbo : 0u );
    glBindFramebuffer( GL_FRAMEBUFFER, target_fbo );
    glViewport( 0, 0, renderer->window_size.width, renderer->window_size.height );
    if ( clip_rect && clip_rect->w > 0.0f && clip_rect->h > 0.0f ) {
        GLint x = ( GLint ) std::lround( clip_rect->x );
        GLint y = ( GLint ) std::lround( ( float ) renderer->window_size.height - clip_rect->y - clip_rect->h );
        GLsizei w = ( GLsizei ) std::lround( clip_rect->w );
        GLsizei h = ( GLsizei ) std::lround( clip_rect->h );
        glEnable( GL_SCISSOR_TEST );
        glScissor( x, y, w < 0 ? 0 : w, h < 0 ? 0 : h );
    } else {
        glDisable( GL_SCISSOR_TEST );
    }
    ve_fontcache_flush_drawlist( &renderer->cache );
}

static void vxui_demo_render_text_cmd( vxui_demo_renderer* renderer, const vxui_draw_cmd_text* text )
{
    if ( !renderer || !text || !text->text ) {
        return;
    }

    float colour[ 4 ] = {
        text->color.r / 255.0f,
        text->color.g / 255.0f,
        text->color.b / 255.0f,
        text->color.a / 255.0f,
    };
    ve_fontcache_set_colour( &renderer->cache, colour );
    float posx = text->pos.x / ( float ) renderer->window_size.width;
    float posy = 1.0f - ( text->pos.y + text->size ) / ( float ) renderer->window_size.height;
    ve_fontcache_draw_text(
        &renderer->cache,
        ( ve_font_id ) text->font_id,
        vxui_demo_to_u8( text->text ),
        posx,
        posy,
        1.0f / ( float ) renderer->window_size.width,
        1.0f / ( float ) renderer->window_size.height,
        true );

    char event_label[ 64 ];
    std::snprintf( event_label, sizeof( event_label ), "Submit Text Cmd: font=%u len=%zu", text->font_id, std::strlen( text->text ) );
    vxui_demo_gl_debug_event( renderer, event_label );
}

static void vxui_demo_render_draw_list( vxui_demo_renderer* renderer, const vxui_draw_list* list )
{
    if ( !renderer || !list ) {
        return;
    }

    std::vector< vxui_rect > clip_stack;
    clip_stack.reserve( 16 );
    glDisable( GL_SCISSOR_TEST );
    glDisable( GL_FRAMEBUFFER_SRGB );

    auto apply_clip = [&]( const vxui_rect* rect ) {
        if ( !rect || rect->w <= 0.0f || rect->h <= 0.0f ) {
            glDisable( GL_SCISSOR_TEST );
            return;
        }
        GLint x = ( GLint ) std::lround( rect->x );
        GLint y = ( GLint ) std::lround( ( float ) renderer->window_size.height - rect->y - rect->h );
        GLsizei w = ( GLsizei ) std::lround( rect->w );
        GLsizei h = ( GLsizei ) std::lround( rect->h );
        glEnable( GL_SCISSOR_TEST );
        glScissor( x, y, w < 0 ? 0 : w, h < 0 ? 0 : h );
    };
    auto clip_matches = []( const vxui_rect* lhs, const vxui_rect* rhs ) {
        if ( lhs == nullptr || rhs == nullptr ) {
            return lhs == rhs;
        }
        return lhs->x == rhs->x && lhs->y == rhs->y && lhs->w == rhs->w && lhs->h == rhs->h;
    };
    bool text_batch_pending = false;
    vxui_rect text_batch_clip = {};
    const vxui_rect* text_batch_clip_ptr = nullptr;
    auto flush_pending_text = [&]() {
        if ( !text_batch_pending ) {
            return;
        }
        vxui_demo_render_fontcache_drawlist( renderer, text_batch_clip_ptr );
        text_batch_pending = false;
        text_batch_clip_ptr = nullptr;
    };

    ve_fontcache_flush_drawlist( &renderer->cache );
    for ( int i = 0; i < list->length; ++i ) {
        const vxui_cmd* cmd = &list->commands[ i ];
        switch ( cmd->type ) {
            case VXUI_CMD_RECT:
                flush_pending_text();
                vxui_demo_draw_primitive( renderer, cmd->rect.bounds, cmd->rect.color, 0.0f, 0.0f );
                break;

            case VXUI_CMD_RECT_ROUNDED:
                flush_pending_text();
                vxui_demo_draw_primitive( renderer, cmd->rect_rounded.bounds, cmd->rect_rounded.color, cmd->rect_rounded.radius, 0.0f );
                break;

            case VXUI_CMD_BORDER:
                flush_pending_text();
                vxui_demo_draw_primitive( renderer, cmd->border.bounds, cmd->border.color, cmd->border.radius, cmd->border.width );
                break;

            case VXUI_CMD_IMAGE:
                flush_pending_text();
                vxui_demo_draw_placeholder_image( renderer, cmd->image.bounds );
                break;

            case VXUI_CMD_TEXT: {
                const vxui_rect* current_clip = clip_stack.empty() ? nullptr : &clip_stack.back();
                if ( current_clip && ( current_clip->w <= 0.0f || current_clip->h <= 0.0f ) ) {
                    current_clip = nullptr;
                }
                if ( text_batch_pending && !clip_matches( text_batch_clip_ptr, current_clip ) ) {
                    flush_pending_text();
                }
                if ( current_clip ) {
                    text_batch_clip = *current_clip;
                    text_batch_clip_ptr = &text_batch_clip;
                } else {
                    text_batch_clip_ptr = nullptr;
                }
                vxui_demo_render_text_cmd( renderer, &cmd->text );
                text_batch_pending = true;
                break;
            }

            case VXUI_CMD_CLIP_PUSH:
                flush_pending_text();
                clip_stack.push_back( cmd->clip.rect );
                apply_clip( &clip_stack.back() );
                break;

            case VXUI_CMD_CLIP_POP:
                flush_pending_text();
                if ( !clip_stack.empty() ) {
                    clip_stack.pop_back();
                }
                apply_clip( clip_stack.empty() ? nullptr : &clip_stack.back() );
                break;
        }
    }

    flush_pending_text();
    glDisable( GL_SCISSOR_TEST );
}

static void vxui_demo_present_draw_list( vxui_demo_renderer* renderer, vxui_ctx* ctx, const vxui_draw_list* list )
{
    if ( !renderer || !list ) {
        return;
    }
    const GLuint target_fbo = renderer->shot_capture_mode ? renderer->shot_capture_fbo : 0u;
    glBindFramebuffer( GL_FRAMEBUFFER, target_fbo );
    glViewport( 0, 0, renderer->window_size.width, renderer->window_size.height );
    glDisable( GL_SCISSOR_TEST );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glDisable( GL_FRAMEBUFFER_SRGB );
    const vxui_demo_command_deck_theme& theme = vxui_demo_command_deck_theme_tokens();
    glClearColor(
        ( float ) theme.app_background_base.r / 255.0f,
        ( float ) theme.app_background_base.g / 255.0f,
        ( float ) theme.app_background_base.b / 255.0f,
        1.0f );
    glClear( GL_COLOR_BUFFER_BIT );

    vxui_demo_gl_debug_begin( renderer, "VXUI Draw List" );
    vxui_demo_render_draw_list( renderer, list );
    vxui_demo_gl_debug_end( renderer );
    if ( ctx ) {
        vxui_flush_text( ctx );
    }
    vxui_demo_gl_debug_event( renderer, renderer->shot_capture_mode ? "Shot Ready" : "Present" );
}

static bool vxui_demo_write_png_rgba( const std::filesystem::path& path, int width, int height, const uint8_t* pixels, char* error, size_t error_size )
{
    if ( width <= 0 || height <= 0 || !pixels ) {
        std::snprintf( error, error_size, "%s", "invalid PNG write arguments" );
        return false;
    }

    std::error_code fs_error;
    const std::filesystem::path parent = path.parent_path();
    if ( !parent.empty() ) {
        std::filesystem::create_directories( parent, fs_error );
        if ( fs_error ) {
            std::snprintf( error, error_size, "failed to create output directory: %s", fs_error.message().c_str() );
            return false;
        }
    }

    HRESULT init_hr = CoInitializeEx( nullptr, COINIT_MULTITHREADED );
    const bool should_uninitialize = SUCCEEDED( init_hr );
    if ( init_hr == RPC_E_CHANGED_MODE ) {
        init_hr = S_OK;
    }
    if ( FAILED( init_hr ) ) {
        std::snprintf( error, error_size, "CoInitializeEx failed: 0x%08lx", ( unsigned long ) init_hr );
        return false;
    }

    IWICImagingFactory* factory = nullptr;
    IWICStream* stream = nullptr;
    IWICBitmapEncoder* encoder = nullptr;
    IWICBitmapFrameEncode* frame = nullptr;
    IPropertyBag2* properties = nullptr;
    bool ok = false;
    const std::wstring wide_path = path.wstring();

    HRESULT hr = CoCreateInstance( CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS( &factory ) );
    if ( FAILED( hr ) ) {
        std::snprintf( error, error_size, "CoCreateInstance(IWICImagingFactory) failed: 0x%08lx", ( unsigned long ) hr );
        goto cleanup;
    }

    hr = factory->CreateStream( &stream );
    if ( FAILED( hr ) ) {
        std::snprintf( error, error_size, "CreateStream failed: 0x%08lx", ( unsigned long ) hr );
        goto cleanup;
    }

    hr = stream->InitializeFromFilename( wide_path.c_str(), GENERIC_WRITE );
    if ( FAILED( hr ) ) {
        std::snprintf( error, error_size, "InitializeFromFilename failed: 0x%08lx", ( unsigned long ) hr );
        goto cleanup;
    }

    hr = factory->CreateEncoder( GUID_ContainerFormatPng, nullptr, &encoder );
    if ( FAILED( hr ) ) {
        std::snprintf( error, error_size, "CreateEncoder failed: 0x%08lx", ( unsigned long ) hr );
        goto cleanup;
    }

    hr = encoder->Initialize( stream, WICBitmapEncoderNoCache );
    if ( FAILED( hr ) ) {
        std::snprintf( error, error_size, "Encoder Initialize failed: 0x%08lx", ( unsigned long ) hr );
        goto cleanup;
    }

    hr = encoder->CreateNewFrame( &frame, &properties );
    if ( FAILED( hr ) ) {
        std::snprintf( error, error_size, "CreateNewFrame failed: 0x%08lx", ( unsigned long ) hr );
        goto cleanup;
    }

    hr = frame->Initialize( properties );
    if ( FAILED( hr ) ) {
        std::snprintf( error, error_size, "Frame Initialize failed: 0x%08lx", ( unsigned long ) hr );
        goto cleanup;
    }

    hr = frame->SetSize( ( UINT ) width, ( UINT ) height );
    if ( FAILED( hr ) ) {
        std::snprintf( error, error_size, "Frame SetSize failed: 0x%08lx", ( unsigned long ) hr );
        goto cleanup;
    }

    WICPixelFormatGUID pixel_format = GUID_WICPixelFormat32bppBGRA;
    hr = frame->SetPixelFormat( &pixel_format );
    if ( FAILED( hr ) ) {
        std::snprintf( error, error_size, "Frame SetPixelFormat failed: 0x%08lx", ( unsigned long ) hr );
        goto cleanup;
    }
    if ( pixel_format != GUID_WICPixelFormat32bppBGRA ) {
        std::snprintf( error, error_size, "%s", "PNG encoder rejected BGRA pixel format" );
        goto cleanup;
    }

    hr = frame->WritePixels( ( UINT ) height, ( UINT ) ( width * 4 ), ( UINT ) ( width * height * 4 ), const_cast< BYTE* >( pixels ) );
    if ( FAILED( hr ) ) {
        std::snprintf( error, error_size, "WritePixels failed: 0x%08lx", ( unsigned long ) hr );
        goto cleanup;
    }

    hr = frame->Commit();
    if ( FAILED( hr ) ) {
        std::snprintf( error, error_size, "Frame Commit failed: 0x%08lx", ( unsigned long ) hr );
        goto cleanup;
    }

    hr = encoder->Commit();
    if ( FAILED( hr ) ) {
        std::snprintf( error, error_size, "Encoder Commit failed: 0x%08lx", ( unsigned long ) hr );
        goto cleanup;
    }

    ok = true;

cleanup:
    if ( properties ) properties->Release();
    if ( frame ) frame->Release();
    if ( encoder ) encoder->Release();
    if ( stream ) stream->Release();
    if ( factory ) factory->Release();
    if ( should_uninitialize ) {
        CoUninitialize();
    }
    return ok;
}

static bool vxui_demo_capture_rgba_texture_png( GLuint texture, int width, int height, const std::filesystem::path& path, char* error, size_t error_size )
{
    if ( texture == 0 || width <= 0 || height <= 0 ) {
        std::snprintf( error, error_size, "%s", "invalid shot capture texture" );
        return false;
    }

    static GLuint readback_fbo = 0;
    if ( readback_fbo == 0 ) {
        glGenFramebuffers( 1, &readback_fbo );
    }

    std::vector< uint8_t > pixels( ( size_t ) width * ( size_t ) height * 4u );
    GLint previous_read_framebuffer = 0;
    glGetIntegerv( GL_READ_FRAMEBUFFER_BINDING, &previous_read_framebuffer );
    glBindFramebuffer( GL_READ_FRAMEBUFFER, readback_fbo );
    glFramebufferTexture2D( GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0 );
    if ( glCheckFramebufferStatus( GL_READ_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE ) {
        glBindFramebuffer( GL_READ_FRAMEBUFFER, static_cast< GLuint >( previous_read_framebuffer ) );
        std::snprintf( error, error_size, "%s", "shot capture framebuffer incomplete" );
        return false;
    }

    while ( glGetError() != GL_NO_ERROR ) {
    }
    glReadBuffer( GL_COLOR_ATTACHMENT0 );
    glPixelStorei( GL_PACK_ALIGNMENT, 1 );
    glReadPixels( 0, 0, width, height, GL_BGRA, GL_UNSIGNED_BYTE, pixels.data() );
    const GLenum read_error = glGetError();
    glBindFramebuffer( GL_READ_FRAMEBUFFER, static_cast< GLuint >( previous_read_framebuffer ) );
    if ( read_error != GL_NO_ERROR ) {
        std::snprintf( error, error_size, "glReadPixels failed for shot capture texture (0x%x)", read_error );
        return false;
    }

    const size_t row_stride = ( size_t ) width * 4u;
    std::vector< uint8_t > flipped( pixels.size() );
    for ( int y = 0; y < height; ++y ) {
        const uint8_t* src = pixels.data() + ( size_t ) ( height - 1 - y ) * row_stride;
        uint8_t* dst = flipped.data() + ( size_t ) y * row_stride;
        std::memcpy( dst, src, row_stride );
    }

    return vxui_demo_write_png_rgba( path, width, height, flipped.data(), error, error_size );
}

static bool vxui_demo_button_edge( vxui_demo_app* app, vxui_demo_button button, bool down )
{
    bool pressed = down && !app->button_prev[ button ];
    app->button_prev[ button ] = down;
    return pressed;
}

static bool vxui_demo_key_down( const TinyWindow::tWindow* window, int key )
{
    if ( !window || key < 0 || key > TinyWindow::last ) {
        return false;
    }
    return window->keys[ key ] == TinyWindow::keyState_t::down;
}

static bool vxui_demo_char_down( const TinyWindow::tWindow* window, char ch )
{
    unsigned char upper = ( unsigned char ) std::toupper( ( unsigned char ) ch );
    unsigned char lower = ( unsigned char ) std::tolower( ( unsigned char ) ch );
    return vxui_demo_key_down( window, upper ) || vxui_demo_key_down( window, lower );
}

static const TinyWindow::gamepad_t* vxui_demo_primary_gamepad( TinyWindow::windowManager* manager )
{
    if ( !manager ) {
        return nullptr;
    }
    for ( TinyWindow::gamepad_t* gamepad : manager->GetGamepads() ) {
        if ( gamepad ) {
            return gamepad;
        }
    }
    return nullptr;
}

static bool vxui_demo_gamepad_down( const TinyWindow::gamepad_t* gamepad, int button )
{
    return gamepad && button >= 0 && button < ( int ) gamepad->buttonStates.size() && gamepad->buttonStates[ button ];
}

int main( int argc, char** argv )
{
    bool vefc_backend_test_mode = false;
    vxui_demo_shot_request shot_request = {};
    char cli_error[ 256 ] = {};
    if ( !vxui_demo_parse_cli( argc, argv, &vefc_backend_test_mode, &shot_request, cli_error, sizeof( cli_error ) ) ) {
        std::fprintf( stderr, "VXUI demo argument error: %s\n", cli_error );
        return 1;
    }

    // Disable MSVC CRT error dialogs
    _set_error_mode( _OUT_TO_STDERR );
    _CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_FILE );
    _CrtSetReportFile( _CRT_ERROR, _CRTDBG_FILE_STDERR );
    _CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_FILE );
    _CrtSetReportFile( _CRT_ASSERT, _CRTDBG_FILE_STDERR );
    _CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE );
    _CrtSetReportFile( _CRT_WARN, _CRTDBG_FILE_STDERR );
    SetErrorMode( SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX );

     TinyWindow::windowSetting_t cfg;
     cfg.name = "VXUI";
     cfg.versionMajor = 3;
     cfg.versionMinor = 3;
     cfg.enableSRGB = false;
     cfg.resolution.width = shot_request.enabled ? ( unsigned int ) shot_request.width : ( vefc_backend_test_mode ? 1920u : 1280u );
     cfg.resolution.height = shot_request.enabled ? ( unsigned int ) shot_request.height : ( vefc_backend_test_mode ? 1080u : 720u );
     cfg.SetProfile( TinyWindow::profile_t::core );
     cfg.startHidden = vefc_backend_test_mode || shot_request.enabled;

     std::unique_ptr< TinyWindow::windowManager > manager( new TinyWindow::windowManager() );
     std::unique_ptr< TinyWindow::tWindow > window( manager->AddWindow( cfg ) );
     if ( !window ) {
         std::fprintf( stderr, "Failed to create the TinyWindow demo window: AddWindow returned null.\n" );
         manager->ShutDown();
         return 1;
     }

     // Note: TinyWindow's 'initialized' flag is not reliably set to true on the Windows path
     // in this vendored version. We rely on contextCreated and valid handles instead.
     // The initialized value is retained here for debug visibility only.
     HWND winHandle = window->GetWindowHandle();
     HDC devCtx = window->GetDeviceContextDeviceHandle();
     if ( !winHandle || !devCtx ) {
         std::fprintf( stderr, "TinyWindow created no usable Win32 window handle or device context.\n" );
         std::fprintf( stderr, "initialized=%d contextCreated=%d windowHandle=%p deviceContextHandle=%p\n",
             window->initialized, window->contextCreated, ( void* ) winHandle, ( void* ) devCtx );
         window.reset( nullptr );
         manager->ShutDown();
         return 1;
     }

     HGLRC glCtx = window->GetGLRenderingContextHandle();
     if ( !window->contextCreated || !glCtx ) {
         std::fprintf( stderr, "TinyWindow failed to create an OpenGL context.\n" );
         std::fprintf( stderr, "initialized=%d contextCreated=%d glRenderingContextHandle=%p\n",
             window->initialized, window->contextCreated, ( void* ) glCtx );
         window.reset( nullptr );
         manager->ShutDown();
         return 1;
     }

    if ( !gladLoadGL() ) {
         std::fprintf( stderr, "Failed to load OpenGL symbols through glad.\n" );
         window.reset( nullptr );
         manager->ShutDown();
         return 1;
     }

    vxui_demo_renderer renderer = {};
    renderer.window_size = window->settings.resolution;
    if ( !vxui_demo_init_renderer( &renderer ) ) {
         std::fprintf( stderr, "Failed to initialize the VXUI demo renderer.\n" );
         vxui_demo_shutdown_renderer( &renderer );
         window.reset( nullptr );
         manager->ShutDown();
         return 1;
    }
    ve_fontcache_init( &renderer.cache, true );
    if ( !vxui_demo_load_fonts( &renderer ) ) {
         std::fprintf( stderr, "Failed to initialize the VXUI demo renderer.\n" );
         vxui_demo_shutdown_renderer( &renderer );
         window.reset( nullptr );
         manager->ShutDown();
         return 1;
     }

    if ( shot_request.enabled && !vxui_demo_setup_shot_capture_fbo( &renderer ) ) {
         std::fprintf( stderr, "Failed to initialize the VXUI demo shot capture surface.\n" );
         vxui_demo_shutdown_renderer( &renderer );
         window.reset( nullptr );
         manager->ShutDown();
         return 1;
     }

    renderer.snap_configured = false;
    vxui_demo_configure_snap_if_needed( &renderer, true );

    if ( vefc_backend_test_mode ) {
        // Setup backend test FBO for VEFC backend tests
        if ( !vxui_demo_setup_backend_test_fbo( &renderer ) ) {
            std::fprintf( stderr, "Failed to setup backend test FBO.\n" );
            vxui_demo_shutdown_renderer( &renderer );
            window.reset( nullptr );
            manager->ShutDown();
            return 1;
        }

        ve_fontcache_backend_test_options options = {};
        options.cache = &renderer.cache;
        options.provision_fonts = []( ve_fontcache* target_cache, const ve_fontcache_backend_test_font_spec* roles, size_t role_count ) -> ve_fontcache_backend_test_font_set {
            return vxui_demo_provision_fonts( target_cache, roles, role_count );
        };
        options.capabilities.has_present_surface = true;
        options.capabilities.has_target_linear_surface = true;
#ifdef VE_FONTCACHE_FREETYPE_RASTERISATION
        options.capabilities.has_cpu_atlas_surface = true;
        options.capabilities.supports_freetype_mode = true;
#endif
#ifdef VE_FONTCACHE_HARFBUZZ
        //options.capabilities.supports_harfbuzz_mode = true;
        options.capabilities.supports_harfbuzz_mode = false; // TEMP DISABLED
#endif
        options.execute_pipeline = []() { vxui_demo_backend_test_execute_pipeline(); };
        options.execute_present = []() { vxui_demo_backend_test_execute_present(); };
        options.execute_frame = []() { vxui_demo_backend_test_execute_frame(); };
        options.readback_surface = []( const char* name, int x, int y, int w, int h, uint8_t* out_pixels ) -> bool { return vxui_demo_backend_test_readback( name, x, y, w, h, out_pixels ); };
        options.reset_surfaces = []() {
            extern vxui_demo_renderer* g_vxui_demo_backend_test_renderer;
            vxui_demo_renderer* r = g_vxui_demo_backend_test_renderer;
            if ( r ) {
                vxui_demo_clear_backend_test_surfaces( r, true );
                // Backend tests immediately inspect the reset surfaces.
                glFinish();
            }
        };
        options.write_surface = []( const char* name, int x, int y, int w, int h, const uint8_t* pixels ) -> bool { return vxui_demo_backend_test_write_surface( name, x, y, w, h, pixels ); };
        options.reload_font = []() -> ve_font_id { return vxui_demo_reload_font(); };
        options.prepare_real_text = []() { vxui_demo_prepare_real_text(); };
        options.finalise_test_state = []() { vxui_demo_backend_test_finalise_state(); };

        // Set global renderer pointer for backend test callbacks
        g_vxui_demo_backend_test_renderer = &renderer;

        // Run VEFC backend tests
        ve_fontcache_backend_test_result result_stb = {};
        ve_fontcache_backend_test_result result_freetype = {};
        ve_fontcache_backend_test_result combined_result = {};

        // stb mode
        if ( !options.capabilities.supports_freetype_mode ) {
            result_stb = ve_fontcache_backend_test_run( options );
            combined_result.passed += result_stb.passed;
            combined_result.failed += result_stb.failed;
            combined_result.skipped += result_stb.skipped;
        } else {
            std::printf( "=== VEFC Backend Tests: stb mode ===\n" );
            vxui_demo_reinit_cache_for_backend_test( &renderer, false );
            result_stb = ve_fontcache_backend_test_run( options );
            std::printf( "stb mode: %d passed, %d failed, %d skipped\n", result_stb.passed, result_stb.failed, result_stb.skipped );
            for ( const auto& failure : result_stb.failures ) {
                std::printf( "  FAIL[stb]: %s\n", failure.c_str() );
            }
            combined_result.passed += result_stb.passed;
            combined_result.failed += result_stb.failed;
            combined_result.skipped += result_stb.skipped;

            // Switch to FreeType mode
            std::printf( "\n=== VEFC Backend Tests: FreeType mode ===\n" );
            vxui_demo_reinit_cache_for_backend_test( &renderer, true );
            options.prepare_real_text = []() { vxui_demo_prepare_real_text(); };
            result_freetype = ve_fontcache_backend_test_run( options );
            std::printf( "FreeType mode: %d passed, %d failed, %d skipped\n", result_freetype.passed, result_freetype.failed, result_freetype.skipped );
            for ( const auto& failure : result_freetype.failures ) {
                std::printf( "  FAIL[ft]: %s\n", failure.c_str() );
            }
            combined_result.passed += result_freetype.passed;
            combined_result.failed += result_freetype.failed;
            combined_result.skipped += result_freetype.skipped;
        }

        std::printf( "\n=== Combined VEFC Backend Test Summary ===\n" );
        std::printf( "Total: %d passed, %d failed, %d skipped\n", combined_result.passed, combined_result.failed, combined_result.skipped );
        if ( !combined_result.failures.empty() ) {
            std::printf( "\nAll failures:\n" );
            for ( const auto& failure : combined_result.failures ) {
                std::printf( "  - %s\n", failure.c_str() );
            }
        }
        if ( !combined_result.skipped_tests.empty() ) {
            std::printf( "\nSkipped tests:\n" );
            for ( const auto& skip : combined_result.skipped_tests ) {
                std::printf( "  - %s\n", skip.c_str() );
            }
        }

        vxui_demo_shutdown_renderer( &renderer );
        window.reset( nullptr );
        manager->ShutDown();
        return combined_result.failed > 0 ? 1 : 0;
    }

    std::vector< uint8_t > memory( ( size_t ) vxui_min_memory_size() );
    vxui_ctx ctx = {};
    vxui_init(
        &ctx,
        vxui_create_arena( ( uint64_t ) memory.size(), memory.data() ),
        ( vxui_config ) {
            .screen_width = ( int ) renderer.window_size.width,
            .screen_height = ( int ) renderer.window_size.height,
            .max_elements = 256,
            .max_anim_states = 256,
            .max_sequences = 16,
    } );
    vxui_set_fontcache( &ctx, &renderer.cache );
    vxui_set_font_resolver( &ctx, vxui_demo_font_resolver, &renderer );
    ctx.default_font_id = VXUI_DEMO_FONT_ROLE_BODY;
    ctx.default_font_size = ( float ) VXUI_DEMO_BODY_SIZE;
    ctx.default_text_color = ( vxui_color ) { 242, 244, 255, 255 };

    vxui_demo_app app = {};
    app.ctx = &ctx;
    app.window = window.get();
    app.difficulty = 1;
    app.volume = 0.40f;
    app.scanline_index = 1;
    app.effect_intensity_index = 1;
    app.shot_layout_surface_max_height_override = 0.0f;
    app.shot_mode = shot_request.enabled;
    app.shot_force_compact_layout = shot_request.compact_override;
    app.watched_seq_path = vxui_demo_make_temp_path( "vxui_demo_sequence.toml" );
    app.last_selected_seq = -1;
    vxui_set_text_fn( &ctx, vxui_demo_text, &app );

    const char* difficulty_keys[] = { "difficulty.easy", "difficulty.normal", "difficulty.hard" };
    const char* slot_keys[] = { "slot.0", "slot.1", "slot.2", "slot.3", "slot.4", "slot.5", "slot.6", "slot.7" };
#ifdef VXUI_DEBUG
    const char* prop_labels[] = { "opacity", "scale", "slide_x", "slide_y" };
#endif
    const vxui_input_table keyboard_table = {
        .confirm = { VXUI_DEMO_FONT_ROLE_BODY, 'E' },
        .cancel = { VXUI_DEMO_FONT_ROLE_BODY, 'Q' },
        .tab_left = { VXUI_DEMO_FONT_ROLE_BODY, '[' },
        .tab_right = { VXUI_DEMO_FONT_ROLE_BODY, ']' },
        .up = { VXUI_DEMO_FONT_ROLE_BODY, '^' },
        .down = { VXUI_DEMO_FONT_ROLE_BODY, 'v' },
        .left = { VXUI_DEMO_FONT_ROLE_BODY, '<' },
        .right = { VXUI_DEMO_FONT_ROLE_BODY, '>' },
    };
    const vxui_input_table gamepad_table = {
        .confirm = { VXUI_DEMO_FONT_ROLE_BODY, 'A' },
        .cancel = { VXUI_DEMO_FONT_ROLE_BODY, 'B' },
        .tab_left = { VXUI_DEMO_FONT_ROLE_BODY, 'L' },
        .tab_right = { VXUI_DEMO_FONT_ROLE_BODY, 'R' },
        .up = { VXUI_DEMO_FONT_ROLE_BODY, '^' },
        .down = { VXUI_DEMO_FONT_ROLE_BODY, 'v' },
        .left = { VXUI_DEMO_FONT_ROLE_BODY, '<' },
        .right = { VXUI_DEMO_FONT_ROLE_BODY, '>' },
    };
    app.keyboard_table = keyboard_table;
    app.gamepad_table = gamepad_table;
    vxui_demo_apply_locale_index( &ctx, &app, VXUI_DEMO_LOCALE_ENGLISH );
    vxui_demo_apply_prompt_table_index( &ctx, &app, 0 );

    if ( !shot_request.enabled && !vxui_demo_write_file(
             app.watched_seq_path.c_str(),
             "[sequence.main_menu_enter]\n"
             "steps = [\n"
              "  { delay = 0, id = \"main.surface\", prop = \"opacity\", target = 0.35 },\n"
              "  { delay = 0, id = \"main.surface\", prop = \"slide_x\", target = 20.0 },\n"
              "  { delay = 100, id = \"main.surface\", prop = \"opacity\", target = 1.0 },\n"
              "  { delay = 120, id = \"main.surface\", prop = \"slide_x\", target = 0.0 },\n"
              "]\n" ) ) {
        std::fprintf( stderr, "Failed to create the watched sequence file at %s.\n", app.watched_seq_path.c_str() );
        vxui_demo_shutdown_renderer( &renderer );
        window.reset( nullptr );
        manager->ShutDown();
        return 1;
    }

    char error[ 256 ] = {};
    if ( !shot_request.enabled && !vxui_load_seq_toml( &ctx, app.watched_seq_path.c_str(), VXUI_DEMO_DEFAULT_WATCHED_SEQUENCE_NAME, error, sizeof( error ) ) ) {
        std::fprintf( stderr, "Failed to load %s: %s\n", app.watched_seq_path.c_str(), error );
        std::remove( app.watched_seq_path.c_str() );
        vxui_demo_shutdown_renderer( &renderer );
        window.reset( nullptr );
        manager->ShutDown();
        return 1;
    }
#ifdef VXUI_DEBUG
    if ( !shot_request.enabled && !vxui_watch_seq_file( &ctx, app.watched_seq_path.c_str(), VXUI_DEMO_DEFAULT_WATCHED_SEQUENCE_NAME ) ) {
        std::fprintf( stderr, "Failed to watch %s for hot reload.\n", app.watched_seq_path.c_str() );
        std::remove( app.watched_seq_path.c_str() );
        vxui_demo_shutdown_renderer( &renderer );
        window.reset( nullptr );
        manager->ShutDown();
        return 1;
    }
    if ( !shot_request.enabled ) {
        ctx.debug_seq_editor.selected_seq = vxui_demo_find_seq_index( &ctx, VXUI_DEMO_DEFAULT_WATCHED_SEQUENCE_NAME );
        vxui_debug_generate_seq_outputs( &ctx );
        vxui_demo_sync_step_editor( &app );
    }
#endif

    auto register_surface_enter = [&]( const char* name, const char* target_id, float slide_x, float slide_y, float scale_start )
    {
        vxui_seq_step steps[] = {
            { 0, vxui_id( target_id ), VXUI_PROP_OPACITY, 0.30f },
            { 0, vxui_id( target_id ), VXUI_PROP_SLIDE_X, slide_x },
            { 0, vxui_id( target_id ), VXUI_PROP_SLIDE_Y, slide_y },
            { 0, vxui_id( target_id ), VXUI_PROP_SCALE, scale_start },
            { 120, vxui_id( target_id ), VXUI_PROP_OPACITY, 1.0f },
            { 120, vxui_id( target_id ), VXUI_PROP_SLIDE_X, 0.0f },
            { 120, vxui_id( target_id ), VXUI_PROP_SLIDE_Y, 0.0f },
            { 140, vxui_id( target_id ), VXUI_PROP_SCALE, 1.0f },
        };
        vxui_register_seq( &ctx, name, steps, ( int ) ( sizeof( steps ) / sizeof( steps[ 0 ] ) ) );
    };

    vxui_seq_step settings_enter[] = {
        { 0, vxui_id( "settings.surface" ), VXUI_PROP_SLIDE_X, 28.0f },
        { 0, vxui_id( "settings.surface" ), VXUI_PROP_OPACITY, 0.35f },
        { 120, vxui_id( "settings.surface" ), VXUI_PROP_SLIDE_X, 0.0f },
        { 120, vxui_id( "settings.surface" ), VXUI_PROP_OPACITY, 1.0f },
    };
    vxui_seq_step settings_exit[] = {
        { 0, vxui_id( "settings.surface" ), VXUI_PROP_OPACITY, 0.4f },
        { 0, vxui_id( "settings.surface" ), VXUI_PROP_SLIDE_X, -20.0f },
    };
    if ( !shot_request.enabled ) {
        vxui_register_seq( &ctx, "settings_enter", settings_enter, ( int ) ( sizeof( settings_enter ) / sizeof( settings_enter[ 0 ] ) ) );
        vxui_register_seq( &ctx, "settings_exit", settings_exit, ( int ) ( sizeof( settings_exit ) / sizeof( settings_exit[ 0 ] ) ) );
        register_surface_enter( "boot_enter", "boot.surface", 0.0f, 18.0f, 0.98f );
        register_surface_enter( "title_enter", "title.surface", 0.0f, 24.0f, 0.98f );
        register_surface_enter( "sortie_enter", "sortie.surface", 24.0f, 0.0f, 0.99f );
        register_surface_enter( "loadout_enter", "loadout.surface", 24.0f, 0.0f, 0.99f );
        register_surface_enter( "archives_enter", "archives.surface", 18.0f, 0.0f, 0.99f );
        register_surface_enter( "records_enter", "records.surface", 18.0f, 0.0f, 0.99f );
        register_surface_enter( "credits_enter", "credits.surface", 0.0f, 18.0f, 0.99f );
        register_surface_enter( "launch_stub_enter", "launch_stub.surface", 0.0f, 18.0f, 0.99f );
        register_surface_enter( "results_stub_enter", "results_stub.surface", 0.0f, 18.0f, 0.99f );
        vxui_push_screen( &ctx, "boot" );
        vxui_demo_refresh_status( &app );
    } else {
        if ( !vxui_demo_apply_shot_request( &app, &ctx, shot_request, error, sizeof( error ) ) ) {
            std::fprintf( stderr, "Failed to apply shot request: %s\n", error );
            Clay_SetCurrentContext( nullptr );
            if ( !app.watched_seq_path.empty() ) {
                std::remove( app.watched_seq_path.c_str() );
            }
            vxui_demo_shutdown_renderer( &renderer );
            manager->ShutDown();
            window.reset( nullptr );
            return 1;
        }
        vxui_demo_refresh_status( &app );

        vxui_begin( &ctx, 1.0f / 60.0f );
        vxui_demo_render_frontend( &app, &renderer, &ctx );
        vxui_draw_list list = vxui_end( &ctx );
#ifdef VXUI_DEBUG
        vxui_demo_debug_validate_demo_layout( &ctx, &list );
#endif
        vxui_demo_present_draw_list( &renderer, &ctx, &list );
        char capture_error[ 256 ] = {};
        const bool captured =
            renderer.shot_capture_mode &&
            renderer.shot_capture_texture &&
            vxui_demo_capture_rgba_texture_png(
                renderer.shot_capture_texture,
                ( int ) renderer.window_size.width,
                ( int ) renderer.window_size.height,
                std::filesystem::path( shot_request.out_path ),
                capture_error,
                sizeof( capture_error ) );
        if ( !captured ) {
            if ( capture_error[ 0 ] == '\0' ) {
                std::snprintf( capture_error, sizeof( capture_error ), "%s", "shot capture texture was not available" );
            }
            std::fprintf( stderr, "Failed to capture screenshot: %s\n", capture_error );
            Clay_SetCurrentContext( nullptr );
            if ( !app.watched_seq_path.empty() ) {
                std::remove( app.watched_seq_path.c_str() );
            }
            vxui_demo_shutdown_renderer( &renderer );
            manager->ShutDown();
            window.reset( nullptr );
            return 1;
        }

        // Shot mode success: skip explicit GL teardown. Per-object glDelete*
        // calls are unnecessary work for a disposable one-shot process — the
        // driver reclaims all context resources when the GL context is destroyed.
        Clay_SetCurrentContext( nullptr );
        manager->ShutDown();
        window.reset( nullptr );
        return 0;
    }

    std::chrono::steady_clock::time_point previous = std::chrono::steady_clock::now();
    const std::chrono::steady_clock::duration kInteractiveFrameBudget =
        std::chrono::duration_cast< std::chrono::steady_clock::duration >( std::chrono::duration< double >( 1.0 / 60.0 ) );
    while ( !window->shouldClose ) {
        const std::chrono::steady_clock::time_point frame_start = std::chrono::steady_clock::now();
        manager->PollForEvents();
        vxui_demo_set_window_size( &renderer, window->settings.resolution );
        ctx.cfg.screen_width = ( int ) renderer.window_size.width;
        ctx.cfg.screen_height = ( int ) renderer.window_size.height;

        float dt = std::chrono::duration< float >( frame_start - previous ).count();
        previous = frame_start;
        if ( dt <= 0.0f || dt > 0.1f ) {
            dt = 1.0f / 60.0f;
        }
        uint64_t now_ms = ( uint64_t ) std::chrono::duration_cast< std::chrono::milliseconds >( frame_start.time_since_epoch() ).count();

#ifdef VXUI_DEBUG
        if ( ctx.debug_seq_editor.selected_seq != app.last_selected_seq ) {
            vxui_demo_sync_step_editor( &app );
            vxui_debug_generate_seq_outputs( &ctx );
        }
#endif
        vxui_demo_refresh_status( &app );

        vxui_begin( &ctx, dt );
        const TinyWindow::gamepad_t* gamepad = vxui_demo_primary_gamepad( manager.get() );
        if ( vxui_demo_button_edge( &app, VXUI_DEMO_BTN_NAV_UP, vxui_demo_key_down( window.get(), TinyWindow::arrowUp ) || vxui_demo_gamepad_down( gamepad, TinyWindow::gamepad_t::Dpad_top ) ) ) {
            vxui_input_nav( &ctx, VXUI_DIR_UP );
        }
        if ( vxui_demo_button_edge( &app, VXUI_DEMO_BTN_NAV_DOWN, vxui_demo_key_down( window.get(), TinyWindow::arrowDown ) || vxui_demo_gamepad_down( gamepad, TinyWindow::gamepad_t::Dpad_bottom ) ) ) {
            vxui_input_nav( &ctx, VXUI_DIR_DOWN );
        }
        if ( vxui_demo_button_edge( &app, VXUI_DEMO_BTN_NAV_LEFT, vxui_demo_key_down( window.get(), TinyWindow::arrowLeft ) || vxui_demo_gamepad_down( gamepad, TinyWindow::gamepad_t::Dpad_left ) ) ) {
            vxui_input_nav( &ctx, VXUI_DIR_LEFT );
        }
        if ( vxui_demo_button_edge( &app, VXUI_DEMO_BTN_NAV_RIGHT, vxui_demo_key_down( window.get(), TinyWindow::arrowRight ) || vxui_demo_gamepad_down( gamepad, TinyWindow::gamepad_t::Dpad_right ) ) ) {
            vxui_input_nav( &ctx, VXUI_DIR_RIGHT );
        }
        const bool confirm_pressed = vxui_demo_button_edge( &app, VXUI_DEMO_BTN_CONFIRM, vxui_demo_key_down( window.get(), TinyWindow::enter ) || vxui_demo_key_down( window.get(), TinyWindow::spacebar ) || vxui_demo_gamepad_down( gamepad, TinyWindow::gamepad_t::face_bottom ) );
        if ( confirm_pressed ) {
            vxui_input_confirm( &ctx );
        }
        const bool cancel_pressed = vxui_demo_button_edge( &app, VXUI_DEMO_BTN_CANCEL, vxui_demo_key_down( window.get(), TinyWindow::escape ) || vxui_demo_key_down( window.get(), TinyWindow::backspace ) || vxui_demo_gamepad_down( gamepad, TinyWindow::gamepad_t::face_right ) );
        if ( cancel_pressed ) {
            vxui_input_cancel( &ctx );
            vxui_demo_handle_cancel_navigation( &app, &ctx );
        }
        if ( vxui_demo_button_edge( &app, VXUI_DEMO_BTN_TAB_LEFT, vxui_demo_char_down( window.get(), '[' ) || vxui_demo_gamepad_down( gamepad, TinyWindow::gamepad_t::left_shoulder ) ) ) {
            vxui_input_tab( &ctx, -1 );
        }
        if ( vxui_demo_button_edge( &app, VXUI_DEMO_BTN_TAB_RIGHT, vxui_demo_char_down( window.get(), ']' ) || vxui_demo_gamepad_down( gamepad, TinyWindow::gamepad_t::right_shoulder ) ) ) {
            vxui_input_tab( &ctx, 1 );
        }
        if ( vxui_demo_button_edge( &app, VXUI_DEMO_BTN_LOCALE_EN, vxui_demo_char_down( window.get(), '1' ) ) ) {
            vxui_demo_apply_locale_index( &ctx, &app, VXUI_DEMO_LOCALE_ENGLISH );
        }
        if ( vxui_demo_button_edge( &app, VXUI_DEMO_BTN_LOCALE_JA, vxui_demo_char_down( window.get(), '2' ) ) ) {
            vxui_demo_apply_locale_index( &ctx, &app, VXUI_DEMO_LOCALE_JAPANESE );
        }
        if ( vxui_demo_button_edge( &app, VXUI_DEMO_BTN_LOCALE_AR, vxui_demo_char_down( window.get(), '3' ) ) ) {
            vxui_demo_apply_locale_index( &ctx, &app, VXUI_DEMO_LOCALE_ARABIC );
        }
        if ( vxui_demo_button_edge( &app, VXUI_DEMO_BTN_PROMPT_KEYBOARD, vxui_demo_char_down( window.get(), '4' ) ) ) {
            vxui_demo_apply_prompt_table_index( &ctx, &app, 0 );
        }
        if ( vxui_demo_button_edge( &app, VXUI_DEMO_BTN_PROMPT_GAMEPAD, vxui_demo_char_down( window.get(), '5' ) ) ) {
            vxui_demo_apply_prompt_table_index( &ctx, &app, 1 );
        }
#ifdef VXUI_DEBUG
        if ( vxui_demo_button_edge( &app, VXUI_DEMO_BTN_DEBUG_TOGGLE, vxui_demo_key_down( window.get(), TinyWindow::F1 ) ) ) {
            ctx.debug_seq_editor.open = !ctx.debug_seq_editor.open;
            ctx.debug_seq_editor.preview_playing = false;
        }
        if ( vxui_demo_button_edge( &app, VXUI_DEMO_BTN_DEBUG_RELOAD, vxui_demo_char_down( window.get(), 'R' ) ) ) {
            app.reload_status[ 0 ] = '\0';
            char reload_error[ 256 ] = {};
            if ( vxui_poll_seq_hot_reload( &ctx, now_ms, reload_error, sizeof( reload_error ) ) ) {
                std::snprintf( app.reload_status, sizeof( app.reload_status ), "%s", vxui_demo_text( "debug.reload.reloaded", &app ) );
                vxui_demo_sync_step_editor( &app );
                vxui_debug_generate_seq_outputs( &ctx );
            } else if ( reload_error[ 0 ] ) {
                std::snprintf( app.reload_status, sizeof( app.reload_status ), "%s: %s", vxui_demo_text( "debug.reload.failed_prefix", &app ), reload_error );
            } else {
                std::snprintf( app.reload_status, sizeof( app.reload_status ), "%s", vxui_demo_text( "debug.reload.unchanged", &app ) );
            }
        }
        if ( vxui_demo_button_edge( &app, VXUI_DEMO_BTN_DEBUG_COPY, vxui_demo_char_down( window.get(), 'C' ) ) ) {
            app.copied_to_clipboard = vxui_demo_copy_to_clipboard( ctx.debug_seq_editor.generated_toml );
            if ( !app.copied_to_clipboard ) {
                if ( ctx.debug_seq_editor.generated_c[ 0 ] ) {
                    std::puts( ctx.debug_seq_editor.generated_c );
                }
                if ( ctx.debug_seq_editor.generated_toml[ 0 ] ) {
                    std::puts( ctx.debug_seq_editor.generated_toml );
                }
            }
        }
#endif
        vxui_demo_handle_auto_transitions( &app, &ctx, dt, confirm_pressed );

#ifdef VXUI_DEBUG
        if ( ctx.debug_seq_editor.open ) {
            vxui_demo_apply_step_editor( &app );
            vxui_debug_generate_seq_outputs( &ctx );
        } else {
            ctx.debug_seq_editor.preview_playing = false;
        }
#endif
        vxui_demo_refresh_status( &app );

        vxui_demo_render_frontend( &app, &renderer, &ctx );

#ifdef VXUI_DEBUG
        if ( ctx.debug_seq_editor.open ) {
            const vxui_demo_command_deck_theme& theme = vxui_demo_command_deck_theme_tokens();
            const char* seq_names[ 16 ] = {};
            int seq_count = ctx.registered_seq_count;
            if ( seq_count > 16 ) {
                seq_count = 16;
            }
            for ( int i = 0; i < seq_count; ++i ) {
                seq_names[ i ] = ctx.registered_seqs[ i ].name;
            }

            VXUI( &ctx, "debug.overlay", {
                .layout = {
                    .sizing = { CLAY_SIZING_FIXED( 436.0f ), CLAY_SIZING_FIT( 0 ) },
                    .padding = CLAY_PADDING_ALL( 16 ),
                .childGap = VXUI_DEMO_ROW_GAP,
                .childAlignment = { .x = ctx.rtl ? CLAY_ALIGN_X_RIGHT : CLAY_ALIGN_X_LEFT },
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
            .backgroundColor = vxui_demo_clay_color( theme.utility_fill ),
            .cornerRadius = CLAY_CORNER_RADIUS( 14 ),
            .border = vxui_demo_panel_border( theme.utility_border, 1 ),
        } ) {
                VXUI_TRAIT( VXUI_TRAIT_SCANLINE, ( vxui_demo_scanline ) { .spacing = 8.0f, .alpha = theme.surface_scanline_alpha * 1.5f } );
                VXUI( &ctx, "debug.overlay.header", {
                    .layout = {
                        .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                        .padding = CLAY_PADDING_ALL( 12 ),
                        .childGap = 6,
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    },
                    .backgroundColor = vxui_demo_clay_color( theme.secondary_panel_fill ),
                    .cornerRadius = CLAY_CORNER_RADIUS( 10 ),
                    .border = vxui_demo_panel_border( theme.secondary_panel_border, 1 ),
                } ) {
                    VXUI_LABEL( &ctx, "debug.overlay.title", ( vxui_label_cfg ) {
                        .font_id = VXUI_DEMO_FONT_ROLE_SECTION,
                        .font_size = ( float ) VXUI_DEMO_SECTION_SIZE,
                        .color = theme.section_text,
                    } );
                    VXUI_LABEL( &ctx, "menu.debug", ( vxui_label_cfg ) {
                        .font_id = VXUI_DEMO_FONT_ROLE_BODY,
                        .font_size = 18.0f,
                        .color = theme.muted_text,
                    } );
                }
                if ( seq_count > 0 ) {
                    VXUI( &ctx, "debug.overlay.controls", {
                        .layout = {
                            .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                            .padding = CLAY_PADDING_ALL( 12 ),
                            .childGap = 10,
                            .layoutDirection = CLAY_TOP_TO_BOTTOM,
                        },
                        .backgroundColor = vxui_demo_clay_color( theme.primary_panel_fill ),
                        .cornerRadius = CLAY_CORNER_RADIUS( 10 ),
                        .border = vxui_demo_panel_border( theme.primary_panel_border, 1 ),
                    } ) {
                        VXUI_OPTION( &ctx, "debug.selected.seq", &ctx.debug_seq_editor.selected_seq, seq_names, seq_count, ( vxui_option_cfg ) { 0 } );
                        VXUI_LABEL( &ctx, "debug.selected_seq", ( vxui_label_cfg ) {
                            .font_id = VXUI_DEMO_FONT_ROLE_BODY,
                            .font_size = 18.0f,
                            .color = theme.muted_text,
                        } );
                        VXUI_LABEL( &ctx, "debug.step.id", ( vxui_label_cfg ) {
                            .font_id = VXUI_DEMO_FONT_ROLE_BODY,
                            .font_size = 18.0f,
                            .color = theme.muted_text,
                        } );
                        VXUI_OPTION( &ctx, "debug.step.prop", &app.debug_prop_index, prop_labels, 4, ( vxui_option_cfg ) { 0 } );
                        VXUI_SLIDER( &ctx, "debug.step.delay", &app.debug_delay_value, 0.0f, 200.0f, ( vxui_slider_cfg ) {
                            .show_value = true,
                            .format = "%.0f",
                        } );
                        VXUI_SLIDER( &ctx, "debug.step.target", &app.debug_target_value, -32.0f, 32.0f, ( vxui_slider_cfg ) {
                            .show_value = true,
                            .format = "%.2f",
                        } );
                    }
                }
                VXUI( &ctx, "debug.overlay.status", {
                    .layout = {
                        .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                        .padding = CLAY_PADDING_ALL( 12 ),
                        .childGap = 8,
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    },
                    .backgroundColor = vxui_demo_clay_color( theme.secondary_panel_fill ),
                    .cornerRadius = CLAY_CORNER_RADIUS( 10 ),
                    .border = vxui_demo_panel_border( theme.secondary_panel_border, 1 ),
                } ) {
                    VXUI_LABEL( &ctx, "debug.preview_status", ( vxui_label_cfg ) {
                        .font_id = VXUI_DEMO_FONT_ROLE_BODY,
                        .font_size = 18.0f,
                        .color = theme.success_text,
                    } );
                    VXUI_LABEL( &ctx, "debug.reload", ( vxui_label_cfg ) {
                        .font_id = VXUI_DEMO_FONT_ROLE_BODY,
                        .font_size = 18.0f,
                        .color = theme.muted_text,
                    } );
                    VXUI_LABEL( &ctx, "debug.clipboard", ( vxui_label_cfg ) {
                        .font_id = VXUI_DEMO_FONT_ROLE_BODY,
                        .font_size = 18.0f,
                        .color = theme.section_text,
                    } );
                }
                VXUI( &ctx, "debug.overlay.exports", {
                    .layout = {
                        .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                        .padding = CLAY_PADDING_ALL( 12 ),
                        .childGap = 8,
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    },
                    .backgroundColor = vxui_demo_clay_color( theme.primary_panel_fill ),
                    .cornerRadius = CLAY_CORNER_RADIUS( 10 ),
                    .border = vxui_demo_panel_border( theme.primary_panel_border, 1 ),
                } ) {
                    VXUI_LABEL( &ctx, "debug.generated_c", ( vxui_label_cfg ) {
                        .font_id = VXUI_DEMO_FONT_ROLE_CODE,
                        .font_size = ( float ) VXUI_DEMO_CODE_SIZE,
                        .color = theme.utility_text,
                    } );
                    VXUI_LABEL( &ctx, "debug.generated_toml", ( vxui_label_cfg ) {
                        .font_id = VXUI_DEMO_FONT_ROLE_CODE,
                        .font_size = ( float ) VXUI_DEMO_CODE_SIZE,
                        .color = theme.success_text,
                    } );
                }
            }
        }
#endif

        vxui_draw_list list = vxui_end( &ctx );
#ifdef VXUI_DEBUG
        if ( ctx.debug_seq_editor.open ) {
            vxui_debug_capture_preview( &ctx, &list );
            ctx.debug_seq_editor.preview_playing = ctx.debug_seq_editor.preview_snapshot.command_count > 0;
            vxui_demo_refresh_status( &app );
        }
        vxui_demo_debug_validate_demo_layout( &ctx, &list );
        int issue_count = vxui_debug_layout_issue_count( &ctx );
        if ( issue_count > 0 ) {
            std::fprintf( stderr, "vxui demo: %d layout issue(s) detected:\n", issue_count );
            int printed = 0;
            for ( int i = 0; i < issue_count && printed < 8; ++i ) {
                const vxui_layout_issue* issue = vxui_debug_layout_issue_at( &ctx, i );
                if ( issue ) {
                    std::fprintf(
                        stderr,
                        "  [%s] subject=%u bounds={%.1f,%.1f,%.1f,%.1f} %s\n",
                        vxui_layout_issue_kind_name( issue->kind ),
                        issue->subject_id,
                        issue->bounds.x,
                        issue->bounds.y,
                        issue->bounds.w,
                        issue->bounds.h,
                        issue->message );
                    printed++;
                }
            }
            if ( issue_count > 8 ) {
                std::fprintf( stderr, "  ... and %d more\n", issue_count - 8 );
            }
        }
#endif

        vxui_demo_present_draw_list( &renderer, &ctx, &list );
        if ( app.request_quit ) {
            window->shouldClose = true;
        }
        window->SwapDrawBuffers();
        const std::chrono::steady_clock::time_point frame_end = std::chrono::steady_clock::now();
        const std::chrono::steady_clock::time_point target_end = frame_start + kInteractiveFrameBudget;
        if ( frame_end < target_end ) {
            std::this_thread::sleep_until( target_end );
        }
    }

    Clay_SetCurrentContext( nullptr );
    std::remove( app.watched_seq_path.c_str() );
    vxui_demo_shutdown_renderer( &renderer );
    manager->ShutDown();
    window.reset( nullptr );
    return 0;
}

static const vxui_demo_archive_entry* vxui_demo_archive_entries_for_category( int category_index, int* out_count, const char* const** out_names )
{
    switch ( std::clamp( category_index, 0, 2 ) ) {
        case 1:
            if ( out_count ) *out_count = ( int ) ( sizeof( VXUI_DEMO_ARCHIVE_FRAMES ) / sizeof( VXUI_DEMO_ARCHIVE_FRAMES[ 0 ] ) );
            if ( out_names ) *out_names = VXUI_DEMO_ARCHIVE_FRAME_NAMES;
            return VXUI_DEMO_ARCHIVE_FRAMES;
        case 2:
            if ( out_count ) *out_count = ( int ) ( sizeof( VXUI_DEMO_ARCHIVE_SIGNALS ) / sizeof( VXUI_DEMO_ARCHIVE_SIGNALS[ 0 ] ) );
            if ( out_names ) *out_names = VXUI_DEMO_ARCHIVE_SIGNAL_NAMES;
            return VXUI_DEMO_ARCHIVE_SIGNALS;
        case 0:
        default:
            if ( out_count ) *out_count = ( int ) ( sizeof( VXUI_DEMO_ARCHIVE_INTEL ) / sizeof( VXUI_DEMO_ARCHIVE_INTEL[ 0 ] ) );
            if ( out_names ) *out_names = VXUI_DEMO_ARCHIVE_INTEL_NAMES;
            return VXUI_DEMO_ARCHIVE_INTEL;
    }
}

static const vxui_demo_main_menu_preview* vxui_demo_current_main_menu_preview( const vxui_demo_app* app )
{
    return vxui_demo_main_menu_preview_from_focused_row( app ? app->main_menu_state.last_focused_row_id : 0u );
}

static vxui_menu_style vxui_demo_menu_style_title_deck( void )
{
    vxui_menu_style style = vxui_demo_make_title_deck_menu_style( VXUI_DEMO_FONT_ROLE_BODY, VXUI_DEMO_FONT_ROLE_TITLE );
    return style;
}

static vxui_menu_style vxui_demo_menu_style_form_deck( float label_lane_width )
{
    vxui_menu_style style = vxui_menu_style_form();
    style.option_wrap_by_default = true;
    style.body_font_id = VXUI_DEMO_FONT_ROLE_BODY;
    style.title_font_id = VXUI_DEMO_FONT_ROLE_SECTION;
    style.badge_font_id = VXUI_DEMO_FONT_ROLE_BODY;
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
    return style;
}

static vxui_menu_style vxui_demo_menu_style_footer_strip( void )
{
    const vxui_demo_command_deck_theme& theme = vxui_demo_command_deck_theme_tokens();
    vxui_menu_style style = vxui_menu_style_footer_strip();
    style.body_font_id = VXUI_DEMO_FONT_ROLE_BODY;
    style.title_font_id = VXUI_DEMO_FONT_ROLE_SECTION;
    style.badge_font_id = VXUI_DEMO_FONT_ROLE_BODY;
    style.section_gap = ( float ) VXUI_DEMO_LAYOUT_SECTION_GAP;
    style.panel_fill_color = theme.utility_fill;
    style.row_border_color = theme.utility_border;
    style.row_focus_border_color = theme.utility_border;
    style.text_color = theme.utility_text;
    style.focused_text_color = theme.utility_text;
    style.secondary_text_color = theme.muted_text;
    style.prompt_text_color = theme.utility_text;
    return style;
}

static void vxui_demo_replace_screen_named( vxui_ctx* ctx, vxui_demo_app* app, const char* screen_name )
{
    if ( !ctx || !app || !screen_name ) {
        return;
    }

    if ( ctx->screen_count > 0 ) {
        vxui_replace_screen( ctx, screen_name );
    } else {
        vxui_push_screen( ctx, screen_name );
    }

    app->show_settings = std::strcmp( screen_name, "settings" ) == 0;
    if ( app->show_settings ) {
        app->settings_body_scroll_current = 0.0f;
        app->settings_body_scroll_target = 0.0f;
        app->settings_body_scroll_velocity = 0.0f;
    }
    if ( std::strcmp( screen_name, "boot" ) == 0 ) app->boot_timer = 0.0f;
    if ( std::strcmp( screen_name, "title" ) == 0 ) app->title_timer = 0.0f;
    if ( std::strcmp( screen_name, "launch_stub" ) == 0 ) app->launch_timer = 0.0f;
    if ( std::strcmp( screen_name, "results_stub" ) == 0 ) app->results_timer = 0.0f;
}

static void vxui_demo_open_title( vxui_ctx* ctx, uint32_t id, void* userdata )
{
    ( void ) id;
    vxui_demo_replace_screen_named( ctx, ( vxui_demo_app* ) userdata, "title" );
}

static void vxui_demo_open_main_menu( vxui_ctx* ctx, uint32_t id, void* userdata )
{
    ( void ) id;
    vxui_demo_replace_screen_named( ctx, ( vxui_demo_app* ) userdata, "main_menu" );
}

static void vxui_demo_open_sortie( vxui_ctx* ctx, uint32_t id, void* userdata )
{
    ( void ) id;
    vxui_demo_replace_screen_named( ctx, ( vxui_demo_app* ) userdata, "sortie" );
}

static void vxui_demo_open_loadout( vxui_ctx* ctx, uint32_t id, void* userdata )
{
    ( void ) id;
    vxui_demo_replace_screen_named( ctx, ( vxui_demo_app* ) userdata, "loadout" );
}

static void vxui_demo_open_archives( vxui_ctx* ctx, uint32_t id, void* userdata )
{
    ( void ) id;
    vxui_demo_replace_screen_named( ctx, ( vxui_demo_app* ) userdata, "archives" );
}

static void vxui_demo_open_records( vxui_ctx* ctx, uint32_t id, void* userdata )
{
    ( void ) id;
    vxui_demo_replace_screen_named( ctx, ( vxui_demo_app* ) userdata, "records" );
}

static void vxui_demo_open_credits( vxui_ctx* ctx, uint32_t id, void* userdata )
{
    ( void ) id;
    vxui_demo_replace_screen_named( ctx, ( vxui_demo_app* ) userdata, "credits" );
}

static void vxui_demo_launch_stub( vxui_ctx* ctx, uint32_t id, void* userdata )
{
    ( void ) id;
    vxui_demo_replace_screen_named( ctx, ( vxui_demo_app* ) userdata, "launch_stub" );
}

static void vxui_demo_open_results_stub( vxui_ctx* ctx, uint32_t id, void* userdata )
{
    ( void ) id;
    vxui_demo_replace_screen_named( ctx, ( vxui_demo_app* ) userdata, "results_stub" );
}

static void vxui_demo_request_quit( vxui_ctx* ctx, uint32_t id, void* userdata )
{
    ( void ) ctx;
    ( void ) id;
    vxui_demo_app* app = ( vxui_demo_app* ) userdata;
    if ( !app ) {
        return;
    }
    app->request_quit = true;
    if ( app->window ) {
        app->window->shouldClose = true;
    }
}

static uint32_t vxui_demo_focus_id_for_screen( vxui_demo_screen_kind screen_kind, const char* focus_name )
{
    const char* focus = focus_name ? focus_name : "";
    switch ( screen_kind ) {
        case VXUI_DEMO_SCREEN_TITLE:
            if ( focus[ 0 ] == '\0' || std::strcmp( focus, "title.enter" ) == 0 || std::strcmp( focus, "enter" ) == 0 ) {
                return vxui_id( "title.enter" );
            }
            break;
        case VXUI_DEMO_SCREEN_MAIN_MENU:
            if ( focus[ 0 ] == '\0' || std::strcmp( focus, "sortie" ) == 0 || std::strcmp( focus, "main.command_menu.sortie" ) == 0 ) return vxui_idi( "main.command_menu", ( int ) vxui_id( "sortie" ) );
            if ( std::strcmp( focus, "loadout" ) == 0 || std::strcmp( focus, "main.command_menu.loadout" ) == 0 ) return vxui_idi( "main.command_menu", ( int ) vxui_id( "loadout" ) );
            if ( std::strcmp( focus, "archives" ) == 0 || std::strcmp( focus, "main.command_menu.archives" ) == 0 ) return vxui_idi( "main.command_menu", ( int ) vxui_id( "archives" ) );
            if ( std::strcmp( focus, "settings" ) == 0 || std::strcmp( focus, "main.command_menu.settings" ) == 0 ) return vxui_idi( "main.command_menu", ( int ) vxui_id( "settings" ) );
            if ( std::strcmp( focus, "records" ) == 0 || std::strcmp( focus, "main.command_menu.records" ) == 0 ) return vxui_idi( "main.command_menu", ( int ) vxui_id( "records" ) );
            if ( std::strcmp( focus, "credits" ) == 0 || std::strcmp( focus, "main.command_menu.credits" ) == 0 ) return vxui_idi( "main.command_menu", ( int ) vxui_id( "credits" ) );
            if ( std::strcmp( focus, "quit" ) == 0 || std::strcmp( focus, "main.command_menu.quit" ) == 0 ) return vxui_idi( "main.command_menu", ( int ) vxui_id( "quit" ) );
            break;
        case VXUI_DEMO_SCREEN_SORTIE:
            if ( focus[ 0 ] == '\0' || std::strcmp( focus, "mission" ) == 0 || std::strcmp( focus, "sortie.menu.mission" ) == 0 ) return vxui_idi( "sortie.menu", ( int ) vxui_id( "mission" ) );
            if ( std::strcmp( focus, "difficulty" ) == 0 || std::strcmp( focus, "sortie.menu.difficulty" ) == 0 ) return vxui_idi( "sortie.menu", ( int ) vxui_id( "difficulty" ) );
            if ( std::strcmp( focus, "start" ) == 0 || std::strcmp( focus, "sortie.menu.start" ) == 0 ) return vxui_idi( "sortie.menu", ( int ) vxui_id( "start" ) );
            if ( std::strcmp( focus, "back" ) == 0 || std::strcmp( focus, "sortie.menu.back" ) == 0 ) return vxui_idi( "sortie.menu", ( int ) vxui_id( "back" ) );
            break;
        case VXUI_DEMO_SCREEN_LOADOUT:
            if ( focus[ 0 ] == '\0' || std::strcmp( focus, "ship" ) == 0 || std::strcmp( focus, "loadout.menu.ship" ) == 0 ) return vxui_idi( "loadout.menu", ( int ) vxui_id( "ship" ) );
            if ( std::strcmp( focus, "primary" ) == 0 || std::strcmp( focus, "loadout.menu.primary" ) == 0 ) return vxui_idi( "loadout.menu", ( int ) vxui_id( "primary" ) );
            if ( std::strcmp( focus, "support" ) == 0 || std::strcmp( focus, "loadout.menu.support" ) == 0 ) return vxui_idi( "loadout.menu", ( int ) vxui_id( "support" ) );
            if ( std::strcmp( focus, "system" ) == 0 || std::strcmp( focus, "loadout.menu.system" ) == 0 ) return vxui_idi( "loadout.menu", ( int ) vxui_id( "system" ) );
            if ( std::strcmp( focus, "back" ) == 0 || std::strcmp( focus, "loadout.menu.back" ) == 0 ) return vxui_idi( "loadout.menu", ( int ) vxui_id( "back" ) );
            break;
        case VXUI_DEMO_SCREEN_ARCHIVES:
            if ( focus[ 0 ] == '\0' || std::strcmp( focus, "category" ) == 0 || std::strcmp( focus, "archives.menu.category" ) == 0 ) return vxui_idi( "archives.menu", ( int ) vxui_id( "category" ) );
            if ( std::strcmp( focus, "entry" ) == 0 || std::strcmp( focus, "archives.menu.entry" ) == 0 ) return vxui_idi( "archives.menu", ( int ) vxui_id( "entry" ) );
            if ( std::strcmp( focus, "back" ) == 0 || std::strcmp( focus, "archives.menu.back" ) == 0 ) return vxui_idi( "archives.menu", ( int ) vxui_id( "back" ) );
            break;
        case VXUI_DEMO_SCREEN_SETTINGS:
            if ( focus[ 0 ] == '\0' || std::strcmp( focus, "challenge" ) == 0 || std::strcmp( focus, "settings.body_menu.challenge" ) == 0 ) return vxui_idi( "settings.body_menu", ( int ) vxui_id( "challenge" ) );
            if ( std::strcmp( focus, "volume" ) == 0 || std::strcmp( focus, "settings.body_menu.volume" ) == 0 ) return vxui_idi( "settings.body_menu", ( int ) vxui_id( "volume" ) );
            if ( std::strcmp( focus, "scanlines" ) == 0 || std::strcmp( focus, "settings.body_menu.scanlines" ) == 0 ) return vxui_idi( "settings.body_menu", ( int ) vxui_id( "scanlines" ) );
            if ( std::strcmp( focus, "effect_intensity" ) == 0 || std::strcmp( focus, "settings.body_menu.effect_intensity" ) == 0 ) return vxui_idi( "settings.body_menu", ( int ) vxui_id( "effect_intensity" ) );
            if ( std::strcmp( focus, "prompt_table" ) == 0 || std::strcmp( focus, "settings.body_menu.prompt_table" ) == 0 ) return vxui_idi( "settings.body_menu", ( int ) vxui_id( "prompt_table" ) );
            if ( std::strcmp( focus, "locale_index" ) == 0 || std::strcmp( focus, "settings.body_menu.locale_index" ) == 0 ) return vxui_idi( "settings.body_menu", ( int ) vxui_id( "locale_index" ) );
            if ( std::strcmp( focus, "back" ) == 0 || std::strcmp( focus, "settings.back" ) == 0 ) return vxui_id( "settings.back" );
            if ( std::strcmp( focus, "defaults" ) == 0 || std::strcmp( focus, "settings.defaults" ) == 0 ) return vxui_id( "settings.defaults" );
            break;
        case VXUI_DEMO_SCREEN_RECORDS:
            if ( focus[ 0 ] == '\0' || std::strcmp( focus, "board" ) == 0 || std::strcmp( focus, "records.menu.board" ) == 0 ) return vxui_idi( "records.menu", ( int ) vxui_id( "board" ) );
            if ( std::strcmp( focus, "run" ) == 0 || std::strcmp( focus, "records.menu.run" ) == 0 ) return vxui_idi( "records.menu", ( int ) vxui_id( "run" ) );
            if ( std::strcmp( focus, "back" ) == 0 || std::strcmp( focus, "records.menu.back" ) == 0 ) return vxui_idi( "records.menu", ( int ) vxui_id( "back" ) );
            break;
        case VXUI_DEMO_SCREEN_CREDITS:
            if ( focus[ 0 ] == '\0' || std::strcmp( focus, "back" ) == 0 || std::strcmp( focus, "credits.back" ) == 0 ) return vxui_id( "credits.back" );
            break;
        case VXUI_DEMO_SCREEN_RESULTS_STUB:
            if ( focus[ 0 ] == '\0' || std::strcmp( focus, "return" ) == 0 || std::strcmp( focus, "results_stub.return" ) == 0 ) return vxui_id( "results_stub.return" );
            break;
        case VXUI_DEMO_SCREEN_BOOT:
        case VXUI_DEMO_SCREEN_LAUNCH_STUB:
        case VXUI_DEMO_SCREEN_UNKNOWN:
        default:
            break;
    }
    return 0u;
}

static bool vxui_demo_apply_shot_request( vxui_demo_app* app, vxui_ctx* ctx, const vxui_demo_shot_request& request, char* error, size_t error_size )
{
    if ( !app || !ctx || !request.enabled ) {
        std::snprintf( error, error_size, "%s", "invalid shot request" );
        return false;
    }

    const vxui_demo_screen_kind screen_kind = vxui_demo_screen_kind_from_name( request.screen_name );
    if ( screen_kind == VXUI_DEMO_SCREEN_UNKNOWN ) {
        std::snprintf( error, error_size, "%s", "shot request referenced an unknown screen" );
        return false;
    }

    app->shot_mode = true;
    app->shot_force_compact_layout = request.compact_override;
    app->shot_layout_surface_max_height_override = request.compact_override
        ? std::max( 0.0f, 648.0f - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f )
        : 0.0f;
    app->request_quit = false;
    app->show_settings = false;
    app->boot_timer = 0.0f;
    app->title_timer = 0.0f;
    app->launch_timer = 0.0f;
    app->results_timer = 0.0f;
    app->settings_body_scroll_current = 0.0f;
    app->settings_body_scroll_target = 0.0f;
    app->settings_body_scroll_velocity = 0.0f;
    app->scanline_index = request.disable_scanline ? 0 : 1;

    vxui_demo_apply_locale_index( ctx, app, request.locale_index );
    vxui_demo_apply_prompt_table_index( ctx, app, request.prompt_table_index );

    ctx->active_seq_count = 0;
    ctx->focused_id = 0;
    ctx->pending_focus_id = 0;
    ctx->focus_ring_state.valid = false;

    vxui_demo_replace_screen_named( ctx, app, request.screen_name );
    if ( ctx->screen_count > 0 ) {
        vxui_screen* live = &ctx->screens[ ctx->screen_count - 1 ];
        live->state = VXUI_SCREEN_ACTIVE;
        live->state_started_ms = ctx->elapsed_ms;
    }
    ctx->active_seq_count = 0;

    const uint32_t focus_id = vxui_demo_focus_id_for_screen( screen_kind, request.focus_id.empty() ? nullptr : request.focus_id.c_str() );
    if ( !request.focus_id.empty() && focus_id == 0u ) {
        std::snprintf( error, error_size, "unsupported --focus value for screen '%s'", request.screen_name );
        return false;
    }
    if ( focus_id != 0u ) {
        vxui_set_focus( ctx, focus_id );
        switch ( screen_kind ) {
            case VXUI_DEMO_SCREEN_MAIN_MENU:
                app->main_menu_state.last_focused_row_id = focus_id;
                break;
            case VXUI_DEMO_SCREEN_SORTIE:
                app->sortie_menu_state.last_focused_row_id = focus_id;
                break;
            case VXUI_DEMO_SCREEN_LOADOUT:
                app->loadout_menu_state.last_focused_row_id = focus_id;
                break;
            case VXUI_DEMO_SCREEN_ARCHIVES:
                app->archives_menu_state.last_focused_row_id = focus_id;
                break;
            case VXUI_DEMO_SCREEN_SETTINGS:
                if ( std::strncmp( request.focus_id.c_str(), "settings.", 9 ) != 0 ) {
                    app->settings_menu_state.last_focused_row_id = focus_id;
                }
                break;
            case VXUI_DEMO_SCREEN_RECORDS:
                app->records_menu_state.last_focused_row_id = focus_id;
                break;
            default:
                break;
        }
    }

    return true;
}

static void vxui_demo_restore_defaults_action( vxui_ctx* ctx, uint32_t id, void* userdata )
{
    ( void ) id;
    vxui_demo_restore_defaults( ctx, ( vxui_demo_app* ) userdata );
}

static void vxui_demo_locale_option_changed( vxui_ctx* ctx, int value, void* userdata )
{
    vxui_demo_apply_locale_index( ctx, ( vxui_demo_app* ) userdata, value );
}

static void vxui_demo_prompt_table_changed( vxui_ctx* ctx, int value, void* userdata )
{
    vxui_demo_apply_prompt_table_index( ctx, ( vxui_demo_app* ) userdata, value );
}

static void vxui_demo_emit_stat_bar( vxui_ctx* ctx, const char* id, const char* label, float value )
{
    const vxui_demo_command_deck_theme& theme = vxui_demo_command_deck_theme_tokens();
    value = std::clamp( value, 0.0f, 1.0f );
    const std::string row_id = std::string( id ) + ".row";
    const std::string fill_id = std::string( id ) + ".fill";
    VXUI_HASH( ctx, vxui_id( row_id.c_str() ), {
        .layout = {
            .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
            .childGap = 8,
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
    } ) {
        VXUI_LABEL( ctx, label, ( vxui_label_cfg ) {
            .font_id = VXUI_DEMO_FONT_ROLE_BODY,
            .font_size = 18.0f,
            .color = theme.muted_text,
        } );
        VXUI_HASH( ctx, vxui_id( id ), {
            .layout = {
                .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIXED( 12 ) },
            },
            .backgroundColor = vxui_demo_clay_color( theme.stat_track ),
            .cornerRadius = CLAY_CORNER_RADIUS( 6 ),
        } ) {
            VXUI_HASH( ctx, vxui_id( fill_id.c_str() ), {
                .layout = {
                    .sizing = { CLAY_SIZING_FIXED( value * 220.0f ), CLAY_SIZING_GROW( 0 ) },
                },
                .backgroundColor = vxui_demo_clay_color( theme.stat_fill ),
                .cornerRadius = CLAY_CORNER_RADIUS( 6 ),
            } ) {}
        }
    }
}

static void vxui_demo_emit_surface_scanline( vxui_ctx* ctx, const char* root_id )
{
    const vxui_demo_command_deck_theme& theme = vxui_demo_command_deck_theme_tokens();
    vxui_demo_decl_scope root_decl( ctx, vxui_id( root_id ) );
    VXUI_TRAIT( VXUI_TRAIT_SCANLINE, ( vxui_demo_scanline ) { .spacing = 10.0f, .alpha = theme.surface_scanline_alpha * 0.45f } );
}

static void vxui_demo_render_boot_screen( vxui_demo_app* app, vxui_ctx* ctx )
{
    const vxui_demo_command_deck_theme& theme = vxui_demo_command_deck_theme_tokens();
    const bool background_scanline = app ? app->scanline_index != 0 : true;
    const float viewport_width = std::max( 0.0f, ( float ) ctx->cfg.screen_width - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f );
    const vxui_demo_surface_metrics surface_metrics = vxui_demo_compute_surface_metrics( viewport_width, ctx->locale, VXUI_DEMO_SURFACE_BOOT );
    const float surface_max_height = std::max( 0.0f, ( float ) ctx->cfg.screen_height - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f );

    if ( background_scanline ) {
        vxui_demo_emit_surface_scanline( ctx, "boot" );
    }
    vxui_menu_surface_cfg surface_cfg = vxui_menu_surface_cfg_default(
        surface_metrics.surface_width,
        surface_max_height,
        theme.app_background_base,
        theme.primary_panel_fill,
        theme.primary_panel_border );
    vxui_menu_surface_begin( ctx, "boot", "boot.surface", &surface_cfg );
    {
        const std::string screen_count_text = std::to_string( ctx ? ctx->screen_count : 0 );
        vxui_menu_prompt_item footer_prompts[] = {
            { "action.confirm", "boot.line.2", false, "boot.footer.prompt.confirm" },
        };
        vxui_menu_status_item footer_status[] = {
            { "status.short.locale", vxui_demo_footer_locale_key( app ? app->locale_index : 0 ), VXUI_MENU_STATUS_PRIMARY, false, false, "boot.footer.status.locale" },
            { "status.short.prompts", vxui_demo_footer_prompt_key( app ? app->prompt_table_index : 0 ), VXUI_MENU_STATUS_SECONDARY, true, false, "boot.footer.status.prompts" },
            { "status.short.screens", screen_count_text.c_str(), VXUI_MENU_STATUS_SECONDARY, true, false, "boot.footer.status.screens" },
            { "status.short.top", vxui_demo_footer_top_name( app, ctx ), VXUI_MENU_STATUS_PRIMARY, false, false, "boot.footer.status.top" },
        };
        vxui_menu_footer_cfg footer_cfg = {
            footer_prompts,
            ( int ) ( sizeof( footer_prompts ) / sizeof( footer_prompts[ 0 ] ) ),
            footer_status,
            ( int ) ( sizeof( footer_status ) / sizeof( footer_status[ 0 ] ) ),
            VXUI_MENU_SHELL_COMPACT_AUTO,
            surface_max_height <= 680.0f ? 3 : 4,
            false,
        };
        vxui_menu_style footer_style = vxui_demo_menu_style_footer_strip();
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

        vxui_menu_screen_begin( ctx, &shell_state, "boot.shell", &screen_cfg );
        VXUI_LABEL( ctx, "boot.line.0", ( vxui_label_cfg ) {
            .font_id = VXUI_DEMO_FONT_ROLE_TITLE,
            .font_size = 42.0f,
            .color = theme.title_text,
        } );
        {
            vxui_demo_decl_scope boot_title_decl( ctx, vxui_id( "boot.line.0" ) );
            VXUI_TRAIT( VXUI_TRAIT_PULSE, ( vxui_demo_pulse ) { .speed = 1.6f, .scale = 0.03f, .alpha = 0.10f } );
        }
        VXUI_LABEL( ctx, "boot.line.1", ( vxui_label_cfg ) {
            .font_id = VXUI_DEMO_FONT_ROLE_SECTION,
            .font_size = 26.0f,
            .color = theme.accent_cool,
        } );
        vxui_demo_emit_stat_bar( ctx, "boot.bar.sync", "Link Sync", 0.92f );
        vxui_demo_emit_stat_bar( ctx, "boot.bar.safety", "Safety Envelope", 0.78f );
        vxui_demo_emit_stat_bar( ctx, "boot.bar.radar", "Radar Uplink", 0.84f );
        vxui_menu_footer( ctx, "boot.footer", &screen_cfg.footer );
        vxui_menu_screen_end( ctx, &shell_state );
    }
    vxui_menu_surface_end( ctx );
}

static void vxui_demo_render_title_screen( vxui_demo_app* app, vxui_ctx* ctx, const vxui_demo_renderer* renderer )
{
    const vxui_demo_command_deck_theme& theme = vxui_demo_command_deck_theme_tokens();
    const bool background_scanline = app ? app->scanline_index != 0 : true;
    const float control_height = vxui_demo_control_height( renderer, ctx->locale );
    const float viewport_width = std::max( 0.0f, ( float ) ctx->cfg.screen_width - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f );
    const vxui_demo_surface_metrics surface_metrics = vxui_demo_compute_surface_metrics( viewport_width, ctx->locale, VXUI_DEMO_SURFACE_TITLE );
    const float surface_max_height = std::max( 0.0f, ( float ) ctx->cfg.screen_height - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f );

    if ( background_scanline ) {
        vxui_demo_emit_surface_scanline( ctx, "title" );
    }
    vxui_menu_surface_cfg surface_cfg = vxui_menu_surface_cfg_default(
        surface_metrics.surface_width,
        surface_max_height,
        theme.app_background_base,
        theme.primary_panel_fill,
        theme.primary_panel_border );
    vxui_menu_surface_begin( ctx, "title", "title.surface", &surface_cfg );
    {
        const std::string screen_count_text = std::to_string( ctx ? ctx->screen_count : 0 );
        vxui_menu_status_item footer_status[] = {
            { "status.short.locale", vxui_demo_footer_locale_key( app ? app->locale_index : 0 ), VXUI_MENU_STATUS_PRIMARY, false, false, "title.footer.status.locale" },
            { "status.short.prompts", vxui_demo_footer_prompt_key( app ? app->prompt_table_index : 0 ), VXUI_MENU_STATUS_SECONDARY, true, false, "title.footer.status.prompts" },
            { "status.short.screens", screen_count_text.c_str(), VXUI_MENU_STATUS_SECONDARY, true, false, "title.footer.status.screens" },
            { "status.short.top", vxui_demo_footer_top_name( app, ctx ), VXUI_MENU_STATUS_PRIMARY, false, false, "title.footer.status.top" },
        };
        vxui_menu_footer_cfg footer_cfg = {
            nullptr,
            0,
            footer_status,
            ( int ) ( sizeof( footer_status ) / sizeof( footer_status[ 0 ] ) ),
            VXUI_MENU_SHELL_COMPACT_AUTO,
            surface_max_height <= 680.0f ? 3 : 4,
            false,
        };
        vxui_menu_style footer_style = vxui_demo_menu_style_footer_strip();
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

        vxui_menu_screen_begin( ctx, &shell_state, "title.shell", &screen_cfg );
        VXUI( ctx, "title.hero", {
            .layout = {
                .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                .padding = CLAY_PADDING_ALL( 24 ),
                .childGap = 12,
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
            .backgroundColor = vxui_demo_clay_color( theme.hero_surface_fill ),
            .cornerRadius = CLAY_CORNER_RADIUS( 14 ),
            .border = vxui_demo_panel_border( theme.hero_surface_border, 1 ),
        } ) {
            VXUI_LABEL( ctx, "menu.main", ( vxui_label_cfg ) {
                .font_id = VXUI_DEMO_FONT_ROLE_TITLE,
                .font_size = 56.0f,
                .color = theme.title_text,
            } );
            {
                vxui_demo_decl_scope title_lockup_decl( ctx, vxui_id( "menu.main" ) );
                VXUI_TRAIT( VXUI_TRAIT_GLOW, ( vxui_demo_glow ) { .padding = 10.0f, .alpha = 0.18f } );
            }
            VXUI_LABEL( ctx, "title.subtitle", ( vxui_label_cfg ) {
                .font_id = VXUI_DEMO_FONT_ROLE_SECTION,
                .font_size = 28.0f,
                .color = theme.accent_cool,
            } );
            VXUI_LABEL( ctx, "title.status", ( vxui_label_cfg ) {
                .font_id = VXUI_DEMO_FONT_ROLE_BODY,
                .font_size = 20.0f,
                .color = theme.muted_text,
            } );
        }

        VXUI( ctx, "title.action_band", {
            .layout = {
                .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                .padding = CLAY_PADDING_ALL( 18 ),
                .childGap = 12,
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
            .backgroundColor = vxui_demo_clay_color( theme.utility_fill ),
            .cornerRadius = CLAY_CORNER_RADIUS( 12 ),
            .border = vxui_demo_panel_border( theme.utility_border, 1 ),
        } ) {
            vxui_demo_emit_action_button( ctx, "title.enter", "title.prompt", vxui_demo_open_main_menu, ( vxui_action_cfg ) {
                .userdata = app,
            }, control_height );
            {
                vxui_demo_decl_scope title_enter_decl( ctx, vxui_id( "title.enter" ) );
                VXUI_TRAIT( VXUI_TRAIT_GLOW, ( vxui_demo_glow ) { .padding = 6.0f, .alpha = 0.18f } );
            }
        }
        vxui_menu_footer( ctx, "title.footer", &screen_cfg.footer );
        vxui_menu_screen_end( ctx, &shell_state );
    }
    vxui_menu_surface_end( ctx );
}

static void vxui_demo_render_main_menu_screen( vxui_demo_app* app, vxui_ctx* ctx )
{
    const vxui_demo_command_deck_theme& theme = vxui_demo_command_deck_theme_tokens();
    const bool rtl = ctx->rtl;
    const bool background_scanline = app ? app->scanline_index != 0 : true;
    const float viewport_width = std::max( 0.0f, ( float ) ctx->cfg.screen_width - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f );
    const float surface_max_height = std::max( 0.0f, ( float ) ctx->cfg.screen_height - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f );
    const float layout_surface_max_height =
        app && app->shot_layout_surface_max_height_override > 0.0f
        ? std::min( surface_max_height, app->shot_layout_surface_max_height_override )
        : surface_max_height;
    const vxui_demo_main_menu_layout_spec layout = vxui_demo_resolve_main_menu_layout( viewport_width, layout_surface_max_height, ctx->locale );
    vxui_menu_style menu_style = vxui_demo_menu_style_title_deck();
    if ( layout.surface_max_height <= 650.0f ) {
        menu_style.body_font_size = 14.0f;
        menu_style.secondary_font_size = 12.0f;
        menu_style.badge_font_size = 8.0f;
        menu_style.row_height = 25.0f;
        menu_style.row_gap = 0.0f;
        menu_style.section_gap = 2.0f;
        menu_style.padding_y = 3.0f;
    }
    vxui_menu_style command_menu_style = menu_style;
    command_menu_style.panel_fill_color = { 0, 0, 0, 1 };

    if ( background_scanline ) {
        vxui_demo_emit_surface_scanline( ctx, "main_menu" );
    }
    vxui_menu_surface_cfg surface_cfg = vxui_menu_surface_cfg_default(
        layout.surface.surface_width,
        layout.surface_max_height,
        theme.app_background_base,
        theme.primary_panel_fill,
        theme.primary_panel_border );
    vxui_menu_surface_begin( ctx, "main_menu", "main.surface", &surface_cfg );
    {
        const vxui_demo_main_menu_preview* preview = vxui_demo_current_main_menu_preview( app );
        vxui_demo_emit_main_menu_shell(
            ctx,
            rtl,
            ctx->locale,
            layout,
            ( vxui_demo_main_menu_visuals ) {
                VXUI_DEMO_FONT_ROLE_BODY,
                VXUI_DEMO_FONT_ROLE_TITLE,
                VXUI_DEMO_FONT_ROLE_SECTION,
            },
            ( vxui_demo_main_menu_shell_copy ) {
                "menu.main",
                "main.status_banner",
                "main.preview",
                "status.short.locale",
                vxui_demo_footer_locale_key( app ? app->locale_index : 0 ),
                "status.short.prompts",
                vxui_demo_footer_prompt_key( app ? app->prompt_table_index : 0 ),
                "status.short.screens",
                ctx ? ctx->screen_count : 0,
                "status.short.top",
                vxui_demo_footer_top_name( app, ctx ),
                "menu.confirm",
                "menu.cancel",
            },
            *preview,
            [&]( float viewport_height ) {
                vxui_menu_begin( ctx, &app->main_menu_state, "main.command_menu", ( vxui_menu_cfg ) {
                    .style = &command_menu_style,
                    .viewport_height = viewport_height,
                } );
                vxui_menu_action( ctx, &app->main_menu_state, "sortie", "menu.sortie", vxui_demo_open_sortie, ( vxui_menu_row_cfg ) {
                    .badge_text_key = "badge.recommended",
                }, ( vxui_action_cfg ) { .userdata = app } );
                vxui_menu_action( ctx, &app->main_menu_state, "loadout", "menu.loadout", vxui_demo_open_loadout, ( vxui_menu_row_cfg ) {
                    .badge_text_key = "badge.demo",
                }, ( vxui_action_cfg ) { .userdata = app } );
                vxui_menu_action( ctx, &app->main_menu_state, "archives", "menu.archives", vxui_demo_open_archives, ( vxui_menu_row_cfg ) {
                    .badge_text_key = "badge.demo",
                }, ( vxui_action_cfg ) { .userdata = app } );
                vxui_menu_action( ctx, &app->main_menu_state, "settings", "menu.settings", vxui_demo_open_settings, ( vxui_menu_row_cfg ) {
                    .badge_text_key = "badge.recommended",
                }, ( vxui_action_cfg ) { .userdata = app } );
                vxui_menu_action( ctx, &app->main_menu_state, "records", "menu.records", vxui_demo_open_records, ( vxui_menu_row_cfg ) {
                    .badge_text_key = "badge.demo",
                }, ( vxui_action_cfg ) { .userdata = app } );
                vxui_menu_action( ctx, &app->main_menu_state, "credits", "menu.credits", vxui_demo_open_credits, ( vxui_menu_row_cfg ) {
                    .badge_text_key = "badge.demo",
                }, ( vxui_action_cfg ) { .userdata = app } );
                vxui_menu_action( ctx, &app->main_menu_state, "quit", "menu.quit_demo", vxui_demo_request_quit, ( vxui_menu_row_cfg ) { 0 }, ( vxui_action_cfg ) {
                    .userdata = app,
                } );
                vxui_menu_end( ctx, &app->main_menu_state );
            } );
    }
    vxui_menu_surface_end( ctx );
}

static void vxui_demo_render_sortie_screen( vxui_demo_app* app, vxui_ctx* ctx )
{
    vxui_demo_render_sortie_screen_shared( ctx,
        ( vxui_demo_split_deck_visuals ) { VXUI_DEMO_FONT_ROLE_BODY, VXUI_DEMO_FONT_ROLE_TITLE, VXUI_DEMO_FONT_ROLE_SECTION },
        ( vxui_demo_sortie_screen_cfg ) {
            .menu_state = &app->sortie_menu_state,
            .selected_mission_index = &app->selected_mission_index,
            .difficulty_index = &app->difficulty,
            .layout_surface_max_height_override = app ? app->shot_layout_surface_max_height_override : 0.0f,
            .background_scanline = app ? app->scanline_index != 0 : true,
            .start_fn = vxui_demo_launch_stub,
            .start_cfg = ( vxui_action_cfg ) { .userdata = app },
            .back_fn = vxui_demo_open_main_menu,
            .back_cfg = ( vxui_action_cfg ) { .userdata = app },
            .status = {
                vxui_demo_footer_locale_key( app ? app->locale_index : 0 ),
                vxui_demo_footer_prompt_key( app ? app->prompt_table_index : 0 ),
                vxui_demo_footer_top_name( app, ctx ),
                ctx ? ctx->screen_count : 0,
            },
        } );
}

static void vxui_demo_render_loadout_screen( vxui_demo_app* app, vxui_ctx* ctx )
{
    vxui_demo_render_loadout_screen_shared( ctx,
        ( vxui_demo_split_deck_visuals ) { VXUI_DEMO_FONT_ROLE_BODY, VXUI_DEMO_FONT_ROLE_TITLE, VXUI_DEMO_FONT_ROLE_SECTION },
        ( vxui_demo_loadout_screen_cfg ) {
            .menu_state = &app->loadout_menu_state,
            .selected_ship_index = &app->selected_ship_index,
            .selected_primary_index = &app->selected_primary_index,
            .selected_support_index = &app->selected_support_index,
            .selected_system_index = &app->selected_system_index,
            .layout_surface_max_height_override = app ? app->shot_layout_surface_max_height_override : 0.0f,
            .background_scanline = app ? app->scanline_index != 0 : true,
            .back_fn = vxui_demo_open_main_menu,
            .back_cfg = ( vxui_action_cfg ) { .userdata = app },
            .status = {
                vxui_demo_footer_locale_key( app ? app->locale_index : 0 ),
                vxui_demo_footer_prompt_key( app ? app->prompt_table_index : 0 ),
                vxui_demo_footer_top_name( app, ctx ),
                ctx ? ctx->screen_count : 0,
            },
        } );
}

static void vxui_demo_render_archives_screen( vxui_demo_app* app, vxui_ctx* ctx )
{
    vxui_demo_render_archives_screen_shared( ctx,
        ( vxui_demo_split_deck_visuals ) { VXUI_DEMO_FONT_ROLE_BODY, VXUI_DEMO_FONT_ROLE_TITLE, VXUI_DEMO_FONT_ROLE_SECTION },
        ( vxui_demo_archives_screen_cfg ) {
            .menu_state = &app->archives_menu_state,
            .archive_category_index = &app->archive_category_index,
            .archive_entry_index = &app->archive_entry_index,
            .layout_surface_max_height_override = app ? app->shot_layout_surface_max_height_override : 0.0f,
            .background_scanline = app ? app->scanline_index != 0 : true,
            .back_fn = vxui_demo_open_main_menu,
            .back_cfg = ( vxui_action_cfg ) { .userdata = app },
            .status = {
                vxui_demo_footer_locale_key( app ? app->locale_index : 0 ),
                vxui_demo_footer_prompt_key( app ? app->prompt_table_index : 0 ),
                vxui_demo_footer_top_name( app, ctx ),
                ctx ? ctx->screen_count : 0,
            },
        } );
}

static void vxui_demo_render_settings_screen( vxui_demo_app* app, vxui_ctx* ctx, const vxui_demo_renderer* renderer )
{
    const vxui_demo_command_deck_theme& theme = vxui_demo_command_deck_theme_tokens();
    const bool rtl = ctx->rtl;
    const bool background_scanline = app ? app->scanline_index != 0 : true;
    ( void ) renderer;
    const float viewport_width = std::max( 0.0f, ( float ) ctx->cfg.screen_width - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f );
    const float surface_max_height = std::max( 0.0f, ( float ) ctx->cfg.screen_height - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f );
    const float layout_surface_max_height =
        app && app->shot_layout_surface_max_height_override > 0.0f
        ? std::min( surface_max_height, app->shot_layout_surface_max_height_override )
        : surface_max_height;
    const vxui_demo_settings_layout_spec layout = vxui_demo_resolve_settings_layout( viewport_width, layout_surface_max_height, ctx->locale );
    const vxui_demo_surface_metrics& surface_metrics = layout.surface;
    vxui_menu_style form_style = vxui_demo_menu_style_form_deck( surface_metrics.label_lane_width );
    form_style.body_font_size = 16.0f;
    form_style.secondary_font_size = 13.0f;
    form_style.title_font_size = 20.0f;
    form_style.badge_font_size = 11.0f;
    form_style.row_height = 36.0f;
    form_style.row_gap = 3.0f;
    form_style.section_gap = 10.0f;
    form_style.padding_x = 12.0f;
    form_style.padding_y = 8.0f;
    form_style.lane_gap = 12.0f;
    const float settings_viewport_height = std::max( layout.menu_viewport_height, layout.surface_max_height - 220.0f );
    vxui_menu_style body_menu_style = form_style;
    body_menu_style.panel_fill_color = { 0, 0, 0, 1 };

    if ( background_scanline ) {
        vxui_demo_emit_surface_scanline( ctx, "settings" );
    }
    vxui_menu_surface_cfg surface_cfg = vxui_menu_surface_cfg_default(
        surface_metrics.surface_width,
        layout.surface_max_height,
        theme.app_background_base,
        theme.primary_panel_fill,
        theme.primary_panel_border );
    vxui_menu_surface_begin( ctx, "settings", "settings.surface", &surface_cfg );
    {
        const std::string settings_screen_count = std::to_string( ctx ? ctx->screen_count : 0 );
        vxui_menu_prompt_item footer_prompts[] = {
            { "action.confirm", "menu.confirm", false, "settings.prompt.confirm" },
            { "action.cancel", "menu.cancel", false, "settings.prompt.cancel" },
        };
        vxui_menu_status_item footer_status[] = {
            { "status.short.locale", vxui_demo_footer_locale_key( app ? app->locale_index : 0 ), VXUI_MENU_STATUS_PRIMARY, false, false, "settings.footer.status.locale" },
            { "status.short.prompts", vxui_demo_footer_prompt_key( app ? app->prompt_table_index : 0 ), VXUI_MENU_STATUS_SECONDARY, true, false, "settings.footer.status.prompts" },
            { "status.short.screens", settings_screen_count.c_str(), VXUI_MENU_STATUS_SECONDARY, true, false, "settings.footer.status.screens" },
            { "status.short.top", vxui_demo_footer_top_name( app, ctx ), VXUI_MENU_STATUS_PRIMARY, false, false, "settings.footer.status.top" },
        };
        vxui_menu_footer_cfg footer_cfg = {
            footer_prompts,
            ( int ) ( sizeof( footer_prompts ) / sizeof( footer_prompts[ 0 ] ) ),
            footer_status,
            ( int ) ( sizeof( footer_status ) / sizeof( footer_status[ 0 ] ) ),
            VXUI_MENU_SHELL_COMPACT_AUTO,
            layout.surface_max_height <= 680.0f ? 3 : 4,
            false,
        };
        vxui_menu_screen_cfg screen_cfg = {
            VXUI_MENU_SHELL_FORM,
            &form_style,
            VXUI_MENU_SHELL_COMPACT_AUTO,
            680.0f,
            0.0f,
            false,
            { "menu.settings", "Carry-forward shell controls and layout-safe menu scrolling.", false },
            { 0.0f, surface_metrics.content_width, true, false, false },
            { 0.0f, 0.0f, false, false, true },
            { 0.0f, 0.0f, false, true, true },
            {},
            footer_cfg,
        };
        vxui_menu_state shell_state = {};
        vxui_menu_screen_begin( ctx, &shell_state, "settings.shell", &screen_cfg );
        vxui_menu_header( ctx, "settings.header", &screen_cfg.header );
        vxui_menu_primary_lane_begin( ctx, "settings.body_panel", &screen_cfg.primary_lane );
        
            vxui_menu_begin( ctx, &app->settings_menu_state, "settings.body_menu", ( vxui_menu_cfg ) {
                .style = &body_menu_style,
                .viewport_height = settings_viewport_height,
            } );
            vxui_menu_section( ctx, &app->settings_menu_state, "interface", "menu.interface", ( vxui_menu_section_cfg ) { 0 } );
            vxui_menu_option( ctx, &app->settings_menu_state, "challenge", "menu.challenge", &app->difficulty, const_cast< const char** >( VXUI_DEMO_DIFFICULTY_KEYS ), 3, ( vxui_menu_row_cfg ) {
                .badge_text_key = "badge.recommended",
            }, ( vxui_option_cfg ) { 0 } );
            vxui_menu_section( ctx, &app->settings_menu_state, "audio", "menu.audio", ( vxui_menu_section_cfg ) { 0 } );
            vxui_menu_slider( ctx, &app->settings_menu_state, "volume", "menu.volume", &app->volume, 0.0f, 1.0f, ( vxui_menu_row_cfg ) { 0 }, ( vxui_slider_cfg ) {
                .show_value = true,
                .format = "%.2f",
            } );
            vxui_menu_section( ctx, &app->settings_menu_state, "visual_fx", "menu.visual_fx", ( vxui_menu_section_cfg ) { 0 } );
            vxui_menu_option( ctx, &app->settings_menu_state, "scanlines", "menu.scanlines", &app->scanline_index, const_cast< const char** >( VXUI_DEMO_BOOL_KEYS ), 2, ( vxui_menu_row_cfg ) { 0 }, ( vxui_option_cfg ) { 0 } );
            vxui_menu_option( ctx, &app->settings_menu_state, "effect_intensity", "menu.effect_intensity", &app->effect_intensity_index, const_cast< const char** >( VXUI_DEMO_EFFECT_KEYS ), 3, ( vxui_menu_row_cfg ) { 0 }, ( vxui_option_cfg ) { 0 } );
            vxui_menu_section( ctx, &app->settings_menu_state, "input_prompts", "menu.input_prompts", ( vxui_menu_section_cfg ) { 0 } );
            vxui_menu_option( ctx, &app->settings_menu_state, "prompt_table", "menu.prompts", &app->prompt_table_index, const_cast< const char** >( VXUI_DEMO_PROMPT_TABLE_KEYS ), 2, ( vxui_menu_row_cfg ) {
                .secondary_key = "Hotkeys still work globally.",
            }, ( vxui_option_cfg ) {
                .on_change = vxui_demo_prompt_table_changed,
                .userdata = app,
            } );
            vxui_menu_section( ctx, &app->settings_menu_state, "locale", "menu.locale", ( vxui_menu_section_cfg ) { 0 } );
            vxui_menu_option( ctx, &app->settings_menu_state, "locale_index", "menu.locale", &app->locale_index, const_cast< const char** >( VXUI_DEMO_LOCALE_OPTION_KEYS ), 3, ( vxui_menu_row_cfg ) { 0 }, ( vxui_option_cfg ) {
                .on_change = vxui_demo_locale_option_changed,
                .userdata = app,
            } );
            vxui_menu_section( ctx, &app->settings_menu_state, "actions", "menu.confirm", ( vxui_menu_section_cfg ) {
                .secondary_key = "Quick actions",
            } );
            vxui_menu_action( ctx, &app->settings_menu_state, "defaults", "menu.restore_defaults", vxui_demo_restore_defaults_action, ( vxui_menu_row_cfg ) { 0 }, ( vxui_action_cfg ) {
                .userdata = app,
            } );
            vxui_menu_action( ctx, &app->settings_menu_state, "back", "menu.return_command_deck", vxui_demo_close_settings, ( vxui_menu_row_cfg ) { 0 }, ( vxui_action_cfg ) {
                .userdata = app,
            } );
            vxui_menu_end( ctx, &app->settings_menu_state );
        vxui_menu_primary_lane_end( ctx );
        vxui_menu_footer( ctx, "settings.footer", &footer_cfg );
        vxui_menu_screen_end( ctx, &shell_state );
    }
    vxui_menu_surface_end( ctx );
}

static void vxui_demo_render_records_screen( vxui_demo_app* app, vxui_ctx* ctx )
{
    vxui_demo_render_records_screen_shared( ctx,
        ( vxui_demo_split_deck_visuals ) { VXUI_DEMO_FONT_ROLE_BODY, VXUI_DEMO_FONT_ROLE_TITLE, VXUI_DEMO_FONT_ROLE_SECTION },
        ( vxui_demo_records_screen_cfg ) {
            .menu_state = &app->records_menu_state,
            .records_board_index = &app->records_board_index,
            .records_entry_index = &app->records_entry_index,
            .layout_surface_max_height_override = app ? app->shot_layout_surface_max_height_override : 0.0f,
            .background_scanline = app ? app->scanline_index != 0 : true,
            .back_fn = vxui_demo_open_main_menu,
            .back_cfg = ( vxui_action_cfg ) { .userdata = app },
            .status = {
                vxui_demo_footer_locale_key( app ? app->locale_index : 0 ),
                vxui_demo_footer_prompt_key( app ? app->prompt_table_index : 0 ),
                vxui_demo_footer_top_name( app, ctx ),
                ctx ? ctx->screen_count : 0,
            },
        } );
}

static void vxui_demo_render_credits_screen( vxui_demo_app* app, vxui_ctx* ctx, const vxui_demo_renderer* renderer )
{
    const vxui_demo_command_deck_theme& theme = vxui_demo_command_deck_theme_tokens();
    const bool background_scanline = app ? app->scanline_index != 0 : true;
    const float control_height = vxui_demo_control_height( renderer, ctx->locale );
    const float viewport_width = std::max( 0.0f, ( float ) ctx->cfg.screen_width - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f );
    const vxui_demo_surface_metrics surface_metrics = vxui_demo_compute_surface_metrics( viewport_width, ctx->locale, VXUI_DEMO_SURFACE_CREDITS );
    const float surface_max_height = std::max( 0.0f, ( float ) ctx->cfg.screen_height - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f );

    if ( background_scanline ) {
        vxui_demo_emit_surface_scanline( ctx, "credits" );
    }
    vxui_menu_surface_cfg surface_cfg = vxui_menu_surface_cfg_default(
        surface_metrics.surface_width,
        surface_max_height,
        theme.app_background_base,
        theme.primary_panel_fill,
        theme.primary_panel_border );
    vxui_menu_surface_begin( ctx, "credits", "credits.surface", &surface_cfg );
    {
        const std::string screen_count_text = std::to_string( ctx ? ctx->screen_count : 0 );
        vxui_menu_status_item footer_status[] = {
            { "status.short.locale", vxui_demo_footer_locale_key( app ? app->locale_index : 0 ), VXUI_MENU_STATUS_PRIMARY, false, false, "credits.footer.status.locale" },
            { "status.short.prompts", vxui_demo_footer_prompt_key( app ? app->prompt_table_index : 0 ), VXUI_MENU_STATUS_SECONDARY, true, false, "credits.footer.status.prompts" },
            { "status.short.screens", screen_count_text.c_str(), VXUI_MENU_STATUS_SECONDARY, true, false, "credits.footer.status.screens" },
            { "status.short.top", vxui_demo_footer_top_name( app, ctx ), VXUI_MENU_STATUS_PRIMARY, false, false, "credits.footer.status.top" },
        };
        vxui_menu_footer_cfg footer_cfg = {
            nullptr,
            0,
            footer_status,
            ( int ) ( sizeof( footer_status ) / sizeof( footer_status[ 0 ] ) ),
            VXUI_MENU_SHELL_COMPACT_AUTO,
            surface_max_height <= 680.0f ? 3 : 4,
            false,
        };
        vxui_menu_style footer_style = vxui_demo_menu_style_footer_strip();
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

        vxui_menu_screen_begin( ctx, &shell_state, "credits.shell", &screen_cfg );
        VXUI( ctx, "credits.header", {
            .layout = {
                .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                .padding = CLAY_PADDING_ALL( 18 ),
            },
            .backgroundColor = vxui_demo_clay_color( theme.hero_surface_fill ),
            .cornerRadius = CLAY_CORNER_RADIUS( 14 ),
            .border = vxui_demo_panel_border( theme.hero_surface_border, 1 ),
        } ) {
            VXUI_LABEL( ctx, "menu.credits", vxui_demo_text_style( VXUI_DEMO_FONT_ROLE_TITLE, 46.0f, theme.title_text ) );
        }

        VXUI( ctx, "credits.stack", {
            .layout = {
                .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_GROW( 160.0f ) },
                .padding = CLAY_PADDING_ALL( 20 ),
                .childGap = 12,
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
            .backgroundColor = vxui_demo_clay_color( theme.secondary_panel_fill ),
            .cornerRadius = CLAY_CORNER_RADIUS( 12 ),
            .border = vxui_demo_panel_border( theme.secondary_panel_border, 1 ),
        } ) {
            VXUI_LABEL( ctx, "credits.section.stack", ( vxui_label_cfg ) {
                .font_id = VXUI_DEMO_FONT_ROLE_SECTION,
                .font_size = 24.0f,
                .color = theme.section_text,
            } );
            VXUI( ctx, "credits.body_viewport", {
                .layout = {
                    .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_GROW( 120.0f ) },
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                },
                .clip = {
                    .horizontal = true,
                    .vertical = true,
                },
            } ) {
                VXUI( ctx, "credits.body", {
                    .layout = {
                        .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                        .padding = { 0, 0, 0, 12 },
                        .childGap = 12,
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    },
                } ) {
                    for ( const vxui_demo_credit_entry& credit : VXUI_DEMO_CREDITS ) {
                        VXUI_LABEL( ctx, credit.role, ( vxui_label_cfg ) {
                            .font_id = VXUI_DEMO_FONT_ROLE_SECTION,
                            .font_size = 22.0f,
                            .color = theme.accent_cool,
                        } );
                        VXUI_LABEL( ctx, credit.name, ( vxui_label_cfg ) {
                            .font_id = VXUI_DEMO_FONT_ROLE_BODY,
                            .font_size = 24.0f,
                            .color = theme.title_text,
                        } );
                        VXUI_LABEL( ctx, credit.detail, ( vxui_label_cfg ) {
                            .font_id = VXUI_DEMO_FONT_ROLE_BODY,
                            .font_size = 18.0f,
                            .color = theme.muted_text,
                        } );
                    }
                }
            }
        }

        VXUI( ctx, "credits.actions", {
            .layout = {
                .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                .padding = CLAY_PADDING_ALL( 18 ),
                .childGap = 12,
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
            .backgroundColor = vxui_demo_clay_color( theme.utility_fill ),
            .cornerRadius = CLAY_CORNER_RADIUS( 12 ),
            .border = vxui_demo_panel_border( theme.utility_border, 1 ),
        } ) {
            vxui_demo_emit_action_button( ctx, "credits.back", "menu.return_command_deck", vxui_demo_open_main_menu, ( vxui_action_cfg ) {
                .userdata = app,
            }, control_height );
        }
        vxui_menu_footer( ctx, "credits.footer", &screen_cfg.footer );
        vxui_menu_screen_end( ctx, &shell_state );
    }
    vxui_menu_surface_end( ctx );
}

static void vxui_demo_render_launch_stub_screen( vxui_demo_app* app, vxui_ctx* ctx )
{
    const vxui_demo_command_deck_theme& theme = vxui_demo_command_deck_theme_tokens();
    const bool background_scanline = app ? app->scanline_index != 0 : true;
    const float progress = std::clamp( app->launch_timer / 1.4f, 0.0f, 1.0f );
    const float viewport_width = std::max( 0.0f, ( float ) ctx->cfg.screen_width - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f );
    const vxui_demo_surface_metrics surface_metrics = vxui_demo_compute_surface_metrics( viewport_width, ctx->locale, VXUI_DEMO_SURFACE_LAUNCH_STUB );
    const float surface_max_height = std::max( 0.0f, ( float ) ctx->cfg.screen_height - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f );
    const vxui_demo_mission& mission = VXUI_DEMO_MISSIONS[ std::clamp( app->selected_mission_index, 0, 3 ) ];

    if ( background_scanline ) {
        vxui_demo_emit_surface_scanline( ctx, "launch_stub" );
    }
    vxui_menu_surface_cfg surface_cfg = vxui_menu_surface_cfg_default(
        surface_metrics.surface_width,
        surface_max_height,
        theme.app_background_base,
        theme.primary_panel_fill,
        theme.primary_panel_border );
    vxui_menu_surface_begin( ctx, "launch_stub", "launch_stub.surface", &surface_cfg );
    {
        const std::string screen_count_text = std::to_string( ctx ? ctx->screen_count : 0 );
        vxui_menu_prompt_item footer_prompts[] = {
            { "action.confirm", "launch.prompt", false, "launch_stub.footer.prompt.confirm" },
        };
        vxui_menu_status_item footer_status[] = {
            { "status.short.locale", vxui_demo_footer_locale_key( app ? app->locale_index : 0 ), VXUI_MENU_STATUS_PRIMARY, false, false, "launch_stub.footer.status.locale" },
            { "status.short.prompts", vxui_demo_footer_prompt_key( app ? app->prompt_table_index : 0 ), VXUI_MENU_STATUS_SECONDARY, true, false, "launch_stub.footer.status.prompts" },
            { "status.short.screens", screen_count_text.c_str(), VXUI_MENU_STATUS_SECONDARY, true, false, "launch_stub.footer.status.screens" },
            { "status.short.top", vxui_demo_footer_top_name( app, ctx ), VXUI_MENU_STATUS_PRIMARY, false, false, "launch_stub.footer.status.top" },
        };
        vxui_menu_footer_cfg footer_cfg = {
            footer_prompts,
            ( int ) ( sizeof( footer_prompts ) / sizeof( footer_prompts[ 0 ] ) ),
            footer_status,
            ( int ) ( sizeof( footer_status ) / sizeof( footer_status[ 0 ] ) ),
            VXUI_MENU_SHELL_COMPACT_AUTO,
            surface_max_height <= 680.0f ? 3 : 4,
            false,
        };
        vxui_menu_style footer_style = vxui_demo_menu_style_footer_strip();
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

        vxui_menu_screen_begin( ctx, &shell_state, "launch_stub.shell", &screen_cfg );
        VXUI( ctx, "launch_stub.header", {
            .layout = {
                .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                .padding = CLAY_PADDING_ALL( 18 ),
                .childGap = 8,
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
            .backgroundColor = vxui_demo_clay_color( theme.hero_surface_fill ),
            .cornerRadius = CLAY_CORNER_RADIUS( 14 ),
            .border = vxui_demo_panel_border( theme.hero_surface_border, 1 ),
        } ) {
            VXUI_LABEL( ctx, "screen.launch_stub", vxui_demo_text_style( VXUI_DEMO_FONT_ROLE_TITLE, 46.0f, theme.title_text ) );
            VXUI_LABEL( ctx, mission.name, vxui_demo_text_style( VXUI_DEMO_FONT_ROLE_SECTION, 24.0f, theme.accent_cool ) );
        }
        VXUI_LABEL( ctx, "Launch path is a stub front-end handoff; no gameplay runtime exists in this sample.", vxui_demo_text_style( VXUI_DEMO_FONT_ROLE_BODY, 20.0f, theme.body_text ) );
        vxui_demo_emit_stat_bar( ctx, "launch_stub.progress", "Launch Sync", progress );
        vxui_menu_footer( ctx, "launch_stub.footer", &screen_cfg.footer );
        vxui_menu_screen_end( ctx, &shell_state );
    }
    vxui_menu_surface_end( ctx );
}

static void vxui_demo_render_results_stub_screen( vxui_demo_app* app, vxui_ctx* ctx, const vxui_demo_renderer* renderer )
{
    const vxui_demo_command_deck_theme& theme = vxui_demo_command_deck_theme_tokens();
    const bool background_scanline = app ? app->scanline_index != 0 : true;
    const float control_height = vxui_demo_control_height( renderer, ctx->locale );
    const float viewport_width = std::max( 0.0f, ( float ) ctx->cfg.screen_width - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f );
    const vxui_demo_surface_metrics surface_metrics = vxui_demo_compute_surface_metrics( viewport_width, ctx->locale, VXUI_DEMO_SURFACE_RESULTS_STUB );
    const float surface_max_height = std::max( 0.0f, ( float ) ctx->cfg.screen_height - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f );
    const vxui_demo_record& record = VXUI_DEMO_RECORDS[ std::clamp( app->records_entry_index, 0, 3 ) ];

    if ( background_scanline ) {
        vxui_demo_emit_surface_scanline( ctx, "results_stub" );
    }
    vxui_menu_surface_cfg surface_cfg = vxui_menu_surface_cfg_default(
        surface_metrics.surface_width,
        surface_max_height,
        theme.app_background_base,
        theme.primary_panel_fill,
        theme.primary_panel_border );
    vxui_menu_surface_begin( ctx, "results_stub", "results_stub.surface", &surface_cfg );
    {
        const std::string screen_count_text = std::to_string( ctx ? ctx->screen_count : 0 );
        vxui_menu_prompt_item footer_prompts[] = {
            { "action.confirm", "results.prompt", false, "results_stub.footer.prompt.confirm" },
        };
        vxui_menu_status_item footer_status[] = {
            { "status.short.locale", vxui_demo_footer_locale_key( app ? app->locale_index : 0 ), VXUI_MENU_STATUS_PRIMARY, false, false, "results_stub.footer.status.locale" },
            { "status.short.prompts", vxui_demo_footer_prompt_key( app ? app->prompt_table_index : 0 ), VXUI_MENU_STATUS_SECONDARY, true, false, "results_stub.footer.status.prompts" },
            { "status.short.screens", screen_count_text.c_str(), VXUI_MENU_STATUS_SECONDARY, true, false, "results_stub.footer.status.screens" },
            { "status.short.top", vxui_demo_footer_top_name( app, ctx ), VXUI_MENU_STATUS_PRIMARY, false, false, "results_stub.footer.status.top" },
        };
        vxui_menu_footer_cfg footer_cfg = {
            footer_prompts,
            ( int ) ( sizeof( footer_prompts ) / sizeof( footer_prompts[ 0 ] ) ),
            footer_status,
            ( int ) ( sizeof( footer_status ) / sizeof( footer_status[ 0 ] ) ),
            VXUI_MENU_SHELL_COMPACT_AUTO,
            surface_max_height <= 680.0f ? 3 : 4,
            false,
        };
        vxui_menu_style footer_style = vxui_demo_menu_style_footer_strip();
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

        vxui_menu_screen_begin( ctx, &shell_state, "results_stub.shell", &screen_cfg );
        VXUI( ctx, "results_stub.header", {
            .layout = {
                .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                .padding = CLAY_PADDING_ALL( 18 ),
                .childGap = 8,
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
            .backgroundColor = vxui_demo_clay_color( theme.hero_surface_fill ),
            .cornerRadius = CLAY_CORNER_RADIUS( 14 ),
            .border = vxui_demo_panel_border( theme.hero_surface_border, 1 ),
        } ) {
            VXUI_LABEL( ctx, "screen.results_stub", vxui_demo_text_style( VXUI_DEMO_FONT_ROLE_TITLE, 46.0f, theme.title_text ) );
            VXUI_LABEL( ctx, record.score_text, vxui_demo_text_style( VXUI_DEMO_FONT_ROLE_SECTION, 26.0f, theme.section_text ) );
            VXUI_LABEL( ctx, record.clear_text, vxui_demo_text_style( VXUI_DEMO_FONT_ROLE_BODY, 20.0f, theme.success_text ) );
        }
        VXUI_LABEL( ctx, "The front-end loop is complete: sortie selection, launch handoff, and debrief all return to the command deck without hidden gameplay state.", vxui_demo_text_style( VXUI_DEMO_FONT_ROLE_BODY, 20.0f, theme.body_text ) );
        VXUI( ctx, "results_stub.actions", {
            .layout = {
                .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                .padding = CLAY_PADDING_ALL( 18 ),
                .childGap = 10,
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
            .backgroundColor = vxui_demo_clay_color( theme.utility_fill ),
            .cornerRadius = CLAY_CORNER_RADIUS( 12 ),
            .border = vxui_demo_panel_border( theme.utility_border, 1 ),
        } ) {
            vxui_demo_emit_action_button( ctx, "results_stub.return", "menu.return_command_deck", vxui_demo_open_main_menu, ( vxui_action_cfg ) {
                .userdata = app,
            }, control_height );
        }
        vxui_menu_footer( ctx, "results_stub.footer", &screen_cfg.footer );
        vxui_menu_screen_end( ctx, &shell_state );
    }
    vxui_menu_surface_end( ctx );
}

static void vxui_demo_render_frontend( vxui_demo_app* app, vxui_demo_renderer* renderer, vxui_ctx* ctx )
{
    switch ( vxui_demo_screen_kind_from_name( vxui_demo_top_screen_name( ctx ) ) ) {
        case VXUI_DEMO_SCREEN_BOOT:
            vxui_demo_render_boot_screen( app, ctx );
            break;
        case VXUI_DEMO_SCREEN_TITLE:
            vxui_demo_render_title_screen( app, ctx, renderer );
            break;
        case VXUI_DEMO_SCREEN_SORTIE:
            vxui_demo_render_sortie_screen( app, ctx );
            break;
        case VXUI_DEMO_SCREEN_LOADOUT:
            vxui_demo_render_loadout_screen( app, ctx );
            break;
        case VXUI_DEMO_SCREEN_ARCHIVES:
            vxui_demo_render_archives_screen( app, ctx );
            break;
        case VXUI_DEMO_SCREEN_SETTINGS:
            vxui_demo_render_settings_screen( app, ctx, renderer );
            break;
        case VXUI_DEMO_SCREEN_RECORDS:
            vxui_demo_render_records_screen( app, ctx );
            break;
        case VXUI_DEMO_SCREEN_CREDITS:
            vxui_demo_render_credits_screen( app, ctx, renderer );
            break;
        case VXUI_DEMO_SCREEN_LAUNCH_STUB:
            vxui_demo_render_launch_stub_screen( app, ctx );
            break;
        case VXUI_DEMO_SCREEN_RESULTS_STUB:
            vxui_demo_render_results_stub_screen( app, ctx, renderer );
            break;
        case VXUI_DEMO_SCREEN_MAIN_MENU:
        case VXUI_DEMO_SCREEN_UNKNOWN:
        default:
            vxui_demo_render_main_menu_screen( app, ctx );
            break;
    }
}

static void vxui_demo_handle_auto_transitions( vxui_demo_app* app, vxui_ctx* ctx, float dt, bool confirm_pressed )
{
    switch ( vxui_demo_screen_kind_from_name( vxui_demo_top_screen_name( ctx ) ) ) {
        case VXUI_DEMO_SCREEN_BOOT:
            app->boot_timer += dt;
            if ( app->boot_timer >= 1.25f || confirm_pressed ) {
                vxui_demo_open_title( ctx, 0, app );
            }
            break;
        case VXUI_DEMO_SCREEN_TITLE:
            app->title_timer += dt;
            break;
        case VXUI_DEMO_SCREEN_LAUNCH_STUB:
            app->launch_timer += dt;
            if ( app->launch_timer >= 1.40f || confirm_pressed ) {
                vxui_demo_open_results_stub( ctx, 0, app );
            }
            break;
        case VXUI_DEMO_SCREEN_RESULTS_STUB:
            app->results_timer += dt;
            break;
        default:
            break;
    }
}

static void vxui_demo_handle_cancel_navigation( vxui_demo_app* app, vxui_ctx* ctx )
{
    switch ( vxui_demo_screen_kind_from_name( vxui_demo_top_screen_name( ctx ) ) ) {
        case VXUI_DEMO_SCREEN_SORTIE:
        case VXUI_DEMO_SCREEN_LOADOUT:
        case VXUI_DEMO_SCREEN_ARCHIVES:
        case VXUI_DEMO_SCREEN_SETTINGS:
        case VXUI_DEMO_SCREEN_RECORDS:
        case VXUI_DEMO_SCREEN_CREDITS:
        case VXUI_DEMO_SCREEN_LAUNCH_STUB:
        case VXUI_DEMO_SCREEN_RESULTS_STUB:
            vxui_demo_open_main_menu( ctx, 0, app );
            break;
        default:
            break;
    }
}
