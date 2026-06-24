add_library(
    cappy_capture_service
    STATIC
        include/cappy/services/capture/CaptureCoordinator.h
        src/CaptureCoordinator.cpp
)

target_include_directories(
    cappy_capture_service
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
    cappy_capture_service
    PUBLIC
        Qt6::Core
        Qt6::Gui
        cappy_localization
        cappy_shortcuts
        cappy_capture_domain
        cappy_capture_feature
        cappy_capture_platform
)

cappy_configure_target(cappy_capture_service)
