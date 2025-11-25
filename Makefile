# Makefile for Inchrosil DNA Compression Project
# Raspberry Pi 5 - ARM Cortex-A76 Optimized Build System

# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -O3 -march=armv8.2-a -mtune=cortex-a76 -Wall
INCLUDES = -Iinclude

# Directories
SRC_DIR = src
INC_DIR = include
BIN_DIR = bin
SCRIPT_DIR = scripts
DATA_DIR = data
DOCS_DIR = docs

# Source files
CLIENT_SRC = $(SRC_DIR)/dna_client.cpp
SERVER_SRC = $(SRC_DIR)/dna_server.cpp
BINARY_DECODER_SRC = $(SRC_DIR)/dna_binary_decoder.cpp
BINARY_GEN_SRC = $(SRC_DIR)/generate_binary_files.cpp
TEST_BINARY_SRC = $(SRC_DIR)/test_binary_files.cpp
TEST_COMPRESS_SRC = $(SRC_DIR)/test_compression_sizes.cpp
TEST_SIZES_SRC = $(SRC_DIR)/test_different_sizes.cpp
SERIAL_EXAMPLE_SRC = $(SRC_DIR)/dna_serial_example_optimized.cpp

# Binaries
CLIENT_BIN = $(BIN_DIR)/dna_client
SERVER_BIN = $(BIN_DIR)/dna_server
BINARY_DECODER_BIN = $(BIN_DIR)/dna_binary_decoder
BINARY_GEN_BIN = $(BIN_DIR)/generate_binary_files
TEST_BINARY_BIN = $(BIN_DIR)/test_binary_files
TEST_COMPRESS_BIN = $(BIN_DIR)/test_compression_sizes
TEST_SIZES_BIN = $(BIN_DIR)/test_different_sizes
SERIAL_EXAMPLE_BIN = $(BIN_DIR)/dna_serial_example

# Default target
.PHONY: all
all: $(BIN_DIR) $(CLIENT_BIN) $(SERVER_BIN) $(BINARY_DECODER_BIN) $(BINARY_GEN_BIN) \
     $(TEST_BINARY_BIN) $(TEST_COMPRESS_BIN) $(TEST_SIZES_BIN)

# Create bin directory
$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

# Client-Server
$(CLIENT_BIN): $(CLIENT_SRC) $(INC_DIR)/dna_serial_processor.hpp
	@echo "🔨 Building DNA Client..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -pthread $(CLIENT_SRC) -o $(CLIENT_BIN)
	@echo "✅ Built: $(CLIENT_BIN)"

$(SERVER_BIN): $(SERVER_SRC) $(INC_DIR)/dna_serial_processor.hpp
	@echo "🔨 Building DNA Server..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -pthread $(SERVER_SRC) -o $(SERVER_BIN)
	@echo "✅ Built: $(SERVER_BIN)"

# Binary tools
$(BINARY_DECODER_BIN): $(BINARY_DECODER_SRC)
	@echo "🔨 Building Binary Decoder..."
	$(CXX) $(CXXFLAGS) $(BINARY_DECODER_SRC) -o $(BINARY_DECODER_BIN)
	@echo "✅ Built: $(BINARY_DECODER_BIN)"

$(BINARY_GEN_BIN): $(BINARY_GEN_SRC)
	@echo "🔨 Building Binary Generator..."
	$(CXX) $(CXXFLAGS) $(BINARY_GEN_SRC) -o $(BINARY_GEN_BIN)
	@echo "✅ Built: $(BINARY_GEN_BIN)"

# Test suites
$(TEST_BINARY_BIN): $(TEST_BINARY_SRC)
	@echo "🔨 Building Binary File Tests..."
	$(CXX) $(CXXFLAGS) $(TEST_BINARY_SRC) -o $(TEST_BINARY_BIN)
	@echo "✅ Built: $(TEST_BINARY_BIN)"

$(TEST_COMPRESS_BIN): $(TEST_COMPRESS_SRC)
	@echo "🔨 Building Compression Tests..."
	$(CXX) $(CXXFLAGS) $(TEST_COMPRESS_SRC) -o $(TEST_COMPRESS_BIN)
	@echo "✅ Built: $(TEST_COMPRESS_BIN)"

$(TEST_SIZES_BIN): $(TEST_SIZES_SRC)
	@echo "🔨 Building Size Scaling Tests..."
	$(CXX) $(CXXFLAGS) $(TEST_SIZES_SRC) -o $(TEST_SIZES_BIN)
	@echo "✅ Built: $(TEST_SIZES_BIN)"

$(SERIAL_EXAMPLE_BIN): $(SERIAL_EXAMPLE_SRC) $(INC_DIR)/dna_serial_processor.hpp
	@echo "🔨 Building Serial Example..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(SERIAL_EXAMPLE_SRC) -o $(SERIAL_EXAMPLE_BIN)
	@echo "✅ Built: $(SERIAL_EXAMPLE_BIN)"

# Specific build targets
.PHONY: client-server
client-server: $(CLIENT_BIN) $(SERVER_BIN)
	@echo "✅ Client-Server built"

.PHONY: tools
tools: $(BINARY_DECODER_BIN) $(BINARY_GEN_BIN)
	@echo "✅ Binary tools built"

.PHONY: tests
tests: $(TEST_BINARY_BIN) $(TEST_COMPRESS_BIN) $(TEST_SIZES_BIN)
	@echo "✅ Test suites built"

# Run tests
.PHONY: test
test: $(TEST_BINARY_BIN) $(TEST_COMPRESS_BIN) $(TEST_SIZES_BIN)
	@echo ""
	@echo "╔══════════════════════════════════════════════════════════════╗"
	@echo "║              Running All Test Suites                         ║"
	@echo "╚══════════════════════════════════════════════════════════════╝"
	@echo ""
	@echo "🧪 Test 1: Binary File Validation"
	@cd $(DATA_DIR) && ../$(TEST_BINARY_BIN) || true
	@echo ""
	@echo "🧪 Test 2: Compression Ratios"
	@$(TEST_COMPRESS_BIN) || true
	@echo ""
	@echo "🧪 Test 3: Size Scaling"
	@$(TEST_SIZES_BIN) || true

# Generate binary files from FASTA
.PHONY: generate-binary
generate-binary: $(BINARY_GEN_BIN)
	@echo "📦 Generating binary files from FASTA..."
	@cd $(DATA_DIR) && ../$(BINARY_GEN_BIN)

# Clean
.PHONY: clean
clean:
	@echo "🧹 Cleaning build artifacts..."
	@rm -f $(BIN_DIR)/*
	@echo "✅ Clean complete"

.PHONY: clean-data
clean-data:
	@echo "🧹 Cleaning data files..."
	@rm -f $(DATA_DIR)/*.ich $(DATA_DIR)/*.bin
	@echo "✅ Data cleaned"

# Help
.PHONY: help
help:
	@echo "Inchrosil DNA Compression - Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all              - Build all binaries (default)"
	@echo "  client-server    - Build client and server"
	@echo "  tools            - Build binary encoder/decoder tools"
	@echo "  tests            - Build test suites"
	@echo "  test             - Run all tests"
	@echo "  generate-binary  - Generate .bin files from FASTA data"
	@echo "  clean            - Remove binaries"
	@echo "  clean-data       - Remove generated data files"
	@echo "  help             - Show this help message"
	@echo ""
	@echo "Examples:"
	@echo "  make                    # Build everything"
	@echo "  make client-server      # Build only client/server"
	@echo "  make test               # Run all tests"
	@echo "  make generate-binary    # Create binary files"

.PHONY: info
info:
	@echo "╔══════════════════════════════════════════════════════════════╗"
	@echo "║         Inchrosil DNA Compression Project Info              ║"
	@echo "╚══════════════════════════════════════════════════════════════╝"
	@echo ""
	@echo "Platform:     Raspberry Pi 5"
	@echo "CPU:          4× Cortex-A76 @ 2.4 GHz"
	@echo "Compiler:     $(CXX) (GCC 14.2.0)"
	@echo "Optimization: -O3 -march=armv8.2-a -mtune=cortex-a76"
	@echo ""
	@echo "Project Structure:"
	@echo "  src/       - Source files (.cpp)"
	@echo "  include/   - Header files (.hpp)"
	@echo "  bin/       - Compiled binaries"
	@echo "  scripts/   - Build scripts (.sh)"
	@echo "  data/      - FASTA files and binary data"
	@echo "  docs/      - Documentation (.md)"
	@echo ""
	@echo "Encoding:     2-bit DNA (A=00, T=01, G=10, C=11)"
	@echo "Compression:  4:1 ratio (75% space savings)"
	@echo "Throughput:   235+ MB/s on large sequences"
