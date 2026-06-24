add_library(
    cappy_ocr_service
    STATIC
        include/cappy/services/ocr/OcrService.h
        include/cappy/services/ocr/OcrTypes.h
        src/OcrService.cpp
)

target_include_directories(
    cappy_ocr_service
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
    cappy_ocr_service
    PUBLIC
        Qt6::Core
        Qt6::Gui
        Qt6::Network
)

cappy_configure_target(cappy_ocr_service)

qt_add_executable(
    cappy_ocr_service_test
    tests/OcrServiceTest.cpp
)

target_include_directories(
    cappy_ocr_service_test
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
    cappy_ocr_service_test
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Test
        cappy_ocr_service
)

cappy_configure_target(cappy_ocr_service_test)

add_test(
    NAME cappy_ocr_service_test
    COMMAND cappy_ocr_service_test
)
