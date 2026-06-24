# Build and Package

This document defines the baseline build and distribution process for `Cappy`.

## Scope

- Supported targets:
  - Windows 10/11 x64
  - Linux x86_64 with X11
- Packaging outputs:
  - Windows `.exe` installer
  - Debian `.deb`

Wayland and macOS are intentionally excluded from this phase.

## Toolchain

### Windows

- Visual Studio 2022 with MSVC v143, or MinGW-w64 if the whole team standardizes on it
- CMake 3.28+
- Qt 6.4+ Widgets build matching the compiler
- NSIS for `.exe` installer generation
- Ninja recommended

Preferred baseline:

- MSVC 2022
- Qt online installer desktop build
- Ninja

Current implementation baseline on Windows:

- Qt screen-grab based capture backend is enabled
- Win32 global hotkeys use `RegisterHotKey`
- active-window and window-fit capture depend on Win32 foreground/window geometry queries

### Linux

- GCC 13+ or Clang 17+
- CMake 3.28+
- Qt 6.4+ development packages
- `dpkg-dev`, `fakeroot`, `lintian`
- Ninja recommended

Ubuntu/Debian baseline packages:

```bash
sudo apt update
sudo apt install -y \
  build-essential \
  cmake \
  ninja-build \
  pkg-config \
  qt6-base-dev \
  qt6-base-dev-tools \
  libx11-dev \
  libxfixes-dev \
  libxtst-dev \
  libxcb1-dev \
  dpkg-dev \
  fakeroot \
  lintian
```

Optional OCR runtime packages:

```bash
sudo apt install -y \
  tesseract-ocr \
  tesseract-ocr-eng \
  tesseract-ocr-chi-sim
```

The local OCR chain defaults to `tesseract` and `eng+chi_sim`. If a different local OCR command is preferred, configure it in the settings page instead of changing code.

## Configure and build

### Linux

```bash
cmake -S . -B build -G Ninja
cmake --build build
```

### Windows

```powershell
cmake -S . -B build -G Ninja
cmake --build build --config Release
```

If Ninja is not installed, use the Visual Studio generator instead.

### Ubuntu user-space fallback without sudo

When system package installation is unavailable, we can bootstrap a local Qt sysroot from Ubuntu `.deb` packages:

```bash
./scripts/bootstrap-ubuntu-qt-userland.sh
./scripts/configure-local-ubuntuqt.sh
```

This creates:

- `.venv-tools/` for `ninja`
- `.local-tools/debs/` for downloaded packages
- `.local-tools/sysroot/` for the extracted Qt runtime and CMake files

The build then uses:

- `CMAKE_PREFIX_PATH=.local-tools/sysroot/usr/lib/x86_64-linux-gnu/cmake`
- `LD_LIBRARY_PATH=.local-tools/sysroot/usr/lib/x86_64-linux-gnu`

## Package strategy

### Windows `.exe`

We should not rely on a raw executable drop. The output must contain:

- `cappy.exe`
- Qt runtime DLLs
- platform plugins
- image format plugins
- `plugins/` runtime directory for Cappy modules

Recommended flow:

1. build `Release`
2. run `windeployqt` on `cappy.exe`
3. stage first-party Cappy plugins into the packaged `plugins/` directory
4. run `cpack -G NSIS`

Current Windows packaging script:

```powershell
./scripts/package-windows.ps1 -BuildDir build-win -QtBinDir "C:\Qt\6.8.3\msvc2022_64\bin"
```

The script performs:

1. configure a Release build
2. build the binaries
3. run `windeployqt` on `bin/cappy.exe`
4. keep the installed `plugins/` directory next to the app runtime
5. generate `ZIP` and `NSIS` packages with `cpack`
6. verify that the executable, plugin directory, ZIP package, and NSIS installer exist

Manual Windows package verification:

```powershell
./scripts/verify-windows-package.ps1 -BuildDir build-win
```

Notes:

- NSIS is a practical default because it produces a real `.exe` installer and integrates with CPack.
- Inno Setup is also viable, but it adds a second installer description language. We should avoid that until CPack becomes a limitation.

### Debian `.deb`

The Linux package should initially depend on system Qt packages instead of bundling Qt.

Recommended flow:

1. build release artifacts
2. `cmake --install build --prefix build/stage`
3. run `cpack -G DEB --config build/CPackConfig.cmake`
4. inspect the package with `dpkg-deb -I` and `lintian`

Notes:

- `.deb` packaging assumes X11 runtime dependencies are available in the target distro.
- X11 global hotkeys use passive grabs on the X root window. The current Linux-first default is `F1` for region capture, so desktop-environment bindings may need to be disabled if they claim it first.
- Linux install now ships a `.desktop` launcher and an application icon under the standard XDG install paths.

Current Linux packaging script:

```bash
./scripts/package-linux.sh build-release
```

The script performs:

1. configure a Release build
2. build the binaries
3. install into `build-release/stage`
4. generate a `.deb` package with `cpack`
5. verify the generated package through `dpkg-deb` and `lintian` when available
6. print staged files and generated package paths

## Current local status

Verification completed on 2026-06-02:

- `cmake` configure succeeded with a user-space Ubuntu Qt sysroot
- `cmake --build` succeeded with Ninja
- `QT_QPA_PLATFORM=offscreen timeout 3s ./build-ubuntuqt/bin/cappy` launched successfully
- runtime log confirmed shell initialization and expected tray degradation in offscreen mode

Additional verification completed on 2026-06-03:

- `QT_QPA_PLATFORM=offscreen ./build-verify/bin/cappy --smoke-test` exited successfully
- `timeout 10s ./build-verify/bin/cappy --smoke-fullscreen-capture` exited successfully in an active X11 session
- `timeout 10s ./build-verify/bin/cappy --smoke-pin-latest-capture` exited successfully in an active X11 session

Additional verification completed on 2026-06-06:

- `cmake --build build-verify -j2` succeeded after OCR service and window integration
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build-verify --output-on-failure` succeeded

Additional verification completed on 2026-06-24:

- Linux install rules now stage:
  - `bin/cappy`
  - `share/applications/cappy.desktop`
  - `share/icons/hicolor/scalable/apps/cappy.svg`
- Debian packaging metadata now includes Qt runtime dependencies and OCR recommendations
- Debian verification now has a shared script: `scripts/verify-deb-package.sh`
- Windows CI baseline now builds with MSVC on `windows-2022` and runs the packaging script
- Windows CI now installs NSIS explicitly and prints the resolved Qt toolchain to simplify packaging diagnostics
- Windows CI now uses Qt `6.8.3` because `win64_msvc2022_64` is only supported by the install action on Qt `6.8+`
- Windows CI now installs `ninja` explicitly and resolves `windeployqt` from `QT_ROOT_DIR/bin`
- Windows CI now uses newer `aqtinstall` settings with external `7z` extraction to reduce Qt archive install failures on the runner
- Windows package verification now accepts dynamically discovered artifact paths because CPack generators do not always emit ZIP and NSIS outputs into the same directory

Important note:

- `aqtinstall` Linux Qt packages were tested and rejected for this workspace because the downloaded Qt runtime linked against `ICU 56`, which is incompatible with the current Ubuntu 24.04 host without extra compatibility libraries.

## Formatting and static analysis baseline

Current repository-level baseline:

- `.clang-format`
- `.clang-tidy`
- `scripts/format-cpp.sh`
- `scripts/run-clang-tidy.sh`

Usage:

```bash
./scripts/format-cpp.sh
./scripts/run-clang-tidy.sh build-verify
```

Notes:

- `clang-format` and `clang-tidy` must be available on `PATH`
- `clang-tidy` requires `compile_commands.json`, which is now exported by default through CMake
- Debian verification can be run manually with:

```bash
./scripts/verify-deb-package.sh ./cappy_0.1.0_amd64.deb
```

## CI target

We should add CI in this order:

1. Linux build job
2. Windows build job
3. Linux `.deb` artifact job
4. Windows installer artifact job

CI should fail on:

- compile errors
- plugin load smoke test failure
- packaging script failure

## Release layout

Planned artifact names:

- `Cappy-${version}-win64.exe`
- `Cappy-${version}-win64.zip`
- `Cappy-${version}-linux-amd64.deb`

## Near-term follow-up

- add `.desktop` file and icon assets
- add packaging smoke scripts
- add CI workflow
- add dependency bootstrap docs for Windows setup in more detail
- add a real install/staging script for Linux `.deb`
