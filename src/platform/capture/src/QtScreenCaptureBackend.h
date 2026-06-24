#pragma once

#include "cappy/platform/capture/IDesktopCaptureBackend.h"

namespace cappy::platform::capture {

class QtScreenCaptureBackend final : public IDesktopCaptureBackend {
  public:
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
    [[nodiscard]] QRect queryActiveWindowGeometry() const;
};

} // namespace cappy::platform::capture
