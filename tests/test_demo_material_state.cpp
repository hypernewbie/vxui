#include "demo_gl_record.h"
#ifdef _MSC_VER
#include <crtdbg.h>
#endif

static const char* s_executable = nullptr;

static void set_material( vxui_gl_state* gl, int slot )
{
    for ( int i = 0; i < 8; i++ ) gl->material_data[slot].params[i] = (float) ( slot * 8 + i ) + 0.25f;
    gl->material_data[slot].flags = (uint32_t) slot + 42;
}

UTEST(demo_material_state, every_material_uses_its_shader_colour_and_declared_payload_slot) {
    uint32_t passes[] = { DEMO_GL_PASS_CRT, DEMO_GL_PASS_ROUND, DEMO_GL_PASS_OUTLINE };
    for ( int i = 0; i < 3; i++ )
    {
        vxui_gl_state gl;
        init_gl( &gl );
        gl.material_data_count = 2;
        set_material( &gl, 0 );
        set_material( &gl, 1 );
        ve_fontcache cache = {};
        push_draw( &cache, passes[i], 1 );
        float colour[] = { 0.1f, 0.2f, 0.3f, 0.4f };
        memcpy( cache.drawlist.dcalls[0].colour, colour, sizeof( colour ) );
        vxui_gl_execute( &gl, &cache, 640, 480 );

        ASSERT_EQ( s_draw_count, 1 );
        ASSERT_EQ( s_draws[0].program, (GLuint) ( 7 + i ) );
        ASSERT_EQ( s_draws[0].params_uploads, 1 );
        ASSERT_EQ( s_draws[0].colour_uploads, 1 );
        ASSERT_EQ( s_draws[0].flags_uploads, i == 0 ? 1 : 0 );
        for ( int j = 0; j < 4; j++ )
        {
            ASSERT_EQ( s_draws[0].params[j], gl.material_data[1].params[j] );
            ASSERT_EQ( s_draws[0].colour[j], colour[j] );
        }
        if ( i == 0 ) ASSERT_EQ( s_draws[0].flags, (GLuint) 43 );
    }
}

UTEST(demo_material_state, mixed_materials_preserve_producer_order_and_per_slot_uploads) {
    vxui_gl_state gl;
    init_gl( &gl );
    gl.material_data_count = 3;
    for ( int i = 0; i < 3; i++ ) set_material( &gl, i );
    uint32_t passes[] = { DEMO_GL_PASS_ROUND, DEMO_GL_PASS_CRT, DEMO_GL_PASS_OUTLINE, DEMO_GL_PASS_CRT };
    uint32_t slots[] = { 2, 0, 1, 2 };
    GLuint programs[] = { 8, 7, 9, 7 };
    int flags_uploads[] = { 0, 1, 1, 2 };
    ve_fontcache cache = {};
    for ( int i = 0; i < 4; i++ ) push_draw( &cache, passes[i], slots[i] );
    vxui_gl_execute( &gl, &cache, 640, 480 );

    ASSERT_EQ( s_draw_count, 4 );
    for ( int i = 0; i < 4; i++ )
    {
        ASSERT_EQ( s_draws[i].program, programs[i] );
        ASSERT_EQ( s_draws[i].params_uploads, i + 1 );
        ASSERT_EQ( s_draws[i].flags_uploads, flags_uploads[i] );
        for ( int j = 0; j < 4; j++ ) ASSERT_EQ( s_draws[i].params[j], gl.material_data[slots[i]].params[j] );
    }
    ASSERT_TRUE( cache.drawlist.dcalls.empty() );
}

UTEST(demo_material_state, all_materials_accept_the_first_and_last_payload_slots) {
    vxui_gl_state gl;
    init_gl( &gl );
    gl.material_data_count = DEMO_GL_MAX_MATERIAL_DCALLS;
    set_material( &gl, 0 );
    set_material( &gl, DEMO_GL_MAX_MATERIAL_DCALLS - 1 );
    ve_fontcache cache = {};
    uint32_t passes[] = { DEMO_GL_PASS_CRT, DEMO_GL_PASS_ROUND, DEMO_GL_PASS_OUTLINE };
    for ( int i = 0; i < 3; i++ )
    {
        push_draw( &cache, passes[i], 0 );
        push_draw( &cache, passes[i], DEMO_GL_MAX_MATERIAL_DCALLS - 1 );
    }
    vxui_gl_execute( &gl, &cache, 640, 480 );

    ASSERT_EQ( s_draw_count, 6 );
    for ( int i = 0; i < 6; i++ )
    {
        int slot = i % 2 == 0 ? 0 : DEMO_GL_MAX_MATERIAL_DCALLS - 1;
        for ( int j = 0; j < 4; j++ ) ASSERT_EQ( s_draws[i].params[j], gl.material_data[slot].params[j] );
    }
}

UTEST(demo_material_state, crt_keeps_signed_fractional_parameters_and_all_flag_bits) {
    vxui_gl_state gl;
    init_gl( &gl );
    float params[] = { -0.5f, 0.25f, 30.75f, 1000.0f };
    memcpy( gl.material_data[0].params, params, sizeof( params ) );
    gl.material_data[0].flags = UINT32_MAX;
    ve_fontcache cache = {};
    push_draw( &cache, DEMO_GL_PASS_CRT );
    vxui_gl_execute( &gl, &cache, 640, 480 );

    ASSERT_EQ( s_draws[0].flags, (GLuint) UINT32_MAX );
    for ( int i = 0; i < 4; i++ ) ASSERT_EQ( s_draws[0].params[i], params[i] );
}

UTEST(demo_material_state, other_target_passes_never_read_the_material_pool) {
    vxui_gl_state gl;
    init_gl( &gl );
    gl.material_data_count = 0;
    ve_fontcache cache = {};
    uint32_t passes[] = { DEMO_GL_PASS_RECT, DEMO_GL_PASS_IMAGE, DEMO_GL_PASS_RECT_TEXTURED, VE_FONTCACHE_FRAMEBUFFER_PASS_TARGET, VE_FONTCACHE_FRAMEBUFFER_PASS_TARGET_UNCACHED };
    for ( int i = 0; i < 5; i++ ) push_draw( &cache, passes[i], 77 );
    vxui_gl_execute( &gl, &cache, 640, 480 );

    ASSERT_EQ( s_draw_count, 5 );
    for ( int i = 0; i < 5; i++ )
    {
        ASSERT_EQ( s_draws[i].params_uploads, 0 );
        ASSERT_EQ( s_draws[i].flags_uploads, 0 );
    }
    ASSERT_EQ( s_draws[1].texture, (GLuint) 77 );
    ASSERT_EQ( s_draws[2].texture, (GLuint) 77 );
}

UTEST(demo_material_state, every_material_restores_window_state_and_ui_clip_after_glyphs) {
    vxui_gl_state gl;
    init_gl( &gl );
    ve_fontcache cache = {};
    vxui_gl_push_scissor_set( &cache.drawlist, &gl, { 0.25f, 0.25f, 0.5f, 0.5f } );
    push_draw( &cache, VE_FONTCACHE_FRAMEBUFFER_PASS_GLYPH );
    push_draw( &cache, VE_FONTCACHE_FRAMEBUFFER_PASS_GLYPH_RESOLVE );
    uint32_t passes[] = { DEMO_GL_PASS_CRT, DEMO_GL_PASS_ROUND, DEMO_GL_PASS_OUTLINE };
    for ( int i = 0; i < 3; i++ ) push_draw( &cache, passes[i] );
    vxui_gl_execute( &gl, &cache, 640, 480 );

    ASSERT_EQ( s_draw_count, 5 );
    for ( int i = 2; i < 5; i++ )
    {
        ASSERT_EQ( s_draws[i].framebuffer, (GLuint) 0 );
        ASSERT_TRUE( s_draws[i].blend );
        ASSERT_TRUE( s_draws[i].srgb );
        ASSERT_FALSE( s_draws[i].stencil );
        ASSERT_TRUE( s_draws[i].colour_mask.a );
        ASSERT_EQ( s_draws[i].viewport.z, 640 );
        ASSERT_EQ( s_draws[i].scissor.x, 160 );
        ASSERT_EQ( s_draws[i].scissor.y, 120 );
        ASSERT_EQ( s_draws[i].scissor.z, 320 );
        ASSERT_EQ( s_draws[i].scissor.w, 240 );
    }
}

UTEST(demo_material_state, full_fill_and_outline_batch_uses_every_material_slot_once) {
    vxui_gl_state gl;
    init_gl( &gl );
    gl.material_data_count = DEMO_GL_MAX_MATERIAL_DCALLS;
    ve_fontcache cache = {};
    for ( int i = 0; i < DEMO_GL_MAX_MATERIAL_DCALLS; i++ )
    {
        set_material( &gl, i );
        push_draw( &cache, i % 2 == 0 ? DEMO_GL_PASS_CRT : DEMO_GL_PASS_OUTLINE, (uint32_t) i );
    }
    vxui_gl_execute( &gl, &cache, 640, 480 );

    ASSERT_EQ( s_draw_count, DEMO_GL_MAX_MATERIAL_DCALLS );
    for ( int i = 0; i < s_draw_count; i++ )
    {
        ASSERT_EQ( s_draws[i].program, i % 2 == 0 ? gl.shader_crt : gl.shader_outline );
        ASSERT_EQ( s_draws[i].params_uploads, i + 1 );
        ASSERT_EQ( s_draws[i].flags_uploads, i / 2 + 1 );
        for ( int j = 0; j < 4; j++ ) ASSERT_EQ( s_draws[i].params[j], gl.material_data[i].params[j] );
    }
}

UTEST(demo_material_state, clear_only_material_command_keeps_its_uniforms_and_window_target) {
    vxui_gl_state gl;
    init_gl( &gl );
    ve_fontcache cache = {};
    push_draw( &cache, DEMO_GL_PASS_CRT );
    cache.drawlist.dcalls[0].end_index = 0;
    cache.drawlist.dcalls[0].clear_before_draw = true;
    vxui_gl_execute( &gl, &cache, 640, 480 );

    ASSERT_EQ( s_draw_count, 0 );
    ASSERT_EQ( s_clear_count, 1 );
    ASSERT_EQ( s_clears[0].program, gl.shader_crt );
    ASSERT_EQ( s_clears[0].framebuffer, (GLuint) 0 );
    ASSERT_EQ( s_clear_bits[0], (GLbitfield) GL_COLOR_BUFFER_BIT );
    ASSERT_EQ( s_clears[0].flags, (GLuint) 42 );
    for ( int i = 0; i < 4; i++ ) ASSERT_EQ( s_clears[0].params[i], (float) i + 1.0f );
}

#ifndef NDEBUG
static bool slot_asserts( const char* pass, const char* slot )
{
    char command[4096];
#ifdef _MSC_VER
    int len = snprintf( command, sizeof( command ), "\"\"%s\" --bad-slot %s %s 2>&1\"", s_executable, pass, slot );
#else
    int len = snprintf( command, sizeof( command ), "ulimit -c 0; \"%s\" --bad-slot %s %s 2>&1", s_executable, pass, slot );
#endif
    assert( len > 0 && len < (int) sizeof( command ) );
#ifdef _MSC_VER
    FILE* pipe = _popen( command, "r" );
#else
    FILE* pipe = popen( command, "r" );
#endif
    if ( !pipe ) return false;
    bool seen = false;
    char line[512];
    while ( fgets( line, sizeof( line ), pipe ) )
        if ( strstr( line, "mi >= 0 && mi < gl->material_data_count" ) ) seen = true;
#ifdef _MSC_VER
    int status = _pclose( pipe );
#else
    int status = pclose( pipe );
#endif
    return status != 0 && seen;
}

UTEST(demo_material_state, crt_rejects_negative_and_inactive_payload_slots) {
    ASSERT_TRUE( slot_asserts( "crt", "negative" ) );
    ASSERT_TRUE( slot_asserts( "crt", "inactive" ) );
}

UTEST(demo_material_state, round_rejects_negative_and_inactive_payload_slots) {
    ASSERT_TRUE( slot_asserts( "round", "negative" ) );
    ASSERT_TRUE( slot_asserts( "round", "inactive" ) );
}

UTEST(demo_material_state, outline_rejects_negative_and_inactive_payload_slots) {
    ASSERT_TRUE( slot_asserts( "outline", "negative" ) );
    ASSERT_TRUE( slot_asserts( "outline", "inactive" ) );
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
    if ( argc == 4 && strcmp( argv[1], "--bad-slot" ) == 0 )
    {
        vxui_gl_state gl;
        init_gl( &gl );
        ve_fontcache cache = {};
        uint32_t pass = strcmp( argv[2], "crt" ) == 0 ? DEMO_GL_PASS_CRT : strcmp( argv[2], "round" ) == 0 ? DEMO_GL_PASS_ROUND : DEMO_GL_PASS_OUTLINE;
        uint32_t slot = strcmp( argv[3], "negative" ) == 0 ? UINT32_MAX : (uint32_t) gl.material_data_count;
        push_draw( &cache, pass, slot );
        vxui_gl_execute( &gl, &cache, 640, 480 );
        return 0;
    }
    return utest_main( argc, argv );
}
