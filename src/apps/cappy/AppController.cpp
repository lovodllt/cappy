#include "AppController.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QDateTime>
#include <QDesktopServices>
#include <QDebug>
#include <QDir>
#include <QImage>
#include <QIcon>
#include <QCoreApplication>
#include <QKeySequence>
#include <QMenu>
#include <QProcess>
#include <QStyle>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QUrl>
#include <QUuid>

#include <optional>
#include <utility>

#include "MainWindow.h"
#include "SettingsDialog.h"
#include "cappy/domain/capture/CaptureTypes.h"
#include "cappy/features/capture/CaptureOverlayWidget.h"
#include "cappy/features/editor/CaptureEditorWindow.h"
#include "cappy/features/ocr/OcrResultWindow.h"
#include "cappy/services/capture/CaptureCoordinator.h"
#include "cappy/services/hotkey/GlobalHotkeyService.h"
#include "cappy/services/pinboard/PinboardManager.h"

namespace {

constexpr auto kShowHomeHotkeyId = "show_home";
constexpr auto kRegionCaptureHotkeyId = "capture_region";

QString captureModeDisplayLabel(
    cappy::localization::AppLanguage language,
    cappy::domain::capture::CaptureMode mode
) {
    const auto& text = cappy::localization::strings(language);
    switch (mode) {
    case cappy::domain::capture::CaptureMode::Region:
        return text.captureModeRegion;
    case cappy::domain::capture::CaptureMode::CurrentScreen:
        return text.captureModeCurrentScreen;
    case cappy::domain::capture::CaptureMode::Fullscreen:
        return text.captureModeFullscreen;
    case cappy::domain::capture::CaptureMode::ActiveWindow:
        return text.captureModeActiveWindow;
    case cappy::domain::capture::CaptureMode::WindowFit:
        return text.captureModeWindowFit;
    }

    return text.captureModeUnknown;
}

QString captureModeFileLabel(cappy::domain::capture::CaptureMode mode) {
    switch (mode) {
    case cappy::domain::capture::CaptureMode::Region:
        return "region";
    case cappy::domain::capture::CaptureMode::CurrentScreen:
        return "screen";
    case cappy::domain::capture::CaptureMode::Fullscreen:
        return "global";
    case cappy::domain::capture::CaptureMode::ActiveWindow:
        return "window";
    case cappy::domain::capture::CaptureMode::WindowFit:
        return "window-fit";
    }

    return "capture";
}

std::optional<cappy::domain::capture::CaptureMode> captureModeFromValue(int captureModeValue) {
    switch (captureModeValue) {
    case 0:
        return cappy::domain::capture::CaptureMode::Region;
    case 1:
        return cappy::domain::capture::CaptureMode::Fullscreen;
    case 2:
        return cappy::domain::capture::CaptureMode::ActiveWindow;
    case 3:
        return cappy::domain::capture::CaptureMode::CurrentScreen;
    case 4:
        return cappy::domain::capture::CaptureMode::WindowFit;
    default:
        return std::nullopt;
    }
}

QKeySequence shortcutFromSettings(const QString& text, const QKeySequence& fallback) {
    const QKeySequence sequence = QKeySequence::fromString(text, QKeySequence::PortableText);
    return sequence.isEmpty() ? fallback : sequence;
}

cappy::platform::hotkey::GlobalHotkey globalHotkeyFromSequence(
    QString id,
    QString displayName,
    const QKeySequence& sequence
) {
    const int shortcutValue = sequence[0].toCombined();
    return cappy::platform::hotkey::GlobalHotkey{
        .id = std::move(id),
        .displayName = std::move(displayName),
        .key = static_cast<Qt::Key>(shortcutValue & ~Qt::KeyboardModifierMask),
        .modifiers = static_cast<Qt::KeyboardModifiers>(shortcutValue & Qt::KeyboardModifierMask),
    };
}

QList<cappy::platform::hotkey::GlobalHotkey> globalHotkeysFromSettings(
    const AppSettings::ShellSettings& settings,
    cappy::localization::AppLanguage language
) {
    QList<cappy::platform::hotkey::GlobalHotkey> bindings;
    const auto& text = cappy::localization::strings(language);

    const QKeySequence openHome = shortcutFromSettings(
        settings.shortcuts.global.openHome,
        QKeySequence{}
    );
    if (!openHome.isEmpty()) {
        bindings.push_back(globalHotkeyFromSequence(
            kShowHomeHotkeyId,
            text.actionOpenCappy,
            openHome
        ));
    }

    const QKeySequence screenshot = shortcutFromSettings(
        settings.shortcuts.global.screenshot,
        QKeySequence{}
    );
    if (!screenshot.isEmpty()) {
        bindings.push_back(globalHotkeyFromSequence(
            kRegionCaptureHotkeyId,
            text.actionScreenshot,
            screenshot
        ));
    }

    return bindings;
}

}  // namespace

AppController::AppController(
    QApplication& app,
    MainWindow& window,
    AppSettings& settings,
    QString logFilePath,
    QObject* parent
)
    : QObject(parent)
    , app_(app)
    , window_(window)
    , settings_(settings)
    , logFilePath_(std::move(logFilePath))
    , captureCoordinator_(std::make_unique<cappy::services::capture::CaptureCoordinator>(this))
    , hotkeyService_(std::make_unique<cappy::services::hotkey::GlobalHotkeyService>(this))
    , pinboardManager_(std::make_unique<cappy::services::pinboard::PinboardManager>(this)) {
}

AppController::~AppController() = default;

void AppController::initialize() {
    shellSettings_ = settings_.loadShellSettings();
    currentLanguage_ = cappy::localization::resolvedAppLanguageFromSettings(
        shellSettings_.interfaceLanguage
    );

    setupWindow();
    setupTray();
    setupGlobalHotkeys();
    restoreShellState();

    qInfo() << "App shell initialized. log file:" << logFilePath_;
    qInfo() << "Capture stack:" << captureCoordinator_->backendSummary();
    if (hotkeyService_ != nullptr) {
        qInfo() << "Hotkey stack:" << hotkeyService_->backendSummary();
        qInfo() << "Hotkeys:" << hotkeyService_->bindingsSummary();
        for (const QString& error : hotkeyService_->lastRegistrationErrors()) {
            qWarning() << "Global hotkey registration:" << error;
        }
    }
}

void AppController::persistShellState() {
    shellSettings_.mainWindowGeometry = window_.saveGeometry();
    settings_.saveShellSettings(shellSettings_);
}

void AppController::runFullscreenCaptureSmokeTest() {
    exitAfterNextCaptureEvent_ = true;
    startFullscreenCapture();
}

void AppController::runActiveWindowCaptureSmokeTest() {
    exitAfterNextCaptureEvent_ = true;
    startActiveWindowCapture();
}

void AppController::runPinLatestCaptureSmokeTest() {
    exitAfterNextCaptureEvent_ = true;
    pinAfterNextCaptureEvent_ = true;
    startFullscreenCapture();
}

void AppController::dispatch(AppCommand command) {
    if (settingsDialogOpen_) {
        return;
    }

    switch (command) {
    case AppCommand::ShowHome:
        showMainWindow();
        break;
    case AppCommand::StartRegionCapture:
        startRegionCapture();
        break;
    case AppCommand::StartCurrentScreenCapture:
        startCurrentScreenCapture();
        break;
    case AppCommand::StartFullscreenCapture:
        startFullscreenCapture();
        break;
    case AppCommand::StartActiveWindowCapture:
        startActiveWindowCapture();
        break;
    case AppCommand::StartWindowFitCapture:
        startWindowFitCapture();
        break;
    case AppCommand::PinLatestCapture:
        pinLatestCapture();
        break;
    case AppCommand::SaveLatestCapture:
        saveLatestCapture();
        break;
    case AppCommand::CloseAllPins:
        closeAllPins();
        break;
    case AppCommand::EnablePinClickThrough:
        setPinsClickThrough(true);
        break;
    case AppCommand::RestorePinInput:
        setPinsClickThrough(false);
        break;
    case AppCommand::OpenCapturesDirectory:
        openCapturesDirectory();
        break;
    case AppCommand::OpenSettings:
        openSettingsDialog();
        break;
    case AppCommand::Restart:
        restartApplication();
        break;
    case AppCommand::HideToTray:
        hideToTray();
        break;
    case AppCommand::Quit:
        quitApplication();
        break;
    }
}

void AppController::setupTray() {
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        qWarning() << "System tray is not available. Running without tray integration.";
        window_.setCloseToTrayEnabled(false);
        return;
    }

    trayMenu_ = std::make_unique<QMenu>();
    trayIcon_ = std::make_unique<QSystemTrayIcon>();

    const QIcon icon = app_.style()->standardIcon(QStyle::SP_DesktopIcon);

    trayIcon_->setIcon(icon);
    window_.setWindowIcon(icon);

    auto addAction = [this](const QString& text, AppCommand command) -> QAction* {
        QAction* action = trayMenu_->addAction(text);
        connect(action, &QAction::triggered, this, [this, command]() {
            dispatch(command);
        });
        return action;
    };

    const auto& text = cappy::localization::strings(currentLanguage_);
    trayRegionCaptureAction_ = addAction(text.actionScreenshot, AppCommand::StartRegionCapture);
    trayCurrentScreenCaptureAction_ = addAction(
        text.actionCurrentScreenCapture,
        AppCommand::StartCurrentScreenCapture
    );
    trayFullscreenCaptureAction_ = addAction(text.actionFullscreenCapture, AppCommand::StartFullscreenCapture);
    traySettingsAction_ = addAction(text.actionSettings, AppCommand::OpenSettings);
    trayRestartAction_ = addAction(text.actionRestart, AppCommand::Restart);
    trayQuitAction_ = addAction(text.actionQuit, AppCommand::Quit);

    trayIcon_->setContextMenu(trayMenu_.get());
    connect(
        trayIcon_.get(),
        &QSystemTrayIcon::activated,
        this,
        [this](QSystemTrayIcon::ActivationReason reason) {
            if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick) {
                if (window_.isVisible()) {
                    dispatch(AppCommand::HideToTray);
                } else {
                    dispatch(AppCommand::ShowHome);
                }
            }
        }
    );

    trayIcon_->show();
}

void AppController::setupWindow() {
    window_.applyAppearanceMode(shellSettings_.appearanceMode);
    window_.applyLanguage(currentLanguage_);
    window_.applyShortcutSettings(shellSettings_.shortcuts.mainWindow);
    window_.setHistoryLimit(shellSettings_.historyLimit);
    window_.setCloseToTrayEnabled(shellSettings_.closeToTray);
    captureCoordinator_->setOverlayShortcutSettings(shellSettings_.shortcuts.overlay);
    captureCoordinator_->setOverlayLanguage(currentLanguage_);
    pinboardManager_->setPinWindowShortcutSettings(shellSettings_.shortcuts.pinWindow);
    pinboardManager_->setPinWindowLanguage(currentLanguage_);

    connect(&window_, &MainWindow::regionCaptureRequested, this, [this]() {
        dispatch(AppCommand::StartRegionCapture);
    });
    connect(&window_, &MainWindow::fullscreenCaptureRequested, this, [this]() {
        dispatch(AppCommand::StartFullscreenCapture);
    });
    connect(&window_, &MainWindow::activeWindowCaptureRequested, this, [this]() {
        dispatch(AppCommand::StartActiveWindowCapture);
    });
    connect(&window_, &MainWindow::windowFitCaptureRequested, this, [this]() {
        dispatch(AppCommand::StartWindowFitCapture);
    });
    connect(&window_, &MainWindow::pinLatestCaptureRequested, this, [this]() {
        dispatch(AppCommand::PinLatestCapture);
    });
    connect(&window_, &MainWindow::saveLatestCaptureRequested, this, [this]() {
        dispatch(AppCommand::SaveLatestCapture);
    });
    connect(&window_, &MainWindow::closeAllPinsRequested, this, [this]() {
        dispatch(AppCommand::CloseAllPins);
    });
    connect(&window_, &MainWindow::restorePinInputRequested, this, [this]() {
        dispatch(AppCommand::RestorePinInput);
    });
    connect(&window_, &MainWindow::historyPinRequested, this, [this](const QImage& image) {
        pinCaptureFromHistory(image);
    });
    connect(
        &window_,
        &MainWindow::historySaveRequested,
        this,
        [this](const QString& entryId, const QImage& image, int captureMode) {
            saveHistoryCapture(entryId, image, captureMode);
        }
    );
    connect(&window_, &MainWindow::historyCopyRequested, this, [this](const QImage& image) {
        copyCaptureToClipboard(image);
    });
    connect(&window_, &MainWindow::openCapturesDirectoryRequested, this, [this]() {
        dispatch(AppCommand::OpenCapturesDirectory);
    });
    connect(&window_, &MainWindow::settingsRequested, this, [this]() {
        dispatch(AppCommand::OpenSettings);
    });
    connect(&window_, &MainWindow::hideToTrayRequested, this, [this]() {
        dispatch(AppCommand::HideToTray);
    });
    connect(&window_, &MainWindow::quitRequested, this, [this]() {
        dispatch(AppCommand::Quit);
    });
    connect(
        captureCoordinator_.get(),
        &cappy::services::capture::CaptureCoordinator::captureCompleted,
        this,
        &AppController::onCaptureCompleted
    );
    connect(
        captureCoordinator_.get(),
        &cappy::services::capture::CaptureCoordinator::captureFinalized,
        this,
        &AppController::onCaptureFinalized
    );
    connect(
        captureCoordinator_.get(),
        &cappy::services::capture::CaptureCoordinator::captureFailed,
        this,
        &AppController::onCaptureFailed
    );
    connect(
        captureCoordinator_.get(),
        &cappy::services::capture::CaptureCoordinator::captureCanceled,
        this,
        &AppController::onCaptureCanceled
    );
    connect(
        captureCoordinator_.get(),
        &cappy::services::capture::CaptureCoordinator::ocrRequested,
        this,
        [this](const QImage& image) {
            setGlobalHotkeysSuspended(false);
            openOcrWindow(image);
        }
    );
    connect(
        pinboardManager_.get(),
        &cappy::services::pinboard::PinboardManager::ocrRequested,
        this,
        &AppController::openOcrWindow
    );
}

void AppController::setupGlobalHotkeys() {
    if (hotkeyService_ == nullptr) {
        return;
    }

    hotkeyService_->setEnabled(shellSettings_.globalHotkeysEnabled);
    hotkeyService_->setBindings(globalHotkeysFromSettings(shellSettings_, currentLanguage_));
    connect(
        hotkeyService_.get(),
        &cappy::services::hotkey::GlobalHotkeyService::hotkeyActivated,
        this,
        [this](const QString& hotkeyId) {
            if (hotkeyId == kShowHomeHotkeyId) {
                dispatch(AppCommand::ShowHome);
            } else if (hotkeyId == kRegionCaptureHotkeyId) {
                dispatch(AppCommand::StartWindowFitCapture);
            }
        }
    );
}

void AppController::refreshTrayTexts() {
    const auto& text = cappy::localization::strings(currentLanguage_);
    if (trayRegionCaptureAction_ != nullptr) trayRegionCaptureAction_->setText(text.actionScreenshot);
    if (trayCurrentScreenCaptureAction_ != nullptr) {
        trayCurrentScreenCaptureAction_->setText(text.actionCurrentScreenCapture);
    }
    if (trayFullscreenCaptureAction_ != nullptr) trayFullscreenCaptureAction_->setText(text.actionFullscreenCapture);
    if (traySettingsAction_ != nullptr) traySettingsAction_->setText(text.actionSettings);
    if (trayRestartAction_ != nullptr) trayRestartAction_->setText(text.actionRestart);
    if (trayQuitAction_ != nullptr) trayQuitAction_->setText(text.actionQuit);
}

void AppController::restoreShellState() {
    if (!shellSettings_.mainWindowGeometry.isEmpty()) {
        window_.restoreGeometry(shellSettings_.mainWindowGeometry);
    }

    if (shellSettings_.startMinimized && trayIcon_ != nullptr) {
        hideToTray();
        return;
    }

    showMainWindow();
}

void AppController::showMainWindow() {
    window_.show();
    window_.raise();
    window_.activateWindow();
    window_.setCommandStatus(cappy::localization::strings(currentLanguage_).readyStatus);
}

void AppController::hideToTray() {
    window_.hide();
    if (trayIcon_ != nullptr) {
        trayIcon_->show();
    }
    window_.setCommandStatus(cappy::localization::strings(currentLanguage_).runningInTrayStatus);
}

void AppController::closeActiveEditorWindow() {
    if (activeEditorWindow_ != nullptr) {
        activeEditorWindow_->close();
        activeEditorWindow_ = nullptr;
    }
}

void AppController::syncLatestCaptureImage(const QImage& image, bool clearSavedState) {
    if (image.isNull()) {
        return;
    }

    lastCapturedImage_ = image;
    window_.updateHistoryEntryImage(lastCaptureHistoryEntryId_, image, clearSavedState);
}

void AppController::rememberCaptureResult(const cappy::domain::capture::CaptureResult& result) {
    lastCapturedImage_ = result.image;
    lastCaptureMode_ = result.mode;
    lastCaptureGeometry_ = result.geometry;
    lastCaptureHistoryEntryId_ = QUuid::createUuid().toString(QUuid::WithoutBraces);

    window_.showCaptureResult(result.image, result.geometry, result.backendName);
    window_.addCaptureHistoryEntry(
        {
            .id = lastCaptureHistoryEntryId_,
            .title = QString("%1 | %2 | %3x%4")
                .arg(
                    QDateTime::currentDateTime().toString("HH:mm:ss"),
                    captureModeDisplayLabel(currentLanguage_, result.mode),
                    QString::number(result.image.width()),
                    QString::number(result.image.height())
                ),
            .image = result.image,
            .filePath = {},
            .captureMode = static_cast<int>(result.mode),
        }
    );
}

void AppController::setGlobalHotkeysSuspended(bool suspended) {
    if (hotkeyService_ != nullptr) {
        hotkeyService_->setSuspended(suspended);
    }
}

QString AppController::saveCaptureToDefaultDirectory(
    const QImage& image,
    const QString& label
) const {
    if (image.isNull()) {
        return {};
    }

    QDir outputDir(shellSettings_.defaultSaveDirectory);
    if (!outputDir.exists()) {
        outputDir.mkpath(".");
    }

    const QString fileName = QString("%1-%2.png")
        .arg(label, QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss-zzz"));
    const QString filePath = outputDir.filePath(fileName);
    if (!image.save(filePath, "PNG")) {
        return {};
    }

    return filePath;
}

void AppController::pinLatestCapture() {
    if (activeEditorWindow_ != nullptr) {
        syncLatestCaptureImage(activeEditorWindow_->currentImage());
    }

    if (lastCapturedImage_.isNull()) {
        window_.setCommandStatus(cappy::localization::strings(currentLanguage_).statusNoCaptureToPin);
        return;
    }

    if (!pinboardManager_->pinImage(
            lastCapturedImage_,
            lastCaptureGeometry_.has_value() ? std::optional(lastCaptureGeometry_->topLeft()) : std::nullopt
        )) {
        window_.setCommandStatus(cappy::localization::strings(currentLanguage_).statusFailedCreatePin);
        return;
    }

    qInfo() << "Pinned latest capture. Open pins:" << pinboardManager_->openPinCount();
    window_.setCommandStatus(
        cappy::localization::strings(currentLanguage_).statusPinnedLatestTemplate.arg(
            pinboardManager_->openPinCount()
        )
    );
}

void AppController::saveLatestCapture() {
    if (activeEditorWindow_ != nullptr) {
        syncLatestCaptureImage(activeEditorWindow_->currentImage());
    }

    if (lastCapturedImage_.isNull() || !lastCaptureMode_.has_value()) {
        window_.setCommandStatus(cappy::localization::strings(currentLanguage_).statusNoCaptureToSave);
        return;
    }

    const QString savedPath = saveCaptureToDefaultDirectory(
        lastCapturedImage_,
        captureModeFileLabel(*lastCaptureMode_)
    );
    if (savedPath.isEmpty()) {
        window_.setCommandStatus(cappy::localization::strings(currentLanguage_).statusFailedSaveLatest);
        return;
    }

    window_.markHistoryEntrySaved(lastCaptureHistoryEntryId_, savedPath);
    window_.setCommandStatus(
        cappy::localization::strings(currentLanguage_).statusSavedLatestTemplate.arg(savedPath)
    );
}

void AppController::pinCaptureFromHistory(const QImage& image) {
    if (image.isNull()) {
        window_.setCommandStatus(cappy::localization::strings(currentLanguage_).statusSelectedHistoryNoImage);
        return;
    }

    if (!pinboardManager_->pinImage(image)) {
        window_.setCommandStatus(cappy::localization::strings(currentLanguage_).statusFailedPinHistory);
        return;
    }

    qInfo() << "Pinned capture from history. Open pins:" << pinboardManager_->openPinCount();
    window_.setCommandStatus(
        cappy::localization::strings(currentLanguage_).statusPinnedHistoryTemplate.arg(
            pinboardManager_->openPinCount()
        )
    );
}

void AppController::saveHistoryCapture(const QString& entryId, const QImage& image, int captureMode) {
    if (image.isNull()) {
        window_.setCommandStatus(cappy::localization::strings(currentLanguage_).statusSelectedHistoryNoImage);
        return;
    }

    const std::optional<cappy::domain::capture::CaptureMode> mode = captureModeFromValue(captureMode);
    const QString savedPath = saveCaptureToDefaultDirectory(
        image,
        mode.has_value() ? captureModeFileLabel(*mode) : "history"
    );
    if (savedPath.isEmpty()) {
        window_.setCommandStatus(cappy::localization::strings(currentLanguage_).statusFailedSaveHistory);
        return;
    }

    window_.markHistoryEntrySaved(entryId, savedPath);
    window_.setCommandStatus(
        cappy::localization::strings(currentLanguage_).statusSavedHistoryTemplate.arg(savedPath)
    );
}

void AppController::copyCaptureToClipboard(const QImage& image) {
    if (image.isNull()) {
        window_.setCommandStatus(cappy::localization::strings(currentLanguage_).statusSelectedHistoryNoImage);
        return;
    }

    QApplication::clipboard()->setImage(image);
    window_.setCommandStatus(cappy::localization::strings(currentLanguage_).statusCopiedHistory);
}

void AppController::closeAllPins() {
    pinboardManager_->closeAllPins();
    window_.setCommandStatus(cappy::localization::strings(currentLanguage_).statusClosedAllPins);
}

void AppController::setPinsClickThrough(bool enabled) {
    const int updatedPins = pinboardManager_->setClickThroughForAllPins(enabled);
    if (updatedPins == 0) {
        window_.setCommandStatus(cappy::localization::strings(currentLanguage_).statusNoPinnedWindowsToUpdate);
        return;
    }

    window_.setCommandStatus(
        enabled
            ? cappy::localization::strings(currentLanguage_).statusEnabledClickThroughTemplate.arg(updatedPins)
            : cappy::localization::strings(currentLanguage_).statusRestoredPinInputTemplate.arg(updatedPins)
    );
}

void AppController::openCapturesDirectory() {
    const QUrl url = QUrl::fromLocalFile(shellSettings_.defaultSaveDirectory);
    if (QDesktopServices::openUrl(url)) {
        window_.setCommandStatus(
            cappy::localization::strings(currentLanguage_).statusOpenedCaptureDirectoryTemplate.arg(
                url.toLocalFile()
            )
        );
        return;
    }

    window_.setCommandStatus(
        cappy::localization::strings(currentLanguage_).statusFailedOpenCaptureDirectoryTemplate.arg(
            url.toLocalFile()
        )
    );
}

void AppController::openSettingsDialog() {
    qInfo() << "Open settings requested";
    QWidget* dialogParent = window_.isVisible() ? &window_ : nullptr;
    const bool restoreHotkeysSuspended = hotkeyService_ != nullptr
        ? hotkeyService_->isSuspended()
        : false;
    const auto& text = cappy::localization::strings(currentLanguage_);

    settingsDialogOpen_ = true;
    setGlobalHotkeysSuspended(true);
    window_.setShortcutActionsSuspended(true);

    SettingsDialog dialog(
        shellSettings_,
        SettingsDialog::Diagnostics{
            .captureBackendSummary = captureCoordinator_->backendSummary(),
            .hotkeyBackendSummary = hotkeyService_ == nullptr
                ? text.diagnosticsUnavailable
                : hotkeyService_->backendSummary(),
            .hotkeyBindingsSummary = hotkeyService_ == nullptr
                ? text.diagnosticsUnavailable
                : hotkeyService_->bindingsSummary(),
            .hotkeyRegistrationErrors = hotkeyService_ == nullptr
                ? QStringList{}
                : hotkeyService_->lastRegistrationErrors(),
            .logFilePath = logFilePath_,
        },
        dialogParent
    );

    if (dialog.exec() != QDialog::Accepted) {
        window_.setShortcutActionsSuspended(false);
        settingsDialogOpen_ = false;
        setGlobalHotkeysSuspended(restoreHotkeysSuspended);
        return;
    }

    shellSettings_ = dialog.shellSettings();
    settings_.saveShellSettings(shellSettings_);
    currentLanguage_ = cappy::localization::resolvedAppLanguageFromSettings(
        shellSettings_.interfaceLanguage
    );
    window_.applyAppearanceMode(shellSettings_.appearanceMode);
    window_.applyLanguage(currentLanguage_);
    window_.applyShortcutSettings(shellSettings_.shortcuts.mainWindow);
    window_.setHistoryLimit(shellSettings_.historyLimit);
    window_.setCloseToTrayEnabled(shellSettings_.closeToTray);
    captureCoordinator_->setOverlayShortcutSettings(shellSettings_.shortcuts.overlay);
    captureCoordinator_->setOverlayLanguage(currentLanguage_);
    pinboardManager_->setPinWindowShortcutSettings(shellSettings_.shortcuts.pinWindow);
    pinboardManager_->setPinWindowLanguage(currentLanguage_);
    refreshTrayTexts();
    if (activeEditorWindow_ != nullptr) {
        activeEditorWindow_->setLanguage(currentLanguage_);
        activeEditorWindow_->applyShortcutSettings(shellSettings_.shortcuts.editor);
    }
    if (hotkeyService_ != nullptr) {
        hotkeyService_->setEnabled(shellSettings_.globalHotkeysEnabled);
        hotkeyService_->setBindings(globalHotkeysFromSettings(shellSettings_, currentLanguage_));
        hotkeyService_->setSuspended(restoreHotkeysSuspended);
        qInfo() << "Applied hotkeys:" << hotkeyService_->bindingsSummary();
        for (const QString& error : hotkeyService_->lastRegistrationErrors()) {
            qWarning() << "Global hotkey registration:" << error;
        }
    }
    window_.setShortcutActionsSuspended(false);
    settingsDialogOpen_ = false;
    const auto& updatedText = cappy::localization::strings(currentLanguage_);
    window_.setCommandStatus(
        hotkeyService_ != nullptr && !hotkeyService_->lastRegistrationErrors().isEmpty()
            ? updatedText.settingsSavedWithErrorsStatus
            : updatedText.settingsSavedStatus
    );
}

void AppController::startFullscreenCapture() {
    qInfo() << "Fullscreen capture requested";

    if (!captureCoordinator_->isFullscreenCaptureSupported()) {
        onCaptureFailed(captureCoordinator_->backendSummary());
        return;
    }

    closeActiveEditorWindow();
    wasWindowVisibleBeforeCapture_ = window_.isVisible();
    window_.setCommandStatus(
        cappy::localization::strings(currentLanguage_).statusStartingFullscreenCapture
    );
    setGlobalHotkeysSuspended(true);

    QTimer::singleShot(120, this, [this]() {
        captureCoordinator_->captureFullscreen();
    });
}

void AppController::startCurrentScreenCapture() {
    qInfo() << "Current-screen capture requested";

    if (!captureCoordinator_->isCurrentScreenCaptureSupported()) {
        onCaptureFailed(captureCoordinator_->backendSummary());
        return;
    }

    closeActiveEditorWindow();
    wasWindowVisibleBeforeCapture_ = window_.isVisible();
    window_.setCommandStatus(
        cappy::localization::strings(currentLanguage_).statusStartingCurrentScreenCapture
    );
    setGlobalHotkeysSuspended(true);

    QTimer::singleShot(120, this, [this]() {
        captureCoordinator_->captureCurrentScreen();
    });
}

void AppController::startActiveWindowCapture() {
    qInfo() << "Active-window capture requested";

    if (!captureCoordinator_->isActiveWindowCaptureSupported()) {
        onCaptureFailed(captureCoordinator_->backendSummary());
        return;
    }

    closeActiveEditorWindow();
    wasWindowVisibleBeforeCapture_ = window_.isVisible();
    window_.setCommandStatus(
        cappy::localization::strings(currentLanguage_).statusStartingActiveWindowCapture
    );
    setGlobalHotkeysSuspended(true);

    QTimer::singleShot(120, this, [this]() {
        captureCoordinator_->captureActiveWindow();
    });
}

void AppController::startWindowFitCapture() {
    qInfo() << "Window-fit capture requested";

    if (!captureCoordinator_->isWindowFitCaptureSupported()) {
        onCaptureFailed(captureCoordinator_->backendSummary());
        return;
    }

    closeActiveEditorWindow();
    wasWindowVisibleBeforeCapture_ = window_.isVisible();
    window_.setCommandStatus(
        cappy::localization::strings(currentLanguage_).statusStartingWindowFitCapture
    );
    setGlobalHotkeysSuspended(true);

    QTimer::singleShot(120, this, [this]() {
        captureCoordinator_->startWindowFitCapture();
    });
}

void AppController::startRegionCapture() {
    qInfo() << "Region capture requested";

    if (!captureCoordinator_->isRegionCaptureSupported()) {
        onCaptureFailed(captureCoordinator_->backendSummary());
        return;
    }

    closeActiveEditorWindow();
    wasWindowVisibleBeforeCapture_ = window_.isVisible();
    window_.setCommandStatus(
        cappy::localization::strings(currentLanguage_).statusStartingRegionCapture
    );
    setGlobalHotkeysSuspended(true);

    QTimer::singleShot(120, this, [this]() {
        captureCoordinator_->startRegionCapture();
    });
}

void AppController::onCaptureCompleted(const cappy::domain::capture::CaptureResult& result) {
    setGlobalHotkeysSuspended(false);
    rememberCaptureResult(result);
    const auto& text = cappy::localization::strings(currentLanguage_);
    qInfo() << "Capture completed:"
            << result.image.size()
            << result.geometry
            << result.backendName;
    window_.setCommandStatus(text.statusCaptureReadyTemplate.arg(
        captureModeDisplayLabel(currentLanguage_, result.mode),
        QString::number(result.image.width()),
        QString::number(result.image.height()),
        QString::number(result.geometry.x()),
        QString::number(result.geometry.y())
    ));

    if (pinAfterNextCaptureEvent_) {
        pinAfterNextCaptureEvent_ = false;
        pinLatestCapture();
    }

    if (exitAfterNextCaptureEvent_) {
        exitAfterNextCaptureEvent_ = false;
        QTimer::singleShot(0, &app_, &QCoreApplication::quit);
        return;
    }

    closeActiveEditorWindow();
    activeEditorWindow_ = new cappy::features::editor::CaptureEditorWindow(
        result.image,
        result.geometry,
        shellSettings_.shortcuts.editor,
        currentLanguage_
    );
    connect(
        activeEditorWindow_,
        &cappy::features::editor::CaptureEditorWindow::imageChanged,
        this,
        [this](const QImage& image) {
            syncLatestCaptureImage(image, true);
            window_.setCommandStatus(
                cappy::localization::strings(currentLanguage_).statusCaptureUpdatedInEditor
            );
        }
    );
    connect(
        activeEditorWindow_,
        &cappy::features::editor::CaptureEditorWindow::copyRequested,
        this,
        [this](const QImage& image) {
            syncLatestCaptureImage(image);
            QApplication::clipboard()->setImage(lastCapturedImage_);
            window_.setCommandStatus(
                cappy::localization::strings(currentLanguage_).statusCopiedCapture
            );
        }
    );
    connect(
        activeEditorWindow_,
        &cappy::features::editor::CaptureEditorWindow::saveRequested,
        this,
        [this, captureMode = result.mode](const QImage& image) {
            syncLatestCaptureImage(image);
            const QString savedPath = saveCaptureToDefaultDirectory(
                lastCapturedImage_,
                captureModeFileLabel(captureMode)
            );
            if (savedPath.isEmpty()) {
                window_.setCommandStatus(
                    cappy::localization::strings(currentLanguage_).statusFailedSaveCapture
                );
                return;
            }

            window_.markHistoryEntrySaved(lastCaptureHistoryEntryId_, savedPath);
            window_.setCommandStatus(
                cappy::localization::strings(currentLanguage_).statusSavedCaptureTemplate.arg(savedPath)
            );
        }
    );
    connect(
        activeEditorWindow_,
        &cappy::features::editor::CaptureEditorWindow::pinRequested,
        this,
        [this, pinTopLeft = result.geometry.topLeft()](const QImage& image) {
            syncLatestCaptureImage(image);
            if (!pinboardManager_->pinImage(
                    lastCapturedImage_,
                    pinTopLeft
                )) {
                window_.setCommandStatus(
                    cappy::localization::strings(currentLanguage_).statusFailedPinCapture
                );
                return;
            }

            window_.setCommandStatus(
                cappy::localization::strings(currentLanguage_).statusPinnedCaptureTemplate.arg(
                    pinboardManager_->openPinCount()
                )
            );
        }
    );
    connect(
        activeEditorWindow_,
        &cappy::features::editor::CaptureEditorWindow::ocrRequested,
        this,
        &AppController::openOcrWindow
    );
    connect(
        activeEditorWindow_,
        &QObject::destroyed,
        this,
        [this]() {
            activeEditorWindow_ = nullptr;
        }
    );
    activeEditorWindow_->show();
}

void AppController::onCaptureFinalized(
    const cappy::domain::capture::CaptureResult& result,
    cappy::features::capture::CaptureFinalizeAction action
) {
    setGlobalHotkeysSuspended(false);
    rememberCaptureResult(result);
    const auto& text = cappy::localization::strings(currentLanguage_);

    switch (action) {
    case cappy::features::capture::CaptureFinalizeAction::Copy:
        QApplication::clipboard()->setImage(result.image);
        window_.setCommandStatus(text.statusCopiedCapture);
        break;
    case cappy::features::capture::CaptureFinalizeAction::Save: {
        const QString savedPath = saveCaptureToDefaultDirectory(
            result.image,
            captureModeFileLabel(result.mode)
        );
        if (savedPath.isEmpty()) {
            window_.setCommandStatus(text.statusFailedSaveCapture);
            break;
        }
        window_.markHistoryEntrySaved(lastCaptureHistoryEntryId_, savedPath);
        window_.setCommandStatus(text.statusSavedCaptureTemplate.arg(savedPath));
        break;
    }
    case cappy::features::capture::CaptureFinalizeAction::Pin:
        if (!pinboardManager_->pinImage(result.image, result.geometry.topLeft())) {
            window_.setCommandStatus(text.statusFailedPinCapture);
            break;
        }
        window_.setCommandStatus(text.statusPinnedCaptureTemplate.arg(pinboardManager_->openPinCount()));
        break;
    }

    if (exitAfterNextCaptureEvent_) {
        exitAfterNextCaptureEvent_ = false;
        QTimer::singleShot(0, &app_, &QCoreApplication::quit);
    }
}

void AppController::openOcrWindow(const QImage& image) {
    if (image.isNull()) {
        return;
    }

    auto* window = new cappy::features::ocr::OcrResultWindow(
        image,
        shellSettings_.ocr,
        currentLanguage_,
        shellSettings_.appearanceMode,
        window_.isVisible() ? &window_ : nullptr
    );
    window->setAttribute(Qt::WA_DeleteOnClose, true);
    window->show();
    window->raise();
    window->activateWindow();
}

void AppController::onCaptureFailed(const QString& message) {
    setGlobalHotkeysSuspended(false);
    qWarning() << "Capture failed:" << message;
    if (wasWindowVisibleBeforeCapture_) {
        showMainWindow();
    }
    window_.setCommandStatus(
        cappy::localization::strings(currentLanguage_).statusCaptureFailedTemplate.arg(message)
    );

    if (exitAfterNextCaptureEvent_) {
        exitAfterNextCaptureEvent_ = false;
        QTimer::singleShot(0, &app_, &QCoreApplication::quit);
    }
}

void AppController::onCaptureCanceled() {
    setGlobalHotkeysSuspended(false);
    qInfo() << "Capture canceled";
    window_.setCommandStatus(cappy::localization::strings(currentLanguage_).statusCaptureCanceled);

    if (exitAfterNextCaptureEvent_) {
        exitAfterNextCaptureEvent_ = false;
        QTimer::singleShot(0, &app_, &QCoreApplication::quit);
    }
}

void AppController::quitApplication() {
    persistShellState();
    window_.setCloseToTrayEnabled(false);
    app_.quit();
}

void AppController::restartApplication() {
    persistShellState();
    window_.setCloseToTrayEnabled(false);

    QStringList arguments = QCoreApplication::arguments();
    if (!arguments.isEmpty()) {
        arguments.removeFirst();
    }

    QStringList filteredArguments;
    filteredArguments.reserve(arguments.size() + 1);
    for (const QString& argument : arguments) {
        if (!argument.startsWith("--restart-delay-ms=")) {
            filteredArguments.push_back(argument);
        }
    }
    filteredArguments.push_back("--restart-delay-ms=800");

    const bool launched = QProcess::startDetached(
        QCoreApplication::applicationFilePath(),
        filteredArguments,
        QCoreApplication::applicationDirPath()
    );
    if (!launched) {
        window_.setCloseToTrayEnabled(shellSettings_.closeToTray);
        window_.setCommandStatus(cappy::localization::strings(currentLanguage_).statusRestartFailed);
        return;
    }

    app_.quit();
}
