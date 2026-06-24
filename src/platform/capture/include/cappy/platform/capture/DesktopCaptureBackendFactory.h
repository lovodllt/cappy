#pragma once

#include <memory>

namespace cappy::platform::capture {

class IDesktopCaptureBackend;

std::unique_ptr<IDesktopCaptureBackend> createDesktopCaptureBackend();

} // namespace cappy::platform::capture
