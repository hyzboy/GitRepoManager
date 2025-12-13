# Source files
set(SOURCES
    main.cpp
)

# Find Qt
find_package(Qt6 COMPONENTS Core Widgets REQUIRED)
message(STATUS "Found Qt6 components: Core, Widgets")

# Create executable
add_executable(GitRepoManager ${SOURCES})

# Link libraries
target_link_libraries(GitRepoManager PRIVATE
    Qt6::Core
    Qt6::Widgets
)

find_package(libgit2 CONFIG REQUIRED)
message(STATUS "find_package(libgit2) completed")

# Link libgit2 (prefer imported target when available)
if(TARGET libgit2::libgit2)
    message(STATUS "Using vcpkg-configured libgit2 target 'libgit2::libgit2'")
    target_link_libraries(GitRepoManager PRIVATE libgit2::libgit2)
else()
    message(STATUS "Using fallback libgit2 target 'libgit2::libgit2package'")
    # Fallback when a different target name or pkg-config is used
    target_link_libraries(GitRepoManager PRIVATE libgit2::libgit2package)
endif()

# Set target properties
set_target_properties(GitRepoManager PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
