#pragma once

#include <memory>
#include <optional>

#include <QImage>
#include <QObject>
#include <QPointer>
#include <QRect>
#include <QString>

#include "AppCommand.h"
#include "AppSettings.h"
#include "cappy/domain/capture/CaptureTypes.h"
#include "cappy/features/capture/CaptureOverlayWidget.h"
#include "cappy/localization/Localization.h"

class QApplication;
class QAction;
class QMenu;
class QSystemTrayIcon;

class MainWindow;

namespace cappy::services::pinboard {
class PinboardManager;
}

namespace cappy::services::capture {
class CaptureCoordinator;
}

namespace cappy::services::hotkey {
class GlobalHotkeyService;
}

namespace cappy::features::editor {
class CaptureEditorWindow;
}

namespace cappy::features::ocr {
class OcrResultWindow;
}

class AppController final : public QObject {
    Q_OBJECT

public:
    AppController(
        QApplication& app,
        MainWindow& window,
        AppSettings& settings,
        QString logFilePath,
        QObject* parent = nullptr
    );
    ~AppController() override;

    void initialize();
    void persistShellState();
    void runFullscreenCaptureSmokeTest();
    void runActiveWindowCaptureSmokeTest();
    void runPinLatestCaptureSmokeTest();

private:
    void dispatch(AppCommand command);
    void setupTray();
    void refreshTrayTexts();
    void setupWindow();
    void setupGlobalHotkeys();
    void restoreShellState();
    void showMainWindow();
    void hideToTray();
    void closeActiveEditorWindow();
    void syncLatestCaptureImage(const QImage& image, bool clearSavedState = false);
    void rememberCaptureResult(const cappy::domain::capture::CaptureResult& result);
    void setGlobalHotkeysSuspended(bool suspended);
    [[nodiscard]] QString saveCaptureToDefaultDirectory(
        const QImage& image,
        const QString& label
    ) const;
    void pinLatestCapture();
    void saveLatestCapture();
    void pinCaptureFromHistory(const QImage& image);
    void saveHistoryCapture(const QString& entryId, const QImage& image, int captureMode);
    void copyCaptureToClipboard(const QImage& image);
    void closeAllPins();
    void setPinsClickThrough(bool enabled);
    void openCapturesDirectory();
    void openSettingsDialog();
    void restartApplication();
    void startCurrentScreenCapture();
    void startFullscreenCapture();
    void startActiveWindowCapture();
    void startWindowFitCapture();
    void startRegionCapture();
    void onCaptureCompleted(const cappy::domain::capture::CaptureResult& result);
    void onCaptureFinalized(
        const cappy::domain::capture::CaptureResult& result,
        cappy::features::capture::CaptureFinalizeAction action
    );
    void openOcrWindow(const QImage& image);
    void onCaptureFailed(const QString& message);
    void onCaptureCanceled();
    void quitApplication();

    QApplication& app_;
    MainWindow& window_;
    AppSettings& settings_;
    QString logFilePath_;
    AppSettings::ShellSettings shellSettings_;
    cappy::localization::AppLanguage currentLanguage_ = cappy::localization::AppLanguage::English;
    std::optional<cappy::domain::capture::CaptureMode> lastCaptureMode_;
    std::optional<QRect> lastCaptureGeometry_;
    QImage lastCapturedImage_;
    QString lastCaptureHistoryEntryId_;
    bool wasWindowVisibleBeforeCapture_ = false;
    bool exitAfterNextCaptureEvent_ = false;
    bool pinAfterNextCaptureEvent_ = false;
    bool settingsDialogOpen_ = false;
    QPointer<cappy::features::editor::CaptureEditorWindow> activeEditorWindow_;
    std::unique_ptr<QSystemTrayIcon> trayIcon_;
    std::unique_ptr<QMenu> trayMenu_;
    QAction* trayRegionCaptureAction_ = nullptr;
    QAction* trayCurrentScreenCaptureAction_ = nullptr;
    QAction* trayFullscreenCaptureAction_ = nullptr;
    QAction* traySettingsAction_ = nullptr;
    QAction* trayRestartAction_ = nullptr;
    QAction* trayQuitAction_ = nullptr;
    std::unique_ptr<cappy::services::capture::CaptureCoordinator> captureCoordinator_;
    std::unique_ptr<cappy::services::hotkey::GlobalHotkeyService> hotkeyService_;
    std::unique_ptr<cappy::services::pinboard::PinboardManager> pinboardManager_;
};
