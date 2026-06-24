add_library(cappy_capture_domain INTERFACE)

target_include_directories(
    cappy_capture_domain
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
    cappy_capture_domain
    INTERFACE
        Qt6::Core
        Qt6::Gui
)

