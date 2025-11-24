# Inchrosil Raspberry Pi 5 RTOS Example

Proof of Concept for DNA encoding/decoding on Raspberry Pi 5 using the Inchrosil library with RTOS (Real-Time Operating System) capabilities.

## Overview

This project demonstrates real-time DNA sequence processing on Raspberry Pi 5 using:
- **Inchrosil**: DNA encoding/decoding library
- **RTOS Components**: Priority-based task scheduling and deterministic memory allocation
- **Multi-core Processing**: Utilizes all 4 Cortex-A76 cores

## Features

- ✅ Priority-based DNA processing (Critical, High, Normal, Low)
- ✅ Deterministic memory allocation with memory pools
- ✅ Real-time performance monitoring
- ✅ Multi-core task scheduling (4 worker threads)
- ✅ Deadline tracking and metrics
- ✅ Cache-optimized for Raspberry Pi 5 (ARM Cortex-A76)

## Hardware Requirements

- **Raspberry Pi 5** (4GB or 8GB RAM)
- ARM Cortex-A76 quad-core processor
- Raspberry Pi OS (64-bit recommended)

## Software Requirements

- CMake 3.15 or higher
- GCC/G++ with C++17 support
- pthread library (usually pre-installed)

## Project Structure

```
inchrosil-arduino-PoC/
├── Inchrosil/                          # Inchrosil library submodule
│   ├── include/
│   │   ├── nucleotide.hpp             # DNA encoding/decoding
│   │   ├── rtos_scheduler.hpp         # RTOS task scheduler
│   │   └── rtos_memory_pool.hpp       # Deterministic memory pool
│   └── src/
├── rpi5_inchrosil_rtos_example.cpp    # Main example application
├── CMakeLists.txt                      # Build configuration
├── build.sh                            # Build script
└── README.md                           # This file
```

## Quick Start

### 1. Clone the Repository

```bash
git clone https://github.com/jdanielllopis/inchrosil-arduino-PoC.git
cd inchrosil-arduino-PoC
```

### 2. Initialize Submodules

```bash
git submodule update --init --recursive
```

### 3. Build the Project

```bash
chmod +x build.sh
./build.sh
```

Or manually:

```bash
mkdir -p build
cd build
cmake ..
make -j4
```

### 4. Run the Example

```bash
./build/rpi5_dna_rtos
```

## Example Output

```
╔════════════════════════════════════════════════╗
║  Raspberry Pi 5 - DNA Processing with RTOS    ║
╚════════════════════════════════════════════════╝

Hardware Configuration:
  CPU: ARM Cortex-A76 (4 cores)
  Cores: 4
  Memory Pool: 2048 KB
  Block Size: 4096 bytes
  Total Blocks: 512

═══════════════════════════════════════════════
Starting DNA Processing Tasks...
═══════════════════════════════════════════════

[CRITICAL] Genome #1 | 112 nucleotides | 156µs | ✓
[CRITICAL] Genome #2 | 112 nucleotides | 142µs | ✓
[HIGH] Error correction: PATIENT_SAMPLE_A | 272 nucleotides | 189µs
[NORMAL] Encoded: "Hello Raspberry Pi 5" | 160 nucleotides | 124µs
[LOW] Archived #1 | 136 nucleotides | 98µs

╔════════════════════════════════════════════════╗
║         Performance Metrics                    ║
╚════════════════════════════════════════════════╝

Total Deadline Misses: 0
Memory Pool Utilization: 2.34%
```

## DNA Processing Tasks

### Critical Priority (10ms deadline)
- **Genome Sequencing**: Real-time DNA sequence reading and encoding
- Use case: Time-critical medical diagnostics

### High Priority (50ms deadline)
- **Error Correction**: Add redundancy to DNA sequences
- Use case: Data integrity verification

### Normal Priority (100ms deadline)
- **Data Encoding**: General purpose DNA encoding
- Use case: Standard data storage operations

### Low Priority (500ms deadline)
- **Backup Archival**: Background DNA database backup
- Use case: Non-critical maintenance tasks

## RTOS Features

### Priority-Based Scheduler
```cpp
RTOSScheduler scheduler(4);  // 4 worker threads
scheduler.scheduleTask(
    Priority::CRITICAL,
    []() { /* task */ },
    std::chrono::milliseconds(10)  // deadline
);
```

### Deterministic Memory Pool
```cpp
RTOSMemoryPool pool(2*1024*1024, 4096);  // 2MB, 4KB blocks
RTOSDNABuffer buffer(pool, 1024);        // Allocate from pool
```

### Performance Metrics
- Total executions
- Average execution time
- Worst-case execution time (WCET)
- Jitter variance
- Deadline miss count

## Optimization for Raspberry Pi 5

The project includes specific optimizations for RPi 5:

```cmake
# Cortex-A76 optimizations
-march=armv8.2-a -mtune=cortex-a76 -O3
```

- **Cache-aligned blocks**: 4KB blocks (64-byte cache lines × 64)
- **Multi-core utilization**: 4 worker threads for 4 cores
- **SIMD support**: ARM NEON instructions available

## Configuration

Edit the constants in `rpi5_inchrosil_rtos_example.cpp`:

```cpp
constexpr size_t RPI5_CORES = 4;              // Number of cores
constexpr size_t POOL_SIZE = 2 * 1024 * 1024; // Memory pool size
constexpr size_t BLOCK_SIZE = 4096;           // Block size
```

## Future Enhancements

- [ ] GPIO integration for DNA sensor reading
- [ ] Periodic task support
- [ ] Task preemption
- [ ] FreeRTOS API compatibility layer
- [ ] SPI/I2C communication for DNA sequencers
- [ ] Real-time data streaming

## Documentation

For detailed RTOS usage, see:
- [RTOS Usage Guide](Inchrosil/RTOS_USAGE_GUIDE.md)
- [RTOS Compatibility Analysis](Inchrosil/RTOS_COMPATIBILITY_ANALYSIS.md)
- [Inchrosil Documentation](Inchrosil/DOCUMENTATION.md)

## Troubleshooting

### Build Errors

If you encounter build errors, ensure:
1. Submodules are initialized: `git submodule update --init --recursive`
2. CMake version ≥ 3.15: `cmake --version`
3. C++17 support: `g++ --version` (GCC 7+)

### Runtime Issues

If the program fails to run:
1. Check thread support: `ldd ./build/rpi5_dna_rtos`
2. Verify memory availability: `free -h`
3. Check CPU info: `cat /proc/cpuinfo`

## License

See [LICENSE](Inchrosil/LICENSE) for the Inchrosil library license.

## References

- **Inchrosil Library**: https://github.com/jdanielllopis/Inchrosil
- **Raspberry Pi 5**: https://www.raspberrypi.com/products/raspberry-pi-5/
- **ARM Cortex-A76**: https://developer.arm.com/Processors/Cortex-A76

## Author

Based on the Inchrosil library by jdanielllopis
