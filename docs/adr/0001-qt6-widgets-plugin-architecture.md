# ADR 0001: Qt 6 Widgets with a Plugin-Oriented Core

## Status

Accepted

## Context

The product target is a resident desktop screenshot utility for Windows and Linux X11. It must stay lightweight, start quickly, and tolerate platform-specific code without spreading it across the whole tree.

## Decision

We will use:

- C++20
- Qt 6.4+ Widgets
- CMake
- a small runtime plugin surface based on Qt plugin loading

## Why

- Qt Widgets is mature for desktop utility UI and custom drawing.
- Screenshot overlays, pin windows, and annotation surfaces need tight control over paint and events.
- Qt on Windows and X11 is well-understood compared to a web runtime.
- A plugin-oriented boundary helps keep optional capability isolated without committing to a public extension marketplace yet.

## Consequences

Positive:

- good control over memory, startup, and native desktop behavior
- simple packaging path to `.exe` and `.deb`
- platform adapters can be isolated early

Negative:

- more manual UI state management than declarative frameworks
- plugin ABI stability requires discipline
- Qt deployment on Windows must be handled carefully

## Revisit trigger

Revisit this decision only if:

- Widgets becomes a material blocker for annotation UX, or
- the plugin boundary becomes too narrow for first-party feature growth
