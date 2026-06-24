#include <QApplication>
#include <QComboBox>
#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QListWidget>
#include <QPainter>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QStackedWidget>

#include "SettingsDialog.h"

namespace {

QComboBox* findOcrProviderCombo(SettingsDialog& dialog) {
    const auto comboBoxes = dialog.findChildren<QComboBox*>();
    for (QComboBox* combo : comboBoxes) {
        if (combo->findData(QStringLiteral("local")) >= 0
            && combo->findData(QStringLiteral("cloud")) >= 0) {
            return combo;
        }
    }
    return nullptr;
}

}  // namespace

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    QString outputPath = argc >= 2
        ? QString::fromLocal8Bit(argv[1])
        : QStringLiteral("/tmp/cappy-settings-ocr.png");
    const QString provider = argc >= 3
        ? QString::fromLocal8Bit(argv[2]).trimmed().toLower()
        : QStringLiteral("local");
    const QString language = argc >= 4
        ? QString::fromLocal8Bit(argv[3]).trimmed()
        : QStringLiteral("zh-CN");

    AppSettings::ShellSettings settings;
    settings.appearanceMode = "dark";
    settings.interfaceLanguage = language;
    settings.defaultSaveDirectory = QDir::homePath();
    settings.ocr.preferredProvider = provider == "cloud" ? "cloud" : "local";
    settings.ocr.localCommand = "tesseract";
    settings.ocr.localLanguage = "eng+chi_sim";
    settings.ocr.cloudEndpoint = "https://api.example.com/v1/chat/completions";
    settings.ocr.cloudModel = "gpt-4.1-mini";
    settings.ocr.cloudApiKey = "sk-example";
    settings.ocr.cloudPrompt = "Recognize visible text only.";

    SettingsDialog dialog(
        settings,
        SettingsDialog::Diagnostics{
            .captureBackendSummary = "qt-screen",
            .hotkeyBackendSummary = "x11",
            .hotkeyBindingsSummary = "F1 screenshot",
            .hotkeyRegistrationErrors = {},
            .logFilePath = "/tmp/cappy.log",
        }
    );
    dialog.show();
    app.processEvents();

    auto* navigation = dialog.findChild<QListWidget*>("settingsNavigation");
    auto* pageStack = dialog.findChild<QStackedWidget*>("settingsPageStack");
    if (navigation != nullptr && pageStack != nullptr && pageStack->count() >= 4) {
        navigation->setCurrentRow(3);
        pageStack->setCurrentIndex(3);
        app.processEvents();
    }

    if (QComboBox* providerCombo = findOcrProviderCombo(dialog)) {
        const int index = providerCombo->findData(provider == "cloud" ? "cloud" : "local");
        if (index >= 0) {
            providerCombo->setCurrentIndex(index);
            app.processEvents();
        }
    }

    dialog.resize(980, 760);
    app.processEvents();

    QImage image(dialog.size() * dialog.devicePixelRatioF(), QImage::Format_ARGB32_Premultiplied);
    image.setDevicePixelRatio(dialog.devicePixelRatioF());
    image.fill(Qt::transparent);

    QPainter painter(&image);
    dialog.render(&painter);
    painter.end();

    QFileInfo outputInfo(outputPath);
    QDir().mkpath(outputInfo.absolutePath());
    image.save(outputPath);
    return 0;
}
