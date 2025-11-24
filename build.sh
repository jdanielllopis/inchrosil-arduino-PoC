#!/bin/bash

# Build script for Raspberry Pi 5 Inchrosil RTOS Example
# This script automates the build process

set -e  # Exit on error

echo "╔════════════════════════════════════════════════╗"
echo "║  Raspberry Pi 5 Inchrosil RTOS Build Script   ║"
echo "╚════════════════════════════════════════════════╝"
echo ""

# Check for required tools
echo "Checking dependencies..."

if ! command -v cmake &> /dev/null; then
    echo "❌ CMake not found. Please install: sudo apt-get install cmake"
    exit 1
fi

if ! command -v g++ &> /dev/null; then
    echo "❌ G++ not found. Please install: sudo apt-get install build-essential"
    exit 1
fi

echo "✓ CMake found: $(cmake --version | head -n1)"
echo "✓ G++ found: $(g++ --version | head -n1)"
echo ""

# Check if submodule is initialized
if [ ! -f "Inchrosil/include/nucleotide.hpp" ]; then
    echo "⚠️  Inchrosil submodule not found. Initializing..."
    git submodule update --init --recursive
    echo "✓ Submodule initialized"
else
    echo "✓ Inchrosil submodule found"
fi
echo ""

# Create build directory
echo "Creating build directory..."
mkdir -p build
cd build

# Run CMake
echo "Running CMake configuration..."
cmake .. -DCMAKE_BUILD_TYPE=Release

echo ""
echo "Building project..."
make -j$(nproc)

echo ""
echo "╔════════════════════════════════════════════════╗"
echo "║  Build Complete!                               ║"
echo "╚════════════════════════════════════════════════╝"
echo ""
echo "Run the example with:"
echo "  ./build/rpi5_dna_rtos"
echo ""
