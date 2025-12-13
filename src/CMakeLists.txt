# Source files
set(SOURCES
    main.cpp
)

# Create executable
add_executable(GitRepoManager ${SOURCES})

# Link libraries
target_link_libraries(GitRepoManager PRIVATE
    Qt6::Core
    Qt6::Widgets
)

# Link libgit2 depending on how it was found
if(TARGET libgit2::libgit2)
    # Found via CONFIG (vcpkg)
    target_link_libraries(GitRepoManager PRIVATE libgit2::libgit2)
elseif(TARGET PkgConfig::libgit2)
    # Found via pkg-config
    target_link_libraries(GitRepoManager PRIVATE PkgConfig::libgit2)
endif()

# Set target properties
set_target_properties(GitRepoManager PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
