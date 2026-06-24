add_library(
    cappy_pinboard_service
    STATIC
        include/cappy/services/pinboard/PinboardManager.h
        src/PinboardManager.cpp
)

target_include_directories(
    cappy_pinboard_service
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
    cappy_pinboard_service
    PUBLIC
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        cappy_localization
        cappy_shortcuts
        cappy_pinboard_feature
)

cappy_configure_target(cappy_pinboard_service)
