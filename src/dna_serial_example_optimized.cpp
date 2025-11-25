/**
 * @file dna_serial_example_optimized.cpp
 * @brief Hardware-Optimized DNA Serial Processing Example for Raspberry Pi 5
 * 
 * This example demonstrates the optimized DNA serial processing system with:
 * - NEON SIMD acceleration
 * - Hardware CRC32/SHA256
 * - Cache-aligned structures
 * - Thread pinning for cache locality
 * - Lock-free queues
 * 
 * Compile with:
 *   g++ -std=c++17 -O3 -march=armv8.2-a -mtune=cortex-a76 \
 *       -pthread -o dna_serial_optimized dna_serial_example_optimized.cpp
 * 
 * @version 2.0
 * @date 2025-11-24
 */

#include "dna_serial_processor.hpp"
#include <iostream>
#include <iomanip>
#include <cstring>
#include <signal.h>
#include <unistd.h>

using namespace DNASerialProcessor;

// Global processor instance for signal handling
DNASerialProcessor::DNASerialProcessor* g_processor = nullptr;

void signalHandler(int signal) {
    std::cout << "\nReceived signal " << signal << ", shutting down..." << std::endl;
    if (g_processor) {
        g_processor->stop();
    }
}

void printSystemInfo() {
    std::cout << "========================================" << std::endl;
    std::cout << "DNA Serial Processor - Hardware Optimized" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Platform: Raspberry Pi 5" << std::endl;
    std::cout << "CPU: 4× Cortex-A76 @ 2.4 GHz" << std::endl;
    std::cout << "Optimizations:" << std::endl;
    std::cout << "  - NEON SIMD: " << (USE_NEON_SIMD ? "Enabled" : "Disabled") << std::endl;
    std::cout << "  - HW CRC32: " << (USE_HW_CRC32 ? "Enabled" : "Disabled") << std::endl;
    std::cout << "  - HW Crypto: " << (USE_HW_CRYPTO ? "Enabled" : "Disabled") << std::endl;
    std::cout << "  - Cache Line: " << CACHE_LINE_SIZE << " bytes" << std::endl;
    std::cout << "========================================\n" << std::endl;
}

void printConfiguration(const ProcessorConfig& config) {
    std::cout << "Configuration:" << std::endl;
    std::cout << "  Serial Ports: " << config.serialPorts.size() << std::endl;
    for (size_t i = 0; i < config.serialPorts.size(); i++) {
        const auto& port = config.serialPorts[i];
        std::cout << "    Port " << i << ": " << port.device 
                  << " @ " << port.baudRate << " baud"
                  << " (core: " << port.coreAffinity << ")" << std::endl;
    }
    std::cout << "  Storage Path: " << config.storage.basePath << std::endl;
    std::cout << "  Memory Pool: " << (config.memoryPoolSize / 1024 / 1024) << " MB" << std::endl;
    std::cout << "  Write Cache: " << (config.storage.writeCacheSize / 1024 / 1024) << " MB" << std::endl;
    std::cout << "  Optimal Block: " << (config.storage.optimalBlockSize / 1024) << " KB" << std::endl;
    std::cout << "  Performance Mode: " << (config.enablePerformanceMode ? "Yes" : "No") << std::endl;
    std::cout << "  Thermal Monitor: " << (config.enableThermalMonitoring ? "Yes" : "No") << std::endl;
    std::cout << std::endl;
}

void printStats(const ProcessorStats& stats, float temperature) {
    std::cout << "\r";
    std::cout << "Received: " << std::setw(10) << stats.totalBytesReceived.load() << " bytes | ";
    std::cout << "Processed: " << std::setw(10) << stats.totalBytesProcessed.load() << " bytes | ";
    std::cout << "Sequences: " << std::setw(8) << stats.totalSequences.load() << " | ";
    std::cout << "Errors: " << std::setw(6) << stats.validationErrors.load() << " | ";
    std::cout << "Temp: " << std::fixed << std::setprecision(1) << temperature << "°C | ";
    std::cout << "Throughput: " << std::setw(6) << std::setprecision(1) 
              << stats.getThroughputKBps() << " KB/s | ";
    std::cout << "CPU: " << std::setw(4) << std::setprecision(1) 
              << stats.getCPUUtilization() << "% ";
    std::cout << std::flush;
}

void testHardwareAcceleration() {
    std::cout << "Testing Hardware Acceleration..." << std::endl;
    
    // Test NEON validation
    const char* testSeq = "ATCGATCGATCGATCGATCGATCGATCGATCG";
    bool valid = NEONValidator::validateNucleotides(testSeq, strlen(testSeq));
    std::cout << "  NEON Validation: " << (valid ? "PASS" : "FAIL") << std::endl;
    
    // Test hardware CRC32
    uint32_t crc = HardwareCRC32::calculate(
        reinterpret_cast<const uint8_t*>(testSeq), 
        strlen(testSeq)
    );
    std::cout << "  Hardware CRC32: 0x" << std::hex << crc << std::dec << std::endl;
    
    // Test format detection
    DNAFormat format = FormatDetector::detect(
        reinterpret_cast<const uint8_t*>(">seq1\nATCG"), 11
    );
    std::cout << "  Format Detection: " << (format == DNAFormat::FASTA ? "PASS" : "FAIL") << std::endl;
    
    std::cout << std::endl;
}

void runExample() {
    printSystemInfo();
    testHardwareAcceleration();
    
    // Create configuration
    ProcessorConfig config;
    
    // Configure serial ports (example: 4 virtual ports for testing)
    // In production, use actual serial devices like /dev/ttyUSB0
    for (int i = 0; i < 4; i++) {
        SerialPortConfig portConfig;
        portConfig.device = "/dev/ttyUSB" + std::to_string(i);
        portConfig.baudRate = 115200;
        portConfig.coreAffinity = i;  // Pin to core i
        config.serialPorts.push_back(portConfig);
    }
    
    // Configure storage
    config.storage.basePath = "./dna_data";
    config.storage.storeOriginal = true;
    config.storage.storeDecoded = true;
    config.storage.writeCacheSize = 128 * 1024 * 1024;  // 128 MB
    config.storage.optimalBlockSize = 262144;            // 256 KB
    config.storage.enableIndexing = true;
    
    // Configure processor
    config.memoryPoolSize = 32 * 1024 * 1024;  // 32 MB
    config.enablePerformanceMode = true;
    config.enableThermalMonitoring = true;
    
    printConfiguration(config);
    
    // Create processor
    std::cout << "Starting DNA Serial Processor..." << std::endl;
    DNASerialProcessor::DNASerialProcessor processor(config);
    g_processor = &processor;
    
    // Set up signal handlers
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    // Start processing
    if (!processor.start()) {
        std::cerr << "Failed to start processor!" << std::endl;
        return;
    }
    
    std::cout << "Processor started. Press Ctrl+C to stop.\n" << std::endl;
    
    // Main monitoring loop
    auto startTime = std::chrono::steady_clock::now();
    while (true) {
        sleep(1);
        
        // Get current stats and temperature
        const auto& stats = processor.getStats();
        float temperature = processor.getCurrentTemperature();
        
        // Print stats
        printStats(stats, temperature);
        
        // Check thermal throttling
        if (processor.isThrottled()) {
            std::cout << "\n[WARNING] CPU thermal throttling detected!" << std::endl;
        }
        
        // Calculate runtime
        auto now = std::chrono::steady_clock::now();
        auto runtime = std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count();
        
        // Exit conditions (for testing)
        if (runtime > 300) {  // 5 minutes
            std::cout << "\n\nTest duration complete. Shutting down..." << std::endl;
            break;
        }
    }
    
    // Stop processor
    processor.stop();
    
    // Print final statistics
    std::cout << "\n\n========================================" << std::endl;
    std::cout << "Final Statistics:" << std::endl;
    std::cout << "========================================" << std::endl;
    const auto& finalStats = processor.getStats();
    std::cout << "Total Bytes Received: " << finalStats.totalBytesReceived.load() << std::endl;
    std::cout << "Total Bytes Processed: " << finalStats.totalBytesProcessed.load() << std::endl;
    std::cout << "Total Sequences: " << finalStats.totalSequences.load() << std::endl;
    std::cout << "Validation Errors: " << finalStats.validationErrors.load() << std::endl;
    std::cout << "Parsing Errors: " << finalStats.parsingErrors.load() << std::endl;
    std::cout << "Storage Errors: " << finalStats.storageErrors.load() << std::endl;
    std::cout << "Average Latency: " << std::fixed << std::setprecision(2) 
              << finalStats.getAverageLatencyMs() << " ms" << std::endl;
    std::cout << "Average Throughput: " << std::fixed << std::setprecision(1) 
              << finalStats.getThroughputKBps() << " KB/s" << std::endl;
    std::cout << "Average CPU: " << std::fixed << std::setprecision(1) 
              << finalStats.getCPUUtilization() << "%" << std::endl;
    std::cout << "========================================" << std::endl;
}

int main(int argc, char* argv[]) {
    try {
        runExample();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
