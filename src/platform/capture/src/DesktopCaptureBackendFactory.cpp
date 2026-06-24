#include "cappy/platform/capture/DesktopCaptureBackendFactory.h"

#include <QGuiApplication>

#include "QtScreenCaptureBackend.h"
#include "UnsupportedCaptureBackend.h"

namespace cappy::platform::capture {

std::unique_ptr<IDesktopCaptureBackend> createDesktopCaptureBackend() {
    const QString platformName = QGuiApplication::platformName();

#if defined(Q_OS_WIN)
    return std::make_unique<QtScreenCaptureBackend>();
#elif defined(Q_OS_LINUX)
    if (platformName == "xcb") {
        return std::make_unique<QtScreenCaptureBackend>();
    }

    return std::make_unique<UnsupportedCaptureBackend>(
        QString("Unsupported Linux platform plugin: %1").arg(platformName)
    );
#else
    return std::make_unique<UnsupportedCaptureBackend>(
        QString("Unsupported platform plugin: %1").arg(platformName)
    );
#endif
}

}  // namespace cappy::platform::capture

