cmake_minimum_required(VERSION 3.27)
project(im_c_tests C)

# Enable testing
enable_testing()

# Add Unity testing framework
include(FetchContent)

FetchContent_Declare(
    unity
    GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
    GIT_TAG v2.5.2
    GIT_PROGRESS TRUE
)

FetchContent_Declare(
    cmocka
    GIT_REPOSITORY https://git.cryptomilk.org/projects/cmocka.git
    GIT_TAG cmocka-1.1.7
    GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(unity cmocka)

# Find libwebsockets for integration tests
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBWEBSOCKETS REQUIRED libwebsockets)

# Test configuration
set(TEST_SOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)

# Include directories
include_directories(
    ${PROJECT_SOURCE_DIR}/frontend
    ${PROJECT_SOURCE_DIR}/frontend/network
    ${PROJECT_SOURCE_DIR}/frontend/components
    ${unity_SOURCE_DIR}/src
    ${cmocka_SOURCE_DIR}/include
    ${LIBWEBSOCKETS_INCLUDE_DIRS}
)

# Unit Tests
add_executable(test_message_types
    unit/test_message_types.c
    ${PROJECT_SOURCE_DIR}/frontend/network/message_types.c
    ${unity_SOURCE_DIR}/src/unity.c
)

add_executable(test_textbox
    unit/test_textbox.c
    ${unity_SOURCE_DIR}/src/unity.c
)

# UI Tests (headless Clay testing)
add_executable(test_ui_components
    ui/test_ui_components.c
    ${unity_SOURCE_DIR}/src/unity.c
)

# Add compiler flags for debugging
target_compile_options(test_message_types PRIVATE -g -O0 -DUNITY_INCLUDE_DOUBLE)
target_compile_options(test_textbox PRIVATE -g -O0 -DUNITY_INCLUDE_DOUBLE)
target_compile_options(test_ui_components PRIVATE -g -O0 -DUNITY_INCLUDE_DOUBLE)

# Register tests with CTest
add_test(NAME MessageTypesTest COMMAND test_message_types)
add_test(NAME TextboxTest COMMAND test_textbox)
add_test(NAME UIComponentsTest COMMAND test_ui_components)

# Test coverage (optional)
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID MATCHES "Clang")
    option(ENABLE_COVERAGE "Enable coverage reporting" OFF)
    if(ENABLE_COVERAGE)
        add_compile_options(--coverage)
        add_link_options(--coverage)
    endif()
endif()