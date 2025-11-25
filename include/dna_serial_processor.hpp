#ifndef DNA_SERIAL_PROCESSOR_HPP
#define DNA_SERIAL_PROCESSOR_HPP

/**
 * @file dna_serial_processor.hpp (Unified & Optimized)
 * @brief Hardware-Optimized DNA Serial Processing System for Raspberry Pi 5
 * 
 * Unified implementation combining comprehensive API with hardware optimizations.
 * 
 * Optimizations for RPi 5:
 * - 4× Cortex-A76 cores @ 2.4 GHz
 * - 8 GB RAM with cache-aligned allocations
 * - NEON SIMD for parallel processing
 * - Hardware CRC32 and SHA256 acceleration
 * - Cache-aligned structures (64-byte cache line)
 * - Lock-free queues with atomic operations
 * 
 * Performance Targets:
 * - 400-500 KB/s total throughput (4 ports)
 * - < 5ms end-to-end latency
 * - 40% average CPU utilization
 * - 200 MB memory footprint
 * 
 * @version 3.0 (Unified)
 * @date 2025-11-24
 */

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <map>
#include <queue>
#include <memory>
#include <functional>
#include <atomic>
#include <mutex>
#include <thread>
#include <array>
#include <chrono>

// ARM-specific optimizations
#ifdef __aarch64__
#include <arm_neon.h>
#include <arm_acle.h>
#endif

// Cache line size for Cortex-A76
#define CACHE_LINE_SIZE 64
#define CACHE_ALIGNED alignas(CACHE_LINE_SIZE)

// Hardware acceleration macros
#define USE_NEON_SIMD 1
#define USE_HW_CRC32 1
#define USE_HW_CRYPTO 1

namespace DNASerialProcessor {

//=============================================================================
// Hardware Optimization Utilities
//=============================================================================

/**
 * @brief Hardware-accelerated CRC32 calculation
 */
class HardwareCRC32 {
public:
    static uint32_t calculate(const uint8_t* data, size_t len) {
#ifdef __aarch64__
        uint32_t crc = 0xFFFFFFFF;
        
        // Process 8 bytes at a time using ARM CRC32 instruction
        while (len >= 8) {
            uint64_t val = *reinterpret_cast<const uint64_t*>(data);
            crc = __builtin_aarch64_crc32x(crc, val);
            data += 8;
            len -= 8;
        }
        
        // Process remaining bytes
        while (len > 0) {
            crc = __builtin_aarch64_crc32b(crc, *data++);
            len--;
        }
        
        return ~crc;
#else
        // Fallback software implementation
        return calculateSoftware(data, len);
#endif
    }

private:
    static uint32_t calculateSoftware(const uint8_t* data, size_t len);
};

/**
 * @brief NEON SIMD-accelerated nucleotide validation
 */
class NEONValidator {
public:
    /**
     * @brief Validate nucleotides using NEON (16 bytes parallel)
     * @return true if all nucleotides are valid (A, T, C, G, N)
     */
    static bool validateNucleotides(const char* seq, size_t len) {
#ifdef __aarch64__
        const uint8x16_t validA = vdupq_n_u8('A');
        const uint8x16_t validT = vdupq_n_u8('T');
        const uint8x16_t validC = vdupq_n_u8('C');
        const uint8x16_t validG = vdupq_n_u8('G');
        const uint8x16_t validN = vdupq_n_u8('N');
        
        size_t i = 0;
        for (; i + 16 <= len; i += 16) {
            uint8x16_t data = vld1q_u8(reinterpret_cast<const uint8_t*>(seq + i));
            
            // Compare against valid nucleotides
            uint8x16_t isA = vceqq_u8(data, validA);
            uint8x16_t isT = vceqq_u8(data, validT);
            uint8x16_t isC = vceqq_u8(data, validC);
            uint8x16_t isG = vceqq_u8(data, validG);
            uint8x16_t isN = vceqq_u8(data, validN);
            
            // Combine results
            uint8x16_t valid = vorrq_u8(
                vorrq_u8(vorrq_u8(isA, isT), vorrq_u8(isC, isG)),
                isN
            );
            
            // Check if all are valid (all bits set)
            uint64x2_t valid64 = vreinterpretq_u64_u8(valid);
            uint64_t result = vgetq_lane_u64(valid64, 0) & vgetq_lane_u64(valid64, 1);
            
            if (result != 0xFFFFFFFFFFFFFFFFULL) {
                return false;  // Found invalid nucleotide
            }
        }
        
        // Validate remaining bytes
        for (; i < len; i++) {
            char c = seq[i];
            if (c != 'A' && c != 'T' && c != 'C' && c != 'G' && c != 'N') {
                return false;
            }
        }
        
        return true;
#else
        return validateSoftware(seq, len);
#endif
    }

private:
    static bool validateSoftware(const char* seq, size_t len);
};

/**
 * @brief Lock-free ring buffer using atomic operations
 */
template<typename T, size_t SIZE = 4096>
class LockFreeRingBuffer {
private:
    CACHE_ALIGNED std::array<T, SIZE> buffer_;
    CACHE_ALIGNED std::atomic<uint32_t> writePos_{0};
    CACHE_ALIGNED std::atomic<uint32_t> readPos_{0};

public:
    bool push(const T& item) {
        uint32_t write = writePos_.load(std::memory_order_relaxed);
        uint32_t next = (write + 1) % SIZE;
        
        if (next == readPos_.load(std::memory_order_acquire)) {
            return false;  // Buffer full
        }
        
        buffer_[write] = item;
        writePos_.store(next, std::memory_order_release);
        return true;
    }
    
    bool pop(T& item) {
        uint32_t read = readPos_.load(std::memory_order_relaxed);
        
        if (read == writePos_.load(std::memory_order_acquire)) {
            return false;  // Buffer empty
        }
        
        item = buffer_[read];
        readPos_.store((read + 1) % SIZE, std::memory_order_release);
        return true;
    }
    
    size_t size() const {
        uint32_t write = writePos_.load(std::memory_order_acquire);
        uint32_t read = readPos_.load(std::memory_order_acquire);
        return (write >= read) ? (write - read) : (SIZE - read + write);
    }
    
    bool empty() const {
        return readPos_.load(std::memory_order_acquire) == 
               writePos_.load(std::memory_order_acquire);
    }
};

//=============================================================================
// CPU Affinity Management
//=============================================================================

/**
 * @brief Thread pinning for cache locality
 */
class CPUAffinity {
public:
    static bool pinThreadToCore(std::thread& thread, int coreId) {
#ifdef __linux__
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(coreId, &cpuset);
        
        int rc = pthread_setaffinity_np(thread.native_handle(),
                                       sizeof(cpu_set_t), &cpuset);
        return rc == 0;
#else
        return false;
#endif
    }
    
    static bool pinCurrentThreadToCore(int coreId) {
#ifdef __linux__
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(coreId, &cpuset);
        
        return pthread_setaffinity_np(pthread_self(),
                                     sizeof(cpu_set_t), &cpuset) == 0;
#else
        return false;
#endif
    }
};

//=============================================================================
// Cache-Aligned Data Structures
//=============================================================================

/**
 * @brief Cache-aligned DNA buffer for optimal performance
 */
struct CACHE_ALIGNED DNABuffer {
    static constexpr size_t BUFFER_SIZE = 4096 - 64;  // Leave room for metadata
    
    char data[BUFFER_SIZE];
    size_t size;
    uint32_t checksum;
    uint64_t timestamp;
    
    DNABuffer() : size(0), checksum(0), timestamp(0) {}
};

/**
 * @brief Cache-aligned metadata structure
 */
struct CACHE_ALIGNED DNAMetadata {
    char sequenceId[128];
    char description[256];
    char format[32];
    uint64_t originalLength;
    uint64_t encodedLength;
    uint64_t timestamp;
    uint32_t crc32;
    uint8_t sha256[32];
    
    DNAMetadata() : originalLength(0), encodedLength(0), 
                    timestamp(0), crc32(0) {
        sequenceId[0] = '\0';
        description[0] = '\0';
        format[0] = '\0';
    }
};

//=============================================================================
// DNA Format Parsers
//=============================================================================

enum class DNAFormat {
    FASTA,
    FASTQ,
    GENBANK,
    RAW,
    UNKNOWN
};

/**
 * @brief Auto-detect DNA file format
 */
class FormatDetector {
public:
    static DNAFormat detect(const uint8_t* data, size_t size) {
        if (size < 2) return DNAFormat::UNKNOWN;
        
        // Check first character
        if (data[0] == '>') return DNAFormat::FASTA;
        if (data[0] == '@') return DNAFormat::FASTQ;
        
        // Check for GenBank keywords
        if (size >= 5 && strncmp(reinterpret_cast<const char*>(data), 
                                 "LOCUS", 5) == 0) {
            return DNAFormat::GENBANK;
        }
        
        // Check if all valid nucleotides (RAW format)
        if (NEONValidator::validateNucleotides(
                reinterpret_cast<const char*>(data), 
                std::min(size, size_t(1024)))) {
            return DNAFormat::RAW;
        }
        
        return DNAFormat::UNKNOWN;
    }
};

struct FASTASequence {
    std::string id;
    std::string description;
    std::string sequence;
};

struct FASTQRead {
    std::string id;
    std::string sequence;
    std::string quality;
};

/**
 * @brief Optimized FASTA parser
 */
class FASTAParser {
public:
    static std::vector<FASTASequence> parse(const std::string& data) {
        std::vector<FASTASequence> sequences;
        FASTASequence current;
        bool inSequence = false;
        
        size_t start = 0;
        size_t end = 0;
        
        while (end < data.length()) {
            // Find next newline
            end = data.find('\n', start);
            if (end == std::string::npos) end = data.length();
            
            std::string line = data.substr(start, end - start);
            start = end + 1;
            
            if (line.empty()) continue;
            
            if (line[0] == '>') {
                // Save previous sequence
                if (inSequence) {
                    sequences.push_back(current);
                }
                
                // Start new sequence
                current = FASTASequence();
                size_t spacePos = line.find(' ');
                if (spacePos != std::string::npos) {
                    current.id = line.substr(1, spacePos - 1);
                    current.description = line.substr(spacePos + 1);
                } else {
                    current.id = line.substr(1);
                }
                inSequence = true;
            } else {
                // Append to sequence
                current.sequence += line;
            }
        }
        
        // Save last sequence
        if (inSequence) {
            sequences.push_back(current);
        }
        
        return sequences;
    }
};

/**
 * @brief Optimized FASTQ parser
 */
class FASTQParser {
public:
    static std::vector<FASTQRead> parse(const std::string& data) {
        std::vector<FASTQRead> reads;
        
        size_t pos = 0;
        while (pos < data.length()) {
            // Find @ line
            if (data[pos] != '@') {
                pos = data.find('\n', pos) + 1;
                continue;
            }
            
            FASTQRead read;
            
            // ID line
            size_t lineEnd = data.find('\n', pos);
            if (lineEnd == std::string::npos) break;
            read.id = data.substr(pos + 1, lineEnd - pos - 1);
            pos = lineEnd + 1;
            
            // Sequence line
            lineEnd = data.find('\n', pos);
            if (lineEnd == std::string::npos) break;
            read.sequence = data.substr(pos, lineEnd - pos);
            pos = lineEnd + 1;
            
            // + line (skip)
            lineEnd = data.find('\n', pos);
            if (lineEnd == std::string::npos) break;
            pos = lineEnd + 1;
            
            // Quality line
            lineEnd = data.find('\n', pos);
            if (lineEnd == std::string::npos) lineEnd = data.length();
            read.quality = data.substr(pos, lineEnd - pos);
            pos = lineEnd + 1;
            
            reads.push_back(read);
        }
        
        return reads;
    }
};

//=============================================================================
// Serial Port Management
//=============================================================================

enum class SerialParity {
    NONE,
    EVEN,
    ODD
};

struct SerialPortConfig {
    std::string device;          // e.g., "/dev/ttyUSB0"
    int baudRate = 115200;       // Default: 115200
    SerialParity parity = SerialParity::NONE;
    int dataBits = 8;
    int stopBits = 1;
    int coreAffinity = -1;       // CPU core to pin thread (-1 = no pinning)
};

/**
 * @brief Serial port manager with hardware optimization
 */
class SerialPortManager {
public:
    using DataCallback = std::function<void(const std::string&, 
                                           const uint8_t*, size_t)>;
    
    SerialPortManager() = default;
    ~SerialPortManager() { closeAll(); }
    
    bool openPort(const SerialPortConfig& config);
    void closePort(const std::string& device);
    void closeAll();
    
    size_t readData(const std::string& device, 
                   uint8_t* buffer, 
                   size_t maxSize);
    
    void setDataCallback(DataCallback callback) {
        dataCallback_ = callback;
    }
    
    bool isPortOpen(const std::string& device) const {
        return portDescriptors_.find(device) != portDescriptors_.end();
    }
    
    std::vector<std::string> getOpenPorts() const;

private:
    std::map<std::string, int> portDescriptors_;
    std::map<std::string, std::unique_ptr<std::thread>> readerThreads_;
    std::map<std::string, std::atomic<bool>> shouldStop_;
    DataCallback dataCallback_;
    
    void readerThread(const std::string& device, int fd, int coreAffinity);
    bool configurePort(int fd, const SerialPortConfig& config);
};

//=============================================================================
// Storage Management
//=============================================================================

struct StorageConfig {
    std::string basePath = "/data/dna";
    bool storeOriginal = true;
    bool storeDecoded = true;
    bool storeRaw = false;
    bool compressOld = true;
    size_t writeCacheSize = 128 * 1024 * 1024;  // 128 MB
    size_t optimalBlockSize = 262144;            // 256 KB (optimal for NVMe)
    bool enableIndexing = true;
    bool useDirectIO = false;  // O_DIRECT for large sequential writes
};

/**
 * @brief Optimized storage manager with batched writes
 */
class StorageManager {
public:
    explicit StorageManager(const StorageConfig& config);
    ~StorageManager();
    
    bool storeOriginal(const std::string& filename,
                      const std::string& data,
                      const DNAMetadata& metadata);
    
    bool storeEncoded(const std::string& filename,
                     const std::vector<uint8_t>& data,
                     const DNAMetadata& metadata);
    
    bool storeDecoded(const std::string& filename,
                     const std::string& data,
                     const DNAMetadata& metadata);
    
    bool retrieveOriginal(const std::string& filename, std::string& data);
    bool retrieveDecoded(const std::string& filename, std::string& data);
    
    void flush();  // Force write all cached data
    
    uint64_t getTotalBytesWritten() const { 
        return totalBytesWritten_.load(); 
    }
    
    uint64_t getCacheHits() const { 
        return cacheHits_.load(); 
    }

private:
    StorageConfig config_;
    std::atomic<uint64_t> totalBytesWritten_{0};
    std::atomic<uint64_t> cacheHits_{0};
    
    // Write cache
    std::vector<uint8_t> writeCache_;
    std::mutex cacheMutex_;
    std::thread flushThread_;
    std::atomic<bool> shouldStop_{false};
    
    void flushLoop();
    void createDirectoryStructure();
    std::string generateFilePath(const std::string& filename, 
                                 const std::string& type);
};

//=============================================================================
// Main DNA Serial Processor
//=============================================================================

struct ProcessorConfig {
    std::vector<SerialPortConfig> serialPorts;
    StorageConfig storage;
    size_t memoryPoolSize = 32 * 1024 * 1024;  // 32 MB
    bool enablePerformanceMode = true;          // Set CPU governor to performance
    bool enableThermalMonitoring = true;
};

struct ProcessorStats {
    CACHE_ALIGNED std::atomic<uint64_t> totalBytesReceived{0};
    CACHE_ALIGNED std::atomic<uint64_t> totalBytesProcessed{0};
    CACHE_ALIGNED std::atomic<uint64_t> totalSequences{0};
    CACHE_ALIGNED std::atomic<uint64_t> validationErrors{0};
    CACHE_ALIGNED std::atomic<uint64_t> parsingErrors{0};
    CACHE_ALIGNED std::atomic<uint64_t> storageErrors{0};
    
    double getAverageLatencyMs() const;
    double getThroughputKBps() const;
    double getCPUUtilization() const;
};

/**
 * @brief Main DNA serial processor with hardware optimizations
 */
class DNASerialProcessor {
public:
    explicit DNASerialProcessor(const ProcessorConfig& config);
    ~DNASerialProcessor();
    
    bool start();
    void stop();
    
    const ProcessorStats& getStats() const { return stats_; }
    
    // Thermal monitoring
    float getCurrentTemperature() const;
    bool isThrottled() const;

private:
    ProcessorConfig config_;
    ProcessorStats stats_;
    
    std::unique_ptr<SerialPortManager> serialManager_;
    std::unique_ptr<StorageManager> storageManager_;
    
    // Processing queues
    LockFreeRingBuffer<DNABuffer, 1024> parseQueue_;
    LockFreeRingBuffer<DNABuffer, 1024> encodeQueue_;
    LockFreeRingBuffer<DNABuffer, 1024> storeQueue_;
    
    // Worker threads
    std::vector<std::unique_ptr<std::thread>> workers_;
    std::atomic<bool> running_{false};
    
    // Thread functions
    void serialWorker(int portIndex, int coreAffinity);
    void parseWorker(int coreAffinity);
    void encodeWorker(int coreAffinity);
    void storeWorker(int coreAffinity);
    
    // Helper functions
    void setPerformanceMode();
    void restoreNormalMode();
    void monitorThermal();
};

} // namespace DNASerialProcessor

// Backward compatibility aliases
namespace dna_serial = DNASerialProcessor;

#endif // DNA_SERIAL_PROCESSOR_HPP
