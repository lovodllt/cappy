#include "X11GlobalHotkeyBackend.h"

#include <utility>

#include <QCoreApplication>
#include <QGuiApplication>

#include <QtGui/qguiapplication_platform.h>
#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <xcb/xcb.h>

namespace cappy::platform::hotkey {

namespace {

constexpr uint16_t kRelevantModifierMask =
    XCB_MOD_MASK_SHIFT
    | XCB_MOD_MASK_CONTROL
    | XCB_MOD_MASK_1
    | XCB_MOD_MASK_4
    | XCB_MOD_MASK_LOCK;

}  // namespace

bool X11GlobalHotkeyBackend::canCreate() {
    return QGuiApplication::platformName() == "xcb";
}

X11GlobalHotkeyBackend::X11GlobalHotkeyBackend() {
    if (initializeNativeHandles()) {
        QCoreApplication::instance()->installNativeEventFilter(this);
        eventFilterInstalled_ = true;
    }
}

X11GlobalHotkeyBackend::~X11GlobalHotkeyBackend() {
    unregisterAll();
    if (eventFilterInstalled_ && QCoreApplication::instance() != nullptr) {
        QCoreApplication::instance()->removeNativeEventFilter(this);
    }
}

QString X11GlobalHotkeyBackend::backendName() const {
    return "x11-global-hotkey";
}

bool X11GlobalHotkeyBackend::isSupported() const {
    return display_ != nullptr && connection_ != nullptr && rootWindow_ != XCB_WINDOW_NONE;
}

QString X11GlobalHotkeyBackend::unsupportedReason() const {
    if (isSupported()) {
        return {};
    }

    return "No X11 display or root window is available.";
}

QString X11GlobalHotkeyBackend::lastError() const {
    return lastError_;
}

void X11GlobalHotkeyBackend::setActivationHandler(std::function<void(const QString&)> handler) {
    activationHandler_ = std::move(handler);
}

bool X11GlobalHotkeyBackend::registerHotkey(const GlobalHotkey& hotkey) {
    if (!isSupported()) {
        lastError_ = unsupportedReason();
        return false;
    }

    const auto existing = hotkeys_.find(hotkey.id.toStdString());
    if (existing != hotkeys_.end()) {
        unregisterHotkey(existing->second);
        hotkeys_.erase(existing);
    }

    const unsigned long keysym = qtKeyToKeysym(hotkey.key);
    if (keysym == NoSymbol) {
        lastError_ = failureForRequest(hotkey.displayName, "Unsupported key.");
        return false;
    }

    const KeyCode keycode = XKeysymToKeycode(display_, keysym);
    if (keycode == 0) {
        lastError_ = failureForRequest(hotkey.displayName, "Could not resolve an X11 keycode.");
        return false;
    }

    refreshNumLockMask();

    RegisteredHotkey registration;
    registration.id = hotkey.id;
    registration.keycode = static_cast<uint8_t>(keycode);
    registration.modifiers = qtModifiersToX11(hotkey.modifiers);
    registration.grabModifiers = {
        registration.modifiers,
        static_cast<uint16_t>(registration.modifiers | XCB_MOD_MASK_LOCK),
        static_cast<uint16_t>(registration.modifiers | numLockMask_),
        static_cast<uint16_t>(registration.modifiers | numLockMask_ | XCB_MOD_MASK_LOCK),
    };

    for (uint16_t grabModifiers : registration.grabModifiers) {
        const xcb_void_cookie_t cookie = xcb_grab_key_checked(
            connection_,
            1,
            rootWindow_,
            grabModifiers,
            registration.keycode,
            XCB_GRAB_MODE_ASYNC,
            XCB_GRAB_MODE_ASYNC
        );
        xcb_generic_error_t* error = xcb_request_check(connection_, cookie);
        if (error != nullptr) {
            free(error);
            unregisterHotkey(registration);
            lastError_ = failureForRequest(
                hotkey.displayName,
                "The key combination is already in use."
            );
            return false;
        }
    }

    xcb_flush(connection_);
    hotkeys_.insert_or_assign(hotkey.id.toStdString(), registration);
    lastError_.clear();
    return true;
}

void X11GlobalHotkeyBackend::unregisterAll() {
    if (connection_ == nullptr) {
        hotkeys_.clear();
        return;
    }

    for (const auto& [id, hotkey] : hotkeys_) {
        Q_UNUSED(id);
        unregisterHotkey(hotkey);
    }

    hotkeys_.clear();
    xcb_flush(connection_);
}

bool X11GlobalHotkeyBackend::nativeEventFilter(
    const QByteArray& eventType,
    void* message,
    qintptr* result
) {
    Q_UNUSED(result);

    if (eventType != "xcb_generic_event_t" || message == nullptr) {
        return false;
    }

    auto* event = static_cast<xcb_generic_event_t*>(message);
    if ((event->response_type & ~0x80) != XCB_KEY_PRESS) {
        return false;
    }

    auto* keyEvent = reinterpret_cast<xcb_key_press_event_t*>(event);
    const uint16_t normalizedState = normalizedEventState(keyEvent->state);
    for (const auto& [id, hotkey] : hotkeys_) {
        if (hotkey.keycode == keyEvent->detail && hotkey.modifiers == normalizedState) {
            if (activationHandler_) {
                activationHandler_(hotkey.id);
            }
            return true;
        }
    }

    return false;
}

bool X11GlobalHotkeyBackend::initializeNativeHandles() {
    auto* nativeApp = qGuiApp->nativeInterface<QNativeInterface::QX11Application>();
    if (nativeApp == nullptr) {
        return false;
    }

    display_ = nativeApp->display();
    connection_ = nativeApp->connection();
    if (display_ == nullptr || connection_ == nullptr) {
        return false;
    }

    const xcb_setup_t* setup = xcb_get_setup(connection_);
    xcb_screen_iterator_t screenIterator = xcb_setup_roots_iterator(setup);
    if (screenIterator.rem == 0 || screenIterator.data == nullptr) {
        return false;
    }

    rootWindow_ = screenIterator.data->root;
    refreshNumLockMask();
    return rootWindow_ != XCB_WINDOW_NONE;
}

void X11GlobalHotkeyBackend::refreshNumLockMask() {
    numLockMask_ = 0;
    if (display_ == nullptr) {
        return;
    }

    XModifierKeymap* modifierKeymap = XGetModifierMapping(display_);
    if (modifierKeymap == nullptr) {
        return;
    }

    const KeyCode numLockKeycode = XKeysymToKeycode(display_, XK_Num_Lock);
    if (numLockKeycode != 0) {
        for (int modifierIndex = 0; modifierIndex < 8; ++modifierIndex) {
            for (int keyIndex = 0; keyIndex < modifierKeymap->max_keypermod; ++keyIndex) {
                const int offset = modifierIndex * modifierKeymap->max_keypermod + keyIndex;
                if (modifierKeymap->modifiermap[offset] == numLockKeycode) {
                    numLockMask_ = static_cast<uint16_t>(1u << modifierIndex);
                    break;
                }
            }
            if (numLockMask_ != 0) {
                break;
            }
        }
    }

    XFreeModifiermap(modifierKeymap);
}

void X11GlobalHotkeyBackend::unregisterHotkey(const RegisteredHotkey& hotkey) {
    if (connection_ == nullptr) {
        return;
    }

    for (uint16_t grabModifiers : hotkey.grabModifiers) {
        xcb_ungrab_key(connection_, hotkey.keycode, rootWindow_, grabModifiers);
    }
}

uint16_t X11GlobalHotkeyBackend::qtModifiersToX11(Qt::KeyboardModifiers modifiers) const {
    uint16_t nativeModifiers = 0;
    if (modifiers.testFlag(Qt::ShiftModifier)) {
        nativeModifiers |= XCB_MOD_MASK_SHIFT;
    }
    if (modifiers.testFlag(Qt::ControlModifier)) {
        nativeModifiers |= XCB_MOD_MASK_CONTROL;
    }
    if (modifiers.testFlag(Qt::AltModifier)) {
        nativeModifiers |= XCB_MOD_MASK_1;
    }
    if (modifiers.testFlag(Qt::MetaModifier)) {
        nativeModifiers |= XCB_MOD_MASK_4;
    }
    return nativeModifiers;
}

unsigned long X11GlobalHotkeyBackend::qtKeyToKeysym(Qt::Key key) const {
    if (key >= Qt::Key_A && key <= Qt::Key_Z) {
        return static_cast<unsigned long>(XK_A + (key - Qt::Key_A));
    }
    if (key >= Qt::Key_0 && key <= Qt::Key_9) {
        return static_cast<unsigned long>(XK_0 + (key - Qt::Key_0));
    }
    if (key >= Qt::Key_F1 && key <= Qt::Key_F12) {
        return static_cast<unsigned long>(XK_F1 + (key - Qt::Key_F1));
    }

    switch (key) {
    case Qt::Key_Space:
        return XK_space;
    case Qt::Key_Print:
        return XK_Print;
    default:
        return NoSymbol;
    }
}

uint16_t X11GlobalHotkeyBackend::normalizedEventState(uint16_t state) const {
    const uint16_t effectiveMask = static_cast<uint16_t>(kRelevantModifierMask | numLockMask_);
    return static_cast<uint16_t>(state & effectiveMask & ~(XCB_MOD_MASK_LOCK | numLockMask_));
}

QString X11GlobalHotkeyBackend::failureForRequest(
    const QString& displayName,
    const QString& reason
) {
    return QString("%1: %2").arg(displayName, reason);
}

}  // namespace cappy::platform::hotkey
