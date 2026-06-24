#include "cappy/localization/Localization.h"

#include <QLocale>

namespace cappy::localization {

namespace {

bool isChinese(AppLanguage language) {
    return language == AppLanguage::SimplifiedChinese;
}

const Strings kEnglishStrings{
    .appName = "Cappy",
    .actionOpenCappy = "Open Cappy",
    .actionScreenshot = "Screenshot",
    .actionRegionCapture = "Region Capture",
    .actionCurrentScreenCapture = "Screen Capture",
    .actionFullscreenCapture = "Global Capture",
    .actionActiveWindow = "Active Window",
    .actionWindowFitCapture = "Window Fit Capture",
    .actionPinLastCapture = "Pin Last Capture",
    .actionSaveLastCapture = "Save Last Capture",
    .actionClosePins = "Close Pins",
    .actionRestorePinInput = "Restore Pin Input",
    .actionOpenCaptureFolder = "Open Capture Folder",
    .actionSettings = "Settings",
    .actionRestart = "Restart",
    .actionHideToTray = "Hide to Tray",
    .actionQuit = "Quit",
    .actionPinSelectedHistoryItem = "Pin Selected History Item",
    .actionCopySelectedHistoryItem = "Copy Selected History Item",
    .actionSaveSelectedHistoryItem = "Save Selected History Item",
    .actionRemoveSelectedHistoryItem = "Remove Selected History Item",
    .actionMakePinsClickThrough = "Make Pins Click-Through",
    .actionCopyToClipboard = "Copy",
    .actionChangeImage = "Change Image",
    .actionSaveToFile = "Save As",
    .actionSaveToCaptureFolder = "Save to Capture Folder",
    .actionPinToDesktop = "Pin",
    .actionCancel = "Cancel",
    .actionClose = "Close",
    .actionFlipHorizontal = "Flip Horizontal",
    .actionFlipVertical = "Flip Vertical",
    .actionRotateClockwise90 = "Rotate Clockwise 90deg",
    .actionRotateCounterclockwise90 = "Rotate Counterclockwise 90deg",
    .actionInvertColors = "Invert Colors",
    .actionLock = "Lock",
    .actionRestore = "Restore",
    .actionQuickCopy = "Quick Copy",
    .actionCopyAndClose = "Copy and Close",
    .actionExtractText = "OCR",
    .actionMore = "More actions",
    .pinScaleUp = "Scale Up",
    .pinScaleDown = "Scale Down",
    .pinResetScaleAndOpacity = "Reset Scale and Opacity",
    .pinOpacityDown = "Opacity Down",
    .pinOpacityUp = "Opacity Up",
    .pinToggleLock = "Toggle Lock",
    .pinToggleClickThrough = "Toggle Click-Through",
    .toolRectangle = "Rectangle",
    .toolEllipse = "Ellipse",
    .toolArrow = "Arrow",
    .toolPen = "Pen",
    .toolMarker = "Marker",
    .toolMosaic = "Mosaic",
    .toolText = "Text",
    .toolSerial = "Serial",
    .toolUndo = "Undo",
    .toolRedo = "Redo",
    .mainWorkspaceTitle = "Capture Workspace",
    .mainHistoryTitle = "History",
    .mainPreviewPlaceholder = "Capture preview",
    .readyStatus = "Ready",
    .runningInTrayStatus = "Running in tray",
    .trayStillRunningMessage = "Cappy is still running in the tray.",
    .historyClipboardOnly = "clipboard only",
    .historySaved = "saved",
    .historyModeLabel = "Mode",
    .historyStateLabel = "State",
    .historySavedToLabel = "Saved to",
    .settingsTitle = "Settings",
    .settingsGeneralTab = "General",
    .settingsShortcutsTab = "Shortcuts",
    .settingsStorageTab = "Storage",
    .settingsOcrTab = "OCR",
    .settingsDiagnosticsTab = "Diagnostics",
    .settingsAppearanceCard = "Appearance",
    .settingsShellCard = "Shell",
    .settingsCaptureOutputCard = "Capture Output",
    .settingsOcrGeneralCard = "OCR",
    .settingsOcrLocalCard = "Local OCR",
    .settingsOcrCloudCard = "Cloud OCR",
    .settingsRuntimeCard = "Runtime",
    .settingsPageStyleLabel = "Page style",
    .settingsInterfaceLanguageLabel = "Interface language",
    .settingsLightMode = "Light",
    .settingsDarkMode = "Dark",
    .settingsStartMinimized = "Start minimized to tray",
    .settingsCloseToTray = "Close window to tray",
    .settingsEnableGlobalHotkeys = "Enable global hotkeys",
    .settingsPrimaryShortcut = "Primary",
    .settingsAlternateShortcut = "Alternate",
    .settingsAlternateShortcutTooltip = "Alternate shortcut",
    .settingsDefaultSaveDirectory = "Default save directory",
    .settingsHistoryLimit = "History limit",
    .settingsBrowse = "Browse...",
    .settingsSave = "Save",
    .settingsOcrProviderLabel = "Default OCR provider",
    .settingsOcrProviderLocal = "Local OCR",
    .settingsOcrProviderCloud = "Cloud API",
    .settingsOcrLocalCommand = "Local command",
    .settingsOcrLocalLanguage = "Recognition language",
    .settingsOcrCloudEndpoint = "API endpoint",
    .settingsOcrCloudModel = "Model",
    .settingsOcrCloudApiKey = "API key",
    .settingsOcrCloudPrompt = "OCR prompt",
    .settingsOcrTimeoutSeconds = "Timeout (seconds)",
    .settingsOcrLocalHint = "Used for the local command path. Default: tesseract + eng+chi_sim.",
    .settingsOcrCloudHint = "Used for the cloud OCR path. Endpoint, model, key, and prompt are "
                            "only required when cloud OCR is selected.",
    .settingsOcrApiKeyShow = "Show",
    .settingsOcrApiKeyHide = "Hide",
    .settingsCaptureBackend = "Capture backend",
    .settingsHotkeyBackend = "Hotkey backend",
    .settingsActiveBindings = "Active bindings",
    .settingsRegistrationErrors = "Registration errors",
    .settingsLogFile = "Log file",
    .settingsNone = "None",
    .settingsChooseDefaultSaveDirectory = "Select default save directory",
    .settingsWarningTitle = "Settings",
    .settingsDefaultSaveDirEmpty = "Default save directory cannot be empty.",
    .settingsDefaultSaveDirMissing = "Default save directory does not exist.",
    .settingsOcrLocalCommandEmpty = "Local OCR command cannot be empty.",
    .settingsOcrCloudEndpointEmpty = "Cloud OCR endpoint cannot be empty.",
    .settingsOcrCloudModelEmpty = "Cloud OCR model cannot be empty.",
    .settingsOcrCloudApiKeyEmpty = "Cloud OCR API key cannot be empty.",
    .settingsShortcutSingleTemplate = "%1 must be a single shortcut.",
    .settingsDuplicateShortcutTemplate = "Duplicate shortcut in %1: %2 and %3 both use %4.",
    .settingsSavedStatus = "Settings saved.",
    .settingsSavedWithErrorsStatus = "Settings saved with hotkey registration errors.",
    .settingsSectionGlobal = "Global",
    .settingsSectionMainWindow = "Main Window",
    .settingsSectionCaptureOverlay = "Capture Overlay",
    .settingsSectionCaptureEditor = "Capture Editor",
    .settingsSectionPinnedWindow = "Pinned Window",
    .settingsLanguageSystem = "Follow System",
    .settingsLanguageEnglish = "English",
    .settingsLanguageChinese = "Simplified Chinese",
    .overlayInstruction = "Drag to capture a region. Press Esc to cancel.",
    .dialogAddTextTitle = "Add Text",
    .dialogAddTextLabel = "Text:",
    .dialogOcrTitle = "OCR",
    .dialogOcrRun = "Recognize",
    .dialogOcrZoomIn = "Zoom In",
    .dialogOcrZoomOut = "Zoom Out",
    .dialogOcrFitImage = "Fit Image",
    .dialogOcrCopyText = "Copy Text",
    .dialogOcrSaveText = "Save Text",
    .dialogOcrPreviewTitle = "Preview",
    .dialogOcrResultTitle = "Recognized Text",
    .dialogOcrRegionListTitle = "Lines",
    .dialogOcrFullTextTitle = "Full Text",
    .dialogOcrRegionConfidenceTemplate = "Confidence %1%",
    .dialogOcrRegionNoConfidence = "Confidence unavailable",
    .dialogOcrImageMetaTemplate = "Image %1 x %2",
    .dialogOcrZoomTemplate = "Zoom %1%",
    .dialogOcrTextPlaceholder = "Recognized text will appear here.",
    .dialogOcrTextFileFilter = "Text Files (*.txt)",
    .dialogOcrStatusRunningLocal = "Running local OCR...",
    .dialogOcrStatusRunningCloud = "Running cloud OCR...",
    .dialogOcrStatusFinishedTemplate = "%1 characters recognized, %2 regions highlighted",
    .dialogOcrStatusCopiedText = "Recognized text copied to clipboard.",
    .dialogOcrStatusSaveFailed = "Failed to save OCR text.",
    .dialogOcrStatusSavedTextTemplate = "OCR text saved to %1",
    .dialogOpenImage = "Open Image",
    .dialogSaveAs = "Save As",
    .fileDialogImageFilter = "PNG Images (*.png);;JPEG Images (*.jpg *.jpeg);;BMP Images (*.bmp)",
    .singleInstanceAlreadyRunningMessage = "Cappy is already running.",
    .diagnosticsUnavailable = "unavailable",
    .statusNoCaptureToPin = "No capture available to pin.",
    .statusFailedCreatePin = "Failed to create pinned window.",
    .statusPinnedLatestTemplate = "Pinned latest capture. Open pins: %1",
    .statusNoCaptureToSave = "No capture available to save.",
    .statusFailedSaveLatest = "Failed to save latest capture.",
    .statusSavedLatestTemplate = "Saved latest capture to %1",
    .statusSelectedHistoryNoImage = "Selected history item has no image data.",
    .statusFailedPinHistory = "Failed to pin capture from history.",
    .statusPinnedHistoryTemplate = "Pinned capture from history. Open pins: %1",
    .statusFailedSaveHistory = "Failed to save history capture.",
    .statusSavedHistoryTemplate = "Saved history capture to %1",
    .statusCopiedHistory = "Copied history capture to clipboard.",
    .statusClosedAllPins = "Closed all pinned windows.",
    .statusNoPinnedWindowsToUpdate = "No pinned windows to update.",
    .statusEnabledClickThroughTemplate =
        "Enabled click-through for %1 pinned window(s). Use Restore Pin Input to interact again.",
    .statusRestoredPinInputTemplate = "Restored pointer input for %1 pinned window(s).",
    .statusOpenedCaptureDirectoryTemplate = "Opened capture directory: %1",
    .statusFailedOpenCaptureDirectoryTemplate = "Failed to open capture directory: %1",
    .statusStartingCurrentScreenCapture = "Starting current-screen capture...",
    .statusStartingFullscreenCapture = "Starting global capture...",
    .statusStartingActiveWindowCapture = "Starting active-window capture...",
    .statusStartingWindowFitCapture = "Starting window-fit capture...",
    .statusStartingRegionCapture = "Starting region capture...",
    .statusCaptureReadyTemplate = "%1 capture ready: %2x%3 at (%4, %5).",
    .statusCaptureUpdatedInEditor = "Capture updated in review window.",
    .statusCopiedCapture = "Copied capture to clipboard.",
    .statusFailedSaveCapture = "Failed to save capture.",
    .statusSavedCaptureTemplate = "Saved capture to %1",
    .statusFailedPinCapture = "Failed to pin capture.",
    .statusPinnedCaptureTemplate = "Pinned capture. Open pins: %1",
    .statusCaptureFailedTemplate = "Capture failed: %1",
    .statusCaptureCanceled = "Capture canceled",
    .statusRestartFailed = "Failed to restart application.",
    .statusRemovedHistoryEntry = "Removed history entry.",
    .captureModeRegion = "Region",
    .captureModeCurrentScreen = "Screen",
    .captureModeFullscreen = "Global",
    .captureModeActiveWindow = "Active Window",
    .captureModeWindowFit = "Window Fit",
    .captureModeUnknown = "Unknown",
};

const Strings kChineseStrings{
    .appName = "Cappy",
    .actionOpenCappy = "打开 Cappy",
    .actionScreenshot = "截图",
    .actionRegionCapture = "区域截图",
    .actionCurrentScreenCapture = "屏幕截图",
    .actionFullscreenCapture = "全局截图",
    .actionActiveWindow = "活动窗口",
    .actionWindowFitCapture = "窗口贴合截图",
    .actionPinLastCapture = "贴出最近截图",
    .actionSaveLastCapture = "保存最近截图",
    .actionClosePins = "关闭贴图",
    .actionRestorePinInput = "恢复贴图交互",
    .actionOpenCaptureFolder = "打开截图目录",
    .actionSettings = "设置",
    .actionRestart = "重启",
    .actionHideToTray = "最小化到托盘",
    .actionQuit = "退出",
    .actionPinSelectedHistoryItem = "贴出所选历史项",
    .actionCopySelectedHistoryItem = "复制所选历史项",
    .actionSaveSelectedHistoryItem = "保存所选历史项",
    .actionRemoveSelectedHistoryItem = "移除所选历史项",
    .actionMakePinsClickThrough = "贴图穿透",
    .actionCopyToClipboard = "复制",
    .actionChangeImage = "更改图片",
    .actionSaveToFile = "另存为",
    .actionSaveToCaptureFolder = "保存到截图目录",
    .actionPinToDesktop = "贴图",
    .actionCancel = "取消",
    .actionClose = "关闭",
    .actionFlipHorizontal = "水平翻转",
    .actionFlipVertical = "垂直翻转",
    .actionRotateClockwise90 = "顺时针旋转 90 度",
    .actionRotateCounterclockwise90 = "逆时针旋转 90 度",
    .actionInvertColors = "反色",
    .actionLock = "锁定",
    .actionRestore = "复原",
    .actionQuickCopy = "快速复制",
    .actionCopyAndClose = "复制并关闭",
    .actionExtractText = "OCR",
    .actionMore = "更多操作",
    .pinScaleUp = "放大",
    .pinScaleDown = "缩小",
    .pinResetScaleAndOpacity = "重置缩放和透明度",
    .pinOpacityDown = "降低透明度",
    .pinOpacityUp = "提高透明度",
    .pinToggleLock = "切换锁定",
    .pinToggleClickThrough = "切换穿透",
    .toolRectangle = "矩形",
    .toolEllipse = "椭圆",
    .toolArrow = "箭头",
    .toolPen = "画笔",
    .toolMarker = "标注笔",
    .toolMosaic = "马赛克",
    .toolText = "文字",
    .toolSerial = "序号",
    .toolUndo = "撤销",
    .toolRedo = "重做",
    .mainWorkspaceTitle = "截图工作区",
    .mainHistoryTitle = "历史记录",
    .mainPreviewPlaceholder = "截图预览",
    .readyStatus = "就绪",
    .runningInTrayStatus = "正在托盘中运行",
    .trayStillRunningMessage = "Cappy 仍在系统托盘中运行。",
    .historyClipboardOnly = "仅剪贴板",
    .historySaved = "已保存",
    .historyModeLabel = "模式",
    .historyStateLabel = "状态",
    .historySavedToLabel = "保存到",
    .settingsTitle = "设置",
    .settingsGeneralTab = "常规",
    .settingsShortcutsTab = "快捷键",
    .settingsStorageTab = "存储",
    .settingsOcrTab = "OCR",
    .settingsDiagnosticsTab = "诊断",
    .settingsAppearanceCard = "界面",
    .settingsShellCard = "程序行为",
    .settingsCaptureOutputCard = "截图输出",
    .settingsOcrGeneralCard = "OCR",
    .settingsOcrLocalCard = "本地 OCR",
    .settingsOcrCloudCard = "云端 OCR",
    .settingsRuntimeCard = "运行时",
    .settingsPageStyleLabel = "页面风格",
    .settingsInterfaceLanguageLabel = "界面语言",
    .settingsLightMode = "浅色",
    .settingsDarkMode = "深色",
    .settingsStartMinimized = "启动时最小化到托盘",
    .settingsCloseToTray = "关闭窗口时退到托盘",
    .settingsEnableGlobalHotkeys = "启用全局快捷键",
    .settingsPrimaryShortcut = "主快捷键",
    .settingsAlternateShortcut = "备用快捷键",
    .settingsAlternateShortcutTooltip = "备用快捷键",
    .settingsDefaultSaveDirectory = "默认保存目录",
    .settingsHistoryLimit = "历史上限",
    .settingsBrowse = "浏览...",
    .settingsSave = "保存",
    .settingsOcrProviderLabel = "默认 OCR 链路",
    .settingsOcrProviderLocal = "本地 OCR",
    .settingsOcrProviderCloud = "云端接口",
    .settingsOcrLocalCommand = "本地命令",
    .settingsOcrLocalLanguage = "识别语言",
    .settingsOcrCloudEndpoint = "接口地址",
    .settingsOcrCloudModel = "模型名称",
    .settingsOcrCloudApiKey = "接口密钥",
    .settingsOcrCloudPrompt = "OCR 提示词",
    .settingsOcrTimeoutSeconds = "超时时间（秒）",
    .settingsOcrLocalHint = "用于本地命令链路。默认：tesseract + eng+chi_sim。",
    .settingsOcrCloudHint =
        "用于云端 OCR 链路。仅在选择云端 OCR 时需要填写接口地址、模型、密钥和提示词。",
    .settingsOcrApiKeyShow = "显示",
    .settingsOcrApiKeyHide = "隐藏",
    .settingsCaptureBackend = "截图后端",
    .settingsHotkeyBackend = "热键后端",
    .settingsActiveBindings = "当前绑定",
    .settingsRegistrationErrors = "注册错误",
    .settingsLogFile = "日志文件",
    .settingsNone = "无",
    .settingsChooseDefaultSaveDirectory = "选择默认保存目录",
    .settingsWarningTitle = "设置",
    .settingsDefaultSaveDirEmpty = "默认保存目录不能为空。",
    .settingsDefaultSaveDirMissing = "默认保存目录不存在。",
    .settingsOcrLocalCommandEmpty = "本地 OCR 命令不能为空。",
    .settingsOcrCloudEndpointEmpty = "云端 OCR 接口地址不能为空。",
    .settingsOcrCloudModelEmpty = "云端 OCR 模型名称不能为空。",
    .settingsOcrCloudApiKeyEmpty = "云端 OCR 接口密钥不能为空。",
    .settingsShortcutSingleTemplate = "%1 只能绑定单个快捷键。",
    .settingsDuplicateShortcutTemplate = "%1 中存在重复快捷键：%2 和 %3 都使用了 %4。",
    .settingsSavedStatus = "设置已保存。",
    .settingsSavedWithErrorsStatus = "设置已保存，但热键注册存在错误。",
    .settingsSectionGlobal = "全局",
    .settingsSectionMainWindow = "主窗口",
    .settingsSectionCaptureOverlay = "截图浮层",
    .settingsSectionCaptureEditor = "截图编辑器",
    .settingsSectionPinnedWindow = "贴图窗口",
    .settingsLanguageSystem = "跟随系统",
    .settingsLanguageEnglish = "English",
    .settingsLanguageChinese = "简体中文",
    .overlayInstruction = "拖动以框选截图区域，按 Esc 取消。",
    .dialogAddTextTitle = "添加文字",
    .dialogAddTextLabel = "文字：",
    .dialogOcrTitle = "OCR",
    .dialogOcrRun = "开始识别",
    .dialogOcrZoomIn = "放大",
    .dialogOcrZoomOut = "缩小",
    .dialogOcrFitImage = "适配图片",
    .dialogOcrCopyText = "复制文字",
    .dialogOcrSaveText = "保存文字",
    .dialogOcrPreviewTitle = "预览",
    .dialogOcrResultTitle = "识别结果",
    .dialogOcrRegionListTitle = "识别条目",
    .dialogOcrFullTextTitle = "完整文本",
    .dialogOcrRegionConfidenceTemplate = "置信度 %1%",
    .dialogOcrRegionNoConfidence = "置信度不可用",
    .dialogOcrImageMetaTemplate = "图片 %1 x %2",
    .dialogOcrZoomTemplate = "缩放 %1%",
    .dialogOcrTextPlaceholder = "识别结果会显示在这里。",
    .dialogOcrTextFileFilter = "文本文件 (*.txt)",
    .dialogOcrStatusRunningLocal = "正在执行本地 OCR...",
    .dialogOcrStatusRunningCloud = "正在执行云端 OCR...",
    .dialogOcrStatusFinishedTemplate = "已识别 %1 个字符，已高亮 %2 个区域",
    .dialogOcrStatusCopiedText = "已将识别结果复制到剪贴板。",
    .dialogOcrStatusSaveFailed = "保存 OCR 文本失败。",
    .dialogOcrStatusSavedTextTemplate = "OCR 文本已保存到 %1",
    .dialogOpenImage = "打开图片",
    .dialogSaveAs = "另存为",
    .fileDialogImageFilter = "PNG 图片 (*.png);;JPEG 图片 (*.jpg *.jpeg);;BMP 图片 (*.bmp)",
    .singleInstanceAlreadyRunningMessage = "Cappy 已经在运行中。",
    .diagnosticsUnavailable = "不可用",
    .statusNoCaptureToPin = "没有可贴出的截图。",
    .statusFailedCreatePin = "创建贴图窗口失败。",
    .statusPinnedLatestTemplate = "已贴出最近截图。当前贴图数：%1",
    .statusNoCaptureToSave = "没有可保存的截图。",
    .statusFailedSaveLatest = "保存最近截图失败。",
    .statusSavedLatestTemplate = "最近截图已保存到 %1",
    .statusSelectedHistoryNoImage = "所选历史项没有图像数据。",
    .statusFailedPinHistory = "从历史记录贴图失败。",
    .statusPinnedHistoryTemplate = "已贴出历史截图。当前贴图数：%1",
    .statusFailedSaveHistory = "保存历史截图失败。",
    .statusSavedHistoryTemplate = "历史截图已保存到 %1",
    .statusCopiedHistory = "历史截图已复制到剪贴板。",
    .statusClosedAllPins = "已关闭所有贴图窗口。",
    .statusNoPinnedWindowsToUpdate = "没有可更新的贴图窗口。",
    .statusEnabledClickThroughTemplate =
        "已为 %1 个贴图窗口开启穿透。需要交互时使用“恢复贴图交互”。",
    .statusRestoredPinInputTemplate = "已恢复 %1 个贴图窗口的鼠标交互。",
    .statusOpenedCaptureDirectoryTemplate = "已打开截图目录：%1",
    .statusFailedOpenCaptureDirectoryTemplate = "打开截图目录失败：%1",
    .statusStartingCurrentScreenCapture = "正在启动屏幕截图...",
    .statusStartingFullscreenCapture = "正在启动全局截图...",
    .statusStartingActiveWindowCapture = "正在启动活动窗口截图...",
    .statusStartingWindowFitCapture = "正在启动窗口贴合截图...",
    .statusStartingRegionCapture = "正在启动区域截图...",
    .statusCaptureReadyTemplate = "%1已就绪：%2x%3，位置 (%4, %5)。",
    .statusCaptureUpdatedInEditor = "截图已在复核窗口中更新。",
    .statusCopiedCapture = "截图已复制到剪贴板。",
    .statusFailedSaveCapture = "保存截图失败。",
    .statusSavedCaptureTemplate = "截图已保存到 %1",
    .statusFailedPinCapture = "贴图失败。",
    .statusPinnedCaptureTemplate = "已贴图。当前贴图数：%1",
    .statusCaptureFailedTemplate = "截图失败：%1",
    .statusCaptureCanceled = "截图已取消",
    .statusRestartFailed = "重启程序失败。",
    .statusRemovedHistoryEntry = "已移除历史记录。",
    .captureModeRegion = "区域截图",
    .captureModeCurrentScreen = "屏幕截图",
    .captureModeFullscreen = "全局截图",
    .captureModeActiveWindow = "活动窗口",
    .captureModeWindowFit = "窗口贴合截图",
    .captureModeUnknown = "未知",
};

} // namespace

AppLanguage appLanguageFromSettingsValue(const QString& value) {
    const QString normalized = value.trimmed().toLower();
    if (normalized == "zh-cn" || normalized == "zh_cn" || normalized == "zh") {
        return AppLanguage::SimplifiedChinese;
    }
    if (normalized == "en" || normalized == "en-us" || normalized == "en_us") {
        return AppLanguage::English;
    }
    return AppLanguage::System;
}

AppLanguage resolvedAppLanguage(AppLanguage requested, const QLocale& locale) {
    if (requested == AppLanguage::English || requested == AppLanguage::SimplifiedChinese) {
        return requested;
    }

    return locale.language() == QLocale::Chinese ? AppLanguage::SimplifiedChinese
                                                 : AppLanguage::English;
}

AppLanguage resolvedAppLanguageFromSettings(const QString& value, const QLocale& locale) {
    return resolvedAppLanguage(appLanguageFromSettingsValue(value), locale);
}

QString appLanguageToSettingsValue(AppLanguage language) {
    switch (language) {
    case AppLanguage::System:
        return "system";
    case AppLanguage::English:
        return "en";
    case AppLanguage::SimplifiedChinese:
        return "zh-CN";
    }

    return "system";
}

QString appLanguageOptionLabel(AppLanguage option, AppLanguage uiLanguage) {
    const Strings& text = strings(uiLanguage);
    switch (option) {
    case AppLanguage::System:
        return text.settingsLanguageSystem;
    case AppLanguage::English:
        return text.settingsLanguageEnglish;
    case AppLanguage::SimplifiedChinese:
        return text.settingsLanguageChinese;
    }

    return text.settingsLanguageSystem;
}

const Strings& strings(AppLanguage language) {
    return isChinese(resolvedAppLanguage(language)) ? kChineseStrings : kEnglishStrings;
}

QString shortcutLabel(AppLanguage language, const QString& id) {
    const Strings& text = strings(language);
    if (id == "global.open_home")
        return text.actionOpenCappy;
    if (id == "global.screenshot")
        return text.actionScreenshot;
    if (id == "main.region_capture")
        return text.actionRegionCapture;
    if (id == "main.fullscreen_capture")
        return text.actionFullscreenCapture;
    if (id == "main.active_window_capture")
        return text.actionActiveWindow;
    if (id == "main.window_fit_capture")
        return text.actionWindowFitCapture;
    if (id == "main.pin_latest")
        return text.actionPinLastCapture;
    if (id == "main.save_latest")
        return text.actionSaveLastCapture;
    if (id == "main.open_capture_folder")
        return text.actionOpenCaptureFolder;
    if (id == "main.settings")
        return text.actionSettings;
    if (id == "main.hide_to_tray")
        return text.actionHideToTray;
    if (id == "main.quit")
        return text.actionQuit;
    if (id == "main.close_pins")
        return text.actionClosePins;
    if (id == "main.restore_pin_input")
        return text.actionRestorePinInput;
    if (id == "main.history_pin")
        return text.actionPinSelectedHistoryItem;
    if (id == "main.history_copy")
        return text.actionCopySelectedHistoryItem;
    if (id == "main.history_save")
        return text.actionSaveSelectedHistoryItem;
    if (id == "main.history_remove")
        return text.actionRemoveSelectedHistoryItem;
    if (id == "overlay.rectangle" || id == "editor.rectangle")
        return text.toolRectangle;
    if (id == "overlay.ellipse" || id == "editor.ellipse")
        return text.toolEllipse;
    if (id == "overlay.arrow" || id == "editor.arrow")
        return text.toolArrow;
    if (id == "overlay.pen" || id == "editor.pen")
        return text.toolPen;
    if (id == "overlay.marker" || id == "editor.marker")
        return text.toolMarker;
    if (id == "overlay.mosaic" || id == "editor.mosaic")
        return text.toolMosaic;
    if (id == "overlay.text" || id == "editor.text")
        return text.toolText;
    if (id == "overlay.serial" || id == "editor.serial")
        return text.toolSerial;
    if (id == "overlay.undo" || id == "editor.undo")
        return text.toolUndo;
    if (id == "overlay.redo" || id == "editor.redo")
        return text.toolRedo;
    if (id == "overlay.copy" || id == "editor.copy")
        return text.actionCopyToClipboard;
    if (id == "overlay.quick_copy")
        return text.actionQuickCopy;
    if (id == "editor.copy_and_close")
        return text.actionCopyAndClose;
    if (id == "overlay.save")
        return text.actionSaveToFile;
    if (id == "editor.save")
        return text.actionSaveToCaptureFolder;
    if (id == "overlay.pin" || id == "editor.pin")
        return text.actionPinToDesktop;
    if (id == "overlay.cancel")
        return text.actionCancel;
    if (id == "editor.close" || id == "pin.close")
        return text.actionClose;
    if (id == "pin.scale_up")
        return text.pinScaleUp;
    if (id == "pin.scale_down")
        return text.pinScaleDown;
    if (id == "pin.reset_scale_opacity")
        return text.pinResetScaleAndOpacity;
    if (id == "pin.opacity_down")
        return text.pinOpacityDown;
    if (id == "pin.opacity_up")
        return text.pinOpacityUp;
    if (id == "pin.toggle_lock")
        return text.pinToggleLock;
    if (id == "pin.toggle_click_through")
        return text.pinToggleClickThrough;
    if (id.endsWith("_alt")) {
        return QString("%1 %2").arg(shortcutLabel(language, id.first(id.size() - 4)),
                                    isChinese(resolvedAppLanguage(language)) ? "备用" : "Alt");
    }
    return id;
}

QString shortcutScopeLabel(AppLanguage language, cappy::shortcuts::ShortcutScope scope) {
    const Strings& text = strings(language);
    switch (scope) {
    case cappy::shortcuts::ShortcutScope::Global:
        return text.settingsSectionGlobal;
    case cappy::shortcuts::ShortcutScope::MainWindow:
        return text.settingsSectionMainWindow;
    case cappy::shortcuts::ShortcutScope::CaptureOverlay:
        return text.settingsSectionCaptureOverlay;
    case cappy::shortcuts::ShortcutScope::CaptureEditor:
        return text.settingsSectionCaptureEditor;
    case cappy::shortcuts::ShortcutScope::PinWindow:
        return text.settingsSectionPinnedWindow;
    }

    return text.settingsShortcutsTab;
}

QString historyCountLabel(AppLanguage language, int count) {
    if (isChinese(resolvedAppLanguage(language))) {
        return QString("%1 项").arg(count);
    }
    return count == 1 ? "1 item" : QString("%1 items").arg(count);
}

QString historyItemDetailText(AppLanguage language, const QString& baseTitle,
                              const QString& modeLabel, const QString& filePath) {
    const Strings& text = strings(language);
    if (filePath.isEmpty()) {
        return QString("%1\n%2: %3\n%4: %5")
            .arg(baseTitle, text.historyModeLabel, modeLabel, text.historyStateLabel,
                 text.historyClipboardOnly);
    }

    return QString("%1\n%2: %3\n%4: %5")
        .arg(baseTitle, text.historyModeLabel, modeLabel, text.historySavedToLabel, filePath);
}

} // namespace cappy::localization
