# Makefile for Raspberry Pi 5 Inchrosil RTOS Example
# Simple alternative to CMake build

CXX = g++
CXXFLAGS = -std=c++17 -O3 -Wall -Wextra -march=armv8.2-a -mtune=cortex-a76
INCLUDES = -I. -IInchrosil/include
LDFLAGS = -pthread

# Source files
INCHROSIL_SRC = Inchrosil/src/nucleotide.cpp \
                Inchrosil/src/rtos_scheduler.cpp \
                Inchrosil/src/rtos_memory_pool.cpp

MAIN_SRC = rpi5_inchrosil_rtos_example.cpp

# Object files
INCHROSIL_OBJ = $(INCHROSIL_SRC:.cpp=.o)
MAIN_OBJ = $(MAIN_SRC:.cpp=.o)

# Target executable
TARGET = rpi5_dna_rtos

.PHONY: all clean run info

all: info $(TARGET)
	@echo ""
	@echo "╔════════════════════════════════════════════════╗"
	@echo "║  Build Complete!                               ║"
	@echo "╚════════════════════════════════════════════════╝"
	@echo ""
	@echo "Run with: ./$(TARGET)"

$(TARGET): $(INCHROSIL_OBJ) $(MAIN_OBJ)
	@echo "Linking $@..."
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	@echo "Cleaning build artifacts..."
	rm -f $(INCHROSIL_OBJ) $(MAIN_OBJ) $(TARGET)
	rm -rf build/
	@echo "Clean complete!"

run: $(TARGET)
	@echo "Running $(TARGET)..."
	@echo ""
	./$(TARGET)

info:
	@echo "╔════════════════════════════════════════════════╗"
	@echo "║  Raspberry Pi 5 Inchrosil RTOS Build          ║"
	@echo "╚════════════════════════════════════════════════╝"
	@echo ""
	@echo "Compiler: $(CXX)"
	@echo "Flags: $(CXXFLAGS)"
	@echo "Threads: $(shell nproc) cores available"
	@echo ""

# Dependencies
$(MAIN_OBJ): Inchrosil/include/nucleotide.hpp \
             Inchrosil/include/rtos_scheduler.hpp \
             Inchrosil/include/rtos_memory_pool.hpp

Inchrosil/src/nucleotide.o: Inchrosil/include/nucleotide.hpp
Inchrosil/src/rtos_scheduler.o: Inchrosil/include/rtos_scheduler.hpp
Inchrosil/src/rtos_memory_pool.o: Inchrosil/include/rtos_memory_pool.hpp
