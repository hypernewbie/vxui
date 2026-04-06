#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>

#include "../demo/internal/demo_layout.h"
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

typedef struct locale_fixture
{
    uint8_t* memory;
    uint64_t memory_size;
    vxui_ctx ctx;
    const char* long_text;
    vxui_test_fontcache_handle* fontcache;
} locale_fixture;

static const char* vxui__locale_text( const char* key, void* userdata )
{
    locale_fixture* fixture = ( locale_fixture* ) userdata;

    if ( std::strcmp( key, "label.hello" ) == 0 ) {
        return "Hello";
    }
    if ( std::strcmp( key, "label.left" ) == 0 ) {
        return "Left";
    }
    if ( std::strcmp( key, "label.right" ) == 0 ) {
        return "Right";
    }
    if ( std::strcmp( key, "label.long" ) == 0 ) {
        return fixture->long_text;
    }
    if ( std::strcmp( key, "label.body" ) == 0 ) {
        return "Typography";
    }
    if ( std::strcmp( key, "label.title" ) == 0 ) {
        return "Typography";
    }
    if ( std::strcmp( key, "label.measure.small" ) == 0 ) {
        return "MMMMMM";
    }
    if ( std::strcmp( key, "label.measure.large" ) == 0 ) {
        return "MMMMMM";
    }
    if ( std::strcmp( key, "prompt.confirm.label" ) == 0 ) {
        return fixture->ctx.rtl ? "تأكيد" : "Confirm";
    }
    if ( std::strcmp( key, "status.label.screens" ) == 0 ) {
        return fixture->ctx.rtl ? "الشاشات" : "Screens";
    }
    if ( std::strcmp( key, "status.label.top" ) == 0 ) {
        return fixture->ctx.rtl ? "الأعلى" : "Top";
    }

    return key;
}

static float vxui__locale_find_text_x( const vxui_draw_list* list, const char* text )
{
    for ( int i = 0; i < list->length; ++i ) {
        if ( list->commands[ i ].type == VXUI_CMD_TEXT && list->commands[ i ].text.text && std::strcmp( list->commands[ i ].text.text, text ) == 0 ) {
            return list->commands[ i ].text.pos.x;
        }
    }
    return -1.0f;
}

static bool vxui__locale_find_text_pos( const vxui_draw_list* list, const char* text, vxui_vec2* out )
{
    if ( !list || !text ) {
        return false;
    }

    for ( int i = 0; i < list->length; ++i ) {
        if ( list->commands[ i ].type != VXUI_CMD_TEXT || !list->commands[ i ].text.text ) {
            continue;
        }
        if ( std::strcmp( list->commands[ i ].text.text, text ) != 0 ) {
            continue;
        }
        if ( out ) {
            *out = list->commands[ i ].text.pos;
        }
        return true;
    }
    return false;
}

static bool vxui__locale_text_starts_with( const char* text, const char* prefix )
{
    return text && prefix && std::strncmp( text, prefix, std::strlen( prefix ) ) == 0;
}

static bool vxui__locale_any_slot_text_below( const vxui_draw_list* list, float y_limit )
{
    if ( !list ) {
        return false;
    }

    for ( int i = 0; i < list->length; ++i ) {
        if ( list->commands[ i ].type != VXUI_CMD_TEXT || !vxui__locale_text_starts_with( list->commands[ i ].text.text, "slot." ) ) {
            continue;
        }
        const float clip_bottom = list->commands[ i ].clip_rect.y + list->commands[ i ].clip_rect.h;
        if ( list->commands[ i ].text.pos.y >= y_limit && clip_bottom > y_limit ) {
            return true;
        }
    }
    return false;
}

static bool vxui__locale_find_anim_bounds( const vxui_ctx* ctx, uint32_t id, vxui_rect* out )
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

static Clay_String vxui__locale_clay_string( const char* text )
{
    return ( Clay_String ) {
        .isStaticallyAllocated = false,
        .length = text ? ( int32_t ) std::strlen( text ) : 0,
        .chars = text ? text : "",
    };
}

static bool vxui__locale_find_element_bounds( const char* id, vxui_rect* out )
{
    Clay_ElementData element = Clay_GetElementData( Clay_GetElementId( vxui__locale_clay_string( id ) ) );
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

static bool vxui__locale_matches( const char* locale, const char* prefix )
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

static void vxui__locale_font_resolver(
    vxui_ctx* ctx,
    uint32_t requested_font_id,
    float requested_font_size,
    const char* locale,
    void* userdata,
    vxui_resolved_font* out )
{
    ( void ) ctx;
    locale_fixture* fixture = ( locale_fixture* ) userdata;
    if ( !fixture || !out ) {
        return;
    }

    switch ( requested_font_id ) {
        case VXUI_TEST_FONT_ROLE_BODY:
            out->font_id = requested_font_size >= 40.0f ? VXUI_TEST_FONT_UI_LARGE : VXUI_TEST_FONT_UI;
            out->line_height = requested_font_size >= 40.0f ? 48.0f : 28.0f;
            return;

        case VXUI_TEST_FONT_ROLE_TITLE:
            if ( vxui__locale_matches( locale, "ja" ) ) {
                out->font_id = VXUI_TEST_FONT_JAPANESE_TITLE;
            } else if ( vxui__locale_matches( locale, "ar" ) ) {
                out->font_id = VXUI_TEST_FONT_ARABIC_TITLE;
            } else {
                out->font_id = VXUI_TEST_FONT_TITLE;
            }
            out->line_height = 52.0f;
            return;

        default:
            return;
    }
}

template <typename TEmitControl>
static void vxui__locale_emit_form_row(
    locale_fixture* fixture,
    const char* id,
    const char* label_key,
    float label_lane_width,
    float control_lane_width,
    bool rtl,
    TEmitControl&& emit_control )
{
    CLAY( Clay_GetElementId( vxui__locale_clay_string( id ) ), {
        .layout = {
            .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
            .childGap = ( uint16_t ) VXUI_DEMO_LAYOUT_ROW_GAP,
            .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
            .layoutDirection = CLAY_LEFT_TO_RIGHT,
        },
    } ) {
        auto emit_label_lane = [&]() {
            const std::string label_lane_id = std::string( id ) + ".label_lane";
            CLAY( Clay_GetElementId( vxui__locale_clay_string( label_lane_id.c_str() ) ), {
                .layout = {
                    .sizing = { CLAY_SIZING_FIXED( label_lane_width ), CLAY_SIZING_FIT( 0 ) },
                    .childAlignment = { .x = rtl ? CLAY_ALIGN_X_RIGHT : CLAY_ALIGN_X_LEFT },
                },
            } ) {
                VXUI_LABEL( &fixture->ctx, label_key, ( vxui_label_cfg ) { 0 } );
            }
        };

        auto emit_control_lane = [&]() {
            const std::string control_lane_id = std::string( id ) + ".control";
            CLAY( Clay_GetElementId( vxui__locale_clay_string( control_lane_id.c_str() ) ), {
                .layout = {
                    .sizing = { CLAY_SIZING_FIXED( control_lane_width ), CLAY_SIZING_FIT( 0 ) },
                    .childAlignment = { .x = rtl ? CLAY_ALIGN_X_RIGHT : CLAY_ALIGN_X_LEFT },
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                },
            } ) {
                emit_control();
            }
        };

        if ( rtl ) {
            emit_control_lane();
            emit_label_lane();
        } else {
            emit_label_lane();
            emit_control_lane();
        }
    }
}

static void vxui__locale_emit_compact_meta_row( locale_fixture* fixture, const char* id, const char* label_key, const char* value_key, bool rtl )
{
    CLAY( Clay_GetElementId( vxui__locale_clay_string( id ) ), {
        .layout = {
            .sizing = { CLAY_SIZING_FIT( 0 ), CLAY_SIZING_FIT( 0 ) },
            .childGap = ( uint16_t ) VXUI_DEMO_LAYOUT_INLINE_GAP,
            .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
            .layoutDirection = CLAY_LEFT_TO_RIGHT,
        },
    } ) {
        if ( rtl ) {
            VXUI_LABEL( &fixture->ctx, value_key, ( vxui_label_cfg ) { 0 } );
            VXUI_LABEL( &fixture->ctx, label_key, ( vxui_label_cfg ) { 0 } );
        } else {
            VXUI_LABEL( &fixture->ctx, label_key, ( vxui_label_cfg ) { 0 } );
            VXUI_LABEL( &fixture->ctx, value_key, ( vxui_label_cfg ) { 0 } );
        }
    }
}

static void vxui__locale_emit_status_summary(
    locale_fixture* fixture,
    const char* id,
    const char* locale_name_key,
    const char* prompt_name_key,
    const char* top_name_key,
    int screen_count,
    bool rtl )
{
    const std::string primary_id = std::string( id ) + ".row.primary";
    const std::string secondary_id = std::string( id ) + ".row.secondary";
    const std::string locale_id = std::string( id ) + ".locale";
    const std::string prompts_id = std::string( id ) + ".prompts";
    const std::string screens_id = std::string( id ) + ".screens";
    const std::string top_id = std::string( id ) + ".top";

    CLAY( Clay_GetElementId( vxui__locale_clay_string( id ) ), {
        .layout = {
            .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
            .childGap = ( uint16_t ) VXUI_DEMO_LAYOUT_ROW_GAP,
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
    } ) {
        CLAY( Clay_GetElementId( vxui__locale_clay_string( primary_id.c_str() ) ), {
            .layout = {
                .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                .childGap = ( uint16_t ) VXUI_DEMO_LAYOUT_ROW_GAP,
                .layoutDirection = CLAY_LEFT_TO_RIGHT,
            },
        } ) {
            vxui__locale_emit_compact_meta_row( fixture, locale_id.c_str(), "status.label.locale", locale_name_key, rtl );
            vxui__locale_emit_compact_meta_row( fixture, prompts_id.c_str(), "status.label.prompts", prompt_name_key, rtl );
        }

        CLAY( Clay_GetElementId( vxui__locale_clay_string( secondary_id.c_str() ) ), {
            .layout = {
                .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                .childGap = ( uint16_t ) VXUI_DEMO_LAYOUT_ROW_GAP,
                .layoutDirection = CLAY_LEFT_TO_RIGHT,
            },
        } ) {
            CLAY( Clay_GetElementId( vxui__locale_clay_string( screens_id.c_str() ) ), {
                .layout = {
                    .sizing = { CLAY_SIZING_FIT( 0 ), CLAY_SIZING_FIT( 0 ) },
                },
            } ) {
                VXUI_VALUE( &fixture->ctx, "status.label.screens", ( float ) screen_count, ( vxui_value_cfg ) {
                    .font_id = VXUI_TEST_FONT_ROLE_BODY,
                    .font_size = 24.0f,
                    .format = "%.0f",
                } );
            }
            vxui__locale_emit_compact_meta_row( fixture, top_id.c_str(), "status.label.top", top_name_key, rtl );
        }
    }
}

static void vxui__locale_emit_controls_block( locale_fixture* fixture, const char* id )
{
    static const char* kHints[] = {
        "hint.controls.0",
        "hint.controls.1",
        "hint.controls.2",
        "hint.controls.3",
    };

    CLAY( Clay_GetElementId( vxui__locale_clay_string( id ) ), {
        .layout = {
            .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
            .padding = CLAY_PADDING_ALL( 18 ),
            .childGap = ( uint16_t ) VXUI_DEMO_LAYOUT_ROW_GAP,
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
    } ) {
        VXUI_LABEL( &fixture->ctx, "menu.controls", ( vxui_label_cfg ) {
            .font_id = VXUI_TEST_FONT_ROLE_BODY,
            .font_size = 24.0f,
        } );
        for ( const char* hint : kHints ) {
            VXUI_LABEL( &fixture->ctx, hint, ( vxui_label_cfg ) { 0 } );
        }
    }
}

static vxui_draw_list vxui__locale_render_settings_desktop_impl(
    locale_fixture* fixture,
    const char* locale,
    int screen_height,
    int extra_body_rows,
    float body_scroll_offset,
    const char* focused_id )
{
    static const char* kDifficultyKeys[] = { "difficulty.easy", "difficulty.normal", "difficulty.hard" };
    static const char* kSlotKeys[] = { "slot.0", "slot.1", "slot.2", "slot.3", "slot.4", "slot.5", "slot.6", "slot.7" };

    fixture->ctx.cfg.screen_width = 1280;
    fixture->ctx.cfg.screen_height = screen_height;
    vxui_set_font_resolver( &fixture->ctx, vxui__locale_font_resolver, fixture );
    vxui_set_locale( &fixture->ctx, locale );
    fixture->ctx.focused_id = focused_id ? vxui_id( focused_id ) : 0u;

    const bool rtl = fixture->ctx.rtl;
    const vxui_demo_surface_metrics metrics =
        vxui_demo_compute_surface_metrics( 1280.0f - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f, locale, VXUI_DEMO_SURFACE_SETTINGS );

    int difficulty = 1;
    float volume = 0.40f;
    vxui_begin( &fixture->ctx, 0.016f );
    VXUI( &fixture->ctx, "settings", {
        .layout = {
            .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_GROW( 0 ) },
            .padding = CLAY_PADDING_ALL( ( uint16_t ) VXUI_DEMO_LAYOUT_OUTER_PADDING ),
            .childAlignment = { .x = CLAY_ALIGN_X_CENTER },
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
    } ) {
        VXUI( &fixture->ctx, "settings.surface", {
            .layout = {
                .sizing = {
                    CLAY_SIZING_FIXED( metrics.surface_width ),
                    CLAY_SIZING_FIXED( ( float ) fixture->ctx.cfg.screen_height - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f ),
                },
                .padding = {
                    ( uint16_t ) VXUI_DEMO_LAYOUT_SURFACE_PADDING_X,
                    ( uint16_t ) VXUI_DEMO_LAYOUT_SURFACE_PADDING_X,
                    ( uint16_t ) VXUI_DEMO_LAYOUT_SURFACE_PADDING_Y,
                    ( uint16_t ) VXUI_DEMO_LAYOUT_SURFACE_PADDING_Y,
                },
                .childGap = ( uint16_t ) VXUI_DEMO_LAYOUT_SECTION_GAP,
                .childAlignment = { .x = rtl ? CLAY_ALIGN_X_RIGHT : CLAY_ALIGN_X_LEFT },
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
        } ) {
            VXUI( &fixture->ctx, "settings.header", {
                .layout = {
                    .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                    .padding = { 6, 0, 0, 0 },
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                },
            } ) {
                VXUI( &fixture->ctx, "settings.title", {
                    .layout = {
                        .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    },
                } ) {
                    VXUI_LABEL( &fixture->ctx, "label.title", ( vxui_label_cfg ) {
                        .font_id = VXUI_TEST_FONT_ROLE_TITLE,
                        .font_size = 44.0f,
                    } );
                }
            }

            VXUI( &fixture->ctx, "settings.body_viewport", {
                .layout = {
                    .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_GROW( 160.0f ) },
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                },
            } ) {
                CLAY( Clay_GetElementId( vxui__locale_clay_string( "settings.body_scroll" ) ), {
                    .layout = {
                        .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_GROW( 0 ) },
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    },
                    .clip = {
                        .horizontal = true,
                        .vertical = true,
                        .childOffset = { 0.0f, -body_scroll_offset },
                    },
                } ) {
                    VXUI( &fixture->ctx, "settings.body", {
                        .layout = {
                            .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                            .childGap = ( uint16_t ) VXUI_DEMO_LAYOUT_SECTION_GAP,
                            .layoutDirection = CLAY_TOP_TO_BOTTOM,
                        },
                    } ) {
                    VXUI( &fixture->ctx, "settings.controls_section", {
                        .layout = {
                            .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                            .padding = CLAY_PADDING_ALL( 18 ),
                            .childGap = ( uint16_t ) VXUI_DEMO_LAYOUT_ROW_GAP,
                            .layoutDirection = CLAY_TOP_TO_BOTTOM,
                        },
                    } ) {
                        vxui__locale_emit_form_row( fixture, "settings.form.difficulty.row", "label.left", metrics.label_lane_width, metrics.control_lane_width, rtl, [&]() {
                            VXUI_OPTION( &fixture->ctx, "settings.difficulty", &difficulty, kDifficultyKeys, 3, ( vxui_option_cfg ) { 0 } );
                        } );
                        vxui__locale_emit_form_row( fixture, "settings.form.volume.row", "label.right", metrics.label_lane_width, metrics.control_lane_width, rtl, [&]() {
                            VXUI_SLIDER( &fixture->ctx, "settings.volume", &volume, 0.0f, 1.0f, ( vxui_slider_cfg ) {
                                .show_value = true,
                            } );
                        } );
                    }

                    VXUI( &fixture->ctx, "settings.saves_section", {
                        .layout = {
                            .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                            .padding = CLAY_PADDING_ALL( 16 ),
                            .childGap = 10,
                            .layoutDirection = CLAY_TOP_TO_BOTTOM,
                        },
                    } ) {
                        VXUI_LABEL( &fixture->ctx, "label.body", ( vxui_label_cfg ) { 0 } );
                        VXUI( &fixture->ctx, "settings.saves.row", {
                            .layout = {
                                .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                                .childGap = ( uint16_t ) VXUI_DEMO_LAYOUT_ROW_GAP,
                                .layoutDirection = CLAY_LEFT_TO_RIGHT,
                            },
                        } ) {
                            auto emit_spacer_lane = [&]() {
                                CLAY( Clay_GetElementId( vxui__locale_clay_string( "settings.saves.spacer" ) ), {
                                    .layout = {
                                        .sizing = { CLAY_SIZING_FIXED( metrics.label_lane_width ), CLAY_SIZING_FIT( 0 ) },
                                    },
                                } ) {}
                            };

                            auto emit_list_lane = [&]() {
                                CLAY( Clay_GetElementId( vxui__locale_clay_string( "settings.saves.lane" ) ), {
                                    .layout = {
                                        .sizing = { CLAY_SIZING_FIXED( metrics.control_lane_width ), CLAY_SIZING_FIT( 0 ) },
                                        .childGap = 8,
                                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                                    },
                                } ) {
                                    const int slot_count = 8 + extra_body_rows;
                                    for ( int i = 0; i < slot_count; ++i ) {
                                        std::string slot_id = "settings.saves.slot." + std::to_string( i );
                                        std::string slot_label = i < 8 ? kSlotKeys[ i ] : ( "slot." + std::to_string( i ) );
                                        CLAY( Clay_GetElementId( vxui__locale_clay_string( slot_id.c_str() ) ), {
                                            .layout = {
                                                .sizing = { CLAY_SIZING_FIXED( metrics.control_lane_width ), CLAY_SIZING_FIXED( 44.0f ) },
                                                .padding = { 14, 14, 8, 8 },
                                                .childAlignment = { .x = rtl ? CLAY_ALIGN_X_RIGHT : CLAY_ALIGN_X_LEFT, .y = CLAY_ALIGN_Y_CENTER },
                                            },
                                        } ) {
                                            VXUI_LABEL( &fixture->ctx, slot_label.c_str(), ( vxui_label_cfg ) { 0 } );
                                        }
                                    }
                                }
                            };

                            if ( rtl ) {
                                emit_list_lane();
                                emit_spacer_lane();
                            } else {
                                emit_spacer_lane();
                                emit_list_lane();
                            }
                        }
                    }
                    }
                }
            }

            VXUI( &fixture->ctx, "settings.footer", {
                .layout = {
                    .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                    .padding = CLAY_PADDING_ALL( 18 ),
                    .childGap = ( uint16_t ) VXUI_DEMO_LAYOUT_ROW_GAP,
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                },
            } ) {
                VXUI( &fixture->ctx, "settings.footer.actions", {
                    .layout = {
                        .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                        .childGap = ( uint16_t ) VXUI_DEMO_LAYOUT_ROW_GAP,
                        .layoutDirection = CLAY_LEFT_TO_RIGHT,
                    },
                } ) {
                    VXUI_LABEL( &fixture->ctx, "label.hello", ( vxui_label_cfg ) { 0 } );
                    VXUI_LABEL( &fixture->ctx, "label.body", ( vxui_label_cfg ) { 0 } );
                }
                VXUI( &fixture->ctx, "settings.status", {
                    .layout = {
                        .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                    },
                } ) {
                    vxui__locale_emit_status_summary( fixture, "settings.status.summary", "label.hello", "label.right", "label.left", 2, rtl );
                }
            }
        }
    }
    return vxui_end( &fixture->ctx );
}

static void vxui__locale_render_settings_desktop( locale_fixture* fixture, const char* locale )
{
    ( void ) vxui__locale_render_settings_desktop_impl( fixture, locale, 720, 0, 0.0f, nullptr );
}

static void vxui__locale_render_settings_desktop_overflow( locale_fixture* fixture, const char* locale )
{
    ( void ) vxui__locale_render_settings_desktop_impl( fixture, locale, 520, 10, 0.0f, nullptr );
}

static void vxui__locale_render_main_menu_desktop( locale_fixture* fixture, const char* locale )
{
    vxui_input_table table = {
        .confirm = { VXUI_TEST_FONT_ROLE_BODY, 'A' },
        .cancel = { VXUI_TEST_FONT_ROLE_BODY, 'B' },
    };

    fixture->ctx.cfg.screen_width = 1280;
    fixture->ctx.cfg.screen_height = 720;
    vxui_set_font_resolver( &fixture->ctx, vxui__locale_font_resolver, fixture );
    vxui_set_input_table( &fixture->ctx, &table );
    vxui_set_locale( &fixture->ctx, locale );

    const bool rtl = fixture->ctx.rtl;
    const vxui_demo_surface_metrics metrics =
        vxui_demo_compute_surface_metrics( 1280.0f - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f, locale, VXUI_DEMO_SURFACE_MAIN_MENU );

    vxui_begin( &fixture->ctx, 0.016f );
    VXUI( &fixture->ctx, "main_menu", {
        .layout = {
            .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_GROW( 0 ) },
            .padding = CLAY_PADDING_ALL( ( uint16_t ) VXUI_DEMO_LAYOUT_OUTER_PADDING ),
            .childAlignment = { .x = CLAY_ALIGN_X_CENTER },
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
    } ) {
        VXUI( &fixture->ctx, "main.surface", {
            .layout = {
                .sizing = { CLAY_SIZING_FIXED( metrics.surface_width ), CLAY_SIZING_FIT( 0 ) },
                .padding = {
                    ( uint16_t ) VXUI_DEMO_LAYOUT_SURFACE_PADDING_X,
                    ( uint16_t ) VXUI_DEMO_LAYOUT_SURFACE_PADDING_X,
                    ( uint16_t ) VXUI_DEMO_LAYOUT_SURFACE_PADDING_Y,
                    ( uint16_t ) VXUI_DEMO_LAYOUT_SURFACE_PADDING_Y,
                },
                .childGap = ( uint16_t ) VXUI_DEMO_LAYOUT_SECTION_GAP,
                .childAlignment = { .x = rtl ? CLAY_ALIGN_X_RIGHT : CLAY_ALIGN_X_LEFT },
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
        } ) {
            VXUI( &fixture->ctx, "main.hero", {
                .layout = {
                    .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                    .padding = CLAY_PADDING_ALL( 20 ),
                    .childGap = 14,
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                },
            } ) {
                VXUI_LABEL( &fixture->ctx, "label.title", ( vxui_label_cfg ) {
                    .font_id = VXUI_TEST_FONT_ROLE_TITLE,
                    .font_size = 44.0f,
                } );
                VXUI_LABEL( &fixture->ctx, "label.hello", ( vxui_label_cfg ) { 0 } );
            }

            VXUI( &fixture->ctx, "main.meta", {
                .layout = {
                    .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                    .padding = CLAY_PADDING_ALL( 18 ),
                    .childGap = ( uint16_t ) VXUI_DEMO_LAYOUT_ROW_GAP,
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                },
            } ) {
                VXUI( &fixture->ctx, "main.prompts", {
                    .layout = {
                        .sizing = { CLAY_SIZING_FIT( 0 ), CLAY_SIZING_FIT( 0 ) },
                        .childGap = ( uint16_t ) VXUI_DEMO_LAYOUT_INLINE_GAP,
                        .layoutDirection = CLAY_LEFT_TO_RIGHT,
                    },
                } ) {
                    VXUI_PROMPT( &fixture->ctx, "action.confirm" );
                    VXUI_LABEL( &fixture->ctx, "label.hello", ( vxui_label_cfg ) { 0 } );
                    VXUI_PROMPT( &fixture->ctx, "action.cancel" );
                    VXUI_LABEL( &fixture->ctx, "label.right", ( vxui_label_cfg ) { 0 } );
                }
                VXUI( &fixture->ctx, "main.status", {
                    .layout = {
                        .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                    },
                } ) {
                    vxui__locale_emit_status_summary( fixture, "main.status.summary", "label.hello", "label.right", "label.left", 1, rtl );
                }
            }

            vxui__locale_emit_controls_block( fixture, "main.help" );
        }
    }
    vxui_end( &fixture->ctx );
}

UTEST_F_SETUP( locale_fixture )
{
    static const char* kLongText = "長い長い長い長い長い長い長い長い長い長い長い長い長い長い長い長い長い長い長い長い";

    utest_fixture->memory_size = vxui_min_memory_size();
    utest_fixture->memory = ( uint8_t* ) std::malloc( ( size_t ) utest_fixture->memory_size );
    ASSERT_TRUE( utest_fixture->memory != nullptr );
    utest_fixture->long_text = kLongText;

    vxui_init(
        &utest_fixture->ctx,
        vxui_create_arena( utest_fixture->memory_size, utest_fixture->memory ),
        ( vxui_config ) {
            .screen_width = 640,
            .screen_height = 360,
            .max_elements = 128,
        } );
    vxui_set_text_fn( &utest_fixture->ctx, vxui__locale_text, utest_fixture );
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

UTEST_F_TEARDOWN( locale_fixture )
{
    vxui_shutdown( &utest_fixture->ctx );
    vxui_test_fontcache_destroy( utest_fixture->fontcache );
    utest_fixture->fontcache = nullptr;
    std::free( utest_fixture->memory );
    utest_fixture->memory = nullptr;
    utest_fixture->memory_size = 0;
}

UTEST_F( locale_fixture, locale_string_is_stored_correctly )
{
    vxui_set_locale( &utest_fixture->ctx, "ja-JP" );
    EXPECT_STREQ( utest_fixture->ctx.locale, "ja-JP" );
}

UTEST_F( locale_fixture, locale_font_mapping_overrides_default_font )
{
    vxui_set_locale_font( &utest_fixture->ctx, "ja", VXUI_TEST_FONT_JAPANESE );
    vxui_set_locale( &utest_fixture->ctx, "ja" );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {} ) {
        VXUI_LABEL( &utest_fixture->ctx, "label.hello", ( vxui_label_cfg ) { 0 } );
    }
    vxui_end( &utest_fixture->ctx );

    EXPECT_EQ( utest_fixture->ctx.text_queue[ 0 ].font_id, VXUI_TEST_FONT_JAPANESE );
}

UTEST_F( locale_fixture, missing_locale_mapping_falls_back_to_default_font )
{
    utest_fixture->ctx.default_font_id = VXUI_TEST_FONT_TITLE;
    vxui_set_locale( &utest_fixture->ctx, "fr" );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {} ) {
        VXUI_LABEL( &utest_fixture->ctx, "label.hello", ( vxui_label_cfg ) { 0 } );
    }
    vxui_end( &utest_fixture->ctx );

    EXPECT_EQ( utest_fixture->ctx.text_queue[ 0 ].font_id, VXUI_TEST_FONT_TITLE );
}

UTEST_F( locale_fixture, prefix_locale_lookup_works_for_ja_jp )
{
    vxui_set_locale_font( &utest_fixture->ctx, "ja", VXUI_TEST_FONT_JAPANESE );
    vxui_set_locale( &utest_fixture->ctx, "ja-JP" );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {} ) {
        VXUI_LABEL( &utest_fixture->ctx, "label.hello", ( vxui_label_cfg ) { 0 } );
    }
    vxui_end( &utest_fixture->ctx );

    EXPECT_EQ( utest_fixture->ctx.text_queue[ 0 ].font_id, VXUI_TEST_FONT_JAPANESE );
}

UTEST_F( locale_fixture, input_table_swap_changes_prompt_source )
{
    vxui_input_table keyboard = {
        .confirm = { VXUI_TEST_FONT_UI, 'E' },
    };
    vxui_input_table gamepad = {
        .confirm = { VXUI_TEST_FONT_DEBUG, 'A' },
    };

    vxui_set_input_table( &utest_fixture->ctx, &keyboard );
    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {} ) {
        VXUI_PROMPT( &utest_fixture->ctx, "action.confirm" );
    }
    vxui_end( &utest_fixture->ctx );
    EXPECT_STREQ( utest_fixture->ctx.text_queue[ 0 ].text, "E" );
    EXPECT_EQ( utest_fixture->ctx.text_queue[ 0 ].font_id, VXUI_TEST_FONT_UI );

    vxui_set_input_table( &utest_fixture->ctx, &gamepad );
    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {} ) {
        VXUI_PROMPT( &utest_fixture->ctx, "action.confirm" );
    }
    vxui_end( &utest_fixture->ctx );
    EXPECT_STREQ( utest_fixture->ctx.text_queue[ 0 ].text, "A" );
    EXPECT_EQ( utest_fixture->ctx.text_queue[ 0 ].font_id, VXUI_TEST_FONT_DEBUG );
}

UTEST_F( locale_fixture, rtl_detection_for_ar_returns_true )
{
    vxui_set_locale( &utest_fixture->ctx, "ar" );
    EXPECT_TRUE( utest_fixture->ctx.rtl );
}

UTEST_F( locale_fixture, plain_label_rows_preserve_explicit_order_under_rtl )
{
    uint32_t ids_ltr[ 2 ] = {};
    uint32_t ids_rtl[ 2 ] = {};

    vxui_set_locale( &utest_fixture->ctx, "en" );
    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "row", {
        .layout = {
            .childGap = 20,
            .layoutDirection = CLAY_LEFT_TO_RIGHT,
        },
    } ) {
        VXUI_LABEL( &utest_fixture->ctx, "label.left", ( vxui_label_cfg ) { 0 } );
        VXUI_LABEL( &utest_fixture->ctx, "label.right", ( vxui_label_cfg ) { 0 } );
    }
    vxui_draw_list ltr = vxui_end( &utest_fixture->ctx );
    ids_ltr[ 0 ] = utest_fixture->ctx.decls[ 0 ].id;
    ids_ltr[ 1 ] = utest_fixture->ctx.decls[ 1 ].id;
    float left_x_ltr = vxui__locale_find_text_x( &ltr, "Left" );
    float right_x_ltr = vxui__locale_find_text_x( &ltr, "Right" );

    vxui_set_locale( &utest_fixture->ctx, "ar" );
    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "row", {
        .layout = {
            .childGap = 20,
            .layoutDirection = CLAY_LEFT_TO_RIGHT,
        },
    } ) {
        VXUI_LABEL( &utest_fixture->ctx, "label.left", ( vxui_label_cfg ) { 0 } );
        VXUI_LABEL( &utest_fixture->ctx, "label.right", ( vxui_label_cfg ) { 0 } );
    }
    vxui_draw_list rtl = vxui_end( &utest_fixture->ctx );
    ids_rtl[ 0 ] = utest_fixture->ctx.decls[ 0 ].id;
    ids_rtl[ 1 ] = utest_fixture->ctx.decls[ 1 ].id;
    float left_x_rtl = vxui__locale_find_text_x( &rtl, "Left" );
    float right_x_rtl = vxui__locale_find_text_x( &rtl, "Right" );

    EXPECT_EQ( ids_ltr[ 0 ], ids_rtl[ 0 ] );
    EXPECT_EQ( ids_ltr[ 1 ], ids_rtl[ 1 ] );
    EXPECT_TRUE( left_x_ltr < right_x_ltr );
    EXPECT_TRUE( left_x_rtl < right_x_rtl );
}

UTEST_F( locale_fixture, value_widget_swaps_label_and_value_order_in_rtl )
{
    vxui_set_font_resolver( &utest_fixture->ctx, vxui__locale_font_resolver, utest_fixture );

    vxui_set_locale( &utest_fixture->ctx, "en" );
    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {} ) {
        VXUI_VALUE( &utest_fixture->ctx, "status.label.screens", 2.0f, ( vxui_value_cfg ) {
            .font_id = VXUI_TEST_FONT_ROLE_BODY,
            .font_size = 24.0f,
            .format = "%.0f",
        } );
    }
    vxui_draw_list ltr = vxui_end( &utest_fixture->ctx );
    float label_x_ltr = vxui__locale_find_text_x( &ltr, "Screens" );
    float value_x_ltr = vxui__locale_find_text_x( &ltr, "2" );
    uint32_t value_id_ltr = utest_fixture->ctx.decls[ 0 ].id;

    vxui_set_locale( &utest_fixture->ctx, "ar" );
    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {} ) {
        VXUI_VALUE( &utest_fixture->ctx, "status.label.screens", 2.0f, ( vxui_value_cfg ) {
            .font_id = VXUI_TEST_FONT_ROLE_BODY,
            .font_size = 24.0f,
            .format = "%.0f",
        } );
    }
    vxui_draw_list rtl = vxui_end( &utest_fixture->ctx );
    float label_x_rtl = vxui__locale_find_text_x( &rtl, "الشاشات" );
    float value_x_rtl = vxui__locale_find_text_x( &rtl, "2" );
    uint32_t value_id_rtl = utest_fixture->ctx.decls[ 0 ].id;

    EXPECT_EQ( value_id_ltr, value_id_rtl );
    EXPECT_TRUE( label_x_ltr < value_x_ltr );
    EXPECT_TRUE( value_x_rtl < label_x_rtl );
}

UTEST_F( locale_fixture, font_resolver_maps_title_faces_by_locale )
{
    vxui_set_font_resolver( &utest_fixture->ctx, vxui__locale_font_resolver, utest_fixture );

    struct locale_case
    {
        const char* locale;
        uint32_t expected_font_id;
    };
    const locale_case cases[] = {
        { "en", VXUI_TEST_FONT_TITLE },
        { "ja-JP", VXUI_TEST_FONT_JAPANESE_TITLE },
        { "ar", VXUI_TEST_FONT_ARABIC_TITLE },
    };

    for ( const locale_case& test_case : cases ) {
        vxui_set_locale( &utest_fixture->ctx, test_case.locale );
        vxui_begin( &utest_fixture->ctx, 0.016f );
        VXUI( &utest_fixture->ctx, "root", {} ) {
            VXUI_LABEL( &utest_fixture->ctx, "label.title", ( vxui_label_cfg ) {
                .font_id = VXUI_TEST_FONT_ROLE_TITLE,
                .font_size = 44.0f,
            } );
        }
        vxui_end( &utest_fixture->ctx );

        ASSERT_TRUE( utest_fixture->ctx.text_queue_count >= 1 );
        EXPECT_EQ( utest_fixture->ctx.text_queue[ 0 ].font_id, test_case.expected_font_id );
    }
}

UTEST_F( locale_fixture, resolved_line_height_changes_text_bounds )
{
    vxui_set_font_resolver( &utest_fixture->ctx, vxui__locale_font_resolver, utest_fixture );
    vxui_set_locale( &utest_fixture->ctx, "en" );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {
        .layout = {
            .childGap = 8,
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
    } ) {
        VXUI_LABEL( &utest_fixture->ctx, "label.body", ( vxui_label_cfg ) {
            .font_id = VXUI_TEST_FONT_ROLE_BODY,
            .font_size = 24.0f,
        } );
        VXUI_LABEL( &utest_fixture->ctx, "label.title", ( vxui_label_cfg ) {
            .font_id = VXUI_TEST_FONT_ROLE_TITLE,
            .font_size = 44.0f,
        } );
    }
    vxui_end( &utest_fixture->ctx );

    vxui_rect body_bounds = {};
    vxui_rect title_bounds = {};
    ASSERT_TRUE( vxui__locale_find_anim_bounds( &utest_fixture->ctx, vxui_id( "label.body" ), &body_bounds ) );
    ASSERT_TRUE( vxui__locale_find_anim_bounds( &utest_fixture->ctx, vxui_id( "label.title" ), &title_bounds ) );
    EXPECT_GT( body_bounds.h, 20.0f );
    EXPECT_GT( title_bounds.h, body_bounds.h );
}

UTEST_F( locale_fixture, resolver_uses_requested_size_for_measured_width )
{
    vxui_set_font_resolver( &utest_fixture->ctx, vxui__locale_font_resolver, utest_fixture );
    vxui_set_locale( &utest_fixture->ctx, "en" );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {
        .layout = {
            .childGap = 8,
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
    } ) {
        VXUI_LABEL( &utest_fixture->ctx, "label.measure.small", ( vxui_label_cfg ) {
            .font_id = VXUI_TEST_FONT_ROLE_BODY,
            .font_size = 24.0f,
        } );
        VXUI_LABEL( &utest_fixture->ctx, "label.measure.large", ( vxui_label_cfg ) {
            .font_id = VXUI_TEST_FONT_ROLE_BODY,
            .font_size = 44.0f,
        } );
    }
    vxui_end( &utest_fixture->ctx );

    vxui_rect small_bounds = {};
    vxui_rect large_bounds = {};
    ASSERT_TRUE( vxui__locale_find_anim_bounds( &utest_fixture->ctx, vxui_id( "label.measure.small" ), &small_bounds ) );
    ASSERT_TRUE( vxui__locale_find_anim_bounds( &utest_fixture->ctx, vxui_id( "label.measure.large" ), &large_bounds ) );
    EXPECT_EQ( utest_fixture->ctx.text_queue[ 0 ].font_id, ( uint32_t ) VXUI_TEST_FONT_UI );
    EXPECT_EQ( utest_fixture->ctx.text_queue[ 1 ].font_id, ( uint32_t ) VXUI_TEST_FONT_UI_LARGE );
    EXPECT_GT( small_bounds.w, 0.0f );
    EXPECT_GT( large_bounds.w, small_bounds.w );
}

UTEST_F( locale_fixture, arabic_prompt_and_status_rows_stay_on_screen_with_semantic_order )
{
    vxui_set_font_resolver( &utest_fixture->ctx, vxui__locale_font_resolver, utest_fixture );
    vxui_set_locale( &utest_fixture->ctx, "ar" );

    vxui_input_table table = {
        .confirm = { VXUI_TEST_FONT_ROLE_BODY, 'A' },
    };
    vxui_set_input_table( &utest_fixture->ctx, &table );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {
        .layout = {
            .sizing = { CLAY_SIZING_FIXED( 640 ), CLAY_SIZING_GROW( 0 ) },
            .childGap = 12,
            .childAlignment = { .x = CLAY_ALIGN_X_RIGHT },
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
    } ) {
        VXUI( &utest_fixture->ctx, "prompt.row", {
            .layout = {
                .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                .childGap = 8,
                .childAlignment = { .x = CLAY_ALIGN_X_RIGHT, .y = CLAY_ALIGN_Y_CENTER },
                .layoutDirection = CLAY_LEFT_TO_RIGHT,
            },
        } ) {
            VXUI_PROMPT( &utest_fixture->ctx, "action.confirm" );
            VXUI_LABEL( &utest_fixture->ctx, "prompt.confirm.label", ( vxui_label_cfg ) {
                .font_id = VXUI_TEST_FONT_ROLE_BODY,
                .font_size = 24.0f,
            } );
        }

        VXUI( &utest_fixture->ctx, "status.row", {
            .layout = {
                .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                .childGap = 8,
                .childAlignment = { .x = CLAY_ALIGN_X_RIGHT, .y = CLAY_ALIGN_Y_CENTER },
                .layoutDirection = CLAY_LEFT_TO_RIGHT,
            },
        } ) {
            VXUI_VALUE( &utest_fixture->ctx, "status.label.screens", 2.0f, ( vxui_value_cfg ) {
                .font_id = VXUI_TEST_FONT_ROLE_BODY,
                .font_size = 24.0f,
                .format = "%.0f",
            } );
        }
    }
    vxui_draw_list list = vxui_end( &utest_fixture->ctx );

    vxui_vec2 prompt_pos = {};
    vxui_vec2 prompt_label_pos = {};
    vxui_vec2 value_pos = {};
    vxui_vec2 status_label_pos = {};
    ASSERT_TRUE( vxui__locale_find_text_pos( &list, "A", &prompt_pos ) );
    ASSERT_TRUE( vxui__locale_find_text_pos( &list, "تأكيد", &prompt_label_pos ) );
    ASSERT_TRUE( vxui__locale_find_text_pos( &list, "2", &value_pos ) );
    ASSERT_TRUE( vxui__locale_find_text_pos( &list, "الشاشات", &status_label_pos ) );
    EXPECT_LT( prompt_pos.x, prompt_label_pos.x );
    EXPECT_LT( value_pos.x, status_label_pos.x );

    for ( int i = 0; i < list.length; ++i ) {
        if ( list.commands[ i ].type != VXUI_CMD_TEXT ) {
            continue;
        }
        EXPECT_GE( list.commands[ i ].text.pos.x, 0.0f );
        EXPECT_LE( list.commands[ i ].text.pos.x, ( float ) utest_fixture->ctx.cfg.screen_width );
        EXPECT_GE( list.commands[ i ].text.pos.y, 0.0f );
        EXPECT_LT( list.commands[ i ].text.pos.y, ( float ) utest_fixture->ctx.cfg.screen_height );
    }
}

UTEST_F( locale_fixture, repeated_locale_swaps_do_not_corrupt_font_mapping )
{
    vxui_set_locale_font( &utest_fixture->ctx, "en", VXUI_TEST_FONT_UI );
    vxui_set_locale_font( &utest_fixture->ctx, "ja", VXUI_TEST_FONT_JAPANESE );
    vxui_set_locale_font( &utest_fixture->ctx, "ar", VXUI_TEST_FONT_ARABIC );

    const char* locales[] = { "en", "ja", "ar", "ja-JP", "en" };
    const uint32_t expected_fonts[] = { VXUI_TEST_FONT_UI, VXUI_TEST_FONT_JAPANESE, VXUI_TEST_FONT_ARABIC, VXUI_TEST_FONT_JAPANESE, VXUI_TEST_FONT_UI };
    for ( int i = 0; i < 5; ++i ) {
        vxui_set_locale( &utest_fixture->ctx, locales[ i ] );
        vxui_begin( &utest_fixture->ctx, 0.016f );
        VXUI( &utest_fixture->ctx, "root", {} ) {
            VXUI_LABEL( &utest_fixture->ctx, "label.hello", ( vxui_label_cfg ) { 0 } );
        }
        vxui_end( &utest_fixture->ctx );
        EXPECT_EQ( utest_fixture->ctx.text_queue[ 0 ].font_id, expected_fonts[ i ] );
    }
}

UTEST_F( locale_fixture, named_demo_font_map_matches_expected_locale_faces )
{
    utest_fixture->ctx.default_font_id = VXUI_TEST_FONT_UI;
    vxui_set_locale_font( &utest_fixture->ctx, "ja", VXUI_TEST_FONT_JAPANESE );
    vxui_set_locale_font( &utest_fixture->ctx, "ar", VXUI_TEST_FONT_ARABIC );

    const char* locales[] = { "en", "ja-JP", "ar" };
    const uint32_t expected_fonts[] = { VXUI_TEST_FONT_UI, VXUI_TEST_FONT_JAPANESE, VXUI_TEST_FONT_ARABIC };
    for ( int i = 0; i < 3; ++i ) {
        vxui_set_locale( &utest_fixture->ctx, locales[ i ] );
        vxui_begin( &utest_fixture->ctx, 0.016f );
        VXUI( &utest_fixture->ctx, "root", {} ) {
            VXUI_LABEL( &utest_fixture->ctx, "label.hello", ( vxui_label_cfg ) { 0 } );
        }
        vxui_end( &utest_fixture->ctx );
        EXPECT_EQ( utest_fixture->ctx.text_queue[ 0 ].font_id, expected_fonts[ i ] );
    }
}

UTEST_F( locale_fixture, long_utf8_strings_still_measure_safely )
{
    vxui_set_locale( &utest_fixture->ctx, "ja" );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {} ) {
        VXUI_LABEL( &utest_fixture->ctx, "label.long", ( vxui_label_cfg ) { 0 } );
    }
    vxui_end( &utest_fixture->ctx );

    EXPECT_TRUE( utest_fixture->ctx.text_queue_count >= 1 );
    EXPECT_STREQ( utest_fixture->ctx.text_queue[ 0 ].text, utest_fixture->long_text );
}

UTEST_F( locale_fixture, repeated_label_keys_generate_unique_clay_ids )
{
    vxui_set_locale_font( &utest_fixture->ctx, "en", VXUI_TEST_FONT_UI );
    vxui_set_locale( &utest_fixture->ctx, "en" );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {} ) {
        VXUI_LABEL( &utest_fixture->ctx, "label.hello", ( vxui_label_cfg ) { 0 } );
        VXUI_LABEL( &utest_fixture->ctx, "label.hello", ( vxui_label_cfg ) { 0 } );
        VXUI_LABEL( &utest_fixture->ctx, "label.hello", ( vxui_label_cfg ) { 0 } );
        VXUI_LABEL( &utest_fixture->ctx, "label.left", ( vxui_label_cfg ) { 0 } );
    }
    vxui_end( &utest_fixture->ctx );

    ASSERT_GE( utest_fixture->ctx.decl_count, 4 );
    uint32_t decl_id_hello = vxui_id( "label.hello" );

    int hello_count = 0;
    for ( int i = 0; i < utest_fixture->ctx.decl_count; ++i ) {
        if ( utest_fixture->ctx.decls[ i ].kind != VXUI_DECL_LABEL ) continue;
        if ( utest_fixture->ctx.decls[ i ].id == decl_id_hello ) {
            hello_count++;
        }
    }
    EXPECT_GE( hello_count, 3 );

    for ( int i = 0; i < utest_fixture->ctx.decl_count; ++i ) {
        for ( int j = i + 1; j < utest_fixture->ctx.decl_count; ++j ) {
            if ( utest_fixture->ctx.decls[ i ].kind != VXUI_DECL_LABEL ) continue;
            if ( utest_fixture->ctx.decls[ j ].kind != VXUI_DECL_LABEL ) continue;
            EXPECT_NE( utest_fixture->ctx.decls[ i ].clay_id, utest_fixture->ctx.decls[ j ].clay_id );
        }
    }
}

UTEST_F( locale_fixture, repeated_value_keys_generate_unique_clay_ids )
{
    vxui_set_locale_font( &utest_fixture->ctx, "en", VXUI_TEST_FONT_UI );
    vxui_set_locale( &utest_fixture->ctx, "en" );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {} ) {
        VXUI_VALUE( &utest_fixture->ctx, "label.hello", 1.0f, ( vxui_value_cfg ) { .format = "%.0f" } );
        VXUI_VALUE( &utest_fixture->ctx, "label.hello", 2.0f, ( vxui_value_cfg ) { .format = "%.0f" } );
    }
    vxui_end( &utest_fixture->ctx );

    for ( int i = 0; i < utest_fixture->ctx.decl_count; ++i ) {
        for ( int j = i + 1; j < utest_fixture->ctx.decl_count; ++j ) {
            if ( utest_fixture->ctx.decls[ i ].kind != VXUI_DECL_VALUE ) continue;
            if ( utest_fixture->ctx.decls[ j ].kind != VXUI_DECL_VALUE ) continue;
            EXPECT_NE( utest_fixture->ctx.decls[ i ].clay_id, utest_fixture->ctx.decls[ j ].clay_id );
        }
    }
}

UTEST_F( locale_fixture, clay_ids_unique_across_screen_stack_snapshots )
{
    vxui_set_locale_font( &utest_fixture->ctx, "en", VXUI_TEST_FONT_UI );
    vxui_set_locale( &utest_fixture->ctx, "en" );

    vxui_push_screen( &utest_fixture->ctx, "screen_a" );
    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "screen_a", {} ) {
        VXUI_LABEL( &utest_fixture->ctx, "label.hello", ( vxui_label_cfg ) { 0 } );
    }
    vxui_end( &utest_fixture->ctx );

    vxui_push_screen( &utest_fixture->ctx, "screen_b" );
    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "screen_b", {} ) {
        VXUI_LABEL( &utest_fixture->ctx, "label.hello", ( vxui_label_cfg ) { 0 } );
    }
    vxui_draw_list list = vxui_end( &utest_fixture->ctx );

    bool found_text = false;
    for ( int i = 0; i < list.length; ++i ) {
        if ( list.commands[ i ].type == VXUI_CMD_TEXT ) {
            found_text = true;
            break;
        }
    }
    EXPECT_TRUE( found_text );
}

UTEST_F( locale_fixture, label_decl_id_stable_but_clay_id_unique )
{
    vxui_set_locale_font( &utest_fixture->ctx, "en", VXUI_TEST_FONT_UI );
    vxui_set_locale( &utest_fixture->ctx, "en" );

    uint32_t expected_decl_id = vxui_id( "label.hello" );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {} ) {
        VXUI_LABEL( &utest_fixture->ctx, "label.hello", ( vxui_label_cfg ) { 0 } );
        VXUI_LABEL( &utest_fixture->ctx, "label.hello", ( vxui_label_cfg ) { 0 } );
    }
    vxui_end( &utest_fixture->ctx );

    int label_count = 0;
    for ( int i = 0; i < utest_fixture->ctx.decl_count; ++i ) {
        if ( utest_fixture->ctx.decls[ i ].kind != VXUI_DECL_LABEL ) continue;
        label_count++;
        EXPECT_EQ( utest_fixture->ctx.decls[ i ].id, expected_decl_id );
        EXPECT_NE( utest_fixture->ctx.decls[ i ].clay_id, 0u );
    }
    ASSERT_GE( label_count, 2 );

    EXPECT_NE( utest_fixture->ctx.decls[ 0 ].clay_id, utest_fixture->ctx.decls[ 1 ].clay_id );
}

UTEST_F( locale_fixture, rtl_text_stays_within_screen_bounds )
{
    vxui_set_locale_font( &utest_fixture->ctx, "ar", VXUI_TEST_FONT_ARABIC );
    vxui_set_locale( &utest_fixture->ctx, "ar" );
    utest_fixture->ctx.rtl = true;

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {
        .layout = {
            .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_GROW( 0 ) },
            .padding = CLAY_PADDING_ALL( 18 ),
            .childGap = 12,
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
    } ) {
        VXUI_LABEL( &utest_fixture->ctx, "label.hello", ( vxui_label_cfg ) { 0 } );
        VXUI_LABEL( &utest_fixture->ctx, "label.right", ( vxui_label_cfg ) { 0 } );

        VXUI( &utest_fixture->ctx, "row.difficulty", {
            .layout = {
                .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                .childGap = 8,
                .childAlignment = { .x = CLAY_ALIGN_X_RIGHT, .y = CLAY_ALIGN_Y_CENTER },
                .layoutDirection = CLAY_LEFT_TO_RIGHT,
            },
        } ) {
            VXUI_LABEL( &utest_fixture->ctx, "label.right", ( vxui_label_cfg ) { 0 } );
            VXUI_LABEL( &utest_fixture->ctx, "label.left", ( vxui_label_cfg ) { 0 } );
        }
    }
    vxui_draw_list list = vxui_end( &utest_fixture->ctx );

    float sw = ( float ) utest_fixture->ctx.cfg.screen_width;
    float sh = ( float ) utest_fixture->ctx.cfg.screen_height;
    for ( int i = 0; i < list.length; ++i ) {
        if ( list.commands[ i ].type != VXUI_CMD_TEXT ) continue;
        EXPECT_GE( list.commands[ i ].text.pos.x, 0.0f );
        EXPECT_LE( list.commands[ i ].text.pos.x, sw );
        EXPECT_GE( list.commands[ i ].text.pos.y, 0.0f );
        EXPECT_LT( list.commands[ i ].text.pos.y, sh );
    }
}

UTEST_F( locale_fixture, settings_surface_is_desktop_scale )
{
    const char* locales[] = { "en", "ja-JP", "ar" };
    const float desktop_min_surface_width = vxui_demo_surface_desktop_min_width( VXUI_DEMO_SURFACE_SETTINGS );
    for ( const char* locale : locales ) {
        vxui__locale_render_settings_desktop( utest_fixture, locale );

        vxui_rect root = {};
        vxui_rect surface = {};
        vxui_rect header = {};
        vxui_rect body_viewport = {};
        vxui_rect body_scroll = {};
        vxui_rect footer = {};
        vxui_rect controls = {};
        vxui_rect saves = {};
        ASSERT_TRUE( vxui__locale_find_element_bounds( "settings", &root ) );
        ASSERT_TRUE( vxui__locale_find_element_bounds( "settings.surface", &surface ) );
        ASSERT_TRUE( vxui__locale_find_element_bounds( "settings.header", &header ) );
        ASSERT_TRUE( vxui__locale_find_element_bounds( "settings.body_viewport", &body_viewport ) );
        ASSERT_TRUE( vxui__locale_find_element_bounds( "settings.body_scroll", &body_scroll ) );
        ASSERT_TRUE( vxui__locale_find_element_bounds( "settings.footer", &footer ) );
        ASSERT_TRUE( vxui__locale_find_element_bounds( "settings.controls_section", &controls ) );
        ASSERT_TRUE( vxui__locale_find_element_bounds( "settings.saves_section", &saves ) );

        EXPECT_NEAR( root.w, ( float ) utest_fixture->ctx.cfg.screen_width, 1.0f );
        EXPECT_NEAR( root.h, ( float ) utest_fixture->ctx.cfg.screen_height, 1.0f );
        EXPECT_GT( surface.w, desktop_min_surface_width );
        EXPECT_LT( header.y, body_scroll.y );
        EXPECT_NEAR( body_viewport.y, body_scroll.y, 1.0f );
        EXPECT_NEAR( body_viewport.h, body_scroll.h, 1.0f );
        EXPECT_LT( body_scroll.y, footer.y );
        EXPECT_GE( body_scroll.h, 160.0f );
        EXPECT_GE( controls.y, body_scroll.y );
        EXPECT_GE( saves.y, controls.y );
        EXPECT_LE( footer.y + footer.h, surface.y + surface.h + 1.0f );
        EXPECT_GE( surface.x, VXUI_DEMO_LAYOUT_OUTER_PADDING );
        EXPECT_LE( surface.x + surface.w, ( float ) utest_fixture->ctx.cfg.screen_width - VXUI_DEMO_LAYOUT_OUTER_PADDING + 1.0f );
    }
}

UTEST_F( locale_fixture, demo_motion_contract_targets_bounded_surfaces )
{
    EXPECT_STREQ( vxui_demo_root_id( VXUI_DEMO_SURFACE_SETTINGS ), "settings" );
    EXPECT_STREQ( vxui_demo_surface_id( VXUI_DEMO_SURFACE_SETTINGS ), "settings.surface" );
    EXPECT_STREQ( vxui_demo_root_id( VXUI_DEMO_SURFACE_MAIN_MENU ), "main_menu" );
    EXPECT_STREQ( vxui_demo_surface_id( VXUI_DEMO_SURFACE_MAIN_MENU ), "main.surface" );
    EXPECT_NE( std::strcmp( vxui_demo_root_id( VXUI_DEMO_SURFACE_SETTINGS ), vxui_demo_surface_id( VXUI_DEMO_SURFACE_SETTINGS ) ), 0 );
    EXPECT_NE( std::strcmp( vxui_demo_root_id( VXUI_DEMO_SURFACE_MAIN_MENU ), vxui_demo_surface_id( VXUI_DEMO_SURFACE_MAIN_MENU ) ), 0 );
}

UTEST_F( locale_fixture, screen_stack_text_reuse_drawlist_valid )
{
    vxui_set_locale_font( &utest_fixture->ctx, "en", VXUI_TEST_FONT_UI );
    vxui_set_locale( &utest_fixture->ctx, "en" );

    vxui_push_screen( &utest_fixture->ctx, "screen_a" );
    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "screen_a", {} ) {
        VXUI_LABEL( &utest_fixture->ctx, "label.hello", ( vxui_label_cfg ) { 0 } );
        VXUI_LABEL( &utest_fixture->ctx, "label.left", ( vxui_label_cfg ) { 0 } );
    }
    vxui_end( &utest_fixture->ctx );

    vxui_push_screen( &utest_fixture->ctx, "screen_b" );
    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "screen_b", {} ) {
        VXUI_LABEL( &utest_fixture->ctx, "label.hello", ( vxui_label_cfg ) { 0 } );
    }
    vxui_draw_list list = vxui_end( &utest_fixture->ctx );

    int text_count = 0;
    for ( int i = 0; i < list.length; ++i ) {
        if ( list.commands[ i ].type == VXUI_CMD_TEXT ) {
            text_count++;
            EXPECT_TRUE( list.commands[ i ].text.text != nullptr );
        }
    }
    EXPECT_GE( text_count, 1 );
}

UTEST_F( locale_fixture, repeated_label_keys_both_appear_in_draw_list_without_conflict )
{
    vxui_set_locale_font( &utest_fixture->ctx, "en", VXUI_TEST_FONT_UI );
    vxui_set_locale( &utest_fixture->ctx, "en" );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {
        .layout = {
            .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_GROW( 0 ) },
            .childGap = 8,
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
    } ) {
        VXUI_LABEL( &utest_fixture->ctx, "label.hello", ( vxui_label_cfg ) { 0 } );
        VXUI_LABEL( &utest_fixture->ctx, "label.hello", ( vxui_label_cfg ) { 0 } );
    }
    vxui_draw_list list = vxui_end( &utest_fixture->ctx );

    int text_count = 0;
    for ( int i = 0; i < list.length; ++i ) {
        if ( list.commands[ i ].type == VXUI_CMD_TEXT ) {
            text_count++;
            EXPECT_TRUE( list.commands[ i ].text.text != nullptr );
            EXPECT_GE( list.commands[ i ].text.pos.x, 0.0f );
            EXPECT_GE( list.commands[ i ].text.pos.y, 0.0f );
        }
    }
    EXPECT_EQ( text_count, 2 );
}

UTEST_F( locale_fixture, prompt_pair_helper_uses_compact_fit_sizing )
{
    vxui_input_table table = {
        .confirm = { VXUI_TEST_FONT_ROLE_BODY, 'A' },
        .cancel = { VXUI_TEST_FONT_ROLE_BODY, 'B' },
    };
    vxui_set_input_table( &utest_fixture->ctx, &table );
    vxui_set_locale( &utest_fixture->ctx, "en" );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {
        .layout = {
            .sizing = { CLAY_SIZING_FIXED( 640 ), CLAY_SIZING_GROW( 0 ) },
            .padding = CLAY_PADDING_ALL( 18 ),
            .childGap = 12,
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
    } ) {
        CLAY( CLAY_ID( "prompt_pair" ), {
            .layout = {
                .sizing = { CLAY_SIZING_FIT( 0 ), CLAY_SIZING_FIT( 0 ) },
                .childGap = 8,
                .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                .layoutDirection = CLAY_LEFT_TO_RIGHT,
            },
        } ) {
            VXUI_PROMPT( &utest_fixture->ctx, "action.confirm" );
            VXUI_LABEL( &utest_fixture->ctx, "label.hello", ( vxui_label_cfg ) {
                .font_id = VXUI_TEST_FONT_ROLE_BODY,
                .font_size = 24.0f,
            } );
        }
    }
    vxui_draw_list list = vxui_end( &utest_fixture->ctx );

    vxui_vec2 confirm_pos = {};
    vxui_vec2 label_pos = {};
    ASSERT_TRUE( vxui__locale_find_text_pos( &list, "A", &confirm_pos ) );
    ASSERT_TRUE( vxui__locale_find_text_pos( &list, "Hello", &label_pos ) );
    float pair_width = label_pos.x + 100.0f - confirm_pos.x;
    EXPECT_LT( pair_width, 400.0f );
}

UTEST_F( locale_fixture, label_pair_helper_uses_compact_fit_sizing )
{
    vxui_set_locale( &utest_fixture->ctx, "en" );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {
        .layout = {
            .sizing = { CLAY_SIZING_FIXED( 640 ), CLAY_SIZING_GROW( 0 ) },
            .padding = CLAY_PADDING_ALL( 18 ),
            .childGap = 12,
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
    } ) {
        CLAY( CLAY_ID( "label_pair" ), {
            .layout = {
                .sizing = { CLAY_SIZING_FIT( 0 ), CLAY_SIZING_FIT( 0 ) },
                .childGap = 8,
                .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                .layoutDirection = CLAY_LEFT_TO_RIGHT,
            },
        } ) {
            VXUI_LABEL( &utest_fixture->ctx, "label.hello", ( vxui_label_cfg ) {
                .font_id = VXUI_TEST_FONT_ROLE_BODY,
                .font_size = 24.0f,
            } );
            VXUI_LABEL( &utest_fixture->ctx, "label.right", ( vxui_label_cfg ) {
                .font_id = VXUI_TEST_FONT_ROLE_BODY,
                .font_size = 24.0f,
            } );
        }
    }
    vxui_draw_list list = vxui_end( &utest_fixture->ctx );

    vxui_vec2 hello_pos = {};
    vxui_vec2 right_pos = {};
    ASSERT_TRUE( vxui__locale_find_text_pos( &list, "Hello", &hello_pos ) );
    ASSERT_TRUE( vxui__locale_find_text_pos( &list, "Right", &right_pos ) );
    float pair_width = right_pos.x + 100.0f - hello_pos.x;
    EXPECT_LT( pair_width, 400.0f );
}

UTEST_F( locale_fixture, status_rows_stay_compact )
{
    vxui_input_table table = {
        .confirm = { VXUI_TEST_FONT_ROLE_BODY, 'A' },
    };
    vxui_set_input_table( &utest_fixture->ctx, &table );
    vxui_set_locale( &utest_fixture->ctx, "en" );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {
        .layout = {
            .sizing = { CLAY_SIZING_FIXED( 640 ), CLAY_SIZING_GROW( 0 ) },
            .padding = CLAY_PADDING_ALL( 18 ),
            .childGap = 12,
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
    } ) {
        CLAY( CLAY_ID( "status_compact" ), {
            .layout = {
                .sizing = { CLAY_SIZING_FIT( 0 ), CLAY_SIZING_FIT( 0 ) },
                .childGap = 12,
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
        } ) {
            CLAY( CLAY_ID( "status_row_1" ), {
                .layout = {
                    .sizing = { CLAY_SIZING_FIT( 0 ), CLAY_SIZING_FIT( 0 ) },
                    .childGap = 8,
                    .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                    .layoutDirection = CLAY_LEFT_TO_RIGHT,
                },
            } ) {
                VXUI_LABEL( &utest_fixture->ctx, "label.hello", ( vxui_label_cfg ) {
                    .font_id = VXUI_TEST_FONT_ROLE_BODY,
                    .font_size = 24.0f,
                } );
                VXUI_VALUE( &utest_fixture->ctx, "label.value", 42.0f, ( vxui_value_cfg ) {
                    .font_id = VXUI_TEST_FONT_ROLE_BODY,
                    .font_size = 24.0f,
                    .format = "%.0f",
                } );
            }
            CLAY( CLAY_ID( "status_row_2" ), {
                .layout = {
                    .sizing = { CLAY_SIZING_FIT( 0 ), CLAY_SIZING_FIT( 0 ) },
                    .childGap = 8,
                    .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                    .layoutDirection = CLAY_LEFT_TO_RIGHT,
                },
            } ) {
                VXUI_PROMPT( &utest_fixture->ctx, "action.confirm" );
                VXUI_LABEL( &utest_fixture->ctx, "label.right", ( vxui_label_cfg ) {
                    .font_id = VXUI_TEST_FONT_ROLE_BODY,
                    .font_size = 24.0f,
                } );
            }
        }
    }
    vxui_draw_list list = vxui_end( &utest_fixture->ctx );

    vxui_vec2 row1_start = {};
    vxui_vec2 row2_start = {};
    ASSERT_TRUE( vxui__locale_find_text_pos( &list, "Hello", &row1_start ) );
    ASSERT_TRUE( vxui__locale_find_text_pos( &list, "A", &row2_start ) );
    EXPECT_GT( row2_start.y, row1_start.y );
}

UTEST_F( locale_fixture, form_controls_share_alignment )
{
    vxui_set_locale( &utest_fixture->ctx, "en" );

    vxui_begin( &utest_fixture->ctx, 0.016f );
    VXUI( &utest_fixture->ctx, "root", {
        .layout = {
            .sizing = { CLAY_SIZING_FIXED( 640 ), CLAY_SIZING_GROW( 0 ) },
            .padding = CLAY_PADDING_ALL( 18 ),
            .childGap = 12,
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
    } ) {
        CLAY( CLAY_ID( "form" ), {
            .layout = {
                .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                .childGap = 12,
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
        } ) {
            CLAY( CLAY_ID( "form_row_1" ), {
                .layout = {
                    .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                    .childGap = 12,
                    .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                    .layoutDirection = CLAY_LEFT_TO_RIGHT,
                },
            } ) {
                CLAY( CLAY_ID( "form_row_1_label" ), {
                    .layout = {
                        .sizing = { CLAY_SIZING_FIXED( 140.0f ), CLAY_SIZING_FIT( 0 ) },
                    },
                } ) {
                    VXUI_LABEL( &utest_fixture->ctx, "label.hello", ( vxui_label_cfg ) {
                        .font_id = VXUI_TEST_FONT_ROLE_BODY,
                        .font_size = 24.0f,
                    } );
                }
                VXUI_VALUE( &utest_fixture->ctx, "form.row1.value", 1.0f, ( vxui_value_cfg ) {
                    .font_id = VXUI_TEST_FONT_ROLE_BODY,
                    .font_size = 24.0f,
                    .format = "%.0f",
                } );
            }
            CLAY( CLAY_ID( "form_row_2" ), {
                .layout = {
                    .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                    .childGap = 12,
                    .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                    .layoutDirection = CLAY_LEFT_TO_RIGHT,
                },
            } ) {
                CLAY( CLAY_ID( "form_row_2_label" ), {
                    .layout = {
                        .sizing = { CLAY_SIZING_FIXED( 140.0f ), CLAY_SIZING_FIT( 0 ) },
                    },
                } ) {
                    VXUI_LABEL( &utest_fixture->ctx, "label.right", ( vxui_label_cfg ) {
                        .font_id = VXUI_TEST_FONT_ROLE_BODY,
                        .font_size = 24.0f,
                    } );
                }
                VXUI_VALUE( &utest_fixture->ctx, "form.row2.value", 2.0f, ( vxui_value_cfg ) {
                    .font_id = VXUI_TEST_FONT_ROLE_BODY,
                    .font_size = 24.0f,
                    .format = "%.0f",
                } );
            }
        }
    }
    vxui_draw_list list = vxui_end( &utest_fixture->ctx );

    vxui_vec2 row1_value_pos = {};
    vxui_vec2 row2_value_pos = {};
    ASSERT_TRUE( vxui__locale_find_text_pos( &list, "1", &row1_value_pos ) );
    ASSERT_TRUE( vxui__locale_find_text_pos( &list, "2", &row2_value_pos ) );
    float x_diff = row2_value_pos.x - row1_value_pos.x;
    EXPECT_LT( std::abs( x_diff ), 50.0f );
}

UTEST_F( locale_fixture, main_menu_surface_is_desktop_scale )
{
    const char* locales[] = { "en", "ja-JP", "ar" };
    const float desktop_min_surface_width = vxui_demo_surface_desktop_min_width( VXUI_DEMO_SURFACE_MAIN_MENU );
    for ( const char* locale : locales ) {
        vxui__locale_render_main_menu_desktop( utest_fixture, locale );

        vxui_rect root = {};
        vxui_rect surface = {};
        vxui_rect hero = {};
        vxui_rect meta = {};
        vxui_rect help = {};
        ASSERT_TRUE( vxui__locale_find_element_bounds( "main_menu", &root ) );
        ASSERT_TRUE( vxui__locale_find_element_bounds( "main.surface", &surface ) );
        ASSERT_TRUE( vxui__locale_find_element_bounds( "main.hero", &hero ) );
        ASSERT_TRUE( vxui__locale_find_element_bounds( "main.meta", &meta ) );
        ASSERT_TRUE( vxui__locale_find_element_bounds( "main.help", &help ) );

        EXPECT_NEAR( root.w, ( float ) utest_fixture->ctx.cfg.screen_width, 1.0f );
        EXPECT_NEAR( root.h, ( float ) utest_fixture->ctx.cfg.screen_height, 1.0f );
        EXPECT_GT( surface.w, desktop_min_surface_width );
        EXPECT_LT( hero.y, meta.y );
        EXPECT_LT( meta.y, help.y );
        EXPECT_GE( hero.w, surface.w * 0.70f );
        EXPECT_GE( meta.w, surface.w * 0.68f );
        EXPECT_GE( help.w, surface.w * 0.68f );
    }
}

UTEST_F( locale_fixture, main_menu_meta_rows_do_not_collapse_into_narrow_mobile_column )
{
    const char* locales[] = { "en", "ja-JP", "ar" };
    for ( const char* locale : locales ) {
        vxui__locale_render_main_menu_desktop( utest_fixture, locale );

        vxui_rect surface = {};
        vxui_rect meta = {};
        vxui_rect primary_row = {};
        vxui_rect secondary_row = {};
        ASSERT_TRUE( vxui__locale_find_element_bounds( "main.surface", &surface ) );
        ASSERT_TRUE( vxui__locale_find_element_bounds( "main.meta", &meta ) );
        ASSERT_TRUE( vxui__locale_find_element_bounds( "main.status.summary.row.primary", &primary_row ) );
        ASSERT_TRUE( vxui__locale_find_element_bounds( "main.status.summary.row.secondary", &secondary_row ) );

        EXPECT_GE( meta.w, surface.w * 0.68f );
        EXPECT_GE( primary_row.w, meta.w * 0.60f );
        EXPECT_GE( secondary_row.w, meta.w * 0.60f );
        EXPECT_LT( primary_row.y, secondary_row.y );
    }
}

UTEST_F( locale_fixture, settings_form_uses_shared_label_and_control_lanes )
{
    const char* locales[] = { "en", "ja-JP", "ar" };
    for ( const char* locale : locales ) {
        vxui__locale_render_settings_desktop( utest_fixture, locale );

        vxui_rect surface = {};
        vxui_rect difficulty_label = {};
        vxui_rect volume_label = {};
        vxui_rect difficulty_control = {};
        vxui_rect volume_control = {};
        ASSERT_TRUE( vxui__locale_find_element_bounds( "settings.surface", &surface ) );
        ASSERT_TRUE( vxui__locale_find_element_bounds( "settings.form.difficulty.row.label_lane", &difficulty_label ) );
        ASSERT_TRUE( vxui__locale_find_element_bounds( "settings.form.volume.row.label_lane", &volume_label ) );
        ASSERT_TRUE( vxui__locale_find_element_bounds( "settings.form.difficulty.row.control", &difficulty_control ) );
        ASSERT_TRUE( vxui__locale_find_element_bounds( "settings.form.volume.row.control", &volume_control ) );

        EXPECT_NEAR( difficulty_label.x, volume_label.x, 1.0f );
        EXPECT_NEAR( difficulty_label.w, volume_label.w, 1.0f );
        EXPECT_NEAR( difficulty_control.x, volume_control.x, 1.0f );
        EXPECT_NEAR( difficulty_control.w, volume_control.w, 1.0f );
        EXPECT_GE( difficulty_control.w, surface.w * 0.45f );
    }
}

UTEST_F( locale_fixture, save_slot_rows_use_surface_width_contract )
{
    const char* locales[] = { "en", "ja-JP", "ar" };
    for ( const char* locale : locales ) {
        vxui__locale_render_settings_desktop( utest_fixture, locale );

        vxui_rect lane = {};
        vxui_rect row = {};
        ASSERT_TRUE( vxui__locale_find_element_bounds( "settings.saves.lane", &lane ) );
        ASSERT_TRUE( vxui__locale_find_element_bounds( "settings.saves.slot.0", &row ) );

        EXPECT_NEAR( row.w, lane.w, 2.0f );
        EXPECT_GE( lane.w, 500.0f );
        EXPECT_FALSE( vxui__locale_find_element_bounds( "settings.saves", nullptr ) );
    }
}

UTEST_F( locale_fixture, settings_body_scroll_region_clips_overflowing_content )
{
    const char* locales[] = { "en", "ja-JP", "ar" };
    for ( const char* locale : locales ) {
        vxui_draw_list list = vxui__locale_render_settings_desktop_impl( utest_fixture, locale, 520, 10, 0.0f, nullptr );

        vxui_rect body_scroll = {};
        vxui_rect body = {};
        vxui_rect footer = {};
        vxui_rect surface = {};
        ASSERT_TRUE( vxui__locale_find_element_bounds( "settings.body_scroll", &body_scroll ) );
        ASSERT_TRUE( vxui__locale_find_element_bounds( "settings.body", &body ) );
        ASSERT_TRUE( vxui__locale_find_element_bounds( "settings.footer", &footer ) );
        ASSERT_TRUE( vxui__locale_find_element_bounds( "settings.surface", &surface ) );

        EXPECT_LT( body_scroll.h, body.h );
        EXPECT_LT( body_scroll.y, footer.y );
        EXPECT_LE( footer.y + footer.h, surface.y + surface.h + 1.0f );
        EXPECT_FALSE( vxui__locale_any_slot_text_below( &list, footer.y ) );
    }
}

UTEST_F( locale_fixture, focused_lower_save_slot_stays_inside_body_viewport )
{
    const char* locales[] = { "en", "ja-JP", "ar" };
    for ( const char* locale : locales ) {
        vxui_draw_list list =
            vxui__locale_render_settings_desktop_impl( utest_fixture, locale, 520, 10, 620.0f, "settings.saves.slot.12" );

        vxui_rect body_scroll = {};
        vxui_rect footer = {};
        vxui_vec2 focused_text = {};
        ASSERT_TRUE( vxui__locale_find_element_bounds( "settings.body_scroll", &body_scroll ) );
        ASSERT_TRUE( vxui__locale_find_element_bounds( "settings.footer", &footer ) );
        ASSERT_TRUE( vxui__locale_find_text_pos( &list, "slot.12", &focused_text ) );

        EXPECT_GE( focused_text.y, body_scroll.y - 1.0f );
        EXPECT_LT( focused_text.y, footer.y );
        EXPECT_FALSE( vxui__locale_any_slot_text_below( &list, footer.y ) );
    }
}

static bool vxui__locale_find_menu_scope_bounds( const char* id, int offset, vxui_rect* out )
{
    Clay_ElementId clay_id = {};
    clay_id.id = vxui_idi( id, ( uint32_t ) offset );
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

static void vxui__locale_render_command_deck_title( locale_fixture* fixture, const char* locale )
{
    fixture->ctx.cfg.screen_width = 1280;
    fixture->ctx.cfg.screen_height = 720;
    vxui_set_font_resolver( &fixture->ctx, vxui__locale_font_resolver, fixture );
    vxui_set_locale( &fixture->ctx, locale );

    const bool rtl = fixture->ctx.rtl;
    const vxui_demo_surface_metrics metrics =
        vxui_demo_compute_surface_metrics( 1280.0f - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f, locale, VXUI_DEMO_SURFACE_TITLE );

    vxui_begin( &fixture->ctx, 0.016f );
    VXUI( &fixture->ctx, "title", {
        .layout = {
            .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_GROW( 0 ) },
            .padding = CLAY_PADDING_ALL( ( uint16_t ) VXUI_DEMO_LAYOUT_OUTER_PADDING ),
            .childAlignment = { .x = CLAY_ALIGN_X_CENTER },
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
    } ) {
        VXUI( &fixture->ctx, "title.surface", {
            .layout = {
                .sizing = { CLAY_SIZING_FIXED( metrics.surface_width ), CLAY_SIZING_GROW( 0 ) },
                .padding = {
                    ( uint16_t ) VXUI_DEMO_LAYOUT_SURFACE_PADDING_X,
                    ( uint16_t ) VXUI_DEMO_LAYOUT_SURFACE_PADDING_X,
                    ( uint16_t ) VXUI_DEMO_LAYOUT_SURFACE_PADDING_Y,
                    ( uint16_t ) VXUI_DEMO_LAYOUT_SURFACE_PADDING_Y,
                },
                .childGap = ( uint16_t ) VXUI_DEMO_LAYOUT_SECTION_GAP,
                .childAlignment = { .x = rtl ? CLAY_ALIGN_X_RIGHT : CLAY_ALIGN_X_LEFT },
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
        } ) {
            VXUI( &fixture->ctx, "title.hero", {
                .layout = {
                    .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                    .padding = CLAY_PADDING_ALL( 24 ),
                    .childGap = 12,
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                },
            } ) {
                VXUI_LABEL( &fixture->ctx, "label.title", ( vxui_label_cfg ) {
                    .font_id = VXUI_TEST_FONT_ROLE_TITLE,
                    .font_size = 44.0f,
                } );
                VXUI_LABEL( &fixture->ctx, "label.body", ( vxui_label_cfg ) { 0 } );
            }
            VXUI( &fixture->ctx, "title.action_band", {
                .layout = {
                    .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                    .padding = CLAY_PADDING_ALL( 18 ),
                    .childGap = 10,
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                },
            } ) {
                VXUI_PROMPT( &fixture->ctx, "action.confirm" );
                VXUI_LABEL( &fixture->ctx, "prompt.confirm.label", ( vxui_label_cfg ) { 0 } );
            }
        }
    }
    ( void ) vxui_end( &fixture->ctx );
}

static void vxui__locale_render_command_deck_main_menu( locale_fixture* fixture, const char* locale )
{
    fixture->ctx.cfg.screen_width = 1280;
    fixture->ctx.cfg.screen_height = 720;
    vxui_set_font_resolver( &fixture->ctx, vxui__locale_font_resolver, fixture );
    vxui_set_locale( &fixture->ctx, locale );

    const bool rtl = fixture->ctx.rtl;
    const vxui_demo_surface_metrics metrics =
        vxui_demo_compute_surface_metrics( 1280.0f - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f, locale, VXUI_DEMO_SURFACE_MAIN_MENU );
    const float menu_width = std::clamp( metrics.content_width * 0.40f, 290.0f, 360.0f );
    const float preview_width = std::max( 260.0f, metrics.content_width - menu_width - VXUI_DEMO_LAYOUT_SECTION_GAP );

    vxui_begin( &fixture->ctx, 0.016f );
    VXUI( &fixture->ctx, "main_menu", {
        .layout = {
            .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_GROW( 0 ) },
            .padding = CLAY_PADDING_ALL( ( uint16_t ) VXUI_DEMO_LAYOUT_OUTER_PADDING ),
            .childAlignment = { .x = CLAY_ALIGN_X_CENTER },
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
    } ) {
        VXUI( &fixture->ctx, "main.surface", {
            .layout = {
                .sizing = { CLAY_SIZING_FIXED( metrics.surface_width ), CLAY_SIZING_GROW( 0 ) },
                .padding = {
                    ( uint16_t ) VXUI_DEMO_LAYOUT_SURFACE_PADDING_X,
                    ( uint16_t ) VXUI_DEMO_LAYOUT_SURFACE_PADDING_X,
                    ( uint16_t ) VXUI_DEMO_LAYOUT_SURFACE_PADDING_Y,
                    ( uint16_t ) VXUI_DEMO_LAYOUT_SURFACE_PADDING_Y,
                },
                .childGap = ( uint16_t ) VXUI_DEMO_LAYOUT_SECTION_GAP,
                .childAlignment = { .x = rtl ? CLAY_ALIGN_X_RIGHT : CLAY_ALIGN_X_LEFT },
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
        } ) {
            VXUI( &fixture->ctx, "main.hero", {
                .layout = {
                    .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                    .padding = CLAY_PADDING_ALL( 20 ),
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                },
            } ) {
                VXUI_LABEL( &fixture->ctx, "label.title", ( vxui_label_cfg ) {
                    .font_id = VXUI_TEST_FONT_ROLE_TITLE,
                    .font_size = 44.0f,
                } );
            }
            VXUI( &fixture->ctx, "main.deck", {
                .layout = {
                    .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_GROW( 180.0f ) },
                    .childGap = ( uint16_t ) VXUI_DEMO_LAYOUT_SECTION_GAP,
                    .layoutDirection = CLAY_LEFT_TO_RIGHT,
                },
            } ) {
                VXUI( &fixture->ctx, "main.command_panel", {
                    .layout = { .sizing = { CLAY_SIZING_FIXED( menu_width ), CLAY_SIZING_GROW( 0 ) } },
                } ) {
                    VXUI_LABEL( &fixture->ctx, "menu.sortie", ( vxui_label_cfg ) { 0 } );
                    VXUI_LABEL( &fixture->ctx, "menu.loadout", ( vxui_label_cfg ) { 0 } );
                    VXUI_LABEL( &fixture->ctx, "menu.settings", ( vxui_label_cfg ) { 0 } );
                }
                VXUI( &fixture->ctx, "main.preview_panel", {
                    .layout = {
                        .sizing = { CLAY_SIZING_FIXED( preview_width ), CLAY_SIZING_GROW( 0 ) },
                        .padding = CLAY_PADDING_ALL( 20 ),
                        .childGap = 8,
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    },
                } ) {
                    VXUI_LABEL( &fixture->ctx, "label.body", ( vxui_label_cfg ) { 0 } );
                    VXUI_LABEL( &fixture->ctx, "label.long", ( vxui_label_cfg ) { 0 } );
                }
            }
            VXUI( &fixture->ctx, "main.footer", {
                .layout = {
                    .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                    .padding = CLAY_PADDING_ALL( 18 ),
                    .childGap = ( uint16_t ) VXUI_DEMO_LAYOUT_ROW_GAP,
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                },
            } ) {
                vxui__locale_emit_status_summary( fixture, "main.footer.status", "label.hello", "label.right", "label.left", 3, rtl );
            }
        }
    }
    ( void ) vxui_end( &fixture->ctx );
}

static void vxui__locale_render_command_deck_settings_menu( locale_fixture* fixture, const char* locale )
{
    static const char* kToggleKeys[] = { "Off", "On" };
    static const char* kPromptKeys[] = { "Keyboard", "Gamepad" };
    static const char* kLocaleKeys[] = { "English", "Japanese", "Arabic" };

    fixture->ctx.cfg.screen_width = 1280;
    fixture->ctx.cfg.screen_height = 720;
    vxui_set_font_resolver( &fixture->ctx, vxui__locale_font_resolver, fixture );
    vxui_set_locale( &fixture->ctx, locale );

    const bool rtl = fixture->ctx.rtl;
    const vxui_demo_surface_metrics metrics =
        vxui_demo_compute_surface_metrics( 1280.0f - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f, locale, VXUI_DEMO_SURFACE_SETTINGS );
    vxui_menu_style style = vxui_menu_style_form();
    style.label_lane_width = metrics.label_lane_width;
    int difficulty = 1;
    float volume = 0.4f;
    int scanlines = 1;
    int prompt_table = 0;
    int locale_index = 0;

    vxui_begin( &fixture->ctx, 0.016f );
    VXUI( &fixture->ctx, "settings", {
        .layout = {
            .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_GROW( 0 ) },
            .padding = CLAY_PADDING_ALL( ( uint16_t ) VXUI_DEMO_LAYOUT_OUTER_PADDING ),
            .childAlignment = { .x = CLAY_ALIGN_X_CENTER },
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
    } ) {
        VXUI( &fixture->ctx, "settings.surface", {
            .layout = {
                .sizing = { CLAY_SIZING_FIXED( metrics.surface_width ), CLAY_SIZING_GROW( 0 ) },
                .padding = {
                    ( uint16_t ) VXUI_DEMO_LAYOUT_SURFACE_PADDING_X,
                    ( uint16_t ) VXUI_DEMO_LAYOUT_SURFACE_PADDING_X,
                    ( uint16_t ) VXUI_DEMO_LAYOUT_SURFACE_PADDING_Y,
                    ( uint16_t ) VXUI_DEMO_LAYOUT_SURFACE_PADDING_Y,
                },
                .childGap = ( uint16_t ) VXUI_DEMO_LAYOUT_SECTION_GAP,
                .childAlignment = { .x = rtl ? CLAY_ALIGN_X_RIGHT : CLAY_ALIGN_X_LEFT },
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
        } ) {
            VXUI( &fixture->ctx, "settings.header", {
                .layout = {
                    .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                },
            } ) {
                VXUI_LABEL( &fixture->ctx, "menu.settings", ( vxui_label_cfg ) {
                    .font_id = VXUI_TEST_FONT_ROLE_TITLE,
                    .font_size = 44.0f,
                } );
            }
            VXUI( &fixture->ctx, "settings.body_panel", {
                .layout = {
                    .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_GROW( 180.0f ) },
                },
            } ) {
                vxui_menu_state menu_state = {};
                vxui_menu_begin( &fixture->ctx, &menu_state, "settings.body_menu", ( vxui_menu_cfg ) {
                    .style = &style,
                    .viewport_height = 260.0f,
                } );
                vxui_menu_section( &fixture->ctx, &menu_state, "interface", "menu.interface", ( vxui_menu_section_cfg ) { 0 } );
                vxui_menu_option( &fixture->ctx, &menu_state, "difficulty", "menu.challenge", &difficulty, kLocaleKeys, 3, ( vxui_menu_row_cfg ) { 0 }, ( vxui_option_cfg ) { 0 } );
                vxui_menu_section( &fixture->ctx, &menu_state, "audio", "menu.audio", ( vxui_menu_section_cfg ) { 0 } );
                vxui_menu_slider( &fixture->ctx, &menu_state, "volume", "menu.volume", &volume, 0.0f, 1.0f, ( vxui_menu_row_cfg ) { 0 }, ( vxui_slider_cfg ) {
                    .show_value = true,
                    .format = "%.2f",
                } );
                vxui_menu_section( &fixture->ctx, &menu_state, "visuals", "menu.visual_fx", ( vxui_menu_section_cfg ) { 0 } );
                vxui_menu_option( &fixture->ctx, &menu_state, "scanlines", "menu.scanlines", &scanlines, kToggleKeys, 2, ( vxui_menu_row_cfg ) { 0 }, ( vxui_option_cfg ) { 0 } );
                vxui_menu_option( &fixture->ctx, &menu_state, "prompt_table", "menu.prompts", &prompt_table, kPromptKeys, 2, ( vxui_menu_row_cfg ) { 0 }, ( vxui_option_cfg ) { 0 } );
                vxui_menu_option( &fixture->ctx, &menu_state, "locale", "menu.locale", &locale_index, kLocaleKeys, 3, ( vxui_menu_row_cfg ) { 0 }, ( vxui_option_cfg ) { 0 } );
                vxui_menu_section( &fixture->ctx, &menu_state, "overflow_a", "Overflow A", ( vxui_menu_section_cfg ) { 0 } );
                vxui_menu_option( &fixture->ctx, &menu_state, "overflow_0", "Option 0", &scanlines, kToggleKeys, 2, ( vxui_menu_row_cfg ) { 0 }, ( vxui_option_cfg ) { 0 } );
                vxui_menu_option( &fixture->ctx, &menu_state, "overflow_1", "Option 1", &scanlines, kToggleKeys, 2, ( vxui_menu_row_cfg ) { 0 }, ( vxui_option_cfg ) { 0 } );
                vxui_menu_option( &fixture->ctx, &menu_state, "overflow_2", "Option 2", &scanlines, kToggleKeys, 2, ( vxui_menu_row_cfg ) { 0 }, ( vxui_option_cfg ) { 0 } );
                vxui_menu_section( &fixture->ctx, &menu_state, "overflow_b", "Overflow B", ( vxui_menu_section_cfg ) { 0 } );
                vxui_menu_option( &fixture->ctx, &menu_state, "overflow_3", "Option 3", &scanlines, kToggleKeys, 2, ( vxui_menu_row_cfg ) { 0 }, ( vxui_option_cfg ) { 0 } );
                vxui_menu_option( &fixture->ctx, &menu_state, "overflow_4", "Option 4", &scanlines, kToggleKeys, 2, ( vxui_menu_row_cfg ) { 0 }, ( vxui_option_cfg ) { 0 } );
                vxui_menu_end( &fixture->ctx, &menu_state );
            }
            VXUI( &fixture->ctx, "settings.footer", {
                .layout = {
                    .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                    .padding = CLAY_PADDING_ALL( 18 ),
                    .childGap = ( uint16_t ) VXUI_DEMO_LAYOUT_ROW_GAP,
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                },
            } ) {
                VXUI_LABEL( &fixture->ctx, "menu.return_command_deck", ( vxui_label_cfg ) { 0 } );
            }
        }
    }
    ( void ) vxui_end( &fixture->ctx );
}

static void vxui__locale_render_command_deck_sortie( locale_fixture* fixture, const char* locale )
{
    fixture->ctx.cfg.screen_width = 1280;
    fixture->ctx.cfg.screen_height = 720;
    vxui_set_font_resolver( &fixture->ctx, vxui__locale_font_resolver, fixture );
    vxui_set_locale( &fixture->ctx, locale );

    const bool rtl = fixture->ctx.rtl;
    const vxui_demo_surface_metrics metrics =
        vxui_demo_compute_surface_metrics( 1280.0f - VXUI_DEMO_LAYOUT_OUTER_PADDING * 2.0f, locale, VXUI_DEMO_SURFACE_SORTIE );
    const float left_width = std::clamp( metrics.content_width * 0.34f, 300.0f, 360.0f );
    const float right_width = std::clamp( metrics.content_width * 0.24f, 220.0f, 280.0f );
    const float center_width = std::max( 260.0f, metrics.content_width - left_width - right_width - VXUI_DEMO_LAYOUT_SECTION_GAP * 2.0f );

    vxui_begin( &fixture->ctx, 0.016f );
    VXUI( &fixture->ctx, "sortie", {
        .layout = {
            .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_GROW( 0 ) },
            .padding = CLAY_PADDING_ALL( ( uint16_t ) VXUI_DEMO_LAYOUT_OUTER_PADDING ),
            .childAlignment = { .x = CLAY_ALIGN_X_CENTER },
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
    } ) {
        VXUI( &fixture->ctx, "sortie.surface", {
            .layout = {
                .sizing = { CLAY_SIZING_FIXED( metrics.surface_width ), CLAY_SIZING_GROW( 0 ) },
                .padding = {
                    ( uint16_t ) VXUI_DEMO_LAYOUT_SURFACE_PADDING_X,
                    ( uint16_t ) VXUI_DEMO_LAYOUT_SURFACE_PADDING_X,
                    ( uint16_t ) VXUI_DEMO_LAYOUT_SURFACE_PADDING_Y,
                    ( uint16_t ) VXUI_DEMO_LAYOUT_SURFACE_PADDING_Y,
                },
                .childGap = ( uint16_t ) VXUI_DEMO_LAYOUT_SECTION_GAP,
                .childAlignment = { .x = rtl ? CLAY_ALIGN_X_RIGHT : CLAY_ALIGN_X_LEFT },
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
        } ) {
            VXUI_LABEL( &fixture->ctx, "menu.sortie", ( vxui_label_cfg ) {
                .font_id = VXUI_TEST_FONT_ROLE_TITLE,
                .font_size = 44.0f,
            } );
            VXUI( &fixture->ctx, "sortie.deck", {
                .layout = {
                    .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_GROW( 200.0f ) },
                    .childGap = ( uint16_t ) VXUI_DEMO_LAYOUT_SECTION_GAP,
                    .layoutDirection = CLAY_LEFT_TO_RIGHT,
                },
            } ) {
                VXUI( &fixture->ctx, "sortie.menu_panel", {
                    .layout = { .sizing = { CLAY_SIZING_FIXED( left_width ), CLAY_SIZING_GROW( 0 ) } },
                } ) {
                    VXUI_LABEL( &fixture->ctx, "sortie.section.operations", ( vxui_label_cfg ) { 0 } );
                }
                VXUI( &fixture->ctx, "sortie.briefing", {
                    .layout = {
                        .sizing = { CLAY_SIZING_FIXED( center_width ), CLAY_SIZING_GROW( 0 ) },
                        .padding = CLAY_PADDING_ALL( 20 ),
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    },
                } ) {
                    VXUI_LABEL( &fixture->ctx, "label.long", ( vxui_label_cfg ) { 0 } );
                }
                VXUI( &fixture->ctx, "sortie.detail", {
                    .layout = {
                        .sizing = { CLAY_SIZING_FIXED( right_width ), CLAY_SIZING_GROW( 0 ) },
                        .padding = CLAY_PADDING_ALL( 18 ),
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    },
                } ) {
                    VXUI_LABEL( &fixture->ctx, "label.body", ( vxui_label_cfg ) { 0 } );
                }
            }
            VXUI( &fixture->ctx, "sortie.footer", {
                .layout = {
                    .sizing = { CLAY_SIZING_GROW( 0 ), CLAY_SIZING_FIT( 0 ) },
                    .padding = CLAY_PADDING_ALL( 18 ),
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                },
            } ) {
                VXUI_LABEL( &fixture->ctx, "menu.start_sortie", ( vxui_label_cfg ) { 0 } );
            }
        }
    }
    ( void ) vxui_end( &fixture->ctx );
}

UTEST_F( locale_fixture, command_deck_sequence_contract_declares_representative_enter_sequences )
{
    vxui_seq_step step = { 0, vxui_id( "dummy.surface" ), VXUI_PROP_OPACITY, 1.0f };
    EXPECT_STREQ( vxui_demo_enter_sequence_name_for_screen( "main_menu" ), VXUI_DEMO_DEFAULT_WATCHED_SEQUENCE_NAME );
    EXPECT_EQ( VXUI_DEMO_FRONTEND_ENTER_SEQUENCE_COUNT, 11 );

    for ( int i = 0; i < VXUI_DEMO_FRONTEND_ENTER_SEQUENCE_COUNT; ++i ) {
        vxui_register_seq( &utest_fixture->ctx, VXUI_DEMO_FRONTEND_ENTER_SEQUENCE_NAMES[ i ], &step, 1 );
        ASSERT_TRUE( vxui_find_seq( &utest_fixture->ctx, VXUI_DEMO_FRONTEND_ENTER_SEQUENCE_NAMES[ i ] ) != nullptr );
    }
}

UTEST_F( locale_fixture, command_deck_title_surface_uses_desktop_hero_spacing )
{
    const char* locales[] = { "en", "ja-JP", "ar" };
    const float desktop_min_surface_width = vxui_demo_surface_desktop_min_width( VXUI_DEMO_SURFACE_TITLE );
    for ( const char* locale : locales ) {
        vxui__locale_render_command_deck_title( utest_fixture, locale );

        vxui_rect surface = {};
        vxui_rect hero = {};
        vxui_rect action_band = {};
        ASSERT_TRUE( vxui__locale_find_element_bounds( "title.surface", &surface ) );
        ASSERT_TRUE( vxui__locale_find_element_bounds( "title.hero", &hero ) );
        ASSERT_TRUE( vxui__locale_find_element_bounds( "title.action_band", &action_band ) );

        EXPECT_GE( surface.w, desktop_min_surface_width );
        EXPECT_LT( hero.y + hero.h, action_band.y );
        EXPECT_LE( action_band.y + action_band.h, surface.y + surface.h + 1.0f );
    }
}

UTEST_F( locale_fixture, command_deck_main_menu_uses_split_desktop_surface )
{
    const char* locales[] = { "en", "ja-JP", "ar" };
    const float desktop_min_surface_width = vxui_demo_surface_desktop_min_width( VXUI_DEMO_SURFACE_MAIN_MENU );
    for ( const char* locale : locales ) {
        vxui__locale_render_command_deck_main_menu( utest_fixture, locale );

        vxui_rect surface = {};
        vxui_rect hero = {};
        vxui_rect deck = {};
        vxui_rect command_panel = {};
        vxui_rect preview_panel = {};
        vxui_rect footer = {};
        ASSERT_TRUE( vxui__locale_find_element_bounds( "main.surface", &surface ) );
        ASSERT_TRUE( vxui__locale_find_element_bounds( "main.hero", &hero ) );
        ASSERT_TRUE( vxui__locale_find_element_bounds( "main.deck", &deck ) );
        ASSERT_TRUE( vxui__locale_find_element_bounds( "main.command_panel", &command_panel ) );
        ASSERT_TRUE( vxui__locale_find_element_bounds( "main.preview_panel", &preview_panel ) );
        ASSERT_TRUE( vxui__locale_find_element_bounds( "main.footer", &footer ) );

        EXPECT_GE( surface.w, desktop_min_surface_width );
        EXPECT_GE( command_panel.w, 280.0f );
        EXPECT_GE( preview_panel.w, command_panel.w * 0.80f );
        EXPECT_LT( hero.y + hero.h, deck.y );
        EXPECT_LT( deck.y + deck.h, footer.y );
    }
}

UTEST_F( locale_fixture, command_deck_settings_menu_keeps_single_scroll_owner_and_pinned_footer )
{
    const char* locales[] = { "en", "ja-JP", "ar" };
    const float desktop_min_surface_width = vxui_demo_surface_desktop_min_width( VXUI_DEMO_SURFACE_SETTINGS );
    for ( const char* locale : locales ) {
        vxui__locale_render_command_deck_settings_menu( utest_fixture, locale );

        vxui_rect surface = {};
        vxui_rect body_panel = {};
        vxui_rect footer = {};
        vxui_rect menu_viewport = {};
        vxui_rect menu_content = {};
        ASSERT_TRUE( vxui__locale_find_element_bounds( "settings.surface", &surface ) );
        ASSERT_TRUE( vxui__locale_find_element_bounds( "settings.body_panel", &body_panel ) );
        ASSERT_TRUE( vxui__locale_find_element_bounds( "settings.footer", &footer ) );
        ASSERT_TRUE( vxui__locale_find_menu_scope_bounds( "settings.body_menu", 1, &menu_viewport ) );
        ASSERT_TRUE( vxui__locale_find_menu_scope_bounds( "settings.body_menu", 2, &menu_content ) );

        EXPECT_GE( surface.w, desktop_min_surface_width );
        EXPECT_GE( menu_viewport.w, surface.w * VXUI_DEMO_SETTINGS_MENU_VIEWPORT_MIN_WIDTH_FRACTION );
        EXPECT_GT( menu_content.h, menu_viewport.h );
        EXPECT_GE( menu_viewport.y, body_panel.y - 1.0f );
        EXPECT_LE( menu_viewport.y + menu_viewport.h, body_panel.y + body_panel.h + 1.0f );
        EXPECT_LT( body_panel.y + body_panel.h, footer.y );
        EXPECT_LE( footer.y + footer.h, surface.y + surface.h + 1.0f );
    }
}

UTEST_F( locale_fixture, command_deck_sortie_surface_uses_three_lane_split )
{
    const char* locales[] = { "en", "ja-JP", "ar" };
    const float desktop_min_surface_width = vxui_demo_surface_desktop_min_width( VXUI_DEMO_SURFACE_SORTIE );
    for ( const char* locale : locales ) {
        vxui__locale_render_command_deck_sortie( utest_fixture, locale );

        vxui_rect surface = {};
        vxui_rect deck = {};
        vxui_rect menu_panel = {};
        vxui_rect briefing = {};
        vxui_rect detail = {};
        vxui_rect footer = {};
        ASSERT_TRUE( vxui__locale_find_element_bounds( "sortie.surface", &surface ) );
        ASSERT_TRUE( vxui__locale_find_element_bounds( "sortie.deck", &deck ) );
        ASSERT_TRUE( vxui__locale_find_element_bounds( "sortie.menu_panel", &menu_panel ) );
        ASSERT_TRUE( vxui__locale_find_element_bounds( "sortie.briefing", &briefing ) );
        ASSERT_TRUE( vxui__locale_find_element_bounds( "sortie.detail", &detail ) );
        ASSERT_TRUE( vxui__locale_find_element_bounds( "sortie.footer", &footer ) );

        EXPECT_GE( surface.w, desktop_min_surface_width );
        EXPECT_GE( menu_panel.w, 300.0f );
        EXPECT_GE( briefing.w, 320.0f );
        EXPECT_GE( detail.w, 220.0f );
        EXPECT_GT( briefing.w, detail.w );
        EXPECT_LT( deck.y + deck.h, footer.y );
    }
}
