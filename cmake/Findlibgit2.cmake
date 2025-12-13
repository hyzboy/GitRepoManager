# Findlibgit2.cmake
# Find libgit2 library
#
# This module defines:
#  libgit2_FOUND - system has libgit2
#  libgit2_INCLUDE_DIRS - the libgit2 include directories
#  libgit2_LIBRARIES - link these to use libgit2
#  libgit2::libgit2 - imported target

# Try to find via pkg-config first
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(PC_LIBGIT2 QUIET libgit2)
endif()

# Find include directory
find_path(libgit2_INCLUDE_DIR
    NAMES git2.h
    HINTS
        ${PC_LIBGIT2_INCLUDEDIR}
        ${PC_LIBGIT2_INCLUDE_DIRS}
    PATH_SUFFIXES git2
)

# Find library
find_library(libgit2_LIBRARY
    NAMES git2 libgit2
    HINTS
        ${PC_LIBGIT2_LIBDIR}
        ${PC_LIBGIT2_LIBRARY_DIRS}
)

# Handle standard arguments
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(libgit2
    REQUIRED_VARS
        libgit2_LIBRARY
        libgit2_INCLUDE_DIR
)

if(libgit2_FOUND)
    set(libgit2_LIBRARIES ${libgit2_LIBRARY})
    set(libgit2_INCLUDE_DIRS ${libgit2_INCLUDE_DIR})
    
    # Create imported target if not exists
    if(NOT TARGET libgit2::libgit2)
        add_library(libgit2::libgit2 UNKNOWN IMPORTED)
        set_target_properties(libgit2::libgit2 PROPERTIES
            IMPORTED_LOCATION "${libgit2_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${libgit2_INCLUDE_DIR}"
        )
        
        # Add link libraries from pkg-config if available
        if(PC_LIBGIT2_LIBRARIES)
            set_target_properties(libgit2::libgit2 PROPERTIES
                INTERFACE_LINK_LIBRARIES "${PC_LIBGIT2_LIBRARIES}"
            )
        endif()
    endif()
endif()

mark_as_advanced(
    libgit2_INCLUDE_DIR
    libgit2_LIBRARY
)
