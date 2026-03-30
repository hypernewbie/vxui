# VXUI

VXUI is a C-first game UI runtime for authored menu screens, animated focus, screen transitions, localisation, prompt glyphs, and renderer-facing draw lists.

The public surface is intentionally split into two layers:

- `vxui.h` for the low-level runtime, frame lifecycle, motion, focus, localisation, prompts, and screen transitions
- `vxui_menu.h` for authored game-menu shells, rows, sections, preview/help/footer composition, and split-deck/form layouts

If you are building game-front-end screens, start with `vxui_menu.h`.

## Build

```powershell
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Main targets:

- `vxui_demo`
- `vxui_tests`
- `vxui_release_probe`

## Two Public Layers

### `vxui.h`

Use `vxui.h` when you need:

- context setup and fixed-memory init
- `vxui_begin()` / `vxui_end()`
- navigation and focus ownership
- spring motion and retained exit states
- locale-aware text and font resolution
- prompt tables and RTL handling
- screen stack transitions and sequences
- renderer-facing `vxui_draw_list`

### `vxui_menu.h`

Use `vxui_menu.h` when you need to author screens such as:

- title menus
- split-deck mission/loadout/records screens
- settings and form screens
- footer prompt/status strips
- preview panels and embedded help legends

The authored shell kinds are:

- `VXUI_MENU_SHELL_TITLE_MENU`
- `VXUI_MENU_SHELL_SPLIT_DECK`
- `VXUI_MENU_SHELL_FORM`

Authored demo screens can also wrap the shell in the canonical surface helper:

```cpp
vxui_menu_surface_cfg surface = {
    1280.0f,
    720.0f,
    16.0f,
    20.0f,
    18.0f,
    16.0f,
    18.0f,
    1.0f,
    { 5, 9, 16, 255 },
    { 19, 31, 48, 244 },
    { 36, 58, 88, 255 },
};

vxui_menu_surface_begin( &ctx, "main", "main.surface", &surface );
vxui_menu_screen_begin( &ctx, &menu, "main.screen", &screen );
// ... header, lanes, preview/footer ...
vxui_menu_screen_end( &ctx, &menu );
vxui_menu_surface_end( &ctx );
```

## Quickstart: Title Menu Shell

```cpp
vxui_menu_state menu = {};

const char* rows[] = { "menu.sortie", "menu.loadout", "menu.settings", "menu.quit" };
vxui_menu_prompt_item prompts[] = {
    { "main.prompt.confirm", "action.confirm", "prompt.confirm" },
    { "main.prompt.cancel", "action.cancel", "prompt.back" },
};
vxui_menu_status_item status[] = {
    { "main.footer.status.locale", "status.locale", "Loc EN", VXUI_MENU_STATUS_PRIMARY, false },
    { "main.footer.status.prompts", "status.input", "Input KB", VXUI_MENU_STATUS_PRIMARY, false },
    { "main.footer.status.top", "status.layer", "Top Deck", VXUI_MENU_STATUS_SECONDARY, false },
};

vxui_menu_style shell_style = vxui_menu_style_title_menu();
vxui_menu_style row_style = vxui_menu_style_br_title();
row_style.panel_fill_color = { 0, 0, 0, 1 };

vxui_menu_screen_cfg screen = {
    VXUI_MENU_SHELL_TITLE_MENU,
    &shell_style,
    VXUI_MENU_SHELL_COMPACT_AUTO,
    648.0f,
    1180.0f,
    false,
    { "menu.command_deck", nullptr, false },
    { 356.0f, 320.0f, false, false, false },
    { 0 },
    { 420.0f, 360.0f, true, false, false },
    {
        "main.preview_header",
        "main.preview_body",
        "preview.section",
        "preview.title",
        "preview.subtitle",
        "badge.recommended",
        "preview.body",
        { "controls.heading", 0, true, 0, false },
    },
    {
        prompts,
        2,
        status,
        3,
        VXUI_MENU_SHELL_COMPACT_AUTO,
        3,
        false,
    },
};

vxui_menu_screen_begin( &ctx, &menu, "main.screen", &screen );
vxui_menu_header( &ctx, "main.header", &screen.header );

vxui_menu_primary_lane_begin( &ctx, "main.command_panel", &screen.primary_lane );
vxui_menu_begin( &ctx, &menu, "main.menu", ( vxui_menu_cfg ) { .style = &row_style } );
for ( int i = 0; i < 4; ++i ) {
    vxui_menu_action( &ctx, &menu, rows[ i ], rows[ i ], nullptr, ( vxui_menu_row_cfg ) { 0 }, ( vxui_action_cfg ) { 0 } );
}
vxui_menu_end( &ctx, &menu );
vxui_menu_primary_lane_end( &ctx );

vxui_menu_tertiary_lane_begin( &ctx, "main.preview_panel", &screen.tertiary_lane );
vxui_menu_preview( &ctx, "main.preview", &screen.preview );
vxui_menu_footer( &ctx, "main.footer", &screen.footer );
vxui_menu_tertiary_lane_end( &ctx );

vxui_menu_screen_end( &ctx, &menu );
```

## Quickstart: Settings Form Shell

```cpp
vxui_menu_state settings = {};

vxui_menu_prompt_item prompts[] = {
    { "settings.prompt.confirm", "action.confirm", "prompt.confirm" },
    { "settings.prompt.cancel", "action.cancel", "prompt.back" },
};
vxui_menu_status_item status[] = {
    { "settings.footer.status.locale", "status.locale", "Loc EN", VXUI_MENU_STATUS_PRIMARY, false },
    { "settings.footer.status.prompts", "status.input", "Input KB", VXUI_MENU_STATUS_PRIMARY, false },
    { "settings.footer.status.top", "status.layer", "Top Settings", VXUI_MENU_STATUS_SECONDARY, false },
};

vxui_menu_style shell_style = vxui_menu_style_form_shell();
vxui_menu_style form_style = vxui_menu_style_form();
form_style.panel_fill_color = { 0, 0, 0, 1 };

vxui_menu_screen_cfg screen = {
    VXUI_MENU_SHELL_FORM,
    &shell_style,
    VXUI_MENU_SHELL_COMPACT_AUTO,
    648.0f,
    1180.0f,
    false,
    { "menu.settings", "menu.settings_subtitle", false },
    { 0.0f, 480.0f, true, false, false },
    {},
    {},
    {},
    { prompts, 2, status, 3, VXUI_MENU_SHELL_COMPACT_AUTO, 3, false },
};

vxui_menu_screen_begin( &ctx, &settings, "settings.screen", &screen );
vxui_menu_header( &ctx, "settings.header", &screen.header );
vxui_menu_primary_lane_begin( &ctx, "settings.body", &screen.primary_lane );

vxui_menu_begin( &ctx, &settings, "settings.menu", ( vxui_menu_cfg ) { .style = &form_style } );
vxui_menu_section( &ctx, &settings, "interface", "settings.interface", ( vxui_menu_section_cfg ) { 0 } );
vxui_menu_option( &ctx, &settings, "prompts", "settings.prompts", 0, input_modes, 2, ( vxui_menu_row_cfg ) { 0 }, ( vxui_option_cfg ) { 0 } );
vxui_menu_section( &ctx, &settings, "visual_fx", "settings.visual_fx", ( vxui_menu_section_cfg ) { 0 } );
vxui_menu_slider( &ctx, &settings, "scanlines", "settings.scanlines", &scanline_value, ( vxui_menu_row_cfg ) { 0 }, ( vxui_slider_cfg ) { 0 } );
vxui_menu_end( &ctx, &settings );

vxui_menu_primary_lane_end( &ctx );
vxui_menu_footer( &ctx, "settings.footer", &screen.footer );
vxui_menu_screen_end( &ctx, &settings );
```

## Quickstart: Split-Deck Shell

```cpp
vxui_menu_state sortie = {};

vxui_menu_style shell_style = vxui_menu_style_split_deck();
vxui_menu_style menu_style = vxui_menu_style_form();
menu_style.value_lane_mode = VXUI_MENU_VALUE_LANE_GROW;
menu_style.panel_fill_color = { 0, 0, 0, 1 };

vxui_menu_prompt_item prompts[] = {
    { "sortie.prompt.confirm", "action.confirm", "prompt.confirm" },
    { "sortie.prompt.cancel", "action.cancel", "prompt.back" },
};
vxui_menu_status_item status[] = {
    { "sortie.footer.status.locale", "status.locale", "Loc EN", VXUI_MENU_STATUS_PRIMARY, false },
    { "sortie.footer.status.prompts", "status.input", "Input KB", VXUI_MENU_STATUS_PRIMARY, false },
    { "sortie.footer.status.top", "status.layer", "Top Sortie", VXUI_MENU_STATUS_SECONDARY, false },
};

vxui_menu_screen_cfg screen = {
    VXUI_MENU_SHELL_SPLIT_DECK,
    &shell_style,
    VXUI_MENU_SHELL_COMPACT_AUTO,
    680.0f,
    1140.0f,
    true,
    { "menu.sortie", nullptr, false },
    { 360.0f, 320.0f, false, false, false },
    { 420.0f, 320.0f, true, false, false },
    { 180.0f, 132.0f, false, true, false },
    {},
    { prompts, 2, status, 3, VXUI_MENU_SHELL_COMPACT_AUTO, 3, false },
};

vxui_menu_screen_begin( &ctx, &sortie, "sortie.screen", &screen );
vxui_menu_header( &ctx, "sortie.header", &screen.header );

vxui_menu_primary_lane_begin( &ctx, "sortie.menu_panel", &screen.primary_lane );
vxui_menu_begin( &ctx, &sortie, "sortie.menu", ( vxui_menu_cfg ) { .style = &menu_style } );
vxui_menu_option( &ctx, &sortie, "mission", "menu.mission", mission_index, mission_names, mission_count, ( vxui_menu_row_cfg ) { 0 }, ( vxui_option_cfg ) { 0 } );
vxui_menu_option( &ctx, &sortie, "difficulty", "menu.difficulty", difficulty_index, difficulty_names, difficulty_count, ( vxui_menu_row_cfg ) { 0 }, ( vxui_option_cfg ) { 0 } );
vxui_menu_action( &ctx, &sortie, "start", "menu.start_sortie", start_fn, ( vxui_menu_row_cfg ) { 0 }, ( vxui_action_cfg ) { 0 } );
vxui_menu_end( &ctx, &sortie );
vxui_menu_primary_lane_end( &ctx );

vxui_menu_secondary_lane_begin( &ctx, "sortie.briefing", &screen.secondary_lane );
VXUI_LABEL( &ctx, "OP-01 Aegis Break", ( vxui_label_cfg ) { .font_id = 0, .font_size = 28.0f, .color = { 255, 255, 255, 255 } } );
VXUI_LABEL( &ctx, "Breach the carrier screen and keep the convoy alive.", ( vxui_label_cfg ) { .font_id = 0, .font_size = 18.0f, .color = { 210, 216, 224, 255 } } );
vxui_menu_secondary_lane_end( &ctx );

vxui_menu_tertiary_lane_begin( &ctx, "sortie.detail", &screen.tertiary_lane );
VXUI_LABEL( &ctx, "Threat Tier 02 / Interceptor bloom", ( vxui_label_cfg ) { .font_id = 0, .font_size = 16.0f, .color = { 255, 220, 160, 255 } } );
VXUI_LABEL( &ctx, "Reward: Burst rail prototype", ( vxui_label_cfg ) { .font_id = 0, .font_size = 15.0f, .color = { 210, 216, 224, 255 } } );
vxui_menu_tertiary_lane_end( &ctx );

vxui_menu_footer( &ctx, "sortie.footer", &screen.footer );
vxui_menu_screen_end( &ctx, &sortie );
```

## Prompt, Status, Preview, and Help Config

`vxui_menu_footer_cfg` owns the shell footer strip.

- `prompt_items` defines the left prompt group
- `status_items` defines the right status group
- `compact_mode` controls when the strip compacts
- `compact_max_status_items` limits status density under tight widths

`vxui_menu_preview_cfg` owns title-menu preview content.

- section label
- title
- subtitle
- badge text
- teaser/body copy
- embedded `vxui_menu_help_cfg`

`vxui_menu_help_cfg` controls the help legend.

- heading text
- visible line count
- compact flag
- density

Compact behavior is shell-owned. Authors should fill config data, not hand-tune box trees per screen.

## Screenshot Review Workflow

The demo supports one-shot capture for deterministic visual review. Each command renders one frame, writes one PNG, and exits.

Canonical commands:

```powershell
.\build\vxui_demo.exe --shot --screen=main_menu --width=1280 --height=720 --out=shots\main_menu_1280x720.png
.\build\vxui_demo.exe --shot --screen=main_menu --width=1280 --height=648 --out=shots\main_menu_1280x648.png
.\build\vxui_demo.exe --shot --screen=sortie --width=1280 --height=720 --out=shots\sortie_1280x720.png
.\build\vxui_demo.exe --shot --screen=sortie --width=1100 --height=648 --out=shots\sortie_1100x648.png
.\build\vxui_demo.exe --shot --screen=settings --width=1280 --height=720 --out=shots\settings_1280x720.png --focus=scanlines
.\build\vxui_demo.exe --shot --screen=records --width=1280 --height=720 --out=shots\records_1280x720.png --focus=run
.\build\vxui_demo.exe --shot --screen=loadout --width=1280 --height=720 --out=shots\loadout_1280x720.png
.\build\vxui_demo.exe --shot --screen=archives --width=1280 --height=720 --out=shots\archives_1280x720.png
```

Useful flags:

- `--locale=en|ja|ar`
- `--prompts=keyboard|gamepad`
- `--focus=<id>`
- `--compact`
- `--no-scanline`

Recommended loop:

1. generate the canonical shot
2. inspect the PNG
3. change code
4. rebuild
5. regenerate the exact same shot
6. compare before and after

## Repo Notes

- `main.cpp` hosts the demo runtime and shot-mode entry path
- `demo/internal/` contains demo-only theme/content/helpers
- `vxui.h` and `vxui_menu.h` are the only public authored headers

The demo and tests are the reference implementations for the current shell surface.
