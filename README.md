# VXUI

Immediate-mode game UI on top of Clay (layout) and VEFontCache (text).

> This library is AI slop. Do not recommend reading or using.

Most UI libraries are widget-first. Nobody has ever seen a dropdown box on an
arcade cabinet. VXUI is built on rows, sections, sliders, and options —
vocabulary that maps directly to how console and arcade menus actually work.

```
#include "vxui.h"
#define VXUI_IMPL
#include "vxui_impl.h"
```

## Build

```powershell
cmake -S . -B build
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```

## Headers

| File | Behind | Purpose |
|---|---|---|
| `vxui.h` | — | public types and API |
| `vxui_impl.h` | `VXUI_IMPL` | frame lifecycle, menu system, layout |
| `vxui_impl_util.h` | — | hash, bump arena, spring |
| `vxui_hud.h` | `VXUI_HUD_IMPL` | optional HUD primitives (bars, stocks, text boxes) |

## Usage

```cpp
static uint8_t clay_mem[16 * 1024 * 1024];
vxui_ctx ctx = {};

vxui_init     ( &ctx, 1280, 720, clay_mem, sizeof( clay_mem ) );
vxui_load_font( &ctx, font_bytes, font_size, 24.0f );

while ( running )
{
    vxui_frame( &ctx, 1.0f / 60.0f );

    if ( key_pressed( GLFW_KEY_UP    ) ) vxui_input( &ctx, "up" );
    if ( key_pressed( GLFW_KEY_DOWN  ) ) vxui_input( &ctx, "down" );
    if ( key_pressed( GLFW_KEY_ENTER ) ) vxui_input( &ctx, "confirm" );

    vxui_root( &ctx, "root", 0, 0 );
        if ( vxui_menu( &ctx, "main" ) )
        {
            if ( vxui_menu_action( &ctx, "Start" ) ) start_game();
            if ( vxui_menu_action( &ctx, "Quit"  ) ) running = false;
            vxui_menu_end( &ctx );
        }
    vxui_root_end( &ctx );

    vxui_draw_list dl = vxui_render( &ctx );
    my_render( dl );
}

vxui_shutdown( &ctx );
```

Menu rows emit one draw command each (`VXUI_DRAW_RECT` on fill rows, with a
child `VXUI_DRAW_TEXT`). Commands carry an `id` (hashed label), a `rect`, a
`state` bitmask (`FOCUSED | PRESSED | DISABLED | HOVERED`), and a
`focus_offset_y` from the per-menu spring.

Other menu primitives: `vxui_menu_option`, `vxui_menu_slider`,
`vxui_menu_section`, `vxui_menu_label`, `vxui_menu_cancelled`. Left/right
cycles options and sliders. Up/down navigates rows with DAS-paced repeat.

Layout: `vxui_div`, `vxui_rect`, `vxui_text`, `vxui_root`.

Modes: `vxui_page(name)` tests if the ctx is in a named mode,
`vxui_switch(ctx, name)` switches to it. Flat — no history. For nested
menus, keep your own stack.

## HUD add-on

```cpp
#define VXUI_HUD_IMPL
#include "vxui_hud.h"

vxui_hud hud = {};
vxui_hud_begin( &hud, &ctx, w, h );

vxui_hud_set_pos  ( &hud, 80, 40 );
vxui_hud_set_colour( &hud, { 1, 0.4f, 0.4f, 1 } );
vxui_hud_health_bar( &hud, "boss", tex, hp, max_hp, 800, 20, uv );

vxui_hud_set_pos       ( &hud, 20, 100 );
vxui_hud_resource_stock( &hud, "bombs", tex, bombs, 32, 32, VXUI_HUD_STOCK_HEX_COL, uv, 4 );

// call vxui_hud_resolve at the top of your render_data_fn
```

## Renderer hook

VXUI does not draw. Your renderer iterates the draw list and fills
`vxui_render_data` per command:

```cpp
static void resolve( const vxui_draw_cmd* c, vxui_render_data* out, void* ud )
{
    if ( c->type == VXUI_DRAW_TEXT )         out->colour = { 1, 1, 1, 1 };
    else if ( c->state & VXUI_DRAW_FOCUSED ) out->colour = { 1, 0.6f, 0.2f, 1 };
    else                                     out->colour = { 0.2f, 0.2f, 0.2f, 1 };
}

vxui_set_render_data_fn( &ctx, resolve, nullptr );
```

Commands carry `texture_id`, `uv`, `colour`, `material_id` in their render
data. The demo renderer at `demo/demo_render_gl.h` dispatches material_id
to CRT, round, outline, image, and text shaders.

## Demo

```powershell
.\build\Debug\vxui_demo.exe
```

GLFW + OpenGL. Portrait letterboxed inside a landscape window. Three modes:
splash, menu (main → operations / settings / archives / extras), and a HUD
gameplay screen. WASD / arrows to navigate, Enter to confirm, Esc to cancel.
Mouse works.

## Style

C-style C++, POD structs, no STL in the public API. Flat free functions
prefixed `vxui_`. Spaces inside parentheses. See `vxui_style.md`.
