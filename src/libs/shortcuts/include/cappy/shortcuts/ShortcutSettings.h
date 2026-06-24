#pragma once

#include <QList>
#include <QString>

namespace cappy::shortcuts {

struct GlobalShortcutSettings {
    QString openHome = "Ctrl+Shift+Space";
    QString screenshot = "F1";
};

struct MainWindowShortcutSettings {
    QString regionCapture = "Ctrl+Shift+A";
    QString fullscreenCapture = "Ctrl+Shift+F";
    QString activeWindowCapture = "Ctrl+Shift+W";
    QString windowFitCapture;
    QString pinLatest = "Ctrl+Shift+P";
    QString saveLatest = "Ctrl+S";
    QString closePins = "Ctrl+Alt+X";
    QString restorePinInput = "Ctrl+Alt+R";
    QString openCaptureFolder = "Ctrl+O";
    QString settings = "Ctrl+,";
    QString hideToTray = "Ctrl+H";
    QString quit = "Ctrl+Q";
    QString historyPin = "Return";
    QString historyCopy = "Ctrl+C";
    QString historySave = "Ctrl+Shift+S";
    QString historyRemove = "Delete";
};

struct CaptureOverlayShortcutSettings {
    QString rectangle = "1";
    QString ellipse = "2";
    QString arrow = "3";
    QString pen = "4";
    QString marker = "5";
    QString mosaic = "6";
    QString text = "7";
    QString serial = "8";
    QString rectangleAlt = "Ctrl+1";
    QString ellipseAlt = "Ctrl+2";
    QString arrowAlt = "Ctrl+3";
    QString penAlt = "Ctrl+4";
    QString markerAlt = "Ctrl+5";
    QString mosaicAlt = "Ctrl+6";
    QString textAlt = "Ctrl+7";
    QString serialAlt = "Ctrl+8";
    QString undo = "Ctrl+Z";
    QString redo = "Ctrl+Y";
    QString copy = "C";
    QString copyAlt = "Ctrl+C";
    QString quickCopy = "Return";
    QString save = "S";
    QString saveAlt = "Ctrl+S";
    QString pin = "P";
    QString pinAlt = "Ctrl+P";
    QString cancel = "Esc";
};

struct CaptureEditorShortcutSettings {
    QString rectangle = "Ctrl+1";
    QString ellipse = "Ctrl+2";
    QString arrow = "Ctrl+3";
    QString pen = "Ctrl+4";
    QString marker = "Ctrl+5";
    QString mosaic = "Ctrl+6";
    QString text = "Ctrl+7";
    QString serial = "Ctrl+8";
    QString undo = "Ctrl+Z";
    QString redo = "Ctrl+Y";
    QString copy = "Ctrl+C";
    QString copyAndClose = "Return";
    QString save = "Ctrl+S";
    QString saveAlt = "Ctrl+Shift+S";
    QString pin = "P";
    QString pinAlt = "Ctrl+P";
    QString close = "Esc";
};

struct PinWindowShortcutSettings {
    QString close = "Esc";
    QString scaleUp = "=";
    QString scaleDown = "-";
    QString resetScaleAndOpacity = "0";
    QString opacityDown = "[";
    QString opacityUp = "]";
    QString toggleLock = "L";
    QString toggleClickThrough = "T";
};

struct ShortcutSettings {
    GlobalShortcutSettings global;
    MainWindowShortcutSettings mainWindow;
    CaptureOverlayShortcutSettings overlay;
    CaptureEditorShortcutSettings editor;
    PinWindowShortcutSettings pinWindow;
};

enum class ShortcutScope {
    Global,
    MainWindow,
    CaptureOverlay,
    CaptureEditor,
    PinWindow,
};

struct ShortcutFieldDefinition {
    QString id;
    ShortcutScope scope;
    QString label;
    QString defaultSequence;
};

QList<ShortcutFieldDefinition> shortcutFieldDefinitions();
QString shortcutScopeLabel(ShortcutScope scope);
QString shortcutValue(const ShortcutSettings& settings, const QString& id);
bool setShortcutValue(ShortcutSettings* settings, const QString& id, const QString& value);

}  // namespace cappy::shortcuts
