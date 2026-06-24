#pragma once

#include <QFlags>
#include <QString>
#include <Qt>

namespace cappy::platform::hotkey {

struct GlobalHotkey {
    QString id;
    QString displayName;
    Qt::Key key = Qt::Key_unknown;
    Qt::KeyboardModifiers modifiers = Qt::NoModifier;
};

} // namespace cappy::platform::hotkey
