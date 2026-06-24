# Cappy

`Cappy` is a cross-platform screenshot and pinning tool focused on fast startup, low overhead, and clean engineering boundaries.

Current scope:

- Windows 10/11
- Linux X11
- Packaging targets: `.exe` and `.deb`

Current implementation focus:

- Linux X11 first
- SunnyCapturer-style shortcut-first capture flow

Out of scope for the first phase:

- Wayland
- macOS
- Cloud or account features

## Principles

- Keep the core fast enough to stay resident in tray with low memory pressure.
- Prefer explicit module boundaries over convenience coupling.
- Treat platform integrations as replaceable adapters.
- Build a plugin system early, but keep the initial plugin surface narrow.

## Planned capabilities

- Region / fullscreen / active-window capture
- Explicit copy / pin / save without default auto-save
- Pin window with topmost, scale, opacity, drag, and click-through
- Annotation tools: rectangle, arrow, text, blur/mosaic
- Shortcut-first command surface, then global hotkeys
- Multi-screen and HiDPI support
- Tray, settings, history, undo/redo

## Repository layout

- [`docs/build-and-package.md`](docs/build-and-package.md): build, dependency, packaging, and distribution baseline
- [`docs/ci.md`](docs/ci.md): current GitHub Actions baseline and artifact expectations
- [`docs/architecture.md`](docs/architecture.md): module boundaries and plugin model
- [`docs/roadmap.md`](docs/roadmap.md): staged delivery plan
- [`docs/adr/0001-qt6-widgets-plugin-architecture.md`](docs/adr/0001-qt6-widgets-plugin-architecture.md): first architectural decision record

## Toolchain baseline

- C++20
- Qt 6.4+ Widgets
- CMake 3.28+
- MSVC 2022 or MinGW-w64 on Windows
- GCC 13+ or Clang 17+ on Linux

## Current status

This repository currently contains:

- the initial project scaffold
- the first packaging/build document
- the plugin API and loader skeleton
- a desktop shell with tray lifecycle, command routing, settings persistence, and file logging
- a capture pipeline covering region, fullscreen, and active-window capture
- multi-screen virtual desktop composition with per-screen normalized region/window cropping
- headless `F1`-first capture flow on Linux X11 without requiring the main window
- an in-place region review overlay with shortcut-driven copy, save, pin, and drawing tools aligned to `1..8` and `Ctrl+1..8`
- a full, always-visible action row under the active selection
- in-memory history with explicit save and no default auto-save
- pinned image windows with drag, zoom, opacity, lock, click-through, tray-based input restore, and source-position pin placement
- a compact icon-first main window with local shortcuts for capture, save, pin, and history actions
- an X11 global hotkey baseline aligned to the primary SunnyCapturer screenshot entry path
- a settings surface covering light/dark shell theme, global hotkey bindings, default save directory, history limit, and tray behavior
- a complete shortcut management surface covering global hotkeys, main-window actions, capture overlay tools/actions, editor actions, and pinned-window controls
- runtime application of shell appearance, hotkey enablement, and history retention limit without restarting the app
- a dual-path OCR baseline with local command execution and configurable cloud API inference
- an OCR result workspace with preview, line list, full-text output, provider switching, rerun, copy, and save flows
- local OCR line-region highlighting rendered back onto the preview image with bidirectional preview/text linkage
- formatting and static-analysis baseline through `.clang-format`, `.clang-tidy`, and helper scripts

The next coding step is stabilizing the first Windows build-and-package loop, then adding Windows runtime validation.

## Latest verification

Validated on 2026-06-06 with a user-space Ubuntu Qt sysroot:

- CMake configure succeeded
- full build succeeded
- offscreen smoke startup succeeded with plugin discovery and log output
- OCR service, OCR window, settings, history-limit, and shell-theme changes compile and pass startup smoke
- X11 fullscreen capture smoke passed
- X11 pin-after-capture smoke passed
- Qt widget tests cover overlay, pin window, main-window history retention, and settings dialog structure
- earlier interactive region / fullscreen / active-window / save / pin flows were manually validated in the active X11 session
- GitHub Actions Linux CI now covers formatting checks, clang-tidy reporting, build, offscreen tests, install staging, desktop-entry validation, and Debian packaging
- GitHub Actions Windows CI now covers MSVC build, packaging, and package verification
