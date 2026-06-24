qt_add_executable(
    cappy
    main.cpp
    AppBootstrap.cpp
    AppController.cpp
    AppDirectories.cpp
    AppLogger.cpp
    AppSettings.cpp
    MainWindow.cpp
    ShellTheme.cpp
    SettingsDialog.cpp
)

target_include_directories(
    cappy
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(
    cappy
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        cappy_localization
        cappy_ocr_feature
        cappy_ocr_service
        cappy_shortcuts
        cappy_capture_service
        cappy_editor_feature
        cappy_hotkey_service
        cappy_pinboard_service
        cappy_plugin_host
)

cappy_configure_target(cappy)

set_target_properties(
    cappy
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

install(TARGETS cappy RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

if(UNIX AND NOT APPLE)
    install(
        FILES "${CAPPY_DESKTOP_FILE}"
        DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications"
    )
    install(
        FILES "${CMAKE_SOURCE_DIR}/resources/linux/cappy.svg"
        DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/scalable/apps"
        RENAME cappy.svg
    )
endif()

qt_add_executable(
    cappy_main_window_test
    tests/MainWindowTest.cpp
    MainWindow.cpp
    ShellTheme.cpp
)

target_include_directories(
    cappy_main_window_test
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(
    cappy_main_window_test
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Test
        cappy_localization
        cappy_ocr_service
        cappy_shortcuts
)

cappy_configure_target(cappy_main_window_test)

add_test(
    NAME cappy_main_window_test
    COMMAND cappy_main_window_test
)

qt_add_executable(
    cappy_settings_dialog_test
    tests/SettingsDialogTest.cpp
    AppSettings.cpp
    SettingsDialog.cpp
    ShellTheme.cpp
)

target_include_directories(
    cappy_settings_dialog_test
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(
    cappy_settings_dialog_test
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Test
        cappy_localization
        cappy_ocr_service
        cappy_shortcuts
)

cappy_configure_target(cappy_settings_dialog_test)

add_test(
    NAME cappy_settings_dialog_test
    COMMAND cappy_settings_dialog_test
)

qt_add_executable(
    cappy_settings_dialog_snapshot
    tests/SettingsDialogSnapshot.cpp
    AppSettings.cpp
    SettingsDialog.cpp
    ShellTheme.cpp
)

target_include_directories(
    cappy_settings_dialog_snapshot
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(
    cappy_settings_dialog_snapshot
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        cappy_localization
        cappy_ocr_service
        cappy_shortcuts
)

cappy_configure_target(cappy_settings_dialog_snapshot)

qt_add_executable(
    cappy_main_window_snapshot
    tests/MainWindowSnapshot.cpp
    MainWindow.cpp
    ShellTheme.cpp
)

target_include_directories(
    cappy_main_window_snapshot
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(
    cappy_main_window_snapshot
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        cappy_localization
        cappy_ocr_service
        cappy_shortcuts
)

cappy_configure_target(cappy_main_window_snapshot)
