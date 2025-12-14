# Source files
set(SOURCES
    main.cpp
    MainWindow.cpp
    CommitsDockWidget.cpp
    CommitDiffWidget.cpp
    RepositoryTreeWidget.cpp
    CommitDetailWidget.cpp
    DLG_About.cpp
)

set(SyntaxHighlighter_SOURCES
    SyntaxHighlighter.cpp
    SyntaxLoader.cpp
    ThemeLoader.cpp

    SyntaxManager.cpp
    ThemeManager.cpp
)

source_group("SyntaxHighlighter" FILES ${SyntaxHighlighter_SOURCES})

# Find Qt
find_package(Qt6 COMPONENTS Core Widgets Xml REQUIRED)
message(STATUS "Found Qt6 components: Core, Widgets")

# Create executable
add_executable(GitRepoManager ${SOURCES} ${SyntaxHighlighter_SOURCES})

# Link libraries
target_link_libraries(GitRepoManager PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Xml
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
