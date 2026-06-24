#pragma once

#include <QCloseEvent>
#include <QImage>
#include <QPoint>
#include <QRect>
#include <QVector>
#include <QWidget>
#include <QtGui/qwindowdefs.h>

#include <functional>
#include <optional>

#include "cappy/shortcuts/ShortcutSettings.h"
#include "cappy/domain/capture/CaptureTypes.h"
#include "cappy/localization/Localization.h"

class QAction;
class QFrame;
class QLineEdit;
class QToolButton;

namespace cappy::features::capture {

enum class CaptureFinalizeAction {
    Copy,
    Save,
    Pin,
};

class CaptureOverlayWidget final : public QWidget {
    Q_OBJECT

  public:
    using WindowGeometryResolver = std::function<QRect(const QPoint&, WId)>;

    explicit CaptureOverlayWidget(
        const cappy::domain::capture::DesktopFrame& desktopFrame,
        cappy::shortcuts::CaptureOverlayShortcutSettings shortcuts = {},
        cappy::localization::AppLanguage language = cappy::localization::AppLanguage::English,
        std::optional<QRect> initialSelection = std::nullopt,
        cappy::domain::capture::CaptureMode captureMode =
            cappy::domain::capture::CaptureMode::Region,
        WindowGeometryResolver windowGeometryResolver = {}, QWidget* parent = nullptr);
    void setLanguage(cappy::localization::AppLanguage language);

  signals:
    void captureFinalized(const cappy::domain::capture::CaptureResult& result,
                          cappy::features::capture::CaptureFinalizeAction action);
    void captureCanceled();
    void ocrRequested(const QImage& image);

  protected:
    void closeEvent(QCloseEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void leaveEvent(QEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

  private:
    enum class Tool {
        None,
        Rectangle,
        Ellipse,
        Arrow,
        Pencil,
        Marker,
        Mosaic,
        Text,
        Serial,
    };

    enum class SelectionMode {
        None,
        Create,
        Move,
        ResizeLeft,
        ResizeRight,
        ResizeTop,
        ResizeBottom,
        ResizeTopLeft,
        ResizeTopRight,
        ResizeBottomLeft,
        ResizeBottomRight,
    };

    struct Snapshot {
        struct TextAnnotation {
            QString text;
            QRect rect;
            int pixelSize = 18;
        };
        QImage image;
        QVector<TextAnnotation> textAnnotations;
        int nextSerialNumber = 1;
    };

    void setupToolbar();
    void positionToolbar();
    void refreshActionTooltips();
    void updateToolActionState();
    void updateUndoRedoState();
    void updateInteractionCursor(const QPoint& point);
    void setActiveTool(Tool tool);
    void applyInitialSelection(const QRect& absoluteRect);
    void updateTrackedWindowSelection(const QPoint& overlayPoint);
    void refreshWorkingImageFromSelection();
    void finalizeSelection(CaptureFinalizeAction action);
    void handleShortcut(QKeyEvent* event);
    void beginSelectionGesture(const QPoint& point);
    void beginMoveOrResize(const QPoint& point, SelectionMode mode);
    void updateSelectionGeometry(const QPoint& point);
    void handlePointTool(const QPoint& point);
    void commitCurrentShape();
    void drawShapePreview(QPainter& painter) const;
    void drawArrow(QPainter& painter, const QPoint& start, const QPoint& end) const;
    void applyMosaic(const QRect& imageRect);
    void beginTextEntry(const QPoint& imagePoint, const QPoint& overlayPoint);
    void commitTextEntry();
    void cancelTextEntry();
    void updateTextInputAppearance();
    void adjustTextInputScale(int delta);
    void updatePendingTextPointFromPanel();
    [[nodiscard]] QPoint clampTextInputPanelTopLeft(const QPoint& topLeft) const;
    [[nodiscard]] QImage compositedImage() const;
    void drawTextAnnotations(QPainter& painter) const;
    void drawTextAnnotationSelection(QPainter& painter,
                                     const Snapshot::TextAnnotation& annotation) const;
    [[nodiscard]] int hitTestTextAnnotation(const QPoint& imagePoint,
                                            bool* resizeHandle = nullptr) const;
    [[nodiscard]] QRect normalizedTextRect(const QString& text, int pixelSize,
                                           const QPoint& topLeft) const;
    void drawSerialMarker(const QPoint& imagePoint);
    void drawSerialPreview(QPainter& painter) const;
    void drawSerialBadge(QPainter& painter, const QRect& rect, const QString& text,
                         const QColor& fillColor, const QColor& outlineColor) const;
    void pushUndoSnapshot();
    void undo();
    void redo();
    [[nodiscard]] bool hasSelection() const;
    [[nodiscard]] bool isPointTool() const;
    [[nodiscard]] bool selectionCanBeAdjusted() const;
    [[nodiscard]] bool isWithinSelection(const QPoint& point) const;
    [[nodiscard]] QRect normalizedSelectionRect() const;
    [[nodiscard]] QRect localSelectionRect() const;
    [[nodiscard]] QRect clampSelectionRect(const QRect& rect) const;
    [[nodiscard]] QPoint clampToBounds(const QPoint& point) const;
    [[nodiscard]] QPoint mapOverlayPointToImage(const QPoint& point) const;
    [[nodiscard]] SelectionMode hitTestSelection(const QPoint& point) const;
    [[nodiscard]] QColor activeStrokeColor() const;
    [[nodiscard]] int activeStrokeWidth() const;

    cappy::domain::capture::DesktopFrame desktopFrame_;
    cappy::domain::capture::CaptureMode captureMode_ = cappy::domain::capture::CaptureMode::Region;
    QFrame* toolbar_ = nullptr;
    QAction* rectangleAction_ = nullptr;
    QAction* ellipseAction_ = nullptr;
    QAction* arrowAction_ = nullptr;
    QAction* pencilAction_ = nullptr;
    QAction* markerAction_ = nullptr;
    QAction* mosaicAction_ = nullptr;
    QAction* textAction_ = nullptr;
    QAction* serialAction_ = nullptr;
    QAction* undoAction_ = nullptr;
    QAction* redoAction_ = nullptr;
    QAction* copyAction_ = nullptr;
    QAction* saveAction_ = nullptr;
    QAction* pinAction_ = nullptr;
    QAction* ocrAction_ = nullptr;
    QAction* closeAction_ = nullptr;
    QWidget* textInputPanel_ = nullptr;
    QLineEdit* textInput_ = nullptr;
    QToolButton* textMoveHandleButton_ = nullptr;
    QToolButton* textScaleDownButton_ = nullptr;
    QToolButton* textScaleUpButton_ = nullptr;
    cappy::shortcuts::CaptureOverlayShortcutSettings shortcuts_;
    cappy::localization::AppLanguage language_ = cappy::localization::AppLanguage::English;
    Tool activeTool_ = Tool::None;
    SelectionMode selectionMode_ = SelectionMode::None;
    QColor strokeColor_ = QColor(255, 80, 80);
    int strokeWidth_ = 3;
    int nextSerialNumber_ = 1;
    bool dragging_ = false;
    bool trackingClickPending_ = false;
    bool hasHoverPoint_ = false;
    bool hasEdits_ = false;
    bool textEntryActive_ = false;
    bool textInputDragging_ = false;
    bool textInputResizing_ = false;
    bool suppressNextReturnShortcut_ = false;
    int pendingTextPixelSize_ = 18;
    QSize textInputResizeStartSize_;
    QPoint textInputResizeStartGlobalPos_;
    QPoint dragStart_;
    QPoint dragCurrent_;
    QPoint selectionDragAnchor_;
    QPoint lastPencilPoint_;
    QPoint hoverPoint_;
    QPoint pendingTextPoint_;
    QPoint textInputDragOffset_;
    QRect selectionRect_;
    QRect trackedWindowSelectionRect_;
    QRect selectionRectAtDragStart_;
    QImage workingImage_;
    QVector<Snapshot> undoStack_;
    QVector<Snapshot> redoStack_;
    int selectedTextAnnotationIndex_ = -1;
    bool textAnnotationDragging_ = false;
    bool textAnnotationResizing_ = false;
    QPoint textAnnotationDragOffset_;
    QVector<Snapshot::TextAnnotation> textAnnotations_;
    WindowGeometryResolver windowGeometryResolver_;
};

} // namespace cappy::features::capture
