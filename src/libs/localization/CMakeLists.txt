add_library(
    cappy_localization
    STATIC
        include/cappy/localization/Localization.h
        src/Localization.cpp
)

target_include_directories(
    cappy_localization
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
    cappy_localization
    PUBLIC
        Qt6::Core
        cappy_shortcuts
)

cappy_configure_target(cappy_localization)
