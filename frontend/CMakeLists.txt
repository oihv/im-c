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

# Find libwebsockets
# find_package(PkgConfig REQUIRED)
# pkg_check_modules(LIBWEBSOCKETS REQUIRED libwebsockets)

add_executable(im_c 
        main.c
        network/websocket_service.c
)

target_compile_options(im_c PUBLIC 
# ${LIBWEBSOCKETS_CFLAGS_OTHER}
)
target_include_directories(im_c PUBLIC 
        .
        # ${LIBWEBSOCKETS_INCLUDE_DIRS}
)

target_link_libraries(im_c PUBLIC 
        raylib
        # ${LIBWEBSOCKETS_LIBRARIES}
)

target_link_directories(im_c PUBLIC 
        raylib
        # ${LIBWEBSOCKETS_LIBRARY_DIRS}
)

if(MSVC)
  set(CMAKE_C_FLAGS_DEBUG "/D CLAY_DEBUG")
else()
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
endif()

add_custom_command(
        TARGET im_c POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/resources
        ${CMAKE_CURRENT_BINARY_DIR}/resources)
