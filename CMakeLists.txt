cmake_minimum_required(VERSION 3.28)

project(
    Cappy
    VERSION 0.1.0
    DESCRIPTION "Lightweight screenshot and pinning tool for Windows and Linux X11"
    LANGUAGES CXX
)

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CAPPY_DESKTOP_FILE "${CMAKE_CURRENT_BINARY_DIR}/resources/linux/cappy.desktop")
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/resources/linux/cappy.desktop.in"
    "${CAPPY_DESKTOP_FILE}"
    @ONLY
)

option(CAPPY_BUILD_PLUGINS "Build runtime plugins" ON)
option(CAPPY_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(Qt6 6.4 REQUIRED COMPONENTS Core Gui Widgets Network Test)
if(UNIX AND NOT APPLE)
    find_package(X11 REQUIRED)
endif()

include(cmake/ProjectOptions.cmake)
include(cmake/Packaging.cmake)
enable_testing()

add_subdirectory(src)

include(CPack)
