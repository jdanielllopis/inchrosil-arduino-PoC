# Raspberry Pi 5 Setup Guide for Inchrosil RTOS

Complete setup guide for running the Inchrosil DNA processing example with RTOS on Raspberry Pi 5.

## Prerequisites

### Hardware
- Raspberry Pi 5 (4GB or 8GB RAM recommended)
- MicroSD card (32GB+, Class 10 or better)
- Power supply (5V/5A USB-C for RPi 5)
- (Optional) Monitor, keyboard, mouse for initial setup

### Software
- Raspberry Pi OS (64-bit recommended)
- Git
- CMake 3.15+
- GCC/G++ with C++17 support

## Step-by-Step Setup

### 1. Install Raspberry Pi OS

1. Download Raspberry Pi Imager: https://www.raspberrypi.com/software/
2. Flash Raspberry Pi OS (64-bit) to your SD card
3. Boot your Raspberry Pi 5

### 2. Update System

```bash
sudo apt-get update
sudo apt-get upgrade -y
```

### 3. Install Development Tools

```bash
# Install essential build tools
sudo apt-get install -y build-essential cmake git

# Verify installations
gcc --version      # Should be 11+ for good C++17 support
cmake --version    # Should be 3.15+
git --version
```

### 4. Clone the Repository

```bash
cd ~
git clone https://github.com/jdanielllopis/inchrosil-arduino-PoC.git
cd inchrosil-arduino-PoC
```

### 5. Initialize Submodules

```bash
git submodule update --init --recursive
```

This downloads the Inchrosil library into the project.

### 6. Build the Project

**Option A: Using build script (recommended)**

```bash
chmod +x build.sh
./build.sh
```

**Option B: Using CMake manually**

```bash
mkdir -p build
cd build
cmake ..
make -j4
```

**Option C: Using Makefile**

```bash
make -j4
```

### 7. Run the Example

```bash
# If using CMake/build script
./build/rpi5_dna_rtos

# If using Makefile
./rpi5_dna_rtos

# Or use make run
make run
```

## Expected Output

You should see output similar to:

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

Initializing RTOS components...
  Memory pool created: 512 blocks available
  RTOS scheduler started with 4 worker threads

═══════════════════════════════════════════════
Starting DNA Processing Tasks...
═══════════════════════════════════════════════

Scheduling CRITICAL genome sequencing tasks...
[CRITICAL] Genome #1 | 112 nucleotides | 156µs | ✓
[CRITICAL] Genome #2 | 112 nucleotides | 142µs | ✓
[CRITICAL] Genome #3 | 112 nucleotides | 138µs | ✓

Scheduling HIGH priority error correction...
[HIGH] Error correction: PATIENT_SAMPLE_A | 272 nucleotides | 189µs
[HIGH] Error correction: RESEARCH_DATA_B | 272 nucleotides | 176µs
[HIGH] Error correction: CLINICAL_TEST_C | 264 nucleotides | 171µs

...
```

## Performance Monitoring

### CPU Usage

Monitor CPU usage while running:

```bash
# In another terminal
htop
```

You should see ~100% usage across all 4 cores during task execution.

### Temperature Monitoring

Check RPi 5 temperature:

```bash
vcgencmd measure_temp
```

Normal operating temperature: 40-70°C under load.

### Memory Usage

```bash
free -h
```

The example uses ~2MB for the RTOS memory pool plus overhead.

## Customization

### Adjust Number of Worker Threads

Edit `rpi5_inchrosil_rtos_example.cpp`:

```cpp
constexpr size_t RPI5_CORES = 4;  // Change to 2 or 3 if desired
```

### Modify Memory Pool Size

```cpp
constexpr size_t POOL_SIZE = 2 * 1024 * 1024;  // 2MB
constexpr size_t BLOCK_SIZE = 4096;            // 4KB blocks
```

### Change Task Priorities

Adjust deadlines in the scheduling code:

```cpp
scheduler.scheduleTask(
    Priority::CRITICAL,
    [&]() { genomeSequencingTask(pool, i); },
    std::chrono::milliseconds(10)  // Adjust deadline
);
```

## Troubleshooting

### Build Fails with "C++17 required"

Update GCC:
```bash
sudo apt-get install g++-11
export CXX=g++-11
```

### Submodule Not Found

Re-initialize submodules:
```bash
git submodule update --init --recursive --force
```

### Threading Errors

Ensure pthread is available:
```bash
sudo apt-get install libpthread-stubs0-dev
```

### Out of Memory

Reduce pool size in the code:
```cpp
constexpr size_t POOL_SIZE = 1 * 1024 * 1024;  // 1MB instead of 2MB
```

### Slow Performance

1. Check CPU governor:
```bash
cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
```

2. Set to performance mode:
```bash
echo performance | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor
```

## Advanced Configuration

### Enable Debug Symbols

```bash
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j4
```

### Profile with gprof

```bash
cmake .. -DCMAKE_CXX_FLAGS="-pg"
make -j4
./rpi5_dna_rtos
gprof rpi5_dna_rtos gmon.out > analysis.txt
```

### Optimize for Size

```bash
cmake .. -DCMAKE_CXX_FLAGS="-Os"
make -j4
```

## Running on Boot (Optional)

To run the example automatically on boot:

1. Create a systemd service:

```bash
sudo nano /etc/systemd/system/inchrosil-rtos.service
```

2. Add content:

```ini
[Unit]
Description=Inchrosil RTOS DNA Processing
After=network.target

[Service]
Type=simple
User=pi
WorkingDirectory=/home/pi/inchrosil-arduino-PoC
ExecStart=/home/pi/inchrosil-arduino-PoC/build/rpi5_dna_rtos
Restart=on-failure

[Install]
WantedBy=multi-user.target
```

3. Enable and start:

```bash
sudo systemctl daemon-reload
sudo systemctl enable inchrosil-rtos
sudo systemctl start inchrosil-rtos
```

## Next Steps

1. **Explore the code**: Review `rpi5_inchrosil_rtos_example.cpp`
2. **Modify tasks**: Add custom DNA processing tasks
3. **Add GPIO**: Integrate with GPIO for real sensors
4. **Extend RTOS**: Add periodic tasks or preemption
5. **Benchmark**: Test with larger DNA sequences

## Resources

- [Inchrosil Documentation](Inchrosil/DOCUMENTATION.md)
- [RTOS Usage Guide](Inchrosil/RTOS_USAGE_GUIDE.md)
- [Raspberry Pi 5 Documentation](https://www.raspberrypi.com/documentation/)
- [ARM Cortex-A76 Manual](https://developer.arm.com/documentation/)

## Support

For issues specific to:
- **This example**: Open an issue on GitHub
- **Inchrosil library**: See [Inchrosil repository](https://github.com/jdanielllopis/Inchrosil)
- **Raspberry Pi 5**: Visit [Raspberry Pi Forums](https://forums.raspberrypi.com/)

---

Happy DNA computing! 🧬
