#include "cappy/features/editor/CaptureEditorWindow.h"

#include <algorithm>
#include <cmath>

#include <QAction>
#include <QBoxLayout>
#include <QCursor>
#include <QEvent>
#include <QFrame>
#include <QGuiApplication>
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

namespace cappy::features::editor {

namespace {

constexpr double kPi = 3.14159265358979323846;
constexpr int kOuterPadding = 10;
constexpr int kToolbarGap = 10;
constexpr QSize kButtonSize(28, 28);
constexpr QSize kIconSize(16, 16);

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
    const qreal w = size.width();
    const qreal h = size.height();

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
    } else if (id == "close") {
        painter.drawLine(QPointF(4, 4), QPointF(size.width() - 4, size.height() - 4));
        painter.drawLine(QPointF(size.width() - 4, 4), QPointF(4, size.height() - 4));
    } else if (id == "more") {
        painter.setBrush(QColor(245, 245, 245));
        painter.setPen(Qt::NoPen);
        for (int i = 0; i < 3; ++i) {
            painter.drawEllipse(QPointF(size.width() / 2.0, 4.0 + i * 4.0), 1.3, 1.3);
        }
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
        painter.drawLine(QPointF(5.0, h - 4.0), QPointF(w - 4.0, 5.0));
        painter.drawLine(QPointF(8.0, h - 4.0), QPointF(w - 4.0, 8.0));
        painter.drawLine(QPointF(11.0, h - 4.0), QPointF(w - 4.0, 11.0));
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
    button->setFixedSize(kButtonSize);
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
    const QKeySequence pressed(static_cast<int>(event->modifiers()) | key);
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

CaptureEditorWindow::CaptureEditorWindow(
    const QImage& image,
    const QRect& globalGeometry,
    cappy::shortcuts::CaptureEditorShortcutSettings shortcuts,
    cappy::localization::AppLanguage language,
    QWidget* parent
)
    : QWidget(parent)
    , workingImage_(image)
    , globalGeometry_(globalGeometry)
    , shortcuts_(std::move(shortcuts))
    , language_(cappy::localization::resolvedAppLanguage(language)) {
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_DeleteOnClose, true);
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    setupUi();

    const QSize imageSize = workingImage_.isNull() ? QSize(360, 220) : workingImage_.size();
    const QSize toolbarSize = toolbar_->sizeHint();
    const int width = std::max(
        imageSize.width() + kOuterPadding * 2,
        toolbarSize.width() + kOuterPadding * 2
    );
    const int height = imageSize.height() + toolbarSize.height() + kOuterPadding * 2 + kToolbarGap;

    resize(width, height);
    imageViewportRect_ = QRect(
        (width - imageSize.width()) / 2,
        kOuterPadding,
        imageSize.width(),
        imageSize.height()
    );
    toolbar_->setGeometry(
        (width - toolbarSize.width()) / 2,
        imageViewportRect_.bottom() + 1 + kToolbarGap,
        toolbarSize.width(),
        toolbarSize.height()
    );

    const QScreen* screen = globalGeometry_.isValid()
        ? QGuiApplication::screenAt(globalGeometry_.center())
        : QGuiApplication::screenAt(QCursor::pos());
    if (screen == nullptr) {
        screen = QGuiApplication::primaryScreen();
    }

    QPoint topLeft;
    if (globalGeometry_.isValid()) {
        topLeft = globalGeometry_.topLeft() - imageViewportRect_.topLeft();
    } else if (screen != nullptr) {
        topLeft = screen->availableGeometry().center() - rect().center();
    }

    if (screen != nullptr) {
        const QRect available = screen->availableGeometry();
        topLeft.setX(std::clamp(topLeft.x(), available.left(), available.right() - width));
        topLeft.setY(std::clamp(topLeft.y(), available.top(), available.bottom() - height));
    }
    move(topLeft);

    updateToolActionState();
    updateUndoRedoState();
}

QImage CaptureEditorWindow::currentImage() const {
    return compositedImage();
}

void CaptureEditorWindow::applyShortcutSettings(
    const cappy::shortcuts::CaptureEditorShortcutSettings& shortcuts
) {
    shortcuts_ = shortcuts;
    refreshActionTooltips();
}

void CaptureEditorWindow::setLanguage(cappy::localization::AppLanguage language) {
    language_ = cappy::localization::resolvedAppLanguage(language);
    refreshActionTooltips();
}

void CaptureEditorWindow::closeEvent(QCloseEvent* event) {
    if (QWidget::keyboardGrabber() == this) {
        releaseKeyboard();
    }
    QWidget::closeEvent(event);
}

void CaptureEditorWindow::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    raise();
    activateWindow();
    setFocus(Qt::ActiveWindowFocusReason);
    grabKeyboard();
}

void CaptureEditorWindow::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.fillRect(rect(), QColor(18, 18, 18));

    if (!workingImage_.isNull()) {
        painter.drawImage(imageViewportRect_, workingImage_);
    }

    painter.save();
    painter.setClipRect(imageViewportRect_);
    drawTextAnnotations(painter);
    painter.restore();

    if (dragging_ && activeTool_ != Tool::Pencil && activeTool_ != Tool::Marker) {
        painter.save();
        painter.setClipRect(imageViewportRect_);
        drawShapePreview(painter);
        painter.restore();
    }

    if (!dragging_ && activeTool_ == Tool::Serial && hasHoverPoint_) {
        painter.save();
        painter.setClipRect(imageViewportRect_);
        drawSerialPreview(painter);
        painter.restore();
    }

    painter.setPen(QPen(QColor(255, 255, 255, 96), 1));
    painter.drawRect(imageViewportRect_.adjusted(0, 0, -1, -1));
}

void CaptureEditorWindow::mousePressEvent(QMouseEvent* event) {
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
        QWidget::mousePressEvent(event);
        return;
    }

    if (event->button() != Qt::LeftButton) {
        QWidget::mousePressEvent(event);
        return;
    }

    if (!isWithinImageViewport(event->pos())) {
        QWidget::mousePressEvent(event);
        return;
    }

    if (activeTool_ == Tool::None || workingImage_.isNull()) {
        const QPoint imagePoint = mapWidgetPointToImage(event->pos());
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
        selectedTextAnnotationIndex_ = -1;
        QWidget::mousePressEvent(event);
        return;
    }

    if (isPointTool()) {
        handlePointTool(event->pos());
        event->accept();
        return;
    }

    if (activeTool_ == Tool::Pencil || activeTool_ == Tool::Marker) {
        pushUndoSnapshot();
        lastPencilPoint_ = mapWidgetPointToImage(event->pos());
        dragging_ = true;
        update();
        event->accept();
        return;
    }

    beginShapeGesture(event->pos());
    event->accept();
}

void CaptureEditorWindow::mouseMoveEvent(QMouseEvent* event) {
    hasHoverPoint_ = isWithinImageViewport(event->pos());
    if (hasHoverPoint_) {
        hoverPoint_ = event->pos();
    }

    if (selectedTextAnnotationIndex_ >= 0
        && selectedTextAnnotationIndex_ < textAnnotations_.size()
        && (textAnnotationDragging_ || textAnnotationResizing_)) {
        auto& annotation = textAnnotations_[selectedTextAnnotationIndex_];
        const QPoint imagePoint = mapWidgetPointToImage(event->pos());
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
        event->accept();
        return;
    }

    if (!dragging_) {
        if (activeTool_ == Tool::Serial) {
            update();
        }
        QWidget::mouseMoveEvent(event);
        return;
    }

    if (activeTool_ == Tool::Pencil || activeTool_ == Tool::Marker) {
        const QPoint nextPoint = mapWidgetPointToImage(event->pos());
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
        update();
        return;
    }

    dragCurrent_ = event->pos();
    update();
}

void CaptureEditorWindow::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && (textAnnotationDragging_ || textAnnotationResizing_)) {
        textAnnotationDragging_ = false;
        textAnnotationResizing_ = false;
        emit imageChanged(compositedImage());
        update();
        event->accept();
        return;
    }

    if (event->button() != Qt::LeftButton || !dragging_) {
        QWidget::mouseReleaseEvent(event);
        return;
    }

    if (activeTool_ == Tool::Pencil || activeTool_ == Tool::Marker) {
        const QPoint nextPoint = mapWidgetPointToImage(event->pos());
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
        emit imageChanged(workingImage_);
        updateUndoRedoState();
        update();
        return;
    }

    dragCurrent_ = event->pos();
    commitCurrentShape();
}

void CaptureEditorWindow::mouseDoubleClickEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && activeTool_ == Tool::None) {
        emit copyRequested(compositedImage());
        close();
        event->accept();
        return;
    }

    QWidget::mouseDoubleClickEvent(event);
}

void CaptureEditorWindow::keyPressEvent(QKeyEvent* event) {
    handleShortcut(event);
    if (event->isAccepted()) {
        return;
    }

    QWidget::keyPressEvent(event);
}

void CaptureEditorWindow::leaveEvent(QEvent* event) {
    hasHoverPoint_ = false;
    update();
    QWidget::leaveEvent(event);
}

bool CaptureEditorWindow::eventFilter(QObject* watched, QEvent* event) {
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
            const QRect bounds = imageViewportRect_.adjusted(8, 8, -8, -8);
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

void CaptureEditorWindow::setupUi() {
    toolbar_ = new QFrame(this);
    toolbar_->setObjectName("captureEditorToolbar");
    toolbar_->setStyleSheet(
        "#captureEditorToolbar {"
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

    auto makeAction = [this](const QIcon& icon, bool checkable, auto handler) {
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
        themedIcon(this, "edit-copy", QStyle::SP_FileDialogListView),
        false,
        [this]() { emit copyRequested(compositedImage()); }
    );
    saveAction_ = makeAction(
        themedIcon(this, "document-save", QStyle::SP_DialogSaveButton),
        false,
        [this]() { emit saveRequested(compositedImage()); }
    );
    pinAction_ = makeAction(
        drawSymbolicIcon("pin"),
        false,
        [this]() { emit pinRequested(compositedImage()); }
    );
    ocrAction_ = makeAction(
        themedIcon(this, "scanner", QStyle::SP_FileDialogContentsView),
        false,
        [this]() { emit ocrRequested(compositedImage()); }
    );
    closeAction_ = makeAction(
        drawSymbolicIcon("close"),
        false,
        [this]() { close(); }
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

    auto* moreMenu = new QMenu(toolbar_);
    moreMenu->addAction(copyAction_);
    moreMenu->addAction(saveAction_);
    moreMenu->addAction(pinAction_);
    moreMenu->addSeparator();
    moreMenu->addAction(closeAction_);

    auto* moreButton = new QToolButton(toolbar_);
    moreButton->setAutoRaise(true);
    moreButton->setCursor(Qt::PointingHandCursor);
    moreButton->setIcon(drawSymbolicIcon("more"));
    moreButton->setIconSize(kIconSize);
    moreButton->setFixedSize(kButtonSize);
    moreButton->setPopupMode(QToolButton::InstantPopup);
    moreButton->setMenu(moreMenu);
    moreButton_ = moreButton;
    layout->addWidget(createToolbarButton(toolbar_, ocrAction_));
    layout->addWidget(moreButton);

    textInputPanel_ = new QWidget(this);
    textInputPanel_->hide();
    textInputPanel_->setObjectName("captureEditorTextInputPanel");
    textInputPanel_->setStyleSheet(
        "#captureEditorTextInputPanel {"
        "  background: rgba(22, 22, 22, 236);"
        "  border: 1px solid rgba(255, 255, 255, 70);"
        "  border-radius: 8px;"
        "}"
        "#captureEditorTextInputPanel QToolButton {"
        "  border: 0;"
        "  background: transparent;"
        "  color: rgb(220, 220, 220);"
        "  border-radius: 4px;"
        "}"
        "#captureEditorTextInputPanel QToolButton:hover {"
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
}

void CaptureEditorWindow::refreshActionTooltips() {
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
        joinShortcutLabels({shortcuts_.rectangle})
    ));
    setTooltip(ellipseAction_, QString("%1 (%2)").arg(
        text.toolEllipse,
        joinShortcutLabels({shortcuts_.ellipse})
    ));
    setTooltip(arrowAction_, QString("%1 (%2)").arg(
        text.toolArrow,
        joinShortcutLabels({shortcuts_.arrow})
    ));
    setTooltip(pencilAction_, QString("%1 (%2)").arg(
        text.toolPen,
        joinShortcutLabels({shortcuts_.pen})
    ));
    setTooltip(markerAction_, QString("%1 (%2)").arg(
        text.toolMarker,
        joinShortcutLabels({shortcuts_.marker})
    ));
    setTooltip(mosaicAction_, QString("%1 (%2)").arg(
        text.toolMosaic,
        joinShortcutLabels({shortcuts_.mosaic})
    ));
    setTooltip(textAction_, QString("%1 (%2)").arg(
        text.toolText,
        joinShortcutLabels({shortcuts_.text})
    ));
    setTooltip(serialAction_, QString("%1 (%2)").arg(
        text.toolSerial,
        joinShortcutLabels({shortcuts_.serial})
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
        joinShortcutLabels({shortcuts_.copy, shortcuts_.copyAndClose})
    ));
    setTooltip(saveAction_, QString("%1 (%2)").arg(
        text.actionSaveToCaptureFolder,
        joinShortcutLabels({shortcuts_.save, shortcuts_.saveAlt})
    ));
    setTooltip(pinAction_, QString("%1 (%2)").arg(
        text.actionPinToDesktop,
        joinShortcutLabels({shortcuts_.pin, shortcuts_.pinAlt})
    ));
    setTooltip(ocrAction_, text.actionExtractText);
    setTooltip(closeAction_, QString("%1 (%2)").arg(
        text.actionClose,
        joinShortcutLabels({shortcuts_.close})
    ));
    if (moreButton_ != nullptr) {
        moreButton_->setToolTip(text.actionMore);
        moreButton_->setStatusTip(text.actionMore);
    }
}

void CaptureEditorWindow::updateToolActionState() {
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
    setCursor(activeTool_ == Tool::None ? Qt::ArrowCursor : Qt::CrossCursor);
}

void CaptureEditorWindow::updateUndoRedoState() {
    if (undoAction_ != nullptr) {
        undoAction_->setEnabled(!undoStack_.isEmpty());
    }
    if (redoAction_ != nullptr) {
        redoAction_->setEnabled(!redoStack_.isEmpty());
    }
}

void CaptureEditorWindow::setActiveTool(Tool tool) {
    if (textEntryActive_ && tool != Tool::Text) {
        cancelTextEntry();
    }
    activeTool_ = tool;
    dragging_ = false;
    updateToolActionState();
    update();
}

void CaptureEditorWindow::beginShapeGesture(const QPoint& point) {
    pushUndoSnapshot();
    dragStart_ = point;
    dragCurrent_ = point;
    dragging_ = true;
    update();
}

void CaptureEditorWindow::handlePointTool(const QPoint& widgetPoint) {
    const QPoint imagePoint = mapWidgetPointToImage(widgetPoint);
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
        beginTextEntry(imagePoint, widgetPoint);
        break;
    }
    case Tool::Serial:
        drawSerialMarker(imagePoint);
        break;
    default:
        break;
    }
}

void CaptureEditorWindow::commitCurrentShape() {
    if (!dragging_) {
        return;
    }

    dragging_ = false;
    const QPoint imageStart = mapWidgetPointToImage(dragStart_);
    const QPoint imageEnd = mapWidgetPointToImage(dragCurrent_);

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

    emit imageChanged(workingImage_);
    updateUndoRedoState();
    update();
}

void CaptureEditorWindow::drawShapePreview(QPainter& painter) const {
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
        painter.drawRect(normalizedDragRect());
        break;
    case Tool::Ellipse:
        painter.drawEllipse(normalizedDragRect());
        break;
    case Tool::Arrow:
        drawArrow(painter, dragStart_, dragCurrent_);
        break;
    case Tool::Mosaic:
        painter.fillRect(normalizedDragRect(), QColor(255, 255, 255, 40));
        painter.drawRect(normalizedDragRect());
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

void CaptureEditorWindow::drawArrow(QPainter& painter, const QPoint& start, const QPoint& end) const {
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

void CaptureEditorWindow::applyMosaic(const QRect& imageRect) {
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

void CaptureEditorWindow::beginTextEntry(const QPoint& imagePoint, const QPoint& widgetPoint) {
    if (textInput_ == nullptr || textInputPanel_ == nullptr) {
        return;
    }

    Q_UNUSED(imagePoint);
    textEntryActive_ = true;
    textInputDragging_ = false;
    textInputResizing_ = false;
    if (QWidget::keyboardGrabber() == this) {
        releaseKeyboard();
    }

    pendingTextPixelSize_ = std::clamp(workingImage_.height() / 18, 16, 40);
    textInputPanel_->resize(
        std::clamp(imageViewportRect_.width() / 2, 220, 360),
        std::clamp(pendingTextPixelSize_ + 18, 36, 60)
    );
    updateTextInputAppearance();
    QPoint topLeft = clampTextInputPanelTopLeft(widgetPoint + QPoint(10, 10));

    textInput_->setPlaceholderText(cappy::localization::strings(language_).dialogAddTextLabel);
    textInput_->clear();
    textInputPanel_->move(topLeft);
    textInputPanel_->show();
    textInputPanel_->raise();
    updatePendingTextPointFromPanel();
    textInput_->setFocus(Qt::MouseFocusReason);
    textInput_->selectAll();
}

void CaptureEditorWindow::commitTextEntry() {
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
    EditorSnapshot::TextAnnotation annotation;
    annotation.text = labelText;
    annotation.pixelSize = pendingTextPixelSize_;
    annotation.rect = normalizedTextRect(labelText, pendingTextPixelSize_, pendingTextPoint_);
    textAnnotations_.push_back(annotation);
    selectedTextAnnotationIndex_ = textAnnotations_.size() - 1;

    emit imageChanged(compositedImage());
    updateUndoRedoState();
    update();
    activeTool_ = Tool::None;
    updateToolActionState();
}

void CaptureEditorWindow::cancelTextEntry() {
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

void CaptureEditorWindow::updateTextInputAppearance() {
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

void CaptureEditorWindow::adjustTextInputScale(int delta) {
    Q_UNUSED(delta);
}

void CaptureEditorWindow::updatePendingTextPointFromPanel() {
    if (textInputPanel_ == nullptr) {
        return;
    }

    pendingTextPoint_ = mapWidgetPointToImage(textInputPanel_->geometry().topLeft() + QPoint(10, 8));
}

QPoint CaptureEditorWindow::clampTextInputPanelTopLeft(const QPoint& topLeft) const {
    if (textInputPanel_ == nullptr) {
        return topLeft;
    }

    const QRect bounds = imageViewportRect_.adjusted(8, 8, -8, -8);
    return QPoint(
        std::clamp(topLeft.x(), bounds.left(), bounds.right() - textInputPanel_->width()),
        std::clamp(topLeft.y(), bounds.top(), bounds.bottom() - textInputPanel_->height())
    );
}

QImage CaptureEditorWindow::compositedImage() const {
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

void CaptureEditorWindow::drawTextAnnotations(QPainter& painter) const {
    for (int i = 0; i < textAnnotations_.size(); ++i) {
        const auto& annotation = textAnnotations_[i];
        const QRect widgetRect = annotation.rect.translated(imageViewportRect_.topLeft());
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

void CaptureEditorWindow::drawTextAnnotationSelection(
    QPainter& painter,
    const EditorSnapshot::TextAnnotation& annotation
) const {
    const QRect widgetRect = annotation.rect.translated(imageViewportRect_.topLeft());
    painter.setPen(QPen(QColor(255, 255, 255, 180), 1, Qt::DashLine));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(widgetRect.adjusted(-4, -4, 4, 4));
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 255, 255, 220));
    painter.drawEllipse(widgetRect.bottomRight() + QPoint(4, 4), 4, 4);
}

int CaptureEditorWindow::hitTestTextAnnotation(const QPoint& imagePoint, bool* resizeHandle) const {
    if (resizeHandle != nullptr) {
        *resizeHandle = false;
    }

    for (int i = textAnnotations_.size() - 1; i >= 0; --i) {
        const QRect rect = textAnnotations_[i].rect.adjusted(-4, -4, 4, 4);
        const QRect handleRect(
            rect.bottomRight() - QPoint(10, 10),
            QSize(20, 20)
        );
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

QRect CaptureEditorWindow::normalizedTextRect(
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

void CaptureEditorWindow::drawSerialMarker(const QPoint& imagePoint) {
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

    emit imageChanged(workingImage_);
    updateUndoRedoState();
    update();
}

void CaptureEditorWindow::drawSerialPreview(QPainter& painter) const {
    const int diameter = std::clamp(workingImage_.height() / 14, 26, 42);
    QRect previewRect(
        hoverPoint_.x() - diameter / 2,
        hoverPoint_.y() - diameter / 2,
        diameter,
        diameter
    );
    previewRect = previewRect.intersected(imageViewportRect_.adjusted(0, 0, -1, -1));
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

void CaptureEditorWindow::drawSerialBadge(
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

QColor CaptureEditorWindow::activeStrokeColor() const {
    switch (activeTool_) {
    case Tool::Marker:
        return QColor(255, 235, 59, 140);
    case Tool::Text:
        return QColor(255, 236, 120);
    default:
        return strokeColor_;
    }
}

int CaptureEditorWindow::activeStrokeWidth() const {
    switch (activeTool_) {
    case Tool::Marker:
        return 12;
    default:
        return strokeWidth_;
    }
}

bool CaptureEditorWindow::isPointTool() const {
    return activeTool_ == Tool::Text || activeTool_ == Tool::Serial;
}

QRect CaptureEditorWindow::imageViewportRect() const {
    return imageViewportRect_;
}

bool CaptureEditorWindow::isWithinImageViewport(const QPoint& point) const {
    return imageViewportRect().contains(point);
}

void CaptureEditorWindow::handleShortcut(QKeyEvent* event) {
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

    const auto toggleToolIfMatched = [this, event](const QString& shortcut, Tool tool) -> bool {
        if (!matchesShortcut(event, shortcut)) {
            return false;
        }

        setActiveTool(activeTool_ == tool ? Tool::None : tool);
        event->accept();
        return true;
    };

    if (toggleToolIfMatched(shortcuts_.rectangle, Tool::Rectangle)
        || toggleToolIfMatched(shortcuts_.ellipse, Tool::Ellipse)
        || toggleToolIfMatched(shortcuts_.arrow, Tool::Arrow)
        || toggleToolIfMatched(shortcuts_.pen, Tool::Pencil)
        || toggleToolIfMatched(shortcuts_.marker, Tool::Marker)
        || toggleToolIfMatched(shortcuts_.mosaic, Tool::Mosaic)
        || toggleToolIfMatched(shortcuts_.text, Tool::Text)
        || toggleToolIfMatched(shortcuts_.serial, Tool::Serial)) {
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

    if (matchesShortcut(event, shortcuts_.copy)) {
        emit copyRequested(compositedImage());
        event->accept();
        return;
    }

    if (matchesShortcut(event, shortcuts_.save) || matchesShortcut(event, shortcuts_.saveAlt)) {
        emit saveRequested(compositedImage());
        event->accept();
        return;
    }

    if (matchesShortcut(event, shortcuts_.pin) || matchesShortcut(event, shortcuts_.pinAlt)) {
        emit pinRequested(compositedImage());
        event->accept();
        return;
    }

    if (matchesShortcut(event, shortcuts_.copyAndClose)) {
        emit copyRequested(compositedImage());
        close();
        event->accept();
        return;
    }

    if (matchesShortcut(event, shortcuts_.close)) {
        close();
        event->accept();
    }
}

void CaptureEditorWindow::pushUndoSnapshot() {
    undoStack_.push_back(EditorSnapshot{
        .image = workingImage_,
        .textAnnotations = textAnnotations_,
        .nextSerialNumber = nextSerialNumber_,
    });
    redoStack_.clear();
    updateUndoRedoState();
}

void CaptureEditorWindow::undo() {
    if (undoStack_.isEmpty()) {
        return;
    }

    redoStack_.push_back(EditorSnapshot{
        .image = workingImage_,
        .textAnnotations = textAnnotations_,
        .nextSerialNumber = nextSerialNumber_,
    });
    const EditorSnapshot snapshot = undoStack_.takeLast();
    workingImage_ = snapshot.image;
    textAnnotations_ = snapshot.textAnnotations;
    nextSerialNumber_ = snapshot.nextSerialNumber;
    selectedTextAnnotationIndex_ = -1;
    emit imageChanged(compositedImage());
    updateUndoRedoState();
    update();
}

void CaptureEditorWindow::redo() {
    if (redoStack_.isEmpty()) {
        return;
    }

    undoStack_.push_back(EditorSnapshot{
        .image = workingImage_,
        .textAnnotations = textAnnotations_,
        .nextSerialNumber = nextSerialNumber_,
    });
    const EditorSnapshot snapshot = redoStack_.takeLast();
    workingImage_ = snapshot.image;
    textAnnotations_ = snapshot.textAnnotations;
    nextSerialNumber_ = snapshot.nextSerialNumber;
    selectedTextAnnotationIndex_ = -1;
    emit imageChanged(compositedImage());
    updateUndoRedoState();
    update();
}

QRect CaptureEditorWindow::normalizedDragRect() const {
    return QRect(dragStart_, dragCurrent_).normalized();
}

QPoint CaptureEditorWindow::mapWidgetPointToImage(const QPoint& point) const {
    if (workingImage_.isNull() || imageViewportRect_.width() <= 0 || imageViewportRect_.height() <= 0) {
        return {};
    }

    const QRect viewport = imageViewportRect();
    const int localX = std::clamp(point.x() - viewport.left(), 0, std::max(0, viewport.width() - 1));
    const int localY = std::clamp(point.y() - viewport.top(), 0, std::max(0, viewport.height() - 1));

    const int x = std::clamp(
        qRound((static_cast<double>(localX) / std::max(1, viewport.width() - 1))
               * (workingImage_.width() - 1)),
        0,
        std::max(0, workingImage_.width() - 1)
    );
    const int y = std::clamp(
        qRound((static_cast<double>(localY) / std::max(1, viewport.height() - 1))
               * (workingImage_.height() - 1)),
        0,
        std::max(0, workingImage_.height() - 1)
    );
    return {x, y};
}

}  // namespace cappy::features::editor
