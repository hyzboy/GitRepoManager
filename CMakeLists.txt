cmake_minimum_required(VERSION 3.16)

project(GitRepoManager 
    VERSION 1.0.0
    DESCRIPTION "Git Repository Manager with Qt6 and libgit2"
    LANGUAGES CXX
)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add cmake directory to module path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Platform-specific settings
if(WIN32)
    # Windows: Suggest vcpkg if not already using it
    if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND DEFINED ENV{VCPKG_ROOT})
        message(STATUS "VCPKG_ROOT environment variable detected: $ENV{VCPKG_ROOT}")
        message(STATUS "Consider using: -DCMAKE_TOOLCHAIN_FILE=$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
    endif()
elseif(APPLE)
    # macOS: Suggest Homebrew Qt path if not using vcpkg
    if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND NOT DEFINED CMAKE_PREFIX_PATH)
        execute_process(
            COMMAND brew --prefix qt@6
            OUTPUT_VARIABLE HOMEBREW_QT_PATH
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )
        if(HOMEBREW_QT_PATH)
            message(STATUS "Homebrew Qt6 detected at: ${HOMEBREW_QT_PATH}")
            message(STATUS "Consider using: -DCMAKE_PREFIX_PATH=${HOMEBREW_QT_PATH}")
            list(APPEND CMAKE_PREFIX_PATH ${HOMEBREW_QT_PATH})
        endif()
    endif()
endif()

# Qt6 Components
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# libgit2 - Try CONFIG first (for vcpkg), then fall back to PkgConfig
find_package(libgit2 CONFIG QUIET)
if(NOT libgit2_FOUND)
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(libgit2 IMPORTED_TARGET libgit2)
    endif()
endif()

if(NOT libgit2_FOUND AND NOT TARGET PkgConfig::libgit2)
    message(FATAL_ERROR "libgit2 not found. Please install libgit2 via:\n"
        "  - Windows: vcpkg install libgit2\n"
        "  - Linux/BSD: system package manager (libgit2-dev)\n"
        "  - macOS: brew install libgit2")
endif()

# Enable automoc for Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Add subdirectories
add_subdirectory(src)

# Installation rules
install(TARGETS GitRepoManager
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
