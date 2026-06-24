add_library(
    cappy_capture_feature
    STATIC
        include/cappy/features/capture/CaptureOverlayWidget.h
        src/CaptureOverlayWidget.cpp
)

target_include_directories(
    cappy_capture_feature
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
    cappy_capture_feature
    PUBLIC
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        cappy_localization
        cappy_shortcuts
        cappy_capture_domain
)

cappy_configure_target(cappy_capture_feature)

qt_add_executable(
    cappy_capture_overlay_widget_test
    tests/CaptureOverlayWidgetTest.cpp
)

target_include_directories(
    cappy_capture_overlay_widget_test
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
    cappy_capture_overlay_widget_test
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Test
        cappy_localization
        cappy_shortcuts
        cappy_capture_feature
)

cappy_configure_target(cappy_capture_overlay_widget_test)

add_test(
    NAME cappy_capture_overlay_widget_test
    COMMAND cappy_capture_overlay_widget_test
)
