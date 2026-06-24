# Architecture

## Goals

- Fast startup
- Low idle memory
- Clear ownership boundaries
- Platform-specific code isolated behind adapters
- Plugin support without making the first release dependent on third-party extensions

## Top-level structure

```text
src/
  apps/
    cappy/                 Desktop shell and composition root
  libs/
    localization/          App-wide UI strings and language resolution
    plugin_api/            Stable plugin interfaces
    plugin_host/           Plugin discovery and lifecycle
  domain/                  Core value types and use cases
  services/                App services orchestrating domain flows
  features/
    capture/               Capture workflows and UI integration
    annotation/            Draw tools and edit stack
    pinboard/              Pinned image windows
    settings/              Settings models and persistence
    history/               Capture history and indexing
  platform/
    win/                   Win32 hotkeys, screen capture, window enumeration
    x11/                   X11 capture, hotkeys, window enumeration
  plugins/
    builtin_core/          First-party runtime plugin(s)
```

Only a subset is scaffolded today. The rest is reserved so growth does not collapse into a flat `src/` tree.

## Dependency rules

- `apps` may depend on anything.
- `services` may depend on `domain`, `features`, `platform`, and `plugin_api`.
- `features` may depend on `domain` and narrow `platform` abstractions.
- `platform` must not depend on UI modules outside of adapter-specific helpers.
- `plugin_api` must stay small, stable, and UI-framework-light.
- Plugins must talk to the host through explicit interfaces, not by reaching into internal headers.

## Runtime model

The final runtime should split into these responsibilities:

1. `App shell`
   - bootstrap Qt
   - own tray lifecycle
   - own top-level windows
   - own local command actions and shortcut bindings
   - wire services
   - persist shell settings
   - initialize logging

2. `Capture service`
   - resolve capture mode
   - ask platform adapter for pixels/window geometry
   - emit capture result
   - own overlay session lifecycle for interactive region capture
   - keep the main window out of the critical screenshot path

3. `Annotation service`
   - own the lightweight post-capture review window
   - own edit commands
   - own undo/redo stack
   - export final bitmap

4. `Pinboard service`
   - create and manage pinned windows
   - persist per-window state only if needed later

5. `Hotkey service`
   - register platform-specific global shortcuts
   - map them to application commands
   - suspend registrations during exclusive capture sessions when needed
   - current baseline backends:
     - X11 passive grabs on Linux `xcb`
     - Win32 `RegisterHotKey` on Windows

6. `Localization library`
   - resolve system vs explicit UI language choice
   - provide stable string keys for shell, overlay, editor, and pinboard surfaces
   - keep language switching out of platform adapters and business logic

The shell still provides window-scoped shortcuts for the desktop UI even when the global hotkey service is available.

Primary Linux workflow target:

1. global `F1` enters region capture directly
2. an in-place overlay keeps the outside area dimmed while the selection can still be moved or resized
3. copy / save / pin / draw actions complete inside the overlay without surfacing the main window
4. the main window stays a secondary management surface for history, tray recovery, and diagnostics

## Plugin model

The first plugin surface should stay intentionally narrow:

- tool providers
- export providers
- optional capture strategies

Do not expose internals such as QWidget pointers, concrete repositories, or direct event bus access in v1.

Recommended plugin loading policy:

- built-in first-party plugins ship with the app
- third-party plugins disabled by default in early milestones
- explicit compatibility version in metadata
- failed plugin load must not block app startup

## Packaging model

- Windows: app binary + deployed Qt runtime + plugins + installer
- Linux X11: app binary + plugins + desktop integration + `.deb`

We should package plugins next to the app/runtime, not in user-writable extension directories during the first phase.

## Current capture slice

The current region capture path is:

1. `AppController` routes the command
2. `CaptureCoordinator` asks the backend for a full desktop frame
3. `CaptureOverlayWidget` collects the user selection
4. the selected region stays live in-place so the user can move, resize, annotate, and finalize without a separate review window
5. the finalized bitmap is stored in in-memory history and can be copied, saved, or pinned
6. the main window remains optional and is not required to finish the capture

Current backend implementation:

- `qt-screen` backend using Qt screen grabs
- Linux support target: X11 / `xcb`
- Windows baseline uses Qt screen grabs with Win32 foreground-window and point-window geometry queries
- unsupported platform plugins fail fast with a clear status string
- virtual-desktop frames keep a logical preview image plus per-screen native fragments for normalized cropping

Current pinboard slice:

1. `AppController` hands a `QImage` to `PinboardManager`
2. `PinboardManager` creates a `PinnedImageWindow` per pinned image
3. region captures can place the pinned image back at the original screen position instead of re-centering it
4. each pinned window manages drag, lock, scale, opacity, and optional click-through locally
5. tray and main-window actions provide a recovery path for restoring pointer input on all pins

Current overlay editing slice:

1. `CaptureOverlayWidget` owns the interactive selection, toolbar, and editing surface for region capture
2. drawing tools commit directly onto a raster image with snapshot-based undo/redo
3. copy / save / pin actions flow back through `AppController`, which keeps history and pinboard state consistent
4. the current Linux baseline supports `1..8` and `Ctrl+1..8`: rectangle, ellipse, arrow, pen, marker, mosaic, text, serial
5. action shortcuts remain keyboard-first: `C` copy, `S` save, `P` pin, `Z/Y` undo/redo, `Esc` cancel, `Enter` quick copy

Current localization slice:

1. `AppSettings` persists an explicit interface language or `system`
2. `cappy_localization` resolves the effective language and provides shared strings
3. `AppController` fans language changes out to the shell, tray, overlay, editor, and pinboard windows
4. current supported UI languages are English and Simplified Chinese

Current OCR slice:

1. capture overlay, capture editor, and pinned image windows emit OCR requests upward using only `QImage`
2. `AppController` opens a dedicated OCR result window and injects current language and shell appearance
3. `OcrService` provides two interchangeable chains:
   - local command execution, defaulting to `tesseract`
   - cloud OCR through a configurable compatible API endpoint
4. the local `tesseract` path also parses TSV word boxes so the OCR preview can highlight recognized areas
5. OCR settings stay in shell settings so provider choice and credentials are centralized
6. OCR UI stays independent from capture/pin flows so the recognition backend can later be replaced or pluginized
