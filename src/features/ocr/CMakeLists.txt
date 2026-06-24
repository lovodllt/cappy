add_library(
    cappy_ocr_feature
    STATIC
        include/cappy/features/ocr/OcrResultWindow.h
        src/OcrResultWindow.cpp
)

target_include_directories(
    cappy_ocr_feature
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
    cappy_ocr_feature
    PUBLIC
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        cappy_localization
        cappy_ocr_service
)

cappy_configure_target(cappy_ocr_feature)

qt_add_executable(
    cappy_ocr_result_window_snapshot
    tests/OcrResultWindowSnapshot.cpp
)

target_include_directories(
    cappy_ocr_result_window_snapshot
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
    cappy_ocr_result_window_snapshot
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        cappy_localization
        cappy_ocr_service
        cappy_ocr_feature
)

cappy_configure_target(cappy_ocr_result_window_snapshot)
