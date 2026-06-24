#include "cappy/shortcuts/ShortcutSettings.h"

namespace cappy::shortcuts {

namespace {

#define CAPPY_SHORTCUT_FIELD_LIST(X) \
    X("global.open_home", ShortcutScope::Global, "Open Cappy", "Ctrl+Shift+Space", global.openHome) \
    X("global.screenshot", ShortcutScope::Global, "Screenshot", "F1", global.screenshot) \
    X("main.region_capture", ShortcutScope::MainWindow, "Region Capture", "Ctrl+Shift+A", mainWindow.regionCapture) \
    X("main.fullscreen_capture", ShortcutScope::MainWindow, "Fullscreen Capture", "Ctrl+Shift+F", mainWindow.fullscreenCapture) \
    X("main.active_window_capture", ShortcutScope::MainWindow, "Active Window Capture", "Ctrl+Shift+W", mainWindow.activeWindowCapture) \
    X("main.pin_latest", ShortcutScope::MainWindow, "Pin Last Capture", "Ctrl+Shift+P", mainWindow.pinLatest) \
    X("main.save_latest", ShortcutScope::MainWindow, "Save Last Capture", "Ctrl+S", mainWindow.saveLatest) \
    X("main.close_pins", ShortcutScope::MainWindow, "Close Pins", "Ctrl+Alt+X", mainWindow.closePins) \
    X("main.restore_pin_input", ShortcutScope::MainWindow, "Restore Pin Input", "Ctrl+Alt+R", mainWindow.restorePinInput) \
    X("main.open_capture_folder", ShortcutScope::MainWindow, "Open Capture Folder", "Ctrl+O", mainWindow.openCaptureFolder) \
    X("main.settings", ShortcutScope::MainWindow, "Settings", "Ctrl+,", mainWindow.settings) \
    X("main.hide_to_tray", ShortcutScope::MainWindow, "Hide to Tray", "Ctrl+H", mainWindow.hideToTray) \
    X("main.quit", ShortcutScope::MainWindow, "Quit", "Ctrl+Q", mainWindow.quit) \
    X("main.history_pin", ShortcutScope::MainWindow, "History: Pin Selected", "Return", mainWindow.historyPin) \
    X("main.history_copy", ShortcutScope::MainWindow, "History: Copy Selected", "Ctrl+C", mainWindow.historyCopy) \
    X("main.history_save", ShortcutScope::MainWindow, "History: Save Selected", "Ctrl+Shift+S", mainWindow.historySave) \
    X("main.history_remove", ShortcutScope::MainWindow, "History: Remove Selected", "Delete", mainWindow.historyRemove) \
    X("overlay.rectangle", ShortcutScope::CaptureOverlay, "Tool: Rectangle", "1", overlay.rectangle) \
    X("overlay.ellipse", ShortcutScope::CaptureOverlay, "Tool: Ellipse", "2", overlay.ellipse) \
    X("overlay.arrow", ShortcutScope::CaptureOverlay, "Tool: Arrow", "3", overlay.arrow) \
    X("overlay.pen", ShortcutScope::CaptureOverlay, "Tool: Pen", "4", overlay.pen) \
    X("overlay.marker", ShortcutScope::CaptureOverlay, "Tool: Marker", "5", overlay.marker) \
    X("overlay.mosaic", ShortcutScope::CaptureOverlay, "Tool: Mosaic", "6", overlay.mosaic) \
    X("overlay.text", ShortcutScope::CaptureOverlay, "Tool: Text", "7", overlay.text) \
    X("overlay.serial", ShortcutScope::CaptureOverlay, "Tool: Serial", "8", overlay.serial) \
    X("overlay.rectangle_alt", ShortcutScope::CaptureOverlay, "Tool: Rectangle Alt", "Ctrl+1", overlay.rectangleAlt) \
    X("overlay.ellipse_alt", ShortcutScope::CaptureOverlay, "Tool: Ellipse Alt", "Ctrl+2", overlay.ellipseAlt) \
    X("overlay.arrow_alt", ShortcutScope::CaptureOverlay, "Tool: Arrow Alt", "Ctrl+3", overlay.arrowAlt) \
    X("overlay.pen_alt", ShortcutScope::CaptureOverlay, "Tool: Pen Alt", "Ctrl+4", overlay.penAlt) \
    X("overlay.marker_alt", ShortcutScope::CaptureOverlay, "Tool: Marker Alt", "Ctrl+5", overlay.markerAlt) \
    X("overlay.mosaic_alt", ShortcutScope::CaptureOverlay, "Tool: Mosaic Alt", "Ctrl+6", overlay.mosaicAlt) \
    X("overlay.text_alt", ShortcutScope::CaptureOverlay, "Tool: Text Alt", "Ctrl+7", overlay.textAlt) \
    X("overlay.serial_alt", ShortcutScope::CaptureOverlay, "Tool: Serial Alt", "Ctrl+8", overlay.serialAlt) \
    X("overlay.undo", ShortcutScope::CaptureOverlay, "Undo", "Ctrl+Z", overlay.undo) \
    X("overlay.redo", ShortcutScope::CaptureOverlay, "Redo", "Ctrl+Y", overlay.redo) \
    X("overlay.copy", ShortcutScope::CaptureOverlay, "Copy", "C", overlay.copy) \
    X("overlay.copy_alt", ShortcutScope::CaptureOverlay, "Copy Alt", "Ctrl+C", overlay.copyAlt) \
    X("overlay.quick_copy", ShortcutScope::CaptureOverlay, "Quick Copy", "Return", overlay.quickCopy) \
    X("overlay.save", ShortcutScope::CaptureOverlay, "Save", "S", overlay.save) \
    X("overlay.save_alt", ShortcutScope::CaptureOverlay, "Save Alt", "Ctrl+S", overlay.saveAlt) \
    X("overlay.pin", ShortcutScope::CaptureOverlay, "Pin", "P", overlay.pin) \
    X("overlay.pin_alt", ShortcutScope::CaptureOverlay, "Pin Alt", "Ctrl+P", overlay.pinAlt) \
    X("overlay.cancel", ShortcutScope::CaptureOverlay, "Cancel", "Esc", overlay.cancel) \
    X("editor.rectangle", ShortcutScope::CaptureEditor, "Tool: Rectangle", "Ctrl+1", editor.rectangle) \
    X("editor.ellipse", ShortcutScope::CaptureEditor, "Tool: Ellipse", "Ctrl+2", editor.ellipse) \
    X("editor.arrow", ShortcutScope::CaptureEditor, "Tool: Arrow", "Ctrl+3", editor.arrow) \
    X("editor.pen", ShortcutScope::CaptureEditor, "Tool: Pen", "Ctrl+4", editor.pen) \
    X("editor.marker", ShortcutScope::CaptureEditor, "Tool: Marker", "Ctrl+5", editor.marker) \
    X("editor.mosaic", ShortcutScope::CaptureEditor, "Tool: Mosaic", "Ctrl+6", editor.mosaic) \
    X("editor.text", ShortcutScope::CaptureEditor, "Tool: Text", "Ctrl+7", editor.text) \
    X("editor.serial", ShortcutScope::CaptureEditor, "Tool: Serial", "Ctrl+8", editor.serial) \
    X("editor.undo", ShortcutScope::CaptureEditor, "Undo", "Ctrl+Z", editor.undo) \
    X("editor.redo", ShortcutScope::CaptureEditor, "Redo", "Ctrl+Y", editor.redo) \
    X("editor.copy", ShortcutScope::CaptureEditor, "Copy", "Ctrl+C", editor.copy) \
    X("editor.copy_and_close", ShortcutScope::CaptureEditor, "Copy And Close", "Return", editor.copyAndClose) \
    X("editor.save", ShortcutScope::CaptureEditor, "Save", "Ctrl+S", editor.save) \
    X("editor.save_alt", ShortcutScope::CaptureEditor, "Save Alt", "Ctrl+Shift+S", editor.saveAlt) \
    X("editor.pin", ShortcutScope::CaptureEditor, "Pin", "P", editor.pin) \
    X("editor.pin_alt", ShortcutScope::CaptureEditor, "Pin Alt", "Ctrl+P", editor.pinAlt) \
    X("editor.close", ShortcutScope::CaptureEditor, "Close", "Esc", editor.close) \
    X("pin.close", ShortcutScope::PinWindow, "Close", "Esc", pinWindow.close) \
    X("pin.scale_up", ShortcutScope::PinWindow, "Scale Up", "=", pinWindow.scaleUp) \
    X("pin.scale_down", ShortcutScope::PinWindow, "Scale Down", "-", pinWindow.scaleDown) \
    X("pin.reset_scale_opacity", ShortcutScope::PinWindow, "Reset Scale And Opacity", "0", pinWindow.resetScaleAndOpacity) \
    X("pin.opacity_down", ShortcutScope::PinWindow, "Opacity Down", "[", pinWindow.opacityDown) \
    X("pin.opacity_up", ShortcutScope::PinWindow, "Opacity Up", "]", pinWindow.opacityUp) \
    X("pin.toggle_lock", ShortcutScope::PinWindow, "Toggle Lock", "L", pinWindow.toggleLock) \
    X("pin.toggle_click_through", ShortcutScope::PinWindow, "Toggle Click-Through", "T", pinWindow.toggleClickThrough)

}  // namespace

QList<ShortcutFieldDefinition> shortcutFieldDefinitions() {
    QList<ShortcutFieldDefinition> definitions;
#define CAPPY_ADD_SHORTCUT_DEFINITION(id, scope, label, defaultSequence, field) \
    definitions.push_back(ShortcutFieldDefinition{ \
        QStringLiteral(id), \
        scope, \
        QStringLiteral(label), \
        QStringLiteral(defaultSequence), \
    });
    CAPPY_SHORTCUT_FIELD_LIST(CAPPY_ADD_SHORTCUT_DEFINITION)
#undef CAPPY_ADD_SHORTCUT_DEFINITION
    return definitions;
}

QString shortcutScopeLabel(ShortcutScope scope) {
    switch (scope) {
    case ShortcutScope::Global:
        return "Global";
    case ShortcutScope::MainWindow:
        return "Main Window";
    case ShortcutScope::CaptureOverlay:
        return "Capture Overlay";
    case ShortcutScope::CaptureEditor:
        return "Capture Editor";
    case ShortcutScope::PinWindow:
        return "Pinned Window";
    }

    return "Shortcuts";
}

QString shortcutValue(const ShortcutSettings& settings, const QString& id) {
#define CAPPY_GET_SHORTCUT_VALUE(shortcutId, scope, label, defaultSequence, field) \
    if (id == QLatin1String(shortcutId)) { \
        return settings.field; \
    }
    CAPPY_SHORTCUT_FIELD_LIST(CAPPY_GET_SHORTCUT_VALUE)
#undef CAPPY_GET_SHORTCUT_VALUE
    return {};
}

bool setShortcutValue(ShortcutSettings* settings, const QString& id, const QString& value) {
    if (settings == nullptr) {
        return false;
    }

#define CAPPY_SET_SHORTCUT_VALUE(shortcutId, scope, label, defaultSequence, field) \
    if (id == QLatin1String(shortcutId)) { \
        settings->field = value; \
        return true; \
    }
    CAPPY_SHORTCUT_FIELD_LIST(CAPPY_SET_SHORTCUT_VALUE)
#undef CAPPY_SET_SHORTCUT_VALUE
    return false;
}

}  // namespace cappy::shortcuts
