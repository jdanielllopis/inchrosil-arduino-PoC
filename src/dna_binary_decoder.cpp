/**
 * @file dna_binary_decoder.cpp
 * @brief DNA Binary Decoder - 2-bit Inchrosil Encoding/Decoding
 * 
 * Demonstrates Inchrosil's 2-bit DNA encoding:
 * - A = 00 (0)
 * - T = 01 (1)
 * - G = 10 (2)
 * - C = 11 (3)
 * 
 * @date 2025-11-24
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
#include <cstdint>
#include <bitset>

// ANSI colors
#define COLOR_RESET   "\033[0m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_RED     "\033[31m"

/**
 * @brief 2-bit nucleotide encoding
 */
enum class Nucleotide : uint8_t {
    A = 0b00,  // Adenine
    T = 0b01,  // Thymine
    G = 0b10,  // Guanine
    C = 0b11   // Cytosine
};

/**
 * @brief Convert nucleotide character to 2-bit value
 */
Nucleotide charToNucleotide(char c) {
    switch(c) {
        case 'A': case 'a': return Nucleotide::A;
        case 'T': case 't': return Nucleotide::T;
        case 'G': case 'g': return Nucleotide::G;
        case 'C': case 'c': return Nucleotide::C;
        default: return Nucleotide::A;  // Default to A
    }
}

/**
 * @brief Convert 2-bit value to nucleotide character
 */
char nucleotideToChar(Nucleotide nt) {
    switch(nt) {
        case Nucleotide::A: return 'A';
        case Nucleotide::T: return 'T';
        case Nucleotide::G: return 'G';
        case Nucleotide::C: return 'C';
        default: return 'N';
    }
}

/**
 * @brief Encode DNA sequence to 2-bit binary
 */
std::vector<uint8_t> encodeDNA(const std::string& sequence) {
    std::vector<uint8_t> encoded;
    
    // Pack 4 nucleotides per byte (4 × 2 bits = 8 bits)
    for (size_t i = 0; i < sequence.length(); i += 4) {
        uint8_t byte = 0;
        
        for (int j = 0; j < 4 && (i + j) < sequence.length(); j++) {
            Nucleotide nt = charToNucleotide(sequence[i + j]);
            byte |= (static_cast<uint8_t>(nt) << (6 - j * 2));
        }
        
        encoded.push_back(byte);
    }
    
    return encoded;
}

/**
 * @brief Decode 2-bit binary to DNA sequence
 */
std::string decodeDNA(const std::vector<uint8_t>& encoded, size_t length) {
    std::string sequence;
    sequence.reserve(length);
    
    size_t nucleotidesDecoded = 0;
    
    for (uint8_t byte : encoded) {
        // Extract 4 nucleotides from each byte (2 bits each)
        for (int j = 0; j < 4 && nucleotidesDecoded < length; j++) {
            uint8_t bits = (byte >> (6 - j * 2)) & 0b11;
            Nucleotide nt = static_cast<Nucleotide>(bits);
            sequence += nucleotideToChar(nt);
            nucleotidesDecoded++;
        }
        
        if (nucleotidesDecoded >= length) break;
    }
    
    return sequence;
}

/**
 * @brief Display binary representation
 */
void displayBinary(const std::vector<uint8_t>& data, size_t maxBytes = 16) {
    std::cout << COLOR_CYAN << "Binary Representation (hex + binary):" << COLOR_RESET << "\n";
    
    size_t count = std::min(data.size(), maxBytes);
    
    for (size_t i = 0; i < count; i++) {
        uint8_t byte = data[i];
        
        // Show hex
        std::cout << "  Byte " << std::setw(2) << i << ": 0x" 
                  << std::hex << std::setw(2) << std::setfill('0') 
                  << static_cast<int>(byte) << std::dec << "  ";
        
        // Show binary with nucleotide grouping
        std::cout << COLOR_YELLOW;
        for (int j = 0; j < 4; j++) {
            uint8_t bits = (byte >> (6 - j * 2)) & 0b11;
            std::cout << ((bits >> 1) & 1) << (bits & 1);
            
            // Color code nucleotides
            std::cout << COLOR_RESET << " (";
            Nucleotide nt = static_cast<Nucleotide>(bits);
            
            switch(nt) {
                case Nucleotide::A: std::cout << COLOR_GREEN << nucleotideToChar(nt); break;
                case Nucleotide::T: std::cout << COLOR_BLUE << nucleotideToChar(nt); break;
                case Nucleotide::G: std::cout << COLOR_MAGENTA << nucleotideToChar(nt); break;
                case Nucleotide::C: std::cout << COLOR_CYAN << nucleotideToChar(nt); break;
            }
            
            std::cout << COLOR_RESET << ") ";
        }
        std::cout << "\n";
    }
    
    if (data.size() > maxBytes) {
        std::cout << "  ... (" << (data.size() - maxBytes) << " more bytes)\n";
    }
}

/**
 * @brief Test encoding/decoding
 */
void testEncodeDecode(const std::string& sequence) {
    std::cout << COLOR_CYAN << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << COLOR_RESET << "\n";
    std::cout << COLOR_YELLOW << "TEST: " << COLOR_RESET << "Encoding/Decoding DNA Sequence\n";
    std::cout << COLOR_CYAN << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << COLOR_RESET << "\n\n";
    
    // Original
    std::cout << COLOR_BLUE << "Original DNA sequence (" << sequence.length() << " nucleotides):" << COLOR_RESET << "\n";
    std::cout << "  " << sequence << "\n\n";
    
    // Encode
    auto encoded = encodeDNA(sequence);
    std::cout << COLOR_BLUE << "Encoded to binary (" << encoded.size() << " bytes):" << COLOR_RESET << "\n";
    displayBinary(encoded);
    
    // Compression stats
    size_t originalBytes = sequence.length();  // 1 byte per char in ASCII
    size_t compressedBytes = encoded.size();
    double ratio = static_cast<double>(originalBytes) / compressedBytes;
    double savings = (1.0 - static_cast<double>(compressedBytes) / originalBytes) * 100.0;
    
    std::cout << "\n" << COLOR_GREEN << "Compression Statistics:" << COLOR_RESET << "\n";
    std::cout << "  Original (ASCII):     " << originalBytes << " bytes\n";
    std::cout << "  Compressed (2-bit):   " << compressedBytes << " bytes\n";
    std::cout << "  Compression ratio:    " << std::fixed << std::setprecision(2) << ratio << ":1\n";
    std::cout << "  Space savings:        " << std::fixed << std::setprecision(1) << savings << "%\n\n";
    
    // Decode
    auto decoded = decodeDNA(encoded, sequence.length());
    std::cout << COLOR_BLUE << "Decoded DNA sequence:" << COLOR_RESET << "\n";
    std::cout << "  " << decoded << "\n\n";
    
    // Verify
    bool match = (sequence == decoded);
    std::cout << COLOR_BLUE << "Verification: " << COLOR_RESET;
    if (match) {
        std::cout << COLOR_GREEN << "✓ PASS - Perfect reconstruction!" << COLOR_RESET << "\n";
    } else {
        std::cout << COLOR_RED << "✗ FAIL - Mismatch detected!" << COLOR_RESET << "\n";
    }
}

/**
 * @brief Read FASTA file
 */
std::vector<std::pair<std::string, std::string>> readFASTA(const std::string& filename) {
    std::vector<std::pair<std::string, std::string>> sequences;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << COLOR_RED << "Error: Cannot open file " << filename << COLOR_RESET << "\n";
        return sequences;
    }
    
    std::string line, id, sequence;
    
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        if (line[0] == '>') {
            // Save previous sequence
            if (!id.empty() && !sequence.empty()) {
                sequences.push_back({id, sequence});
            }
            
            // Start new sequence
            id = line.substr(1);
            sequence.clear();
        } else {
            sequence += line;
        }
    }
    
    // Save last sequence
    if (!id.empty() && !sequence.empty()) {
        sequences.push_back({id, sequence});
    }
    
    file.close();
    return sequences;
}

/**
 * @brief Process FASTA file
 */
void processFASTAFile(const std::string& filename) {
    std::cout << COLOR_CYAN << "\n╔═══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  Processing FASTA File: " << std::left << std::setw(40) << filename << "║\n";
    std::cout << "╚═══════════════════════════════════════════════════════════════╝" << COLOR_RESET << "\n";
    
    auto sequences = readFASTA(filename);
    
    if (sequences.empty()) {
        std::cout << COLOR_RED << "No sequences found in file!" << COLOR_RESET << "\n";
        return;
    }
    
    std::cout << "\nFound " << sequences.size() << " sequence(s)\n";
    
    size_t totalOriginal = 0;
    size_t totalCompressed = 0;
    
    for (size_t i = 0; i < sequences.size(); i++) {
        const auto& [id, seq] = sequences[i];
        
        std::cout << COLOR_CYAN << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << COLOR_RESET << "\n";
        std::cout << COLOR_YELLOW << "Sequence " << (i+1) << ": " << COLOR_RESET << id << "\n";
        std::cout << COLOR_CYAN << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << COLOR_RESET << "\n";
        
        std::cout << COLOR_BLUE << "Length: " << COLOR_RESET << seq.length() << " nucleotides\n";
        
        if (seq.length() <= 100) {
            std::cout << COLOR_BLUE << "Sequence: " << COLOR_RESET << seq << "\n\n";
        } else {
            std::cout << COLOR_BLUE << "Sequence preview: " << COLOR_RESET 
                      << seq.substr(0, 80) << "...\n\n";
        }
        
        // Encode
        auto encoded = encodeDNA(seq);
        
        std::cout << COLOR_GREEN << "Encoding Results:" << COLOR_RESET << "\n";
        std::cout << "  Original (ASCII):     " << seq.length() << " bytes\n";
        std::cout << "  Compressed (2-bit):   " << encoded.size() << " bytes\n";
        std::cout << "  Compression ratio:    " 
                  << std::fixed << std::setprecision(2) 
                  << (static_cast<double>(seq.length()) / encoded.size()) << ":1\n";
        std::cout << "  Space savings:        " 
                  << std::fixed << std::setprecision(1)
                  << ((1.0 - static_cast<double>(encoded.size()) / seq.length()) * 100.0) << "%\n";
        
        if (seq.length() <= 40) {
            std::cout << "\n";
            displayBinary(encoded);
        } else {
            std::cout << "\nBinary preview (first 8 bytes):\n";
            displayBinary(encoded, 8);
        }
        
        // Verify
        auto decoded = decodeDNA(encoded, seq.length());
        bool match = (seq == decoded);
        
        std::cout << "\n" << COLOR_BLUE << "Verification: " << COLOR_RESET;
        if (match) {
            std::cout << COLOR_GREEN << "✓ PASS" << COLOR_RESET << "\n";
        } else {
            std::cout << COLOR_RED << "✗ FAIL" << COLOR_RESET << "\n";
        }
        
        totalOriginal += seq.length();
        totalCompressed += encoded.size();
    }
    
    // Summary
    std::cout << COLOR_CYAN << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << COLOR_RESET << "\n";
    std::cout << COLOR_YELLOW << "SUMMARY" << COLOR_RESET << "\n";
    std::cout << COLOR_CYAN << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << COLOR_RESET << "\n\n";
    
    std::cout << COLOR_GREEN << "Total Statistics:" << COLOR_RESET << "\n";
    std::cout << "  Total sequences:      " << sequences.size() << "\n";
    std::cout << "  Total original:       " << totalOriginal << " bytes\n";
    std::cout << "  Total compressed:     " << totalCompressed << " bytes\n";
    std::cout << "  Overall ratio:        " 
              << std::fixed << std::setprecision(2)
              << (static_cast<double>(totalOriginal) / totalCompressed) << ":1\n";
    std::cout << "  Overall savings:      "
              << std::fixed << std::setprecision(1)
              << ((1.0 - static_cast<double>(totalCompressed) / totalOriginal) * 100.0) << "%\n";
}

int main(int argc, char* argv[]) {
    std::cout << "\n";
    std::cout << COLOR_CYAN << "╔═══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║   DNA BINARY DECODER - 2-bit Inchrosil Encoding               ║\n";
    std::cout << "║   Raspberry Pi 5 Hardware-Optimized                           ║\n";
    std::cout << "╚═══════════════════════════════════════════════════════════════╝" << COLOR_RESET << "\n";
    
    // Test 1: Simple encoding/decoding demonstration
    testEncodeDecode("ATCGATCGATCGATCG");
    
    // Test 2: Longer sequence
    testEncodeDecode("ATCGATCGATCGATCGGGCCTTAACCGGTTAACCGGTTAACCGG");
    
    // Test 3: Process FASTA files if provided
    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            processFASTAFile(argv[i]);
        }
    } else {
        // Process default test files
        std::cout << "\n" << COLOR_YELLOW << "Processing default FASTA files..." << COLOR_RESET << "\n";
        
        if (std::ifstream("test_sequences.fasta").good()) {
            processFASTAFile("test_sequences.fasta");
        }
        
        if (std::ifstream("large_genome.fasta").good()) {
            std::cout << "\n" << COLOR_YELLOW << "Note: Large file detected - processing first 3 sequences only" << COLOR_RESET << "\n";
            
            auto sequences = readFASTA("large_genome.fasta");
            if (!sequences.empty()) {
                std::cout << COLOR_CYAN << "\n╔═══════════════════════════════════════════════════════════════╗\n";
                std::cout << "║  Processing: large_genome.fasta (first 3 of " << sequences.size() << ")       ║\n";
                std::cout << "╚═══════════════════════════════════════════════════════════════╝" << COLOR_RESET << "\n";
                
                size_t totalOrig = 0, totalComp = 0;
                size_t limit = std::min(size_t(3), sequences.size());
                
                for (size_t i = 0; i < limit; i++) {
                    const auto& [id, seq] = sequences[i];
                    auto encoded = encodeDNA(seq);
                    
                    std::cout << "\n" << COLOR_YELLOW << "Sequence " << (i+1) << ": " << COLOR_RESET << id.substr(0, 50) << "...\n";
                    std::cout << "  Length:      " << seq.length() << " nucleotides\n";
                    std::cout << "  Original:    " << seq.length() << " bytes\n";
                    std::cout << "  Compressed:  " << encoded.size() << " bytes\n";
                    std::cout << "  Ratio:       " << std::fixed << std::setprecision(2)
                              << (static_cast<double>(seq.length()) / encoded.size()) << ":1\n";
                    
                    totalOrig += seq.length();
                    totalComp += encoded.size();
                }
                
                std::cout << "\n" << COLOR_GREEN << "Summary (3 sequences):" << COLOR_RESET << "\n";
                std::cout << "  Total original:   " << totalOrig << " bytes (" << (totalOrig/1024.0) << " KB)\n";
                std::cout << "  Total compressed: " << totalComp << " bytes (" << (totalComp/1024.0) << " KB)\n";
                std::cout << "  Overall ratio:    " << std::fixed << std::setprecision(2)
                          << (static_cast<double>(totalOrig) / totalComp) << ":1\n";
            }
        }
    }
    
    std::cout << "\n" << COLOR_CYAN << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << COLOR_RESET << "\n";
    std::cout << COLOR_YELLOW << "Encoding Scheme (2-bit per nucleotide):" << COLOR_RESET << "\n";
    std::cout << "  " << COLOR_GREEN << "A (Adenine)  = 00" << COLOR_RESET << "\n";
    std::cout << "  " << COLOR_BLUE << "T (Thymine)  = 01" << COLOR_RESET << "\n";
    std::cout << "  " << COLOR_MAGENTA << "G (Guanine)  = 10" << COLOR_RESET << "\n";
    std::cout << "  " << COLOR_CYAN << "C (Cytosine) = 11" << COLOR_RESET << "\n";
    std::cout << "\n  → 4 nucleotides per byte (4 × 2 bits = 8 bits)\n";
    std::cout << "  → 4:1 compression ratio vs ASCII\n";
    std::cout << COLOR_CYAN << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << COLOR_RESET << "\n\n";
    
    std::cout << COLOR_GREEN << "✓ All tests completed!\n" << COLOR_RESET << "\n";
    
    return 0;
}
