# Quick Reference Card - Raspberry Pi 5 Inchrosil RTOS

## 🚀 Quick Start (30 seconds)

```bash
git clone https://github.com/jdanielllopis/inchrosil-arduino-PoC.git
cd inchrosil-arduino-PoC
./build.sh
./build/rpi5_dna_rtos
```

## 📁 Project Files

| File | Purpose |
|------|---------|
| `rpi5_inchrosil_rtos_example.cpp` | Main RTOS application |
| `CMakeLists.txt` | CMake build config |
| `Makefile` | Simple build alternative |
| `build.sh` | Automated build script |
| `README.md` | Full documentation |
| `SETUP_GUIDE.md` | Detailed setup steps |
| `PROJECT_SUMMARY.md` | Technical overview |
| `gui.py` | Python GUI (bonus) |

## 🔧 Build Commands

```bash
# Method 1: Build script (recommended)
./build.sh

# Method 2: CMake
mkdir build && cd build
cmake ..
make -j4

# Method 3: Makefile
make -j4

# Clean builds
rm -rf build/        # CMake
make clean          # Makefile
```

## ▶️ Run Commands

```bash
# After CMake build
./build/rpi5_dna_rtos

# After Makefile build
./rpi5_dna_rtos

# Or
make run
```

## 🎯 Task Priorities

| Priority | Deadline | Use Case |
|----------|----------|----------|
| CRITICAL | 10ms | Genome sequencing |
| HIGH | 50ms | Error correction |
| NORMAL | 100ms | Data encoding |
| LOW | 500ms | Backup/archival |

## 🧬 DNA Processing Flow

```
Binary Data → encodeBitsToNucleotides() → DNA Sequence
DNA Sequence → decodeNucleotidesToBits() → Binary Data
```

## 🔬 Example DNA Encoding

```cpp
std::string bits = "00011011";
std::string dna = encodeBitsToNucleotides(bits);
// Result: "ACGT" (example)
```

## ⚙️ Configuration Variables

```cpp
constexpr size_t RPI5_CORES = 4;              // CPU cores
constexpr size_t POOL_SIZE = 2 * 1024 * 1024; // 2MB pool
constexpr size_t BLOCK_SIZE = 4096;           // 4KB blocks
```

## 📊 Performance Monitoring

```bash
# CPU usage
htop

# Temperature
vcgencmd measure_temp

# Memory
free -h

# CPU frequency
cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq
```

## 🔍 Key Metrics

The application tracks:
- Total executions
- Average execution time
- Worst-case execution time (WCET)
- Jitter variance
- Deadline misses
- Memory pool utilization

## 🐛 Troubleshooting

```bash
# Submodule issues
git submodule update --init --recursive

# Check C++ version
g++ --version  # Need 7+ for C++17

# Install build tools
sudo apt-get install build-essential cmake git

# Check threading
ldd ./build/rpi5_dna_rtos | grep pthread
```

## 🔑 Essential API Calls

### Scheduler
```cpp
RTOSScheduler scheduler(4);
scheduler.start();

uint64_t task_id = scheduler.scheduleTask(
    Priority::HIGH,
    []() { /* task */ },
    std::chrono::milliseconds(50)
);

scheduler.stop();
```

### Memory Pool
```cpp
RTOSMemoryPool pool(2*1024*1024, 4096);
void* ptr = pool.allocate();
pool.deallocate(ptr);
double usage = pool.getUtilization();
```

### DNA Buffer
```cpp
RTOSDNABuffer buffer(pool, 1024);
buffer.resize(512);
size_t cap = buffer.capacity();
```

## 📈 Expected Performance

| Task Type | Time (µs) |
|-----------|-----------|
| Genome Sequencing | 140-160 |
| Error Correction | 170-190 |
| Data Encoding | 120-150 |
| Archival | 90-110 |

## 🌡️ Operating Ranges

- **CPU**: 40-70°C normal under load
- **Memory**: <20MB typical usage
- **Pool**: <5% utilization typical

## 📚 Documentation

- **Full Docs**: `README.md`
- **Setup Guide**: `SETUP_GUIDE.md`
- **RTOS Details**: `Inchrosil/RTOS_USAGE_GUIDE.md`
- **Project Info**: `PROJECT_SUMMARY.md`

## 🔗 Links

- **Inchrosil**: https://github.com/jdanielllopis/Inchrosil
- **RPi 5**: https://www.raspberrypi.com/products/raspberry-pi-5/
- **ARM A76**: https://developer.arm.com/Processors/Cortex-A76

## ⚡ Performance Tips

```bash
# Set CPU to performance mode
echo performance | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor

# Check CPU governor
cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor

# Monitor in real-time
watch -n 1 vcgencmd measure_temp
```

## 🎓 Learning Path

1. Read `README.md` - Overview
2. Follow `SETUP_GUIDE.md` - Setup
3. Run example - See it work
4. Read code comments - Understand
5. Modify tasks - Experiment
6. Check `RTOS_USAGE_GUIDE.md` - Deep dive

## 🚨 Common Errors

| Error | Fix |
|-------|-----|
| "C++17 required" | Update GCC: `sudo apt install g++-11` |
| "Submodule not found" | Run: `git submodule update --init --recursive` |
| "pthread not found" | Install: `sudo apt install libpthread-stubs0-dev` |
| "No display" (GUI) | Set: `export DISPLAY=:0` |

## 💡 Quick Modifications

### Change worker threads
```cpp
RTOSScheduler scheduler(2);  // Use 2 cores instead of 4
```

### Adjust memory pool
```cpp
RTOSMemoryPool pool(4*1024*1024, 8192);  // 4MB, 8KB blocks
```

### Modify deadline
```cpp
scheduler.scheduleTask(Priority::HIGH, task, 
    std::chrono::milliseconds(100)  // 100ms deadline
);
```

---

**Keep this card handy for quick reference!** 🧬💻
