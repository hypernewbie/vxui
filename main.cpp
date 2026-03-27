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
#include <vector>

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

enum
{
    VXUI_DEMO_BODY_SIZE = 24,
    VXUI_DEMO_TITLE_SIZE = 44,
    VXUI_DEMO_SECTION_SIZE = 28,
    VXUI_DEMO_CODE_SIZE = 16,
};

static constexpr uint16_t VXUI_DEMO_BUTTON_PADDING_X = 12;
static constexpr uint16_t VXUI_DEMO_BUTTON_PADDING_Y = 6;
static constexpr uint16_t VXUI_DEMO_INLINE_GAP = 8;
static constexpr uint16_t VXUI_DEMO_ROW_GAP = 12;
static constexpr uint16_t VXUI_DEMO_SCREEN_GAP = 16;
static constexpr float VXUI_DEMO_CONTENT_MAX_WIDTH = 620.0f;
static constexpr float VXUI_DEMO_FORM_LABEL_WIDTH = 140.0f;

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
    int locale_index;
    int prompt_table_index;
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
    GLuint fontcache_fbo[ 2 ];
    GLuint fontcache_fbo_texture[ 2 ];
    std::vector< GLuint > cpu_atlas_textures;
    TinyWindow::vec2_t< unsigned int > window_size;
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
static const char* vxui_demo_screen_name_key( const char* screen_name );
static bool vxui_demo_locale_matches( const char* locale, const char* prefix );
static vxui_demo_font_metrics vxui_demo_resolve_font_metrics( const vxui_demo_renderer* renderer, uint32_t requested_font_id, float requested_font_size, const char* locale );
static void vxui_demo_font_resolver( vxui_ctx* ctx, uint32_t requested_font_id, float requested_font_size, const char* locale, void* userdata, vxui_resolved_font* out );
static float vxui_demo_control_height( const vxui_demo_renderer* renderer, const char* locale );
static void vxui_demo_emit_label_pair( vxui_ctx* ctx, const char* id, const char* label_key, const char* value_key, bool rtl );
static void vxui_demo_emit_prompt_pair( vxui_ctx* ctx, const char* id, const char* action_name, const char* label_key );
static void vxui_demo_emit_action_button( vxui_ctx* ctx, const char* id, const char* l10n_key, vxui_action_fn fn, vxui_action_cfg cfg, float control_height );
static void vxui_demo_open_settings( vxui_ctx* ctx, uint32_t id, void* userdata );
static void vxui_demo_close_settings( vxui_ctx* ctx, uint32_t id, void* userdata );
static GLuint vxui_demo_compile_program( const char* vs_source, const char* fs_source, const char* label );
static void vxui_demo_gl_debug_init( vxui_demo_renderer* renderer );
static void vxui_demo_gl_debug_label( vxui_demo_renderer* renderer, GLenum identifier, GLuint name, const char* label );
static void vxui_demo_gl_debug_begin( vxui_demo_renderer* renderer, const char* label );
static void vxui_demo_gl_debug_end( vxui_demo_renderer* renderer );
static void vxui_demo_gl_debug_event( vxui_demo_renderer* renderer, const char* label );
static bool vxui_demo_init_renderer( vxui_demo_renderer* renderer );
static void vxui_demo_shutdown_renderer( vxui_demo_renderer* renderer );
static bool vxui_demo_load_fonts( vxui_demo_renderer* renderer );
static void vxui_demo_render_draw_list( vxui_demo_renderer* renderer, const vxui_draw_list* list );
static void vxui_demo_draw_primitive( vxui_demo_renderer* renderer, vxui_rect bounds, vxui_color color, float radius, float border_width );
static void vxui_demo_draw_placeholder_image( vxui_demo_renderer* renderer, vxui_rect bounds );
static void vxui_demo_render_text_cmd( vxui_demo_renderer* renderer, const vxui_draw_cmd_text* text, const vxui_rect* clip_rect );
static void vxui_demo_render_fontcache_drawlist( vxui_demo_renderer* renderer, const vxui_rect* clip_rect );
static void vxui_demo_setup_fontcache_fbo( vxui_demo_renderer* renderer );
static void vxui_demo_flip_fontcache_upload( ve_fontcache_drawlist* drawlist, ve_fontcache_draw& dcall );
static void vxui_demo_clear_backend_test_surfaces( vxui_demo_renderer* renderer, bool clear_cpu_atlas_pages );
static bool vxui_demo_setup_backend_test_fbo( vxui_demo_renderer* renderer );
static void vxui_demo_shutdown_backend_test_fbo( vxui_demo_renderer* renderer );
static bool vxui_demo_readback_r8_texture( GLuint texture, int x, int y, int w, int h, uint8_t* out_pixels );
static bool vxui_demo_button_edge( vxui_demo_app* app, vxui_demo_button button, bool down );
static bool vxui_demo_key_down( const TinyWindow::tWindow* window, int key );
static bool vxui_demo_char_down( const TinyWindow::tWindow* window, char ch );
static const TinyWindow::gamepad_t* vxui_demo_primary_gamepad( TinyWindow::windowManager* manager );
static bool vxui_demo_gamepad_down( const TinyWindow::gamepad_t* gamepad, int button );

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
        if ( std::strcmp( key, "locale.name.en" ) == 0 ) return "英語";
        if ( std::strcmp( key, "locale.name.ja" ) == 0 ) return "日本語";
        if ( std::strcmp( key, "locale.name.ar" ) == 0 ) return "アラビア語";
        if ( std::strcmp( key, "prompt.name.keyboard" ) == 0 ) return "キーボード";
        if ( std::strcmp( key, "prompt.name.gamepad" ) == 0 ) return "ゲームパッド";
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
        if ( std::strcmp( key, "locale.name.en" ) == 0 ) return "الإنجليزية";
        if ( std::strcmp( key, "locale.name.ja" ) == 0 ) return "اليابانية";
        if ( std::strcmp( key, "locale.name.ar" ) == 0 ) return "العربية";
        if ( std::strcmp( key, "prompt.name.keyboard" ) == 0 ) return "لوحة المفاتيح";
        if ( std::strcmp( key, "prompt.name.gamepad" ) == 0 ) return "يد التحكم";
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
        if ( std::strcmp( key, "locale.name.en" ) == 0 ) return "English";
        if ( std::strcmp( key, "locale.name.ja" ) == 0 ) return "Japanese";
        if ( std::strcmp( key, "locale.name.ar" ) == 0 ) return "Arabic";
        if ( std::strcmp( key, "prompt.name.keyboard" ) == 0 ) return "Keyboard";
        if ( std::strcmp( key, "prompt.name.gamepad" ) == 0 ) return "Gamepad";
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

static const char* vxui_demo_prompt_name_key( int prompt_table_index )
{
    return prompt_table_index == 0 ? "prompt.name.keyboard" : "prompt.name.gamepad";
}

static const char* vxui_demo_screen_name_key( const char* screen_name )
{
    if ( screen_name && std::strcmp( screen_name, "main_menu" ) == 0 ) {
        return "menu.main";
    }
    if ( screen_name && std::strcmp( screen_name, "settings" ) == 0 ) {
        return "menu.settings";
    }
    return "screen.none";
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

static void vxui_demo_emit_label_pair( vxui_ctx* ctx, const char* id, const char* label_key, const char* value_key, bool rtl )
{
    uint32_t pair_id = vxui_id( id );
    CLAY( vxui__clay_id_from_hash( pair_id ), {
        .layout = {
            .sizing = { CLAY_SIZING_FIT( 0 ), CLAY_SIZING_FIT( 0 ) },
            .childGap = VXUI_DEMO_INLINE_GAP,
            .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
            .layoutDirection = CLAY_LEFT_TO_RIGHT,
        },
    } ) {
        if ( rtl ) {
            VXUI_LABEL( ctx, value_key, ( vxui_label_cfg ) { 0 } );
            VXUI_LABEL( ctx, label_key, ( vxui_label_cfg ) { 0 } );
        } else {
            VXUI_LABEL( ctx, label_key, ( vxui_label_cfg ) { 0 } );
            VXUI_LABEL( ctx, value_key, ( vxui_label_cfg ) { 0 } );
        }
    }
}

static void vxui_demo_emit_prompt_pair( vxui_ctx* ctx, const char* id, const char* action_name, const char* label_key )
{
    uint32_t pair_id = vxui_id( id );
    CLAY( vxui__clay_id_from_hash( pair_id ), {
        .layout = {
            .sizing = { CLAY_SIZING_FIT( 0 ), CLAY_SIZING_FIT( 0 ) },
            .childGap = VXUI_DEMO_INLINE_GAP,
            .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
            .layoutDirection = CLAY_LEFT_TO_RIGHT,
        },
    } ) {
        VXUI_PROMPT( ctx, action_name );
        VXUI_LABEL( ctx, label_key, ( vxui_label_cfg ) { 0 } );
    }
}

static void vxui_demo_emit_action_button( vxui_ctx* ctx, const char* id, const char* l10n_key, vxui_action_fn fn, vxui_action_cfg cfg, float control_height )
{
    const char* resolved = vxui__resolve_text( ctx, l10n_key );
    uint32_t action_id = vxui_id( id );

    vxui__register_action( ctx, action_id, fn, cfg );
    vxui__get_anim_state( ctx, action_id, true );
    ctx->current_decl_id = action_id;

    CLAY( vxui__clay_id_from_hash( action_id ), {
        .layout = {
            .sizing = { CLAY_SIZING_FIT( 0 ), CLAY_SIZING_FIXED( control_height ) },
            .padding = { ( uint16_t ) VXUI_DEMO_BUTTON_PADDING_X, ( uint16_t ) VXUI_DEMO_BUTTON_PADDING_X, ( uint16_t ) VXUI_DEMO_BUTTON_PADDING_Y, ( uint16_t ) VXUI_DEMO_BUTTON_PADDING_Y },
            .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
        },
        .backgroundColor = { 26, 44, 72, 255 },
        .cornerRadius = CLAY_CORNER_RADIUS( 8 ),
    } ) {
        vxui__emit_text(
            ctx,
            resolved,
            VXUI_DEMO_FONT_ROLE_BODY,
            ( float ) VXUI_DEMO_BODY_SIZE,
            ctx->default_text_color,
            action_id );
    }
}

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
    vxui_push_screen( ctx, "settings" );
}

static void vxui_demo_close_settings( vxui_ctx* ctx, uint32_t id, void* userdata )
{
    ( void ) id;
    vxui_demo_app* app = ( vxui_demo_app* ) userdata;
    if ( !ctx || !app || !app->show_settings ) {
        return;
    }
    app->show_settings = false;
    vxui_pop_screen( ctx );
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
    ve_fontcache_configure_snap( &renderer->cache, renderer->window_size.width, renderer->window_size.height );

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
    ve_fontcache_configure_snap( &renderer->cache, renderer->window_size.width, renderer->window_size.height );
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
    ve_fontcache_configure_snap( &renderer->cache, renderer->window_size.width, renderer->window_size.height );
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

    glGenVertexArrays( 1, &renderer->vao );
    glBindVertexArray( renderer->vao );
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
    if ( renderer->vao ) glDeleteVertexArrays( 1, &renderer->vao );
    if ( renderer->fontcache_fbo[ 0 ] || renderer->fontcache_fbo[ 1 ] ) glDeleteFramebuffers( 2, renderer->fontcache_fbo );
    if ( renderer->fontcache_fbo_texture[ 0 ] || renderer->fontcache_fbo_texture[ 1 ] ) glDeleteTextures( 2, renderer->fontcache_fbo_texture );
    if ( !renderer->cpu_atlas_textures.empty() ) {
        glDeleteTextures( ( GLsizei ) renderer->cpu_atlas_textures.size(), renderer->cpu_atlas_textures.data() );
    }
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
    glUniform4f( glGetUniformLocation( renderer->primitive_program, "u_bounds" ), bounds.x, bounds.y, bounds.w, bounds.h );
    glUniform2f( glGetUniformLocation( renderer->primitive_program, "u_viewport" ), ( float ) renderer->window_size.width, ( float ) renderer->window_size.height );
    glUniform2f( glGetUniformLocation( renderer->primitive_program, "u_size" ), bounds.w, bounds.h );
    glUniform4f(
        glGetUniformLocation( renderer->primitive_program, "u_color" ),
        color.r / 255.0f,
        color.g / 255.0f,
        color.b / 255.0f,
        color.a / 255.0f );
    glUniform1f( glGetUniformLocation( renderer->primitive_program, "u_radius" ), radius );
    glUniform1f( glGetUniformLocation( renderer->primitive_program, "u_border" ), border_width );
    glDrawArrays( GL_TRIANGLES, 0, 6 );
}

static void vxui_demo_draw_placeholder_image( vxui_demo_renderer* renderer, vxui_rect bounds )
{
    vxui_demo_draw_primitive( renderer, bounds, ( vxui_color ) { 40, 48, 68, 255 }, 6.0f, 0.0f );
    vxui_demo_draw_primitive( renderer, bounds, ( vxui_color ) { 255, 128, 200, 255 }, 6.0f, 2.0f );
}

static void vxui_demo_render_fontcache_drawlist( vxui_demo_renderer* renderer, const vxui_rect* clip_rect )
{
    ve_fontcache_drawlist* drawlist = ve_fontcache_get_drawlist( &renderer->cache );
    if ( !drawlist || drawlist->dcalls.empty() ) {
        ve_fontcache_flush_drawlist( &renderer->cache );
        return;
    }

    ve_fontcache_optimise_drawlist( &renderer->cache );

    GLuint vbo = 0;
    GLuint ibo = 0;
    if ( !drawlist->vertices.empty() && !drawlist->indices.empty() ) {
        glGenBuffers( 1, &vbo );
        glGenBuffers( 1, &ibo );
        glBindVertexArray( renderer->vao );
        glBindBuffer( GL_ARRAY_BUFFER, vbo );
        glBufferData( GL_ARRAY_BUFFER, drawlist->vertices.size() * sizeof( ve_fontcache_vertex ), drawlist->vertices.data(), GL_DYNAMIC_DRAW );
        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibo );
        glBufferData( GL_ELEMENT_ARRAY_BUFFER, drawlist->indices.size() * sizeof( uint32_t ), drawlist->indices.data(), GL_DYNAMIC_DRAW );
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
            glUniform1i( glGetUniformLocation( renderer->fontcache_shader_blit_atlas, "src_texture" ), 0 );
            glUniform1ui( glGetUniformLocation( renderer->fontcache_shader_blit_atlas, "region" ), dcall.region );
            glActiveTexture( GL_TEXTURE0 );
            glBindTexture( GL_TEXTURE_2D, renderer->fontcache_fbo_texture[ 0 ] );
            glDisable( GL_FRAMEBUFFER_SRGB );
        } else if ( dcall.pass == VE_FONTCACHE_FRAMEBUFFER_PASS_TARGET || dcall.pass == VE_FONTCACHE_FRAMEBUFFER_PASS_TARGET_UNCACHED || dcall.pass == VE_FONTCACHE_FRAMEBUFFER_PASS_TARGET_CPU_CACHED ) {
            begin_pass_group( 2, "VEFC Target Text" );
            glUseProgram( renderer->fontcache_shader_draw_text );
            GLuint target_fbo = renderer->backend_test_mode ? renderer->backend_target_fbo : 0;
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
            glUniform1i( glGetUniformLocation( renderer->fontcache_shader_draw_text, "src_texture" ), 0 );
            GLuint src_tex = 0;
#ifdef VE_FONTCACHE_FREETYPE_RASTERISATION
            if ( dcall.pass == VE_FONTCACHE_FRAMEBUFFER_PASS_TARGET_CPU_CACHED ) {
                if ( dcall.atlas_page >= ( uint32_t ) renderer->cpu_atlas_textures.size() ) {
                    std::fprintf( stderr, "vxui_demo: TARGET_CPU_CACHED draw arrived but cpu_atlas_textures[%u] not created\n", dcall.atlas_page );
                } else {
                    glUniform1ui( glGetUniformLocation( renderer->fontcache_shader_draw_text, "downsample" ), 0 );
                    glActiveTexture( GL_TEXTURE0 );
                    glBindTexture( GL_TEXTURE_2D, renderer->cpu_atlas_textures[ dcall.atlas_page ] );
                    src_tex = renderer->cpu_atlas_textures[ dcall.atlas_page ];
                }
            } else
#endif
            {
                glUniform1ui( glGetUniformLocation( renderer->fontcache_shader_draw_text, "downsample" ), dcall.pass == VE_FONTCACHE_FRAMEBUFFER_PASS_TARGET_UNCACHED ? 1u : 0u );
                glActiveTexture( GL_TEXTURE0 );
                src_tex = dcall.pass == VE_FONTCACHE_FRAMEBUFFER_PASS_TARGET_UNCACHED ? renderer->fontcache_fbo_texture[ 0 ] : renderer->fontcache_fbo_texture[ 1 ];
                glBindTexture( GL_TEXTURE_2D, src_tex );
            }
            glUniform4fv( glGetUniformLocation( renderer->fontcache_shader_draw_text, "colour" ), 1, dcall.colour );
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
    if ( vbo != 0 ) {
        glDeleteBuffers( 1, &vbo );
        glDeleteBuffers( 1, &ibo );
    }
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    ve_fontcache_flush_drawlist( &renderer->cache );
}

static void vxui_demo_render_text_cmd( vxui_demo_renderer* renderer, const vxui_draw_cmd_text* text, const vxui_rect* clip_rect )
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

    vxui_demo_render_fontcache_drawlist( renderer, clip_rect );
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

    ve_fontcache_flush_drawlist( &renderer->cache );
    for ( int i = 0; i < list->length; ++i ) {
        const vxui_cmd* cmd = &list->commands[ i ];
        switch ( cmd->type ) {
            case VXUI_CMD_RECT:
                vxui_demo_draw_primitive( renderer, cmd->rect.bounds, cmd->rect.color, 0.0f, 0.0f );
                break;

            case VXUI_CMD_RECT_ROUNDED:
                vxui_demo_draw_primitive( renderer, cmd->rect_rounded.bounds, cmd->rect_rounded.color, cmd->rect_rounded.radius, 0.0f );
                break;

            case VXUI_CMD_BORDER:
                vxui_demo_draw_primitive( renderer, cmd->border.bounds, cmd->border.color, cmd->border.radius, cmd->border.width );
                break;

            case VXUI_CMD_IMAGE:
                vxui_demo_draw_placeholder_image( renderer, cmd->image.bounds );
                break;

            case VXUI_CMD_TEXT:
                vxui_demo_render_text_cmd( renderer, &cmd->text, clip_stack.empty() ? nullptr : &clip_stack.back() );
                break;

            case VXUI_CMD_CLIP_PUSH:
                clip_stack.push_back( cmd->clip.rect );
                apply_clip( &clip_stack.back() );
                break;

            case VXUI_CMD_CLIP_POP:
                if ( !clip_stack.empty() ) {
                    clip_stack.pop_back();
                }
                apply_clip( clip_stack.empty() ? nullptr : &clip_stack.back() );
                break;
        }
    }

    glDisable( GL_SCISSOR_TEST );
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
    for ( int i = 1; i < argc; ++i ) {
        if ( std::strcmp( argv[ i ], "--vefc-backend-test" ) == 0 ) {
            vefc_backend_test_mode = true;
            break;
        }
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
     cfg.resolution.width = vefc_backend_test_mode ? 1920 : 1280;
     cfg.resolution.height = vefc_backend_test_mode ? 1080 : 720;
     cfg.SetProfile( TinyWindow::profile_t::core );
     cfg.startHidden = vefc_backend_test_mode;

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

    ve_fontcache_configure_snap( &renderer.cache, renderer.window_size.width, renderer.window_size.height );

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
    app.volume = 0.40f;
    app.watched_seq_path = vxui_demo_make_temp_path( "vxui_demo_sequence.toml" );
    app.last_selected_seq = -1;
    vxui_set_text_fn( &ctx, vxui_demo_text, &app );

    vxui_set_locale( &ctx, "en" );

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
    vxui_set_input_table( &ctx, &keyboard_table );

    if ( !vxui_demo_write_file(
             app.watched_seq_path.c_str(),
             "[sequence.main_menu_enter]\n"
             "steps = [\n"
             "  { delay = 0, id = \"main_menu\", prop = \"opacity\", target = 0.35 },\n"
             "  { delay = 100, id = \"main_menu\", prop = \"opacity\", target = 1.0 },\n"
             "]\n" ) ) {
        std::fprintf( stderr, "Failed to create the watched sequence file at %s.\n", app.watched_seq_path.c_str() );
        vxui_demo_shutdown_renderer( &renderer );
        window.reset( nullptr );
        manager->ShutDown();
        return 1;
    }

    char error[ 256 ] = {};
    if ( !vxui_load_seq_toml( &ctx, app.watched_seq_path.c_str(), "main_menu_enter", error, sizeof( error ) ) ) {
        std::fprintf( stderr, "Failed to load %s: %s\n", app.watched_seq_path.c_str(), error );
        std::remove( app.watched_seq_path.c_str() );
        vxui_demo_shutdown_renderer( &renderer );
        window.reset( nullptr );
        manager->ShutDown();
        return 1;
    }
#ifdef VXUI_DEBUG
    if ( !vxui_watch_seq_file( &ctx, app.watched_seq_path.c_str(), "main_menu_enter" ) ) {
        std::fprintf( stderr, "Failed to watch %s for hot reload.\n", app.watched_seq_path.c_str() );
        std::remove( app.watched_seq_path.c_str() );
        vxui_demo_shutdown_renderer( &renderer );
        window.reset( nullptr );
        manager->ShutDown();
        return 1;
    }
    ctx.debug_seq_editor.selected_seq = vxui_demo_find_seq_index( &ctx, "main_menu_enter" );
    vxui_debug_generate_seq_outputs( &ctx );
    vxui_demo_sync_step_editor( &app );
#endif

    vxui_seq_step settings_enter[] = {
        { 0, vxui_id( "settings" ), VXUI_PROP_SLIDE_X, 28.0f },
        { 120, vxui_id( "settings" ), VXUI_PROP_SLIDE_X, 0.0f },
    };
    vxui_seq_step settings_exit[] = {
        { 0, vxui_id( "settings" ), VXUI_PROP_OPACITY, 0.4f },
        { 0, vxui_id( "settings" ), VXUI_PROP_SLIDE_X, -20.0f },
    };
    vxui_register_seq( &ctx, "settings_enter", settings_enter, ( int ) ( sizeof( settings_enter ) / sizeof( settings_enter[ 0 ] ) ) );
    vxui_register_seq( &ctx, "settings_exit", settings_exit, ( int ) ( sizeof( settings_exit ) / sizeof( settings_exit[ 0 ] ) ) );
     vxui_push_screen( &ctx, "main_menu" );
    vxui_demo_refresh_status( &app );

    std::chrono::steady_clock::time_point previous = std::chrono::steady_clock::now();
    while ( !window->shouldClose ) {
        manager->PollForEvents();
        renderer.window_size = window->settings.resolution;
        if ( renderer.window_size.width == 0 ) renderer.window_size.width = 1;
        if ( renderer.window_size.height == 0 ) renderer.window_size.height = 1;
        ctx.cfg.screen_width = ( int ) renderer.window_size.width;
        ctx.cfg.screen_height = ( int ) renderer.window_size.height;
        ve_fontcache_configure_snap( &renderer.cache, renderer.window_size.width, renderer.window_size.height );

        std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
        float dt = std::chrono::duration< float >( now - previous ).count();
        previous = now;
        if ( dt <= 0.0f || dt > 0.1f ) {
            dt = 1.0f / 60.0f;
        }
        uint64_t now_ms = ( uint64_t ) std::chrono::duration_cast< std::chrono::milliseconds >( now.time_since_epoch() ).count();

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
        if ( vxui_demo_button_edge( &app, VXUI_DEMO_BTN_CONFIRM, vxui_demo_key_down( window.get(), TinyWindow::enter ) || vxui_demo_key_down( window.get(), TinyWindow::spacebar ) || vxui_demo_gamepad_down( gamepad, TinyWindow::gamepad_t::face_bottom ) ) ) {
            vxui_input_confirm( &ctx );
        }
        if ( vxui_demo_button_edge( &app, VXUI_DEMO_BTN_CANCEL, vxui_demo_key_down( window.get(), TinyWindow::escape ) || vxui_demo_key_down( window.get(), TinyWindow::backspace ) || vxui_demo_gamepad_down( gamepad, TinyWindow::gamepad_t::face_right ) ) ) {
            vxui_input_cancel( &ctx );
            if ( app.show_settings ) {
                vxui_demo_close_settings( &ctx, 0, &app );
            }
        }
        if ( vxui_demo_button_edge( &app, VXUI_DEMO_BTN_TAB_LEFT, vxui_demo_char_down( window.get(), '[' ) || vxui_demo_gamepad_down( gamepad, TinyWindow::gamepad_t::left_shoulder ) ) ) {
            vxui_input_tab( &ctx, -1 );
        }
        if ( vxui_demo_button_edge( &app, VXUI_DEMO_BTN_TAB_RIGHT, vxui_demo_char_down( window.get(), ']' ) || vxui_demo_gamepad_down( gamepad, TinyWindow::gamepad_t::right_shoulder ) ) ) {
            vxui_input_tab( &ctx, 1 );
        }
        if ( vxui_demo_button_edge( &app, VXUI_DEMO_BTN_LOCALE_EN, vxui_demo_char_down( window.get(), '1' ) ) ) {
            app.locale_index = 0;
            vxui_set_locale( &ctx, "en" );
        }
        if ( vxui_demo_button_edge( &app, VXUI_DEMO_BTN_LOCALE_JA, vxui_demo_char_down( window.get(), '2' ) ) ) {
            app.locale_index = 1;
            vxui_set_locale( &ctx, "ja-JP" );
        }
        if ( vxui_demo_button_edge( &app, VXUI_DEMO_BTN_LOCALE_AR, vxui_demo_char_down( window.get(), '3' ) ) ) {
            app.locale_index = 2;
            vxui_set_locale( &ctx, "ar" );
        }
        if ( vxui_demo_button_edge( &app, VXUI_DEMO_BTN_PROMPT_KEYBOARD, vxui_demo_char_down( window.get(), '4' ) ) ) {
            app.prompt_table_index = 0;
            vxui_set_input_table( &ctx, &keyboard_table );
        }
        if ( vxui_demo_button_edge( &app, VXUI_DEMO_BTN_PROMPT_GAMEPAD, vxui_demo_char_down( window.get(), '5' ) ) ) {
            app.prompt_table_index = 1;
            vxui_set_input_table( &ctx, &gamepad_table );
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

#ifdef VXUI_DEBUG
        if ( ctx.debug_seq_editor.open ) {
            vxui_demo_apply_step_editor( &app );
            vxui_debug_generate_seq_outputs( &ctx );
        } else {
            ctx.debug_seq_editor.preview_playing = false;
        }
#endif
        vxui_demo_refresh_status( &app );

        const bool rtl = ctx.rtl;
        const float control_height = vxui_demo_control_height( &renderer, ctx.locale );
        const vxui_screen* top_screen = ctx.screen_count > 0 ? &ctx.screens[ ctx.screen_count - 1 ] : nullptr;
        const char* locale_name_key = vxui_demo_locale_name_key( app.locale_index );
        const char* prompt_name_key = vxui_demo_prompt_name_key( app.prompt_table_index );
        const char* top_name_key = vxui_demo_screen_name_key( top_screen ? top_screen->name : nullptr );
        const vxui_label_cfg title_cfg = {
            .font_id = VXUI_DEMO_FONT_ROLE_TITLE,
            .font_size = ( float ) VXUI_DEMO_TITLE_SIZE,
            .color = { 255, 247, 225, 255 },
        };
        const vxui_label_cfg section_cfg = {
            .font_id = VXUI_DEMO_FONT_ROLE_SECTION,
            .font_size = ( float ) VXUI_DEMO_SECTION_SIZE,
            .color = { 255, 214, 153, 255 },
        };

        if ( app.show_settings ) {
            VXUI( &ctx, "settings", {
                .layout = {
                    .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_GROW( 0 ) },
                    .padding = CLAY_PADDING_ALL( 18 ),
                },
                .backgroundColor = { 11, 18, 34, 255 },
            } ) {
                VXUI( &ctx, "settings.content", {
                    .layout = {
                        .sizing = { CLAY_SIZING_GROW( 0, VXUI_DEMO_CONTENT_MAX_WIDTH ), CLAY_SIZING_FIT( 0 ) },
                        .childGap = VXUI_DEMO_SCREEN_GAP,
                        .childAlignment = { .x = rtl ? CLAY_ALIGN_X_RIGHT : CLAY_ALIGN_X_LEFT },
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    },
                } ) {
                VXUI( &ctx, "settings.title", {
                    .layout = {
                        .sizing = { CLAY_SIZING_FIT( 0 ), CLAY_SIZING_FIT( 0 ) },
                        .childGap = 0,
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    },
                } ) {
                    VXUI_LABEL( &ctx, "menu.settings", title_cfg );
                    VXUI_TRAIT( VXUI_TRAIT_SPIN, ( vxui_demo_spin ) { .speed = 2.0f, .padding = 6.0f } );
                }

                VXUI( &ctx, "settings.form", {
                    .layout = {
                        .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                        .childGap = VXUI_DEMO_ROW_GAP,
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    },
                } ) {
                    VXUI( &ctx, "settings.form.difficulty", {
                        .layout = {
                            .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                            .childGap = VXUI_DEMO_ROW_GAP,
                            .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                            .layoutDirection = CLAY_LEFT_TO_RIGHT,
                        },
                    } ) {
                        CLAY( vxui__clay_id_from_hash( vxui_id( "settings.form.difficulty.label" ) ), {
                            .layout = {
                                .sizing = { CLAY_SIZING_FIXED( VXUI_DEMO_FORM_LABEL_WIDTH ), CLAY_SIZING_FIT( 0 ) },
                            },
                        } ) {
                            VXUI_LABEL( &ctx, "menu.difficulty", ( vxui_label_cfg ) { 0 } );
                        }
                        VXUI_OPTION( &ctx, "settings.difficulty", &app.difficulty, difficulty_keys, 3, ( vxui_option_cfg ) { 0 } );
                    }

                    VXUI( &ctx, "settings.form.volume", {
                        .layout = {
                            .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                            .childGap = VXUI_DEMO_ROW_GAP,
                            .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                            .layoutDirection = CLAY_LEFT_TO_RIGHT,
                        },
                    } ) {
                        CLAY( vxui__clay_id_from_hash( vxui_id( "settings.form.volume.label" ) ), {
                            .layout = {
                                .sizing = { CLAY_SIZING_FIXED( VXUI_DEMO_FORM_LABEL_WIDTH ), CLAY_SIZING_FIT( 0 ) },
                            },
                        } ) {
                            VXUI_LABEL( &ctx, "menu.volume", ( vxui_label_cfg ) { 0 } );
                        }
                        VXUI_SLIDER( &ctx, "settings.volume", &app.volume, 0.0f, 1.0f, ( vxui_slider_cfg ) {
                            .show_value = true,
                        } );
                    }
                }

                VXUI( &ctx, "settings.saves_section", {
                    .layout = {
                        .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                        .childGap = VXUI_DEMO_ROW_GAP,
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    },
                } ) {
                    VXUI_LABEL( &ctx, "menu.saves", section_cfg );
                    VXUI_LIST_BEGIN( &ctx, "settings.saves", ( vxui_list_cfg ) {
                        .max_visible = 6,
                        .item_height = control_height,
                    } ) {
                        for ( int i = 0; i < 8; ++i ) {
                            VXUI_LIST_ITEM( &ctx, i ) {
                                VXUI_LABEL( &ctx, slot_keys[ i ], ( vxui_label_cfg ) { 0 } );
                            }
                        }
                    }
                }

                VXUI( &ctx, "settings.footer", {
                    .layout = {
                        .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                        .childGap = VXUI_DEMO_ROW_GAP,
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    },
                } ) {
                    VXUI( &ctx, "settings.prompts", {
                        .layout = {
                            .sizing = { CLAY_SIZING_FIT( 0 ), CLAY_SIZING_FIT( 0 ) },
                            .childGap = VXUI_DEMO_INLINE_GAP,
                            .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                            .layoutDirection = vxui__resolve_dir( CLAY_LEFT_TO_RIGHT, rtl ),
                        },
                    } ) {
                        vxui_demo_emit_prompt_pair( &ctx, "settings.prompt.confirm", "action.confirm", "menu.confirm" );
                        vxui_demo_emit_prompt_pair( &ctx, "settings.prompt.cancel", "action.cancel", "menu.cancel" );
                    }

                    VXUI( &ctx, "settings.status", {
                        .layout = {
                            .sizing = { CLAY_SIZING_FIT( 0 ), CLAY_SIZING_FIT( 0 ) },
                            .childGap = VXUI_DEMO_ROW_GAP,
                            .layoutDirection = CLAY_TOP_TO_BOTTOM,
                        },
                    } ) {
                        vxui_demo_emit_label_pair( &ctx, "settings.status.locale", "status.label.locale", locale_name_key, rtl );
                        vxui_demo_emit_label_pair( &ctx, "settings.status.prompts", "status.label.prompts", prompt_name_key, rtl );
                        vxui_demo_emit_label_pair( &ctx, "settings.status.screens", "status.label.screens", top_name_key, rtl );
                    }

                    vxui_demo_emit_action_button( &ctx, "settings.back", "menu.back", vxui_demo_close_settings, ( vxui_action_cfg ) {
                        .userdata = &app,
                    }, control_height );
                    VXUI_TRAIT( VXUI_TRAIT_GLOW, ( vxui_demo_glow ) { .padding = 6.0f, .alpha = 0.25f } );
                }
                } /* settings.content */
            } /* settings */
        } else {
            VXUI( &ctx, "main_menu", {
                .layout = {
                    .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_GROW( 0 ) },
                    .padding = CLAY_PADDING_ALL( 18 ),
                },
                .backgroundColor = { 8, 12, 22, 255 },
            } ) {
                VXUI( &ctx, "main.content", {
                    .layout = {
                        .sizing = { CLAY_SIZING_GROW( 0, VXUI_DEMO_CONTENT_MAX_WIDTH ), CLAY_SIZING_FIT( 0 ) },
                        .childGap = VXUI_DEMO_SCREEN_GAP,
                        .childAlignment = { .x = rtl ? CLAY_ALIGN_X_RIGHT : CLAY_ALIGN_X_LEFT },
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    },
                } ) {
                VXUI( &ctx, "main.hero", {
                    .layout = {
                        .sizing = { CLAY_SIZING_FIT( 0 ), CLAY_SIZING_FIT( 0 ) },
                        .childGap = 0,
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    },
                } ) {
                    VXUI_LABEL( &ctx, "menu.main", title_cfg );
                    VXUI_TRAIT( VXUI_TRAIT_PULSE, ( vxui_demo_pulse ) { .speed = 2.0f, .scale = 0.05f, .alpha = 0.14f } );

                    vxui_demo_emit_action_button( &ctx, "main.settings", "menu.open_settings", vxui_demo_open_settings, ( vxui_action_cfg ) {
                        .userdata = &app,
                    }, control_height );
                    VXUI_TRAIT( VXUI_TRAIT_GLOW, ( vxui_demo_glow ) { .padding = 6.0f, .alpha = 0.20f } );
                    VXUI_TRAIT( VXUI_TRAIT_SCANLINE, ( vxui_demo_scanline ) { .spacing = 6.0f, .alpha = 0.10f } );
                }

                VXUI( &ctx, "main.meta", {
                    .layout = {
                        .sizing = { CLAY_SIZING_FIT( 0 ), CLAY_SIZING_FIT( 0 ) },
                        .childGap = VXUI_DEMO_SCREEN_GAP,
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    },
                } ) {
                    VXUI( &ctx, "main.prompts", {
                        .layout = {
                            .sizing = { CLAY_SIZING_FIT( 0 ), CLAY_SIZING_FIT( 0 ) },
                            .childGap = VXUI_DEMO_INLINE_GAP,
                            .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                            .layoutDirection = vxui__resolve_dir( CLAY_LEFT_TO_RIGHT, rtl ),
                        },
                    } ) {
                        vxui_demo_emit_prompt_pair( &ctx, "main.prompt.confirm", "action.confirm", "menu.confirm" );
                        vxui_demo_emit_prompt_pair( &ctx, "main.prompt.cancel", "action.cancel", "menu.cancel" );
                    }

                    VXUI( &ctx, "main.status", {
                        .layout = {
                            .sizing = { CLAY_SIZING_FIT( 0 ), CLAY_SIZING_FIT( 0 ) },
                            .childGap = VXUI_DEMO_ROW_GAP,
                            .layoutDirection = CLAY_TOP_TO_BOTTOM,
                        },
                    } ) {
                        vxui_demo_emit_label_pair( &ctx, "main.status.locale", "status.label.locale", locale_name_key, rtl );
                        vxui_demo_emit_label_pair( &ctx, "main.status.prompts", "status.label.prompts", prompt_name_key, rtl );
                        vxui_demo_emit_label_pair( &ctx, "main.status.screens", "status.label.screens", top_name_key, rtl );
                    }
                }

                VXUI( &ctx, "main.help", {
                    .layout = {
                        .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                        .childGap = VXUI_DEMO_ROW_GAP,
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    },
                } ) {
                    VXUI_LABEL( &ctx, "menu.controls", section_cfg );
                    VXUI_LABEL( &ctx, "hint.controls.0", ( vxui_label_cfg ) { 0 } );
                    VXUI_LABEL( &ctx, "hint.controls.1", ( vxui_label_cfg ) { 0 } );
                    VXUI_LABEL( &ctx, "hint.controls.2", ( vxui_label_cfg ) { 0 } );
                    VXUI_LABEL( &ctx, "hint.controls.3", ( vxui_label_cfg ) { 0 } );
                }
                } /* main.content */
            } /* main_menu */
        }

#ifdef VXUI_DEBUG
        if ( ctx.debug_seq_editor.open ) {
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
                    .padding = CLAY_PADDING_ALL( 12 ),
                    .childGap = VXUI_DEMO_INLINE_GAP,
                    .childAlignment = { .x = ctx.rtl ? CLAY_ALIGN_X_RIGHT : CLAY_ALIGN_X_LEFT },
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                },
                .backgroundColor = { 22, 24, 38, 224 },
            } ) {
                VXUI_LABEL( &ctx, "debug.overlay.title", ( vxui_label_cfg ) {
                    .font_id = VXUI_DEMO_FONT_ROLE_SECTION,
                    .font_size = ( float ) VXUI_DEMO_SECTION_SIZE,
                    .color = { 255, 214, 153, 255 },
                } );
                VXUI_LABEL( &ctx, "menu.debug", ( vxui_label_cfg ) { 0 } );
                if ( seq_count > 0 ) {
                    VXUI_OPTION( &ctx, "debug.selected.seq", &ctx.debug_seq_editor.selected_seq, seq_names, seq_count, ( vxui_option_cfg ) { 0 } );
                    VXUI_LABEL( &ctx, "debug.selected_seq", ( vxui_label_cfg ) { 0 } );
                    VXUI_LABEL( &ctx, "debug.step.id", ( vxui_label_cfg ) { 0 } );
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
                VXUI_LABEL( &ctx, "debug.preview_status", ( vxui_label_cfg ) { 0 } );
                VXUI_LABEL( &ctx, "debug.reload", ( vxui_label_cfg ) { 0 } );
                VXUI_LABEL( &ctx, "debug.clipboard", ( vxui_label_cfg ) { 0 } );
                VXUI_LABEL( &ctx, "debug.generated_c", ( vxui_label_cfg ) {
                    .font_id = VXUI_DEMO_FONT_ROLE_CODE,
                    .font_size = ( float ) VXUI_DEMO_CODE_SIZE,
                    .color = { 184, 220, 255, 255 },
                } );
                VXUI_LABEL( &ctx, "debug.generated_toml", ( vxui_label_cfg ) {
                    .font_id = VXUI_DEMO_FONT_ROLE_CODE,
                    .font_size = ( float ) VXUI_DEMO_CODE_SIZE,
                    .color = { 184, 255, 208, 255 },
                } );
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

        glBindFramebuffer( GL_FRAMEBUFFER, 0 );
        glViewport( 0, 0, renderer.window_size.width, renderer.window_size.height );
        glDisable( GL_SCISSOR_TEST );
        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        glDisable( GL_FRAMEBUFFER_SRGB );
        glClearColor( 0.05f, 0.06f, 0.10f, 1.0f );
        glClear( GL_COLOR_BUFFER_BIT );

        vxui_demo_gl_debug_begin( &renderer, "VXUI Draw List" );
        vxui_demo_render_draw_list( &renderer, &list );
        vxui_demo_gl_debug_end( &renderer );
        vxui_flush_text( &ctx );
        vxui_demo_gl_debug_event( &renderer, "Present" );
        window->SwapDrawBuffers();
    }

    Clay_SetCurrentContext( nullptr );
    std::remove( app.watched_seq_path.c_str() );
    vxui_demo_shutdown_renderer( &renderer );
    manager->ShutDown();
    window.reset( nullptr );
    return 0;
}
