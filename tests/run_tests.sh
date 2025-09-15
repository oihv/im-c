#!/bin/bash

# IM-C Testing Guide
# Run this script to build and execute all tests

set -e

echo "=== IM-C Testing System ==="
echo

# Check if we're in the right directory
if [ ! -f "README.md" ] || [ ! -d "frontend" ] || [ ! -d "backend" ]; then
    echo "Error: Please run this script from the root of the im-c project"
    exit 1
fi

# Create build directory
mkdir -p tests/build

echo "Building test suite..."
cd tests

# Build tests using CMake
cmake -B build -S . 
make -C build

echo "✅ Test suite built successfully"
echo

echo "Running tests..."
echo "=================="

# Run unit tests
echo "🧪 Unit Tests:"
echo "---------------"
cd build

echo "📝 Message Types Tests:"
./test_message_types
echo

echo "📱 Textbox Component Tests:"
./test_textbox
echo

echo "🖥️  UI Component Tests:"
./test_ui_components
echo

echo "✅ All tests passed!"
echo

echo "📊 Test Summary:"
echo "- Message Types: 8 tests covering parsing, serialization, and list operations"
echo "- Textbox Component: 3 tests covering focus management" 
echo "- UI Components: 6 tests covering click detection, text input, and backspace handling"
echo
echo "💡 Tips:"
echo "- Run individual tests: cd tests/build && ./test_message_types"
echo "- Generate coverage: cmake -B build -S . -DENABLE_COVERAGE=ON"
echo "- For integration tests, implement websocket mocking in the future"