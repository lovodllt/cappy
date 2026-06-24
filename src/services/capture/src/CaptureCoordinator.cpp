#include "cappy/services/capture/CaptureCoordinator.h"

#include <QtMath>

#include "cappy/domain/capture/CaptureImageOps.h"
#include "cappy/features/capture/CaptureOverlayWidget.h"
#include "cappy/platform/capture/DesktopCaptureBackendFactory.h"
#include "cappy/platform/capture/IDesktopCaptureBackend.h"

namespace cappy::services::capture {

CaptureCoordinator::CaptureCoordinator(QObject* parent)
    : QObject(parent), backend_(cappy::platform::capture::createDesktopCaptureBackend()) {}

CaptureCoordinator::~CaptureCoordinator() = default;

bool CaptureCoordinator::isRegionCaptureSupported() const {
    return backend_ != nullptr && backend_->isSupported();
}

bool CaptureCoordinator::isFullscreenCaptureSupported() const {
    return backend_ != nullptr && backend_->isSupported();
}

bool CaptureCoordinator::isCurrentScreenCaptureSupported() const {
    return backend_ != nullptr && backend_->isSupported();
}

bool CaptureCoordinator::isActiveWindowCaptureSupported() const {
    return backend_ != nullptr && backend_->isSupported();
}

bool CaptureCoordinator::isWindowFitCaptureSupported() const {
    return backend_ != nullptr && backend_->isSupported();
}

QString CaptureCoordinator::backendSummary() const {
    if (backend_ == nullptr) {
        return "capture backend unavailable";
    }

    if (backend_->isSupported()) {
        return QString("%1 (ready)").arg(backend_->backendName());
    }

    return QString("%1 (unavailable: %2)")
        .arg(backend_->backendName(), backend_->unsupportedReason());
}

void CaptureCoordinator::setOverlayShortcutSettings(
    const cappy::shortcuts::CaptureOverlayShortcutSettings& shortcuts) {
    overlayShortcuts_ = shortcuts;
}

void CaptureCoordinator::setOverlayLanguage(cappy::localization::AppLanguage language) {
    overlayLanguage_ = cappy::localization::resolvedAppLanguage(language);
    if (overlay_ != nullptr) {
        overlay_->setLanguage(overlayLanguage_);
    }
}

void CaptureCoordinator::captureFullscreen() {
    if (backend_ == nullptr) {
        emit captureFailed("No capture backend is available.");
        return;
    }

    if (!backend_->isSupported()) {
        emit captureFailed(backend_->unsupportedReason());
        return;
    }

    const cappy::domain::capture::DesktopFrame frame = backend_->captureVirtualDesktop();
    if (frame.isNull()) {
        emit captureFailed("Failed to capture the current desktop frame.");
        return;
    }

    cappy::domain::capture::CaptureResult result;
    result.mode = cappy::domain::capture::CaptureMode::Fullscreen;
    result.geometry = frame.geometry;
    result.backendName = backend_->backendName();
    result.image = frame.image;

    emit captureCompleted(result);
}

void CaptureCoordinator::captureCurrentScreen() {
    if (backend_ == nullptr) {
        emit captureFailed("No capture backend is available.");
        return;
    }

    if (!backend_->isSupported()) {
        emit captureFailed(backend_->unsupportedReason());
        return;
    }

    const cappy::domain::capture::CaptureResult result = backend_->captureCurrentScreen();
    if (result.isNull()) {
        emit captureFailed("Failed to capture the current screen.");
        return;
    }

    emit captureCompleted(result);
}

void CaptureCoordinator::captureActiveWindow() {
    if (backend_ == nullptr) {
        emit captureFailed("No capture backend is available.");
        return;
    }

    if (!backend_->isSupported()) {
        emit captureFailed(backend_->unsupportedReason());
        return;
    }

    const cappy::domain::capture::CaptureResult result = backend_->captureActiveWindow();
    if (result.isNull()) {
        emit captureFailed("Failed to capture the active window.");
        return;
    }

    emit captureCompleted(result);
}

void CaptureCoordinator::startWindowFitCapture() {
    if (backend_ == nullptr) {
        emit captureFailed("No capture backend is available.");
        return;
    }

    if (!backend_->isSupported()) {
        emit captureFailed(backend_->unsupportedReason());
        return;
    }

    if (overlay_ != nullptr) {
        emit captureFailed("A capture session is already active.");
        return;
    }

    currentFrame_ = backend_->captureVirtualDesktop();
    if (currentFrame_.isNull()) {
        emit captureFailed("Failed to capture the current desktop frame.");
        return;
    }

    overlay_ = new cappy::features::capture::CaptureOverlayWidget(
        currentFrame_, overlayShortcuts_, overlayLanguage_, std::nullopt,
        cappy::domain::capture::CaptureMode::WindowFit,
        [this](const QPoint& point, WId excludedWindowId) -> QRect {
            if (backend_ == nullptr) {
                return {};
            }
            return backend_->windowGeometryAtPoint(point, excludedWindowId);
        });
    connect(overlay_, &cappy::features::capture::CaptureOverlayWidget::captureFinalized, this,
            [this](const cappy::domain::capture::CaptureResult& result,
                   cappy::features::capture::CaptureFinalizeAction action) {
                emit captureFinalized(result, action);
                resetOverlay();
            });
    connect(overlay_, &cappy::features::capture::CaptureOverlayWidget::captureCanceled, this,
            [this]() {
                emit captureCanceled();
                resetOverlay();
            });
    connect(overlay_, &cappy::features::capture::CaptureOverlayWidget::ocrRequested, this,
            [this](const QImage& image) {
                const QImage capturedImage = image;
                resetOverlay();
                emit ocrRequested(capturedImage);
            });
    connect(overlay_, &QObject::destroyed, this, [this]() { overlay_ = nullptr; });
    overlay_->show();
}

void CaptureCoordinator::startRegionCapture() {
    if (backend_ == nullptr) {
        emit captureFailed("No capture backend is available.");
        return;
    }

    if (!backend_->isSupported()) {
        emit captureFailed(backend_->unsupportedReason());
        return;
    }

    if (overlay_ != nullptr) {
        emit captureFailed("A capture session is already active.");
        return;
    }

    currentFrame_ = backend_->captureVirtualDesktop();
    if (currentFrame_.isNull()) {
        emit captureFailed("Failed to capture the current desktop frame.");
        return;
    }

    overlay_ = new cappy::features::capture::CaptureOverlayWidget(currentFrame_, overlayShortcuts_,
                                                                  overlayLanguage_);
    connect(overlay_, &cappy::features::capture::CaptureOverlayWidget::captureFinalized, this,
            [this](const cappy::domain::capture::CaptureResult& result,
                   cappy::features::capture::CaptureFinalizeAction action) {
                emit captureFinalized(result, action);
                resetOverlay();
            });
    connect(overlay_, &cappy::features::capture::CaptureOverlayWidget::captureCanceled, this,
            [this]() {
                emit captureCanceled();
                resetOverlay();
            });
    connect(overlay_, &cappy::features::capture::CaptureOverlayWidget::ocrRequested, this,
            [this](const QImage& image) {
                const QImage capturedImage = image;
                resetOverlay();
                emit ocrRequested(capturedImage);
            });
    connect(overlay_, &QObject::destroyed, this, [this]() { overlay_ = nullptr; });
    overlay_->show();
}

void CaptureCoordinator::resetOverlay() {
    currentFrame_ = {};
    if (overlay_ != nullptr) {
        overlay_->disconnect(this);
        overlay_->close();
        overlay_ = nullptr;
    }
}

} // namespace cappy::services::capture
