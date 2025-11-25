#!/bin/bash

###############################################################################
# Inchrosil DNA Compression Project - Unified Build Script
# Raspberry Pi 5 - Hardware Optimized
###############################################################################

set -e  # Exit on error

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
MAGENTA='\033[0;35m'
NC='\033[0m'

# Project directories
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SRC_DIR="$PROJECT_ROOT/src"
INC_DIR="$PROJECT_ROOT/include"
BIN_DIR="$PROJECT_ROOT/bin"
DATA_DIR="$PROJECT_ROOT/data"

# Compiler settings
CXX="g++"
CXXFLAGS="-std=c++17 -O3 -march=armv8.2-a -mtune=cortex-a76 -Wall"
INCLUDES="-I$INC_DIR"

###############################################################################
# Helper Functions
###############################################################################

print_header() {
    echo -e "\n${BLUE}╔══════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${BLUE}║$(printf '%62s' | tr ' ' ' ')║${NC}"
    printf "${BLUE}║${NC}%-62s${BLUE}║${NC}\n" "  $1"
    echo -e "${BLUE}║$(printf '%62s' | tr ' ' ' ')║${NC}"
    echo -e "${BLUE}╚══════════════════════════════════════════════════════════════╝${NC}\n"
}

print_info() {
    echo -e "${GREEN}[✓]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[⚠]${NC} $1"
}

print_error() {
    echo -e "${RED}[✗]${NC} $1"
}

print_build() {
    echo -e "${CYAN}[🔨]${NC} $1"
}

###############################################################################
# System Checks
###############################################################################

check_system() {
    print_header "System Requirements Check"
    
    # Architecture
    ARCH=$(uname -m)
    if [ "$ARCH" = "aarch64" ]; then
        print_info "Architecture: $ARCH (ARM64)"
    else
        print_warning "Architecture: $ARCH (optimizations target ARM64)"
    fi
    
    # CPU
    if grep -q "Cortex-A76" /proc/cpuinfo 2>/dev/null; then
        CPU_COUNT=$(nproc)
        CPU_FREQ=$(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq 2>/dev/null | awk '{print $1/1000 " MHz"}')
        print_info "CPU: Cortex-A76 × $CPU_COUNT @ $CPU_FREQ"
    else
        print_warning "CPU: Not Cortex-A76 (build will work but may be slower)"
    fi
    
    # RAM
    TOTAL_RAM=$(free -h | awk '/^Mem:/{print $2}')
    print_info "RAM: $TOTAL_RAM"
    
    # Compiler
    if command -v g++ &> /dev/null; then
        GCC_VERSION=$(g++ --version | head -n1)
        print_info "Compiler: $GCC_VERSION"
    else
        print_error "g++ not found. Install: sudo apt install build-essential"
        exit 1
    fi
    
    # NEON SIMD
    if grep -q "asimd" /proc/cpuinfo 2>/dev/null; then
        print_info "NEON SIMD: Supported"
    else
        print_warning "NEON SIMD: Not detected"
    fi
    
    # Hardware crypto
    if grep -q "aes" /proc/cpuinfo 2>/dev/null; then
        print_info "ARM Crypto: AES, CRC32, SHA available"
    fi
    
    echo ""
}

###############################################################################
# Build Functions
###############################################################################

build_client_server() {
    print_header "Building Client-Server"
    
    # Create bin directory
    mkdir -p "$BIN_DIR"
    
    # Build server
    print_build "Building DNA Server..."
    $CXX $CXXFLAGS $INCLUDES -pthread "$SRC_DIR/dna_server.cpp" -o "$BIN_DIR/dna_server"
    print_info "Built: $BIN_DIR/dna_server ($(du -h "$BIN_DIR/dna_server" | cut -f1))"
    
    # Build client
    print_build "Building DNA Client..."
    $CXX $CXXFLAGS $INCLUDES -pthread "$SRC_DIR/dna_client.cpp" -o "$BIN_DIR/dna_client"
    print_info "Built: $BIN_DIR/dna_client ($(du -h "$BIN_DIR/dna_client" | cut -f1))"
    
    echo ""
}

build_tools() {
    print_header "Building DNA Tools"
    
    mkdir -p "$BIN_DIR"
    
    # Binary decoder
    print_build "Building Binary Decoder..."
    $CXX $CXXFLAGS "$SRC_DIR/dna_binary_decoder.cpp" -o "$BIN_DIR/dna_binary_decoder"
    print_info "Built: $BIN_DIR/dna_binary_decoder"
    
    # Binary generator
    print_build "Building Binary Generator..."
    $CXX $CXXFLAGS "$SRC_DIR/generate_binary_files.cpp" -o "$BIN_DIR/generate_binary_files"
    print_info "Built: $BIN_DIR/generate_binary_files"
    
    echo ""
}

build_tests() {
    print_header "Building Test Suites"
    
    mkdir -p "$BIN_DIR"
    
    # Binary file tests
    print_build "Building Binary File Tests..."
    $CXX $CXXFLAGS "$SRC_DIR/test_binary_files.cpp" -o "$BIN_DIR/test_binary_files"
    print_info "Built: $BIN_DIR/test_binary_files"
    
    # Compression tests
    print_build "Building Compression Tests..."
    $CXX $CXXFLAGS "$SRC_DIR/test_compression_sizes.cpp" -o "$BIN_DIR/test_compression_sizes"
    print_info "Built: $BIN_DIR/test_compression_sizes"
    
    # Size scaling tests
    print_build "Building Size Scaling Tests..."
    $CXX $CXXFLAGS "$SRC_DIR/test_different_sizes.cpp" -o "$BIN_DIR/test_different_sizes"
    print_info "Built: $BIN_DIR/test_different_sizes"
    
    echo ""
}

build_examples() {
    print_header "Building Examples"
    
    mkdir -p "$BIN_DIR"
    
    # Serial example
    if [ -f "$SRC_DIR/dna_serial_example_optimized.cpp" ]; then
        print_build "Building Serial Example..."
        $CXX $CXXFLAGS $INCLUDES "$SRC_DIR/dna_serial_example_optimized.cpp" -o "$BIN_DIR/dna_serial_example"
        print_info "Built: $BIN_DIR/dna_serial_example"
    fi
    
    echo ""
}

build_all() {
    print_header "Building All Components"
    
    build_client_server
    build_tools
    build_tests
    # build_examples  # Disabled: example code has unimplemented dependencies
    
    print_header "Build Summary"
    ls -lh "$BIN_DIR" | tail -n +2 | awk '{printf "  %-30s %8s\n", $9, $5}'
    echo ""
}

###############################################################################
# Verification
###############################################################################

verify_build() {
    print_header "Verifying Build"
    
    TOTAL=0
    PASSED=0
    
    # Check binaries exist
    for binary in dna_client dna_server dna_binary_decoder generate_binary_files \
                  test_binary_files test_compression_sizes test_different_sizes; do
        TOTAL=$((TOTAL + 1))
        if [ -f "$BIN_DIR/$binary" ] && [ -x "$BIN_DIR/$binary" ]; then
            print_info "$binary: executable"
            PASSED=$((PASSED + 1))
        else
            print_warning "$binary: missing or not executable"
        fi
    done
    
    # Check for hardware acceleration
    if command -v objdump &> /dev/null; then
        echo ""
        print_build "Checking hardware acceleration in binaries..."
        
        if objdump -d "$BIN_DIR/dna_server" 2>/dev/null | grep -q "crc32"; then
            print_info "CRC32 hardware instructions: Found"
        else
            print_warning "CRC32 hardware instructions: Not found"
        fi
    fi
    
    echo ""
    print_info "Build verification: $PASSED/$TOTAL binaries OK"
    echo ""
}

###############################################################################
# Testing
###############################################################################

run_tests() {
    print_header "Running Test Suites"
    
    cd "$PROJECT_ROOT"
    
    echo -e "${CYAN}Test 1: Binary File Validation${NC}"
    if [ -f "$BIN_DIR/test_binary_files" ]; then
        cd "$DATA_DIR" && "../$BIN_DIR/test_binary_files" || true
        cd "$PROJECT_ROOT"
    else
        print_warning "test_binary_files not found"
    fi
    
    echo -e "\n${CYAN}Test 2: Compression Ratios${NC}"
    if [ -f "$BIN_DIR/test_compression_sizes" ]; then
        "$BIN_DIR/test_compression_sizes" || true
    else
        print_warning "test_compression_sizes not found"
    fi
    
    echo -e "\n${CYAN}Test 3: Size Scaling${NC}"
    if [ -f "$BIN_DIR/test_different_sizes" ]; then
        "$BIN_DIR/test_different_sizes" || true
    else
        print_warning "test_different_sizes not found"
    fi
    
    echo ""
}

###############################################################################
# Cleanup
###############################################################################

clean() {
    print_header "Cleaning Build Artifacts"
    
    if [ -d "$BIN_DIR" ]; then
        print_info "Removing $BIN_DIR..."
        rm -rf "$BIN_DIR"
    fi
    
    if [ -d "$PROJECT_ROOT/build" ]; then
        print_info "Removing $PROJECT_ROOT/build..."
        rm -rf "$PROJECT_ROOT/build"
    fi
    
    print_info "Clean complete"
    echo ""
}

###############################################################################
# Performance Setup
###############################################################################

setup_performance() {
    print_header "Performance Optimization Setup"
    
    # Check if running on Raspberry Pi
    if [ ! -f /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor ]; then
        print_warning "CPU frequency scaling not available"
        return
    fi
    
    # Set CPU governor
    CURRENT_GOV=$(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor 2>/dev/null || echo "unknown")
    print_info "Current CPU governor: $CURRENT_GOV"
    
    if [ "$CURRENT_GOV" != "performance" ]; then
        print_build "Setting CPU governor to performance mode..."
        echo "Run: echo performance | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor"
    fi
    
    # Temperature check
    if command -v vcgencmd &> /dev/null; then
        TEMP=$(vcgencmd measure_temp 2>/dev/null | grep -o "[0-9.]*" || echo "N/A")
        print_info "CPU temperature: ${TEMP}°C"
    fi
    
    echo ""
}

###############################################################################
# Usage
###############################################################################

show_usage() {
    cat << EOF
${BLUE}╔══════════════════════════════════════════════════════════════╗
║     Inchrosil DNA Compression - Unified Build Script         ║
╚══════════════════════════════════════════════════════════════╝${NC}

${GREEN}Usage:${NC} $0 [OPTION]

${YELLOW}Options:${NC}
  all               Build all components (default)
  client-server     Build only client and server
  tools             Build binary encoder/decoder tools
  tests             Build test suites
  examples          Build example programs
  
  check             Check system requirements
  verify            Verify built binaries
  test              Run all test suites
  clean             Remove all build artifacts
  
  performance       Setup performance optimizations
  help              Show this help message

${CYAN}Examples:${NC}
  $0                # Build everything
  $0 all            # Build everything
  $0 client-server  # Build only client/server
  $0 clean all      # Clean and rebuild all
  $0 verify test    # Verify build and run tests

${MAGENTA}Platform:${NC} Raspberry Pi 5 (4×Cortex-A76 @ 2.4 GHz)
${MAGENTA}Compiler:${NC} GCC with ARM optimizations (-O3 -march=armv8.2-a)

EOF
}

###############################################################################
# Main
###############################################################################

main() {
    # Change to project root
    cd "$PROJECT_ROOT"
    
    # Default action
    if [ $# -eq 0 ]; then
        check_system
        build_all
        verify_build
        exit 0
    fi
    
    # Process arguments
    for arg in "$@"; do
        case "$arg" in
            all)
                check_system
                build_all
                verify_build
                ;;
            client-server)
                build_client_server
                ;;
            tools)
                build_tools
                ;;
            tests)
                build_tests
                ;;
            examples)
                build_examples
                ;;
            check)
                check_system
                ;;
            verify)
                verify_build
                ;;
            test)
                run_tests
                ;;
            clean)
                clean
                ;;
            performance)
                setup_performance
                ;;
            help|--help|-h)
                show_usage
                exit 0
                ;;
            *)
                print_error "Unknown option: $arg"
                show_usage
                exit 1
                ;;
        esac
    done
}

# Run main
main "$@"
