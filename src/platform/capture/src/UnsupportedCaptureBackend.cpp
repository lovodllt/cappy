#include "UnsupportedCaptureBackend.h"

namespace cappy::platform::capture {

UnsupportedCaptureBackend::UnsupportedCaptureBackend(QString reason) : reason_(std::move(reason)) {}

QString UnsupportedCaptureBackend::backendName() const {
    return "unsupported";
}

bool UnsupportedCaptureBackend::isSupported() const {
    return false;
}

QString UnsupportedCaptureBackend::unsupportedReason() const {
    return reason_;
}

cappy::domain::capture::DesktopFrame UnsupportedCaptureBackend::captureVirtualDesktop() const {
    return {};
}

QRect UnsupportedCaptureBackend::activeWindowGeometry() const {
    return {};
}

QRect UnsupportedCaptureBackend::windowGeometryAtPoint(const QPoint& point,
                                                       WId excludedWindowId) const {
    Q_UNUSED(point);
    Q_UNUSED(excludedWindowId);
    return {};
}

cappy::domain::capture::CaptureResult UnsupportedCaptureBackend::captureCurrentScreen() const {
    return {};
}

cappy::domain::capture::CaptureResult UnsupportedCaptureBackend::captureActiveWindow() const {
    return {};
}

} // namespace cappy::platform::capture
