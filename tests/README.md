# Testing Documentation for IM-C

## Overview

This document describes the comprehensive testing strategy for the IM-C instant messaging application written in C. The testing framework covers unit tests, integration tests, and UI component testing.

## Testing Framework

We use **Unity** as our primary testing framework for C, with the following testing layers:

### 1. Unit Tests (`tests/unit/`)

Test individual functions and modules in isolation:

- **Message Types** (`test_message_types.c`): Tests message parsing, serialization, and list operations
- **Textbox Components** (`test_textbox.c`): Tests UI textbox focus management

### 2. Integration Tests (`tests/integration/`)

Test component interactions:

- **Websocket Integration** (`test_websocket_integration.c`): Tests websocket client-server communication

### 3. UI Tests (`tests/ui/`)

Test UI component logic without rendering:

- **UI Components** (`test_ui_components.c`): Tests textbox interactions, click handling, and input processing

## Setup and Build

### Prerequisites

```bash
# Install required dependencies
sudo apt-get install build-essential cmake pkg-config libwebsockets-dev

# Or on macOS
brew install cmake pkg-config libwebsockets
```

### Building Tests

```bash
# From the project root
cd tests
cmake -B build -S .
make -C build
```

### Running Tests

```bash
# Run all tests
./run_tests.sh

# Run individual test suites
cd tests/build
./test_message_types      # Unit tests for message handling
./test_textbox           # Unit tests for textbox components  
./test_websocket_integration  # Integration tests (requires server)
./test_ui_components     # UI component logic tests
```

## Test Coverage

To generate test coverage reports:

```bash
cmake -B build -S . -DENABLE_COVERAGE=ON
make -C build
# Run tests
gcov build/CMakeFiles/test_*.dir/*.gcno
```

## Testing Strategy by Component

### Message Types Testing

Tests the core messaging functionality:

- **Parsing**: Validates pipe-delimited message format parsing
- **Serialization**: Ensures messages can be converted back to string format
- **List Operations**: Tests message list creation, addition, overflow protection
- **Edge Cases**: Invalid formats, memory management

Example test:
```c
void test_message_parse_from_string_valid_chat_message(void) {
    const char* pipe_delimited_str = "0|1234567890|test_user|Hello, World!|";
    Message message;
    
    bool result = message_parse_from_string(pipe_delimited_str, &message);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_STRING("test_user", message.username);
    TEST_ASSERT_EQUAL_STRING("Hello, World!", message.content);
}
```

### Websocket Integration Testing

Tests client-server communication:

- **Connection Management**: Create, connect, disconnect websocket clients
- **Message Sending**: Queue and send messages to server
- **State Management**: Track connection states
- **Error Handling**: Invalid URLs, connection failures

**Note**: Integration tests require the websocket server to be running:
```bash
cd backend && ./lws-minimal-ws-server
```

### UI Component Testing

Tests UI logic without actual rendering (headless testing):

- **Focus Management**: Textbox focus behavior
- **Click Detection**: Point-in-bounding-box calculations
- **Text Input**: Character input and buffer management
- **Keyboard Handling**: Backspace, special keys

Example test:
```c
void test_handleTextInput(void) {
    size_t len = 0;
    char buffer[10] = "";
    Component_TextBoxData textbox = {
        .buffer = buffer,
        .maxLen = 10,
        .len = &len
    };
    
    handleTextInput(&textbox, 'H');
    handleTextInput(&textbox, 'i');
    
    TEST_ASSERT_EQUAL_STRING("Hi", buffer);
    TEST_ASSERT_EQUAL_INT(2, len);
}
```

## UI Testing Approach

Since Clay is a immediate-mode UI library, we test the UI logic separately from rendering:

1. **Mock Clay Structures**: Create lightweight versions of Clay types
2. **Test UI Logic**: Focus on interaction logic, state management
3. **Boundary Testing**: Test edge cases like buffer overflows, invalid inputs
4. **Event Simulation**: Simulate mouse clicks, keyboard input

This approach allows testing UI behavior without requiring:
- Graphics initialization
- Window creation
- Actual rendering

## Continuous Integration

For CI/CD pipelines, add these steps:

```bash
# In your CI script
- name: Run Tests
  run: |
    cd tests
    cmake -B build -S . -DENABLE_COVERAGE=ON
    make -C build
    ./run_tests.sh
    
- name: Generate Coverage Report
  run: |
    cd tests/build
    gcov CMakeFiles/test_*.dir/*.gcno
    lcov --capture --directory . --output-file coverage.info
```

## Best Practices

1. **Test Isolation**: Each test should be independent and clean up after itself
2. **Meaningful Names**: Test function names should clearly describe what they test
3. **Edge Cases**: Always test boundary conditions and error cases
4. **Mock External Dependencies**: Use mocks for external services (websockets, file I/O)
5. **Fast Execution**: Keep unit tests fast; integration tests can be slower
6. **Regular Execution**: Run tests frequently during development

## Debugging Tests

To debug failing tests:

```bash
# Compile with debug symbols
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug
make -C build

# Run with gdb
gdb ./build/test_message_types
(gdb) run
(gdb) bt  # Print backtrace on failure
```

## Adding New Tests

1. **For Unit Tests**: Add new test files in `tests/unit/`
2. **For Integration Tests**: Add to `tests/integration/`
3. **Update CMakeLists.txt**: Add new test executables
4. **Update run_tests.sh**: Include new tests in the test runner

Example new test addition:
```cmake
add_executable(test_new_component
    unit/test_new_component.c
    ${PROJECT_SOURCE_DIR}/frontend/new_component.c
    ${unity_SOURCE_DIR}/src/unity.c
)
add_test(NAME NewComponentTest COMMAND test_new_component)
```

## Known Limitations

1. **Integration Tests**: Require manual server startup
2. **UI Tests**: Don't test actual rendering, only logic
3. **Platform-Specific**: Some tests may behave differently on Windows vs Linux
4. **Memory Leaks**: Consider adding valgrind checks for memory leak detection

## Future Improvements

- [ ] Add automated memory leak detection with valgrind
- [ ] Implement mock websocket server for integration tests
- [ ] Add performance benchmarking tests
- [ ] Consider property-based testing for message parsing
- [ ] Add test report generation (JUnit XML format)