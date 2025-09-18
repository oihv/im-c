cmake_minimum_required(VERSION 3.27)
project(im_c C)
set(CMAKE_C_STANDARD 99)

# Cross-platform static linking configuration
option(STATIC_BUILD "Build with static linking" ON)
option(CROSS_COMPILE_WINDOWS "Cross-compile for Windows" OFF)

if(STATIC_BUILD)
    set(CMAKE_FIND_LIBRARY_SUFFIXES .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
    if(WIN32 OR CROSS_COMPILE_WINDOWS)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")
    else()
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
    endif()
endif()

# Adding Raylib
include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE) # don't build the supplied example games

# Check if we have local raylib, otherwise fetch from git
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/raylib/CMakeLists.txt")
    message(STATUS "Using local raylib from third_party/raylib")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../third_party/raylib raylib)
else()
    message(STATUS "Fetching raylib from GitHub")
    FetchContent_Declare(
        raylib
        GIT_REPOSITORY "https://github.com/raysan5/raylib.git"
        GIT_TAG "5.5"
        GIT_PROGRESS TRUE
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(raylib)
endif()


# Cross-platform dependency finding
if(WIN32 OR CROSS_COMPILE_WINDOWS)
    # For Windows cross-compilation, try to find libwebsockets
    find_library(WEBSOCKETS_LIB 
        NAMES libwebsockets websockets
        PATHS /usr/x86_64-w64-mingw32/lib
        NO_DEFAULT_PATH
    )
    
    if(WEBSOCKETS_LIB)
        set(LIBWEBSOCKETS_LIBRARIES ${WEBSOCKETS_LIB})
        set(LIBWEBSOCKETS_INCLUDE_DIRS /usr/x86_64-w64-mingw32/include)
        message(STATUS "Found Windows libwebsockets: ${WEBSOCKETS_LIB}")
    else()
        message(WARNING "libwebsockets not found for Windows. Building without networking support.")
        set(LIBWEBSOCKETS_LIBRARIES "")
        set(LIBWEBSOCKETS_INCLUDE_DIRS "")
        add_definitions(-DDISABLE_NETWORKING)
    endif()
    
    # Windows-specific libraries for static linking
    set(PLATFORM_LIBS ws2_32 winmm kernel32 user32 gdi32 winspool shell32 ole32 oleaut32 uuid comdlg32 advapi32)
else()
    # Linux - use pkg-config to find libwebsockets
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBWEBSOCKETS REQUIRED libwebsockets)
endif()

add_executable(im_c 
        main.c
        network/websocket_service.c
        network/message_types.c
)

target_compile_options(im_c PUBLIC 
# ${LIBWEBSOCKETS_CFLAGS_OTHER}
)
target_include_directories(im_c PUBLIC 
        .
        ${LIBWEBSOCKETS_INCLUDE_DIRS}
)

target_link_libraries(im_c PUBLIC 
        raylib
        ${LIBWEBSOCKETS_LIBRARIES}
        ${PLATFORM_LIBS}
        # ${LIBUV_LIBRARIES}
)

target_link_directories(im_c PUBLIC 
        raylib
        ${LIBWEBSOCKETS_LIBRARY_DIRS}
)

# Platform-specific build configuration
if(MSVC)
  set(CMAKE_C_FLAGS_DEBUG "/D CLAY_DEBUG /fsanitize=address")
elseif(NOT (WIN32 OR CROSS_COMPILE_WINDOWS))
  # Linux debug settings (not for cross-compilation)
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DCLAY_DEBUG -g -fsanitize=address -DGLFW_BUILD_WAYLAND=ON")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address")
  set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -fsanitize=address")
endif()

add_custom_command(
        TARGET im_c POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/resources
        ${CMAKE_CURRENT_BINARY_DIR}/resources)

# Windows-specific handling (only for native Windows builds, not cross-compilation)
if(WIN32 AND NOT CROSS_COMPILE_WINDOWS)
    # Copy websockets.dll to output directory after build
    # Alternative: if websockets.dll is in a different location, find it
    find_file(WEBSOCKETS_DLL 
        NAMES websockets.dll libwebsockets.dll
        PATHS 
            "${CMAKE_BINARY_DIR}"
            "${CMAKE_PREFIX_PATH}/bin"
            "C:/vcpkg/installed/x64-windows/bin"
        PATH_SUFFIXES bin
    )
    
    if(WEBSOCKETS_DLL)
        add_custom_command(TARGET im_c POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${WEBSOCKETS_DLL}"
            $<TARGET_FILE_DIR:im_c>
        )
    endif()
endif()
