cmake_minimum_required(VERSION 3.27)
project(im_c C)
set(CMAKE_C_STANDARD 99)

# Adding Raylib
include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE) # don't build the supplied example games

FetchContent_Declare(
    raylib
    GIT_REPOSITORY "https://github.com/raysan5/raylib.git"
    GIT_TAG "5.5"
    GIT_PROGRESS TRUE
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(raylib)


add_executable(im_c 
        main.c
        network/websocket_service.c
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
        # ${LIBUV_LIBRARIES}
)

target_link_directories(im_c PUBLIC 
        raylib
        ${LIBWEBSOCKETS_LIBRARY_DIRS}
)

# Adding address sanitizer, delete when compiling for release
if(MSVC)
  set(CMAKE_C_FLAGS_DEBUG "/D CLAY_DEBUG /fsanitize=address")
else()
# Find libwebsockets
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(LIBWEBSOCKETS REQUIRED libwebsockets)

# Find libwebsockets
  pkg_check_modules(LIBWEBSOCKETS REQUIRED libwebsockets)
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DCLAY_DEBUG -g -fsanitize=address -DGLFW_BUILD_WAYLAND=ON")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address")
  set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -fsanitize=address")
endif()

add_custom_command(
        TARGET im_c POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/resources
        ${CMAKE_CURRENT_BINARY_DIR}/resources)

        # Add this after your target_link_libraries section
if(WIN32)
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
