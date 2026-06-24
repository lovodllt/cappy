#pragma once

#include <memory>

namespace cappy::platform::hotkey {

class IGlobalHotkeyBackend;

[[nodiscard]] std::unique_ptr<IGlobalHotkeyBackend> createGlobalHotkeyBackend();

}  // namespace cappy::platform::hotkey
