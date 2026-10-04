#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
    #define NOMINMAX
#endif
#ifdef _WIN32
    #include <windows.h>
#endif

#include "utest.h"
#include "../vxui.h"
#define VXUI_IMPL
#include "../vxui_impl.h"
#define VXUI_HUD_IMPL
#include "../vxui_hud.h"
#ifdef _MSC_VER
#include <crtdbg.h>
#endif

static uint8_t s_clay_mem[16 * 1024 * 1024];
static const char* s_executable = nullptr;

static Clay_Dimensions measure_text( Clay_StringSlice s, Clay_TextElementConfig* cfg, void* )
{
    return { (float) s.length * 8.0f, (float) cfg->fontSize };
}

static vxui_render_data sentinel_data()
{
    vxui_render_data out  = {};
    out.material_id       = 91;
    out.texture_id        = 92;
    out.flags             = 93;
    out.outline_thickness = 3.5f;
    out.colour            = { 0.1f, 0.2f, 0.3f, 0.4f };
    out.outline_colour    = { 0.5f, 0.6f, 0.7f, 0.8f };
    out.uv                = { 0.25f, 0.5f, 0.75f, 1 };
    for ( int i = 0; i < 8; i++ ) out.params[i] = (float) i + 0.5f;
    return out;
}

static bool rejected_without_writes( const vxui_hud* hud, const void* attachment )
{
    vxui_draw_cmd c = {};
    c.userdata = attachment;
    vxui_render_data out = sentinel_data();
    vxui_render_data before = out;
    return !vxui_hud_resolve( hud, &c, &out ) && memcmp( &out, &before, sizeof( out ) ) == 0;
}

UTEST(hud_lookup, every_active_slot_resolves_its_own_snapshot) {
    vxui_hud hud = {};
    for ( int i = 0; i < VXUI_HUD_MAX_ITEMS; i++ )
    {
        hud.colour = { (float) i, 0.2f, 0.3f, 0.4f };
        vxui_hud_record( &hud, (uint32_t) i + 42, { 0.1f, 0.2f, (float) i, 0.9f } );
    }
    for ( int i = VXUI_HUD_MAX_ITEMS - 1; i >= 0; i-- )
    {
        vxui_draw_cmd c = {};
        c.userdata = &hud.items[i];
        vxui_render_data out = {};
        ASSERT_TRUE( vxui_hud_resolve( &hud, &c, &out ) );
        ASSERT_EQ( out.texture_id, (uint32_t) i + 42 );
        ASSERT_EQ( out.colour.r, (float) i );
        ASSERT_EQ( out.uv.z, (float) i );
    }
}

UTEST(hud_lookup, null_attachment_leaves_output_unchanged) {
    vxui_hud hud = {};
    vxui_hud_record( &hud, 42, { 0, 0, 1, 1 } );
    ASSERT_TRUE( rejected_without_writes( &hud, nullptr ) );
}

UTEST(hud_lookup, empty_table_does_not_claim_its_first_storage_slot) {
    vxui_hud hud = {};
    ASSERT_TRUE( rejected_without_writes( &hud, &hud.items[0] ) );
    ASSERT_TRUE( rejected_without_writes( &hud, &hud.items[VXUI_HUD_MAX_ITEMS - 1] ) );
}

UTEST(hud_lookup, every_inactive_slot_is_rejected) {
    vxui_hud hud = {};
    vxui_hud_record( &hud, 42, { 0, 0, 1, 1 } );
    vxui_hud_record( &hud, 43, { 0, 0, 1, 1 } );
    for ( int i = hud.item_count; i < VXUI_HUD_MAX_ITEMS; i++ )
        ASSERT_TRUE( rejected_without_writes( &hud, &hud.items[i] ) );
}

UTEST(hud_lookup, one_past_active_and_one_past_storage_are_rejected) {
    vxui_hud hud = {};
    vxui_hud_record( &hud, 42, { 0, 0, 1, 1 } );
    ASSERT_TRUE( rejected_without_writes( &hud, &hud.items[hud.item_count] ) );
    ASSERT_TRUE( rejected_without_writes( &hud, hud.items + VXUI_HUD_MAX_ITEMS ) );
}

UTEST(hud_lookup, addresses_just_outside_the_array_are_not_dereferenced) {
    vxui_hud hud = {};
    for ( int i = 0; i < VXUI_HUD_MAX_ITEMS; i++ ) vxui_hud_record( &hud, 42, { 0, 0, 1, 1 } );
    uintptr_t begin = (uintptr_t) hud.items;
    uintptr_t end = (uintptr_t) ( hud.items + VXUI_HUD_MAX_ITEMS );
    ASSERT_TRUE( rejected_without_writes( &hud, (const void*) ( begin - 1 ) ) );
    ASSERT_TRUE( rejected_without_writes( &hud, (const void*) end ) );
    ASSERT_TRUE( rejected_without_writes( &hud, (const void*) ( end + 1 ) ) );
    ASSERT_TRUE( rejected_without_writes( &hud, (const void*) (uintptr_t) 1 ) );
    ASSERT_TRUE( rejected_without_writes( &hud, (const void*) (uintptr_t) UINTPTR_MAX ) );
}

UTEST(hud_lookup, every_interior_byte_is_rejected_even_when_aligned_for_a_field) {
    vxui_hud hud = {};
    for ( int i = 0; i < VXUI_HUD_MAX_ITEMS; i++ ) vxui_hud_record( &hud, 42, { 0, 0, 1, 1 } );
    for ( int i = 0; i < VXUI_HUD_MAX_ITEMS; i++ )
    {
        const uint8_t* bytes = (const uint8_t*) &hud.items[i];
        for ( int j = 1; j < (int) sizeof( vxui_hud_item ); j++ )
            ASSERT_TRUE( rejected_without_writes( &hud, bytes + j ) );
    }
}

UTEST(hud_lookup, another_hud_and_a_copied_table_cannot_claim_the_attachment) {
    vxui_hud hud = {};
    vxui_hud_record( &hud, 42, { 0, 0, 1, 1 } );
    vxui_hud copy = hud;
    int unrelated = 42;
    ASSERT_TRUE( rejected_without_writes( &copy, &hud.items[0] ) );
    ASSERT_TRUE( rejected_without_writes( &hud, &copy.items[0] ) );
    ASSERT_TRUE( rejected_without_writes( &hud, &unrelated ) );
    ASSERT_EQ( unrelated, 42 );
}

UTEST(hud_lookup, attachment_membership_does_not_depend_on_draw_id_or_type) {
    vxui_hud hud = {};
    vxui_hud_record( &hud, 42, { 0, 0, 1, 1 } );
    uint32_t ids[] = { 0, 1, UINT32_MAX };
    uint8_t types[] = { VXUI_DRAW_RECT, VXUI_DRAW_TEXT, 99 };
    for ( int i = 0; i < 3; i++ )
    {
        vxui_draw_cmd c = {};
        c.id       = ids[i];
        c.type     = types[i];
        c.userdata = &hud.items[0];
        vxui_render_data out = {};
        ASSERT_TRUE( vxui_hud_resolve( &hud, &c, &out ) );
        ASSERT_EQ( out.texture_id, (uint32_t) 42 );
    }
}

UTEST(hud_lookup, successful_lookup_writes_only_texture_uv_and_colour) {
    vxui_hud hud = {};
    hud.colour = { 0.9f, 0.8f, 0.7f, 0.6f };
    vxui_hud_record( &hud, 42, { 0, 0.1f, 0.9f, 1 } );
    vxui_draw_cmd c = {};
    c.userdata = &hud.items[0];
    vxui_render_data out = sentinel_data();
    vxui_render_data expected = out;
    expected.texture_id = hud.items[0].texture_id;
    expected.uv         = hud.items[0].uv;
    expected.colour     = hud.items[0].colour;

    ASSERT_TRUE( vxui_hud_resolve( &hud, &c, &out ) );
    ASSERT_EQ( memcmp( &out, &expected, sizeof( out ) ), 0 );
}

UTEST(hud_lookup, multiline_packets_resolve_after_an_empty_logical_item) {
    vxui_ctx ctx = {};
    vxui_init( &ctx, 960, 540, s_clay_mem, sizeof( s_clay_mem ) );
    Clay_SetMeasureTextFunction( measure_text, nullptr );
    vxui_frame( &ctx, 1.0f / 60.0f );
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_text( &hud, "empty", "" );
    vxui_hud_set_colour( &hud, { 0.2f, 0.4f, 0.6f, 0.8f } );
    vxui_hud_text( &hud, "text", "A\nB\nC" );
    vxui_draw_list dl = vxui_render( &ctx );

    ASSERT_EQ( hud.item_count, 2 );
    ASSERT_EQ( dl.count, 3 );
    for ( int i = 0; i < 3; i++ )
    {
        ASSERT_EQ( dl.cmds[i].userdata, (const void*) &hud.items[1] );
        vxui_render_data out = {};
        ASSERT_TRUE( vxui_hud_resolve( &hud, &dl.cmds[i], &out ) );
        ASSERT_EQ( out.colour.r, 0.2f );
        ASSERT_EQ( out.colour.a, 0.8f );
    }
}

UTEST(hud_lookup, begin_resets_membership_and_reused_slots_hold_new_snapshots) {
    vxui_ctx ctx = {};
    vxui_hud hud = {};
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    vxui_hud_item* old = vxui_hud_record( &hud, 42, { 0, 0, 1, 1 } );
    vxui_hud_begin( &hud, &ctx, 960, 540 );
    ASSERT_TRUE( rejected_without_writes( &hud, old ) );
    hud.colour = { 0.2f, 0.4f, 0.6f, 0.8f };
    vxui_hud_item* current = vxui_hud_record( &hud, 43, { 0.1f, 0.2f, 0.3f, 0.4f } );
    ASSERT_EQ( current, old );
    vxui_draw_cmd c = {};
    c.userdata = current;
    vxui_render_data out = {};
    ASSERT_TRUE( vxui_hud_resolve( &hud, &c, &out ) );
    ASSERT_EQ( out.texture_id, (uint32_t) 43 );
    ASSERT_EQ( out.uv.z, 0.3f );
    ASSERT_EQ( out.colour.a, 0.8f );
}

#ifndef NDEBUG
static bool count_asserts( const char* mode )
{
    char command[4096];
#ifdef _MSC_VER
    int len = snprintf( command, sizeof( command ), "\"\"%s\" --bad-count %s 2>&1\"", s_executable, mode );
#else
    int len = snprintf( command, sizeof( command ), "ulimit -c 0; \"%s\" --bad-count %s 2>&1", s_executable, mode );
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
        if ( strstr( line, "hud->item_count >= 0" ) ) seen = true;
#ifdef _MSC_VER
    int status = _pclose( pipe );
#else
    int status = pclose( pipe );
#endif
    return status != 0 && seen;
}

UTEST(hud_lookup, invalid_active_span_asserts_before_attachment_lookup) {
    ASSERT_TRUE( count_asserts( "negative" ) );
    ASSERT_TRUE( count_asserts( "overflow" ) );
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
    if ( argc == 3 && strcmp( argv[1], "--bad-count" ) == 0 )
    {
        vxui_hud hud = {};
        hud.item_count = strcmp( argv[2], "negative" ) == 0 ? -1 : VXUI_HUD_MAX_ITEMS + 1;
        vxui_draw_cmd c = {};
        c.userdata = &hud.items[0];
        vxui_render_data out = {};
        vxui_hud_resolve( &hud, &c, &out );
        return 0;
    }
    return utest_main( argc, argv );
}
