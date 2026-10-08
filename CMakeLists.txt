cmake_minimum_required(VERSION 3.19)
project(qutIM VERSION 0.4.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

include(GNUInstallDirs)

# Plugins check QUTIM_VERSION_STRING against the core with an exact
# comparison, so everything is built from this single version
set(QUTIM_VERSION_STRING "${PROJECT_VERSION}")

# Same layout as the installed tree, so qutim runs straight from the build
# directory: bin/qutim finds ../lib and ../share/apps/qutim
set(QUTIM_BIN_DIR    "bin")
set(QUTIM_LIB_DIR    "lib")
set(QUTIM_PLUGIN_DIR "lib/qutim/plugins")
set(QUTIM_SHARE_DIR  "share/apps/qutim")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${QUTIM_BIN_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${QUTIM_LIB_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${QUTIM_LIB_DIR}")
set(CMAKE_INSTALL_RPATH "$ORIGIN/../${QUTIM_LIB_DIR}")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network Qml Quick)

add_subdirectory(src/3rdparty)
add_subdirectory(src/lib)
add_subdirectory(src/bin)
add_subdirectory(src/plugins)
add_subdirectory(src/share)
