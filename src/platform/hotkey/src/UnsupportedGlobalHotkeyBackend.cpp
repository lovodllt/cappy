#include "UnsupportedGlobalHotkeyBackend.h"

namespace cappy::platform::hotkey {

UnsupportedGlobalHotkeyBackend::UnsupportedGlobalHotkeyBackend(QString reason)
    : reason_(std::move(reason)) {}

QString UnsupportedGlobalHotkeyBackend::backendName() const {
    return "unsupported-global-hotkey";
}

bool UnsupportedGlobalHotkeyBackend::isSupported() const {
    return false;
}

QString UnsupportedGlobalHotkeyBackend::unsupportedReason() const {
    return reason_;
}

QString UnsupportedGlobalHotkeyBackend::lastError() const {
    return lastError_;
}

void UnsupportedGlobalHotkeyBackend::setActivationHandler(
    std::function<void(const QString&)> handler) {
    Q_UNUSED(handler);
}

bool UnsupportedGlobalHotkeyBackend::registerHotkey(const GlobalHotkey& hotkey) {
    Q_UNUSED(hotkey);
    lastError_ = reason_;
    return false;
}

void UnsupportedGlobalHotkeyBackend::unregisterAll() {}

} // namespace cappy::platform::hotkey
