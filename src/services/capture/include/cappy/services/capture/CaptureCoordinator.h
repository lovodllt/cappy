#pragma once

#include <QObject>
#include <QPointer>

#include <memory>

#include "cappy/shortcuts/ShortcutSettings.h"
#include "cappy/domain/capture/CaptureTypes.h"
#include "cappy/features/capture/CaptureOverlayWidget.h"
#include "cappy/localization/Localization.h"

namespace cappy::platform::capture {
class IDesktopCaptureBackend;
}

namespace cappy::services::capture {

class CaptureCoordinator final : public QObject {
    Q_OBJECT

public:
    explicit CaptureCoordinator(QObject* parent = nullptr);
    ~CaptureCoordinator() override;

    [[nodiscard]] bool isRegionCaptureSupported() const;
    [[nodiscard]] bool isFullscreenCaptureSupported() const;
    [[nodiscard]] bool isCurrentScreenCaptureSupported() const;
    [[nodiscard]] bool isActiveWindowCaptureSupported() const;
    [[nodiscard]] bool isWindowFitCaptureSupported() const;
    [[nodiscard]] QString backendSummary() const;
    void setOverlayShortcutSettings(const cappy::shortcuts::CaptureOverlayShortcutSettings& shortcuts);
    void setOverlayLanguage(cappy::localization::AppLanguage language);
    void captureFullscreen();
    void captureCurrentScreen();
    void captureActiveWindow();
    void startWindowFitCapture();
    void startRegionCapture();

signals:
    void captureCompleted(const cappy::domain::capture::CaptureResult& result);
    void captureFinalized(
        const cappy::domain::capture::CaptureResult& result,
        cappy::features::capture::CaptureFinalizeAction action
    );
    void ocrRequested(const QImage& image);
    void captureFailed(const QString& message);
    void captureCanceled();

private:
    void resetOverlay();

    std::unique_ptr<cappy::platform::capture::IDesktopCaptureBackend> backend_;
    QPointer<cappy::features::capture::CaptureOverlayWidget> overlay_;
    cappy::domain::capture::DesktopFrame currentFrame_;
    cappy::shortcuts::CaptureOverlayShortcutSettings overlayShortcuts_;
    cappy::localization::AppLanguage overlayLanguage_ = cappy::localization::AppLanguage::English;
};

}  // namespace cappy::services::capture
