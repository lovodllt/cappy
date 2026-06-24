#pragma once

#include "cappy/platform/capture/IDesktopCaptureBackend.h"

namespace cappy::platform::capture {

class UnsupportedCaptureBackend final : public IDesktopCaptureBackend {
  public:
    explicit UnsupportedCaptureBackend(QString reason);

    [[nodiscard]] QString backendName() const override;
    [[nodiscard]] bool isSupported() const override;
    [[nodiscard]] QString unsupportedReason() const override;
    [[nodiscard]] cappy::domain::capture::DesktopFrame captureVirtualDesktop() const override;
    [[nodiscard]] QRect activeWindowGeometry() const override;
    [[nodiscard]] QRect windowGeometryAtPoint(const QPoint& point,
                                              WId excludedWindowId) const override;
    [[nodiscard]] cappy::domain::capture::CaptureResult captureCurrentScreen() const override;
    [[nodiscard]] cappy::domain::capture::CaptureResult captureActiveWindow() const override;

  private:
    QString reason_;
};

} // namespace cappy::platform::capture
