#include "cappy/features/ocr/OcrResultWindow.h"

#include <algorithm>
#include <limits>

#include <QApplication>
#include <QClipboard>
#include <QComboBox>
#include <QEvent>
#include <QFile>
#include <QFileDialog>
#include <QFrame>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPainter>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QSplitter>
#include <QStyle>
#include <QTextStream>
#include <QTextBlock>
#include <QTimer>
#include <QVBoxLayout>
#include <QMouseEvent>

#include "cappy/services/ocr/OcrService.h"

namespace cappy::features::ocr {

namespace {

bool isDarkMode(const QString& appearanceMode) {
    return appearanceMode.trimmed().compare("dark", Qt::CaseInsensitive) == 0;
}

QIcon themedIcon(QWidget* widget, const QString& themeName, QStyle::StandardPixmap fallback) {
    const QIcon themeIcon = QIcon::fromTheme(themeName);
    if (!themeIcon.isNull()) {
        return themeIcon;
    }

    return widget->style()->standardIcon(fallback);
}

QPushButton* createToolbarButton(QWidget* parent, const QIcon& icon) {
    auto* button = new QPushButton(parent);
    button->setIcon(icon);
    button->setFlat(true);
    button->setFixedSize(30, 30);
    button->setCursor(Qt::PointingHandCursor);
    return button;
}

QString ocrWindowStyleSheet(bool darkMode) {
    if (darkMode) {
        return QStringLiteral(
            "QDialog { background: #11161b; color: #e7edf3; }"
            "QFrame#ocrToolbar {"
            "  background: #182028;"
            "  border: 1px solid #2d3a46;"
            "  border-radius: 10px;"
            "}"
            "QLabel#ocrTitleLabel { color: #eef4f9; font-size: 18px; font-weight: 700; }"
            "QLabel#ocrMetaLabel, QLabel#ocrZoomLabel, QLabel#ocrStatusLabel { color: #96a7b7; }"
            "QFrame#ocrPanel {"
            "  background: #182028;"
            "  border: 1px solid #2d3a46;"
            "  border-radius: 10px;"
            "}"
            "QFrame#ocrCanvas {"
            "  background: #0b0f13;"
            "  border: 1px solid #283441;"
            "  border-radius: 8px;"
            "}"
            "QLabel#ocrPanelTitle { color: #eef4f9; font-weight: 600; }"
            "QLabel#ocrSectionTitle { color: #d7e2ec; font-size: 12px; font-weight: 600; }"
            "QPushButton {"
            "  color: #e7edf3;"
            "  border: 1px solid #31404d;"
            "  border-radius: 6px;"
            "  background: #202a33;"
            "  padding: 0 12px;"
            "}"
            "QPushButton:hover { background: #26323d; }"
            "QPushButton:pressed { background: #2e3c49; }"
            "QPushButton:disabled { color: #6f7f8e; background: #182028; border-color: #25313c; }"
            "QPushButton#accentButton {"
            "  background: #2d5a86;"
            "  border-color: #4a81b8;"
            "  color: #f4f9ff;"
            "}"
            "QPushButton#accentButton:hover { background: #356796; }"
            "QPushButton#accentButton:pressed { background: #2b547b; }"
            "QComboBox {"
            "  min-height: 34px;"
            "  color: #e7edf3;"
            "  border: 1px solid #31404d;"
            "  border-radius: 6px;"
            "  padding: 0 10px;"
            "  background: #202a33;"
            "}"
            "QComboBox QAbstractItemView {"
            "  color: #e7edf3;"
            "  background: #202a33;"
            "  border: 1px solid #31404d;"
            "  selection-background-color: #2d5a86;"
            "  selection-color: #f4f9ff;"
            "}"
            "QComboBox::drop-down { border: 0; }"
            "QPlainTextEdit {"
            "  color: #e7edf3;"
            "  background: #13191f;"
            "  border: 1px solid #283441;"
            "  border-radius: 8px;"
            "  padding: 12px;"
            "  selection-background-color: #2d5a86;"
            "  selection-color: #f4f9ff;"
            "}"
            "QPlainTextEdit[placeholder='true'] { color: #6f7f8e; }"
            "QListWidget {"
            "  color: #e7edf3;"
            "  background: #13191f;"
            "  border: 1px solid #283441;"
            "  border-radius: 8px;"
            "  outline: 0;"
            "  padding: 6px;"
            "}"
            "QListWidget::item {"
            "  border: 1px solid transparent;"
            "  border-radius: 8px;"
            "  padding: 8px 10px;"
            "  margin: 2px 0;"
            "}"
            "QListWidget::item:hover {"
            "  background: #19212a;"
            "  border-color: #31404d;"
            "}"
            "QListWidget::item:selected {"
            "  background: #23313d;"
            "  border-color: #4b6f93;"
            "  color: #f4f9ff;"
            "}"
            "QScrollArea { background: transparent; border: 0; }"
            "QScrollBar:vertical { width: 10px; background: #141b22; border-radius: 5px; }"
            "QScrollBar::handle:vertical { background: #3d4f60; min-height: 28px; border-radius: "
            "5px; }"
            "QScrollBar:horizontal { height: 10px; background: #141b22; border-radius: 5px; }"
            "QScrollBar::handle:horizontal { background: #3d4f60; min-width: 28px; border-radius: "
            "5px; }"
            "QScrollBar::add-line, QScrollBar::sub-line, QScrollBar::add-page, "
            "QScrollBar::sub-page {"
            "  width: 0; height: 0; background: transparent;"
            "}");
    }

    return QStringLiteral(
        "QDialog { background: #e7edf3; color: #13202c; }"
        "QFrame#ocrToolbar {"
        "  background: #f9fbfd;"
        "  border: 1px solid #bcc8d4;"
        "  border-radius: 10px;"
        "}"
        "QLabel#ocrTitleLabel { color: #13202c; font-size: 18px; font-weight: 700; }"
        "QLabel#ocrMetaLabel, QLabel#ocrZoomLabel, QLabel#ocrStatusLabel { color: #485868; }"
        "QFrame#ocrPanel {"
        "  background: #f9fbfd;"
        "  border: 1px solid #bcc8d4;"
        "  border-radius: 10px;"
        "}"
        "QFrame#ocrCanvas {"
        "  background: #dce4eb;"
        "  border: 1px solid #c5d0da;"
        "  border-radius: 8px;"
        "}"
        "QLabel#ocrPanelTitle { color: #13202c; font-weight: 600; }"
        "QLabel#ocrSectionTitle { color: #415365; font-size: 12px; font-weight: 600; }"
        "QPushButton {"
        "  color: #13202c;"
        "  border: 1px solid #b7c4d0;"
        "  border-radius: 6px;"
        "  background: #e4ebf2;"
        "  padding: 0 12px;"
        "}"
        "QPushButton:hover { background: #d9e4ee; }"
        "QPushButton:pressed { background: #ccd9e5; }"
        "QPushButton:disabled { color: #7a8794; background: #eef2f6; border-color: #d5dde5; }"
        "QPushButton#accentButton {"
        "  background: #d6e7fb;"
        "  border-color: #8fb7e6;"
        "  color: #0f2b44;"
        "}"
        "QPushButton#accentButton:hover { background: #c7ddf6; }"
        "QPushButton#accentButton:pressed { background: #b7d2f1; }"
        "QComboBox {"
        "  min-height: 34px;"
        "  color: #13202c;"
        "  border: 1px solid #b7c4d0;"
        "  border-radius: 6px;"
        "  padding: 0 10px;"
        "  background: #fdfefe;"
        "}"
        "QComboBox QAbstractItemView {"
        "  color: #13202c;"
        "  background: #fdfefe;"
        "  border: 1px solid #b7c4d0;"
        "  selection-background-color: #d8eafc;"
        "  selection-color: #0f2940;"
        "}"
        "QComboBox::drop-down { border: 0; }"
        "QPlainTextEdit {"
        "  color: #13202c;"
        "  background: #ffffff;"
        "  border: 1px solid #cad4de;"
        "  border-radius: 8px;"
        "  padding: 12px;"
        "  selection-background-color: #d8eafc;"
        "  selection-color: #0f2940;"
        "}"
        "QPlainTextEdit[placeholder='true'] { color: #7b8a98; }"
        "QListWidget {"
        "  color: #13202c;"
        "  background: #ffffff;"
        "  border: 1px solid #cad4de;"
        "  border-radius: 8px;"
        "  outline: 0;"
        "  padding: 6px;"
        "}"
        "QListWidget::item {"
        "  border: 1px solid transparent;"
        "  border-radius: 8px;"
        "  padding: 8px 10px;"
        "  margin: 2px 0;"
        "}"
        "QListWidget::item:hover {"
        "  background: #f2f6fa;"
        "  border-color: #c4d0da;"
        "}"
        "QListWidget::item:selected {"
        "  background: #e3effb;"
        "  border-color: #94b8e0;"
        "  color: #0f2940;"
        "}"
        "QScrollArea { background: transparent; border: 0; }"
        "QScrollBar:vertical { width: 10px; background: #d9e2ea; border-radius: 5px; }"
        "QScrollBar::handle:vertical { background: #99adbf; min-height: 28px; border-radius: 5px; }"
        "QScrollBar:horizontal { height: 10px; background: #d9e2ea; border-radius: 5px; }"
        "QScrollBar::handle:horizontal { background: #99adbf; min-width: 28px; border-radius: 5px; "
        "}"
        "QScrollBar::add-line, QScrollBar::sub-line, QScrollBar::add-page, QScrollBar::sub-page {"
        "  width: 0; height: 0; background: transparent;"
        "}");
}

} // namespace

OcrResultWindow::OcrResultWindow(const QImage& image, cappy::services::ocr::OcrSettings settings,
                                 cappy::localization::AppLanguage language, QString appearanceMode,
                                 QWidget* parent, cappy::services::ocr::OcrResult initialResult,
                                 bool autoRun)
    : QDialog(parent), image_(image), currentResult_(std::move(initialResult)),
      settings_(std::move(settings)), language_(cappy::localization::resolvedAppLanguage(language)),
      appearanceMode_(std::move(appearanceMode)),
      ocrService_(new cappy::services::ocr::OcrService(this)), autoRun_(autoRun) {
    buildUi();
    refreshTexts();
    resultEdit_->setPlainText(currentResult_.text);
    updateTextRegionSelection();
    updatePreview();

    connect(ocrService_, &cappy::services::ocr::OcrService::busyChanged, this, [this](bool busy) {
        if (runButton_ != nullptr) {
            runButton_->setEnabled(!busy);
        }
    });
    connect(ocrService_, &cappy::services::ocr::OcrService::started, this, [this]() {
        const bool cloud =
            providerComboBox_ != nullptr && providerComboBox_->currentData().toString() == "cloud";
        const auto& text = cappy::localization::strings(language_);
        currentResult_ = {};
        activeRegionIndex_ = -1;
        hoveredRegionIndex_ = -1;
        suppressTextCursorSync_ = true;
        resultEdit_->clear();
        suppressTextCursorSync_ = false;
        updatePreview();
        statusLabel_->setText(cloud ? text.dialogOcrStatusRunningCloud
                                    : text.dialogOcrStatusRunningLocal);
    });
    connect(ocrService_, &cappy::services::ocr::OcrService::finished, this,
            [this](const cappy::services::ocr::OcrResult& result) {
                currentResult_ = result;
                activeRegionIndex_ = currentResult_.regions.isEmpty() ? -1 : 0;
                hoveredRegionIndex_ = -1;
                suppressTextCursorSync_ = true;
                resultEdit_->setPlainText(result.text);
                suppressTextCursorSync_ = false;
                updateTextRegionSelection();
                updatePreview();
                statusLabel_->setText(cappy::localization::strings(language_)
                                          .dialogOcrStatusFinishedTemplate.arg(result.text.size())
                                          .arg(result.regions.size()));
            });
    connect(ocrService_, &cappy::services::ocr::OcrService::failed, this,
            [this](const QString& message) { statusLabel_->setText(message); });

    connect(resultEdit_, &QPlainTextEdit::cursorPositionChanged, this, [this]() {
        if (suppressTextCursorSync_) {
            return;
        }
        setActiveRegion(regionIndexForTextCursor(), false);
    });

    if (previewLabel_ != nullptr) {
        previewLabel_->setMouseTracking(true);
        previewLabel_->installEventFilter(this);
    }

    QTimer::singleShot(0, this, [this]() {
        fitPreview();
        if (autoRun_) {
            runRecognition();
        }
    });
}

bool OcrResultWindow::eventFilter(QObject* watched, QEvent* event) {
    if (watched == previewLabel_ && previewLabel_ != nullptr) {
        switch (event->type()) {
        case QEvent::MouseMove: {
            const auto* mouseEvent = static_cast<QMouseEvent*>(event);
            setHoveredRegion(regionIndexAtPreviewPosition(mouseEvent->pos()));
            break;
        }
        case QEvent::Leave:
            setHoveredRegion(-1);
            break;
        case QEvent::MouseButtonPress: {
            const auto* mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                setActiveRegion(regionIndexAtPreviewPosition(mouseEvent->pos()), true);
            }
            break;
        }
        default:
            break;
        }
    }

    return QDialog::eventFilter(watched, event);
}

void OcrResultWindow::buildUi() {
    setModal(false);
    resize(1180, 760);
    setMinimumSize(900, 560);
    setStyleSheet(ocrWindowStyleSheet(isDarkMode(appearanceMode_)));

    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(14, 14, 14, 14);
    rootLayout->setSpacing(12);

    toolbar_ = new QFrame(this);
    toolbar_->setObjectName("ocrToolbar");
    auto* toolbarLayout = new QHBoxLayout(toolbar_);
    toolbarLayout->setContentsMargins(16, 14, 16, 14);
    toolbarLayout->setSpacing(12);

    auto* titleColumn = new QVBoxLayout();
    titleColumn->setContentsMargins(0, 0, 0, 0);
    titleColumn->setSpacing(4);
    titleLabel_ = new QLabel(toolbar_);
    titleLabel_->setObjectName("ocrTitleLabel");
    imageMetaLabel_ = new QLabel(toolbar_);
    imageMetaLabel_->setObjectName("ocrMetaLabel");
    titleColumn->addWidget(titleLabel_);
    titleColumn->addWidget(imageMetaLabel_);
    toolbarLayout->addLayout(titleColumn, 1);

    zoomInButton_ = createToolbarButton(toolbar_, themedIcon(this, "zoom-in", QStyle::SP_ArrowUp));
    zoomOutButton_ =
        createToolbarButton(toolbar_, themedIcon(this, "zoom-out", QStyle::SP_ArrowDown));
    fitButton_ = createToolbarButton(
        toolbar_, themedIcon(this, "zoom-fit-best", QStyle::SP_TitleBarMaxButton));
    copyButton_ =
        createToolbarButton(toolbar_, themedIcon(this, "edit-copy", QStyle::SP_FileDialogListView));
    saveButton_ = createToolbarButton(
        toolbar_, themedIcon(this, "document-save", QStyle::SP_DialogSaveButton));
    zoomInButton_->setFixedSize(34, 34);
    zoomOutButton_->setFixedSize(34, 34);
    fitButton_->setFixedSize(34, 34);
    copyButton_->setFixedSize(34, 34);
    saveButton_->setFixedSize(34, 34);

    auto* actionsRow = new QHBoxLayout();
    actionsRow->setContentsMargins(0, 0, 0, 0);
    actionsRow->setSpacing(8);
    actionsRow->addWidget(zoomOutButton_);
    actionsRow->addWidget(zoomInButton_);
    actionsRow->addWidget(fitButton_);
    actionsRow->addSpacing(6);
    actionsRow->addWidget(copyButton_);
    actionsRow->addWidget(saveButton_);

    providerComboBox_ = new QComboBox(toolbar_);
    providerComboBox_->addItem(QStringLiteral("Local OCR"), "local");
    providerComboBox_->addItem(QStringLiteral("Cloud API"), "cloud");
    providerComboBox_->setCurrentIndex(
        qMax(0, providerComboBox_->findData(settings_.preferredProvider.trimmed().isEmpty()
                                                ? QStringLiteral("local")
                                                : settings_.preferredProvider.trimmed())));
    providerComboBox_->setMinimumWidth(140);
    runButton_ = new QPushButton(toolbar_);
    runButton_->setObjectName("accentButton");
    runButton_->setCursor(Qt::PointingHandCursor);
    runButton_->setMinimumHeight(34);
    actionsRow->addWidget(providerComboBox_);
    actionsRow->addWidget(runButton_);
    toolbarLayout->addLayout(actionsRow);
    rootLayout->addWidget(toolbar_);

    auto* splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setChildrenCollapsible(false);
    splitter->setHandleWidth(8);

    auto* previewHost = new QFrame(splitter);
    previewHost->setObjectName("ocrPanel");
    auto* previewLayout = new QVBoxLayout(previewHost);
    previewLayout->setContentsMargins(16, 16, 16, 16);
    previewLayout->setSpacing(12);
    auto* previewHeader = new QHBoxLayout();
    previewHeader->setContentsMargins(0, 0, 0, 0);
    previewHeader->setSpacing(8);
    previewTitleLabel_ = new QLabel(previewHost);
    previewTitleLabel_->setObjectName("ocrPanelTitle");
    zoomLabel_ = new QLabel(previewHost);
    zoomLabel_->setObjectName("ocrZoomLabel");
    previewHeader->addWidget(previewTitleLabel_);
    previewHeader->addStretch(1);
    previewHeader->addWidget(zoomLabel_);
    previewLayout->addLayout(previewHeader);

    auto* previewCanvas = new QFrame(previewHost);
    previewCanvas->setObjectName("ocrCanvas");
    auto* previewCanvasLayout = new QVBoxLayout(previewCanvas);
    previewCanvasLayout->setContentsMargins(12, 12, 12, 12);
    previewCanvasLayout->setSpacing(0);
    previewScrollArea_ = new QScrollArea(previewCanvas);
    previewScrollArea_->setWidgetResizable(true);
    previewScrollArea_->setAlignment(Qt::AlignCenter);
    previewScrollArea_->setFrameShape(QFrame::NoFrame);
    previewLabel_ = new QLabel(previewScrollArea_);
    previewLabel_->setAlignment(Qt::AlignCenter);
    previewLabel_->setMinimumSize(1, 1);
    previewScrollArea_->setWidget(previewLabel_);
    previewCanvasLayout->addWidget(previewScrollArea_);
    previewLayout->addWidget(previewCanvas, 1);

    auto* resultHost = new QFrame(splitter);
    resultHost->setObjectName("ocrPanel");
    auto* resultLayout = new QVBoxLayout(resultHost);
    resultLayout->setContentsMargins(16, 16, 16, 16);
    resultLayout->setSpacing(12);
    auto* resultHeader = new QHBoxLayout();
    resultHeader->setContentsMargins(0, 0, 0, 0);
    resultHeader->setSpacing(8);
    resultTitleLabel_ = new QLabel(resultHost);
    resultTitleLabel_->setObjectName("ocrPanelTitle");
    resultHeader->addWidget(resultTitleLabel_);
    resultHeader->addStretch(1);
    resultLayout->addLayout(resultHeader);
    resultSplitter_ = new QSplitter(Qt::Vertical, resultHost);
    resultSplitter_->setChildrenCollapsible(false);
    resultSplitter_->setHandleWidth(8);

    auto* regionHost = new QWidget(resultSplitter_);
    auto* regionLayout = new QVBoxLayout(regionHost);
    regionLayout->setContentsMargins(0, 0, 0, 0);
    regionLayout->setSpacing(8);
    regionTitleLabel_ = new QLabel(regionHost);
    regionTitleLabel_->setObjectName("ocrSectionTitle");
    regionLayout->addWidget(regionTitleLabel_);
    regionListWidget_ = new QListWidget(regionHost);
    regionListWidget_->setSelectionMode(QAbstractItemView::SingleSelection);
    regionListWidget_->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    regionListWidget_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    regionLayout->addWidget(regionListWidget_, 1);

    auto* fullTextHost = new QWidget(resultSplitter_);
    auto* fullTextLayout = new QVBoxLayout(fullTextHost);
    fullTextLayout->setContentsMargins(0, 0, 0, 0);
    fullTextLayout->setSpacing(8);
    fullTextTitleLabel_ = new QLabel(fullTextHost);
    fullTextTitleLabel_->setObjectName("ocrSectionTitle");
    fullTextLayout->addWidget(fullTextTitleLabel_);
    resultEdit_ = new QPlainTextEdit(resultHost);
    resultEdit_->setLineWrapMode(QPlainTextEdit::WidgetWidth);
    resultEdit_->setTabChangesFocus(false);
    resultEdit_->setReadOnly(true);
    fullTextLayout->addWidget(resultEdit_, 1);
    resultSplitter_->addWidget(regionHost);
    resultSplitter_->addWidget(fullTextHost);
    resultSplitter_->setStretchFactor(0, 1);
    resultSplitter_->setStretchFactor(1, 2);
    resultSplitter_->setSizes({230, 360});
    statusLabel_ = new QLabel(resultHost);
    statusLabel_->setObjectName("ocrStatusLabel");
    statusLabel_->setContentsMargins(2, 0, 2, 0);
    resultLayout->addWidget(resultSplitter_, 1);
    resultLayout->addWidget(statusLabel_);

    splitter->addWidget(previewHost);
    splitter->addWidget(resultHost);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 2);
    splitter->setSizes({700, 480});
    rootLayout->addWidget(splitter, 1);

    connect(zoomInButton_, &QPushButton::clicked, this,
            [this]() { setZoomFactor(zoomFactor_ + 0.15); });
    connect(zoomOutButton_, &QPushButton::clicked, this,
            [this]() { setZoomFactor(zoomFactor_ - 0.15); });
    connect(fitButton_, &QPushButton::clicked, this, &OcrResultWindow::fitPreview);
    connect(copyButton_, &QPushButton::clicked, this, &OcrResultWindow::copyRecognizedText);
    connect(saveButton_, &QPushButton::clicked, this, &OcrResultWindow::saveRecognizedText);
    connect(runButton_, &QPushButton::clicked, this, &OcrResultWindow::runRecognition);
    connect(regionListWidget_, &QListWidget::currentRowChanged, this, [this](int row) {
        if (suppressTextCursorSync_) {
            return;
        }
        setActiveRegion(row, true);
    });
}

void OcrResultWindow::refreshTexts() {
    const auto& text = cappy::localization::strings(language_);
    setWindowTitle(text.dialogOcrTitle);
    titleLabel_->setText(text.dialogOcrTitle);
    imageMetaLabel_->setText(
        text.dialogOcrImageMetaTemplate.arg(image_.width()).arg(image_.height()));
    zoomInButton_->setToolTip(text.dialogOcrZoomIn);
    zoomOutButton_->setToolTip(text.dialogOcrZoomOut);
    fitButton_->setToolTip(text.dialogOcrFitImage);
    copyButton_->setToolTip(text.dialogOcrCopyText);
    saveButton_->setToolTip(text.dialogOcrSaveText);
    runButton_->setText(text.dialogOcrRun);
    zoomLabel_->setText(text.dialogOcrZoomTemplate.arg(qRound(zoomFactor_ * 100.0)));
    resultEdit_->setPlaceholderText(text.dialogOcrTextPlaceholder);
    statusLabel_->setText(text.readyStatus);
    const int localIndex = providerComboBox_->findData("local");
    const int cloudIndex = providerComboBox_->findData("cloud");
    if (localIndex >= 0) {
        providerComboBox_->setItemText(localIndex, text.settingsOcrProviderLocal);
    }
    if (cloudIndex >= 0) {
        providerComboBox_->setItemText(cloudIndex, text.settingsOcrProviderCloud);
    }
    previewTitleLabel_->setText(text.dialogOcrPreviewTitle);
    resultTitleLabel_->setText(text.dialogOcrResultTitle);
    regionTitleLabel_->setText(text.dialogOcrRegionListTitle);
    fullTextTitleLabel_->setText(text.dialogOcrFullTextTitle);
    refreshRegionList();
}

void OcrResultWindow::runRecognition() {
    if (ocrService_ == nullptr) {
        return;
    }

    settings_.preferredProvider = providerComboBox_->currentData().toString();
    ocrService_->recognize(
        image_, settings_,
        cappy::services::ocr::ocrProviderFromSettingsValue(settings_.preferredProvider));
}

void OcrResultWindow::setZoomFactor(double factor) {
    zoomFactor_ = std::clamp(factor, 0.2, 6.0);
    updatePreview();
}

void OcrResultWindow::updatePreview() {
    if (image_.isNull() || previewLabel_ == nullptr) {
        return;
    }

    const QSize scaledSize = image_.size() * zoomFactor_;
    QPixmap pixmap = QPixmap::fromImage(image_).scaled(scaledSize, Qt::KeepAspectRatio,
                                                       Qt::SmoothTransformation);
    if (!pixmap.isNull() && !currentResult_.regions.isEmpty()) {
        const bool darkMode = isDarkMode(appearanceMode_);
        const double scaleX = static_cast<double>(pixmap.width()) / std::max(1, image_.width());
        const double scaleY = static_cast<double>(pixmap.height()) / std::max(1, image_.height());

        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing, true);
        const double penWidth = std::clamp(1.15 * std::min(scaleX, scaleY), 1.0, 3.2);
        for (int index = 0; index < currentResult_.regions.size(); ++index) {
            const auto& region = currentResult_.regions.at(index);
            const bool isActive = index == activeRegionIndex_;
            const bool isHovered = index == hoveredRegionIndex_;
            const QColor strokeColor =
                isActive
                    ? (darkMode ? QColor(255, 194, 87, 248) : QColor(207, 111, 14, 244))
                    : (isHovered
                           ? (darkMode ? QColor(156, 214, 255, 232) : QColor(43, 122, 207, 228))
                           : (darkMode ? QColor(106, 183, 255, 214) : QColor(29, 115, 201, 210)));
            const QColor fillColor =
                isActive ? (darkMode ? QColor(255, 194, 87, 84) : QColor(240, 154, 64, 78))
                         : (isHovered
                                ? (darkMode ? QColor(96, 172, 236, 70) : QColor(84, 153, 219, 62))
                                : (darkMode ? QColor(72, 154, 230, 48) : QColor(63, 137, 214, 36)));
            const QRectF rect(region.rect.x() * scaleX, region.rect.y() * scaleY,
                              region.rect.width() * scaleX, region.rect.height() * scaleY);
            QPen pen(strokeColor);
            pen.setWidthF(isActive ? penWidth + 0.9 : (isHovered ? penWidth + 0.45 : penWidth));
            painter.setPen(pen);
            painter.setBrush(fillColor);
            painter.drawRoundedRect(rect, 4.0, 4.0);
        }
    }
    previewLabel_->setPixmap(pixmap);
    if (!pixmap.isNull()) {
        previewLabel_->resize(pixmap.size());
    }
    if (zoomLabel_ != nullptr) {
        zoomLabel_->setText(cappy::localization::strings(language_).dialogOcrZoomTemplate.arg(
            qRound(zoomFactor_ * 100.0)));
    }
}

void OcrResultWindow::fitPreview() {
    if (image_.isNull() || previewScrollArea_ == nullptr) {
        return;
    }

    const QSize viewportSize = previewScrollArea_->viewport()->size() - QSize(24, 24);
    const double widthFactor =
        static_cast<double>(viewportSize.width()) / std::max(1, image_.width());
    const double heightFactor =
        static_cast<double>(viewportSize.height()) / std::max(1, image_.height());
    setZoomFactor(std::min(widthFactor, heightFactor));
}

void OcrResultWindow::copyRecognizedText() {
    QApplication::clipboard()->setText(resultEdit_->toPlainText());
    statusLabel_->setText(cappy::localization::strings(language_).dialogOcrStatusCopiedText);
}

void OcrResultWindow::saveRecognizedText() {
    const auto& text = cappy::localization::strings(language_);
    const QString selectedPath = QFileDialog::getSaveFileName(
        this, text.dialogSaveAs, QStringLiteral("cappy-ocr.txt"), text.dialogOcrTextFileFilter);
    if (selectedPath.isEmpty()) {
        return;
    }

    QFile file(selectedPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        statusLabel_->setText(text.dialogOcrStatusSaveFailed);
        return;
    }

    QTextStream stream(&file);
    stream << resultEdit_->toPlainText();
    statusLabel_->setText(text.dialogOcrStatusSavedTextTemplate.arg(selectedPath));
}

void OcrResultWindow::refreshRegionList() {
    if (regionListWidget_ == nullptr) {
        return;
    }

    const auto& text = cappy::localization::strings(language_);
    suppressTextCursorSync_ = true;
    regionListWidget_->clear();
    for (int index = 0; index < currentResult_.regions.size(); ++index) {
        const auto& region = currentResult_.regions.at(index);
        const QString confidenceText =
            region.confidence >= 0 ? text.dialogOcrRegionConfidenceTemplate.arg(region.confidence)
                                   : text.dialogOcrRegionNoConfidence;
        auto* item = new QListWidgetItem(QStringLiteral("%1\n%2").arg(region.text, confidenceText),
                                         regionListWidget_);
        item->setData(Qt::UserRole, index);
        item->setToolTip(region.text);
        item->setSizeHint(QSize(0, 54));
    }

    if (activeRegionIndex_ >= 0 && activeRegionIndex_ < regionListWidget_->count()) {
        regionListWidget_->setCurrentRow(activeRegionIndex_);
    }
    suppressTextCursorSync_ = false;
}

void OcrResultWindow::setActiveRegion(int index, bool syncTextCursor) {
    if (index < -1 || index >= currentResult_.regions.size()) {
        index = -1;
    }
    if (activeRegionIndex_ == index) {
        return;
    }

    activeRegionIndex_ = index;
    if (regionListWidget_ != nullptr) {
        suppressTextCursorSync_ = true;
        if (index >= 0 && index < regionListWidget_->count()) {
            regionListWidget_->setCurrentRow(index);
            regionListWidget_->scrollToItem(regionListWidget_->item(index));
        } else {
            regionListWidget_->clearSelection();
        }
        suppressTextCursorSync_ = false;
    }
    updatePreview();

    if (!syncTextCursor || resultEdit_ == nullptr) {
        return;
    }
    if (index < 0) {
        return;
    }

    QTextBlock block = resultEdit_->document()->findBlockByNumber(index);
    if (!block.isValid()) {
        return;
    }

    QTextCursor cursor(block);
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    suppressTextCursorSync_ = true;
    resultEdit_->setTextCursor(cursor);
    resultEdit_->centerCursor();
    suppressTextCursorSync_ = false;
}

void OcrResultWindow::setHoveredRegion(int index) {
    if (index < -1 || index >= currentResult_.regions.size()) {
        index = -1;
    }
    if (hoveredRegionIndex_ == index) {
        return;
    }

    hoveredRegionIndex_ = index;
    updatePreview();
}

int OcrResultWindow::regionIndexForTextCursor() const {
    if (resultEdit_ == nullptr || currentResult_.regions.isEmpty()) {
        return -1;
    }

    const QTextCursor cursor = resultEdit_->textCursor();
    const int blockNumber = cursor.block().blockNumber();
    return (blockNumber >= 0 && blockNumber < currentResult_.regions.size()) ? blockNumber : -1;
}

int OcrResultWindow::regionIndexAtPreviewPosition(const QPoint& position) const {
    if (previewLabel_ == nullptr || image_.isNull() || currentResult_.regions.isEmpty()) {
        return -1;
    }

    const QPixmap pixmap = previewLabel_->pixmap(Qt::ReturnByValue);
    if (pixmap.isNull()) {
        return -1;
    }

    const QRect pixmapRect(QPoint(std::max(0, (previewLabel_->width() - pixmap.width()) / 2),
                                  std::max(0, (previewLabel_->height() - pixmap.height()) / 2)),
                           pixmap.size());
    if (!pixmapRect.contains(position)) {
        return -1;
    }

    const QPoint imagePosition = position - pixmapRect.topLeft();

    const double scaleX = static_cast<double>(pixmap.width()) / std::max(1, image_.width());
    const double scaleY = static_cast<double>(pixmap.height()) / std::max(1, image_.height());
    int bestIndex = -1;
    double bestArea = std::numeric_limits<double>::max();

    for (int index = 0; index < currentResult_.regions.size(); ++index) {
        const auto& region = currentResult_.regions.at(index);
        const QRectF rect(region.rect.x() * scaleX, region.rect.y() * scaleY,
                          region.rect.width() * scaleX, region.rect.height() * scaleY);
        if (!rect.contains(imagePosition)) {
            continue;
        }
        const double area = rect.width() * rect.height();
        if (area < bestArea) {
            bestArea = area;
            bestIndex = index;
        }
    }

    return bestIndex;
}

void OcrResultWindow::updateTextRegionSelection() {
    if (currentResult_.regions.isEmpty()) {
        activeRegionIndex_ = -1;
        return;
    }

    activeRegionIndex_ = 0;
}

} // namespace cappy::features::ocr
