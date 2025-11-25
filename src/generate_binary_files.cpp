/**
 * @file generate_binary_files.cpp
 * @brief Generate binary encoded DNA files from FASTA input
 * 
 * Creates .bin files with 2-bit DNA encoding:
 * - A = 00, T = 01, G = 10, C = 11
 * - 4 nucleotides per byte
 * - Includes metadata header
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
#include <filesystem>

namespace fs = std::filesystem;

// Binary file header structure
struct BinaryHeader {
    char magic[8];           // "INCHRSIL" magic number
    uint32_t version;        // File format version
    uint64_t sequence_count; // Number of sequences
    uint64_t total_bases;    // Total nucleotides
    uint64_t compressed_size;// Size of compressed data
    char reserved[32];       // Reserved for future use
} __attribute__((packed));

// Sequence metadata
struct SequenceInfo {
    uint64_t length;         // Sequence length in bases
    uint64_t offset;         // Offset in data section
    char name[256];          // Sequence name
} __attribute__((packed));

/**
 * @brief Encode DNA sequence to 2-bit binary
 */
std::vector<uint8_t> encodeDNA(const std::string& sequence) {
    size_t encoded_size = (sequence.length() + 3) / 4;  // 4 nucleotides per byte
    std::vector<uint8_t> encoded(encoded_size, 0);
    
    for (size_t i = 0; i < sequence.length(); i++) {
        uint8_t bits;
        switch(sequence[i]) {
            case 'A': case 'a': bits = 0b00; break;
            case 'T': case 't': bits = 0b01; break;
            case 'G': case 'g': bits = 0b10; break;
            case 'C': case 'c': bits = 0b11; break;
            default: bits = 0b00; break;
        }
        
        size_t byte_idx = i / 4;
        size_t bit_pos = (3 - (i % 4)) * 2;  // MSB first
        encoded[byte_idx] |= (bits << bit_pos);
    }
    
    return encoded;
}

/**
 * @brief Read FASTA file
 */
struct FastaSequence {
    std::string name;
    std::string sequence;
};

std::vector<FastaSequence> readFASTA(const std::string& filename) {
    std::vector<FastaSequence> sequences;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        return sequences;
    }
    
    FastaSequence current;
    std::string line;
    
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        if (line[0] == '>') {
            if (!current.sequence.empty()) {
                sequences.push_back(current);
                current.sequence.clear();
            }
            current.name = line.substr(1);
        } else {
            current.sequence += line;
        }
    }
    
    if (!current.sequence.empty()) {
        sequences.push_back(current);
    }
    
    file.close();
    return sequences;
}

/**
 * @brief Generate binary file from FASTA
 */
bool generateBinaryFile(const std::string& fasta_file, const std::string& output_file) {
    // Read FASTA
    auto sequences = readFASTA(fasta_file);
    if (sequences.empty()) {
        std::cerr << "No sequences found in " << fasta_file << std::endl;
        return false;
    }
    
    // Calculate totals
    uint64_t total_bases = 0;
    std::vector<std::vector<uint8_t>> encoded_sequences;
    
    for (const auto& seq : sequences) {
        total_bases += seq.sequence.length();
        encoded_sequences.push_back(encodeDNA(seq.sequence));
    }
    
    uint64_t compressed_size = 0;
    for (const auto& enc : encoded_sequences) {
        compressed_size += enc.size();
    }
    
    // Create output file
    std::ofstream out(output_file, std::ios::binary);
    if (!out.is_open()) {
        std::cerr << "Error: Cannot create output file " << output_file << std::endl;
        return false;
    }
    
    // Write header
    BinaryHeader header;
    std::memcpy(header.magic, "INCHROSIL", 8);
    header.version = 1;
    header.sequence_count = sequences.size();
    header.total_bases = total_bases;
    header.compressed_size = compressed_size;
    std::memset(header.reserved, 0, 32);
    
    out.write(reinterpret_cast<const char*>(&header), sizeof(header));
    
    // Write sequence metadata
    uint64_t data_offset = 0;
    for (size_t i = 0; i < sequences.size(); i++) {
        SequenceInfo info;
        info.length = sequences[i].sequence.length();
        info.offset = data_offset;
        std::memset(info.name, 0, 256);
        std::strncpy(info.name, sequences[i].name.c_str(), 255);
        
        out.write(reinterpret_cast<const char*>(&info), sizeof(info));
        data_offset += encoded_sequences[i].size();
    }
    
    // Write encoded data
    for (const auto& enc : encoded_sequences) {
        out.write(reinterpret_cast<const char*>(enc.data()), enc.size());
    }
    
    out.close();
    
    // Print summary
    std::cout << "\n✅ Generated: " << output_file << std::endl;
    std::cout << "   Sequences:  " << sequences.size() << std::endl;
    std::cout << "   Total bases: " << total_bases << " bp" << std::endl;
    std::cout << "   ASCII size:  " << total_bases << " bytes" << std::endl;
    std::cout << "   Binary size: " << compressed_size << " bytes" << std::endl;
    std::cout << "   Header size: " << (sizeof(BinaryHeader) + sequences.size() * sizeof(SequenceInfo)) << " bytes" << std::endl;
    std::cout << "   Total size:  " << (sizeof(BinaryHeader) + sequences.size() * sizeof(SequenceInfo) + compressed_size) << " bytes" << std::endl;
    
    double ratio = static_cast<double>(total_bases) / compressed_size;
    std::cout << "   Compression: " << std::fixed << std::setprecision(2) 
              << ratio << ":1 (" << (100.0 * (1.0 - 1.0/ratio)) << "% savings)" << std::endl;
    
    return true;
}

/**
 * @brief Format file size
 */
std::string formatSize(uint64_t bytes) {
    const char* units[] = {"B", "KB", "MB", "GB"};
    int unit = 0;
    double size = bytes;
    
    while (size >= 1024 && unit < 3) {
        size /= 1024;
        unit++;
    }
    
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << size << " " << units[unit];
    return oss.str();
}

int main(int argc, char* argv[]) {
    std::cout << "╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║        DNA Binary File Generator - Inchrosil Format          ║\n";
    std::cout << "║            Raspberry Pi 5 - November 24, 2025                ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n\n";
    
    if (argc > 1) {
        // Process specified files
        for (int i = 1; i < argc; i++) {
            std::string fasta_file = argv[i];
            std::string output_file = fasta_file;
            
            // Replace .fasta extension with .bin
            size_t ext_pos = output_file.rfind(".fasta");
            if (ext_pos != std::string::npos) {
                output_file = output_file.substr(0, ext_pos) + ".bin";
            } else {
                output_file += ".bin";
            }
            
            generateBinaryFile(fasta_file, output_file);
        }
    } else {
        // Auto-discover FASTA files
        std::cout << "🔍 Searching for FASTA files...\n\n";
        
        std::vector<std::string> fasta_files;
        for (const auto& entry : fs::directory_iterator(".")) {
            if (entry.is_regular_file()) {
                std::string filename = entry.path().filename().string();
                size_t len = filename.length();
                if ((len > 6 && filename.substr(len-6) == ".fasta") || 
                    (len > 3 && filename.substr(len-3) == ".fa")) {
                    fasta_files.push_back(filename);
                }
            }
        }
        
        if (fasta_files.empty()) {
            std::cout << "No FASTA files found in current directory.\n";
            std::cout << "\nUsage: " << argv[0] << " [file1.fasta] [file2.fasta] ...\n";
            return 1;
        }
        
        std::cout << "Found " << fasta_files.size() << " FASTA file(s):\n";
        for (const auto& file : fasta_files) {
            std::cout << "  • " << file << std::endl;
        }
        std::cout << "\n";
        
        // Process all found FASTA files
        for (const auto& fasta_file : fasta_files) {
            std::string output_file = fasta_file;
            size_t ext_pos = output_file.rfind('.');
            if (ext_pos != std::string::npos) {
                output_file = output_file.substr(0, ext_pos) + ".bin";
            } else {
                output_file += ".bin";
            }
            
            generateBinaryFile(fasta_file, output_file);
        }
    }
    
    std::cout << "\n✅ Binary file generation complete!\n";
    return 0;
}
