#pragma once

#include <functional>
#include <unordered_map>
#include <vector>

#include <QAbstractNativeEventFilter>
#include <QObject>
#include <QString>

#include "cappy/platform/hotkey/IGlobalHotkeyBackend.h"

struct _XDisplay;
typedef struct xcb_connection_t xcb_connection_t;
typedef uint32_t xcb_window_t;

namespace cappy::platform::hotkey {

class X11GlobalHotkeyBackend final : public QObject,
                                     public IGlobalHotkeyBackend,
                                     public QAbstractNativeEventFilter {
  public:
    static bool canCreate();

    X11GlobalHotkeyBackend();
    ~X11GlobalHotkeyBackend() override;

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
        QString id;
        uint8_t keycode = 0;
        uint16_t modifiers = 0;
        std::vector<uint16_t> grabModifiers;
    };

    bool initializeNativeHandles();
    void refreshNumLockMask();
    void unregisterHotkey(const RegisteredHotkey& hotkey);
    uint16_t qtModifiersToX11(Qt::KeyboardModifiers modifiers) const;
    unsigned long qtKeyToKeysym(Qt::Key key) const;
    uint16_t normalizedEventState(uint16_t state) const;
    QString failureForRequest(const QString& displayName, const QString& reason);

    std::function<void(const QString&)> activationHandler_;
    QString lastError_;
    _XDisplay* display_ = nullptr;
    xcb_connection_t* connection_ = nullptr;
    xcb_window_t rootWindow_ = 0;
    uint16_t numLockMask_ = 0;
    bool eventFilterInstalled_ = false;
    std::unordered_map<std::string, RegisteredHotkey> hotkeys_;
};

} // namespace cappy::platform::hotkey
