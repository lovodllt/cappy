#include "cappy/services/pinboard/PinboardManager.h"

#include <algorithm>

#include <QImage>

#include "cappy/features/pinboard/PinnedImageWindow.h"

namespace cappy::services::pinboard {

PinboardManager::PinboardManager(QObject* parent) : QObject(parent) {}

PinboardManager::~PinboardManager() = default;

bool PinboardManager::pinImage(const QImage& image, std::optional<QPoint> initialTopLeft) {
    if (image.isNull()) {
        return false;
    }

    auto* window = new cappy::features::pinboard::PinnedImageWindow(
        image, initialTopLeft, pinWindowShortcuts_, pinWindowLanguage_);
    windows_.push_back(window);
    connect(window, &QObject::destroyed, this, [this]() {
        std::erase_if(windows_, [](const auto& candidate) { return candidate.isNull(); });
    });
    connect(window, &cappy::features::pinboard::PinnedImageWindow::ocrRequested, this,
            [this](const QImage& image) { emit ocrRequested(image); });
    window->show();
    return true;
}

void PinboardManager::setPinWindowShortcutSettings(
    const cappy::shortcuts::PinWindowShortcutSettings& shortcuts) {
    pinWindowShortcuts_ = shortcuts;
    for (const auto& window : windows_) {
        if (!window.isNull()) {
            window->applyShortcutSettings(pinWindowShortcuts_);
        }
    }
}

void PinboardManager::setPinWindowLanguage(cappy::localization::AppLanguage language) {
    pinWindowLanguage_ = cappy::localization::resolvedAppLanguage(language);
    for (const auto& window : windows_) {
        if (!window.isNull()) {
            window->setLanguage(pinWindowLanguage_);
        }
    }
}

int PinboardManager::openPinCount() const {
    return static_cast<int>(std::count_if(windows_.begin(), windows_.end(),
                                          [](const auto& window) { return !window.isNull(); }));
}

int PinboardManager::setClickThroughForAllPins(bool enabled) {
    int updatedCount = 0;
    for (const auto& window : windows_) {
        if (!window.isNull()) {
            window->setClickThrough(enabled);
            ++updatedCount;
        }
    }

    std::erase_if(windows_, [](const auto& candidate) { return candidate.isNull(); });
    return updatedCount;
}

void PinboardManager::closeAllPins() {
    for (const auto& window : windows_) {
        if (!window.isNull()) {
            window->close();
        }
    }

    std::erase_if(windows_, [](const auto& candidate) { return candidate.isNull(); });
}

} // namespace cappy::services::pinboard
