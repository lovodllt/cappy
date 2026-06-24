#include "AppSettings.h"

#include <QSettings>

#include <utility>

namespace {

constexpr auto kShellGroup = "shell";
constexpr auto kShortcutGroup = "shortcuts";
constexpr auto kStartMinimizedKey = "startMinimized";
constexpr auto kCloseToTrayKey = "closeToTray";
constexpr auto kGlobalHotkeysEnabledKey = "globalHotkeysEnabled";
constexpr auto kDefaultSaveDirectoryKey = "defaultSaveDirectory";
constexpr auto kAppearanceModeKey = "appearanceMode";
constexpr auto kInterfaceLanguageKey = "interfaceLanguage";
constexpr auto kHistoryLimitKey = "historyLimit";
constexpr auto kMainWindowGeometryKey = "mainWindowGeometry";
constexpr auto kOcrGroup = "ocr";
constexpr auto kOcrPreferredProviderKey = "preferredProvider";
constexpr auto kOcrLocalCommandKey = "localCommand";
constexpr auto kOcrLocalLanguageKey = "localLanguage";
constexpr auto kOcrCloudEndpointKey = "cloudEndpoint";
constexpr auto kOcrCloudModelKey = "cloudModel";
constexpr auto kOcrCloudApiKeyKey = "cloudApiKey";
constexpr auto kOcrCloudPromptKey = "cloudPrompt";
constexpr auto kOcrCloudTimeoutSecondsKey = "cloudTimeoutSeconds";

QString settingsKeyForShortcutId(const QString& id) {
    QString key = id;
    key.replace('.', '/');
    return key;
}

}  // namespace

AppSettings::AppSettings(QString defaultSaveDirectory)
    : defaultSaveDirectory_(std::move(defaultSaveDirectory)) {
}

AppSettings::ShellSettings AppSettings::loadShellSettings() const {
    ShellSettings settings;
    QSettings& store = qsettings();

    store.beginGroup(kShellGroup);
    settings.startMinimized = store.value(kStartMinimizedKey, false).toBool();
    settings.closeToTray = store.value(kCloseToTrayKey, true).toBool();
    settings.globalHotkeysEnabled = store.value(kGlobalHotkeysEnabledKey, true).toBool();
    settings.defaultSaveDirectory = store.value(
        kDefaultSaveDirectoryKey,
        defaultSaveDirectory_
    ).toString();
    settings.appearanceMode = store.value(kAppearanceModeKey, settings.appearanceMode).toString();
    settings.interfaceLanguage = store.value(
        kInterfaceLanguageKey,
        settings.interfaceLanguage
    ).toString();
    settings.historyLimit = store.value(kHistoryLimitKey, settings.historyLimit).toInt();
    settings.mainWindowGeometry = store.value(kMainWindowGeometryKey).toByteArray();
    store.endGroup();

    store.beginGroup(kOcrGroup);
    settings.ocr.preferredProvider = store.value(
        kOcrPreferredProviderKey,
        settings.ocr.preferredProvider
    ).toString();
    settings.ocr.localCommand = store.value(kOcrLocalCommandKey, settings.ocr.localCommand).toString();
    settings.ocr.localLanguage = store.value(kOcrLocalLanguageKey, settings.ocr.localLanguage).toString();
    settings.ocr.cloudEndpoint = store.value(
        kOcrCloudEndpointKey,
        settings.ocr.cloudEndpoint
    ).toString();
    settings.ocr.cloudModel = store.value(kOcrCloudModelKey, settings.ocr.cloudModel).toString();
    settings.ocr.cloudApiKey = store.value(kOcrCloudApiKeyKey, settings.ocr.cloudApiKey).toString();
    settings.ocr.cloudPrompt = store.value(kOcrCloudPromptKey, settings.ocr.cloudPrompt).toString();
    settings.ocr.cloudTimeoutSeconds = store.value(
        kOcrCloudTimeoutSecondsKey,
        settings.ocr.cloudTimeoutSeconds
    ).toInt();
    store.endGroup();

    store.beginGroup(kShortcutGroup);
    for (const auto& definition : cappy::shortcuts::shortcutFieldDefinitions()) {
        const QString fallback = cappy::shortcuts::shortcutValue(settings.shortcuts, definition.id);
        cappy::shortcuts::setShortcutValue(
            &settings.shortcuts,
            definition.id,
            store.value(settingsKeyForShortcutId(definition.id), fallback).toString()
        );
    }
    store.endGroup();

    return settings;
}

void AppSettings::saveShellSettings(const ShellSettings& settings) {
    QSettings& store = qsettings();

    store.beginGroup(kShellGroup);
    store.setValue(kStartMinimizedKey, settings.startMinimized);
    store.setValue(kCloseToTrayKey, settings.closeToTray);
    store.setValue(kGlobalHotkeysEnabledKey, settings.globalHotkeysEnabled);
    store.setValue(kDefaultSaveDirectoryKey, settings.defaultSaveDirectory);
    store.setValue(kAppearanceModeKey, settings.appearanceMode);
    store.setValue(kInterfaceLanguageKey, settings.interfaceLanguage);
    store.setValue(kHistoryLimitKey, settings.historyLimit);
    store.setValue(kMainWindowGeometryKey, settings.mainWindowGeometry);
    store.endGroup();

    store.beginGroup(kOcrGroup);
    store.setValue(kOcrPreferredProviderKey, settings.ocr.preferredProvider);
    store.setValue(kOcrLocalCommandKey, settings.ocr.localCommand);
    store.setValue(kOcrLocalLanguageKey, settings.ocr.localLanguage);
    store.setValue(kOcrCloudEndpointKey, settings.ocr.cloudEndpoint);
    store.setValue(kOcrCloudModelKey, settings.ocr.cloudModel);
    store.setValue(kOcrCloudApiKeyKey, settings.ocr.cloudApiKey);
    store.setValue(kOcrCloudPromptKey, settings.ocr.cloudPrompt);
    store.setValue(kOcrCloudTimeoutSecondsKey, settings.ocr.cloudTimeoutSeconds);
    store.endGroup();

    store.beginGroup(kShortcutGroup);
    for (const auto& definition : cappy::shortcuts::shortcutFieldDefinitions()) {
        store.setValue(
            settingsKeyForShortcutId(definition.id),
            cappy::shortcuts::shortcutValue(settings.shortcuts, definition.id)
        );
    }
    store.endGroup();
    store.sync();
}

QSettings& AppSettings::qsettings() const {
    static QSettings settings;
    return settings;
}
