#!/bin/bash

# IM-C Cross-Platform Build Script
# Builds static binaries for both Linux and Windows

set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$PROJECT_ROOT/build"
DIST_DIR="$PROJECT_ROOT/dist"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_status() {
    echo -e "${BLUE}>>> $1 <<<${NC}"
}

print_success() {
    echo -e "${GREEN}✓ $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠ $1${NC}"
}

print_error() {
    echo -e "${RED}✗ $1${NC}"
}

# Check for required tools
check_dependencies() {
    print_status "Checking dependencies"
    
    if ! command -v cmake &> /dev/null; then
        print_error "cmake not found. Please install cmake."
        exit 1
    fi
    
    if ! command -v x86_64-w64-mingw32-gcc &> /dev/null; then
        print_warning "MinGW-w64 not found. Windows cross-compilation will be skipped."
        print_warning "Install with: sudo apt install mingw-w64"
        SKIP_WINDOWS=1
    fi
    
    if ! pkg-config --exists libwebsockets; then
        print_error "libwebsockets not found. Please install libwebsockets-dev."
        exit 1
    fi
    
    print_success "Dependencies check completed"
}

# Clean previous builds
clean_builds() {
    print_status "Cleaning previous builds"
    rm -rf "$BUILD_DIR" "$DIST_DIR"
    mkdir -p "$BUILD_DIR" "$DIST_DIR"
    print_success "Build directories cleaned"
}

# Build for Linux
build_linux() {
    print_status "Building for Linux (static)"
    
    # Frontend
    cd "$PROJECT_ROOT/frontend"
    cmake . -B "$BUILD_DIR/linux-frontend" \
        -DCMAKE_BUILD_TYPE=Release \
        -DSTATIC_BUILD=ON
    cmake --build "$BUILD_DIR/linux-frontend" --config Release
    
    # Backend  
    cd "$PROJECT_ROOT/backend"
    cmake . -B "$BUILD_DIR/linux-backend" \
        -DCMAKE_BUILD_TYPE=Release \
        -DSTATIC_BUILD=ON
    cmake --build "$BUILD_DIR/linux-backend" --config Release
    
    # Copy binaries
    mkdir -p "$DIST_DIR/linux"
    cp "$BUILD_DIR/linux-frontend/im_c" "$DIST_DIR/linux/"
    cp "$BUILD_DIR/linux-backend/lws-minimal-ws-server" "$DIST_DIR/linux/"
    cp -r "$PROJECT_ROOT/frontend/resources" "$DIST_DIR/linux/" 2>/dev/null || true
    
    print_success "Linux build completed"
}

# Build for Windows
build_windows() {
    if [[ "$SKIP_WINDOWS" == "1" ]]; then
        print_warning "Skipping Windows build (MinGW-w64 not available)"
        return
    fi
    
    print_status "Building for Windows (static, cross-compiled)"
    
    # Frontend
    cd "$PROJECT_ROOT/frontend"
    cmake . -B "$BUILD_DIR/windows-frontend" \
        -DCMAKE_TOOLCHAIN_FILE="$PROJECT_ROOT/cmake/windows-toolchain.cmake" \
        -DCMAKE_BUILD_TYPE=Release \
        -DSTATIC_BUILD=ON \
        -DCROSS_COMPILE_WINDOWS=ON
    cmake --build "$BUILD_DIR/windows-frontend" --config Release
    
    # Backend
    cd "$PROJECT_ROOT/backend"
    cmake . -B "$BUILD_DIR/windows-backend" \
        -DCMAKE_TOOLCHAIN_FILE="$PROJECT_ROOT/cmake/windows-toolchain.cmake" \
        -DCMAKE_BUILD_TYPE=Release \
        -DSTATIC_BUILD=ON \
        -DCROSS_COMPILE_WINDOWS=ON
    cmake --build "$BUILD_DIR/windows-backend" --config Release
    
    # Copy binaries
    mkdir -p "$DIST_DIR/windows"
    cp "$BUILD_DIR/windows-frontend/im_c.exe" "$DIST_DIR/windows/" 2>/dev/null || cp "$BUILD_DIR/windows-frontend/im_c" "$DIST_DIR/windows/im_c.exe"
    cp "$BUILD_DIR/windows-backend/lws-minimal-ws-server.exe" "$DIST_DIR/windows/" 2>/dev/null || cp "$BUILD_DIR/windows-backend/lws-minimal-ws-server" "$DIST_DIR/windows/lws-minimal-ws-server.exe"
    cp -r "$PROJECT_ROOT/frontend/resources" "$DIST_DIR/windows/" 2>/dev/null || true
    
    print_success "Windows build completed"
}

# Create distribution packages
create_packages() {
    print_status "Creating distribution packages"
    
    cd "$DIST_DIR"
    
    # Linux package
    if [[ -d "linux" ]]; then
        tar -czf "im-c-linux-x64.tar.gz" linux/
        print_success "Created im-c-linux-x64.tar.gz"
    fi
    
    # Windows package
    if [[ -d "windows" ]]; then
        zip -r "im-c-windows-x64.zip" windows/
        print_success "Created im-c-windows-x64.zip"
    fi
}

# Show build summary
show_summary() {
    print_status "Build Summary"
    
    echo "Built binaries:"
    if [[ -f "$DIST_DIR/linux/im_c" ]]; then
        echo -e "${GREEN}  ✓ Linux frontend: $(du -h "$DIST_DIR/linux/im_c" | cut -f1)${NC}"
    fi
    if [[ -f "$DIST_DIR/linux/lws-minimal-ws-server" ]]; then
        echo -e "${GREEN}  ✓ Linux backend: $(du -h "$DIST_DIR/linux/lws-minimal-ws-server" | cut -f1)${NC}"
    fi
    if [[ -f "$DIST_DIR/windows/im_c.exe" ]]; then
        echo -e "${GREEN}  ✓ Windows frontend: $(du -h "$DIST_DIR/windows/im_c.exe" | cut -f1)${NC}"
    fi
    if [[ -f "$DIST_DIR/windows/lws-minimal-ws-server.exe" ]]; then
        echo -e "${GREEN}  ✓ Windows backend: $(du -h "$DIST_DIR/windows/lws-minimal-ws-server.exe" | cut -f1)${NC}"
    fi
    
    echo ""
    echo "Distribution packages:"
    if [[ -f "$DIST_DIR/im-c-linux-x64.tar.gz" ]]; then
        echo -e "${GREEN}  ✓ $DIST_DIR/im-c-linux-x64.tar.gz${NC}"
    fi
    if [[ -f "$DIST_DIR/im-c-windows-x64.zip" ]]; then
        echo -e "${GREEN}  ✓ $DIST_DIR/im-c-windows-x64.zip${NC}"
    fi
    
    echo ""
    print_success "Cross-platform build completed successfully!"
    echo -e "${BLUE}Send the Windows .zip to your friend for testing!${NC}"
}

# Main execution
main() {
    echo "==================================="
    echo "  IM-C Cross-Platform Builder"
    echo "==================================="
    echo ""
    
    check_dependencies
    clean_builds
    build_linux
    build_windows
    create_packages
    show_summary
}

# Handle command line arguments
case "$1" in
    "linux")
        check_dependencies
        clean_builds
        build_linux
        ;;
    "windows")
        check_dependencies
        clean_builds
        build_windows
        ;;
    "clean")
        clean_builds
        ;;
    *)
        main
        ;;
esac