# Roadmap

## Phase 0: Baseline

- [x] define scope and exclusions
- [x] choose tech stack
- [x] scaffold CMake project
- [x] add plugin API and loader skeleton
- [x] create first build/package document
- [x] add Linux build/test/package CI baseline
- [x] add formatting and static analysis configuration
- [x] add Linux CI quality gate baseline for formatting and clang-tidy reporting
- [x] add Windows build-and-package CI baseline

## Phase 1: Desktop shell

- [x] tray app lifecycle
- [x] command routing
- [x] settings persistence
- [x] logging
- [x] first-run config directory layout
- [x] capture preview and in-memory history
- [x] explicit save flow without default auto-save
- [x] compact icon-first command surface for primary actions
- [x] local keyboard shortcuts for capture and history workflows
- [x] X11 global hotkey service baseline
- [x] settings page baseline for appearance, save path, history retention, and tray behavior
- [x] app-wide interface language baseline with persisted setting and runtime switching
- [x] headless `F1` capture entry path without opening the main window
- [x] Win32 global hotkey backend baseline

## Phase 2: Capture

- [x] region capture overlay
- [x] fullscreen capture
- [x] active-window capture
- [x] window-fit capture with preselected active-window bounds
- [x] Windows foreground-window and point-window geometry baseline
- [x] multi-screen support baseline
- [x] HiDPI-normalized region/window crop mapping baseline
- [x] post-capture review surface for copy / save / pin

## Phase 3: Editing

- [x] annotation canvas baseline
- [x] undo/redo stack baseline
- [x] export pipeline baseline: clipboard / pin / file save
- [x] richer tool set baseline: marker / mosaic / text / numbering
- [x] in-place region review overlay with resizable selection and always-visible actions
- [x] complete shortcut settings UI for global, shell, overlay, editor, and pinning workflows

## Phase 4: Pinning

- [x] pinned image windows
- [x] scale / opacity / always-on-top
- [x] keyboard and mouse interaction model
- [x] click-through mode with tray/main-window input restore
- [x] history item actions: pin / copy / save / remove

## Phase 5: Packaging

- [ ] Windows installer
- [x] Debian package baseline with desktop entry and icon install
- [x] Debian verification baseline with shared package check script and lintian in CI
- [ ] release checklist

## Phase 6: OCR

- [x] local OCR baseline through a configurable command
- [x] cloud OCR baseline through a configurable API endpoint and key
- [x] OCR settings integrated into the main settings surface
- [x] OCR entry points from capture overlay, editor, and pinned windows
- [x] OCR window visual polish baseline with preview/text linkage and line list
- [ ] OCR window error handling refinement
- [ ] optional local OCR dependency bootstrap guidance per distro
