add_library(
    cappy_builtin_core_plugin
    MODULE
        CorePlugin.cpp
)

target_include_directories(
    cappy_builtin_core_plugin
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(
    cappy_builtin_core_plugin
    PRIVATE
        Qt6::Core
        cappy_plugin_api
)

set_target_properties(
    cappy_builtin_core_plugin
    PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/plugins"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/plugins"
)

cappy_configure_target(cappy_builtin_core_plugin)

install(
    TARGETS cappy_builtin_core_plugin
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/cappy/plugins
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/plugins
)
