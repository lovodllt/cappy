#pragma once

#include <QList>
#include <QObject>
#include <QStringList>

#include <memory>

#include "cappy/platform/hotkey/GlobalHotkeyTypes.h"

namespace cappy::platform::hotkey {
class IGlobalHotkeyBackend;
}

namespace cappy::services::hotkey {

class GlobalHotkeyService final : public QObject {
    Q_OBJECT

  public:
    explicit GlobalHotkeyService(QObject* parent = nullptr);
    ~GlobalHotkeyService() override;

    void setBindings(QList<cappy::platform::hotkey::GlobalHotkey> bindings);
    void setEnabled(bool enabled);
    void setSuspended(bool suspended);
    bool isEnabled() const;
    bool isSuspended() const;
    bool isSupported() const;
    QString backendSummary() const;
    QString bindingsSummary() const;
    QStringList lastRegistrationErrors() const;

  signals:
    void hotkeyActivated(const QString& hotkeyId);

  private:
    void reapplyBindings();

    QList<cappy::platform::hotkey::GlobalHotkey> bindings_;
    QStringList lastRegistrationErrors_;
    bool enabled_ = true;
    bool suspended_ = false;
    std::unique_ptr<cappy::platform::hotkey::IGlobalHotkeyBackend> backend_;
};

} // namespace cappy::services::hotkey
