add_library(
    cappy_capture_platform
    STATIC
        src/DesktopCaptureBackendFactory.cpp
        src/QtScreenCaptureBackend.cpp
        src/UnsupportedCaptureBackend.cpp
)

target_include_directories(
    cappy_capture_platform
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
    cappy_capture_platform
    PUBLIC
        Qt6::Core
        Qt6::Gui
        cappy_capture_domain
)

if(UNIX AND NOT APPLE)
    target_link_libraries(
        cappy_capture_platform
        PUBLIC
            xcb
    )
endif()

cappy_configure_target(cappy_capture_platform)
