set(cappy_hotkey_platform_sources
    src/GlobalHotkeyBackendFactory.cpp
    src/UnsupportedGlobalHotkeyBackend.cpp
)

if(UNIX AND NOT APPLE)
    list(APPEND cappy_hotkey_platform_sources
        src/X11GlobalHotkeyBackend.cpp
    )
endif()

if(WIN32)
    list(APPEND cappy_hotkey_platform_sources
        src/WinGlobalHotkeyBackend.cpp
    )
endif()

add_library(
    cappy_hotkey_platform
    STATIC
        ${cappy_hotkey_platform_sources}
)

target_include_directories(
    cappy_hotkey_platform
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
    cappy_hotkey_platform
    PUBLIC
        Qt6::Core
        Qt6::Gui
)

if(UNIX AND NOT APPLE)
    target_link_libraries(
        cappy_hotkey_platform
        PUBLIC
            X11::X11
            xcb
    )
endif()

cappy_configure_target(cappy_hotkey_platform)
