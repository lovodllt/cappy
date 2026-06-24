# CI

This document defines the first continuous-integration baseline for `Cappy`.

## Current scope

The current CI coverage includes:

- Linux on `ubuntu-24.04`
- Windows on `windows-2022`

Workflow:

- `.github/workflows/linux.yml`
- `.github/workflows/windows.yml`

The workflow executes:

1. run a `quality` job:
   - `clang-format` check
   - `clang-tidy` report generation
2. run a `build-test-package` job:
   - install the Qt/X11 build dependencies
   - configure a Release build with Ninja
   - build the project
   - run widget and service tests in offscreen mode
   - install into a staging directory
   - validate the installed `.desktop` file
   - generate a Debian package with `cpack`
   - resolve the generated `.deb` path dynamically
   - inspect the package metadata and file list
   - run `lintian`
   - upload the `.deb` artifact
3. run a Windows build job:
   - install Qt 6 desktop runtime/tooling with a Windows-supported MSVC package tuple
   - install NSIS for CPack-based `.exe` generation
   - install Ninja explicitly on the runner
   - install `7zip` and use external extraction for Qt archives
   - configure a Release build with Ninja
   - build the project with MSVC
   - run the Windows packaging script
   - verify the generated Windows packages
   - upload `.zip` and `.exe` artifacts

## Why this order

- `clang-format` keeps style drift out of the main branch
- `clang-tidy` starts surfacing structural issues without blocking delivery immediately
- build catches compile and linkage regressions
- offscreen `ctest` keeps the current widget coverage active in CI
- install validation catches packaging regressions before `cpack`
- `.desktop` validation catches launcher mistakes that a pure build would miss
- `lintian` catches common Debian packaging mistakes before release
- artifact upload makes every successful run produce an installable Debian package
- Windows CI verifies that the Win32 platform code and packaging script at least build on a real MSVC toolchain
- the workflow now prints the resolved Qt toolchain and NSIS version to reduce opaque runner failures
- the workflow now pins Windows Qt to `6.8.3` because the `win64_msvc2022_64` package family is not available for `6.7.x`
- the workflow now resolves `windeployqt` from `QT_ROOT_DIR/bin` instead of inferring it from `qmake`
- Linux package verification now resolves the generated `.deb` path dynamically instead of hardcoding a filename

## Current exclusions

Not yet covered:

- installer signing
- blocking `clang-tidy`
- runtime X11 interaction smoke in a real desktop session
- Windows runtime smoke tests

## Expected artifact

Current CI artifacts:

- `cappy-clang-tidy-report`
- `cappy-linux-deb`
- `cappy-windows-packages`

Current package artifacts contain:

- `cappy_0.1.0_amd64.deb`
- Windows `.zip` package
- Windows `.exe` installer

## Near-term follow-up

1. add Windows runtime smoke coverage
2. turn selected `clang-tidy` checks into blocking gates after baseline cleanup
3. add release-oriented workflow split once Windows packaging is ready
