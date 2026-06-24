#pragma once

#include <QPainter>
#include <QtMath>

#include "cappy/domain/capture/CaptureTypes.h"

namespace cappy::domain::capture {

inline QRect mapGlobalRectToFragmentPixels(const QRect& globalRect,
                                           const ScreenFragment& fragment) {
    if (fragment.isNull() || globalRect.isEmpty()) {
        return {};
    }

    const QRect clippedGlobalRect = globalRect.intersected(fragment.geometry);
    if (clippedGlobalRect.isEmpty()) {
        return {};
    }

    const QRect localRect = clippedGlobalRect.translated(-fragment.geometry.topLeft());
    const double xScale = static_cast<double>(fragment.image.width()) / fragment.geometry.width();
    const double yScale = static_cast<double>(fragment.image.height()) / fragment.geometry.height();

    const int left = qFloor(localRect.left() * xScale);
    const int top = qFloor(localRect.top() * yScale);
    const int right = qCeil((localRect.left() + localRect.width()) * xScale);
    const int bottom = qCeil((localRect.top() + localRect.height()) * yScale);

    return QRect(left, top, right - left, bottom - top).intersected(fragment.image.rect());
}

inline QImage cropNormalizedImage(const DesktopFrame& frame, const QRect& globalRect) {
    if (frame.isNull() || globalRect.isEmpty()) {
        return {};
    }

    const QRect clippedGlobalRect = globalRect.intersected(frame.geometry);
    if (clippedGlobalRect.isEmpty()) {
        return {};
    }

    if (frame.screenFragments.isEmpty()) {
        return frame.image.copy(clippedGlobalRect.translated(-frame.geometry.topLeft()));
    }

    QImage composed(clippedGlobalRect.size(), QImage::Format_ARGB32_Premultiplied);
    composed.fill(Qt::transparent);

    QPainter painter(&composed);
    for (const ScreenFragment& fragment : frame.screenFragments) {
        const QRect targetGlobalRect = clippedGlobalRect.intersected(fragment.geometry);
        if (targetGlobalRect.isEmpty()) {
            continue;
        }

        const QRect sourceRect = mapGlobalRectToFragmentPixels(targetGlobalRect, fragment);
        if (sourceRect.isEmpty()) {
            continue;
        }

        const QRect targetRect = targetGlobalRect.translated(-clippedGlobalRect.topLeft());
        painter.drawImage(targetRect, fragment.image, sourceRect);
    }
    painter.end();

    return composed;
}

} // namespace cappy::domain::capture
