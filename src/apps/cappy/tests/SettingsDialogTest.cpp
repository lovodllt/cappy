#include <QtTest>

#include "SettingsDialog.h"

#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QStackedWidget>
#include <QSpinBox>

class SettingsDialogTest final : public QObject {
    Q_OBJECT

  private slots:
    void switchingLanguageKeepsUnsavedValuesAndUpdatesSettings();
    void ocrProviderSwitchesRelevantFields();
};

void SettingsDialogTest::switchingLanguageKeepsUnsavedValuesAndUpdatesSettings() {
    AppSettings::ShellSettings settings;
    settings.appearanceMode = "light";
    settings.interfaceLanguage = "zh-CN";
    settings.defaultSaveDirectory = "/tmp/original";
    settings.historyLimit = 12;
    settings.startMinimized = true;
    settings.closeToTray = false;

    SettingsDialog dialog(settings, SettingsDialog::Diagnostics{
                                        .captureBackendSummary = "capture",
                                        .hotkeyBackendSummary = "hotkey",
                                        .hotkeyBindingsSummary = "bindings",
                                        .hotkeyRegistrationErrors = {},
                                        .logFilePath = "/tmp/cappy.log",
                                    });
    dialog.show();
    QCoreApplication::processEvents();

    auto* titleLabel = dialog.findChild<QLabel*>("settingsTitleLabel");
    auto* languageComboBox = dialog.findChild<QComboBox*>("interfaceLanguageComboBox");
    auto* saveDirectoryEdit = dialog.findChild<QLineEdit*>("defaultSaveDirectoryEdit");
    auto* historyLimitSpinBox = dialog.findChild<QSpinBox*>("historyLimitSpinBox");
    auto* navigation = dialog.findChild<QListWidget*>("settingsNavigation");
    auto* pageStack = dialog.findChild<QStackedWidget*>("settingsPageStack");

    QVERIFY(titleLabel != nullptr);
    QVERIFY(languageComboBox != nullptr);
    QVERIFY(saveDirectoryEdit != nullptr);
    QVERIFY(historyLimitSpinBox != nullptr);
    QVERIFY(navigation != nullptr);
    QVERIFY(pageStack != nullptr);

    QCOMPARE(dialog.windowTitle(), QString("设置"));
    QCOMPARE(titleLabel->text(), QString("设置"));

    saveDirectoryEdit->setText("/tmp/changed");
    historyLimitSpinBox->setValue(37);

    const int englishIndex = languageComboBox->findData(QString("en"));
    QVERIFY(englishIndex >= 0);
    languageComboBox->setCurrentIndex(englishIndex);
    QCoreApplication::processEvents();

    titleLabel = dialog.findChild<QLabel*>("settingsTitleLabel");
    languageComboBox = dialog.findChild<QComboBox*>("interfaceLanguageComboBox");
    saveDirectoryEdit = dialog.findChild<QLineEdit*>("defaultSaveDirectoryEdit");
    historyLimitSpinBox = dialog.findChild<QSpinBox*>("historyLimitSpinBox");
    navigation = dialog.findChild<QListWidget*>("settingsNavigation");
    pageStack = dialog.findChild<QStackedWidget*>("settingsPageStack");

    QVERIFY(titleLabel != nullptr);
    QVERIFY(languageComboBox != nullptr);
    QVERIFY(saveDirectoryEdit != nullptr);
    QVERIFY(historyLimitSpinBox != nullptr);
    QVERIFY(navigation != nullptr);
    QVERIFY(pageStack != nullptr);

    QCOMPARE(dialog.windowTitle(), QString("设置"));
    QCOMPARE(titleLabel->text(), QString("设置"));
    QCOMPARE(navigation->item(0)->text(), QString("常规"));
    QCOMPARE(pageStack->count(), 4);
    QCOMPARE(languageComboBox->currentData().toString(), QString("en"));
    QCOMPARE(saveDirectoryEdit->text(), QString("/tmp/changed"));
    QCOMPARE(historyLimitSpinBox->value(), 37);

    const AppSettings::ShellSettings result = dialog.shellSettings();
    QCOMPARE(result.interfaceLanguage, QString("en"));
    QCOMPARE(result.defaultSaveDirectory, QString("/tmp/changed"));
    QCOMPARE(result.historyLimit, 37);
    QCOMPARE(result.startMinimized, true);
    QCOMPARE(result.closeToTray, false);
}

void SettingsDialogTest::ocrProviderSwitchesRelevantFields() {
    AppSettings::ShellSettings settings;
    settings.defaultSaveDirectory = "/tmp";
    settings.ocr.preferredProvider = "local";
    settings.ocr.localCommand = "tesseract";
    settings.ocr.localLanguage = "eng+chi_sim";
    settings.ocr.cloudEndpoint = "https://example.com/v1/chat/completions";
    settings.ocr.cloudModel = "model-a";
    settings.ocr.cloudApiKey = "secret";
    settings.ocr.cloudPrompt = "prompt";

    SettingsDialog dialog(settings, SettingsDialog::Diagnostics{
                                        .captureBackendSummary = "capture",
                                        .hotkeyBackendSummary = "hotkey",
                                        .hotkeyBindingsSummary = "bindings",
                                        .hotkeyRegistrationErrors = {},
                                        .logFilePath = "/tmp/cappy.log",
                                    });
    dialog.show();
    QCoreApplication::processEvents();

    const auto comboBoxes = dialog.findChildren<QComboBox*>();
    QComboBox* providerCombo = nullptr;
    for (QComboBox* combo : comboBoxes) {
        if (combo->findData(QString("local")) >= 0 && combo->findData(QString("cloud")) >= 0) {
            providerCombo = combo;
            break;
        }
    }

    const auto lineEdits = dialog.findChildren<QLineEdit*>();
    QLineEdit* localCommandEdit = nullptr;
    QLineEdit* localLanguageEdit = nullptr;
    QLineEdit* cloudEndpointEdit = nullptr;
    QLineEdit* cloudModelEdit = nullptr;
    QLineEdit* cloudApiKeyEdit = nullptr;
    for (QLineEdit* edit : lineEdits) {
        if (edit->text() == "tesseract") {
            localCommandEdit = edit;
        } else if (edit->text() == "eng+chi_sim") {
            localLanguageEdit = edit;
        } else if (edit->text() == "https://example.com/v1/chat/completions") {
            cloudEndpointEdit = edit;
        } else if (edit->text() == "model-a") {
            cloudModelEdit = edit;
        } else if (edit->text() == "secret") {
            cloudApiKeyEdit = edit;
        }
    }
    auto* cloudPromptEdit = dialog.findChild<QPlainTextEdit*>();
    auto* visibilityButton = dialog.findChild<QPushButton*>();

    QVERIFY(providerCombo != nullptr);
    QVERIFY(localCommandEdit != nullptr);
    QVERIFY(localLanguageEdit != nullptr);
    QVERIFY(cloudEndpointEdit != nullptr);
    QVERIFY(cloudModelEdit != nullptr);
    QVERIFY(cloudApiKeyEdit != nullptr);
    QVERIFY(cloudPromptEdit != nullptr);
    QVERIFY(visibilityButton != nullptr);

    QVERIFY(localCommandEdit->isEnabled());
    QVERIFY(localLanguageEdit->isEnabled());
    QVERIFY(!cloudEndpointEdit->isEnabled());
    QVERIFY(!cloudModelEdit->isEnabled());
    QVERIFY(!cloudApiKeyEdit->isEnabled());
    QVERIFY(!cloudPromptEdit->isEnabled());
    QVERIFY(!visibilityButton->isEnabled());

    providerCombo->setCurrentIndex(providerCombo->findData(QString("cloud")));
    QCoreApplication::processEvents();

    QVERIFY(!localCommandEdit->isEnabled());
    QVERIFY(!localLanguageEdit->isEnabled());
    QVERIFY(cloudEndpointEdit->isEnabled());
    QVERIFY(cloudModelEdit->isEnabled());
    QVERIFY(cloudApiKeyEdit->isEnabled());
    QVERIFY(cloudPromptEdit->isEnabled());
    QVERIFY(visibilityButton->isEnabled());
}

QTEST_MAIN(SettingsDialogTest)

#include "SettingsDialogTest.moc"
