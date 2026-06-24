#pragma once

#include <QLocale>
#include <QString>

#include "cappy/shortcuts/ShortcutSettings.h"

namespace cappy::localization {

enum class AppLanguage {
    System,
    English,
    SimplifiedChinese,
};

struct Strings {
    QString appName;
    QString actionOpenCappy;
    QString actionScreenshot;
    QString actionRegionCapture;
    QString actionCurrentScreenCapture;
    QString actionFullscreenCapture;
    QString actionActiveWindow;
    QString actionWindowFitCapture;
    QString actionPinLastCapture;
    QString actionSaveLastCapture;
    QString actionClosePins;
    QString actionRestorePinInput;
    QString actionOpenCaptureFolder;
    QString actionSettings;
    QString actionRestart;
    QString actionHideToTray;
    QString actionQuit;
    QString actionPinSelectedHistoryItem;
    QString actionCopySelectedHistoryItem;
    QString actionSaveSelectedHistoryItem;
    QString actionRemoveSelectedHistoryItem;
    QString actionMakePinsClickThrough;
    QString actionCopyToClipboard;
    QString actionChangeImage;
    QString actionSaveToFile;
    QString actionSaveToCaptureFolder;
    QString actionPinToDesktop;
    QString actionCancel;
    QString actionClose;
    QString actionFlipHorizontal;
    QString actionFlipVertical;
    QString actionRotateClockwise90;
    QString actionRotateCounterclockwise90;
    QString actionInvertColors;
    QString actionLock;
    QString actionRestore;
    QString actionQuickCopy;
    QString actionCopyAndClose;
    QString actionExtractText;
    QString actionMore;
    QString pinScaleUp;
    QString pinScaleDown;
    QString pinResetScaleAndOpacity;
    QString pinOpacityDown;
    QString pinOpacityUp;
    QString pinToggleLock;
    QString pinToggleClickThrough;
    QString toolRectangle;
    QString toolEllipse;
    QString toolArrow;
    QString toolPen;
    QString toolMarker;
    QString toolMosaic;
    QString toolText;
    QString toolSerial;
    QString toolUndo;
    QString toolRedo;
    QString mainWorkspaceTitle;
    QString mainHistoryTitle;
    QString mainPreviewPlaceholder;
    QString readyStatus;
    QString runningInTrayStatus;
    QString trayStillRunningMessage;
    QString historyClipboardOnly;
    QString historySaved;
    QString historyModeLabel;
    QString historyStateLabel;
    QString historySavedToLabel;
    QString settingsTitle;
    QString settingsGeneralTab;
    QString settingsShortcutsTab;
    QString settingsStorageTab;
    QString settingsOcrTab;
    QString settingsDiagnosticsTab;
    QString settingsAppearanceCard;
    QString settingsShellCard;
    QString settingsCaptureOutputCard;
    QString settingsOcrGeneralCard;
    QString settingsOcrLocalCard;
    QString settingsOcrCloudCard;
    QString settingsRuntimeCard;
    QString settingsPageStyleLabel;
    QString settingsInterfaceLanguageLabel;
    QString settingsLightMode;
    QString settingsDarkMode;
    QString settingsStartMinimized;
    QString settingsCloseToTray;
    QString settingsEnableGlobalHotkeys;
    QString settingsPrimaryShortcut;
    QString settingsAlternateShortcut;
    QString settingsAlternateShortcutTooltip;
    QString settingsDefaultSaveDirectory;
    QString settingsHistoryLimit;
    QString settingsBrowse;
    QString settingsSave;
    QString settingsOcrProviderLabel;
    QString settingsOcrProviderLocal;
    QString settingsOcrProviderCloud;
    QString settingsOcrLocalCommand;
    QString settingsOcrLocalLanguage;
    QString settingsOcrCloudEndpoint;
    QString settingsOcrCloudModel;
    QString settingsOcrCloudApiKey;
    QString settingsOcrCloudPrompt;
    QString settingsOcrTimeoutSeconds;
    QString settingsOcrLocalHint;
    QString settingsOcrCloudHint;
    QString settingsOcrApiKeyShow;
    QString settingsOcrApiKeyHide;
    QString settingsCaptureBackend;
    QString settingsHotkeyBackend;
    QString settingsActiveBindings;
    QString settingsRegistrationErrors;
    QString settingsLogFile;
    QString settingsNone;
    QString settingsChooseDefaultSaveDirectory;
    QString settingsWarningTitle;
    QString settingsDefaultSaveDirEmpty;
    QString settingsDefaultSaveDirMissing;
    QString settingsOcrLocalCommandEmpty;
    QString settingsOcrCloudEndpointEmpty;
    QString settingsOcrCloudModelEmpty;
    QString settingsOcrCloudApiKeyEmpty;
    QString settingsShortcutSingleTemplate;
    QString settingsDuplicateShortcutTemplate;
    QString settingsSavedStatus;
    QString settingsSavedWithErrorsStatus;
    QString settingsSectionGlobal;
    QString settingsSectionMainWindow;
    QString settingsSectionCaptureOverlay;
    QString settingsSectionCaptureEditor;
    QString settingsSectionPinnedWindow;
    QString settingsLanguageSystem;
    QString settingsLanguageEnglish;
    QString settingsLanguageChinese;
    QString overlayInstruction;
    QString dialogAddTextTitle;
    QString dialogAddTextLabel;
    QString dialogOcrTitle;
    QString dialogOcrRun;
    QString dialogOcrZoomIn;
    QString dialogOcrZoomOut;
    QString dialogOcrFitImage;
    QString dialogOcrCopyText;
    QString dialogOcrSaveText;
    QString dialogOcrPreviewTitle;
    QString dialogOcrResultTitle;
    QString dialogOcrRegionListTitle;
    QString dialogOcrFullTextTitle;
    QString dialogOcrRegionConfidenceTemplate;
    QString dialogOcrRegionNoConfidence;
    QString dialogOcrImageMetaTemplate;
    QString dialogOcrZoomTemplate;
    QString dialogOcrTextPlaceholder;
    QString dialogOcrTextFileFilter;
    QString dialogOcrStatusRunningLocal;
    QString dialogOcrStatusRunningCloud;
    QString dialogOcrStatusFinishedTemplate;
    QString dialogOcrStatusCopiedText;
    QString dialogOcrStatusSaveFailed;
    QString dialogOcrStatusSavedTextTemplate;
    QString dialogOpenImage;
    QString dialogSaveAs;
    QString fileDialogImageFilter;
    QString singleInstanceAlreadyRunningMessage;
    QString diagnosticsUnavailable;
    QString statusNoCaptureToPin;
    QString statusFailedCreatePin;
    QString statusPinnedLatestTemplate;
    QString statusNoCaptureToSave;
    QString statusFailedSaveLatest;
    QString statusSavedLatestTemplate;
    QString statusSelectedHistoryNoImage;
    QString statusFailedPinHistory;
    QString statusPinnedHistoryTemplate;
    QString statusFailedSaveHistory;
    QString statusSavedHistoryTemplate;
    QString statusCopiedHistory;
    QString statusClosedAllPins;
    QString statusNoPinnedWindowsToUpdate;
    QString statusEnabledClickThroughTemplate;
    QString statusRestoredPinInputTemplate;
    QString statusOpenedCaptureDirectoryTemplate;
    QString statusFailedOpenCaptureDirectoryTemplate;
    QString statusStartingCurrentScreenCapture;
    QString statusStartingFullscreenCapture;
    QString statusStartingActiveWindowCapture;
    QString statusStartingWindowFitCapture;
    QString statusStartingRegionCapture;
    QString statusCaptureReadyTemplate;
    QString statusCaptureUpdatedInEditor;
    QString statusCopiedCapture;
    QString statusFailedSaveCapture;
    QString statusSavedCaptureTemplate;
    QString statusFailedPinCapture;
    QString statusPinnedCaptureTemplate;
    QString statusCaptureFailedTemplate;
    QString statusCaptureCanceled;
    QString statusRestartFailed;
    QString statusRemovedHistoryEntry;
    QString captureModeRegion;
    QString captureModeCurrentScreen;
    QString captureModeFullscreen;
    QString captureModeActiveWindow;
    QString captureModeWindowFit;
    QString captureModeUnknown;
};

AppLanguage appLanguageFromSettingsValue(const QString& value);
AppLanguage resolvedAppLanguage(AppLanguage requested, const QLocale& locale = QLocale());
AppLanguage resolvedAppLanguageFromSettings(
    const QString& value,
    const QLocale& locale = QLocale()
);
QString appLanguageToSettingsValue(AppLanguage language);
QString appLanguageOptionLabel(AppLanguage option, AppLanguage uiLanguage);
const Strings& strings(AppLanguage language);
QString shortcutLabel(AppLanguage language, const QString& id);
QString shortcutScopeLabel(AppLanguage language, cappy::shortcuts::ShortcutScope scope);
QString historyCountLabel(AppLanguage language, int count);
QString historyItemDetailText(
    AppLanguage language,
    const QString& baseTitle,
    const QString& modeLabel,
    const QString& filePath
);

}  // namespace cappy::localization
