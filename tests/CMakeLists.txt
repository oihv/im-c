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
    ${PROJECT_SOURCE_DIR}
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

add_executable(test_clay_components_advanced
    ui/test_clay_components_advanced.c
    ${unity_SOURCE_DIR}/src/unity.c
)

# Integration Tests
add_executable(test_websocket_integration
    integration/test_websocket_integration.c
    ${PROJECT_SOURCE_DIR}/frontend/network/websocket_service.c
    ${PROJECT_SOURCE_DIR}/frontend/network/message_types.c
    ${unity_SOURCE_DIR}/src/unity.c
)

add_executable(test_websocket_integration_advanced
    integration/test_websocket_integration_advanced.c
    integration/mock_websocket_server.c
    ${PROJECT_SOURCE_DIR}/frontend/network/websocket_service.c
    ${PROJECT_SOURCE_DIR}/frontend/network/message_types.c
    ${unity_SOURCE_DIR}/src/unity.c
)

# Backend Tests
add_executable(test_backend_components
    backend/test_backend_components.c
    ${unity_SOURCE_DIR}/src/unity.c
)

# Error Handling Tests
add_executable(test_error_handling
    edge_cases/test_error_handling.c
    ${unity_SOURCE_DIR}/src/unity.c
)

# Add compiler flags for debugging and coverage
set(TEST_COMPILE_FLAGS "-g;-O0;-DUNITY_INCLUDE_DOUBLE;--coverage")
set(TEST_LINK_FLAGS "--coverage")

target_compile_options(test_message_types PRIVATE ${TEST_COMPILE_FLAGS})
target_compile_options(test_textbox PRIVATE ${TEST_COMPILE_FLAGS})
target_compile_options(test_ui_components PRIVATE ${TEST_COMPILE_FLAGS})
target_compile_options(test_clay_components_advanced PRIVATE ${TEST_COMPILE_FLAGS})
target_compile_options(test_websocket_integration PRIVATE ${TEST_COMPILE_FLAGS})
target_compile_options(test_websocket_integration_advanced PRIVATE ${TEST_COMPILE_FLAGS})
target_compile_options(test_backend_components PRIVATE ${TEST_COMPILE_FLAGS})
target_compile_options(test_error_handling PRIVATE ${TEST_COMPILE_FLAGS})

target_link_options(test_message_types PRIVATE ${TEST_LINK_FLAGS})
target_link_options(test_textbox PRIVATE ${TEST_LINK_FLAGS})
target_link_options(test_ui_components PRIVATE ${TEST_LINK_FLAGS})
target_link_options(test_clay_components_advanced PRIVATE ${TEST_LINK_FLAGS})
target_link_options(test_websocket_integration PRIVATE ${TEST_LINK_FLAGS})
target_link_options(test_websocket_integration_advanced PRIVATE ${TEST_LINK_FLAGS})
target_link_options(test_backend_components PRIVATE ${TEST_LINK_FLAGS})
target_link_options(test_error_handling PRIVATE ${TEST_LINK_FLAGS})

# Link libraries for integration tests that need libwebsockets
target_link_libraries(test_websocket_integration ${LIBWEBSOCKETS_LIBRARIES})
target_link_libraries(test_websocket_integration_advanced ${LIBWEBSOCKETS_LIBRARIES})

# Register tests with CTest
add_test(NAME MessageTypesTest COMMAND test_message_types)
add_test(NAME TextboxTest COMMAND test_textbox)
add_test(NAME UIComponentsTest COMMAND test_ui_components)
add_test(NAME ClayComponentsAdvancedTest COMMAND test_clay_components_advanced)
add_test(NAME WebSocketIntegrationTest COMMAND test_websocket_integration)
add_test(NAME WebSocketIntegrationAdvancedTest COMMAND test_websocket_integration_advanced)
add_test(NAME BackendComponentsTest COMMAND test_backend_components)
add_test(NAME ErrorHandlingTest COMMAND test_error_handling)

# Test coverage (enabled by default with gcov)
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID MATCHES "Clang")
    option(ENABLE_COVERAGE "Enable coverage reporting" ON)
    if(ENABLE_COVERAGE)
        message(STATUS "Coverage reporting enabled")
    endif()
endif()

# Memory leak detection with Valgrind
option(ENABLE_VALGRIND "Enable Valgrind memory leak detection" OFF)
if(ENABLE_VALGRIND)
    find_program(VALGRIND_EXECUTABLE valgrind)
    if(VALGRIND_EXECUTABLE)
        message(STATUS "Valgrind found: ${VALGRIND_EXECUTABLE}")
        
        # Add Valgrind tests
        add_test(NAME MessageTypesTest_Valgrind 
                 COMMAND ${VALGRIND_EXECUTABLE} --tool=memcheck --leak-check=full --error-exitcode=1 
                 $<TARGET_FILE:test_message_types>)
        add_test(NAME BackendComponentsTest_Valgrind 
                 COMMAND ${VALGRIND_EXECUTABLE} --tool=memcheck --leak-check=full --error-exitcode=1 
                 $<TARGET_FILE:test_backend_components>)
        add_test(NAME ErrorHandlingTest_Valgrind 
                 COMMAND ${VALGRIND_EXECUTABLE} --tool=memcheck --leak-check=full --error-exitcode=1 
                 $<TARGET_FILE:test_error_handling>)
    else()
        message(WARNING "Valgrind not found, skipping memory leak detection tests")
    endif()
endif()

# Fuzzing tests (requires AFL or similar)
option(ENABLE_FUZZING "Enable fuzz testing" OFF)
if(ENABLE_FUZZING)
    find_program(AFL_CC afl-gcc)
    if(AFL_CC)
        message(STATUS "AFL found: ${AFL_CC}")
        
        # Add fuzz test executable
        add_executable(fuzz_message_parser
            fuzz/fuzz_message_parser.c
            ${PROJECT_SOURCE_DIR}/frontend/network/message_types.c
        )
        
        set_target_properties(fuzz_message_parser PROPERTIES
            COMPILE_FLAGS "-g -O0"
            LINK_FLAGS "-g"
        )
        
        # Note: Actual fuzzing would be run manually with: afl-fuzz -i testcases -o findings ./fuzz_message_parser
        
    else()
        message(WARNING "AFL not found, skipping fuzz testing")
    endif()
endif()