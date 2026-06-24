#pragma once

#include <functional>

#include <QString>

#include "cappy/platform/hotkey/GlobalHotkeyTypes.h"

namespace cappy::platform::hotkey {

class IGlobalHotkeyBackend {
  public:
    virtual ~IGlobalHotkeyBackend() = default;

    virtual QString backendName() const = 0;
    virtual bool isSupported() const = 0;
    virtual QString unsupportedReason() const = 0;
    virtual QString lastError() const = 0;
    virtual void setActivationHandler(std::function<void(const QString&)> handler) = 0;
    virtual bool registerHotkey(const GlobalHotkey& hotkey) = 0;
    virtual void unregisterAll() = 0;
};

} // namespace cappy::platform::hotkey
