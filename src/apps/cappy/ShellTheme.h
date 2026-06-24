#pragma once

#include <QString>

enum class ShellThemeMode {
    Light,
    Dark,
};

ShellThemeMode shellThemeModeFromSettings(const QString& value);
QString shellThemeModeToSettingsValue(ShellThemeMode mode);
QString mainWindowStyleSheetForTheme(ShellThemeMode mode);
QString settingsDialogStyleSheetForTheme(ShellThemeMode mode);
