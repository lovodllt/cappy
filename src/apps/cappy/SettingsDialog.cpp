#include "SettingsDialog.h"

#include "ShellTheme.h"

#include <QAbstractButton>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QFont>
#include <QFormLayout>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QKeyEvent>
#include <QKeySequence>
#include <QKeySequenceEdit>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QStackedWidget>
#include <QSpinBox>
#include <QHash>
#include <QVBoxLayout>

#include <utility>

namespace {

struct ShortcutRowSpec {
    QString primaryId;
    QString secondaryId;
};

QWidget* createPage(QWidget* parent) {
    auto* page = new QWidget(parent);
    page->setObjectName("settingsPage");

    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(12);

    return page;
}

QLabel* createPageTitle(const QString& text, QWidget* parent) {
    auto* label = new QLabel(text, parent);
    label->setObjectName("settingsPageTitle");
    return label;
}

QFrame* createPanel(QWidget* parent) {
    auto* frame = new QFrame(parent);
    frame->setObjectName("settingsPanel");

    auto* layout = new QVBoxLayout(frame);
    layout->setContentsMargins(18, 18, 18, 18);
    layout->setSpacing(12);

    return frame;
}

QLabel* createPanelTitle(const QString& text, QWidget* parent) {
    auto* label = new QLabel(text, parent);
    label->setObjectName("settingsPanelTitle");
    return label;
}

QLabel* createFieldLabel(const QString& text, QWidget* parent) {
    auto* label = new QLabel(text, parent);
    label->setObjectName("settingsFieldLabel");
    return label;
}

QWidget* createShortcutEditorRow(QWidget* parent, QKeySequenceEdit* primaryEditor,
                                 QKeySequenceEdit* secondaryEditor = nullptr) {
    auto* row = new QWidget(parent);
    auto* layout = new QHBoxLayout(row);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    primaryEditor->setMinimumWidth(150);
    layout->addWidget(primaryEditor, 1);

    if (secondaryEditor != nullptr) {
        secondaryEditor->setMinimumWidth(150);
        layout->addWidget(secondaryEditor, 1);
    }

    return row;
}

} // namespace

SettingsDialog::SettingsDialog(const AppSettings::ShellSettings& settings, Diagnostics diagnostics,
                               QWidget* parent)
    : QDialog(parent), initialSettings_(settings), diagnostics_(std::move(diagnostics)),
      language_(cappy::localization::resolvedAppLanguageFromSettings(settings.interfaceLanguage)) {
    rootLayout_ = new QVBoxLayout(this);
    rebuildUi(settings);
}

void SettingsDialog::rebuildUi(const AppSettings::ShellSettings& settings, int currentPageIndex) {
    shortcutEditors_.clear();
    clearLayout(rootLayout_);

    titleLabel_ = nullptr;
    navigationList_ = nullptr;
    pageStack_ = nullptr;
    buttonBox_ = nullptr;
    startMinimizedCheckBox_ = nullptr;
    closeToTrayCheckBox_ = nullptr;
    globalHotkeysEnabledCheckBox_ = nullptr;
    appearanceModeComboBox_ = nullptr;
    interfaceLanguageComboBox_ = nullptr;
    ocrProviderComboBox_ = nullptr;
    ocrLocalPanel_ = nullptr;
    ocrCloudPanel_ = nullptr;
    defaultSaveDirectoryEdit_ = nullptr;
    ocrLocalCommandEdit_ = nullptr;
    ocrLocalLanguageEdit_ = nullptr;
    ocrCloudEndpointEdit_ = nullptr;
    ocrCloudModelEdit_ = nullptr;
    ocrCloudApiKeyEdit_ = nullptr;
    ocrCloudApiKeyVisibilityButton_ = nullptr;
    historyLimitSpinBox_ = nullptr;
    ocrCloudTimeoutSpinBox_ = nullptr;
    ocrCloudPromptEdit_ = nullptr;

    const auto& text = cappy::localization::strings(language_);
    setWindowTitle(text.settingsTitle);
    setModal(true);
    resize(980, 640);
    setMinimumSize(900, 580);
    setStyleSheet(
        settingsDialogStyleSheetForTheme(shellThemeModeFromSettings(settings.appearanceMode)));

    rootLayout_->setContentsMargins(16, 16, 16, 16);
    rootLayout_->setSpacing(12);

    titleLabel_ = new QLabel(text.settingsTitle, this);
    titleLabel_->setObjectName("settingsTitleLabel");
    QFont titleFont = titleLabel_->font();
    titleFont.setPointSize(titleFont.pointSize() + 5);
    titleFont.setBold(true);
    titleLabel_->setFont(titleFont);
    rootLayout_->addWidget(titleLabel_);

    auto* contentRow = new QHBoxLayout();
    contentRow->setContentsMargins(0, 0, 0, 0);
    contentRow->setSpacing(24);

    navigationList_ = new QListWidget(this);
    navigationList_->setObjectName("settingsNavigation");
    navigationList_->setFixedWidth(210);
    navigationList_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    navigationList_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    navigationList_->setSpacing(4);
    navigationList_->setFrameShape(QFrame::NoFrame);
    navigationList_->addItem(text.settingsGeneralTab);
    navigationList_->addItem(text.settingsShortcutsTab);
    navigationList_->addItem(text.settingsStorageTab);
    navigationList_->addItem(text.settingsOcrTab);

    pageStack_ = new QStackedWidget(this);
    pageStack_->setObjectName("settingsPageStack");

    auto* generalPage = createPage(pageStack_);
    auto* generalPageLayout = qobject_cast<QVBoxLayout*>(generalPage->layout());
    generalPageLayout->addWidget(createPageTitle(text.settingsGeneralTab, generalPage));

    auto* appearancePanel = createPanel(generalPage);
    auto* appearanceLayout = qobject_cast<QVBoxLayout*>(appearancePanel->layout());
    appearanceLayout->addWidget(createPanelTitle(text.settingsAppearanceCard, appearancePanel));

    auto* appearanceForm = new QFormLayout();
    appearanceForm->setContentsMargins(0, 0, 0, 0);
    appearanceForm->setSpacing(10);
    appearanceForm->setLabelAlignment(Qt::AlignLeft | Qt::AlignTop);

    appearanceModeComboBox_ = new QComboBox(appearancePanel);
    appearanceModeComboBox_->setObjectName("appearanceModeComboBox");
    appearanceModeComboBox_->addItem(text.settingsLightMode, "light");
    appearanceModeComboBox_->addItem(text.settingsDarkMode, "dark");
    interfaceLanguageComboBox_ = new QComboBox(appearancePanel);
    interfaceLanguageComboBox_->setObjectName("interfaceLanguageComboBox");
    interfaceLanguageComboBox_->addItem(
        cappy::localization::appLanguageOptionLabel(cappy::localization::AppLanguage::System,
                                                    language_),
        cappy::localization::appLanguageToSettingsValue(cappy::localization::AppLanguage::System));
    interfaceLanguageComboBox_->addItem(
        cappy::localization::appLanguageOptionLabel(cappy::localization::AppLanguage::English,
                                                    language_),
        cappy::localization::appLanguageToSettingsValue(cappy::localization::AppLanguage::English));
    interfaceLanguageComboBox_->addItem(
        cappy::localization::appLanguageOptionLabel(
            cappy::localization::AppLanguage::SimplifiedChinese, language_),
        cappy::localization::appLanguageToSettingsValue(
            cappy::localization::AppLanguage::SimplifiedChinese));
    appearanceModeComboBox_->setCurrentIndex(
        qMax(0, appearanceModeComboBox_->findData(settings.appearanceMode)));
    interfaceLanguageComboBox_->setCurrentIndex(
        qMax(0, interfaceLanguageComboBox_->findData(settings.interfaceLanguage)));
    appearanceForm->addRow(createFieldLabel(text.settingsPageStyleLabel, appearancePanel),
                           appearanceModeComboBox_);
    appearanceForm->addRow(createFieldLabel(text.settingsInterfaceLanguageLabel, appearancePanel),
                           interfaceLanguageComboBox_);
    appearanceLayout->addLayout(appearanceForm);

    auto* shellPanel = createPanel(generalPage);
    auto* shellLayout = qobject_cast<QVBoxLayout*>(shellPanel->layout());
    shellLayout->addWidget(createPanelTitle(text.settingsShellCard, shellPanel));

    startMinimizedCheckBox_ = new QCheckBox(text.settingsStartMinimized, shellPanel);
    startMinimizedCheckBox_->setChecked(settings.startMinimized);
    closeToTrayCheckBox_ = new QCheckBox(text.settingsCloseToTray, shellPanel);
    closeToTrayCheckBox_->setChecked(settings.closeToTray);
    shellLayout->addWidget(startMinimizedCheckBox_);
    shellLayout->addWidget(closeToTrayCheckBox_);
    shellLayout->addStretch(1);

    auto* generalPanelsRow = new QHBoxLayout();
    generalPanelsRow->setContentsMargins(0, 0, 0, 0);
    generalPanelsRow->setSpacing(12);
    generalPanelsRow->addWidget(appearancePanel, 1);
    generalPanelsRow->addWidget(shellPanel, 1);
    generalPageLayout->addLayout(generalPanelsRow);
    generalPageLayout->addStretch(1);

    auto* shortcutsPage = createPage(pageStack_);
    auto* shortcutsPageLayout = qobject_cast<QVBoxLayout*>(shortcutsPage->layout());
    shortcutsPageLayout->addWidget(createPageTitle(text.settingsShortcutsTab, shortcutsPage));

    auto* shortcutsScrollArea = new QScrollArea(shortcutsPage);
    shortcutsScrollArea->setObjectName("settingsScrollArea");
    shortcutsScrollArea->setWidgetResizable(true);
    shortcutsScrollArea->setFrameShape(QFrame::NoFrame);

    auto* shortcutsScrollContent = new QWidget(shortcutsScrollArea);
    shortcutsScrollContent->setObjectName("settingsScrollContent");
    auto* shortcutsScrollLayout = new QVBoxLayout(shortcutsScrollContent);
    shortcutsScrollLayout->setContentsMargins(0, 0, 0, 0);
    shortcutsScrollLayout->setSpacing(12);

    const auto createShortcutEditorForId = [this, &settings](const QString& id,
                                                             QWidget* parent) -> QKeySequenceEdit* {
        auto* editor = new QKeySequenceEdit(
            QKeySequence::fromString(cappy::shortcuts::shortcutValue(settings.shortcuts, id),
                                     QKeySequence::PortableText),
            parent);
        shortcutEditors_.insert(id, editor);
        editor->setProperty("recording", false);
        editor->installEventFilter(this);
        return editor;
    };

    const auto addShortcutCard = [this, &createShortcutEditorForId, shortcutsScrollContent, &text](
                                     const QString& title, const QList<ShortcutRowSpec>& rows,
                                     bool showGlobalToggle, bool showAlternateColumn) -> QFrame* {
        auto* card = createPanel(shortcutsScrollContent);
        auto* cardLayout = qobject_cast<QVBoxLayout*>(card->layout());
        cardLayout->addWidget(createPanelTitle(title, card));

        if (showGlobalToggle) {
            globalHotkeysEnabledCheckBox_ = new QCheckBox(text.settingsEnableGlobalHotkeys, card);
            cardLayout->addWidget(globalHotkeysEnabledCheckBox_);
        }

        if (showAlternateColumn) {
            auto* header = new QWidget(card);
            auto* headerLayout = new QHBoxLayout(header);
            headerLayout->setContentsMargins(0, 0, 0, 0);
            headerLayout->setSpacing(8);
            auto* spacer = new QLabel(header);
            spacer->setMinimumWidth(180);
            auto* primaryLabel = new QLabel(text.settingsPrimaryShortcut, header);
            auto* alternateLabel = new QLabel(text.settingsAlternateShortcut, header);
            primaryLabel->setObjectName("settingsValueLabel");
            alternateLabel->setObjectName("settingsValueLabel");
            primaryLabel->setMinimumWidth(150);
            alternateLabel->setMinimumWidth(150);
            headerLayout->addWidget(spacer);
            headerLayout->addWidget(primaryLabel, 1);
            headerLayout->addWidget(alternateLabel, 1);
            cardLayout->addWidget(header);
        }

        auto* form = new QFormLayout();
        form->setContentsMargins(0, 0, 0, 0);
        form->setSpacing(10);
        form->setLabelAlignment(Qt::AlignLeft | Qt::AlignTop);

        for (const auto& row : rows) {
            QKeySequenceEdit* primaryEditor = createShortcutEditorForId(row.primaryId, card);
            QKeySequenceEdit* secondaryEditor =
                row.secondaryId.isEmpty() ? nullptr
                                          : createShortcutEditorForId(row.secondaryId, card);
            form->addRow(createFieldLabel(
                             cappy::localization::shortcutLabel(language_, row.primaryId), card),
                         createShortcutEditorRow(card, primaryEditor, secondaryEditor));
        }

        cardLayout->addLayout(form);
        return card;
    };

    QFrame* entryCard = addShortcutCard(text.settingsShortcutsTab,
                                        {
                                            {"global.open_home", {}},
                                            {"global.screenshot", {}},
                                            {"main.region_capture", {}},
                                            {"main.fullscreen_capture", {}},
                                            {"main.active_window_capture", {}},
                                            {"main.pin_latest", {}},
                                            {"main.save_latest", {}},
                                            {"main.open_capture_folder", {}},
                                            {"main.settings", {}},
                                            {"main.hide_to_tray", {}},
                                            {"main.quit", {}},
                                            {"main.close_pins", {}},
                                            {"main.restore_pin_input", {}},
                                            {"main.history_pin", {}},
                                            {"main.history_copy", {}},
                                            {"main.history_save", {}},
                                            {"main.history_remove", {}},
                                        },
                                        true, false);
    if (globalHotkeysEnabledCheckBox_ != nullptr) {
        globalHotkeysEnabledCheckBox_->setChecked(settings.globalHotkeysEnabled);
    }

    QFrame* overlayCard = addShortcutCard(text.settingsSectionCaptureOverlay,
                                          {
                                              {"overlay.rectangle", "overlay.rectangle_alt"},
                                              {"overlay.ellipse", "overlay.ellipse_alt"},
                                              {"overlay.arrow", "overlay.arrow_alt"},
                                              {"overlay.pen", "overlay.pen_alt"},
                                              {"overlay.marker", "overlay.marker_alt"},
                                              {"overlay.mosaic", "overlay.mosaic_alt"},
                                              {"overlay.text", "overlay.text_alt"},
                                              {"overlay.serial", "overlay.serial_alt"},
                                              {"overlay.undo", {}},
                                              {"overlay.redo", {}},
                                              {"overlay.copy", "overlay.copy_alt"},
                                              {"overlay.quick_copy", {}},
                                              {"overlay.save", "overlay.save_alt"},
                                              {"overlay.pin", "overlay.pin_alt"},
                                              {"overlay.cancel", {}},
                                          },
                                          false, true);

    QFrame* editorCard = addShortcutCard(text.settingsSectionCaptureEditor,
                                         {
                                             {"editor.rectangle", {}},
                                             {"editor.ellipse", {}},
                                             {"editor.arrow", {}},
                                             {"editor.pen", {}},
                                             {"editor.marker", {}},
                                             {"editor.mosaic", {}},
                                             {"editor.text", {}},
                                             {"editor.serial", {}},
                                             {"editor.undo", {}},
                                             {"editor.redo", {}},
                                             {"editor.copy", {}},
                                             {"editor.copy_and_close", {}},
                                             {"editor.save", "editor.save_alt"},
                                             {"editor.pin", "editor.pin_alt"},
                                             {"editor.close", {}},
                                         },
                                         false, true);

    QFrame* pinWindowCard = addShortcutCard(text.settingsSectionPinnedWindow,
                                            {
                                                {"pin.close", {}},
                                                {"pin.scale_up", {}},
                                                {"pin.scale_down", {}},
                                                {"pin.reset_scale_opacity", {}},
                                                {"pin.opacity_down", {}},
                                                {"pin.opacity_up", {}},
                                                {"pin.toggle_lock", {}},
                                                {"pin.toggle_click_through", {}},
                                            },
                                            false, false);

    shortcutsScrollLayout->addWidget(entryCard);
    shortcutsScrollLayout->addWidget(overlayCard);
    shortcutsScrollLayout->addWidget(editorCard);
    shortcutsScrollLayout->addWidget(pinWindowCard);
    shortcutsScrollLayout->addStretch(1);
    shortcutsScrollArea->setWidget(shortcutsScrollContent);
    shortcutsPageLayout->addWidget(shortcutsScrollArea, 1);

    auto* storagePage = createPage(pageStack_);
    auto* storagePageLayout = qobject_cast<QVBoxLayout*>(storagePage->layout());
    storagePageLayout->addWidget(createPageTitle(text.settingsStorageTab, storagePage));

    auto* savePanel = createPanel(storagePage);
    auto* saveLayout = qobject_cast<QVBoxLayout*>(savePanel->layout());
    saveLayout->addWidget(createPanelTitle(text.settingsCaptureOutputCard, savePanel));
    auto* saveForm = new QFormLayout();
    saveForm->setContentsMargins(0, 0, 0, 0);
    saveForm->setSpacing(10);
    saveForm->setLabelAlignment(Qt::AlignLeft | Qt::AlignTop);

    auto* saveDirectoryRow = new QWidget(savePanel);
    auto* saveDirectoryLayout = new QHBoxLayout(saveDirectoryRow);
    saveDirectoryLayout->setContentsMargins(0, 0, 0, 0);
    saveDirectoryLayout->setSpacing(8);

    defaultSaveDirectoryEdit_ = new QLineEdit(settings.defaultSaveDirectory, saveDirectoryRow);
    defaultSaveDirectoryEdit_->setObjectName("defaultSaveDirectoryEdit");
    auto* browseButton = new QPushButton(text.settingsBrowse, saveDirectoryRow);
    saveDirectoryLayout->addWidget(defaultSaveDirectoryEdit_, 1);
    saveDirectoryLayout->addWidget(browseButton, 0);
    saveForm->addRow(createFieldLabel(text.settingsDefaultSaveDirectory, savePanel),
                     saveDirectoryRow);

    historyLimitSpinBox_ = new QSpinBox(savePanel);
    historyLimitSpinBox_->setObjectName("historyLimitSpinBox");
    historyLimitSpinBox_->setRange(1, 200);
    historyLimitSpinBox_->setValue(qMax(1, settings.historyLimit));
    saveForm->addRow(createFieldLabel(text.settingsHistoryLimit, savePanel), historyLimitSpinBox_);

    saveLayout->addLayout(saveForm);
    saveLayout->addStretch(1);
    storagePageLayout->addWidget(savePanel);
    storagePageLayout->addStretch(1);

    auto* ocrPage = createPage(pageStack_);
    auto* ocrPageLayout = qobject_cast<QVBoxLayout*>(ocrPage->layout());
    ocrPageLayout->addWidget(createPageTitle(text.settingsOcrTab, ocrPage));

    auto* ocrScrollArea = new QScrollArea(ocrPage);
    ocrScrollArea->setObjectName("settingsScrollArea");
    ocrScrollArea->setWidgetResizable(true);
    ocrScrollArea->setFrameShape(QFrame::NoFrame);

    auto* ocrScrollContent = new QWidget(ocrScrollArea);
    ocrScrollContent->setObjectName("settingsScrollContent");
    auto* ocrScrollLayout = new QVBoxLayout(ocrScrollContent);
    ocrScrollLayout->setContentsMargins(0, 0, 0, 0);
    ocrScrollLayout->setSpacing(12);

    auto* ocrGeneralPanel = createPanel(ocrScrollContent);
    auto* ocrGeneralLayout = qobject_cast<QVBoxLayout*>(ocrGeneralPanel->layout());
    ocrGeneralLayout->addWidget(createPanelTitle(text.settingsOcrGeneralCard, ocrGeneralPanel));
    auto* ocrGeneralForm = new QFormLayout();
    ocrGeneralForm->setContentsMargins(0, 0, 0, 0);
    ocrGeneralForm->setSpacing(10);
    ocrGeneralForm->setLabelAlignment(Qt::AlignLeft | Qt::AlignTop);

    ocrProviderComboBox_ = new QComboBox(ocrGeneralPanel);
    ocrProviderComboBox_->addItem(text.settingsOcrProviderLocal, "local");
    ocrProviderComboBox_->addItem(text.settingsOcrProviderCloud, "cloud");
    ocrProviderComboBox_->setCurrentIndex(
        qMax(0, ocrProviderComboBox_->findData(settings.ocr.preferredProvider)));
    ocrCloudTimeoutSpinBox_ = new QSpinBox(ocrGeneralPanel);
    ocrCloudTimeoutSpinBox_->setRange(5, 300);
    ocrCloudTimeoutSpinBox_->setValue(settings.ocr.cloudTimeoutSeconds);

    ocrGeneralForm->addRow(createFieldLabel(text.settingsOcrProviderLabel, ocrGeneralPanel),
                           ocrProviderComboBox_);
    ocrGeneralForm->addRow(createFieldLabel(text.settingsOcrTimeoutSeconds, ocrGeneralPanel),
                           ocrCloudTimeoutSpinBox_);
    ocrGeneralLayout->addLayout(ocrGeneralForm);

    ocrLocalPanel_ = createPanel(ocrScrollContent);
    auto* ocrLocalLayout = qobject_cast<QVBoxLayout*>(ocrLocalPanel_->layout());
    ocrLocalLayout->addWidget(createPanelTitle(text.settingsOcrLocalCard, ocrLocalPanel_));
    auto* ocrLocalForm = new QFormLayout();
    ocrLocalForm->setContentsMargins(0, 0, 0, 0);
    ocrLocalForm->setSpacing(10);
    ocrLocalForm->setLabelAlignment(Qt::AlignLeft | Qt::AlignTop);
    ocrLocalCommandEdit_ = new QLineEdit(settings.ocr.localCommand, ocrLocalPanel_);
    ocrLocalLanguageEdit_ = new QLineEdit(settings.ocr.localLanguage, ocrLocalPanel_);
    ocrLocalForm->addRow(createFieldLabel(text.settingsOcrLocalCommand, ocrLocalPanel_),
                         ocrLocalCommandEdit_);
    ocrLocalForm->addRow(createFieldLabel(text.settingsOcrLocalLanguage, ocrLocalPanel_),
                         ocrLocalLanguageEdit_);
    ocrLocalLayout->addLayout(ocrLocalForm);
    auto* ocrLocalHint = new QLabel(text.settingsOcrLocalHint, ocrLocalPanel_);
    ocrLocalHint->setObjectName("settingsValueLabel");
    ocrLocalHint->setWordWrap(true);
    ocrLocalLayout->addWidget(ocrLocalHint);

    ocrCloudPanel_ = createPanel(ocrScrollContent);
    auto* ocrCloudLayout = qobject_cast<QVBoxLayout*>(ocrCloudPanel_->layout());
    ocrCloudLayout->addWidget(createPanelTitle(text.settingsOcrCloudCard, ocrCloudPanel_));
    auto* ocrCloudForm = new QFormLayout();
    ocrCloudForm->setContentsMargins(0, 0, 0, 0);
    ocrCloudForm->setSpacing(10);
    ocrCloudForm->setLabelAlignment(Qt::AlignLeft | Qt::AlignTop);
    ocrCloudEndpointEdit_ = new QLineEdit(settings.ocr.cloudEndpoint, ocrCloudPanel_);
    ocrCloudModelEdit_ = new QLineEdit(settings.ocr.cloudModel, ocrCloudPanel_);
    auto* apiKeyRow = new QWidget(ocrCloudPanel_);
    auto* apiKeyLayout = new QHBoxLayout(apiKeyRow);
    apiKeyLayout->setContentsMargins(0, 0, 0, 0);
    apiKeyLayout->setSpacing(8);
    ocrCloudApiKeyEdit_ = new QLineEdit(settings.ocr.cloudApiKey, apiKeyRow);
    ocrCloudApiKeyEdit_->setEchoMode(QLineEdit::Password);
    ocrCloudApiKeyVisibilityButton_ = new QPushButton(text.settingsOcrApiKeyShow, apiKeyRow);
    ocrCloudApiKeyVisibilityButton_->setMinimumWidth(64);
    apiKeyLayout->addWidget(ocrCloudApiKeyEdit_, 1);
    apiKeyLayout->addWidget(ocrCloudApiKeyVisibilityButton_, 0);
    ocrCloudPromptEdit_ = new QPlainTextEdit(ocrCloudPanel_);
    ocrCloudPromptEdit_->setPlainText(settings.ocr.cloudPrompt);
    ocrCloudPromptEdit_->setFixedHeight(108);
    ocrCloudForm->addRow(createFieldLabel(text.settingsOcrCloudEndpoint, ocrCloudPanel_),
                         ocrCloudEndpointEdit_);
    ocrCloudForm->addRow(createFieldLabel(text.settingsOcrCloudModel, ocrCloudPanel_),
                         ocrCloudModelEdit_);
    ocrCloudForm->addRow(createFieldLabel(text.settingsOcrCloudApiKey, ocrCloudPanel_), apiKeyRow);
    ocrCloudForm->addRow(createFieldLabel(text.settingsOcrCloudPrompt, ocrCloudPanel_),
                         ocrCloudPromptEdit_);
    ocrCloudLayout->addLayout(ocrCloudForm);
    auto* ocrCloudHint = new QLabel(text.settingsOcrCloudHint, ocrCloudPanel_);
    ocrCloudHint->setObjectName("settingsValueLabel");
    ocrCloudHint->setWordWrap(true);
    ocrCloudLayout->addWidget(ocrCloudHint);

    auto* ocrPanelsColumn = new QVBoxLayout();
    ocrPanelsColumn->setContentsMargins(0, 0, 0, 0);
    ocrPanelsColumn->setSpacing(12);
    ocrPanelsColumn->addWidget(ocrLocalPanel_);
    ocrPanelsColumn->addWidget(ocrCloudPanel_);
    ocrScrollLayout->addWidget(ocrGeneralPanel);
    ocrScrollLayout->addLayout(ocrPanelsColumn);
    ocrScrollLayout->addStretch(1);
    ocrScrollArea->setWidget(ocrScrollContent);
    ocrPageLayout->addWidget(ocrScrollArea, 1);

    pageStack_->addWidget(generalPage);
    pageStack_->addWidget(shortcutsPage);
    pageStack_->addWidget(storagePage);
    pageStack_->addWidget(ocrPage);
    pageStack_->setCurrentIndex(qBound(0, currentPageIndex, pageStack_->count() - 1));
    navigationList_->setCurrentRow(pageStack_->currentIndex());

    contentRow->addWidget(navigationList_, 0);
    contentRow->addWidget(pageStack_, 1);
    rootLayout_->addLayout(contentRow, 1);

    buttonBox_ = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    if (buttonBox_->button(QDialogButtonBox::Ok) != nullptr) {
        buttonBox_->button(QDialogButtonBox::Ok)->setText(text.settingsSave);
        buttonBox_->button(QDialogButtonBox::Ok)->setIcon(QIcon());
    }
    if (buttonBox_->button(QDialogButtonBox::Cancel) != nullptr) {
        buttonBox_->button(QDialogButtonBox::Cancel)->setText(text.actionCancel);
        buttonBox_->button(QDialogButtonBox::Cancel)->setIcon(QIcon());
    }
    buttonBox_->setCenterButtons(false);
    rootLayout_->addWidget(buttonBox_);

    applySelectedTheme();
    syncShortcutEditorsEnabled();
    updateOcrFieldState();

    connect(navigationList_, &QListWidget::currentRowChanged, this, [this](int row) {
        if (pageStack_ != nullptr && row >= 0 && row < pageStack_->count()) {
            pageStack_->setCurrentIndex(row);
        }
    });

    connect(browseButton, &QPushButton::clicked, this, &SettingsDialog::chooseDefaultSaveDirectory);
    connect(appearanceModeComboBox_, &QComboBox::currentIndexChanged, this,
            &SettingsDialog::applySelectedTheme);
    connect(interfaceLanguageComboBox_, &QComboBox::currentIndexChanged, this,
            &SettingsDialog::applySelectedLanguage);
    connect(globalHotkeysEnabledCheckBox_, &QCheckBox::toggled, this,
            &SettingsDialog::syncShortcutEditorsEnabled);
    connect(ocrProviderComboBox_, &QComboBox::currentIndexChanged, this,
            &SettingsDialog::updateOcrFieldState);
    connect(ocrCloudApiKeyVisibilityButton_, &QPushButton::clicked, this, [this]() {
        if (ocrCloudApiKeyEdit_ == nullptr || ocrCloudApiKeyVisibilityButton_ == nullptr) {
            return;
        }
        const bool hidden = ocrCloudApiKeyEdit_->echoMode() == QLineEdit::Password;
        ocrCloudApiKeyEdit_->setEchoMode(hidden ? QLineEdit::Normal : QLineEdit::Password);
        const auto& text = cappy::localization::strings(language_);
        ocrCloudApiKeyVisibilityButton_->setText(hidden ? text.settingsOcrApiKeyHide
                                                        : text.settingsOcrApiKeyShow);
    });
    connect(buttonBox_, &QDialogButtonBox::accepted, this, &SettingsDialog::validateAndAccept);
    connect(buttonBox_, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

AppSettings::ShellSettings SettingsDialog::shellSettings() const {
    AppSettings::ShellSettings settings = initialSettings_;
    settings.startMinimized =
        startMinimizedCheckBox_ != nullptr && startMinimizedCheckBox_->isChecked();
    settings.closeToTray = closeToTrayCheckBox_ != nullptr && closeToTrayCheckBox_->isChecked();
    settings.globalHotkeysEnabled =
        globalHotkeysEnabledCheckBox_ != nullptr && globalHotkeysEnabledCheckBox_->isChecked();
    settings.defaultSaveDirectory = defaultSaveDirectoryEdit_ == nullptr
                                        ? initialSettings_.defaultSaveDirectory
                                        : defaultSaveDirectoryEdit_->text().trimmed();
    settings.appearanceMode = appearanceModeComboBox_ == nullptr
                                  ? initialSettings_.appearanceMode
                                  : appearanceModeComboBox_->currentData().toString();
    settings.interfaceLanguage = interfaceLanguageComboBox_ == nullptr
                                     ? initialSettings_.interfaceLanguage
                                     : interfaceLanguageComboBox_->currentData().toString();
    settings.historyLimit = historyLimitSpinBox_ == nullptr ? initialSettings_.historyLimit
                                                            : historyLimitSpinBox_->value();
    settings.ocr.preferredProvider = ocrProviderComboBox_ == nullptr
                                         ? initialSettings_.ocr.preferredProvider
                                         : ocrProviderComboBox_->currentData().toString();
    settings.ocr.localCommand = ocrLocalCommandEdit_ == nullptr
                                    ? initialSettings_.ocr.localCommand
                                    : ocrLocalCommandEdit_->text().trimmed();
    settings.ocr.localLanguage = ocrLocalLanguageEdit_ == nullptr
                                     ? initialSettings_.ocr.localLanguage
                                     : ocrLocalLanguageEdit_->text().trimmed();
    settings.ocr.cloudEndpoint = ocrCloudEndpointEdit_ == nullptr
                                     ? initialSettings_.ocr.cloudEndpoint
                                     : ocrCloudEndpointEdit_->text().trimmed();
    settings.ocr.cloudModel = ocrCloudModelEdit_ == nullptr ? initialSettings_.ocr.cloudModel
                                                            : ocrCloudModelEdit_->text().trimmed();
    settings.ocr.cloudApiKey = ocrCloudApiKeyEdit_ == nullptr
                                   ? initialSettings_.ocr.cloudApiKey
                                   : ocrCloudApiKeyEdit_->text().trimmed();
    settings.ocr.cloudPrompt = ocrCloudPromptEdit_ == nullptr
                                   ? initialSettings_.ocr.cloudPrompt
                                   : ocrCloudPromptEdit_->toPlainText().trimmed();
    settings.ocr.cloudTimeoutSeconds = ocrCloudTimeoutSpinBox_ == nullptr
                                           ? initialSettings_.ocr.cloudTimeoutSeconds
                                           : ocrCloudTimeoutSpinBox_->value();
    for (const auto& definition : cappy::shortcuts::shortcutFieldDefinitions()) {
        QKeySequenceEdit* editor = shortcutEditors_.value(definition.id, nullptr);
        cappy::shortcuts::setShortcutValue(
            &settings.shortcuts, definition.id,
            editor == nullptr
                ? cappy::shortcuts::shortcutValue(initialSettings_.shortcuts, definition.id)
                : editor->keySequence().toString(QKeySequence::PortableText));
    }
    return settings;
}

void SettingsDialog::chooseDefaultSaveDirectory() {
    const QString currentPath = defaultSaveDirectoryEdit_ == nullptr
                                    ? QString{}
                                    : defaultSaveDirectoryEdit_->text().trimmed();
    const QString selectedPath = QFileDialog::getExistingDirectory(
        this, cappy::localization::strings(language_).settingsChooseDefaultSaveDirectory,
        currentPath);
    if (!selectedPath.isEmpty() && defaultSaveDirectoryEdit_ != nullptr) {
        defaultSaveDirectoryEdit_->setText(QDir::toNativeSeparators(selectedPath));
    }
}

void SettingsDialog::validateAndAccept() {
    const auto& text = cappy::localization::strings(language_);
    AppSettings::ShellSettings settings = shellSettings();
    if (settings.defaultSaveDirectory.isEmpty()) {
        QMessageBox::warning(this, text.settingsWarningTitle, text.settingsDefaultSaveDirEmpty);
        return;
    }

    QFileInfo directoryInfo(settings.defaultSaveDirectory);
    if (!directoryInfo.exists() || !directoryInfo.isDir()) {
        QMessageBox::warning(this, text.settingsWarningTitle, text.settingsDefaultSaveDirMissing);
        return;
    }

    if (settings.ocr.preferredProvider == "local" && settings.ocr.localCommand.isEmpty()) {
        QMessageBox::warning(this, text.settingsWarningTitle, text.settingsOcrLocalCommandEmpty);
        return;
    }
    if (settings.ocr.preferredProvider == "cloud") {
        if (settings.ocr.cloudEndpoint.isEmpty()) {
            QMessageBox::warning(this, text.settingsWarningTitle,
                                 text.settingsOcrCloudEndpointEmpty);
            return;
        }
        if (settings.ocr.cloudModel.isEmpty()) {
            QMessageBox::warning(this, text.settingsWarningTitle, text.settingsOcrCloudModelEmpty);
            return;
        }
        if (settings.ocr.cloudApiKey.isEmpty()) {
            QMessageBox::warning(this, text.settingsWarningTitle, text.settingsOcrCloudApiKeyEmpty);
            return;
        }
    }

    for (const auto& definition : cappy::shortcuts::shortcutFieldDefinitions()) {
        const QKeySequence sequence = QKeySequence::fromString(
            cappy::shortcuts::shortcutValue(settings.shortcuts, definition.id),
            QKeySequence::PortableText);
        if (!sequence.isEmpty() && sequence.count() != 1) {
            QMessageBox::warning(this, text.settingsWarningTitle,
                                 text.settingsShortcutSingleTemplate.arg(
                                     cappy::localization::shortcutLabel(language_, definition.id)));
            return;
        }
    }

    QHash<int, QHash<QString, QString>> seenShortcutsByScope;
    for (const auto& definition : cappy::shortcuts::shortcutFieldDefinitions()) {
        const QString normalized = QKeySequence::fromString(cappy::shortcuts::shortcutValue(
                                                                settings.shortcuts, definition.id),
                                                            QKeySequence::PortableText)
                                       .toString(QKeySequence::PortableText);
        if (normalized.isEmpty()) {
            continue;
        }

        QHash<QString, QString>& scopeEntries =
            seenShortcutsByScope[static_cast<int>(definition.scope)];
        const QString existingLabel = scopeEntries.value(normalized);
        if (!existingLabel.isEmpty()) {
            QMessageBox::warning(
                this, text.settingsWarningTitle,
                text.settingsDuplicateShortcutTemplate.arg(
                    cappy::localization::shortcutScopeLabel(language_, definition.scope),
                    existingLabel, cappy::localization::shortcutLabel(language_, definition.id),
                    normalized));
            return;
        }
        scopeEntries.insert(normalized,
                            cappy::localization::shortcutLabel(language_, definition.id));
    }

    accept();
}

void SettingsDialog::applySelectedTheme() {
    const QString appearanceMode = appearanceModeComboBox_ == nullptr
                                       ? initialSettings_.appearanceMode
                                       : appearanceModeComboBox_->currentData().toString();
    setStyleSheet(settingsDialogStyleSheetForTheme(shellThemeModeFromSettings(appearanceMode)));
}

void SettingsDialog::applySelectedLanguage() {
    if (interfaceLanguageComboBox_ == nullptr) {
        return;
    }

    language_ = cappy::localization::resolvedAppLanguageFromSettings(
        interfaceLanguageComboBox_->currentData().toString());
}

void SettingsDialog::clearLayout(QLayout* layout) {
    if (layout == nullptr) {
        return;
    }

    while (QLayoutItem* item = layout->takeAt(0)) {
        if (QWidget* widget = item->widget()) {
            const auto descendants = widget->findChildren<QWidget*>();
            for (QWidget* child : descendants) {
                if (child == nullptr) {
                    continue;
                }
                child->removeEventFilter(this);
                child->disconnect(this);
            }
            widget->hide();
            widget->removeEventFilter(this);
            widget->disconnect(this);
            widget->setParent(nullptr);
            widget->deleteLater();
        } else if (QLayout* childLayout = item->layout()) {
            clearLayout(childLayout);
            delete childLayout;
        }
        delete item;
    }
}

bool SettingsDialog::eventFilter(QObject* watched, QEvent* event) {
    auto* editor = qobject_cast<QKeySequenceEdit*>(watched);
    if (editor == nullptr) {
        return QDialog::eventFilter(watched, event);
    }

    switch (event->type()) {
    case QEvent::ShortcutOverride:
        static_cast<QKeyEvent*>(event)->accept();
        return true;
    case QEvent::FocusIn:
    case QEvent::MouseButtonPress:
        setShortcutEditorRecording(editor, true);
        break;
    case QEvent::FocusOut:
        setShortcutEditorRecording(editor, false);
        break;
    default:
        break;
    }

    return QDialog::eventFilter(watched, event);
}

void SettingsDialog::syncShortcutEditorsEnabled() {
    const bool enabled =
        globalHotkeysEnabledCheckBox_ != nullptr && globalHotkeysEnabledCheckBox_->isChecked();
    for (const auto& definition : cappy::shortcuts::shortcutFieldDefinitions()) {
        if (definition.scope != cappy::shortcuts::ShortcutScope::Global) {
            continue;
        }

        QKeySequenceEdit* editor = shortcutEditors_.value(definition.id, nullptr);
        if (editor != nullptr) {
            editor->setEnabled(enabled);
        }
    }
}

void SettingsDialog::updateOcrFieldState() {
    const bool cloudSelected = ocrProviderComboBox_ != nullptr &&
                               ocrProviderComboBox_->currentData().toString() == "cloud";

    if (ocrLocalPanel_ != nullptr) {
        ocrLocalPanel_->setVisible(!cloudSelected);
    }
    if (ocrCloudPanel_ != nullptr) {
        ocrCloudPanel_->setVisible(cloudSelected);
    }

    if (ocrLocalCommandEdit_ != nullptr) {
        ocrLocalCommandEdit_->setEnabled(!cloudSelected);
    }
    if (ocrLocalLanguageEdit_ != nullptr) {
        ocrLocalLanguageEdit_->setEnabled(!cloudSelected);
    }
    if (ocrCloudEndpointEdit_ != nullptr) {
        ocrCloudEndpointEdit_->setEnabled(cloudSelected);
    }
    if (ocrCloudModelEdit_ != nullptr) {
        ocrCloudModelEdit_->setEnabled(cloudSelected);
    }
    if (ocrCloudApiKeyEdit_ != nullptr) {
        ocrCloudApiKeyEdit_->setEnabled(cloudSelected);
    }
    if (ocrCloudApiKeyVisibilityButton_ != nullptr) {
        ocrCloudApiKeyVisibilityButton_->setEnabled(cloudSelected);
        const auto& text = cappy::localization::strings(language_);
        ocrCloudApiKeyVisibilityButton_->setText(
            (ocrCloudApiKeyEdit_ != nullptr && ocrCloudApiKeyEdit_->echoMode() == QLineEdit::Normal)
                ? text.settingsOcrApiKeyHide
                : text.settingsOcrApiKeyShow);
    }
    if (ocrCloudPromptEdit_ != nullptr) {
        ocrCloudPromptEdit_->setEnabled(cloudSelected);
    }
}

void SettingsDialog::setShortcutEditorRecording(QKeySequenceEdit* editor, bool recording) {
    if (editor == nullptr) {
        return;
    }

    QLineEdit* display = editor->findChild<QLineEdit*>();
    editor->setProperty("recording", recording);
    editor->setStyleSheet(recording ? shortcutEditorRecordingStyleSheet() : QString{});
    if (display != nullptr) {
        display->setStyleSheet(recording ? shortcutEditorRecordingStyleSheet() : QString{});
    }
    if (recording) {
        editor->grabKeyboard();
    } else {
        editor->releaseKeyboard();
    }
    editor->update();
}

QString SettingsDialog::shortcutEditorRecordingStyleSheet() const {
    const QString appearanceMode = appearanceModeComboBox_ == nullptr
                                       ? initialSettings_.appearanceMode
                                       : appearanceModeComboBox_->currentData().toString();
    if (shellThemeModeFromSettings(appearanceMode) == ShellThemeMode::Dark) {
        return QStringLiteral("QKeySequenceEdit, QLineEdit {"
                              "  border: 2px solid #68a9e2;"
                              "  border-radius: 6px;"
                              "  padding: 0 9px;"
                              "  background: #2c4358;"
                              "  color: #f4f9ff;"
                              "  selection-background-color: #7fb4e4;"
                              "}"
                              "QLineEdit {"
                              "  font-weight: 600;"
                              "}");
    }

    return QStringLiteral("QKeySequenceEdit, QLineEdit {"
                          "  border: 2px solid #4f91d0;"
                          "  border-radius: 6px;"
                          "  padding: 0 9px;"
                          "  background: #dceeff;"
                          "  color: #0e3557;"
                          "  selection-background-color: #8ab8e2;"
                          "}"
                          "QLineEdit {"
                          "  font-weight: 600;"
                          "}");
}
