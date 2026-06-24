#include "cappy/platform/hotkey/GlobalHotkeyBackendFactory.h"

#include "cappy/platform/hotkey/IGlobalHotkeyBackend.h"
#include "UnsupportedGlobalHotkeyBackend.h"

#if defined(Q_OS_LINUX)
#include "X11GlobalHotkeyBackend.h"
#elif defined(Q_OS_WIN)
#include "WinGlobalHotkeyBackend.h"
#endif

namespace cappy::platform::hotkey {

std::unique_ptr<IGlobalHotkeyBackend> createGlobalHotkeyBackend() {
#if defined(Q_OS_LINUX)
    if (X11GlobalHotkeyBackend::canCreate()) {
        return std::make_unique<X11GlobalHotkeyBackend>();
    }

    return std::make_unique<UnsupportedGlobalHotkeyBackend>(
        "Global hotkeys currently require an X11 (xcb) session.");
#elif defined(Q_OS_WIN)
    if (WinGlobalHotkeyBackend::canCreate()) {
        return std::make_unique<WinGlobalHotkeyBackend>();
    }

    return std::make_unique<UnsupportedGlobalHotkeyBackend>(
        "Global hotkeys currently require a Win32 desktop session.");
#else
    return std::make_unique<UnsupportedGlobalHotkeyBackend>(
        "Global hotkeys are not implemented on this platform yet.");
#endif
}

} // namespace cappy::platform::hotkey
