#include "MainWindow.h"

#include "ShellTheme.h"

#include <cmath>

#include <QAction>
#include <QAbstractItemView>
#include <QCloseEvent>
#include <QFileInfo>
#include <QFont>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidgetItem>
#include <QMenu>
#include <QPainter>
#include <QPen>
#include <QPixmap>
#include <QPushButton>
#include <QResizeEvent>
#include <QSizePolicy>
#include <QSplitter>
#include <QStyle>
#include <QString>
#include <QToolButton>
#include <QVBoxLayout>

namespace {

enum HistoryItemRole {
    ImageRole = Qt::UserRole,
    FilePathRole = Qt::UserRole + 1,
    CaptureModeRole = Qt::UserRole + 2,
    EntryIdRole = Qt::UserRole + 3,
    BaseTitleRole = Qt::UserRole + 4,
};

QString historyModeLabel(cappy::localization::AppLanguage language, int captureMode) {
    const auto& text = cappy::localization::strings(language);
    Q_UNUSED(captureMode);
    switch (captureMode) {
    case 0:
        return text.captureModeRegion;
    case 1:
        return text.captureModeFullscreen;
    case 2:
        return text.captureModeActiveWindow;
    case 3:
        return text.captureModeCurrentScreen;
    case 4:
        return text.captureModeWindowFit;
    default:
        return text.captureModeUnknown;
    }
}

QIcon themedIcon(QWidget* widget, const QString& themeName, QStyle::StandardPixmap fallback) {
    const QIcon themeIcon = QIcon::fromTheme(themeName);
    if (!themeIcon.isNull()) {
        return themeIcon;
    }

    return widget->style()->standardIcon(fallback);
}

QIcon drawHeaderIcon(const QString& id, const QSize& size = QSize(18, 18)) {
    QPixmap pixmap(size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(QColor(245, 245, 245), 1.7, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.setBrush(Qt::NoBrush);

    const qreal w = size.width();
    const qreal h = size.height();
    const QPointF center(w / 2.0, h / 2.0);

    if (id == "settings") {
        painter.drawEllipse(center, 2.6, 2.6);
        painter.drawEllipse(center, 6.4, 6.4);
        for (int i = 0; i < 8; ++i) {
            const qreal angle = static_cast<qreal>(i) * M_PI / 4.0;
            const QPointF inner(
                center.x() + std::cos(angle) * 4.8,
                center.y() + std::sin(angle) * 4.8
            );
            const QPointF outer(
                center.x() + std::cos(angle) * 7.4,
                center.y() + std::sin(angle) * 7.4
            );
            painter.drawLine(inner, outer);
        }
    }

    return QIcon(pixmap);
}

QToolButton* createToolButton(QWidget* parent, QAction* action) {
    auto* button = new QToolButton(parent);
    button->setDefaultAction(action);
    button->setToolButtonStyle(Qt::ToolButtonIconOnly);
    button->setIconSize(QSize(18, 18));
    button->setFixedSize(34, 34);
    button->setCursor(Qt::PointingHandCursor);
    return button;
}

QFrame* createSectionSurface(QWidget* parent, const char* objectName) {
    auto* frame = new QFrame(parent);
    frame->setObjectName(objectName);
    frame->setFrameShape(QFrame::NoFrame);
    return frame;
}

void applyActionShortcut(QAction* action, const QKeySequence& shortcut) {
    if (action == nullptr) {
        return;
    }

    action->setShortcut(shortcut);
    const QString tooltip = shortcut.isEmpty()
        ? action->text()
        : QString("%1 (%2)").arg(action->text(), shortcut.toString(QKeySequence::NativeText));
    action->setToolTip(tooltip);
    action->setStatusTip(tooltip);
}

}  // namespace

MainWindow::MainWindow(const QStringList& pluginIds, QWidget* parent)
    : QMainWindow(parent) {
    Q_UNUSED(pluginIds);

    setWindowTitle("Cappy");
    resize(1180, 760);
    setMinimumSize(980, 640);

    auto* root = new QWidget(this);
    auto* layout = new QVBoxLayout(root);
    layout->setContentsMargins(20, 18, 20, 18);
    layout->setSpacing(14);

    auto* headerRow = new QHBoxLayout();
    headerRow->setSpacing(8);

    titleLabel_ = new QLabel("Cappy", root);
    QFont titleFont = titleLabel_->font();
    titleFont.setPointSize(titleFont.pointSize() + 7);
    titleFont.setBold(true);
    titleLabel_->setFont(titleFont);
    titleLabel_->setObjectName("sectionTitleLabel");
    headerRow->addWidget(titleLabel_);
    headerRow->addStretch(1);

    auto* splitter = new QSplitter(Qt::Horizontal, root);
    splitter->setChildrenCollapsible(false);
    splitter->setHandleWidth(8);

    auto* workspacePanel = createSectionSurface(splitter, "sectionSurface");
    auto* workspaceLayout = new QVBoxLayout(workspacePanel);
    workspaceLayout->setContentsMargins(16, 16, 16, 16);
    workspaceLayout->setSpacing(14);

    auto* workspaceHeader = new QVBoxLayout();
    workspaceHeader->setSpacing(4);
    workspaceTitleLabel_ = new QLabel("Capture Workspace", workspacePanel);
    workspaceTitleLabel_->setObjectName("sectionTitleLabel");
    QFont workspaceTitleFont = workspaceTitleLabel_->font();
    workspaceTitleFont.setPointSize(workspaceTitleFont.pointSize() + 2);
    workspaceTitleFont.setBold(true);
    workspaceTitleLabel_->setFont(workspaceTitleFont);
    workspaceHeader->addWidget(workspaceTitleLabel_);

    statusLabel_ = new QLabel("Ready", workspacePanel);
    statusLabel_->setObjectName("statusLabel");
    statusLabel_->setWordWrap(true);

    previewLabel_ = new QLabel(workspacePanel);
    previewLabel_->setObjectName("previewSurface");
    previewLabel_->setMinimumSize(420, 280);
    previewLabel_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    previewLabel_->setAlignment(Qt::AlignCenter);
    previewLabel_->setText("Capture preview");

    historyList_ = new QListWidget(splitter);
    historyList_->setMinimumWidth(300);
    historyList_->setIconSize(QSize(84, 56));
    historyList_->setContextMenuPolicy(Qt::CustomContextMenu);
    historyList_->setSelectionMode(QAbstractItemView::SingleSelection);
    historyList_->setAlternatingRowColors(false);
    historyList_->setSelectionBehavior(QAbstractItemView::SelectRows);
    historyList_->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    historyList_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    historyList_->setUniformItemSizes(true);
    historyList_->setSpacing(2);

    regionCaptureAction_ = createWindowAction(
        themedIcon(this, "transform-crop", QStyle::SP_FileDialogDetailedView),
        "Region Capture",
        QKeySequence("Ctrl+Shift+A")
    );
    fullscreenCaptureAction_ = createWindowAction(
        themedIcon(this, "view-fullscreen", QStyle::SP_TitleBarMaxButton),
        "Fullscreen Capture",
        QKeySequence("Ctrl+Shift+F")
    );
    activeWindowCaptureAction_ = createWindowAction(
        themedIcon(this, "window", QStyle::SP_ComputerIcon),
        "Active Window",
        QKeySequence("Ctrl+Shift+W")
    );
    windowFitCaptureAction_ = createWindowAction(
        themedIcon(this, "selection-rectangular", QStyle::SP_DirIcon),
        "Window Fit Capture"
    );
    pinLatestAction_ = createWindowAction(
        themedIcon(this, "pin", QStyle::SP_DialogApplyButton),
        "Pin Last Capture",
        QKeySequence("Ctrl+Shift+P")
    );
    saveLatestAction_ = createWindowAction(
        themedIcon(this, "document-save", QStyle::SP_DialogSaveButton),
        "Save Last Capture",
        QKeySequence::Save
    );
    closePinsAction_ = createWindowAction(
        themedIcon(this, "window-close", QStyle::SP_DialogCloseButton),
        "Close Pins",
        QKeySequence("Ctrl+Alt+X")
    );
    restorePinInputAction_ = createWindowAction(
        themedIcon(this, "input-mouse", QStyle::SP_BrowserReload),
        "Restore Pin Input",
        QKeySequence("Ctrl+Alt+R")
    );
    openFolderAction_ = createWindowAction(
        themedIcon(this, "folder-open", QStyle::SP_DirOpenIcon),
        "Open Capture Folder",
        QKeySequence::Open
    );
    settingsAction_ = createWindowAction(
        drawHeaderIcon("settings"),
        "Settings",
        QKeySequence("Ctrl+,")
    );
    trayAction_ = createWindowAction(
        themedIcon(this, "window-minimize", QStyle::SP_TitleBarMinButton),
        "Hide to Tray",
        QKeySequence("Ctrl+H")
    );
    quitAction_ = createWindowAction(
        themedIcon(this, "application-exit", QStyle::SP_DialogCloseButton),
        "Quit",
        QKeySequence::Quit
    );

    headerRow->addWidget(createToolButton(root, settingsAction_));
    headerRow->addWidget(createToolButton(root, quitAction_));

    pinSelectedHistoryAction_ = createWindowAction(
        themedIcon(this, "pin", QStyle::SP_DialogApplyButton),
        "Pin Selected History Item",
        QKeySequence(Qt::Key_Return)
    );
    copySelectedHistoryAction_ = createWindowAction(
        themedIcon(this, "edit-copy", QStyle::SP_FileDialogListView),
        "Copy Selected History Item",
        QKeySequence::Copy
    );
    saveSelectedHistoryAction_ = createWindowAction(
        themedIcon(this, "document-save", QStyle::SP_DialogSaveButton),
        "Save Selected History Item",
        QKeySequence("Ctrl+Shift+S")
    );
    removeSelectedHistoryAction_ = createWindowAction(
        themedIcon(this, "edit-delete", QStyle::SP_TrashIcon),
        "Remove Selected History Item",
        QKeySequence::Delete
    );

    auto* historyPanel = createSectionSurface(splitter, "sectionSurface");
    auto* historyLayout = new QVBoxLayout(historyPanel);
    historyLayout->setContentsMargins(16, 16, 16, 16);
    historyLayout->setSpacing(12);

    auto* historyHeader = new QHBoxLayout();
    historyTitleLabel_ = new QLabel("History", historyPanel);
    historyTitleLabel_->setObjectName("sectionTitleLabel");
    QFont historyTitleFont = historyTitleLabel_->font();
    historyTitleFont.setPointSize(historyTitleFont.pointSize() + 2);
    historyTitleFont.setBold(true);
    historyTitleLabel_->setFont(historyTitleFont);
    historySummaryLabel_ = new QLabel("0 items", historyPanel);
    historySummaryLabel_->setObjectName("mutedLabel");
    historyHeader->addWidget(historyTitleLabel_);
    historyHeader->addStretch(1);
    historyHeader->addWidget(historySummaryLabel_);

    auto* historyActionRow = new QHBoxLayout();
    historyActionRow->setSpacing(6);
    historyActionRow->addWidget(createToolButton(historyPanel, pinSelectedHistoryAction_));
    historyActionRow->addWidget(createToolButton(historyPanel, copySelectedHistoryAction_));
    historyActionRow->addWidget(createToolButton(historyPanel, saveSelectedHistoryAction_));
    historyActionRow->addWidget(createToolButton(historyPanel, removeSelectedHistoryAction_));
    historyActionRow->addStretch(1);

    workspaceLayout->addLayout(workspaceHeader);
    workspaceLayout->addWidget(previewLabel_, 1);
    workspaceLayout->addWidget(statusLabel_);

    historyLayout->addLayout(historyHeader);
    historyLayout->addLayout(historyActionRow);
    historyLayout->addWidget(historyList_, 1);

    splitter->addWidget(workspacePanel);
    splitter->addWidget(historyPanel);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 2);

    layout->addLayout(headerRow);
    layout->addWidget(splitter, 1);

    setCentralWidget(root);
    applyAppearanceMode("light");
    applyLanguage(language_);

    connect(regionCaptureAction_, &QAction::triggered, this, &MainWindow::regionCaptureRequested);
    connect(
        fullscreenCaptureAction_,
        &QAction::triggered,
        this,
        &MainWindow::fullscreenCaptureRequested
    );
    connect(
        activeWindowCaptureAction_,
        &QAction::triggered,
        this,
        &MainWindow::activeWindowCaptureRequested
    );
    connect(
        windowFitCaptureAction_,
        &QAction::triggered,
        this,
        &MainWindow::windowFitCaptureRequested
    );
    connect(pinLatestAction_, &QAction::triggered, this, &MainWindow::pinLatestCaptureRequested);
    connect(saveLatestAction_, &QAction::triggered, this, &MainWindow::saveLatestCaptureRequested);
    connect(closePinsAction_, &QAction::triggered, this, &MainWindow::closeAllPinsRequested);
    connect(
        restorePinInputAction_,
        &QAction::triggered,
        this,
        &MainWindow::restorePinInputRequested
    );
    connect(
        openFolderAction_,
        &QAction::triggered,
        this,
        &MainWindow::openCapturesDirectoryRequested
    );
    connect(settingsAction_, &QAction::triggered, this, &MainWindow::settingsRequested);
    connect(trayAction_, &QAction::triggered, this, &MainWindow::hideToTrayRequested);
    connect(quitAction_, &QAction::triggered, this, &MainWindow::quitRequested);
    connect(historyList_, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem* item) {
        emit historyPinRequested(item->data(ImageRole).value<QImage>());
    });
    connect(pinSelectedHistoryAction_, &QAction::triggered, this, &MainWindow::pinSelectedHistoryItem);
    connect(copySelectedHistoryAction_, &QAction::triggered, this, &MainWindow::copySelectedHistoryItem);
    connect(saveSelectedHistoryAction_, &QAction::triggered, this, &MainWindow::saveSelectedHistoryItem);
    connect(
        removeSelectedHistoryAction_,
        &QAction::triggered,
        this,
        &MainWindow::removeSelectedHistoryItem
    );
    connect(
        historyList_,
        &QListWidget::customContextMenuRequested,
        this,
        &MainWindow::showHistoryContextMenu
    );
    connect(historyList_, &QListWidget::itemSelectionChanged, this, [this]() {
        updateHistoryActionState();
        syncPreviewToCurrentContext();
    });

    updateHistoryActionState();
    updateHistorySummary();
    syncPreviewToCurrentContext();
}

void MainWindow::applyAppearanceMode(const QString& appearanceMode) {
    setStyleSheet(mainWindowStyleSheetForTheme(shellThemeModeFromSettings(appearanceMode)));
}

void MainWindow::applyLanguage(cappy::localization::AppLanguage language) {
    language_ = cappy::localization::resolvedAppLanguage(language);
    const auto& text = cappy::localization::strings(language_);
    setWindowTitle(text.appName);
    if (titleLabel_ != nullptr) {
        titleLabel_->setText(text.appName);
    }
    if (workspaceTitleLabel_ != nullptr) {
        workspaceTitleLabel_->setText(text.mainWorkspaceTitle);
    }
    if (historyTitleLabel_ != nullptr) {
        historyTitleLabel_->setText(text.mainHistoryTitle);
    }
    if (statusLabel_ != nullptr && statusLabel_->text().isEmpty()) {
        statusLabel_->setText(text.readyStatus);
    }
    if (previewLabel_ != nullptr && previewImage_.isNull()) {
        previewLabel_->setText(text.mainPreviewPlaceholder);
    }

    refreshActionTexts();
    applyShortcutSettings(mainWindowShortcuts_);
    updateHistorySummary();
    for (int row = 0; historyList_ != nullptr && row < historyList_->count(); ++row) {
        updateHistoryItemDisplay(historyList_->item(row));
    }
}

void MainWindow::applyShortcutSettings(
    const cappy::shortcuts::MainWindowShortcutSettings& shortcuts
) {
    mainWindowShortcuts_ = shortcuts;
    applyActionShortcut(
        regionCaptureAction_,
        QKeySequence::fromString(shortcuts.regionCapture, QKeySequence::PortableText)
    );
    applyActionShortcut(
        fullscreenCaptureAction_,
        QKeySequence::fromString(shortcuts.fullscreenCapture, QKeySequence::PortableText)
    );
    applyActionShortcut(
        activeWindowCaptureAction_,
        QKeySequence::fromString(shortcuts.activeWindowCapture, QKeySequence::PortableText)
    );
    applyActionShortcut(
        windowFitCaptureAction_,
        QKeySequence::fromString(shortcuts.windowFitCapture, QKeySequence::PortableText)
    );
    applyActionShortcut(
        pinLatestAction_,
        QKeySequence::fromString(shortcuts.pinLatest, QKeySequence::PortableText)
    );
    applyActionShortcut(
        saveLatestAction_,
        QKeySequence::fromString(shortcuts.saveLatest, QKeySequence::PortableText)
    );
    applyActionShortcut(
        closePinsAction_,
        QKeySequence::fromString(shortcuts.closePins, QKeySequence::PortableText)
    );
    applyActionShortcut(
        restorePinInputAction_,
        QKeySequence::fromString(shortcuts.restorePinInput, QKeySequence::PortableText)
    );
    applyActionShortcut(
        openFolderAction_,
        QKeySequence::fromString(shortcuts.openCaptureFolder, QKeySequence::PortableText)
    );
    applyActionShortcut(
        settingsAction_,
        QKeySequence::fromString(shortcuts.settings, QKeySequence::PortableText)
    );
    applyActionShortcut(
        trayAction_,
        QKeySequence::fromString(shortcuts.hideToTray, QKeySequence::PortableText)
    );
    applyActionShortcut(
        quitAction_,
        QKeySequence::fromString(shortcuts.quit, QKeySequence::PortableText)
    );
    applyActionShortcut(
        pinSelectedHistoryAction_,
        QKeySequence::fromString(shortcuts.historyPin, QKeySequence::PortableText)
    );
    applyActionShortcut(
        copySelectedHistoryAction_,
        QKeySequence::fromString(shortcuts.historyCopy, QKeySequence::PortableText)
    );
    applyActionShortcut(
        saveSelectedHistoryAction_,
        QKeySequence::fromString(shortcuts.historySave, QKeySequence::PortableText)
    );
    applyActionShortcut(
        removeSelectedHistoryAction_,
        QKeySequence::fromString(shortcuts.historyRemove, QKeySequence::PortableText)
    );
}

void MainWindow::setShortcutActionsSuspended(bool suspended) {
    for (QAction* action : shortcutActions()) {
        if (action != nullptr) {
            action->setEnabled(!suspended);
        }
    }
}

void MainWindow::setCloseToTrayEnabled(bool enabled) {
    closeToTrayEnabled_ = enabled;
}

void MainWindow::setCommandStatus(const QString& status) {
    if (statusLabel_ != nullptr) {
        statusLabel_->setText(status);
    }
}

void MainWindow::setHistoryLimit(int limit) {
    historyLimit_ = qMax(1, limit);
    trimHistoryToLimit();
}

void MainWindow::showCaptureResult(
    const QImage& image,
    const QRect& geometry,
    const QString& backendName
) {
    Q_UNUSED(geometry);
    Q_UNUSED(backendName);
    latestCaptureImage_ = image;
    syncPreviewToCurrentContext();
}

void MainWindow::addCaptureHistoryEntry(const CaptureHistoryEntry& entry) {
    if (historyList_ == nullptr) {
        return;
    }

    auto* item = new QListWidgetItem(entry.title);
    item->setData(ImageRole, entry.image);
    item->setData(FilePathRole, entry.filePath);
    item->setData(CaptureModeRole, entry.captureMode);
    item->setData(EntryIdRole, entry.id);
    item->setData(BaseTitleRole, entry.title);
    if (!entry.image.isNull()) {
        item->setIcon(QPixmap::fromImage(entry.image).scaled(
            historyList_->iconSize(),
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
        ));
    }
    updateHistoryItemDisplay(item);
    historyList_->insertItem(0, item);
    historyList_->setCurrentItem(item);
    trimHistoryToLimit();
    updateHistorySummary();
    updateHistoryActionState();
    syncPreviewToCurrentContext();
}

void MainWindow::markHistoryEntrySaved(const QString& entryId, const QString& filePath) {
    if (historyList_ == nullptr || entryId.isEmpty()) {
        return;
    }

    for (int row = 0; row < historyList_->count(); ++row) {
        QListWidgetItem* item = historyList_->item(row);
        if (item != nullptr && item->data(EntryIdRole).toString() == entryId) {
            item->setData(FilePathRole, filePath);
            updateHistoryItemDisplay(item);
            if (item == historyList_->currentItem()) {
                syncPreviewToCurrentContext();
            }
            return;
        }
    }
}

void MainWindow::updateHistoryEntryImage(
    const QString& entryId,
    const QImage& image,
    bool clearSavedState
) {
    if (historyList_ == nullptr || entryId.isEmpty() || image.isNull()) {
        return;
    }

    for (int row = 0; row < historyList_->count(); ++row) {
        QListWidgetItem* item = historyList_->item(row);
        if (item == nullptr || item->data(EntryIdRole).toString() != entryId) {
            continue;
        }

        item->setData(ImageRole, image);
        if (clearSavedState) {
            item->setData(FilePathRole, {});
            updateHistoryItemDisplay(item);
        }
        item->setIcon(QPixmap::fromImage(image).scaled(
            historyList_->iconSize(),
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
        ));
        if (item == historyList_->currentItem()) {
            syncPreviewToCurrentContext();
        }
        return;
    }
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (closeToTrayEnabled_) {
        emit hideToTrayRequested();
        event->ignore();
        return;
    }

    QMainWindow::closeEvent(event);
}

void MainWindow::resizeEvent(QResizeEvent* event) {
    QMainWindow::resizeEvent(event);
    updatePreviewPixmap();
}

QAction* MainWindow::createWindowAction(
    const QIcon& icon,
    const QString& text,
    const QKeySequence& shortcut
) {
    auto* action = new QAction(icon, text, this);
    if (!shortcut.isEmpty()) {
        action->setShortcut(shortcut);
    }
    action->setShortcutContext(Qt::WidgetWithChildrenShortcut);

    const QString tooltip = shortcut.isEmpty()
        ? text
        : QString("%1 (%2)").arg(text, shortcut.toString(QKeySequence::NativeText));
    action->setToolTip(tooltip);
    action->setStatusTip(tooltip);
    addAction(action);
    return action;
}

QList<QAction*> MainWindow::shortcutActions() const {
    return {
        regionCaptureAction_,
        fullscreenCaptureAction_,
        activeWindowCaptureAction_,
        windowFitCaptureAction_,
        pinLatestAction_,
        saveLatestAction_,
        closePinsAction_,
        restorePinInputAction_,
        openFolderAction_,
        settingsAction_,
        trayAction_,
        quitAction_,
        pinSelectedHistoryAction_,
        copySelectedHistoryAction_,
        saveSelectedHistoryAction_,
        removeSelectedHistoryAction_,
    };
}

void MainWindow::refreshActionTexts() {
    const auto& text = cappy::localization::strings(language_);
    if (regionCaptureAction_ != nullptr) regionCaptureAction_->setText(text.actionRegionCapture);
    if (fullscreenCaptureAction_ != nullptr) fullscreenCaptureAction_->setText(text.actionFullscreenCapture);
    if (activeWindowCaptureAction_ != nullptr) activeWindowCaptureAction_->setText(text.actionActiveWindow);
    if (windowFitCaptureAction_ != nullptr) windowFitCaptureAction_->setText(text.actionWindowFitCapture);
    if (pinLatestAction_ != nullptr) pinLatestAction_->setText(text.actionPinLastCapture);
    if (saveLatestAction_ != nullptr) saveLatestAction_->setText(text.actionSaveLastCapture);
    if (closePinsAction_ != nullptr) closePinsAction_->setText(text.actionClosePins);
    if (restorePinInputAction_ != nullptr) restorePinInputAction_->setText(text.actionRestorePinInput);
    if (openFolderAction_ != nullptr) openFolderAction_->setText(text.actionOpenCaptureFolder);
    if (settingsAction_ != nullptr) settingsAction_->setText(text.actionSettings);
    if (trayAction_ != nullptr) trayAction_->setText(text.actionHideToTray);
    if (quitAction_ != nullptr) quitAction_->setText(text.actionQuit);
    if (pinSelectedHistoryAction_ != nullptr) pinSelectedHistoryAction_->setText(text.actionPinSelectedHistoryItem);
    if (copySelectedHistoryAction_ != nullptr) copySelectedHistoryAction_->setText(text.actionCopySelectedHistoryItem);
    if (saveSelectedHistoryAction_ != nullptr) saveSelectedHistoryAction_->setText(text.actionSaveSelectedHistoryItem);
    if (removeSelectedHistoryAction_ != nullptr) removeSelectedHistoryAction_->setText(text.actionRemoveSelectedHistoryItem);
}

void MainWindow::syncPreviewToCurrentContext() {
    QListWidgetItem* item = selectedHistoryItem();
    if (item != nullptr) {
        previewImage_ = item->data(ImageRole).value<QImage>();
        updatePreviewPixmap();
        return;
    }

    previewImage_ = latestCaptureImage_;
    updatePreviewPixmap();
}

void MainWindow::updatePreviewPixmap() {
    if (previewLabel_ == nullptr) {
        return;
    }

    if (previewImage_.isNull()) {
        previewLabel_->setPixmap(QPixmap());
        previewLabel_->setText(cappy::localization::strings(language_).mainPreviewPlaceholder);
        return;
    }

    previewLabel_->setText({});
    previewLabel_->setPixmap(
        QPixmap::fromImage(previewImage_).scaled(
            previewLabel_->contentsRect().size(),
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
        )
    );
}

void MainWindow::updateHistorySummary() {
    if (historySummaryLabel_ == nullptr || historyList_ == nullptr) {
        return;
    }

    historySummaryLabel_->setText(cappy::localization::historyCountLabel(language_, historyList_->count()));
}

QListWidgetItem* MainWindow::selectedHistoryItem() const {
    if (historyList_ == nullptr) {
        return nullptr;
    }

    return historyList_->currentItem();
}

void MainWindow::pinSelectedHistoryItem() {
    QListWidgetItem* item = selectedHistoryItem();
    if (item == nullptr) {
        return;
    }

    emit historyPinRequested(item->data(ImageRole).value<QImage>());
}

void MainWindow::copySelectedHistoryItem() {
    QListWidgetItem* item = selectedHistoryItem();
    if (item == nullptr) {
        return;
    }

    emit historyCopyRequested(item->data(ImageRole).value<QImage>());
}

void MainWindow::saveSelectedHistoryItem() {
    QListWidgetItem* item = selectedHistoryItem();
    if (item == nullptr) {
        return;
    }

    emit historySaveRequested(
        item->data(EntryIdRole).toString(),
        item->data(ImageRole).value<QImage>(),
        item->data(CaptureModeRole).toInt()
    );
}

void MainWindow::removeSelectedHistoryItem() {
    QListWidgetItem* item = selectedHistoryItem();
    if (item == nullptr || historyList_ == nullptr) {
        return;
    }

    delete historyList_->takeItem(historyList_->row(item));
    setCommandStatus(cappy::localization::strings(language_).statusRemovedHistoryEntry);
    updateHistorySummary();
    updateHistoryActionState();
    syncPreviewToCurrentContext();
}

void MainWindow::updateHistoryActionState() {
    const bool hasSelection = selectedHistoryItem() != nullptr;
    if (pinSelectedHistoryAction_ != nullptr) {
        pinSelectedHistoryAction_->setEnabled(hasSelection);
    }
    if (copySelectedHistoryAction_ != nullptr) {
        copySelectedHistoryAction_->setEnabled(hasSelection);
    }
    if (saveSelectedHistoryAction_ != nullptr) {
        saveSelectedHistoryAction_->setEnabled(hasSelection);
    }
    if (removeSelectedHistoryAction_ != nullptr) {
        removeSelectedHistoryAction_->setEnabled(hasSelection);
    }
}

void MainWindow::trimHistoryToLimit() {
    if (historyList_ == nullptr) {
        return;
    }

    const int effectiveLimit = qMax(1, historyLimit_);
    while (historyList_->count() > effectiveLimit) {
        delete historyList_->takeItem(historyList_->count() - 1);
    }
    updateHistorySummary();
    updateHistoryActionState();
    syncPreviewToCurrentContext();
}

void MainWindow::showHistoryContextMenu(const QPoint& position) {
    if (historyList_ == nullptr) {
        return;
    }

    QListWidgetItem* item = historyList_->itemAt(position);
    if (item == nullptr) {
        return;
    }

    const QImage image = item->data(ImageRole).value<QImage>();
    const QString entryId = item->data(EntryIdRole).toString();
    const int captureMode = item->data(CaptureModeRole).toInt();

    QMenu menu(this);
    const auto& text = cappy::localization::strings(language_);
    QAction* pinAction = menu.addAction(text.actionPinToDesktop);
    QAction* copyAction = menu.addAction(text.actionCopyToClipboard);
    QAction* saveAction = menu.addAction(text.actionSaveToCaptureFolder);
    menu.addSeparator();
    QAction* removeAction = menu.addAction(text.actionRemoveSelectedHistoryItem);

    pinAction->setEnabled(!image.isNull());
    copyAction->setEnabled(!image.isNull());
    saveAction->setEnabled(!image.isNull());

    QAction* selected = menu.exec(historyList_->viewport()->mapToGlobal(position));
    if (selected == pinAction) {
        emit historyPinRequested(image);
        return;
    }

    if (selected == copyAction) {
        emit historyCopyRequested(image);
        return;
    }

    if (selected == saveAction) {
        emit historySaveRequested(entryId, image, captureMode);
        return;
    }

    if (selected == removeAction) {
        historyList_->setCurrentItem(item);
        removeSelectedHistoryItem();
    }
}

void MainWindow::updateHistoryItemDisplay(QListWidgetItem* item) {
    if (item == nullptr) {
        return;
    }

    const QString baseTitle = item->data(BaseTitleRole).toString();
    const QString filePath = item->data(FilePathRole).toString();
    const int captureMode = item->data(CaptureModeRole).toInt();
    const auto& text = cappy::localization::strings(language_);
    const QString stateLabel = filePath.isEmpty() ? text.historyClipboardOnly : text.historySaved;
    const QString modeLabel = historyModeLabel(language_, captureMode);

    item->setText(QString("%1 | %2").arg(
        baseTitle,
        stateLabel
    ));

    item->setToolTip(cappy::localization::historyItemDetailText(
        language_,
        baseTitle,
        modeLabel,
        filePath.isEmpty() ? QString{} : QFileInfo(filePath).absoluteFilePath()
    ));
}
