#include "WinGlobalHotkeyBackend.h"

#include <QCoreApplication>
#include <QGuiApplication>

#if defined(Q_OS_WIN)
#include <windows.h>
#endif

namespace cappy::platform::hotkey {

bool WinGlobalHotkeyBackend::canCreate() {
#if defined(Q_OS_WIN)
    return QGuiApplication::platformName().contains("windows", Qt::CaseInsensitive);
#else
    return false;
#endif
}

WinGlobalHotkeyBackend::WinGlobalHotkeyBackend() {
    qApp->installNativeEventFilter(this);
}

WinGlobalHotkeyBackend::~WinGlobalHotkeyBackend() {
    unregisterAll();
    qApp->removeNativeEventFilter(this);
}

QString WinGlobalHotkeyBackend::backendName() const {
    return "win32";
}

bool WinGlobalHotkeyBackend::isSupported() const {
#if defined(Q_OS_WIN)
    return canCreate();
#else
    return false;
#endif
}

QString WinGlobalHotkeyBackend::unsupportedReason() const {
    return isSupported() ? QString() : QStringLiteral("No Win32 event dispatcher is available.");
}

QString WinGlobalHotkeyBackend::lastError() const {
    return lastError_;
}

void WinGlobalHotkeyBackend::setActivationHandler(std::function<void(const QString&)> handler) {
    activationHandler_ = std::move(handler);
}

bool WinGlobalHotkeyBackend::registerHotkey(const GlobalHotkey& hotkey) {
#if defined(Q_OS_WIN)
    lastError_.clear();

    const quint32 nativeModifiers = qtModifiersToNative(hotkey.modifiers);
    const quint32 nativeKey = qtKeyToNative(hotkey.key);
    if (nativeKey == 0) {
        lastError_ =
            failureForRequest(hotkey.displayName, "Could not resolve a Win32 virtual key.");
        return false;
    }

    const int nativeId = nextHotkeyId_++;
    if (!RegisterHotKey(nullptr, nativeId, nativeModifiers | MOD_NOREPEAT, nativeKey)) {
        lastError_ = failureForRequest(hotkey.displayName, "RegisterHotKey failed.");
        return false;
    }

    registrations_.insert(nativeId, RegisteredHotkey{
                                        .nativeId = nativeId,
                                        .id = hotkey.id,
                                    });
    return true;
#else
    Q_UNUSED(hotkey);
    lastError_ = QStringLiteral("Win32 global hotkeys are unavailable on this platform.");
    return false;
#endif
}

void WinGlobalHotkeyBackend::unregisterAll() {
#if defined(Q_OS_WIN)
    for (auto it = registrations_.cbegin(); it != registrations_.cend(); ++it) {
        UnregisterHotKey(nullptr, it.key());
    }
#endif
    registrations_.clear();
}

bool WinGlobalHotkeyBackend::nativeEventFilter(const QByteArray& eventType, void* message,
                                               qintptr* result) {
    Q_UNUSED(result);

#if defined(Q_OS_WIN)
    if (!isSupported() || message == nullptr) {
        return false;
    }

    if (eventType != "windows_generic_MSG" && eventType != "windows_dispatcher_MSG") {
        return false;
    }

    const auto* msg = static_cast<MSG*>(message);
    if (msg->message != WM_HOTKEY) {
        return false;
    }

    const int nativeId = static_cast<int>(msg->wParam);
    const auto it = registrations_.find(nativeId);
    if (it == registrations_.end()) {
        return false;
    }

    if (activationHandler_) {
        activationHandler_(it->id);
    }
    return true;
#else
    Q_UNUSED(eventType);
    Q_UNUSED(message);
    return false;
#endif
}

quint32 WinGlobalHotkeyBackend::qtModifiersToNative(Qt::KeyboardModifiers modifiers) const {
    quint32 nativeModifiers = 0;
#if defined(Q_OS_WIN)
    if (modifiers.testFlag(Qt::ShiftModifier)) {
        nativeModifiers |= MOD_SHIFT;
    }
    if (modifiers.testFlag(Qt::ControlModifier)) {
        nativeModifiers |= MOD_CONTROL;
    }
    if (modifiers.testFlag(Qt::AltModifier)) {
        nativeModifiers |= MOD_ALT;
    }
    if (modifiers.testFlag(Qt::MetaModifier)) {
        nativeModifiers |= MOD_WIN;
    }
#else
    Q_UNUSED(modifiers);
#endif
    return nativeModifiers;
}

quint32 WinGlobalHotkeyBackend::qtKeyToNative(Qt::Key key) const {
#if defined(Q_OS_WIN)
    if (key >= Qt::Key_A && key <= Qt::Key_Z) {
        return static_cast<quint32>('A' + (key - Qt::Key_A));
    }
    if (key >= Qt::Key_0 && key <= Qt::Key_9) {
        return static_cast<quint32>('0' + (key - Qt::Key_0));
    }
    if (key >= Qt::Key_F1 && key <= Qt::Key_F24) {
        return static_cast<quint32>(VK_F1 + (key - Qt::Key_F1));
    }

    switch (key) {
    case Qt::Key_Print:
        return VK_SNAPSHOT;
    case Qt::Key_Escape:
        return VK_ESCAPE;
    case Qt::Key_Tab:
        return VK_TAB;
    case Qt::Key_Space:
        return VK_SPACE;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        return VK_RETURN;
    case Qt::Key_Left:
        return VK_LEFT;
    case Qt::Key_Right:
        return VK_RIGHT;
    case Qt::Key_Up:
        return VK_UP;
    case Qt::Key_Down:
        return VK_DOWN;
    case Qt::Key_Insert:
        return VK_INSERT;
    case Qt::Key_Delete:
        return VK_DELETE;
    case Qt::Key_Home:
        return VK_HOME;
    case Qt::Key_End:
        return VK_END;
    case Qt::Key_PageUp:
        return VK_PRIOR;
    case Qt::Key_PageDown:
        return VK_NEXT;
    default:
        return 0;
    }
#else
    Q_UNUSED(key);
    return 0;
#endif
}

QString WinGlobalHotkeyBackend::failureForRequest(const QString& displayName,
                                                  const QString& reason) const {
    return QString("%1: %2").arg(displayName, reason);
}

} // namespace cappy::platform::hotkey
