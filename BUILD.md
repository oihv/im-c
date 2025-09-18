# Cross-Platform Build Setup

This project now supports cross-platform static linking for both Linux and Windows.

## Quick Build Commands

### Build Everything (Linux + Windows)
```bash
./build-cross-platform.sh
```

### Build Only Linux
```bash
./build-cross-platform.sh linux
```

### Build Only Windows
```bash
./build-cross-platform.sh windows
```

### Clean Build Directories
```bash
./build-cross-platform.sh clean
```

## Prerequisites

### For Linux Building
- CMake 3.10+
- GCC
- libwebsockets-dev
- pkg-config

```bash
sudo apt install cmake gcc libwebsockets-dev pkg-config
```

### For Windows Cross-Compilation
- MinGW-w64 cross-compiler
- Windows libraries

```bash
sudo apt install mingw-w64
```

## Output

After building, you'll find:

```
dist/
├── linux/
│   ├── im_c                    # Linux frontend executable
│   ├── lws-minimal-ws-server   # Linux backend executable
│   └── resources/              # Frontend resources
├── windows/
│   ├── im_c.exe                # Windows frontend executable
│   ├── lws-minimal-ws-server.exe # Windows backend executable
│   └── resources/              # Frontend resources
├── im-c-linux-x64.tar.gz      # Linux distribution package
└── im-c-windows-x64.zip       # Windows distribution package
```

## Configuration Options

The CMake files support these options:

- `STATIC_BUILD=ON` - Enable static linking (default: ON)
- `CROSS_COMPILE_WINDOWS=ON` - Cross-compile for Windows
- `CMAKE_BUILD_TYPE=Release` - Build optimized release binaries

## Manual Building

### Linux Static Build
```bash
# Frontend
cd frontend
cmake . -B build-linux -DCMAKE_BUILD_TYPE=Release -DSTATIC_BUILD=ON
cmake --build build-linux

# Backend
cd ../backend
cmake . -B build-linux -DCMAKE_BUILD_TYPE=Release -DSTATIC_BUILD=ON
cmake --build build-linux
```

### Windows Cross-Compilation
```bash
# Frontend
cd frontend
cmake . -B build-windows \
  -DCMAKE_TOOLCHAIN_FILE=../cmake/windows-toolchain.cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -DSTATIC_BUILD=ON \
  -DCROSS_COMPILE_WINDOWS=ON
cmake --build build-windows

# Backend
cd ../backend
cmake . -B build-windows \
  -DCMAKE_TOOLCHAIN_FILE=../cmake/windows-toolchain.cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -DSTATIC_BUILD=ON \
  -DCROSS_COMPILE_WINDOWS=ON
cmake --build build-windows
```

## Distribution

The Windows `.zip` file contains everything needed to run on Windows - no additional DLLs or dependencies required due to static linking.

Simply send `im-c-windows-x64.zip` to Windows users for testing!