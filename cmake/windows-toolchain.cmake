# Windows cross-compilation toolchain for MinGW-w64
# Based on libwebsockets cross-compilation toolchain for proper networking support

set(CROSS_PATH /usr/bin)

# Target operating system name
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# Force static libraries for better distribution
set(BUILD_SHARED_LIBS OFF)

# Cross-compiler tools
set(CMAKE_C_COMPILER "${CROSS_PATH}/x86_64-w64-mingw32-gcc")
set(CMAKE_CXX_COMPILER "${CROSS_PATH}/x86_64-w64-mingw32-g++")
set(CMAKE_RC_COMPILER "${CROSS_PATH}/x86_64-w64-mingw32-windres")
set(CMAKE_AR "${CROSS_PATH}/x86_64-w64-mingw32-ar")
set(CMAKE_RANLIB "${CROSS_PATH}/x86_64-w64-mingw32-ranlib")

# Compiler flags - suppress warnings as errors for cross-compilation compatibility
set(CMAKE_C_FLAGS "-Wno-error -static-libgcc")
set(CMAKE_CXX_FLAGS "-Wno-error -static-libgcc -static-libstdc++")

# Override release optimization to a sane value for cross-compilation
# Some gcc versions enable broken optimizations with -O3
if (CMAKE_BUILD_TYPE MATCHES RELEASE OR CMAKE_BUILD_TYPE MATCHES Release OR CMAKE_BUILD_TYPE MATCHES release)
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")
endif()

# Target environment paths - look in both MinGW sysroot and cross-tools
set(CMAKE_FIND_ROOT_PATH /usr/x86_64-w64-mingw32 ${CROSS_PATH})

# Adjust the default behavior of the FIND_XXX() commands:
# search programs in the host environment only
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# Search headers and libraries in the target environment only
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Static linking flags for Windows - ensure fully static executables
set(CMAKE_EXE_LINKER_FLAGS_INIT "-static -static-libgcc -static-libstdc++")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "-static -static-libgcc -static-libstdc++")

# Windows-specific definitions
add_definitions(-DWIN32 -D_WIN32 -D__WIN32__ -DWINVER=0x0601 -D_WIN32_WINNT=0x0601)

# Prefer static libraries for better distribution
set(CMAKE_FIND_LIBRARY_SUFFIXES .a .lib .dll.a)