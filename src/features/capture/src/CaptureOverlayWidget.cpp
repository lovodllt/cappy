#include "cappy/features/capture/CaptureOverlayWidget.h"

#include <algorithm>
#include <cmath>

#include <QAction>
#include <QApplication>
#include <QBoxLayout>
#include <QCursor>
#include <QEvent>
#include <QFrame>
#include <QIcon>
#include <QKeyEvent>
#include <QKeySequence>
#include <QLineEdit>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QScreen>
#include <QShowEvent>
#include <QStyle>
#include <QToolButton>
#include <QStringList>

#include "cappy/domain/capture/CaptureImageOps.h"

namespace cappy::features::capture {

namespace {

constexpr double kPi = 3.14159265358979323846;
constexpr int kToolbarGap = 10;
constexpr int kButtonSize = 28;
constexpr int kHandleRadius = 5;
constexpr int kHandleHitDistance = 12;
constexpr int kMinimumSelectionSize = 24;
constexpr QSize kIconSize(16, 16);
constexpr int kClickDragThreshold = 6;

QIcon themedIcon(QWidget* widget, const QString& themeName, QStyle::StandardPixmap fallback) {
    const QIcon themeIcon = QIcon::fromTheme(themeName);
    if (!themeIcon.isNull()) {
        return themeIcon;
    }

    return widget->style()->standardIcon(fallback);
}

QIcon drawSymbolicIcon(const QString& id, const QSize& size = kIconSize) {
    QPixmap pixmap(size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(QColor(245, 245, 245), 1.8, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.setBrush(Qt::NoBrush);

    const QRectF r(1.5, 1.5, size.width() - 3.0, size.height() - 3.0);

    if (id == "rectangle") {
        painter.drawRect(r.adjusted(2, 2, -2, -2));
    } else if (id == "ellipse") {
        painter.drawEllipse(r.adjusted(2, 2, -2, -2));
    } else if (id == "arrow") {
        painter.drawLine(QPointF(3, size.height() - 4), QPointF(size.width() - 4, 4));
        painter.drawLine(QPointF(size.width() - 4, 4), QPointF(size.width() - 7, 9));
        painter.drawLine(QPointF(size.width() - 4, 4), QPointF(size.width() - 9, 4));
    } else if (id == "pen") {
        painter.drawLine(QPointF(4, size.height() - 4), QPointF(size.width() - 5, 5));
        painter.drawLine(QPointF(size.width() - 6, 4), QPointF(size.width() - 3, 7));
    } else if (id == "marker") {
        painter.setPen(QPen(QColor(255, 235, 59), 5.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawLine(QPointF(3, size.height() - 5), QPointF(size.width() - 3, 5));
    } else if (id == "mosaic") {
        painter.setBrush(QColor(245, 245, 245));
        const int block = 4;
        for (int y = 2; y < size.height() - 2; y += block) {
            for (int x = 2; x < size.width() - 2; x += block) {
                if (((x + y) / block) % 2 == 0) {
                    painter.drawRect(QRect(x, y, block - 1, block - 1));
                }
            }
        }
    } else if (id == "text") {
        QFont font;
        font.setBold(true);
        font.setPixelSize(13);
        painter.setFont(font);
        painter.drawText(QRect(0, 0, size.width(), size.height()), Qt::AlignCenter, "T");
    } else if (id == "serial") {
        painter.drawEllipse(r.adjusted(1, 1, -1, -1));
        QFont font;
        font.setBold(true);
        font.setPixelSize(10);
        painter.setFont(font);
        painter.drawText(QRect(0, 0, size.width(), size.height()), Qt::AlignCenter, "1");
    } else if (id == "pin") {
        painter.drawLine(QPointF(size.width() / 2.0, 3), QPointF(size.width() / 2.0, size.height() - 4));
        painter.drawLine(QPointF(4, 6), QPointF(size.width() - 4, 6));
        painter.drawLine(QPointF(5, 6), QPointF(size.width() / 2.0, 11));
        painter.drawLine(QPointF(size.width() - 5, 6), QPointF(size.width() / 2.0, 11));
    } else if (id == "copy") {
        painter.drawRect(QRectF(5, 3, 8, 9));
        painter.drawRect(QRectF(3, 5, 8, 9));
    } else if (id == "save") {
        painter.drawRect(r.adjusted(2, 2, -2, -2));
        painter.drawLine(QPointF(4, 5), QPointF(size.width() - 4, 5));
        painter.drawRect(QRectF(5, 8, 6, 4));
    } else if (id == "close") {
        painter.drawLine(QPointF(4, 4), QPointF(size.width() - 4, size.height() - 4));
        painter.drawLine(QPointF(size.width() - 4, 4), QPointF(4, size.height() - 4));
    } else if (id == "grip") {
        painter.setBrush(QColor(245, 245, 245));
        painter.setPen(Qt::NoPen);
        for (int row = 0; row < 3; ++row) {
            for (int col = 0; col < 2; ++col) {
                painter.drawEllipse(
                    QPointF(5.0 + col * 4.0, 4.5 + row * 4.0),
                    0.95,
                    0.95
                );
            }
        }
    } else if (id == "resize") {
        painter.drawLine(QPointF(5.0, size.height() - 4.0), QPointF(size.width() - 4.0, 5.0));
        painter.drawLine(QPointF(8.0, size.height() - 4.0), QPointF(size.width() - 4.0, 8.0));
        painter.drawLine(QPointF(11.0, size.height() - 4.0), QPointF(size.width() - 4.0, 11.0));
    }

    return QIcon(pixmap);
}

QToolButton* createToolbarButton(QWidget* parent, QAction* action) {
    auto* button = new QToolButton(parent);
    button->setAutoRaise(true);
    button->setCursor(Qt::PointingHandCursor);
    button->setDefaultAction(action);
    button->setToolButtonStyle(Qt::ToolButtonIconOnly);
    button->setIconSize(kIconSize);
    button->setFixedSize(kButtonSize, kButtonSize);
    return button;
}

QFrame* createSeparator(QWidget* parent) {
    auto* separator = new QFrame(parent);
    separator->setFrameShape(QFrame::VLine);
    separator->setFrameShadow(QFrame::Plain);
    separator->setStyleSheet("color: rgba(255, 255, 255, 80);");
    return separator;
}

QKeySequence shortcutFromText(const QString& text) {
    return QKeySequence::fromString(text, QKeySequence::PortableText);
}

bool matchesShortcut(QKeyEvent* event, const QString& text) {
    if (event == nullptr || text.trimmed().isEmpty()) {
        return false;
    }

    const QKeySequence configured = shortcutFromText(text);
    if (configured.isEmpty()) {
        return false;
    }

    int key = event->key();
    if (key == Qt::Key_Enter) {
        key = Qt::Key_Return;
    }
    const QKeySequence pressed(
        static_cast<int>(event->modifiers()) | key
    );
    return configured.matches(pressed) == QKeySequence::ExactMatch;
}

QString joinShortcutLabels(std::initializer_list<QString> shortcuts) {
    QStringList labels;
    for (const QString& shortcut : shortcuts) {
        const QKeySequence sequence = shortcutFromText(shortcut);
        if (!sequence.isEmpty()) {
            labels.push_back(sequence.toString(QKeySequence::NativeText));
        }
    }
    return labels.join(" / ");
}

}  // namespace

CaptureOverlayWidget::CaptureOverlayWidget(
    const cappy::domain::capture::DesktopFrame& desktopFrame,
    cappy::shortcuts::CaptureOverlayShortcutSettings shortcuts,
    cappy::localization::AppLanguage language,
    std::optional<QRect> initialSelection,
    cappy::domain::capture::CaptureMode captureMode,
    WindowGeometryResolver windowGeometryResolver,
    QWidget* parent
)
    : QWidget(parent)
    , desktopFrame_(desktopFrame)
    , captureMode_(captureMode)
    , shortcuts_(std::move(shortcuts))
    , language_(cappy::localization::resolvedAppLanguage(language))
    , windowGeometryResolver_(std::move(windowGeometryResolver)) {
    setWindowFlags(
        Qt::FramelessWindowHint
        | Qt::Tool
        | Qt::WindowStaysOnTopHint
        | Qt::BypassWindowManagerHint
    );
    setAttribute(Qt::WA_DeleteOnClose, true);
    setAttribute(Qt::WA_NoSystemBackground, false);
    setMouseTracking(true);
    setCursor(Qt::CrossCursor);
    setFocusPolicy(Qt::StrongFocus);
    setGeometry(desktopFrame_.geometry);

    setupToolbar();
    updateToolActionState();
    updateUndoRedoState();

    if (initialSelection.has_value()) {
        applyInitialSelection(*initialSelection);
    }
}

void CaptureOverlayWidget::setLanguage(cappy::localization::AppLanguage language) {
    language_ = cappy::localization::resolvedAppLanguage(language);
    refreshActionTooltips();
    update();
}

void CaptureOverlayWidget::closeEvent(QCloseEvent* event) {
    if (QWidget::keyboardGrabber() == this) {
        releaseKeyboard();
    }
    QWidget::closeEvent(event);
}

void CaptureOverlayWidget::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    setGeometry(desktopFrame_.geometry);
    raise();
    activateWindow();
    setFocus(Qt::ActiveWindowFocusReason);
    grabKeyboard();
    if (!hasSelection()) {
        updateTrackedWindowSelection(clampToBounds(mapFromGlobal(QCursor::pos())));
    }
}

void CaptureOverlayWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.drawImage(rect(), desktopFrame_.image);
    painter.fillRect(rect(), QColor(0, 0, 0, 120));

    if (hasSelection()) {
        const QRect selection = localSelectionRect();
        if (!workingImage_.isNull()) {
            painter.drawImage(selection, workingImage_);
        } else {
            painter.drawImage(selection, desktopFrame_.image, selection);
        }

        painter.save();
        painter.setClipRect(selection);
        drawTextAnnotations(painter);
        painter.restore();

        if (dragging_ && activeTool_ != Tool::Pencil && activeTool_ != Tool::Marker
            && selectionMode_ == SelectionMode::None) {
            painter.save();
            painter.setClipRect(selection);
            drawShapePreview(painter);
            painter.restore();
        }

        if (!dragging_ && activeTool_ == Tool::Serial && hasHoverPoint_ && isWithinSelection(hoverPoint_)) {
            painter.save();
            painter.setClipRect(selection);
            drawSerialPreview(painter);
            painter.restore();
        }

        painter.setPen(QPen(Qt::white, 2));
        painter.drawRect(selection.adjusted(0, 0, -1, -1));

        painter.setBrush(Qt::white);
        painter.setPen(Qt::NoPen);
        const QList<QPoint> handles = {
            selection.topLeft(),
            selection.topRight(),
            selection.bottomLeft(),
            selection.bottomRight(),
            QPoint(selection.left(), selection.center().y()),
            QPoint(selection.right(), selection.center().y()),
            QPoint(selection.center().x(), selection.top()),
            QPoint(selection.center().x(), selection.bottom()),
        };
        for (const QPoint& handle : handles) {
            painter.drawEllipse(handle, kHandleRadius, kHandleRadius);
        }

        painter.setPen(Qt::white);
        painter.setBrush(QColor(0, 0, 0, 180));
        const QString sizeText = QString("%1 x %2").arg(selection.width()).arg(selection.height());
        const QRect textRect = QRect(selection.topLeft() + QPoint(8, 8), QSize(120, 28));
        painter.drawRoundedRect(textRect, 4, 4);
        painter.drawText(textRect, Qt::AlignCenter, sizeText);
    } else if (!trackedWindowSelectionRect_.isEmpty()) {
        const QRect selection = trackedWindowSelectionRect_;
        painter.drawImage(selection, desktopFrame_.image, selection);
        painter.setPen(QPen(Qt::white, 2));
        painter.drawRect(selection.adjusted(0, 0, -1, -1));

        painter.setPen(Qt::white);
        painter.setBrush(QColor(0, 0, 0, 180));
        const QString sizeText = QString("%1 x %2").arg(selection.width()).arg(selection.height());
        const QRect textRect = QRect(selection.topLeft() + QPoint(8, 8), QSize(120, 28));
        painter.drawRoundedRect(textRect, 4, 4);
        painter.drawText(textRect, Qt::AlignCenter, sizeText);
    } else {
        painter.setPen(Qt::white);
        painter.drawText(
            QRect(16, 16, width() - 32, 32),
            Qt::AlignLeft | Qt::AlignVCenter,
            cappy::localization::strings(language_).overlayInstruction
        );
    }
}

void CaptureOverlayWidget::mousePressEvent(QMouseEvent* event) {
    if (textEntryActive_ && textInputPanel_ != nullptr
        && !textInputPanel_->geometry().contains(event->pos())) {
        commitTextEntry();
    }

    if (event->button() == Qt::RightButton) {
        if (activeTool_ != Tool::None) {
            setActiveTool(Tool::None);
            event->accept();
            return;
        }

        emit captureCanceled();
        close();
        return;
    }

    if (event->button() != Qt::LeftButton) {
        QWidget::mousePressEvent(event);
        return;
    }

    const QPoint point = clampToBounds(event->pos());
    if (!hasSelection()) {
        if (!trackedWindowSelectionRect_.isEmpty()) {
            dragging_ = true;
            trackingClickPending_ = true;
            dragStart_ = point;
            dragCurrent_ = point;
            event->accept();
            return;
        }
        beginSelectionGesture(point);
        event->accept();
        return;
    }

    if (activeTool_ == Tool::None) {
        const QPoint imagePoint = mapOverlayPointToImage(point);
        bool resizeHandle = false;
        const int annotationIndex = hitTestTextAnnotation(imagePoint, &resizeHandle);
        if (annotationIndex >= 0) {
            pushUndoSnapshot();
            selectedTextAnnotationIndex_ = annotationIndex;
            if (resizeHandle) {
                textAnnotationResizing_ = true;
            } else {
                textAnnotationDragging_ = true;
                textAnnotationDragOffset_ = imagePoint - textAnnotations_[annotationIndex].rect.topLeft();
            }
            update();
            event->accept();
            return;
        }
        if (selectionCanBeAdjusted()) {
            const SelectionMode hit = hitTestSelection(point);
            if (hit != SelectionMode::None) {
                beginMoveOrResize(point, hit);
                event->accept();
                return;
            }
        }
    }

    if (!isWithinSelection(point)) {
        QWidget::mousePressEvent(event);
        return;
    }

    if (isPointTool()) {
        handlePointTool(point);
        event->accept();
        return;
    }

    if (activeTool_ == Tool::Pencil || activeTool_ == Tool::Marker) {
        pushUndoSnapshot();
        lastPencilPoint_ = mapOverlayPointToImage(point);
        dragging_ = true;
        event->accept();
        return;
    }

    if (activeTool_ != Tool::None) {
        pushUndoSnapshot();
        dragStart_ = point;
        dragCurrent_ = point;
        dragging_ = true;
        event->accept();
        return;
    }

    QWidget::mousePressEvent(event);
}

void CaptureOverlayWidget::mouseMoveEvent(QMouseEvent* event) {
    const QPoint point = clampToBounds(event->pos());
    hasHoverPoint_ = isWithinSelection(point);
    if (hasHoverPoint_) {
        hoverPoint_ = point;
    }
    updateInteractionCursor(point);

    if (selectedTextAnnotationIndex_ >= 0
        && selectedTextAnnotationIndex_ < textAnnotations_.size()
        && (textAnnotationDragging_ || textAnnotationResizing_)) {
        auto& annotation = textAnnotations_[selectedTextAnnotationIndex_];
        const QPoint imagePoint = mapOverlayPointToImage(point);
        if (textAnnotationDragging_) {
            QPoint topLeft = imagePoint - textAnnotationDragOffset_;
            topLeft.setX(std::clamp(topLeft.x(), 0, std::max(0, workingImage_.width() - annotation.rect.width())));
            topLeft.setY(std::clamp(topLeft.y(), 0, std::max(0, workingImage_.height() - annotation.rect.height())));
            annotation.rect.moveTopLeft(topLeft);
        } else if (textAnnotationResizing_) {
            const int nextPixelSize = std::clamp(
                imagePoint.y() - annotation.rect.top() + 10,
                12,
                72
            );
            annotation.pixelSize = nextPixelSize;
            annotation.rect = normalizedTextRect(annotation.text, annotation.pixelSize, annotation.rect.topLeft());
            if (annotation.rect.right() > workingImage_.width() - 8) {
                annotation.rect.moveRight(workingImage_.width() - 8);
            }
            if (annotation.rect.bottom() > workingImage_.height() - 8) {
                annotation.rect.moveBottom(workingImage_.height() - 8);
            }
        }
        update();
        updateInteractionCursor(point);
        event->accept();
        return;
    }

    if (!hasSelection()) {
        if (!dragging_) {
            updateTrackedWindowSelection(point);
        } else if (trackingClickPending_) {
            dragCurrent_ = point;
            if ((dragCurrent_ - dragStart_).manhattanLength() >= kClickDragThreshold) {
                trackingClickPending_ = false;
                trackedWindowSelectionRect_ = {};
                captureMode_ = cappy::domain::capture::CaptureMode::Region;
                selectionMode_ = SelectionMode::Create;
                selectionRect_ = clampSelectionRect(normalizedSelectionRect());
                updateInteractionCursor(point);
                update();
            }
            event->accept();
            return;
        }
    }

    if (!dragging_) {
        if (activeTool_ == Tool::Serial) {
            update();
        }
        QWidget::mouseMoveEvent(event);
        return;
    }

    if (selectionMode_ != SelectionMode::None) {
        updateSelectionGeometry(point);
        updateInteractionCursor(point);
        event->accept();
        return;
    }

    if (activeTool_ == Tool::Pencil || activeTool_ == Tool::Marker) {
        const QPoint nextPoint = mapOverlayPointToImage(point);
        QPainter painter(&workingImage_);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setPen(QPen(
            activeStrokeColor(),
            activeStrokeWidth(),
            Qt::SolidLine,
            Qt::RoundCap,
            Qt::RoundJoin
        ));
        painter.drawLine(lastPencilPoint_, nextPoint);
        lastPencilPoint_ = nextPoint;
        hasEdits_ = true;
        update();
        event->accept();
        return;
    }

    dragCurrent_ = point;
    update();
    event->accept();
}

void CaptureOverlayWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && (textAnnotationDragging_ || textAnnotationResizing_)) {
        textAnnotationDragging_ = false;
        textAnnotationResizing_ = false;
        update();
        event->accept();
        return;
    }

    if (event->button() != Qt::LeftButton || !dragging_) {
        QWidget::mouseReleaseEvent(event);
        return;
    }

    const QPoint point = clampToBounds(event->pos());

    if (trackingClickPending_) {
        dragging_ = false;
        trackingClickPending_ = false;
        if (!trackedWindowSelectionRect_.isEmpty()) {
            selectionRect_ = trackedWindowSelectionRect_;
            captureMode_ = cappy::domain::capture::CaptureMode::WindowFit;
            refreshWorkingImageFromSelection();
            positionToolbar();
            update();
        }
        event->accept();
        return;
    }

    if (selectionMode_ == SelectionMode::Create || selectionMode_ != SelectionMode::None) {
        dragging_ = false;
        if (selectionMode_ == SelectionMode::Create) {
            selectionRect_ = normalizedSelectionRect();
            selectionRect_ = clampSelectionRect(selectionRect_);
            if (selectionRect_.width() < kMinimumSelectionSize
                || selectionRect_.height() < kMinimumSelectionSize) {
                selectionRect_ = {};
                selectionMode_ = SelectionMode::None;
                positionToolbar();
                update();
                return;
            }
            refreshWorkingImageFromSelection();
            positionToolbar();
        }
        selectionMode_ = SelectionMode::None;
        update();
        event->accept();
        return;
    }

    if (activeTool_ == Tool::Pencil || activeTool_ == Tool::Marker) {
        const QPoint nextPoint = mapOverlayPointToImage(point);
        if (nextPoint == lastPencilPoint_) {
            QPainter painter(&workingImage_);
            painter.setRenderHint(QPainter::Antialiasing, true);
            painter.setPen(QPen(
                activeStrokeColor(),
                activeStrokeWidth(),
                Qt::SolidLine,
                Qt::RoundCap,
                Qt::RoundJoin
            ));
            painter.drawPoint(nextPoint);
        }

        dragging_ = false;
        hasEdits_ = true;
        updateUndoRedoState();
        update();
        return;
    }

    dragCurrent_ = point;
    commitCurrentShape();
    event->accept();
}

void CaptureOverlayWidget::keyPressEvent(QKeyEvent* event) {
    handleShortcut(event);
    if (event->isAccepted()) {
        return;
    }

    QWidget::keyPressEvent(event);
}

void CaptureOverlayWidget::leaveEvent(QEvent* event) {
    hasHoverPoint_ = false;
    updateInteractionCursor(QPoint(-1, -1));
    update();
    QWidget::leaveEvent(event);
}

bool CaptureOverlayWidget::eventFilter(QObject* watched, QEvent* event) {
    if ((watched == textInput_ || watched == textInputPanel_) && textInput_ != nullptr) {
        if (event->type() == QEvent::KeyPress) {
            auto* keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_Escape) {
                cancelTextEntry();
                return true;
            }
        }
    }

    if ((watched == textMoveHandleButton_ || watched == textInputPanel_) && textInputPanel_ != nullptr) {
        if (event->type() == QEvent::MouseButtonPress) {
            auto* mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                textInputDragging_ = true;
                textInputDragOffset_ = mouseEvent->globalPosition().toPoint()
                    - textInputPanel_->frameGeometry().topLeft();
                return true;
            }
        } else if (event->type() == QEvent::MouseMove && textInputDragging_) {
            auto* mouseEvent = static_cast<QMouseEvent*>(event);
            const QPoint topLeft = clampTextInputPanelTopLeft(
                mouseEvent->globalPosition().toPoint() - textInputDragOffset_
            );
            textInputPanel_->move(topLeft);
            updatePendingTextPointFromPanel();
            return true;
        } else if (event->type() == QEvent::MouseButtonRelease && textInputDragging_) {
            auto* mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                textInputDragging_ = false;
                return true;
            }
        }
    }

    if (watched == textScaleDownButton_ && textInputPanel_ != nullptr) {
        if (event->type() == QEvent::MouseButtonPress) {
            auto* mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                textInputResizing_ = true;
                textInputResizeStartSize_ = textInputPanel_->size();
                textInputResizeStartGlobalPos_ = mouseEvent->globalPosition().toPoint();
                return true;
            }
        } else if (event->type() == QEvent::MouseMove && textInputResizing_) {
            auto* mouseEvent = static_cast<QMouseEvent*>(event);
            const QPoint delta = mouseEvent->globalPosition().toPoint() - textInputResizeStartGlobalPos_;
            const QRect bounds = localSelectionRect().adjusted(8, 8, -8, -8);
            const QSize nextSize(
                std::clamp(textInputResizeStartSize_.width() + delta.x(), 220, bounds.width()),
                std::clamp(textInputResizeStartSize_.height() + delta.y(), 36, bounds.height())
            );
            textInputPanel_->resize(nextSize);
            pendingTextPixelSize_ = std::clamp(nextSize.height() - 18, 12, 56);
            updateTextInputAppearance();
            return true;
        } else if (event->type() == QEvent::MouseButtonRelease && textInputResizing_) {
            auto* mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                textInputResizing_ = false;
                return true;
            }
        }
    }

    return QWidget::eventFilter(watched, event);
}

void CaptureOverlayWidget::setupToolbar() {
    toolbar_ = new QFrame(this);
    toolbar_->setObjectName("captureOverlayToolbar");
    toolbar_->setStyleSheet(
        "#captureOverlayToolbar {"
        "  background: rgba(24, 24, 24, 220);"
        "  border: 1px solid rgba(255, 255, 255, 64);"
        "  border-radius: 6px;"
        "}"
        "QToolButton {"
        "  border: 0;"
        "  padding: 0;"
        "  background: transparent;"
        "}"
        "QToolButton:checked {"
        "  background: rgba(255, 255, 255, 36);"
        "  border-radius: 4px;"
        "}"
        "QToolButton:hover {"
        "  background: rgba(255, 255, 255, 18);"
        "  border-radius: 4px;"
        "}"
    );

    auto* layout = new QHBoxLayout(toolbar_);
    layout->setContentsMargins(8, 6, 8, 6);
    layout->setSpacing(3);

    auto makeAction = [this](const QIcon& icon,
                             bool checkable,
                             auto handler) {
        auto* action = new QAction(icon, {}, this);
        action->setCheckable(checkable);
        connect(action, &QAction::triggered, this, [handler]() { handler(); });
        return action;
    };

    rectangleAction_ = makeAction(
        drawSymbolicIcon("rectangle"),
        true,
        [this]() { setActiveTool(activeTool_ == Tool::Rectangle ? Tool::None : Tool::Rectangle); }
    );
    ellipseAction_ = makeAction(
        drawSymbolicIcon("ellipse"),
        true,
        [this]() { setActiveTool(activeTool_ == Tool::Ellipse ? Tool::None : Tool::Ellipse); }
    );
    arrowAction_ = makeAction(
        drawSymbolicIcon("arrow"),
        true,
        [this]() { setActiveTool(activeTool_ == Tool::Arrow ? Tool::None : Tool::Arrow); }
    );
    pencilAction_ = makeAction(
        drawSymbolicIcon("pen"),
        true,
        [this]() { setActiveTool(activeTool_ == Tool::Pencil ? Tool::None : Tool::Pencil); }
    );
    markerAction_ = makeAction(
        drawSymbolicIcon("marker"),
        true,
        [this]() { setActiveTool(activeTool_ == Tool::Marker ? Tool::None : Tool::Marker); }
    );
    mosaicAction_ = makeAction(
        drawSymbolicIcon("mosaic"),
        true,
        [this]() { setActiveTool(activeTool_ == Tool::Mosaic ? Tool::None : Tool::Mosaic); }
    );
    textAction_ = makeAction(
        drawSymbolicIcon("text"),
        true,
        [this]() { setActiveTool(activeTool_ == Tool::Text ? Tool::None : Tool::Text); }
    );
    serialAction_ = makeAction(
        drawSymbolicIcon("serial"),
        true,
        [this]() { setActiveTool(activeTool_ == Tool::Serial ? Tool::None : Tool::Serial); }
    );
    undoAction_ = makeAction(
        themedIcon(this, "edit-undo", QStyle::SP_ArrowBack),
        false,
        [this]() { undo(); }
    );
    redoAction_ = makeAction(
        themedIcon(this, "edit-redo", QStyle::SP_ArrowForward),
        false,
        [this]() { redo(); }
    );
    copyAction_ = makeAction(
        drawSymbolicIcon("copy"),
        false,
        [this]() { finalizeSelection(CaptureFinalizeAction::Copy); }
    );
    saveAction_ = makeAction(
        drawSymbolicIcon("save"),
        false,
        [this]() { finalizeSelection(CaptureFinalizeAction::Save); }
    );
    pinAction_ = makeAction(
        drawSymbolicIcon("pin"),
        false,
        [this]() { finalizeSelection(CaptureFinalizeAction::Pin); }
    );
    ocrAction_ = makeAction(
        themedIcon(this, "scanner", QStyle::SP_FileDialogContentsView),
        false,
        [this]() { emit ocrRequested(compositedImage()); }
    );
    closeAction_ = makeAction(
        drawSymbolicIcon("close"),
        false,
        [this]() {
            emit captureCanceled();
            close();
        }
    );

    layout->addWidget(createToolbarButton(toolbar_, rectangleAction_));
    layout->addWidget(createToolbarButton(toolbar_, ellipseAction_));
    layout->addWidget(createToolbarButton(toolbar_, arrowAction_));
    layout->addWidget(createToolbarButton(toolbar_, pencilAction_));
    layout->addWidget(createToolbarButton(toolbar_, markerAction_));
    layout->addWidget(createToolbarButton(toolbar_, mosaicAction_));
    layout->addWidget(createToolbarButton(toolbar_, textAction_));
    layout->addWidget(createToolbarButton(toolbar_, serialAction_));
    layout->addWidget(createSeparator(toolbar_));
    layout->addWidget(createToolbarButton(toolbar_, undoAction_));
    layout->addWidget(createToolbarButton(toolbar_, redoAction_));
    layout->addWidget(createSeparator(toolbar_));
    layout->addWidget(createToolbarButton(toolbar_, copyAction_));
    layout->addWidget(createToolbarButton(toolbar_, saveAction_));
    layout->addWidget(createToolbarButton(toolbar_, pinAction_));
    layout->addWidget(createToolbarButton(toolbar_, ocrAction_));
    layout->addWidget(createToolbarButton(toolbar_, closeAction_));

    textInputPanel_ = new QWidget(this);
    textInputPanel_->hide();
    textInputPanel_->setObjectName("captureOverlayTextInputPanel");
    textInputPanel_->setStyleSheet(
        "#captureOverlayTextInputPanel {"
        "  background: rgba(22, 22, 22, 236);"
        "  border: 1px solid rgba(255, 255, 255, 70);"
        "  border-radius: 8px;"
        "}"
        "#captureOverlayTextInputPanel QToolButton {"
        "  border: 0;"
        "  background: transparent;"
        "  color: rgb(220, 220, 220);"
        "  border-radius: 4px;"
        "}"
        "#captureOverlayTextInputPanel QToolButton:hover {"
        "  background: rgba(255, 255, 255, 18);"
        "}"
    );
    auto* textInputLayout = new QHBoxLayout(textInputPanel_);
    textInputLayout->setContentsMargins(6, 4, 6, 4);
    textInputLayout->setSpacing(4);

    textMoveHandleButton_ = new QToolButton(textInputPanel_);
    textMoveHandleButton_->setAutoRaise(true);
    textMoveHandleButton_->setCursor(Qt::SizeAllCursor);
    textMoveHandleButton_->setIcon(drawSymbolicIcon("grip"));
    textMoveHandleButton_->setIconSize(QSize(14, 14));
    textMoveHandleButton_->setFixedSize(22, 22);
    textMoveHandleButton_->installEventFilter(this);
    textInputLayout->addWidget(textMoveHandleButton_);

    textInput_ = new QLineEdit(textInputPanel_);
    textInput_->setMaxLength(200);
    textInput_->setFrame(false);
    textInput_->setClearButtonEnabled(false);
    textInput_->setAttribute(Qt::WA_DeleteOnClose, false);
    textInput_->setStyleSheet(
        "QLineEdit {"
        "  background: transparent;"
        "  color: rgb(255, 236, 120);"
        "  border: 0;"
        "  padding: 0 4px;"
        "  selection-background-color: rgba(255, 236, 120, 72);"
        "}"
    );
    textInput_->installEventFilter(this);
    textInputLayout->addWidget(textInput_, 1);

    textScaleDownButton_ = new QToolButton(textInputPanel_);
    textScaleDownButton_->setAutoRaise(true);
    textScaleDownButton_->setCursor(Qt::SizeFDiagCursor);
    textScaleDownButton_->setIcon(drawSymbolicIcon("resize"));
    textScaleDownButton_->setIconSize(QSize(14, 14));
    textScaleDownButton_->setFixedSize(22, 22);
    textScaleDownButton_->installEventFilter(this);
    textInputLayout->addWidget(textScaleDownButton_);

    textInputPanel_->installEventFilter(this);
    connect(textInput_, &QLineEdit::returnPressed, this, [this]() { commitTextEntry(); });

    refreshActionTooltips();
    toolbar_->hide();
}

void CaptureOverlayWidget::positionToolbar() {
    if (toolbar_ == nullptr) {
        return;
    }

    if (!hasSelection()) {
        toolbar_->hide();
        return;
    }

    const QSize size = toolbar_->sizeHint();
    QRect selection = localSelectionRect();
    int x = selection.center().x() - size.width() / 2;
    x = std::clamp(x, 12, width() - size.width() - 12);

    int y = selection.bottom() + kToolbarGap;
    if (y + size.height() > height() - 12) {
        y = selection.top() - size.height() - kToolbarGap;
    }
    y = std::clamp(y, 12, height() - size.height() - 12);

    toolbar_->setGeometry(x, y, size.width(), size.height());
    toolbar_->show();
    toolbar_->raise();
}

void CaptureOverlayWidget::refreshActionTooltips() {
    const auto& text = cappy::localization::strings(language_);
    const auto setTooltip = [](QAction* action, const QString& tooltip) {
        if (action == nullptr) {
            return;
        }
        action->setToolTip(tooltip);
        action->setStatusTip(tooltip);
    };

    setTooltip(rectangleAction_, QString("%1 (%2)").arg(
        text.toolRectangle,
        joinShortcutLabels({shortcuts_.rectangle, shortcuts_.rectangleAlt})
    ));
    setTooltip(ellipseAction_, QString("%1 (%2)").arg(
        text.toolEllipse,
        joinShortcutLabels({shortcuts_.ellipse, shortcuts_.ellipseAlt})
    ));
    setTooltip(arrowAction_, QString("%1 (%2)").arg(
        text.toolArrow,
        joinShortcutLabels({shortcuts_.arrow, shortcuts_.arrowAlt})
    ));
    setTooltip(pencilAction_, QString("%1 (%2)").arg(
        text.toolPen,
        joinShortcutLabels({shortcuts_.pen, shortcuts_.penAlt})
    ));
    setTooltip(markerAction_, QString("%1 (%2)").arg(
        text.toolMarker,
        joinShortcutLabels({shortcuts_.marker, shortcuts_.markerAlt})
    ));
    setTooltip(mosaicAction_, QString("%1 (%2)").arg(
        text.toolMosaic,
        joinShortcutLabels({shortcuts_.mosaic, shortcuts_.mosaicAlt})
    ));
    setTooltip(textAction_, QString("%1 (%2)").arg(
        text.toolText,
        joinShortcutLabels({shortcuts_.text, shortcuts_.textAlt})
    ));
    setTooltip(serialAction_, QString("%1 (%2)").arg(
        text.toolSerial,
        joinShortcutLabels({shortcuts_.serial, shortcuts_.serialAlt})
    ));
    setTooltip(undoAction_, QString("%1 (%2)").arg(
        text.toolUndo,
        joinShortcutLabels({shortcuts_.undo})
    ));
    setTooltip(redoAction_, QString("%1 (%2)").arg(
        text.toolRedo,
        joinShortcutLabels({shortcuts_.redo})
    ));
    setTooltip(copyAction_, QString("%1 (%2)").arg(
        text.actionCopyToClipboard,
        joinShortcutLabels({shortcuts_.copy, shortcuts_.copyAlt, shortcuts_.quickCopy})
    ));
    setTooltip(saveAction_, QString("%1 (%2)").arg(
        text.actionSaveToFile,
        joinShortcutLabels({shortcuts_.save, shortcuts_.saveAlt})
    ));
    setTooltip(pinAction_, QString("%1 (%2)").arg(
        text.actionPinToDesktop,
        joinShortcutLabels({shortcuts_.pin, shortcuts_.pinAlt})
    ));
    setTooltip(ocrAction_, text.actionExtractText);
    setTooltip(closeAction_, QString("%1 (%2)").arg(
        text.actionCancel,
        joinShortcutLabels({shortcuts_.cancel})
    ));
}

void CaptureOverlayWidget::updateToolActionState() {
    if (rectangleAction_ != nullptr) {
        rectangleAction_->setChecked(activeTool_ == Tool::Rectangle);
    }
    if (ellipseAction_ != nullptr) {
        ellipseAction_->setChecked(activeTool_ == Tool::Ellipse);
    }
    if (arrowAction_ != nullptr) {
        arrowAction_->setChecked(activeTool_ == Tool::Arrow);
    }
    if (pencilAction_ != nullptr) {
        pencilAction_->setChecked(activeTool_ == Tool::Pencil);
    }
    if (markerAction_ != nullptr) {
        markerAction_->setChecked(activeTool_ == Tool::Marker);
    }
    if (mosaicAction_ != nullptr) {
        mosaicAction_->setChecked(activeTool_ == Tool::Mosaic);
    }
    if (textAction_ != nullptr) {
        textAction_->setChecked(activeTool_ == Tool::Text);
    }
    if (serialAction_ != nullptr) {
        serialAction_->setChecked(activeTool_ == Tool::Serial);
    }

    updateInteractionCursor(hasHoverPoint_ ? hoverPoint_ : clampToBounds(mapFromGlobal(QCursor::pos())));
}

void CaptureOverlayWidget::updateUndoRedoState() {
    if (undoAction_ != nullptr) {
        undoAction_->setEnabled(!undoStack_.isEmpty());
    }
    if (redoAction_ != nullptr) {
        redoAction_->setEnabled(!redoStack_.isEmpty());
    }
}

void CaptureOverlayWidget::updateInteractionCursor(const QPoint& point) {
    if (dragging_) {
        switch (selectionMode_) {
        case SelectionMode::Move:
            setCursor(Qt::SizeAllCursor);
            return;
        case SelectionMode::ResizeLeft:
        case SelectionMode::ResizeRight:
            setCursor(Qt::SizeHorCursor);
            return;
        case SelectionMode::ResizeTop:
        case SelectionMode::ResizeBottom:
            setCursor(Qt::SizeVerCursor);
            return;
        case SelectionMode::ResizeTopLeft:
        case SelectionMode::ResizeBottomRight:
            setCursor(Qt::SizeFDiagCursor);
            return;
        case SelectionMode::ResizeTopRight:
        case SelectionMode::ResizeBottomLeft:
            setCursor(Qt::SizeBDiagCursor);
            return;
        case SelectionMode::Create:
            setCursor(Qt::CrossCursor);
            return;
        case SelectionMode::None:
            break;
        }
    }

    if (activeTool_ != Tool::None) {
        setCursor((activeTool_ == Tool::Text || activeTool_ == Tool::Serial) ? Qt::IBeamCursor : Qt::CrossCursor);
        return;
    }

    if (!hasSelection()) {
        setCursor(trackedWindowSelectionRect_.isEmpty() ? Qt::CrossCursor : Qt::PointingHandCursor);
        return;
    }

    if (selectionCanBeAdjusted()) {
        switch (hitTestSelection(point)) {
        case SelectionMode::Move:
            setCursor(Qt::SizeAllCursor);
            return;
        case SelectionMode::ResizeLeft:
        case SelectionMode::ResizeRight:
            setCursor(Qt::SizeHorCursor);
            return;
        case SelectionMode::ResizeTop:
        case SelectionMode::ResizeBottom:
            setCursor(Qt::SizeVerCursor);
            return;
        case SelectionMode::ResizeTopLeft:
        case SelectionMode::ResizeBottomRight:
            setCursor(Qt::SizeFDiagCursor);
            return;
        case SelectionMode::ResizeTopRight:
        case SelectionMode::ResizeBottomLeft:
            setCursor(Qt::SizeBDiagCursor);
            return;
        case SelectionMode::Create:
        case SelectionMode::None:
            break;
        }
    }

    setCursor(localSelectionRect().contains(point) ? Qt::OpenHandCursor : Qt::ArrowCursor);
}

void CaptureOverlayWidget::setActiveTool(Tool tool) {
    if (textEntryActive_ && tool != Tool::Text) {
        cancelTextEntry();
    }
    activeTool_ = tool;
    dragging_ = false;
    trackingClickPending_ = false;
    selectionMode_ = SelectionMode::None;
    updateToolActionState();
    update();
}

void CaptureOverlayWidget::applyInitialSelection(const QRect& absoluteRect) {
    if (absoluteRect.isEmpty()) {
        return;
    }

    QRect localRect = absoluteRect.translated(-desktopFrame_.geometry.topLeft());
    localRect = clampSelectionRect(localRect.normalized());
    if (localRect.width() < kMinimumSelectionSize || localRect.height() < kMinimumSelectionSize) {
        return;
    }

    selectionRect_ = localRect;
    refreshWorkingImageFromSelection();
    positionToolbar();
    update();
}

void CaptureOverlayWidget::updateTrackedWindowSelection(const QPoint& overlayPoint) {
    if (windowGeometryResolver_ == nullptr || hasSelection()) {
        return;
    }

    QRect localRect;
    const QRect absoluteRect = windowGeometryResolver_(
        overlayPoint + desktopFrame_.geometry.topLeft(),
        effectiveWinId()
    );
    if (!absoluteRect.isEmpty()) {
        localRect = clampSelectionRect(
            absoluteRect.translated(-desktopFrame_.geometry.topLeft()).normalized()
        );
        if (localRect.width() < kMinimumSelectionSize || localRect.height() < kMinimumSelectionSize) {
            localRect = {};
        }
    }

    if (trackedWindowSelectionRect_ == localRect) {
        return;
    }

    trackedWindowSelectionRect_ = localRect;
    updateInteractionCursor(overlayPoint);
    update();
}

void CaptureOverlayWidget::refreshWorkingImageFromSelection() {
    if (!hasSelection()) {
        workingImage_ = {};
        return;
    }

    workingImage_ = cappy::domain::capture::cropNormalizedImage(
        desktopFrame_,
        selectionRect_.translated(desktopFrame_.geometry.topLeft())
    );
    if (workingImage_.isNull()) {
        workingImage_ = desktopFrame_.image.copy(selectionRect_);
    }
    nextSerialNumber_ = 1;
    textAnnotations_.clear();
    selectedTextAnnotationIndex_ = -1;
    undoStack_.clear();
    redoStack_.clear();
    hasEdits_ = false;
    updateUndoRedoState();
}

void CaptureOverlayWidget::finalizeSelection(CaptureFinalizeAction action) {
    if (!hasSelection() || workingImage_.isNull()) {
        return;
    }

    cappy::domain::capture::CaptureResult result;
    result.mode = captureMode_;
    result.geometry = selectionRect_.translated(desktopFrame_.geometry.topLeft());
    result.backendName = "overlay-edit";
    result.image = compositedImage();
    emit captureFinalized(result, action);
}

void CaptureOverlayWidget::handleShortcut(QKeyEvent* event) {
    if (event == nullptr) {
        return;
    }

    if (suppressNextReturnShortcut_
        && (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)) {
        suppressNextReturnShortcut_ = false;
        event->accept();
        return;
    }

    if (textEntryActive_) {
        if (event->key() == Qt::Key_Escape) {
            cancelTextEntry();
            event->accept();
        }
        return;
    }

    const auto toggleToolIfMatched =
        [this, event](const QString& primary, const QString& alternate, Tool tool) -> bool {
            if (!matchesShortcut(event, primary) && !matchesShortcut(event, alternate)) {
                return false;
            }

            setActiveTool(activeTool_ == tool ? Tool::None : tool);
            event->accept();
            return true;
        };

    if (toggleToolIfMatched(shortcuts_.rectangle, shortcuts_.rectangleAlt, Tool::Rectangle)
        || toggleToolIfMatched(shortcuts_.ellipse, shortcuts_.ellipseAlt, Tool::Ellipse)
        || toggleToolIfMatched(shortcuts_.arrow, shortcuts_.arrowAlt, Tool::Arrow)
        || toggleToolIfMatched(shortcuts_.pen, shortcuts_.penAlt, Tool::Pencil)
        || toggleToolIfMatched(shortcuts_.marker, shortcuts_.markerAlt, Tool::Marker)
        || toggleToolIfMatched(shortcuts_.mosaic, shortcuts_.mosaicAlt, Tool::Mosaic)
        || toggleToolIfMatched(shortcuts_.text, shortcuts_.textAlt, Tool::Text)
        || toggleToolIfMatched(shortcuts_.serial, shortcuts_.serialAlt, Tool::Serial)) {
        return;
    }

    if (matchesShortcut(event, shortcuts_.undo)) {
        undo();
        event->accept();
        return;
    }

    if (matchesShortcut(event, shortcuts_.redo)) {
        redo();
        event->accept();
        return;
    }

    if (matchesShortcut(event, shortcuts_.copy)
        || matchesShortcut(event, shortcuts_.copyAlt)
        || matchesShortcut(event, shortcuts_.quickCopy)) {
        finalizeSelection(CaptureFinalizeAction::Copy);
        event->accept();
        return;
    }

    if (matchesShortcut(event, shortcuts_.save) || matchesShortcut(event, shortcuts_.saveAlt)) {
        finalizeSelection(CaptureFinalizeAction::Save);
        event->accept();
        return;
    }

    if (matchesShortcut(event, shortcuts_.pin) || matchesShortcut(event, shortcuts_.pinAlt)) {
        finalizeSelection(CaptureFinalizeAction::Pin);
        event->accept();
        return;
    }

    if (matchesShortcut(event, shortcuts_.cancel)) {
        emit captureCanceled();
        close();
        event->accept();
    }
}

void CaptureOverlayWidget::beginSelectionGesture(const QPoint& point) {
    selectionMode_ = SelectionMode::Create;
    dragging_ = true;
    trackingClickPending_ = false;
    dragStart_ = point;
    dragCurrent_ = point;
    selectionRect_ = {};
    trackedWindowSelectionRect_ = {};
    toolbar_->hide();
}

void CaptureOverlayWidget::beginMoveOrResize(const QPoint& point, SelectionMode mode) {
    selectionMode_ = mode;
    dragging_ = true;
    selectionDragAnchor_ = point;
    selectionRectAtDragStart_ = selectionRect_;
}

void CaptureOverlayWidget::updateSelectionGeometry(const QPoint& point) {
    if (selectionMode_ == SelectionMode::Create) {
        dragCurrent_ = point;
        selectionRect_ = clampSelectionRect(normalizedSelectionRect());
        update();
        return;
    }

    QRect rect = selectionRectAtDragStart_;
    const QPoint delta = point - selectionDragAnchor_;

    switch (selectionMode_) {
    case SelectionMode::Move:
        rect.translate(delta);
        break;
    case SelectionMode::ResizeLeft:
        rect.setLeft(rect.left() + delta.x());
        break;
    case SelectionMode::ResizeRight:
        rect.setRight(rect.right() + delta.x());
        break;
    case SelectionMode::ResizeTop:
        rect.setTop(rect.top() + delta.y());
        break;
    case SelectionMode::ResizeBottom:
        rect.setBottom(rect.bottom() + delta.y());
        break;
    case SelectionMode::ResizeTopLeft:
        rect.setTop(rect.top() + delta.y());
        rect.setLeft(rect.left() + delta.x());
        break;
    case SelectionMode::ResizeTopRight:
        rect.setTop(rect.top() + delta.y());
        rect.setRight(rect.right() + delta.x());
        break;
    case SelectionMode::ResizeBottomLeft:
        rect.setBottom(rect.bottom() + delta.y());
        rect.setLeft(rect.left() + delta.x());
        break;
    case SelectionMode::ResizeBottomRight:
        rect.setBottom(rect.bottom() + delta.y());
        rect.setRight(rect.right() + delta.x());
        break;
    case SelectionMode::Create:
    case SelectionMode::None:
        break;
    }

    rect = rect.normalized();
    rect = clampSelectionRect(rect);
    if (rect.width() < kMinimumSelectionSize || rect.height() < kMinimumSelectionSize) {
        return;
    }
    selectionRect_ = rect;
    refreshWorkingImageFromSelection();
    positionToolbar();
    update();
}

void CaptureOverlayWidget::handlePointTool(const QPoint& point) {
    const QPoint imagePoint = mapOverlayPointToImage(point);
    switch (activeTool_) {
    case Tool::Text: {
        bool resizeHandle = false;
        const int annotationIndex = hitTestTextAnnotation(imagePoint, &resizeHandle);
        if (annotationIndex >= 0) {
            pushUndoSnapshot();
            selectedTextAnnotationIndex_ = annotationIndex;
            if (resizeHandle) {
                textAnnotationResizing_ = true;
            } else {
                textAnnotationDragging_ = true;
                textAnnotationDragOffset_ = imagePoint - textAnnotations_[annotationIndex].rect.topLeft();
            }
            update();
            break;
        }
        selectedTextAnnotationIndex_ = -1;
        beginTextEntry(imagePoint, point);
        break;
    }
    case Tool::Serial:
        drawSerialMarker(imagePoint);
        break;
    default:
        break;
    }
}

void CaptureOverlayWidget::commitCurrentShape() {
    if (!dragging_ || workingImage_.isNull()) {
        return;
    }

    dragging_ = false;
    const QPoint imageStart = mapOverlayPointToImage(dragStart_);
    const QPoint imageEnd = mapOverlayPointToImage(dragCurrent_);

    switch (activeTool_) {
    case Tool::Rectangle: {
        QPainter painter(&workingImage_);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setPen(QPen(
            activeStrokeColor(),
            activeStrokeWidth(),
            Qt::SolidLine,
            Qt::RoundCap,
            Qt::RoundJoin
        ));
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(QRect(imageStart, imageEnd).normalized());
        break;
    }
    case Tool::Ellipse: {
        QPainter painter(&workingImage_);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setPen(QPen(
            activeStrokeColor(),
            activeStrokeWidth(),
            Qt::SolidLine,
            Qt::RoundCap,
            Qt::RoundJoin
        ));
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(QRect(imageStart, imageEnd).normalized());
        break;
    }
    case Tool::Arrow: {
        QPainter painter(&workingImage_);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setPen(QPen(
            activeStrokeColor(),
            activeStrokeWidth(),
            Qt::SolidLine,
            Qt::RoundCap,
            Qt::RoundJoin
        ));
        painter.setBrush(Qt::NoBrush);
        drawArrow(painter, imageStart, imageEnd);
        break;
    }
    case Tool::Mosaic:
        applyMosaic(QRect(imageStart, imageEnd).normalized());
        break;
    case Tool::Pencil:
    case Tool::Marker:
    case Tool::Text:
    case Tool::Serial:
    case Tool::None:
        break;
    }

    hasEdits_ = true;
    updateUndoRedoState();
    update();
}

void CaptureOverlayWidget::drawShapePreview(QPainter& painter) const {
    painter.save();
    painter.setPen(QPen(
        activeStrokeColor(),
        activeStrokeWidth(),
        Qt::SolidLine,
        Qt::RoundCap,
        Qt::RoundJoin
    ));
    painter.setBrush(Qt::NoBrush);

    switch (activeTool_) {
    case Tool::Rectangle:
        painter.drawRect(normalizedSelectionRect());
        break;
    case Tool::Ellipse:
        painter.drawEllipse(normalizedSelectionRect());
        break;
    case Tool::Arrow:
        drawArrow(painter, dragStart_, dragCurrent_);
        break;
    case Tool::Mosaic:
        painter.fillRect(normalizedSelectionRect(), QColor(255, 255, 255, 40));
        painter.drawRect(normalizedSelectionRect());
        break;
    case Tool::Pencil:
    case Tool::Marker:
    case Tool::Text:
    case Tool::Serial:
    case Tool::None:
        break;
    }

    painter.restore();
}

void CaptureOverlayWidget::drawArrow(QPainter& painter, const QPoint& start, const QPoint& end) const {
    painter.drawLine(start, end);

    const QLineF line(start, end);
    if (qFuzzyIsNull(line.length())) {
        return;
    }

    constexpr double arrowSize = 14.0;
    const double angle = std::atan2(-line.dy(), line.dx());
    const QPointF arrowP1 = end + QPointF(
        std::sin(angle - kPi / 3.0) * arrowSize,
        std::cos(angle - kPi / 3.0) * arrowSize
    );
    const QPointF arrowP2 = end + QPointF(
        std::sin(angle - kPi + kPi / 3.0) * arrowSize,
        std::cos(angle - kPi + kPi / 3.0) * arrowSize
    );

    painter.drawLine(end, arrowP1.toPoint());
    painter.drawLine(end, arrowP2.toPoint());
}

void CaptureOverlayWidget::applyMosaic(const QRect& imageRect) {
    const QRect boundedRect = imageRect.intersected(workingImage_.rect());
    if (boundedRect.isEmpty()) {
        return;
    }

    const QImage source = workingImage_.copy(boundedRect);
    QPainter painter(&workingImage_);

    constexpr int blockSize = 12;
    for (int y = 0; y < boundedRect.height(); y += blockSize) {
        for (int x = 0; x < boundedRect.width(); x += blockSize) {
            const QRect blockRect(
                boundedRect.x() + x,
                boundedRect.y() + y,
                std::min(blockSize, boundedRect.width() - x),
                std::min(blockSize, boundedRect.height() - y)
            );
            const QRect localRect(x, y, blockRect.width(), blockRect.height());
            const QPoint samplePoint(
                std::min(localRect.x() + localRect.width() / 2, source.width() - 1),
                std::min(localRect.y() + localRect.height() / 2, source.height() - 1)
            );
            painter.fillRect(blockRect, source.pixelColor(samplePoint));
        }
    }
}

void CaptureOverlayWidget::beginTextEntry(const QPoint& imagePoint, const QPoint& overlayPoint) {
    if (textInput_ == nullptr || textInputPanel_ == nullptr || !hasSelection()) {
        return;
    }

    Q_UNUSED(imagePoint);
    textEntryActive_ = true;
    textInputDragging_ = false;
    textInputResizing_ = false;
    if (QWidget::keyboardGrabber() == this) {
        releaseKeyboard();
    }

    const QRect bounds = localSelectionRect().adjusted(8, 8, -8, -8);
    if (bounds.width() < 80 || bounds.height() < 24) {
        textEntryActive_ = false;
        if (isVisible()) {
            grabKeyboard();
        }
        return;
    }

    pendingTextPixelSize_ = std::clamp(workingImage_.height() / 18, 16, 40);
    textInputPanel_->resize(
        std::clamp(localSelectionRect().width() / 2, 220, 360),
        std::clamp(pendingTextPixelSize_ + 18, 36, 60)
    );
    updateTextInputAppearance();
    QPoint topLeft = clampTextInputPanelTopLeft(overlayPoint + QPoint(10, 10));

    textInput_->setPlaceholderText(cappy::localization::strings(language_).dialogAddTextLabel);
    textInput_->clear();
    textInputPanel_->move(topLeft);
    textInputPanel_->show();
    textInputPanel_->raise();
    updatePendingTextPointFromPanel();
    textInput_->setFocus(Qt::MouseFocusReason);
    textInput_->selectAll();
}

void CaptureOverlayWidget::commitTextEntry() {
    if (!textEntryActive_ || textInput_ == nullptr || textInputPanel_ == nullptr) {
        return;
    }

    const QString labelText = textInput_->text().trimmed();
    textEntryActive_ = false;
    textInputDragging_ = false;
    textInputResizing_ = false;
    textInputPanel_->hide();
    textInput_->clearFocus();
    suppressNextReturnShortcut_ = true;
    if (isVisible()) {
        grabKeyboard();
        setFocus(Qt::OtherFocusReason);
    }

    if (labelText.isEmpty()) {
        return;
    }

    pushUndoSnapshot();
    Snapshot::TextAnnotation annotation;
    annotation.text = labelText;
    annotation.pixelSize = pendingTextPixelSize_;
    annotation.rect = normalizedTextRect(labelText, pendingTextPixelSize_, pendingTextPoint_);
    textAnnotations_.push_back(annotation);
    selectedTextAnnotationIndex_ = textAnnotations_.size() - 1;
    hasEdits_ = true;
    updateUndoRedoState();
    update();
    activeTool_ = Tool::None;
    updateToolActionState();
}

void CaptureOverlayWidget::cancelTextEntry() {
    if (textInput_ == nullptr || textInputPanel_ == nullptr) {
        return;
    }

    textEntryActive_ = false;
    textInputDragging_ = false;
    textInputResizing_ = false;
    textInputPanel_->hide();
    textInput_->clear();
    textInput_->clearFocus();
    if (isVisible()) {
        grabKeyboard();
        setFocus(Qt::OtherFocusReason);
    }
}

void CaptureOverlayWidget::updateTextInputAppearance() {
    if (textInputPanel_ == nullptr || textInput_ == nullptr) {
        return;
    }

    QFont font = textInput_->font();
    font.setBold(true);
    font.setPixelSize(pendingTextPixelSize_);
    textInput_->setFont(font);

    const int buttonSize = std::clamp(textInputPanel_->height() - 10, 22, 26);
    if (textMoveHandleButton_ != nullptr) {
        textMoveHandleButton_->setFixedSize(buttonSize, buttonSize);
    }
    if (textScaleDownButton_ != nullptr) {
        textScaleDownButton_->setFixedSize(buttonSize, buttonSize);
    }

    if (textInputPanel_->isVisible()) {
        textInputPanel_->move(clampTextInputPanelTopLeft(textInputPanel_->pos()));
        updatePendingTextPointFromPanel();
    }
}

void CaptureOverlayWidget::adjustTextInputScale(int delta) {
    Q_UNUSED(delta);
}

void CaptureOverlayWidget::updatePendingTextPointFromPanel() {
    if (textInputPanel_ == nullptr) {
        return;
    }

    pendingTextPoint_ = mapOverlayPointToImage(textInputPanel_->geometry().topLeft() + QPoint(10, 8));
}

QPoint CaptureOverlayWidget::clampTextInputPanelTopLeft(const QPoint& topLeft) const {
    if (textInputPanel_ == nullptr) {
        return topLeft;
    }

    const QRect bounds = localSelectionRect().adjusted(8, 8, -8, -8);
    return QPoint(
        std::clamp(topLeft.x(), bounds.left(), bounds.right() - textInputPanel_->width()),
        std::clamp(topLeft.y(), bounds.top(), bounds.bottom() - textInputPanel_->height())
    );
}

QImage CaptureOverlayWidget::compositedImage() const {
    if (workingImage_.isNull()) {
        return {};
    }

    QImage result = workingImage_.copy();
    QPainter painter(&result);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    for (const auto& annotation : textAnnotations_) {
        QFont font = painter.font();
        font.setBold(true);
        font.setPixelSize(annotation.pixelSize);
        painter.setFont(font);
        painter.setPen(QColor(0, 0, 0, 170));
        painter.drawText(annotation.rect.translated(1, 1), Qt::AlignLeft | Qt::AlignVCenter, annotation.text);
        painter.setPen(QColor(255, 236, 120));
        painter.drawText(annotation.rect, Qt::AlignLeft | Qt::AlignVCenter, annotation.text);
    }
    return result;
}

void CaptureOverlayWidget::drawTextAnnotations(QPainter& painter) const {
    for (int i = 0; i < textAnnotations_.size(); ++i) {
        const auto& annotation = textAnnotations_[i];
        const QRect widgetRect = annotation.rect.translated(localSelectionRect().topLeft());
        QFont font = painter.font();
        font.setBold(true);
        font.setPixelSize(annotation.pixelSize);
        painter.setFont(font);
        painter.setPen(QColor(0, 0, 0, 170));
        painter.drawText(widgetRect.translated(1, 1), Qt::AlignLeft | Qt::AlignVCenter, annotation.text);
        painter.setPen(QColor(255, 236, 120));
        painter.drawText(widgetRect, Qt::AlignLeft | Qt::AlignVCenter, annotation.text);

        if (i == selectedTextAnnotationIndex_) {
            drawTextAnnotationSelection(painter, annotation);
        }
    }
}

void CaptureOverlayWidget::drawTextAnnotationSelection(
    QPainter& painter,
    const Snapshot::TextAnnotation& annotation
) const {
    const QRect widgetRect = annotation.rect.translated(localSelectionRect().topLeft());
    painter.setPen(QPen(QColor(255, 255, 255, 180), 1, Qt::DashLine));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(widgetRect.adjusted(-4, -4, 4, 4));
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 255, 255, 220));
    painter.drawEllipse(widgetRect.bottomRight() + QPoint(4, 4), 4, 4);
}

int CaptureOverlayWidget::hitTestTextAnnotation(const QPoint& imagePoint, bool* resizeHandle) const {
    if (resizeHandle != nullptr) {
        *resizeHandle = false;
    }

    for (int i = textAnnotations_.size() - 1; i >= 0; --i) {
        const QRect rect = textAnnotations_[i].rect.adjusted(-4, -4, 4, 4);
        const QRect handleRect(rect.bottomRight() - QPoint(10, 10), QSize(20, 20));
        if (handleRect.contains(imagePoint)) {
            if (resizeHandle != nullptr) {
                *resizeHandle = true;
            }
            return i;
        }
        if (rect.contains(imagePoint)) {
            return i;
        }
    }
    return -1;
}

QRect CaptureOverlayWidget::normalizedTextRect(
    const QString& text,
    int pixelSize,
    const QPoint& topLeft
) const {
    QFont font = this->font();
    font.setBold(true);
    font.setPixelSize(pixelSize);
    const QFontMetrics metrics(font);
    QRect rect = metrics.boundingRect(text);
    rect.moveTopLeft(topLeft);
    if (rect.left() < 0) {
        rect.moveLeft(8);
    }
    if (rect.top() < 0) {
        rect.moveTop(8);
    }
    if (!workingImage_.isNull()) {
        if (rect.right() > workingImage_.width() - 8) {
            rect.moveRight(workingImage_.width() - 8);
        }
        if (rect.bottom() > workingImage_.height() - 8) {
            rect.moveBottom(workingImage_.height() - 8);
        }
    }
    return rect;
}

void CaptureOverlayWidget::drawSerialMarker(const QPoint& imagePoint) {
    pushUndoSnapshot();

    const int diameter = std::clamp(workingImage_.height() / 14, 26, 42);
    QRect markerRect(
        imagePoint.x() - diameter / 2,
        imagePoint.y() - diameter / 2,
        diameter,
        diameter
    );
    markerRect = markerRect.intersected(workingImage_.rect().adjusted(0, 0, -1, -1));

    QPainter painter(&workingImage_);
    painter.setRenderHint(QPainter::Antialiasing, true);
    drawSerialBadge(
        painter,
        markerRect,
        QString::number(nextSerialNumber_),
        QColor(235, 68, 90, 236),
        QColor(255, 255, 255, 240)
    );
    ++nextSerialNumber_;

    hasEdits_ = true;
    updateUndoRedoState();
    update();
}

void CaptureOverlayWidget::drawSerialPreview(QPainter& painter) const {
    const int diameter = std::clamp(workingImage_.height() / 14, 26, 42);
    QRect previewRect(
        hoverPoint_.x() - diameter / 2,
        hoverPoint_.y() - diameter / 2,
        diameter,
        diameter
    );
    previewRect = previewRect.intersected(localSelectionRect().adjusted(0, 0, -1, -1));
    if (previewRect.isEmpty()) {
        return;
    }

    drawSerialBadge(
        painter,
        previewRect,
        QString::number(nextSerialNumber_),
        QColor(235, 68, 90, 140),
        QColor(255, 255, 255, 180)
    );
}

void CaptureOverlayWidget::drawSerialBadge(
    QPainter& painter,
    const QRect& rect,
    const QString& text,
    const QColor& fillColor,
    const QColor& outlineColor
) const {
    if (rect.isEmpty()) {
        return;
    }

    const QRect shadowRect = rect.translated(1, 2);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 90));
    painter.drawEllipse(shadowRect);

    painter.setPen(QPen(outlineColor, 2));
    painter.setBrush(fillColor);
    painter.drawEllipse(rect);

    QFont font = painter.font();
    font.setBold(true);
    font.setPointSize(std::clamp(rect.height() / 2, 12, 20));
    painter.setFont(font);
    painter.setPen(Qt::white);
    painter.drawText(rect, Qt::AlignCenter, text);
}

void CaptureOverlayWidget::pushUndoSnapshot() {
    if (workingImage_.isNull()) {
        return;
    }

    undoStack_.push_back(Snapshot{
        .image = workingImage_,
        .textAnnotations = textAnnotations_,
        .nextSerialNumber = nextSerialNumber_,
    });
    redoStack_.clear();
    updateUndoRedoState();
}

void CaptureOverlayWidget::undo() {
    if (undoStack_.isEmpty()) {
        return;
    }

    redoStack_.push_back(Snapshot{
        .image = workingImage_,
        .textAnnotations = textAnnotations_,
        .nextSerialNumber = nextSerialNumber_,
    });
    const Snapshot snapshot = undoStack_.takeLast();
    workingImage_ = snapshot.image;
    textAnnotations_ = snapshot.textAnnotations;
    nextSerialNumber_ = snapshot.nextSerialNumber;
    selectedTextAnnotationIndex_ = -1;
    hasEdits_ = !undoStack_.isEmpty();
    updateUndoRedoState();
    update();
}

void CaptureOverlayWidget::redo() {
    if (redoStack_.isEmpty()) {
        return;
    }

    undoStack_.push_back(Snapshot{
        .image = workingImage_,
        .textAnnotations = textAnnotations_,
        .nextSerialNumber = nextSerialNumber_,
    });
    const Snapshot snapshot = redoStack_.takeLast();
    workingImage_ = snapshot.image;
    textAnnotations_ = snapshot.textAnnotations;
    nextSerialNumber_ = snapshot.nextSerialNumber;
    selectedTextAnnotationIndex_ = -1;
    hasEdits_ = true;
    updateUndoRedoState();
    update();
}

bool CaptureOverlayWidget::hasSelection() const {
    return !selectionRect_.isEmpty();
}

bool CaptureOverlayWidget::isPointTool() const {
    return activeTool_ == Tool::Text || activeTool_ == Tool::Serial;
}

bool CaptureOverlayWidget::selectionCanBeAdjusted() const {
    return hasSelection() && !hasEdits_;
}

bool CaptureOverlayWidget::isWithinSelection(const QPoint& point) const {
    return localSelectionRect().contains(point);
}

QRect CaptureOverlayWidget::normalizedSelectionRect() const {
    return QRect(dragStart_, dragCurrent_).normalized();
}

QRect CaptureOverlayWidget::localSelectionRect() const {
    return selectionRect_;
}

QRect CaptureOverlayWidget::clampSelectionRect(const QRect& rect) const {
    QRect clamped = rect.normalized();
    clamped.setLeft(std::clamp(clamped.left(), 0, width() - 1));
    clamped.setTop(std::clamp(clamped.top(), 0, height() - 1));
    clamped.setRight(std::clamp(clamped.right(), 0, width() - 1));
    clamped.setBottom(std::clamp(clamped.bottom(), 0, height() - 1));
    return clamped.normalized();
}

QPoint CaptureOverlayWidget::clampToBounds(const QPoint& point) const {
    return {
        std::clamp(point.x(), 0, width() - 1),
        std::clamp(point.y(), 0, height() - 1),
    };
}

QPoint CaptureOverlayWidget::mapOverlayPointToImage(const QPoint& point) const {
    if (!hasSelection() || selectionRect_.width() <= 0 || selectionRect_.height() <= 0) {
        return {};
    }

    const QRect selection = localSelectionRect();
    const int localX = std::clamp(point.x() - selection.left(), 0, std::max(0, selection.width() - 1));
    const int localY = std::clamp(point.y() - selection.top(), 0, std::max(0, selection.height() - 1));

    const int x = std::clamp(
        qRound((static_cast<double>(localX) / std::max(1, selection.width() - 1))
               * (workingImage_.width() - 1)),
        0,
        std::max(0, workingImage_.width() - 1)
    );
    const int y = std::clamp(
        qRound((static_cast<double>(localY) / std::max(1, selection.height() - 1))
               * (workingImage_.height() - 1)),
        0,
        std::max(0, workingImage_.height() - 1)
    );
    return {x, y};
}

CaptureOverlayWidget::SelectionMode CaptureOverlayWidget::hitTestSelection(const QPoint& point) const {
    const QRect rect = localSelectionRect();
    if (!rect.isValid()) {
        return SelectionMode::None;
    }

    const QRect topLeft(rect.topLeft() - QPoint(kHandleHitDistance, kHandleHitDistance), QSize(kHandleHitDistance * 2, kHandleHitDistance * 2));
    const QRect topRight(rect.topRight() - QPoint(kHandleHitDistance, kHandleHitDistance), QSize(kHandleHitDistance * 2, kHandleHitDistance * 2));
    const QRect bottomLeft(rect.bottomLeft() - QPoint(kHandleHitDistance, kHandleHitDistance), QSize(kHandleHitDistance * 2, kHandleHitDistance * 2));
    const QRect bottomRight(rect.bottomRight() - QPoint(kHandleHitDistance, kHandleHitDistance), QSize(kHandleHitDistance * 2, kHandleHitDistance * 2));
    if (topLeft.contains(point)) return SelectionMode::ResizeTopLeft;
    if (topRight.contains(point)) return SelectionMode::ResizeTopRight;
    if (bottomLeft.contains(point)) return SelectionMode::ResizeBottomLeft;
    if (bottomRight.contains(point)) return SelectionMode::ResizeBottomRight;

    if (std::abs(point.x() - rect.left()) <= kHandleHitDistance
        && point.y() >= rect.top() && point.y() <= rect.bottom()) {
        return SelectionMode::ResizeLeft;
    }
    if (std::abs(point.x() - rect.right()) <= kHandleHitDistance
        && point.y() >= rect.top() && point.y() <= rect.bottom()) {
        return SelectionMode::ResizeRight;
    }
    if (std::abs(point.y() - rect.top()) <= kHandleHitDistance
        && point.x() >= rect.left() && point.x() <= rect.right()) {
        return SelectionMode::ResizeTop;
    }
    if (std::abs(point.y() - rect.bottom()) <= kHandleHitDistance
        && point.x() >= rect.left() && point.x() <= rect.right()) {
        return SelectionMode::ResizeBottom;
    }
    if (rect.contains(point)) {
        return SelectionMode::Move;
    }

    return SelectionMode::None;
}

QColor CaptureOverlayWidget::activeStrokeColor() const {
    switch (activeTool_) {
    case Tool::Marker:
        return QColor(255, 235, 59, 140);
    case Tool::Text:
        return QColor(255, 236, 120);
    default:
        return strokeColor_;
    }
}

int CaptureOverlayWidget::activeStrokeWidth() const {
    switch (activeTool_) {
    case Tool::Marker:
        return 12;
    default:
        return strokeWidth_;
    }
}

}  // namespace cappy::features::capture
