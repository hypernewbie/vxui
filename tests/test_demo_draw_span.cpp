#include "demo_gl_record.h"
#ifdef _MSC_VER
#include <crtdbg.h>
#endif

static const char* s_executable = nullptr;

UTEST(demo_draw_span, every_supported_geometry_pass_accepts_the_entire_index_span) {
    uint32_t passes[] = { VE_FONTCACHE_FRAMEBUFFER_PASS_GLYPH, VE_FONTCACHE_FRAMEBUFFER_PASS_GLYPH_RESOLVE, VE_FONTCACHE_FRAMEBUFFER_PASS_ATLAS, VE_FONTCACHE_FRAMEBUFFER_PASS_TARGET, VE_FONTCACHE_FRAMEBUFFER_PASS_TARGET_UNCACHED, DEMO_GL_PASS_RECT, DEMO_GL_PASS_RECT_TEXTURED, DEMO_GL_PASS_IMAGE, DEMO_GL_PASS_CRT, DEMO_GL_PASS_ROUND, DEMO_GL_PASS_OUTLINE };
    for ( uint32_t pass : passes )
    {
        vxui_gl_state gl;
        init_gl( &gl );
        ve_fontcache cache = {};
        push_draw( &cache, pass );
        vxui_gl_execute( &gl, &cache, 640, 480 );
        ASSERT_EQ( s_draw_count, 1 );
        ASSERT_EQ( s_draws[0].index_count, 3 );
        ASSERT_EQ( s_draws[0].index_offset, (uintptr_t) 0 );
    }
}

UTEST(demo_draw_span, nonzero_start_is_a_byte_offset_not_an_index_count) {
    vxui_gl_state gl;
    init_gl( &gl );
    ve_fontcache cache = {};
    push_draw( &cache, DEMO_GL_PASS_RECT );
    cache.drawlist.indices = { 0, 1, 2, 0, 1, 2 };
    cache.drawlist.dcalls[0].start_index = 3;
    cache.drawlist.dcalls[0].end_index = 6;
    vxui_gl_execute( &gl, &cache, 640, 480 );
    ASSERT_EQ( s_draw_count, 1 );
    ASSERT_EQ( s_draws[0].index_count, 3 );
    ASSERT_EQ( s_draws[0].index_offset, (uintptr_t) ( 3 * sizeof( uint32_t ) ) );
}

UTEST(demo_draw_span, adjacent_compatible_spans_still_merge_into_one_valid_range) {
    vxui_gl_state gl;
    init_gl( &gl );
    ve_fontcache cache = {};
    push_draw( &cache, DEMO_GL_PASS_RECT );
    push_draw( &cache, DEMO_GL_PASS_RECT );
    cache.drawlist.indices = { 0, 1, 2, 0, 1, 2 };
    cache.drawlist.dcalls[1].start_index = 3;
    cache.drawlist.dcalls[1].end_index = 6;
    vxui_gl_execute( &gl, &cache, 640, 480 );
    ASSERT_EQ( s_draw_count, 1 );
    ASSERT_EQ( s_draws[0].index_count, 6 );
}

UTEST(demo_draw_span, a_zero_length_clear_at_the_end_of_the_buffer_does_not_draw) {
    vxui_gl_state gl;
    init_gl( &gl );
    ve_fontcache cache = {};
    push_draw( &cache, DEMO_GL_PASS_RECT );
    cache.drawlist.dcalls[0].start_index = 3;
    cache.drawlist.dcalls[0].clear_before_draw = true;
    vxui_gl_execute( &gl, &cache, 640, 480 );
    ASSERT_EQ( s_draw_count, 0 );
    ASSERT_EQ( s_clear_count, 1 );
}

UTEST(demo_draw_span, scissor_markers_at_the_buffer_end_preserve_the_following_draw) {
    vxui_gl_state gl;
    init_gl( &gl );
    ve_fontcache cache = {};
    push_draw( &cache, DEMO_GL_PASS_RECT );
    vxui_gl_push_scissor_set( &cache.drawlist, &gl, { 0, 0, 0.5f, 1 } );
    push_draw( &cache, DEMO_GL_PASS_RECT );
    vxui_gl_push_scissor_reset( &cache.drawlist );
    vxui_gl_execute( &gl, &cache, 640, 480 );
    ASSERT_EQ( s_draw_count, 2 );
    ASSERT_EQ( s_draws[0].index_count, 3 );
    ASSERT_EQ( s_draws[1].index_count, 3 );
    ASSERT_EQ( s_draws[1].scissor.z, 320 );
}

UTEST(demo_draw_span, an_empty_stream_flushes_without_a_draw_or_clear) {
    vxui_gl_state gl;
    init_gl( &gl );
    ve_fontcache cache = {};
    vxui_gl_execute( &gl, &cache, 640, 480 );
    ASSERT_EQ( s_draw_count, 0 );
    ASSERT_EQ( s_clear_count, 0 );
    ASSERT_TRUE( cache.drawlist.indices.empty() );
}

UTEST(demo_draw_span, full_packet_batch_keeps_each_colour_separated_span_within_the_buffer) {
    vxui_gl_state gl;
    init_gl( &gl );
    ve_fontcache cache = {};
    for ( int i = 0; i < VXUI_MAX_DRAW_CMDS; i++ )
    {
        glm::vec4 colour = { (float) i / VXUI_MAX_DRAW_CMDS, 0, 0, 1 };
        vxui_gl_push_quad( &cache.drawlist, DEMO_GL_PASS_RECT, 0, 0, 1, 1, {}, 0, colour );
    }
    vxui_gl_execute( &gl, &cache, 640, 480 );
    ASSERT_EQ( s_draw_count, VXUI_MAX_DRAW_CMDS );
    for ( int i = 0; i < s_draw_count; i++ )
    {
        ASSERT_EQ( s_draws[i].index_count, 6 );
        ASSERT_EQ( s_draws[i].index_offset, (uintptr_t) ( i * 6 * sizeof( uint32_t ) ) );
    }
}

UTEST(demo_draw_span, a_flushed_cache_can_reuse_zero_based_ranges_on_the_next_frame) {
    ve_fontcache cache = {};
    for ( int frame = 0; frame < 3; frame++ )
    {
        vxui_gl_state gl;
        init_gl( &gl );
        push_draw( &cache, DEMO_GL_PASS_RECT );
        vxui_gl_execute( &gl, &cache, 640, 480 );
        ASSERT_EQ( s_draws[0].index_offset, (uintptr_t) 0 );
        ASSERT_EQ( s_draws[0].index_count, 3 );
        ASSERT_TRUE( cache.drawlist.dcalls.empty() );
        ASSERT_TRUE( cache.drawlist.indices.empty() );
        ASSERT_TRUE( cache.drawlist.vertices.empty() );
    }
}

#ifndef NDEBUG
static bool span_asserts( const char* mode )
{
    char command[4096];
#ifdef _MSC_VER
    snprintf( command, sizeof( command ), "\"\"%s\" --bad-span %s 2>&1\"", s_executable, mode );
    FILE* pipe = _popen( command, "r" );
#else
    snprintf( command, sizeof( command ), "ulimit -c 0; \"%s\" --bad-span %s 2>&1", s_executable, mode );
    FILE* pipe = popen( command, "r" );
#endif
    if ( !pipe ) return false;
    bool seen = false;
    char line[512];
    while ( fgets( line, sizeof( line ), pipe ) ) if ( strstr( line, "invalid draw index span" ) ) seen = true;
#ifdef _MSC_VER
    int status = _pclose( pipe );
#else
    int status = pclose( pipe );
#endif
    return status != 0 && seen;
}

UTEST(demo_draw_span, reversed_spans_assert_instead_of_wrapping_the_draw_count) {
    ASSERT_TRUE( span_asserts( "reversed" ) );
}

UTEST(demo_draw_span, a_span_past_the_index_buffer_asserts_before_drawing) {
    ASSERT_TRUE( span_asserts( "past" ) );
}

UTEST(demo_draw_span, an_empty_span_past_the_index_buffer_is_still_invalid) {
    ASSERT_TRUE( span_asserts( "empty-past" ) );
}
#endif

UTEST_STATE();
int main( int argc, const char* const argv[] )
{
#ifdef _WIN32
    SetErrorMode( SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX );
#endif
#ifdef _MSC_VER
    _set_error_mode( _OUT_TO_STDERR );
    _set_abort_behavior( 0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT );
    _CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_FILE );
    _CrtSetReportFile( _CRT_ASSERT, _CRTDBG_FILE_STDERR );
#endif
    s_executable = argv[0];
    if ( argc == 3 && strcmp( argv[1], "--bad-span" ) == 0 )
    {
        vxui_gl_state gl;
        init_gl( &gl );
        ve_fontcache cache = {};
        push_draw( &cache, DEMO_GL_PASS_RECT );
        ve_fontcache_draw& d = cache.drawlist.dcalls[0];
        if ( strcmp( argv[2], "reversed" ) == 0 ) { d.start_index = 3; d.end_index = 0; }
        else if ( strcmp( argv[2], "past" ) == 0 ) d.end_index = 4;
        else { d.start_index = 4; d.end_index = 4; }
        vxui_gl_execute( &gl, &cache, 640, 480 );
        return 0;
    }
    return utest_main( argc, argv );
}
