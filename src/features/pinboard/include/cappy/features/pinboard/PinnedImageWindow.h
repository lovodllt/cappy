#pragma once

#include <optional>

#include <QImage>
#include <QPoint>
#include <QWidget>

#include "cappy/localization/Localization.h"
#include "cappy/shortcuts/ShortcutSettings.h"

class PinnedImageWindowTest;

namespace cappy::features::pinboard {

class PinnedImageWindow final : public QWidget {
    Q_OBJECT

public:
    explicit PinnedImageWindow(
        const QImage& image,
        std::optional<QPoint> initialTopLeft = std::nullopt,
        cappy::shortcuts::PinWindowShortcutSettings shortcuts = {},
        cappy::localization::AppLanguage language = cappy::localization::AppLanguage::English,
        QWidget* parent = nullptr
    );
    [[nodiscard]] bool isClickThrough() const;
    void applyShortcutSettings(const cappy::shortcuts::PinWindowShortcutSettings& shortcuts);
    void setLanguage(cappy::localization::AppLanguage language);
    void setClickThrough(bool enabled);

signals:
    void pinWindowClosed();
    void ocrRequested(const QImage& image);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;
    void closeEvent(QCloseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    void applyInteractionCursor();
    void copyImageToClipboard();
    void changeImage();
    void saveImageAs();
    void flipHorizontal();
    void flipVertical();
    void rotateClockwise();
    void rotateCounterclockwise();
    void invertColors();
    void restoreOriginalState();
    void setLocked(bool locked);
    void updateWindowSize();
    void setScaleFactor(double scaleFactor);
    void setWindowOpacityLevel(double opacityLevel);
    void setCurrentImage(const QImage& image);
    void replaceImageForEditing(const QImage& image);
    [[nodiscard]] QRect imageRect() const;

    friend class ::PinnedImageWindowTest;

    QImage initialImage_;
    QImage originalImage_;
    QImage image_;
    QPoint dragOffset_;
    bool dragging_ = false;
    bool locked_ = false;
    bool clickThrough_ = false;
    double scaleFactor_ = 1.0;
    double opacityLevel_ = 1.0;
    cappy::shortcuts::PinWindowShortcutSettings shortcuts_;
    cappy::localization::AppLanguage language_ = cappy::localization::AppLanguage::English;
};

}  // namespace cappy::features::pinboard
