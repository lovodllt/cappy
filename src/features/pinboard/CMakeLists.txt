add_library(
    cappy_pinboard_feature
    STATIC
        include/cappy/features/pinboard/PinnedImageWindow.h
        src/PinnedImageWindow.cpp
)

target_include_directories(
    cappy_pinboard_feature
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
    cappy_pinboard_feature
    PUBLIC
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        cappy_localization
        cappy_shortcuts
)

cappy_configure_target(cappy_pinboard_feature)

qt_add_executable(
    cappy_pinned_image_window_test
    tests/PinnedImageWindowTest.cpp
)

target_include_directories(
    cappy_pinned_image_window_test
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
    cappy_pinned_image_window_test
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Test
        cappy_localization
        cappy_shortcuts
        cappy_pinboard_feature
)

cappy_configure_target(cappy_pinned_image_window_test)

add_test(
    NAME cappy_pinned_image_window_test
    COMMAND cappy_pinned_image_window_test
)
