#pragma once

#include <QPoint>
#include <QRect>
#include <QtGui/qwindowdefs.h>

#include "cappy/domain/capture/CaptureTypes.h"

namespace cappy::platform::capture {

class IDesktopCaptureBackend {
  public:
    virtual ~IDesktopCaptureBackend() = default;

    [[nodiscard]] virtual QString backendName() const = 0;
    [[nodiscard]] virtual bool isSupported() const = 0;
    [[nodiscard]] virtual QString unsupportedReason() const = 0;
    [[nodiscard]] virtual cappy::domain::capture::DesktopFrame captureVirtualDesktop() const = 0;
    [[nodiscard]] virtual QRect activeWindowGeometry() const = 0;
    [[nodiscard]] virtual QRect windowGeometryAtPoint(const QPoint& point,
                                                      WId excludedWindowId) const = 0;
    [[nodiscard]] virtual cappy::domain::capture::CaptureResult captureCurrentScreen() const = 0;
    [[nodiscard]] virtual cappy::domain::capture::CaptureResult captureActiveWindow() const = 0;
};

} // namespace cappy::platform::capture
