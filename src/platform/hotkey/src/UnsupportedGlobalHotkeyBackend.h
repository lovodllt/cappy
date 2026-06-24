#pragma once

#include <functional>

#include "cappy/platform/hotkey/IGlobalHotkeyBackend.h"

namespace cappy::platform::hotkey {

class UnsupportedGlobalHotkeyBackend final : public IGlobalHotkeyBackend {
public:
    explicit UnsupportedGlobalHotkeyBackend(QString reason);

    QString backendName() const override;
    bool isSupported() const override;
    QString unsupportedReason() const override;
    QString lastError() const override;
    void setActivationHandler(std::function<void(const QString&)> handler) override;
    bool registerHotkey(const GlobalHotkey& hotkey) override;
    void unregisterAll() override;

private:
    QString reason_;
    QString lastError_;
};

}  // namespace cappy::platform::hotkey
