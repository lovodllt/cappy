add_library(
    cappy_editor_feature
    STATIC
        include/cappy/features/editor/CaptureEditorWindow.h
        src/CaptureEditorWindow.cpp
)

target_include_directories(
    cappy_editor_feature
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
    cappy_editor_feature
    PUBLIC
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        cappy_localization
        cappy_shortcuts
)

cappy_configure_target(cappy_editor_feature)
