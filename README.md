# GitRepoManager

A cross-platform Git repository manager built with Qt6 and libgit2.

## Features

- Built with modern C++17
- Cross-platform support (Windows, Linux, BSD, macOS)
- Qt6-based user interface
- Powered by libgit2 for Git operations

## Building

This project uses CMake as its build system and supports multiple platforms:

- **Windows**: Uses vcpkg for dependency management
- **Linux/BSD**: Uses system package managers
- **macOS**: Uses Homebrew or vcpkg

For detailed build instructions, see [BUILDING.md](BUILDING.md).

### Quick Start

#### Windows
```cmd
vcpkg install qt6-base qt6-widgets libgit2
mkdir build && cd build
cmake -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake ..
cmake --build . --config Release
```

#### Linux
```bash
# Ubuntu/Debian
sudo apt install qt6-base-dev libqt6widgets6 libgit2-dev cmake ninja-build

# Build
mkdir build && cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

#### macOS
```bash
# Install dependencies via Homebrew
brew install cmake ninja qt@6 libgit2

# Build
mkdir build && cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH=$(brew --prefix qt@6) ..
cmake --build .
```

## Dependencies

- **Qt6** (Core, Widgets)
- **libgit2** (Git operations)
- **CMake** 3.16 or higher
- **C++17** compatible compiler

## License

See LICENSE file for details.

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues.
