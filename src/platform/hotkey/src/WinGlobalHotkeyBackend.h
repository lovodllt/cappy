#pragma once

#include <QAbstractNativeEventFilter>
#include <QHash>

#include "cappy/platform/hotkey/IGlobalHotkeyBackend.h"

namespace cappy::platform::hotkey {

class WinGlobalHotkeyBackend final
    : public IGlobalHotkeyBackend
    , public QAbstractNativeEventFilter {
public:
    static bool canCreate();

    WinGlobalHotkeyBackend();
    ~WinGlobalHotkeyBackend() override;

    QString backendName() const override;
    bool isSupported() const override;
    QString unsupportedReason() const override;
    QString lastError() const override;
    void setActivationHandler(std::function<void(const QString&)> handler) override;
    bool registerHotkey(const GlobalHotkey& hotkey) override;
    void unregisterAll() override;
    bool nativeEventFilter(const QByteArray& eventType, void* message, qintptr* result) override;

private:
    struct RegisteredHotkey {
        int nativeId = 0;
        QString id;
    };

    [[nodiscard]] quint32 qtModifiersToNative(Qt::KeyboardModifiers modifiers) const;
    [[nodiscard]] quint32 qtKeyToNative(Qt::Key key) const;
    [[nodiscard]] QString failureForRequest(const QString& displayName, const QString& reason) const;

    std::function<void(const QString&)> activationHandler_;
    QString lastError_;
    int nextHotkeyId_ = 1;
    QHash<int, RegisteredHotkey> registrations_;
};

}  // namespace cappy::platform::hotkey
