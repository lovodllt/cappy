#pragma once

#include <QColor>
#include <QCloseEvent>
#include <QImage>
#include <QEvent>
#include <QPoint>
#include <QRect>
#include <QVector>
#include <QWidget>

#include "cappy/localization/Localization.h"
#include "cappy/shortcuts/ShortcutSettings.h"

class QAction;
class QLineEdit;
class QToolButton;

namespace cappy::features::editor {

class CaptureEditorWindow final : public QWidget {
    Q_OBJECT

  public:
    explicit CaptureEditorWindow(
        const QImage& image, const QRect& globalGeometry,
        cappy::shortcuts::CaptureEditorShortcutSettings shortcuts = {},
        cappy::localization::AppLanguage language = cappy::localization::AppLanguage::English,
        QWidget* parent = nullptr);

    [[nodiscard]] QImage currentImage() const;
    void applyShortcutSettings(const cappy::shortcuts::CaptureEditorShortcutSettings& shortcuts);
    void setLanguage(cappy::localization::AppLanguage language);

  signals:
    void imageChanged(const QImage& image);
    void copyRequested(const QImage& image);
    void saveRequested(const QImage& image);
    void pinRequested(const QImage& image);
    void ocrRequested(const QImage& image);

  protected:
    void closeEvent(QCloseEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
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

    struct EditorSnapshot {
        QImage image;
        struct TextAnnotation {
            QString text;
            QRect rect;
            int pixelSize = 18;
        };
        QVector<TextAnnotation> textAnnotations;
        int nextSerialNumber = 1;
    };

    void setupUi();
    void refreshActionTooltips();
    void updateToolActionState();
    void updateUndoRedoState();
    void setActiveTool(Tool tool);
    void beginShapeGesture(const QPoint& point);
    void handlePointTool(const QPoint& widgetPoint);
    void commitCurrentShape();
    void drawShapePreview(QPainter& painter) const;
    void drawArrow(QPainter& painter, const QPoint& start, const QPoint& end) const;
    void applyMosaic(const QRect& imageRect);
    void beginTextEntry(const QPoint& imagePoint, const QPoint& widgetPoint);
    void commitTextEntry();
    void cancelTextEntry();
    void updateTextInputAppearance();
    void adjustTextInputScale(int delta);
    void updatePendingTextPointFromPanel();
    [[nodiscard]] QPoint clampTextInputPanelTopLeft(const QPoint& topLeft) const;
    [[nodiscard]] QImage compositedImage() const;
    void drawTextAnnotations(QPainter& painter) const;
    void drawTextAnnotationSelection(QPainter& painter,
                                     const EditorSnapshot::TextAnnotation& annotation) const;
    [[nodiscard]] int hitTestTextAnnotation(const QPoint& imagePoint,
                                            bool* resizeHandle = nullptr) const;
    [[nodiscard]] QRect normalizedTextRect(const QString& text, int pixelSize,
                                           const QPoint& topLeft) const;
    void drawSerialMarker(const QPoint& imagePoint);
    void drawSerialPreview(QPainter& painter) const;
    void drawSerialBadge(QPainter& painter, const QRect& rect, const QString& text,
                         const QColor& fillColor, const QColor& outlineColor) const;
    [[nodiscard]] QColor activeStrokeColor() const;
    [[nodiscard]] int activeStrokeWidth() const;
    [[nodiscard]] bool isPointTool() const;
    [[nodiscard]] QRect imageViewportRect() const;
    [[nodiscard]] bool isWithinImageViewport(const QPoint& point) const;
    void handleShortcut(QKeyEvent* event);
    void pushUndoSnapshot();
    void undo();
    void redo();
    [[nodiscard]] QRect normalizedDragRect() const;
    [[nodiscard]] QPoint mapWidgetPointToImage(const QPoint& point) const;

    QImage workingImage_;
    QRect globalGeometry_;
    QWidget* toolbar_ = nullptr;
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
    QWidget* moreButton_ = nullptr;
    QLineEdit* textInput_ = nullptr;
    QToolButton* textMoveHandleButton_ = nullptr;
    QToolButton* textScaleDownButton_ = nullptr;
    QToolButton* textScaleUpButton_ = nullptr;
    cappy::shortcuts::CaptureEditorShortcutSettings shortcuts_;
    cappy::localization::AppLanguage language_ = cappy::localization::AppLanguage::English;
    Tool activeTool_ = Tool::None;
    QColor strokeColor_ = QColor(255, 80, 80);
    int strokeWidth_ = 3;
    int nextSerialNumber_ = 1;
    bool dragging_ = false;
    QPoint dragStart_;
    QPoint dragCurrent_;
    QPoint lastPencilPoint_;
    QPoint hoverPoint_;
    QPoint pendingTextPoint_;
    QPoint textInputDragOffset_;
    bool hasHoverPoint_ = false;
    bool textEntryActive_ = false;
    bool textInputDragging_ = false;
    bool textInputResizing_ = false;
    bool suppressNextReturnShortcut_ = false;
    int pendingTextPixelSize_ = 18;
    int selectedTextAnnotationIndex_ = -1;
    bool textAnnotationDragging_ = false;
    bool textAnnotationResizing_ = false;
    QPoint textAnnotationDragOffset_;
    QSize textInputResizeStartSize_;
    QPoint textInputResizeStartGlobalPos_;
    QRect imageViewportRect_;
    QVector<EditorSnapshot> undoStack_;
    QVector<EditorSnapshot> redoStack_;
    QVector<EditorSnapshot::TextAnnotation> textAnnotations_;
};

} // namespace cappy::features::editor
