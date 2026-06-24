add_library(cappy_plugin_api INTERFACE)
target_include_directories(
    cappy_plugin_api
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(cappy_plugin_api INTERFACE Qt6::Core)

