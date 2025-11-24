/**
 * @file rpi5_inchrosil_rtos_example.cpp
 * @brief Raspberry Pi 5 DNA Processing Example with RTOS
 * 
 * This example demonstrates real-time DNA encoding/decoding on Raspberry Pi 5
 * using the Inchrosil library with FreeRTOS-like scheduling.
 * 
 * Features:
 * - Multi-core task scheduling (4 cores on RPi 5)
 * - Deterministic memory allocation
 * - Priority-based DNA processing
 * - Real-time performance monitoring
 * - GPIO-ready architecture (future expansion)
 * 
 * Hardware: Raspberry Pi 5 (ARM Cortex-A76, 4 cores)
 * OS: Raspberry Pi OS (Debian-based)
 */

#include <chrono>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <thread>
#include <fstream>

// Inchrosil RTOS components
#include "Inchrosil/include/nucleotide.hpp"
#include "Inchrosil/include/rtos_memory_pool.hpp"
#include "Inchrosil/include/rtos_scheduler.hpp"

using namespace inchrosil;
using namespace inchrosil::rtos;
using namespace nucleotides;

// === Configuration for Raspberry Pi 5 ===
constexpr size_t RPI5_CORES = 4;           // Cortex-A76 cores
constexpr size_t POOL_SIZE = 2 * 1024 * 1024;  // 2MB memory pool
constexpr size_t BLOCK_SIZE = 4096;        // 4KB blocks (cache-aligned)

// === Task Priorities for DNA Processing ===
enum class DNATaskType {
    GENOME_SEQUENCING,    // Critical - highest priority
    ERROR_CORRECTION,     // High priority
    DATA_ENCODING,        // Normal priority
    BACKUP_ARCHIVAL       // Low priority - background
};

// === DNA Processing Tasks ===

/**
 * @brief Critical task: Real-time genome sequencing
 * Simulates reading DNA from sensor/GPIO with tight deadlines
 */
void genomeSequencingTask(RTOSMemoryPool& pool, int sample_id) {
    auto start = std::chrono::high_resolution_clock::now();
    
    // Allocate deterministic memory
    RTOSDNABuffer buffer(pool, 1024);
    
    // Simulate reading genome data
    std::string genome_data = "GENOME_SAMPLE_" + std::to_string(sample_id);
    
    // Convert to binary
    std::string bits;
    for (char c : genome_data) {
        for (int i = 7; i >= 0; --i) {
            bits += ((c >> i) & 1) ? '1' : '0';
        }
    }
    
    // Encode to DNA nucleotides
    std::string dna_sequence = encodeBitsToNucleotides(bits);
    
    // Verify encoding
    std::string decoded = decodeNucleotidesToBits(dna_sequence);
    bool valid = (decoded == bits);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "[CRITICAL] Genome #" << sample_id 
              << " | " << dna_sequence.length() << " nucleotides"
              << " | " << duration.count() << "µs"
              << " | " << (valid ? "✓" : "✗") << std::endl;
}

/**
 * @brief High priority: Error correction in DNA sequences
 */
void errorCorrectionTask(RTOSMemoryPool& pool, const std::string& data) {
    auto start = std::chrono::high_resolution_clock::now();
    
    RTOSDNABuffer buffer(pool, 2048);
    
    // Encode data
    std::string bits;
    for (char c : data) {
        for (int i = 7; i >= 0; --i) {
            bits += ((c >> i) & 1) ? '1' : '0';
        }
    }
    
    std::string encoded = encodeBitsToNucleotides(bits);
    
    // Add redundancy for error correction (simple duplication)
    std::string redundant = encoded + encoded;
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "[HIGH] Error correction: " << data 
              << " | Redundant length: " << redundant.length()
              << " | " << duration.count() << "µs" << std::endl;
}

/**
 * @brief Normal priority: General data encoding
 */
void dataEncodingTask(RTOSMemoryPool& pool, const std::string& message) {
    auto start = std::chrono::high_resolution_clock::now();
    
    RTOSDNABuffer buffer(pool, 1024);
    
    // Encode message to DNA
    std::string bits;
    for (char c : message) {
        for (int i = 7; i >= 0; --i) {
            bits += ((c >> i) & 1) ? '1' : '0';
        }
    }
    
    std::string dna = encodeBitsToNucleotides(bits);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "[NORMAL] Encoded: \"" << message << "\""
              << " | " << dna.length() << " nucleotides"
              << " | " << duration.count() << "µs" << std::endl;
}

/**
 * @brief Low priority: Background archival
 */
void backupArchivalTask(RTOSMemoryPool& pool, int archive_id) {
    auto start = std::chrono::high_resolution_clock::now();
    
    RTOSDNABuffer buffer(pool, 512);
    
    std::string archive_data = "ARCHIVE_" + std::to_string(archive_id) + "_DATA";
    
    std::string bits;
    for (char c : archive_data) {
        for (int i = 7; i >= 0; --i) {
            bits += ((c >> i) & 1) ? '1' : '0';
        }
    }
    
    std::string dna = encodeBitsToNucleotides(bits);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "[LOW] Archived #" << archive_id
              << " | " << dna.length() << " nucleotides"
              << " | " << duration.count() << "µs" << std::endl;
}

/**
 * @brief Display system information
 */
void displaySystemInfo() {
    std::cout << "\n╔════════════════════════════════════════════════╗\n";
    std::cout << "║  Raspberry Pi 5 - DNA Processing with RTOS    ║\n";
    std::cout << "╚════════════════════════════════════════════════╝\n\n";
    
    std::cout << "Hardware Configuration:\n";
    std::cout << "  CPU: ARM Cortex-A76 (4 cores)\n";
    std::cout << "  Cores: " << RPI5_CORES << "\n";
    std::cout << "  Memory Pool: " << POOL_SIZE / 1024 << " KB\n";
    std::cout << "  Block Size: " << BLOCK_SIZE << " bytes\n";
    std::cout << "  Total Blocks: " << POOL_SIZE / BLOCK_SIZE << "\n\n";
}

/**
 * @brief Display performance metrics
 */
void displayMetrics(RTOSScheduler& scheduler, 
                   const std::vector<uint64_t>& task_ids,
                   const std::vector<std::string>& task_names) {
    
    std::cout << "\n╔════════════════════════════════════════════════╗\n";
    std::cout << "║         Performance Metrics                    ║\n";
    std::cout << "╚════════════════════════════════════════════════╝\n\n";
    
    for (size_t i = 0; i < task_ids.size(); ++i) {
        TaskMetrics metrics = scheduler.getTaskMetrics(task_ids[i]);
        
        std::cout << task_names[i] << ":\n";
        std::cout << "  Executions:      " << metrics.total_executions << "\n";
        std::cout << "  Avg Time:        " << metrics.average_execution_time.count() << " µs\n";
        std::cout << "  WCET:            " << metrics.worst_case_execution_time.count() << " µs\n";
        std::cout << "  Jitter Variance: " << std::fixed << std::setprecision(2) 
                  << metrics.jitter_variance << " µs²\n";
        std::cout << "  Deadline Misses: " << metrics.deadline_misses << "\n\n";
    }
    
    std::cout << "Total Deadline Misses: " << scheduler.getTotalDeadlineMisses() << "\n";
}

/**
 * @brief Main application
 */
int main() {
    displaySystemInfo();
    
    // Initialize RTOS components
    std::cout << "Initializing RTOS components...\n";
    
    // Create memory pool optimized for RPi 5 cache
    RTOSMemoryPool dna_pool(POOL_SIZE, BLOCK_SIZE);
    std::cout << "  Memory pool created: " << dna_pool.getAvailableBlocks() << " blocks available\n";
    
    // Create scheduler with 4 worker threads (one per core)
    RTOSScheduler scheduler(RPI5_CORES);
    scheduler.start();
    std::cout << "  RTOS scheduler started with " << RPI5_CORES << " worker threads\n\n";
    
    // Track task IDs for metrics
    std::vector<uint64_t> task_ids;
    std::vector<std::string> task_names;
    
    std::cout << "═══════════════════════════════════════════════\n";
    std::cout << "Starting DNA Processing Tasks...\n";
    std::cout << "═══════════════════════════════════════════════\n\n";
    
    // Schedule CRITICAL priority tasks (genome sequencing)
    std::cout << "Scheduling CRITICAL genome sequencing tasks...\n";
    for (int i = 1; i <= 3; ++i) {
        uint64_t task_id = scheduler.scheduleTask(
            Priority::CRITICAL,
            [&dna_pool, i]() { genomeSequencingTask(dna_pool, i); },
            std::chrono::milliseconds(10)  // 10ms deadline
        );
        task_ids.push_back(task_id);
        task_names.push_back("Genome Sequencing #" + std::to_string(i));
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Schedule HIGH priority tasks (error correction)
    std::cout << "\nScheduling HIGH priority error correction...\n";
    std::vector<std::string> data_samples = {
        "PATIENT_SAMPLE_A",
        "RESEARCH_DATA_B",
        "CLINICAL_TEST_C"
    };
    
    for (const auto& sample : data_samples) {
        uint64_t task_id = scheduler.scheduleTask(
            Priority::HIGH,
            [&dna_pool, sample]() { errorCorrectionTask(dna_pool, sample); },
            std::chrono::milliseconds(50)  // 50ms deadline
        );
        task_ids.push_back(task_id);
        task_names.push_back("Error Correction: " + sample);
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Schedule NORMAL priority tasks (data encoding)
    std::cout << "\nScheduling NORMAL priority data encoding...\n";
    std::vector<std::string> messages = {
        "Hello Raspberry Pi 5",
        "DNA Storage System",
        "Real-Time Computing"
    };
    
    for (const auto& msg : messages) {
        uint64_t task_id = scheduler.scheduleTask(
            Priority::NORMAL,
            [&dna_pool, msg]() { dataEncodingTask(dna_pool, msg); },
            std::chrono::milliseconds(100)  // 100ms deadline
        );
        task_ids.push_back(task_id);
        task_names.push_back("Data Encoding: " + msg.substr(0, 15));
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    
    // Schedule LOW priority tasks (backup/archival)
    std::cout << "\nScheduling LOW priority archival tasks...\n";
    for (int i = 1; i <= 2; ++i) {
        uint64_t task_id = scheduler.scheduleTask(
            Priority::LOW,
            [&dna_pool, i]() { backupArchivalTask(dna_pool, i); },
            std::chrono::milliseconds(500)  // 500ms deadline
        );
        task_ids.push_back(task_id);
        task_names.push_back("Backup Archive #" + std::to_string(i));
    }
    
    // Wait for all tasks to complete
    std::cout << "\nWaiting for tasks to complete...\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // Display performance metrics
    displayMetrics(scheduler, task_ids, task_names);
    
    // Display memory pool utilization
    std::cout << "\n╔════════════════════════════════════════════════╗\n";
    std::cout << "║         Memory Pool Status                     ║\n";
    std::cout << "╚════════════════════════════════════════════════╝\n\n";
    std::cout << "  Pool Utilization: " << std::fixed << std::setprecision(2) 
              << dna_pool.getUtilization() << "%\n";
    std::cout << "  Available Blocks: " << dna_pool.getAvailableBlocks() << "\n";
    std::cout << "  Total Blocks:     " << POOL_SIZE / BLOCK_SIZE << "\n\n";
    
    // Shutdown
    std::cout << "Shutting down RTOS scheduler...\n";
    scheduler.stop();
    
    std::cout << "\n╔════════════════════════════════════════════════╗\n";
    std::cout << "║  DNA Processing Complete                       ║\n";
    std::cout << "╚════════════════════════════════════════════════╝\n\n";
    
    return 0;
}
