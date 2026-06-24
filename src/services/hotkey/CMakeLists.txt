add_library(
    cappy_hotkey_service
    STATIC
        include/cappy/services/hotkey/GlobalHotkeyService.h
        src/GlobalHotkeyService.cpp
)

target_include_directories(
    cappy_hotkey_service
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
    cappy_hotkey_service
    PUBLIC
        Qt6::Core
        Qt6::Gui
        cappy_hotkey_platform
)

cappy_configure_target(cappy_hotkey_service)
