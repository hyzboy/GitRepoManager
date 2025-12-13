# Building GitRepoManager

This document describes how to build GitRepoManager on different platforms.

## Prerequisites

### Windows (vcpkg)

1. **Install vcpkg** (if not already installed):
   ```cmd
   git clone https://github.com/Microsoft/vcpkg.git
   cd vcpkg
   .\bootstrap-vcpkg.bat
   ```

2. **Set environment variable** (optional but recommended):
   ```cmd
   set VCPKG_ROOT=C:\path\to\vcpkg
   ```

3. **Install dependencies**:
   ```cmd
   .\vcpkg install qt6-base qt6-widgets libgit2
   ```
   
   Or for 64-bit specifically:
   ```cmd
   .\vcpkg install qt6-base:x64-windows qt6-widgets:x64-windows libgit2:x64-windows
   ```

### Linux

#### Ubuntu/Debian
```bash
sudo apt update
sudo apt install build-essential cmake ninja-build
sudo apt install qt6-base-dev libqt6widgets6 libgit2-dev pkg-config
```

#### Fedora/RHEL/CentOS
```bash
sudo dnf install cmake ninja-build gcc-c++
sudo dnf install qt6-qtbase-devel libgit2-devel pkg-config
```

#### Arch Linux
```bash
sudo pacman -S cmake ninja gcc
sudo pacman -S qt6-base libgit2 pkg-config
```

### BSD

#### FreeBSD
```bash
sudo pkg install cmake ninja
sudo pkg install qt6-base libgit2 pkgconf
```

#### OpenBSD
```bash
doas pkg_add cmake ninja
doas pkg_add qt6 libgit2 pkgconf
```

## Building

### Windows with vcpkg

Using **Visual Studio** (MSVC):

```cmd
mkdir build
cd build
cmake -G "Visual Studio 17 2022" -A x64 ^
    -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake ^
    ..
cmake --build . --config Release
```

Using **Ninja**:

```cmd
mkdir build
cd build
cmake -G Ninja ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake ^
    ..
cmake --build .
```

### Linux/BSD

Using **Ninja** (recommended):

```bash
mkdir build
cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

Using **Unix Makefiles**:

```bash
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j$(nproc)
```

## Running

After building, the executable will be located in:
- Windows: `build\src\Release\GitRepoManager.exe` (MSVC) or `build\src\GitRepoManager.exe` (Ninja)
- Linux/BSD: `build/src/GitRepoManager`

### Windows
```cmd
cd build\src\Release
.\GitRepoManager.exe
```

Or with Ninja:
```cmd
cd build\src
.\GitRepoManager.exe
```

### Linux/BSD
```bash
./build/src/GitRepoManager
```

## Installation

To install the application system-wide:

```bash
# In the build directory
cmake --install . --prefix /usr/local
```

Or on Windows with administrator privileges:
```cmd
cmake --install . --prefix "C:\Program Files\GitRepoManager"
```

## Troubleshooting

### Windows: vcpkg not found

If CMake cannot find your vcpkg installation, make sure to:
1. Set the `VCPKG_ROOT` environment variable, or
2. Explicitly pass the toolchain file: `-DCMAKE_TOOLCHAIN_FILE=C:\path\to\vcpkg\scripts\buildsystems\vcpkg.cmake`

### Linux: Qt6 not found

Some distributions may have Qt6 in non-standard locations. You can help CMake find Qt6:
```bash
cmake -DCMAKE_PREFIX_PATH=/path/to/qt6 ..
```

### libgit2 not found

Make sure libgit2 is installed:
- **Windows**: Install via vcpkg: `vcpkg install libgit2`
- **Linux**: Install development package (e.g., `libgit2-dev`, `libgit2-devel`)
- **BSD**: Install via pkg/pkg_add

## Optional: Using vcpkg on Linux/BSD

While not required, you can also use vcpkg on Linux/BSD:

1. Install vcpkg:
   ```bash
   git clone https://github.com/Microsoft/vcpkg.git
   cd vcpkg
   ./bootstrap-vcpkg.sh
   ```

2. Install dependencies:
   ```bash
   ./vcpkg install qt6-base qt6-widgets libgit2
   ```

3. Build with vcpkg toolchain:
   ```bash
   mkdir build && cd build
   cmake -G Ninja \
       -DCMAKE_BUILD_TYPE=Release \
       -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake \
       ..
   cmake --build .
   ```

## Development

For development, you may want to use:
- **Visual Studio Code** with CMake Tools extension
- **CLion** (has native CMake support)
- **Qt Creator** (can import CMake projects)

All these IDEs can work with the CMake configuration directly.
