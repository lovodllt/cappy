add_library(
    cappy_shortcuts
    STATIC
        include/cappy/shortcuts/ShortcutSettings.h
        src/ShortcutSettings.cpp
)

target_include_directories(
    cappy_shortcuts
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
    cappy_shortcuts
    PUBLIC
        Qt6::Core
        Qt6::Gui
)

cappy_configure_target(cappy_shortcuts)
