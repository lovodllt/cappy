#pragma once

#include <QImage>
#include <QRect>
#include <QString>
#include <QVector>

namespace cappy::domain::capture {

enum class CaptureMode {
    Region,
    Fullscreen,
    ActiveWindow,
    CurrentScreen,
    WindowFit,
};

struct ScreenFragment {
    QImage image;
    QRect geometry;

    [[nodiscard]] bool isNull() const {
        return image.isNull() || geometry.isEmpty();
    }
};

struct DesktopFrame {
    QImage image;
    QRect geometry;
    QVector<ScreenFragment> screenFragments;

    [[nodiscard]] bool isNull() const {
        return image.isNull() || geometry.isEmpty();
    }
};

struct CaptureResult {
    CaptureMode mode = CaptureMode::Region;
    QImage image;
    QRect geometry;
    QString backendName;

    [[nodiscard]] bool isNull() const {
        return image.isNull() || geometry.isEmpty();
    }
};

}  // namespace cappy::domain::capture
