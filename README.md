# im-c
An instant messaging toy project written in C.

## Usage Instruction
Follow the readme in both 'frontend/' and 'backend/' dir

## Testing

The project includes a comprehensive testing suite with multiple testing methodologies:

### Quick Start

Run the complete test suite with coverage analysis:

```bash
cd tests
./run_comprehensive_tests.sh
```

This will execute all tests, generate coverage reports, perform memory leak detection, and create detailed test reports.

### Test Categories

1. **Unit Tests** - Core functionality testing
   - Message parsing and serialization
   - Textbox focus management
   - Backend message handling logic

2. **Integration Tests** - Component interaction testing
   - WebSocket communication with mock server
   - Client-server message flow simulation
   - Network error handling scenarios

3. **UI Tests** - User interface component testing
   - Clay component interaction (headless)
   - Input handling and validation
   - Layout and collision detection

4. **Error Handling Tests** - Robustness and security testing
   - Input validation and sanitization
   - Buffer overflow protection
   - Memory management safety

5. **Backend Tests** - Server-side functionality
   - Message history management
   - Client session handling
   - Memory allocation/cleanup

### Manual Test Execution

Run individual test categories:

```bash
cd tests/build

# Unit tests
./test_message_types          # Message handling
./test_textbox               # Textbox functionality

# UI tests
./test_ui_components         # Basic UI logic
./test_clay_components_advanced  # Advanced Clay integration

# Integration tests
./test_websocket_integration_advanced  # Mock server tests

# Backend tests
./test_backend_components    # Server logic

# Error handling tests
./test_error_handling        # Input validation & safety
```

### Coverage Analysis

Generate detailed coverage reports:

```bash
cd tests
cmake .. -DENABLE_COVERAGE=ON
make
ctest
lcov --capture --directory . --output-file coverage.info
genhtml coverage.info --output-directory coverage_html
```

Open `coverage_html/index.html` for interactive coverage analysis.

### Memory Leak Detection

Run tests with Valgrind memory checking:

```bash
cd tests
cmake .. -DENABLE_VALGRIND=ON
make
ctest  # Includes Valgrind tests automatically
```

### Fuzz Testing

For security testing with AFL (American Fuzzy Lop):

```bash
cd tests
cmake .. -DENABLE_FUZZING=ON
make
# Run fuzzing (requires AFL installation):
# afl-fuzz -i testcases -o findings ./fuzz_message_parser
```

### Test Results

After running `./run_comprehensive_tests.sh`, find results in:
- `coverage/test_report.md` - Comprehensive test summary
- `coverage/html/index.html` - Interactive coverage report
- `valgrind_logs/` - Memory leak detection logs

### Continuous Integration

The testing framework supports:
- Automated test execution
- Coverage threshold enforcement
- Memory leak detection
- Performance profiling
- Static code analysis

See `tests/README.md` for detailed testing documentation and architecture.

## Development Status

### Testing Implementation ✅ COMPLETED
- [x] Comprehensive test suite with Unity framework
- [x] Mock WebSocket server for integration testing
- [x] Coverage reporting with gcov/lcov
- [x] Memory leak detection with Valgrind
- [x] Fuzz testing for input validation
- [x] Automated test runner with reporting
- [x] Backend unit tests for server components
- [x] UI component testing (headless Clay)
- [x] Error handling and edge case testing

## TODO

### frontend
#### textbox: nik
- [ ] manage focus (disables focus when clicking outside of the box)
- [ ] turn cursor back from IBEAM when not hovering a textbox
- [ ] integrate textbox to main page

#### connection (networking client) 
- [ ] simple authentication system (with websocket too? http) kenet bantu cek
- [ ] send message
- [ ] receive messages from server
- [ ] render messages based on username (whether its you or others thst sent the message)
- [ ] connecting to server loading, new page? handle connection retry and render in ui

#### UI
- [ ] interactivity (button color change when hovered and when pressed)
- [ ] font sizing (if possible, responsive based on window size) 

### backend
- [ ] implement system authentication system for handling username, i. e. handle how the server will name connecting clients, understand how the server differentiate between connected clients

##### Note
- [ ] serverside bakal ribetnya nanti pas lanjut roadmap, gmn cara manage multiple messages dr org ke org, simpen chat kevin ke ben, kevin ke steven, Steve ke ben, group chat

## ROADMAP
- [ ] personal chats
- [ ] group chats
- [ ] persistent storage with db
