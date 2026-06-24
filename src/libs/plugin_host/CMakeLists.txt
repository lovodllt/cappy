add_library(
    cappy_plugin_host
    STATIC
        src/PluginManager.cpp
)

target_include_directories(
    cappy_plugin_host
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
    cappy_plugin_host
    PUBLIC
        Qt6::Core
        cappy_plugin_api
)

cappy_configure_target(cappy_plugin_host)

