#include "cappy/features/pinboard/PinnedImageWindow.h"

#include <algorithm>

#include <QApplication>
#include <QAction>
#include <QClipboard>
#include <QCloseEvent>
#include <QContextMenuEvent>
#include <QDateTime>
#include <QDir>
#include <QFileDialog>
#include <QGuiApplication>
#include <QIcon>
#include <QKeyEvent>
#include <QKeySequence>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPalette>
#include <QPixmap>
#include <QScreen>
#include <QStandardPaths>
#include <QStyle>
#include <QWheelEvent>

namespace cappy::features::pinboard {

namespace {

constexpr int kShadowMargin = 6;
constexpr int kMenuItemMinHeight = 28;
constexpr QSize kMenuIconSize(14, 14);

bool matchesShortcut(QKeyEvent* event, const QString& text) {
    if (event == nullptr || text.trimmed().isEmpty()) {
        return false;
    }

    const QKeySequence configured = QKeySequence::fromString(text, QKeySequence::PortableText);
    if (configured.isEmpty()) {
        return false;
    }

    int key = event->key();
    if (key == Qt::Key_Enter) {
        key = Qt::Key_Return;
    }
    const QKeySequence pressed(static_cast<int>(event->modifiers()) | key);
    return configured.matches(pressed) == QKeySequence::ExactMatch;
}

QIcon themedIcon(QWidget* widget, const QString& themeName, QStyle::StandardPixmap fallback) {
    const QIcon themeIcon = QIcon::fromTheme(themeName);
    if (!themeIcon.isNull()) {
        return themeIcon;
    }

    return widget->style()->standardIcon(fallback);
}

QIcon drawSymbolicIcon(QWidget* widget, const QString& id, const QSize& size = kMenuIconSize) {
    QPixmap pixmap(size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    const QColor stroke = widget->palette().color(QPalette::WindowText);
    painter.setPen(QPen(stroke, 1.6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.setBrush(Qt::NoBrush);

    const qreal w = size.width();
    const qreal h = size.height();

    if (id == "flip-horizontal") {
        painter.drawLine(QPointF(w / 2.0, 2.0), QPointF(w / 2.0, h - 2.0));
        painter.drawLine(QPointF(4.0, 4.0), QPointF(w / 2.0 - 1.5, h / 2.0));
        painter.drawLine(QPointF(4.0, h - 4.0), QPointF(w / 2.0 - 1.5, h / 2.0));
        painter.drawLine(QPointF(w - 4.0, 4.0), QPointF(w / 2.0 + 1.5, h / 2.0));
        painter.drawLine(QPointF(w - 4.0, h - 4.0), QPointF(w / 2.0 + 1.5, h / 2.0));
    } else if (id == "flip-vertical") {
        painter.drawLine(QPointF(2.0, h / 2.0), QPointF(w - 2.0, h / 2.0));
        painter.drawLine(QPointF(4.0, 4.0), QPointF(w / 2.0, h / 2.0 - 1.5));
        painter.drawLine(QPointF(w - 4.0, 4.0), QPointF(w / 2.0, h / 2.0 - 1.5));
        painter.drawLine(QPointF(4.0, h - 4.0), QPointF(w / 2.0, h / 2.0 + 1.5));
        painter.drawLine(QPointF(w - 4.0, h - 4.0), QPointF(w / 2.0, h / 2.0 + 1.5));
    } else if (id == "rotate-clockwise") {
        painter.drawArc(QRectF(2.5, 2.5, w - 5.0, h - 5.0), 30 * 16, 260 * 16);
        painter.drawLine(QPointF(w - 4.0, 5.0), QPointF(w - 2.0, 2.5));
        painter.drawLine(QPointF(w - 4.0, 5.0), QPointF(w - 7.0, 5.0));
    } else if (id == "rotate-counterclockwise") {
        painter.drawArc(QRectF(2.5, 2.5, w - 5.0, h - 5.0), -110 * 16, 260 * 16);
        painter.drawLine(QPointF(4.0, 5.0), QPointF(2.0, 2.5));
        painter.drawLine(QPointF(4.0, 5.0), QPointF(7.0, 5.0));
    } else if (id == "invert-colors") {
        painter.drawEllipse(QRectF(2.0, 2.0, w - 4.0, h - 4.0));
        painter.setBrush(stroke);
        painter.drawPie(QRectF(2.0, 2.0, w - 4.0, h - 4.0), 90 * 16, 180 * 16);
    } else if (id == "ocr") {
        painter.drawLine(QPointF(3.0, 5.0), QPointF(3.0, 3.0));
        painter.drawLine(QPointF(3.0, 3.0), QPointF(5.0, 3.0));
        painter.drawLine(QPointF(w - 3.0, 5.0), QPointF(w - 3.0, 3.0));
        painter.drawLine(QPointF(w - 3.0, 3.0), QPointF(w - 5.0, 3.0));
        painter.drawLine(QPointF(3.0, h - 5.0), QPointF(3.0, h - 3.0));
        painter.drawLine(QPointF(3.0, h - 3.0), QPointF(5.0, h - 3.0));
        painter.drawLine(QPointF(w - 3.0, h - 5.0), QPointF(w - 3.0, h - 3.0));
        painter.drawLine(QPointF(w - 3.0, h - 3.0), QPointF(w - 5.0, h - 3.0));
        painter.drawLine(QPointF(5.0, 5.5), QPointF(w - 5.0, 5.5));
        painter.drawLine(QPointF(5.0, h / 2.0), QPointF(w - 5.0, h / 2.0));
    }

    return QIcon(pixmap);
}

QAction* addMenuAction(QMenu* menu, const QIcon& icon, const QString& text) {
    QAction* action = menu->addAction(text);
    action->setIcon(QIcon(icon.pixmap(kMenuIconSize)));
    action->setIconVisibleInMenu(true);
    return action;
}

}  // namespace

PinnedImageWindow::PinnedImageWindow(
    const QImage& image,
    std::optional<QPoint> initialTopLeft,
    cappy::shortcuts::PinWindowShortcutSettings shortcuts,
    cappy::localization::AppLanguage language,
    QWidget* parent
)
    : QWidget(parent)
    , initialImage_(image)
    , originalImage_(image)
    , image_(image)
    , shortcuts_(std::move(shortcuts))
    , language_(cappy::localization::resolvedAppLanguage(language)) {
    setWindowFlags(
        Qt::Tool
        | Qt::FramelessWindowHint
        | Qt::WindowStaysOnTopHint
    );
    setAttribute(Qt::WA_DeleteOnClose, true);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    applyInteractionCursor();

    setWindowOpacity(opacityLevel_);
    updateWindowSize();

    if (initialTopLeft.has_value()) {
        move(*initialTopLeft - QPoint(kShadowMargin, kShadowMargin));
    } else {
        const QScreen* screen = QGuiApplication::primaryScreen();
        if (screen == nullptr) {
            return;
        }
        const QRect available = screen->availableGeometry();
        move(available.center() - rect().center());
    }
}

void PinnedImageWindow::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    const QRect targetRect = imageRect();
    for (int layer = kShadowMargin; layer >= 1; --layer) {
        const double progress = static_cast<double>(layer) / static_cast<double>(kShadowMargin);
        const int alpha = static_cast<int>(2 + progress * progress * 10);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(0, 0, 0, alpha));
        painter.drawRoundedRect(
            targetRect.adjusted(-layer, -layer + 1, layer, layer + 1),
            5 + layer * 0.28,
            5 + layer * 0.28
        );
    }

    painter.drawImage(targetRect, image_);
    painter.setPen(QPen(QColor(255, 255, 255, 28), 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(targetRect.adjusted(0, 0, -1, -1), 6, 6);
}

void PinnedImageWindow::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && !locked_) {
        dragging_ = true;
        dragOffset_ = event->globalPosition().toPoint() - frameGeometry().topLeft();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }

    QWidget::mousePressEvent(event);
}

void PinnedImageWindow::mouseDoubleClickEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        close();
        event->accept();
        return;
    }

    QWidget::mouseDoubleClickEvent(event);
}

void PinnedImageWindow::mouseMoveEvent(QMouseEvent* event) {
    if (dragging_) {
        move(event->globalPosition().toPoint() - dragOffset_);
        event->accept();
        return;
    }

    QWidget::mouseMoveEvent(event);
}

void PinnedImageWindow::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        dragging_ = false;
        setCursor(locked_ ? Qt::ArrowCursor : Qt::OpenHandCursor);
        event->accept();
        return;
    }

    QWidget::mouseReleaseEvent(event);
}

void PinnedImageWindow::wheelEvent(QWheelEvent* event) {
    if (event->modifiers().testFlag(Qt::ControlModifier)) {
        const double delta = event->angleDelta().y() > 0 ? 0.1 : -0.1;
        setWindowOpacityLevel(opacityLevel_ + delta);
        event->accept();
        return;
    }

    const double delta = event->angleDelta().y() > 0 ? 0.1 : -0.1;
    setScaleFactor(scaleFactor_ + delta);
    event->accept();
}

void PinnedImageWindow::contextMenuEvent(QContextMenuEvent* event) {
    QMenu menu(this);
    menu.setWindowFlag(Qt::NoDropShadowWindowHint, true);
    menu.setStyleSheet(QString(
        "QMenu {"
        "  margin: 0px;"
        "  padding: 1px 4px 1px 6px;"
        "}"
        "QMenu::item {"
        "  padding: 4px 12px 4px 10px;"
        "  min-height: %1px;"
        "}"
        "QMenu::separator {"
        "  height: 1px;"
        "  margin: 4px 8px;"
        "}"
    ).arg(kMenuItemMinHeight));
    const auto& text = cappy::localization::strings(language_);
    QAction* copyAction = addMenuAction(
        &menu,
        themedIcon(this, "edit-copy", QStyle::SP_FileDialogListView),
        text.actionCopyToClipboard
    );
    QAction* saveAsAction = addMenuAction(
        &menu,
        themedIcon(this, "document-save", QStyle::SP_DialogSaveButton),
        text.dialogSaveAs + "..."
    );
    QAction* changeImageAction = addMenuAction(
        &menu,
        themedIcon(this, "document-open", QStyle::SP_DirOpenIcon),
        text.actionChangeImage
    );
    menu.addSeparator();
    QAction* flipHorizontalAction = addMenuAction(
        &menu,
        drawSymbolicIcon(this, "flip-horizontal"),
        text.actionFlipHorizontal
    );
    QAction* flipVerticalAction = addMenuAction(
        &menu,
        drawSymbolicIcon(this, "flip-vertical"),
        text.actionFlipVertical
    );
    QAction* rotateClockwiseAction = addMenuAction(
        &menu,
        drawSymbolicIcon(this, "rotate-clockwise"),
        text.actionRotateClockwise90
    );
    QAction* rotateCounterclockwiseAction = addMenuAction(
        &menu,
        drawSymbolicIcon(this, "rotate-counterclockwise"),
        text.actionRotateCounterclockwise90
    );
    QAction* invertColorsAction = addMenuAction(
        &menu,
        drawSymbolicIcon(this, "invert-colors"),
        text.actionInvertColors
    );
    QAction* lockAction = addMenuAction(
        &menu,
        themedIcon(this, "object-locked", QStyle::SP_MessageBoxWarning),
        text.actionLock
    );
    lockAction->setCheckable(true);
    lockAction->setChecked(locked_);
    QAction* extractTextAction = addMenuAction(
        &menu,
        drawSymbolicIcon(this, "ocr"),
        text.actionExtractText
    );
    QAction* restoreAction = addMenuAction(
        &menu,
        themedIcon(this, "edit-undo", QStyle::SP_ArrowBack),
        text.actionRestore
    );
    QAction* closeAction = addMenuAction(
        &menu,
        themedIcon(this, "window-close", QStyle::SP_DialogCloseButton),
        text.actionClose
    );
    QAction* selected = menu.exec(event->globalPos());
    if (selected == copyAction) {
        copyImageToClipboard();
    } else if (selected == saveAsAction) {
        saveImageAs();
    } else if (selected == changeImageAction) {
        changeImage();
    } else if (selected == flipHorizontalAction) {
        flipHorizontal();
    } else if (selected == flipVerticalAction) {
        flipVertical();
    } else if (selected == rotateClockwiseAction) {
        rotateClockwise();
    } else if (selected == rotateCounterclockwiseAction) {
        rotateCounterclockwise();
    } else if (selected == invertColorsAction) {
        invertColors();
    } else if (selected == lockAction) {
        setLocked(lockAction->isChecked());
    } else if (selected == extractTextAction) {
        emit ocrRequested(image_);
    } else if (selected == restoreAction) {
        restoreOriginalState();
    } else if (selected == closeAction) {
        close();
    }
}

void PinnedImageWindow::closeEvent(QCloseEvent* event) {
    emit pinWindowClosed();
    QWidget::closeEvent(event);
}

void PinnedImageWindow::keyPressEvent(QKeyEvent* event) {
    if (matchesShortcut(event, shortcuts_.close)) {
        close();
        return;
    }
    if (matchesShortcut(event, shortcuts_.scaleUp)) {
        setScaleFactor(scaleFactor_ + 0.1);
        return;
    }
    if (matchesShortcut(event, shortcuts_.scaleDown)) {
        setScaleFactor(scaleFactor_ - 0.1);
        return;
    }
    if (matchesShortcut(event, shortcuts_.resetScaleAndOpacity)) {
        setScaleFactor(1.0);
        setWindowOpacityLevel(1.0);
        return;
    }
    if (matchesShortcut(event, shortcuts_.opacityDown)) {
        setWindowOpacityLevel(opacityLevel_ - 0.1);
        return;
    }
    if (matchesShortcut(event, shortcuts_.opacityUp)) {
        setWindowOpacityLevel(opacityLevel_ + 0.1);
        return;
    }
    if (matchesShortcut(event, shortcuts_.toggleLock)) {
        setLocked(!locked_);
        return;
    }
    if (matchesShortcut(event, shortcuts_.toggleClickThrough)) {
        setClickThrough(!clickThrough_);
        return;
    }

    QWidget::keyPressEvent(event);
}

bool PinnedImageWindow::isClickThrough() const {
    return clickThrough_;
}

void PinnedImageWindow::applyShortcutSettings(
    const cappy::shortcuts::PinWindowShortcutSettings& shortcuts
) {
    shortcuts_ = shortcuts;
}

void PinnedImageWindow::setLanguage(cappy::localization::AppLanguage language) {
    language_ = cappy::localization::resolvedAppLanguage(language);
}

void PinnedImageWindow::setClickThrough(bool enabled) {
    if (clickThrough_ == enabled) {
        return;
    }

    clickThrough_ = enabled;
    dragging_ = false;
    const QRect currentGeometry = geometry();
    const bool wasVisible = isVisible();
    setWindowFlag(Qt::WindowTransparentForInput, clickThrough_);
    setGeometry(currentGeometry);
    if (wasVisible) {
        show();
    }
    applyInteractionCursor();
    update();
}

void PinnedImageWindow::applyInteractionCursor() {
    setCursor((locked_ || clickThrough_) ? Qt::ArrowCursor : Qt::OpenHandCursor);
}

void PinnedImageWindow::copyImageToClipboard() {
    if (image_.isNull()) {
        return;
    }

    QApplication::clipboard()->setImage(image_);
}

void PinnedImageWindow::changeImage() {
    QString defaultDirectory = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    if (defaultDirectory.isEmpty()) {
        defaultDirectory = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    }

    const QString selectedPath = QFileDialog::getOpenFileName(
        this,
        cappy::localization::strings(language_).dialogOpenImage,
        defaultDirectory,
        cappy::localization::strings(language_).fileDialogImageFilter
    );
    if (selectedPath.isEmpty()) {
        return;
    }

    const QImage loadedImage(selectedPath);
    if (loadedImage.isNull()) {
        return;
    }

    replaceImageForEditing(loadedImage);
    setScaleFactor(1.0);
    setWindowOpacityLevel(1.0);
    setLocked(false);
}

void PinnedImageWindow::saveImageAs() {
    if (image_.isNull()) {
        return;
    }

    QString defaultDirectory = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    if (defaultDirectory.isEmpty()) {
        defaultDirectory = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    }

    const QString defaultFileName = QString("cappy-pin-%1.png")
        .arg(QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss"));
    const QString defaultPath = defaultDirectory.isEmpty()
        ? defaultFileName
        : QDir(defaultDirectory).filePath(defaultFileName);
    const QString selectedPath = QFileDialog::getSaveFileName(
        this,
        cappy::localization::strings(language_).dialogSaveAs,
        defaultPath,
        cappy::localization::strings(language_).fileDialogImageFilter
    );
    if (selectedPath.isEmpty()) {
        return;
    }

    image_.save(selectedPath);
}

void PinnedImageWindow::flipHorizontal() {
    if (image_.isNull()) {
        return;
    }

    setCurrentImage(image_.mirrored(true, false));
}

void PinnedImageWindow::flipVertical() {
    if (image_.isNull()) {
        return;
    }

    setCurrentImage(image_.mirrored(false, true));
}

void PinnedImageWindow::rotateClockwise() {
    if (image_.isNull()) {
        return;
    }

    QTransform transform;
    transform.rotate(90.0);
    setCurrentImage(image_.transformed(transform, Qt::SmoothTransformation));
}

void PinnedImageWindow::rotateCounterclockwise() {
    if (image_.isNull()) {
        return;
    }

    QTransform transform;
    transform.rotate(-90.0);
    setCurrentImage(image_.transformed(transform, Qt::SmoothTransformation));
}

void PinnedImageWindow::invertColors() {
    if (image_.isNull()) {
        return;
    }

    QImage inverted = image_.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    inverted.invertPixels();
    setCurrentImage(inverted);
}

void PinnedImageWindow::restoreOriginalState() {
    if (initialImage_.isNull()) {
        return;
    }

    originalImage_ = initialImage_;
    setCurrentImage(initialImage_);
    setScaleFactor(1.0);
    setWindowOpacityLevel(1.0);
    setLocked(false);
}

void PinnedImageWindow::setLocked(bool locked) {
    locked_ = locked;
    dragging_ = false;
    applyInteractionCursor();
    update();
}

void PinnedImageWindow::updateWindowSize() {
    if (image_.isNull()) {
        resize(200 + kShadowMargin * 2, 120 + kShadowMargin * 2);
        return;
    }

    const QSize scaledSize = image_.size() * scaleFactor_;
    resize(scaledSize + QSize(kShadowMargin * 2, kShadowMargin * 2));
    update();
}

void PinnedImageWindow::setScaleFactor(double scaleFactor) {
    scaleFactor_ = std::clamp(scaleFactor, 0.2, 4.0);
    updateWindowSize();
}

void PinnedImageWindow::setWindowOpacityLevel(double opacityLevel) {
    opacityLevel_ = std::clamp(opacityLevel, 0.2, 1.0);
    setWindowOpacity(opacityLevel_);
}

void PinnedImageWindow::setCurrentImage(const QImage& image) {
    if (image.isNull()) {
        return;
    }

    image_ = image;
    updateWindowSize();
}

void PinnedImageWindow::replaceImageForEditing(const QImage& image) {
    if (image.isNull()) {
        return;
    }

    originalImage_ = image;
    setCurrentImage(image);
}

QRect PinnedImageWindow::imageRect() const {
    return rect().adjusted(
        kShadowMargin,
        kShadowMargin,
        -kShadowMargin,
        -kShadowMargin
    );
}

}  // namespace cappy::features::pinboard
