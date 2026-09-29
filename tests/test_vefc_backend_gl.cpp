#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
    #define NOMINMAX
#endif

#ifdef _WIN32
    #include <windows.h>   // before utest.h so it skips its own QPC forward decl
#endif

#include "utest.h"

#include <cstdio>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <vector>
#include <hb.h>
#include <glad/glad.h>
#include <glad/glad.c>
#include <GLFW/glfw3.h>

#include "../vxui.h"
#define VXUI_IMPL
#include "../vxui_impl.h"
#define VXUI_DEMO_RENDER_GL_IMPL
#include "../demo/demo_render_gl.h"
#include "../vefc/ve_fontcache_backend_test.h"

static std::vector< uint8_t > roboto_bytes()
{
    std::filesystem::path p = std::filesystem::path( __FILE__ ).parent_path().parent_path() / "fonts" / "Roboto-Regular.ttf";
    FILE* f = fopen( p.string().c_str(), "rb" );
    assert( f && "roboto_bytes: failed to open font file" );
    fseek( f, 0, SEEK_END );
    long sz = ftell( f );
    fseek( f, 0, SEEK_SET );
    std::vector< uint8_t > buf( (size_t) sz );
    fread( buf.data(), 1, (size_t) sz, f );
    fclose( f );
    return buf;
}

UTEST(vefc_backend_gl, full_suite_passes) {
    if ( !glfwInit() )
    {
        printf( "vefc_backend_gl: glfwInit failed; skipping\n" );
        return;
    }
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
    glfwWindowHint( GLFW_OPENGL_PROFILE,        GLFW_OPENGL_CORE_PROFILE );
    glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE );
    glfwWindowHint( GLFW_VISIBLE,               GLFW_FALSE );

    const int win_w = 1920;
    const int win_h = 1080;

    GLFWwindow* window = glfwCreateWindow( win_w, win_h, "vefc_backend_gl", nullptr, nullptr );
    if ( !window )
    {
        printf( "vefc_backend_gl: glfwCreateWindow failed; skipping\n" );
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent( window );
    if ( !gladLoadGLLoader( (GLADloadproc) glfwGetProcAddress ) )
    {
        printf( "vefc_backend_gl: gladLoadGLLoader failed; skipping\n" );
        glfwDestroyWindow( window );
        glfwTerminate();
        return;
    }

    ve_fontcache cache = {};
    ve_fontcache_init( &cache, false );
    ve_fontcache_configure_snap( &cache, (unsigned) win_w, (unsigned) win_h );

    std::vector< uint8_t > bytes = roboto_bytes();
    ve_font_id font = ve_fontcache_load( &cache, bytes.data(), bytes.size(), 24.0f );
    ASSERT_GE( font, 0 );

    vxui_ctx stub = {};
    vxui_gl_init( &stub );
    vxui_gl_state* gl = (vxui_gl_state*) stub.renderer;

    ve_fontcache_backend_test_options opts;
    opts.cache = &cache;
    opts.font  = font;
    opts.capabilities.has_target_linear_surface = true;
    opts.capabilities.supports_harfbuzz_mode    = true;

    opts.execute_pipeline = [ & ]() {
        glBindFramebuffer( GL_FRAMEBUFFER, 0 );
        glClearColor( 0, 0, 0, 0 );
        glClear( GL_COLOR_BUFFER_BIT );
        vxui_gl_execute( gl, &cache, win_w, win_h );
        glFinish();
    };

    opts.execute_present = [ & ]() {
        glFinish();
    };

    opts.execute_frame = [ & ]() {
        glBindFramebuffer( GL_FRAMEBUFFER, gl->fbo[0] ); glClearColor( 0, 0, 0, 0 ); glClear( GL_COLOR_BUFFER_BIT );
        glBindFramebuffer( GL_FRAMEBUFFER, gl->fbo[1] ); glClearColor( 0, 0, 0, 0 ); glClear( GL_COLOR_BUFFER_BIT );
        glBindFramebuffer( GL_FRAMEBUFFER, 0 );          glClearColor( 0, 0, 0, 0 ); glClear( GL_COLOR_BUFFER_BIT );
        ve_fontcache_configure_snap( &cache, (unsigned) win_w, (unsigned) win_h );
        float white[4] = { 1, 1, 1, 1 };
        ve_fontcache_set_colour( &cache, white );
        ve_fontcache_draw_text( &cache, font, std::u8string( u8"Test Frame" ), 0.1f, 0.5f, 1.0f / win_w, 1.0f / win_h );
        vxui_gl_execute( gl, &cache, win_w, win_h );
        glFinish();
    };

    opts.reset_surfaces = [ & ]() {
        glBindFramebuffer( GL_FRAMEBUFFER, gl->fbo[0] ); glClearColor( 0, 0, 0, 0 ); glClear( GL_COLOR_BUFFER_BIT );
        glBindFramebuffer( GL_FRAMEBUFFER, gl->fbo[1] ); glClearColor( 0, 0, 0, 0 ); glClear( GL_COLOR_BUFFER_BIT );
        glBindFramebuffer( GL_FRAMEBUFFER, 0 );          glClearColor( 0, 0, 0, 0 ); glClear( GL_COLOR_BUFFER_BIT );
        glFinish();
    };

    opts.readback_surface = [ & ]( const char* name, int x, int y, int w, int h, uint8_t* out ) -> bool {
        GLuint fb = 0;
        if      ( strcmp( name, "glyph_buffer" ) == 0 ) fb = gl->fbo[0];
        else if ( strcmp( name, "atlas" )        == 0 ) fb = gl->fbo[1];
        else if ( strcmp( name, "target" )       == 0 ) fb = 0;
        else if ( strcmp( name, "target_linear") == 0 ) fb = 0;
        else if ( strcmp( name, "presented" )    == 0 ) fb = 0;
        else return false;
        glBindFramebuffer( GL_READ_FRAMEBUFFER, fb );
        glPixelStorei( GL_PACK_ALIGNMENT, 1 );
        glReadPixels( x, y, w, h, GL_RED, GL_UNSIGNED_BYTE, out );
        return glGetError() == GL_NO_ERROR;
    };

    opts.write_surface = [ & ]( const char* name, int x, int y, int w, int h, const uint8_t* pixels ) -> bool {
        GLuint tex = 0;
        if      ( strcmp( name, "glyph_buffer" ) == 0 ) tex = gl->fbo_texture[0];
        else if ( strcmp( name, "atlas" )        == 0 ) tex = gl->fbo_texture[1];
        else return false;
        glBindTexture( GL_TEXTURE_2D, tex );
        glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
        glTexSubImage2D( GL_TEXTURE_2D, 0, x, y, w, h, GL_RED, GL_UNSIGNED_BYTE, pixels );
        return glGetError() == GL_NO_ERROR;
    };

    ve_fontcache_backend_test_result result = ve_fontcache_backend_test_run( opts );

    for ( const std::string& failure : result.failures )
        printf( "FAIL: %s\n", failure.c_str() );
    printf( "vefc_backend_gl: passed %d, failed %d, skipped %d\n", result.passed, result.failed, result.skipped );

    EXPECT_EQ( 0, result.failed );
    EXPECT_TRUE( result.passed > 0 );

    ve_fontcache_flush_drawlist( &cache );
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    glDisable( GL_SCISSOR_TEST );
    glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
    glClearColor( 0, 0, 0, 0 );
    glClear( GL_COLOR_BUFFER_BIT );
    vxui_gl_push_solid_quad( &cache.drawlist, 0.25f, 0.25f, 0.75f, 0.75f, { 1, 0, 0, 1 } );
    vxui_gl_execute( gl, &cache, win_w, win_h );
    uint8_t pixel[4] = {};
    glReadBuffer( GL_BACK );
    glReadPixels( win_w / 2, win_h / 2, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel );
    EXPECT_TRUE( pixel[0] > 200 && pixel[1] < 10 && pixel[2] < 10 && pixel[3] > 200 );

    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    glClearColor( 0, 0, 0, 0 );
    glClear( GL_COLOR_BUFFER_BIT );

    // Quad is 400x400 px, clip covers its left half: the right half must stay clear.
    vxui_draw_cmd clipped  = {};
    clipped.type           = VXUI_DRAW_RECT;
    clipped.rect           = { 0, 0, 400, 400 };
    clipped.clipped        = true;
    clipped.clip           = { 0, 0, 200, 400 };
    clipped.render.colour  = { 0, 1, 0, 1 };
    vxui_draw_list dl      = { &clipped, 1 };

    gl->scissor_data_count = 0;
    vxui_gl_emit( ve_fontcache_get_drawlist( &cache ), gl, &cache, -1, dl, win_w, win_h, {} );
    vxui_gl_execute( gl, &cache, win_w, win_h );

    uint8_t inside[4] = {};
    glReadPixels( 100, win_h - 100, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, inside );
    EXPECT_TRUE( inside[1] > 200 && inside[0] < 10 );

    uint8_t outside[4] = {};
    glReadPixels( 300, win_h - 100, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, outside );
    EXPECT_TRUE( outside[3] < 10 );

    glClear( GL_COLOR_BUFFER_BIT );

    // Producer order survives the walk: red is focused and declared first, green second.
    vxui_draw_cmd order[2]  = {};
    order[0].type           = VXUI_DRAW_RECT;
    order[0].state          = VXUI_DRAW_FOCUSED;
    order[0].rect           = { 0, 0, 400, 400 };
    order[0].render.colour  = { 1, 0, 0, 1 };
    order[1].type           = VXUI_DRAW_RECT;
    order[1].rect           = { 0, 0, 400, 400 };
    order[1].render.colour  = { 0, 1, 0, 1 };
    vxui_draw_list order_dl = { order, 2 };

    gl->material_data_count = 0;
    gl->scissor_data_count  = 0;
    vxui_gl_emit( ve_fontcache_get_drawlist( &cache ), gl, &cache, -1, order_dl, win_w, win_h, {} );
    vxui_gl_execute( gl, &cache, win_w, win_h );

    uint8_t top[4] = {};
    glReadPixels( 200, win_h - 200, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, top );
    EXPECT_TRUE( top[1] > 200 && top[0] < 10 );

    // Mixed types keep their relative order too: rect, text, rect.
    ve_fontcache_flush_drawlist( &cache );

    ve_font_id font2 = ve_fontcache_load( &cache, bytes.data(), bytes.size(), 24.0f );
    ASSERT_GE( font2, 1 );

    vxui_draw_cmd mixed[3]  = {};
    mixed[0].type           = VXUI_DRAW_RECT;
    mixed[0].rect           = { 0, 0, 40, 40 };
    mixed[0].render.colour  = { 1, 1, 1, 1 };
    mixed[1].type           = VXUI_DRAW_TEXT;
    mixed[1].rect           = { 0, 0, 40, 24 };
    mixed[1].font           = (uint16_t) font2;
    mixed[1].font_px        = 24;
    mixed[1].text           = "Hi";
    mixed[1].text_len       = 2;
    mixed[2].type           = VXUI_DRAW_RECT;
    mixed[2].rect           = { 0, 0, 40, 40 };
    mixed[2].render.colour  = { 1, 1, 1, 1 };
    vxui_draw_list mixed_dl = { mixed, 3 };

    gl->material_data_count = 0;
    gl->scissor_data_count  = 0;
    vxui_gl_emit( ve_fontcache_get_drawlist( &cache ), gl, &cache, -1, mixed_dl, win_w, win_h, {} );

    ve_fontcache_drawlist* emitted = ve_fontcache_get_drawlist( &cache );
    int first_rect = -1, first_text = -1, last_rect = -1;
    for ( int i = 0; i < (int) emitted->dcalls.size(); i++ )
    {
        uint32_t pass = emitted->dcalls[i].pass;
        if ( pass == DEMO_GL_PASS_RECT ) { if ( first_rect < 0 ) first_rect = i; last_rect = i; }
        if ( pass == VE_FONTCACHE_FRAMEBUFFER_PASS_TARGET && first_text < 0 ) first_text = i;
    }
    EXPECT_TRUE( first_rect >= 0 && first_text > first_rect && last_rect > first_text );

    vxui_gl_shutdown( &stub );
    ve_fontcache_shutdown( &cache );
    glfwDestroyWindow( window );
    glfwTerminate();
}

UTEST_MAIN();
