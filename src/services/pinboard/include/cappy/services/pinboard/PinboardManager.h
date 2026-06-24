#pragma once

#include <memory>
#include <optional>
#include <vector>

#include <QObject>
#include <QImage>
#include <QPoint>
#include <QPointer>

#include "cappy/localization/Localization.h"
#include "cappy/shortcuts/ShortcutSettings.h"

namespace cappy::features::pinboard {
class PinnedImageWindow;
}

namespace cappy::services::pinboard {

class PinboardManager final : public QObject {
    Q_OBJECT

public:
    explicit PinboardManager(QObject* parent = nullptr);
    ~PinboardManager() override;

    [[nodiscard]] bool pinImage(
        const QImage& image,
        std::optional<QPoint> initialTopLeft = std::nullopt
    );
    void setPinWindowShortcutSettings(const cappy::shortcuts::PinWindowShortcutSettings& shortcuts);
    void setPinWindowLanguage(cappy::localization::AppLanguage language);
    [[nodiscard]] int openPinCount() const;
    int setClickThroughForAllPins(bool enabled);
    void closeAllPins();

signals:
    void ocrRequested(const QImage& image);

private:
    cappy::shortcuts::PinWindowShortcutSettings pinWindowShortcuts_;
    cappy::localization::AppLanguage pinWindowLanguage_ = cappy::localization::AppLanguage::English;
    std::vector<QPointer<cappy::features::pinboard::PinnedImageWindow>> windows_;
};

}  // namespace cappy::services::pinboard
