#include "cappy/services/hotkey/GlobalHotkeyService.h"

#include <QKeySequence>

#include "cappy/platform/hotkey/GlobalHotkeyBackendFactory.h"
#include "cappy/platform/hotkey/IGlobalHotkeyBackend.h"

namespace cappy::services::hotkey {

namespace {

QString bindingLabel(const cappy::platform::hotkey::GlobalHotkey& hotkey) {
    const QKeySequence sequence(static_cast<int>(hotkey.modifiers) | hotkey.key);
    return QString("%1=%2").arg(hotkey.displayName, sequence.toString(QKeySequence::NativeText));
}

} // namespace

GlobalHotkeyService::GlobalHotkeyService(QObject* parent)
    : QObject(parent), backend_(cappy::platform::hotkey::createGlobalHotkeyBackend()) {
    if (backend_ != nullptr) {
        backend_->setActivationHandler(
            [this](const QString& hotkeyId) { emit hotkeyActivated(hotkeyId); });
    }
}

GlobalHotkeyService::~GlobalHotkeyService() = default;

void GlobalHotkeyService::setBindings(QList<cappy::platform::hotkey::GlobalHotkey> bindings) {
    bindings_ = std::move(bindings);
    reapplyBindings();
}

void GlobalHotkeyService::setEnabled(bool enabled) {
    if (enabled_ == enabled) {
        return;
    }

    enabled_ = enabled;
    reapplyBindings();
}

void GlobalHotkeyService::setSuspended(bool suspended) {
    if (suspended_ == suspended) {
        return;
    }

    suspended_ = suspended;
    reapplyBindings();
}

bool GlobalHotkeyService::isEnabled() const {
    return enabled_;
}

bool GlobalHotkeyService::isSuspended() const {
    return suspended_;
}

bool GlobalHotkeyService::isSupported() const {
    return backend_ != nullptr && backend_->isSupported();
}

QString GlobalHotkeyService::backendSummary() const {
    if (backend_ == nullptr) {
        return "global hotkey backend unavailable";
    }

    if (backend_->isSupported()) {
        return enabled_ ? QString("%1 (ready)").arg(backend_->backendName())
                        : QString("%1 (disabled)").arg(backend_->backendName());
    }

    return QString("%1 (unavailable: %2)")
        .arg(backend_->backendName(), backend_->unsupportedReason());
}

QString GlobalHotkeyService::bindingsSummary() const {
    if (bindings_.isEmpty()) {
        return "none";
    }

    QStringList labels;
    labels.reserve(bindings_.size());
    for (const auto& binding : bindings_) {
        labels.push_back(bindingLabel(binding));
    }
    return labels.join(", ");
}

QStringList GlobalHotkeyService::lastRegistrationErrors() const {
    return lastRegistrationErrors_;
}

void GlobalHotkeyService::reapplyBindings() {
    lastRegistrationErrors_.clear();
    if (backend_ == nullptr) {
        return;
    }

    backend_->unregisterAll();

    if (!enabled_ || suspended_ || !backend_->isSupported()) {
        return;
    }

    for (const auto& binding : bindings_) {
        if (!backend_->registerHotkey(binding)) {
            lastRegistrationErrors_.push_back(backend_->lastError());
        }
    }
}

} // namespace cappy::services::hotkey
