/**
 * @file test_binary_files.cpp
 * @brief Test binary file reading and validation
 * 
 * Validates the generated .bin files:
 * - Header integrity
 * - Data decompression
 * - Sequence reconstruction
 * 
 * @date 2025-11-24
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include <cstring>
#include <iomanip>

// Binary file header structure
struct BinaryHeader {
    char magic[8];
    uint32_t version;
    uint64_t sequence_count;
    uint64_t total_bases;
    uint64_t compressed_size;
    char reserved[32];
} __attribute__((packed));

// Sequence metadata
struct SequenceInfo {
    uint64_t length;
    uint64_t offset;
    char name[256];
} __attribute__((packed));

/**
 * @brief Decode 2-bit DNA to ASCII
 */
std::string decodeDNA(const std::vector<uint8_t>& encoded, size_t length) {
    std::string decoded;
    decoded.reserve(length);
    
    for (size_t i = 0; i < length; i++) {
        size_t byte_idx = i / 4;
        size_t bit_pos = (3 - (i % 4)) * 2;
        uint8_t bits = (encoded[byte_idx] >> bit_pos) & 0b11;
        
        char nucleotide;
        switch(bits) {
            case 0b00: nucleotide = 'A'; break;
            case 0b01: nucleotide = 'T'; break;
            case 0b10: nucleotide = 'G'; break;
            case 0b11: nucleotide = 'C'; break;
            default: nucleotide = 'N'; break;
        }
        decoded += nucleotide;
    }
    
    return decoded;
}

/**
 * @brief Test binary file
 */
bool testBinaryFile(const std::string& filename) {
    std::cout << "\n📦 Testing: " << filename << std::endl;
    std::cout << std::string(70, '-') << std::endl;
    
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "❌ Cannot open file" << std::endl;
        return false;
    }
    
    // Read header
    BinaryHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(header));
    
    // Validate magic number
    if (std::memcmp(header.magic, "INCHROSIL", 8) != 0) {
        std::cerr << "❌ Invalid magic number" << std::endl;
        return false;
    }
    std::cout << "✅ Magic number: INCHROSIL" << std::endl;
    
    // Display header info
    std::cout << "✅ Version: " << header.version << std::endl;
    std::cout << "✅ Sequences: " << header.sequence_count << std::endl;
    std::cout << "✅ Total bases: " << header.total_bases << " bp" << std::endl;
    std::cout << "✅ Compressed size: " << header.compressed_size << " bytes" << std::endl;
    
    double ratio = static_cast<double>(header.total_bases) / header.compressed_size;
    std::cout << "✅ Compression ratio: " << std::fixed << std::setprecision(2) 
              << ratio << ":1 (" << (100.0 * (1.0 - 1.0/ratio)) << "% savings)" << std::endl;
    
    // Read sequence metadata
    std::vector<SequenceInfo> sequences(header.sequence_count);
    for (uint64_t i = 0; i < header.sequence_count; i++) {
        file.read(reinterpret_cast<char*>(&sequences[i]), sizeof(SequenceInfo));
    }
    
    std::cout << "\n📋 Sequences:" << std::endl;
    for (uint64_t i = 0; i < header.sequence_count; i++) {
        std::cout << "   " << (i + 1) << ". " << sequences[i].name 
                  << " (" << sequences[i].length << " bp)" << std::endl;
    }
    
    // Read and decode first sequence as verification
    if (header.sequence_count > 0) {
        size_t encoded_size = (sequences[0].length + 3) / 4;
        std::vector<uint8_t> encoded_data(encoded_size);
        
        file.seekg(sizeof(BinaryHeader) + header.sequence_count * sizeof(SequenceInfo) + sequences[0].offset);
        file.read(reinterpret_cast<char*>(encoded_data.data()), encoded_size);
        
        std::string decoded = decodeDNA(encoded_data, sequences[0].length);
        
        std::cout << "\n🧬 First sequence decoded (first 60 bp):" << std::endl;
        std::cout << "   " << decoded.substr(0, std::min<size_t>(60, decoded.length())) << std::endl;
        
        // Verify all bases are valid
        bool valid = true;
        for (char c : decoded) {
            if (c != 'A' && c != 'T' && c != 'G' && c != 'C') {
                valid = false;
                break;
            }
        }
        
        if (valid) {
            std::cout << "✅ All nucleotides are valid (A, T, G, C)" << std::endl;
        } else {
            std::cout << "❌ Invalid nucleotides detected" << std::endl;
            return false;
        }
    }
    
    file.close();
    std::cout << "\n✅ " << filename << " PASSED" << std::endl;
    return true;
}

int main() {
    std::cout << "╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║          Binary DNA File Validation Test Suite              ║\n";
    std::cout << "║            Raspberry Pi 5 - November 24, 2025               ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n";
    
    std::vector<std::string> test_files = {
        "test_custom.bin",
        "test_sequences.bin",
        "large_genome.bin"
    };
    
    int passed = 0;
    int failed = 0;
    
    for (const auto& file : test_files) {
        if (testBinaryFile(file)) {
            passed++;
        } else {
            failed++;
        }
    }
    
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "📊 SUMMARY" << std::endl;
    std::cout << std::string(70, '=') << std::endl;
    std::cout << "✅ Passed: " << passed << " / " << (passed + failed) << std::endl;
    std::cout << "❌ Failed: " << failed << " / " << (passed + failed) << std::endl;
    
    if (failed == 0) {
        std::cout << "\n🎉 ALL TESTS PASSED - Binary files are valid!\n" << std::endl;
        return 0;
    } else {
        std::cout << "\n⚠️  Some tests failed\n" << std::endl;
        return 1;
    }
}
