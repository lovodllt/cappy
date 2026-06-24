#pragma once

#include <QDialog>
#include <QEvent>
#include <QMap>
#include <QString>
#include <QStringList>

#include "AppSettings.h"
#include "cappy/localization/Localization.h"

class QCheckBox;
class QComboBox;
class QDialogButtonBox;
class QFrame;
class QKeySequenceEdit;
class QLabel;
class QLayout;
class QLineEdit;
class QListWidget;
class QPlainTextEdit;
class QPushButton;
class QStackedWidget;
class QSpinBox;
class QVBoxLayout;

class SettingsDialog final : public QDialog {
    Q_OBJECT

public:
    struct Diagnostics {
        QString captureBackendSummary;
        QString hotkeyBackendSummary;
        QString hotkeyBindingsSummary;
        QStringList hotkeyRegistrationErrors;
        QString logFilePath;
    };

    SettingsDialog(
        const AppSettings::ShellSettings& settings,
        Diagnostics diagnostics,
        QWidget* parent = nullptr
    );

    [[nodiscard]] AppSettings::ShellSettings shellSettings() const;

private slots:
    void chooseDefaultSaveDirectory();
    void validateAndAccept();
    void applySelectedTheme();
    void applySelectedLanguage();
    void syncShortcutEditorsEnabled();
    void updateOcrFieldState();

private:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void rebuildUi(const AppSettings::ShellSettings& settings, int currentPageIndex = 0);
    void clearLayout(QLayout* layout);
    void setShortcutEditorRecording(QKeySequenceEdit* editor, bool recording);
    [[nodiscard]] QString shortcutEditorRecordingStyleSheet() const;

    AppSettings::ShellSettings initialSettings_;
    Diagnostics diagnostics_;
    cappy::localization::AppLanguage language_ = cappy::localization::AppLanguage::English;
    QVBoxLayout* rootLayout_ = nullptr;
    QLabel* titleLabel_ = nullptr;
    QListWidget* navigationList_ = nullptr;
    QStackedWidget* pageStack_ = nullptr;
    QDialogButtonBox* buttonBox_ = nullptr;
    QCheckBox* startMinimizedCheckBox_ = nullptr;
    QCheckBox* closeToTrayCheckBox_ = nullptr;
    QCheckBox* globalHotkeysEnabledCheckBox_ = nullptr;
    QComboBox* appearanceModeComboBox_ = nullptr;
    QComboBox* interfaceLanguageComboBox_ = nullptr;
    QComboBox* ocrProviderComboBox_ = nullptr;
    QFrame* ocrLocalPanel_ = nullptr;
    QFrame* ocrCloudPanel_ = nullptr;
    QLineEdit* defaultSaveDirectoryEdit_ = nullptr;
    QLineEdit* ocrLocalCommandEdit_ = nullptr;
    QLineEdit* ocrLocalLanguageEdit_ = nullptr;
    QLineEdit* ocrCloudEndpointEdit_ = nullptr;
    QLineEdit* ocrCloudModelEdit_ = nullptr;
    QLineEdit* ocrCloudApiKeyEdit_ = nullptr;
    QPushButton* ocrCloudApiKeyVisibilityButton_ = nullptr;
    QMap<QString, QKeySequenceEdit*> shortcutEditors_;
    QSpinBox* historyLimitSpinBox_ = nullptr;
    QSpinBox* ocrCloudTimeoutSpinBox_ = nullptr;
    QPlainTextEdit* ocrCloudPromptEdit_ = nullptr;
};
