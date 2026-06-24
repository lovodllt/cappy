#pragma once

#include <QByteArray>
#include <QString>

#include "cappy/shortcuts/ShortcutSettings.h"
#include "cappy/services/ocr/OcrTypes.h"

class QSettings;

class AppSettings {
  public:
    struct ShellSettings {
        bool startMinimized = false;
        bool closeToTray = true;
        bool globalHotkeysEnabled = true;
        QString defaultSaveDirectory;
        QString appearanceMode = "light";
        QString interfaceLanguage = "system";
        int historyLimit = 10;
        cappy::services::ocr::OcrSettings ocr;
        cappy::shortcuts::ShortcutSettings shortcuts;
        QByteArray mainWindowGeometry;
    };

    explicit AppSettings(QString defaultSaveDirectory);

    ShellSettings loadShellSettings() const;
    void saveShellSettings(const ShellSettings& settings);

  private:
    QSettings& qsettings() const;

    QString defaultSaveDirectory_;
};
