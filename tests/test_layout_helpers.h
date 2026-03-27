#pragma once

#include <cstdint>
#include <cstring>
#include "../vxui.h"

struct vxui_layout_helpers
{
    static bool find_text_pos( const vxui_draw_list* list, const char* text, vxui_vec2* out );
    static bool find_text_x( const vxui_draw_list* list, const char* text, float* out_x );
    static bool find_rect_by_bounds( const vxui_draw_list* list, float min_x, float min_y, float max_x, float max_y, vxui_rect* out );
    static bool find_border_near( const vxui_draw_list* list, float x, float y, vxui_rect* out );
    static bool find_anim_bounds( const vxui_ctx* ctx, uint32_t id, vxui_rect* out );
    static vxui_rect compute_union_rect( const vxui_draw_list* list, int start_idx, int count );
    static vxui_rect compute_text_bounds_union( const vxui_draw_list* list );

    static bool rect_contains( vxui_rect outer, vxui_rect inner );
    static bool rects_overlap( vxui_rect a, vxui_rect b );
    static float rect_left( vxui_rect r );
    static float rect_right( vxui_rect r );
    static float rect_top( vxui_rect r );
    static float rect_bottom( vxui_rect r );
    static float rect_center_x( vxui_rect r );
    static float rect_center_y( vxui_rect r );

    static bool is_approx_equal( float a, float b, float tolerance );
    static bool is_approx_aligned_x( float a, float b, float tolerance );
    static bool is_within_range( float value, float min_val, float max_val );

    static float gap_between( vxui_rect a, vxui_rect b );
    static float horizontal_gap_between( vxui_rect left, vxui_rect right );
    static float vertical_gap_between( vxui_rect top, vxui_rect bottom );

    static bool width_in_range( vxui_rect r, float min_w, float max_w );
    static bool height_in_range( vxui_rect r, float min_h, float max_h );

    static int count_cmds_by_type( const vxui_draw_list* list, vxui_cmd_type type );
    static bool has_cmd_type_in_range( const vxui_draw_list* list, vxui_cmd_type type, int min_count, int max_count );
};

inline bool vxui_layout_helpers::find_text_pos( const vxui_draw_list* list, const char* text, vxui_vec2* out )
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

inline bool vxui_layout_helpers::find_text_x( const vxui_draw_list* list, const char* text, float* out_x )
{
    vxui_vec2 pos = {};
    if ( find_text_pos( list, text, &pos ) ) {
        if ( out_x ) {
            *out_x = pos.x;
        }
        return true;
    }
    return false;
}

inline bool vxui_layout_helpers::find_rect_by_bounds(
    const vxui_draw_list* list,
    float min_x, float min_y,
    float max_x, float max_y,
    vxui_rect* out )
{
    if ( !list ) {
        return false;
    }
    for ( int i = 0; i < list->length; ++i ) {
        vxui_cmd* cmd = &list->commands[ i ];
        vxui_rect bounds = {};
        bool has_bounds = false;
        if ( cmd->type == VXUI_CMD_RECT ) {
            bounds = cmd->rect.bounds;
            has_bounds = true;
        } else if ( cmd->type == VXUI_CMD_RECT_ROUNDED ) {
            bounds = cmd->rect_rounded.bounds;
            has_bounds = true;
        }
        if ( has_bounds ) {
            if ( bounds.x >= min_x && bounds.y >= min_y &&
                 bounds.x + bounds.w <= max_x && bounds.y + bounds.h <= max_y ) {
                if ( out ) {
                    *out = bounds;
                }
                return true;
            }
        }
    }
    return false;
}

inline bool vxui_layout_helpers::find_border_near( const vxui_draw_list* list, float x, float y, vxui_rect* out )
{
    if ( !list ) {
        return false;
    }
    float best_dist = 1e9f;
    vxui_rect best_rect = {};
    for ( int i = 0; i < list->length; ++i ) {
        vxui_cmd* cmd = &list->commands[ i ];
        if ( cmd->type != VXUI_CMD_BORDER ) {
            continue;
        }
        vxui_rect b = cmd->border.bounds;
        float cx = b.x + b.w * 0.5f;
        float cy = b.y + b.h * 0.5f;
        float dist = std::abs( cx - x ) + std::abs( cy - y );
        if ( dist < best_dist ) {
            best_dist = dist;
            best_rect = b;
        }
    }
    if ( best_dist < 50.0f ) {
        if ( out ) {
            *out = best_rect;
        }
        return true;
    }
    return false;
}

inline bool vxui_layout_helpers::find_anim_bounds( const vxui_ctx* ctx, uint32_t id, vxui_rect* out )
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

inline vxui_rect vxui_layout_helpers::compute_union_rect( const vxui_draw_list* list, int start_idx, int count )
{
    vxui_rect union_rect = { 0, 0, 0, 0 };
    if ( !list || count <= 0 ) {
        return union_rect;
    }
    int end = start_idx + count;
    if ( end > list->length ) {
        end = list->length;
    }
    bool first = true;
    for ( int i = start_idx; i < end; ++i ) {
        vxui_cmd* cmd = &list->commands[ i ];
        vxui_rect bounds = {};
        bool has_bounds = false;
        if ( cmd->type == VXUI_CMD_RECT ) {
            bounds = cmd->rect.bounds;
            has_bounds = true;
        } else if ( cmd->type == VXUI_CMD_RECT_ROUNDED ) {
            bounds = cmd->rect_rounded.bounds;
            has_bounds = true;
        } else if ( cmd->type == VXUI_CMD_BORDER ) {
            bounds = cmd->border.bounds;
            has_bounds = true;
        } else if ( cmd->type == VXUI_CMD_TEXT ) {
            bounds.x = cmd->text.pos.x;
            bounds.y = cmd->text.pos.y;
            bounds.w = cmd->text.size;
            bounds.h = cmd->text.size;
            has_bounds = true;
        }
        if ( has_bounds ) {
            if ( first ) {
                union_rect = bounds;
                first = false;
            } else {
                float min_x = rect_left( union_rect ) < rect_left( bounds ) ? rect_left( union_rect ) : rect_left( bounds );
                float min_y = rect_top( union_rect ) < rect_top( bounds ) ? rect_top( union_rect ) : rect_top( bounds );
                float max_x = rect_right( union_rect ) > rect_right( bounds ) ? rect_right( union_rect ) : rect_right( bounds );
                float max_y = rect_bottom( union_rect ) > rect_bottom( bounds ) ? rect_bottom( union_rect ) : rect_bottom( bounds );
                union_rect.x = min_x;
                union_rect.y = min_y;
                union_rect.w = max_x - min_x;
                union_rect.h = max_y - min_y;
            }
        }
    }
    return union_rect;
}

inline vxui_rect vxui_layout_helpers::compute_text_bounds_union( const vxui_draw_list* list )
{
    vxui_rect union_rect = { 0, 0, 0, 0 };
    if ( !list ) {
        return union_rect;
    }
    bool first = true;
    for ( int i = 0; i < list->length; ++i ) {
        vxui_cmd* cmd = &list->commands[ i ];
        if ( cmd->type != VXUI_CMD_TEXT ) {
            continue;
        }
        vxui_rect bounds = {};
        bounds.x = cmd->text.pos.x;
        bounds.y = cmd->text.pos.y;
        bounds.w = cmd->text.size;
        bounds.h = cmd->text.size;
        if ( first ) {
            union_rect = bounds;
            first = false;
        } else {
            float min_x = rect_left( union_rect ) < rect_left( bounds ) ? rect_left( union_rect ) : rect_left( bounds );
            float min_y = rect_top( union_rect ) < rect_top( bounds ) ? rect_top( union_rect ) : rect_top( bounds );
            float max_x = rect_right( union_rect ) > rect_right( bounds ) ? rect_right( union_rect ) : rect_right( bounds );
            float max_y = rect_bottom( union_rect ) > rect_bottom( bounds ) ? rect_bottom( union_rect ) : rect_bottom( bounds );
            union_rect.x = min_x;
            union_rect.y = min_y;
            union_rect.w = max_x - min_x;
            union_rect.h = max_y - min_y;
        }
    }
    return union_rect;
}

inline bool vxui_layout_helpers::rect_contains( vxui_rect outer, vxui_rect inner )
{
    return rect_left( outer ) <= rect_left( inner ) &&
           rect_right( outer ) >= rect_right( inner ) &&
           rect_top( outer ) <= rect_top( inner ) &&
           rect_bottom( outer ) >= rect_bottom( inner );
}

inline bool vxui_layout_helpers::rects_overlap( vxui_rect a, vxui_rect b )
{
    return rect_left( a ) < rect_right( b ) &&
           rect_right( a ) > rect_left( b ) &&
           rect_top( a ) < rect_bottom( b ) &&
           rect_bottom( a ) > rect_top( b );
}

inline float vxui_layout_helpers::rect_left( vxui_rect r )
{
    return r.x;
}

inline float vxui_layout_helpers::rect_right( vxui_rect r )
{
    return r.x + r.w;
}

inline float vxui_layout_helpers::rect_top( vxui_rect r )
{
    return r.y;
}

inline float vxui_layout_helpers::rect_bottom( vxui_rect r )
{
    return r.y + r.h;
}

inline float vxui_layout_helpers::rect_center_x( vxui_rect r )
{
    return r.x + r.w * 0.5f;
}

inline float vxui_layout_helpers::rect_center_y( vxui_rect r )
{
    return r.y + r.h * 0.5f;
}

inline bool vxui_layout_helpers::is_approx_equal( float a, float b, float tolerance )
{
    return std::abs( a - b ) <= tolerance;
}

inline bool vxui_layout_helpers::is_approx_aligned_x( float a, float b, float tolerance )
{
    return is_approx_equal( a, b, tolerance );
}

inline bool vxui_layout_helpers::is_within_range( float value, float min_val, float max_val )
{
    return value >= min_val && value <= max_val;
}

inline float vxui_layout_helpers::gap_between( vxui_rect a, vxui_rect b )
{
    if ( rects_overlap( a, b ) ) {
        return 0.0f;
    }
    float horiz_gap = 0.0f;
    if ( rect_right( a ) < rect_left( b ) ) {
        horiz_gap = rect_left( b ) - rect_right( a );
    } else if ( rect_right( b ) < rect_left( a ) ) {
        horiz_gap = rect_left( a ) - rect_right( b );
    }
    float vert_gap = 0.0f;
    if ( rect_bottom( a ) < rect_top( b ) ) {
        vert_gap = rect_top( b ) - rect_bottom( a );
    } else if ( rect_bottom( b ) < rect_top( a ) ) {
        vert_gap = rect_top( a ) - rect_bottom( b );
    }
    return horiz_gap + vert_gap;
}

inline float vxui_layout_helpers::horizontal_gap_between( vxui_rect left, vxui_rect right )
{
    if ( rect_right( left ) < rect_left( right ) ) {
        return rect_left( right ) - rect_right( left );
    }
    if ( rect_right( right ) < rect_left( left ) ) {
        return rect_left( left ) - rect_right( right );
    }
    return 0.0f;
}

inline float vxui_layout_helpers::vertical_gap_between( vxui_rect top, vxui_rect bottom )
{
    if ( rect_bottom( top ) < rect_top( bottom ) ) {
        return rect_top( bottom ) - rect_bottom( top );
    }
    if ( rect_bottom( bottom ) < rect_top( top ) ) {
        return rect_top( top ) - rect_bottom( bottom );
    }
    return 0.0f;
}

inline bool vxui_layout_helpers::width_in_range( vxui_rect r, float min_w, float max_w )
{
    return is_within_range( r.w, min_w, max_w );
}

inline bool vxui_layout_helpers::height_in_range( vxui_rect r, float min_h, float max_h )
{
    return is_within_range( r.h, min_h, max_h );
}

inline int vxui_layout_helpers::count_cmds_by_type( const vxui_draw_list* list, vxui_cmd_type type )
{
    if ( !list ) {
        return 0;
    }
    int count = 0;
    for ( int i = 0; i < list->length; ++i ) {
        if ( list->commands[ i ].type == type ) {
            ++count;
        }
    }
    return count;
}

inline bool vxui_layout_helpers::has_cmd_type_in_range( const vxui_draw_list* list, vxui_cmd_type type, int min_count, int max_count )
{
    int count = count_cmds_by_type( list, type );
    return count >= min_count && count <= max_count;
}
