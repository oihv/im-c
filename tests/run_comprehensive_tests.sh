#!/bin/bash

# Test Coverage and Analysis Script for IM-C Project
# This script runs all tests, generates coverage reports, and performs memory leak detection

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
BUILD_DIR="build"
COVERAGE_DIR="coverage"
VALGRIND_LOG_DIR="valgrind_logs"

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}     IM-C Testing Suite v2.0          ${NC}"
echo -e "${BLUE}========================================${NC}"

# Function to print section headers
print_section() {
    echo -e "\n${YELLOW}>>> $1 <<<${NC}"
}

# Function to check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Clean and create directories
print_section "Setting up directories"
rm -rf $BUILD_DIR $COVERAGE_DIR $VALGRIND_LOG_DIR
mkdir -p $BUILD_DIR $COVERAGE_DIR $VALGRIND_LOG_DIR

# Build tests with coverage
print_section "Building tests with coverage support"
cd $BUILD_DIR
cmake .. -DENABLE_COVERAGE=ON -DENABLE_VALGRIND=ON
make -j$(nproc)
cd ..

# Run all tests
print_section "Running test suite"
cd $BUILD_DIR
ctest --output-on-failure

# Check if tests passed
if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ All tests passed!${NC}"
else
    echo -e "${RED}✗ Some tests failed!${NC}"
    exit 1
fi

cd ..

# Generate coverage reports
print_section "Generating coverage reports"

# Check if gcov and lcov are available
if ! command_exists gcov; then
    echo -e "${RED}Warning: gcov not found, skipping coverage analysis${NC}"
elif ! command_exists lcov; then
    echo -e "${RED}Warning: lcov not found, installing basic coverage info${NC}"
    
    # Basic gcov coverage without lcov
    find $BUILD_DIR -name "*.gcno" -exec gcov {} \;
    mkdir -p $COVERAGE_DIR/basic
    mv *.gcov $COVERAGE_DIR/basic/ 2>/dev/null || true
    
    echo -e "${GREEN}Basic coverage files generated in $COVERAGE_DIR/basic/${NC}"
else
    # Full lcov coverage report
    echo "Capturing coverage data..."
    lcov --capture --directory $BUILD_DIR --output-file $COVERAGE_DIR/coverage.info
    
    # Remove system headers and test files from coverage
    lcov --remove $COVERAGE_DIR/coverage.info '/usr/*' '*/unity/*' '*/tests/*' --output-file $COVERAGE_DIR/coverage_filtered.info
    
    # Generate HTML report
    if command_exists genhtml; then
        genhtml $COVERAGE_DIR/coverage_filtered.info --output-directory $COVERAGE_DIR/html
        echo -e "${GREEN}✓ HTML coverage report generated in $COVERAGE_DIR/html/${NC}"
        echo -e "${BLUE}Open $COVERAGE_DIR/html/index.html in a browser to view the report${NC}"
    else
        echo -e "${YELLOW}genhtml not found, HTML report not generated${NC}"
    fi
    
    # Display coverage summary
    echo -e "\n${BLUE}Coverage Summary:${NC}"
    lcov --summary $COVERAGE_DIR/coverage_filtered.info
fi

# Memory leak detection with Valgrind
if command_exists valgrind; then
    print_section "Running memory leak detection"
    
    # Test each executable with Valgrind
    for test_exe in $BUILD_DIR/test_*; do
        if [ -x "$test_exe" ] && [ -f "$test_exe" ]; then
            test_name=$(basename "$test_exe")
            echo "Testing $test_name with Valgrind..."
            
            valgrind --tool=memcheck \
                     --leak-check=full \
                     --show-leak-kinds=all \
                     --track-origins=yes \
                     --verbose \
                     --log-file="$VALGRIND_LOG_DIR/${test_name}.log" \
                     "$test_exe" > /dev/null 2>&1
            
            # Check for memory leaks
            if grep -q "ERROR SUMMARY: 0 errors" "$VALGRIND_LOG_DIR/${test_name}.log" && \
               grep -q "definitely lost: 0 bytes" "$VALGRIND_LOG_DIR/${test_name}.log"; then
                echo -e "${GREEN}✓ $test_name: No memory leaks detected${NC}"
            else
                echo -e "${RED}✗ $test_name: Memory issues detected (see $VALGRIND_LOG_DIR/${test_name}.log)${NC}"
            fi
        fi
    done
else
    echo -e "${YELLOW}Valgrind not found, skipping memory leak detection${NC}"
fi

# Performance profiling (if available)
if command_exists perf; then
    print_section "Running performance profiling"
    
    # Profile a representative test
    if [ -x "$BUILD_DIR/test_message_types" ]; then
        echo "Profiling test_message_types..."
        perf record -o $BUILD_DIR/perf.data $BUILD_DIR/test_message_types > /dev/null 2>&1
        perf report -i $BUILD_DIR/perf.data --stdio > $COVERAGE_DIR/performance_report.txt 2>/dev/null
        echo -e "${GREEN}✓ Performance report generated in $COVERAGE_DIR/performance_report.txt${NC}"
    fi
else
    echo -e "${YELLOW}perf not found, skipping performance profiling${NC}"
fi

# Static analysis (if available)
if command_exists cppcheck; then
    print_section "Running static analysis"
    
    cppcheck --enable=all \
             --inconclusive \
             --xml \
             --xml-version=2 \
             ../frontend/ ../backend/ \
             2> $COVERAGE_DIR/cppcheck_report.xml
    
    echo -e "${GREEN}✓ Static analysis report generated in $COVERAGE_DIR/cppcheck_report.xml${NC}"
else
    echo -e "${YELLOW}cppcheck not found, skipping static analysis${NC}"
fi

# Generate final test report
print_section "Generating test report"

cat > $COVERAGE_DIR/test_report.md << EOF
# IM-C Test Report

## Test Execution Summary
- **Date**: $(date)
- **Total Tests**: $(cd $BUILD_DIR && ctest --list-tests | wc -l)
- **Status**: All tests passed ✓

## Test Categories
1. **Unit Tests**: Message types, textbox functionality
2. **Integration Tests**: WebSocket communication with mocking
3. **UI Tests**: Clay component interaction (headless)
4. **Backend Tests**: Server message handling logic
5. **Error Handling**: Input validation and edge cases

## Coverage Analysis
EOF

if [ -f "$COVERAGE_DIR/coverage_filtered.info" ]; then
    echo "- **Coverage Report**: Available in \`$COVERAGE_DIR/html/index.html\`" >> $COVERAGE_DIR/test_report.md
    lcov --summary $COVERAGE_DIR/coverage_filtered.info >> $COVERAGE_DIR/test_report.md
else
    echo "- **Coverage Report**: Basic coverage files in \`$COVERAGE_DIR/basic/\`" >> $COVERAGE_DIR/test_report.md
fi

cat >> $COVERAGE_DIR/test_report.md << EOF

## Memory Analysis
EOF

if [ -d "$VALGRIND_LOG_DIR" ] && [ "$(ls -A $VALGRIND_LOG_DIR)" ]; then
    echo "- **Memory Leak Detection**: Completed with Valgrind" >> $COVERAGE_DIR/test_report.md
    echo "- **Log Files**: Available in \`$VALGRIND_LOG_DIR/\`" >> $COVERAGE_DIR/test_report.md
else
    echo "- **Memory Leak Detection**: Skipped (Valgrind not available)" >> $COVERAGE_DIR/test_report.md
fi

cat >> $COVERAGE_DIR/test_report.md << EOF

## Static Analysis
EOF

if [ -f "$COVERAGE_DIR/cppcheck_report.xml" ]; then
    echo "- **Static Analysis**: Completed with cppcheck" >> $COVERAGE_DIR/test_report.md
    echo "- **Report**: Available in \`$COVERAGE_DIR/cppcheck_report.xml\`" >> $COVERAGE_DIR/test_report.md
else
    echo "- **Static Analysis**: Skipped (cppcheck not available)" >> $COVERAGE_DIR/test_report.md
fi

echo -e "\n${GREEN}✓ Test report generated in $COVERAGE_DIR/test_report.md${NC}"

# Summary
print_section "Test Suite Complete"
echo -e "${GREEN}All testing phases completed successfully!${NC}"
echo -e "${BLUE}Results available in:${NC}"
echo -e "  • Test Report: $COVERAGE_DIR/test_report.md"
echo -e "  • Coverage: $COVERAGE_DIR/"
echo -e "  • Memory Logs: $VALGRIND_LOG_DIR/"

# Optional: Open coverage report in browser
if command_exists xdg-open && [ -f "$COVERAGE_DIR/html/index.html" ]; then
    read -p "Open coverage report in browser? (y/n): " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        xdg-open "$COVERAGE_DIR/html/index.html"
    fi
fi