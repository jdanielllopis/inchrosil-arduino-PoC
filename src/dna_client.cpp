/**
 * @file dna_client.cpp
 * @brief DNA Serial Processing Client - Slave Node
 * 
 * Client that sends DNA sequences to the server for processing.
 * Can send FASTA, FASTQ, or raw DNA sequences.
 * 
 * Features:
 * - TCP client connecting to server
 * - Multiple send modes (file, interactive, stress test)
 * - Progress tracking
 * - Error handling and reconnection
 * 
 * Compile:
 *   g++ -std=c++17 -O3 -pthread -o dna_client dna_client.cpp
 * 
 * Usage:
 *   ./dna_client <server_ip> [port]
 *   ./dna_client localhost 9090
 *   ./dna_client 192.168.1.100 9090 --file genome.fasta
 *   ./dna_client localhost 9090 --interactive
 *   ./dna_client localhost 9090 --stress 1000
 * 
 * @version 1.0
 * @date 2025-11-24
 */

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <thread>
#include <chrono>
#include <random>
#include <algorithm>
#include <cstring>

// Network includes
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

//=============================================================================
// Configuration
//=============================================================================

constexpr int DEFAULT_PORT = 9090;
constexpr int BUFFER_SIZE = 65536;

//=============================================================================
// DNA Client
//=============================================================================

class DNAClient {
private:
    std::string serverHost_;
    int serverPort_;
    int socket_;
    bool connected_;

public:
    DNAClient(const std::string& host, int port) 
        : serverHost_(host), serverPort_(port), socket_(-1), connected_(false) {}
    
    ~DNAClient() {
        disconnect();
    }
    
    bool connect() {
        // Create socket
        socket_ = socket(AF_INET, SOCK_STREAM, 0);
        if (socket_ < 0) {
            std::cerr << "Failed to create socket" << std::endl;
            return false;
        }
        
        // Resolve hostname
        struct hostent* server = gethostbyname(serverHost_.c_str());
        if (server == nullptr) {
            std::cerr << "Failed to resolve hostname: " << serverHost_ << std::endl;
            close(socket_);
            return false;
        }
        
        // Setup server address
        struct sockaddr_in serverAddr;
        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        memcpy(&serverAddr.sin_addr.s_addr, server->h_addr, server->h_length);
        serverAddr.sin_port = htons(serverPort_);
        
        // Connect
        if (::connect(socket_, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
            std::cerr << "Failed to connect to " << serverHost_ 
                      << ":" << serverPort_ << std::endl;
            close(socket_);
            return false;
        }
        
        connected_ = true;
        std::cout << "Connected to " << serverHost_ << ":" << serverPort_ << std::endl;
        return true;
    }
    
    void disconnect() {
        if (socket_ >= 0) {
            close(socket_);
            socket_ = -1;
        }
        connected_ = false;
    }
    
    bool isConnected() const {
        return connected_;
    }
    
    bool sendSequence(const std::string& sequence, const std::string& format = "RAW") {
        if (!connected_) {
            std::cerr << "Not connected to server" << std::endl;
            return false;
        }
        
        std::string data;
        
        if (format == "FASTA") {
            data = ">sequence\n" + sequence + "\n";
        } else if (format == "FASTQ") {
            data = "@sequence\n" + sequence + "\n+\n";
            // Add quality scores (all 'I' = Phred 40)
            data += std::string(sequence.length(), 'I') + "\n";
        } else {
            data = sequence + "\n";
        }
        
        ssize_t sent = send(socket_, data.c_str(), data.length(), 0);
        
        if (sent < 0) {
            std::cerr << "Failed to send data" << std::endl;
            connected_ = false;
            return false;
        }
        
        return true;
    }
    
    bool sendFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file) {
            std::cerr << "Failed to open file: " << filename << std::endl;
            return false;
        }
        
        std::string line;
        std::string sequence;
        std::string format = "RAW";
        int sequenceCount = 0;
        
        while (std::getline(file, line)) {
            if (line.empty()) continue;
            
            if (line[0] == '>') {
                // FASTA header - send previous sequence if any
                if (!sequence.empty()) {
                    sendSequence(sequence, "FASTA");
                    sequenceCount++;
                    sequence.clear();
                }
                format = "FASTA";
            } else if (line[0] == '@') {
                // FASTQ header
                if (!sequence.empty()) {
                    sendSequence(sequence, "FASTQ");
                    sequenceCount++;
                    sequence.clear();
                }
                format = "FASTQ";
            } else if (line[0] == '+') {
                // FASTQ quality separator - skip
                continue;
            } else {
                // Sequence data
                sequence += line;
            }
            
            if (sequenceCount % 100 == 0 && sequenceCount > 0) {
                std::cout << "\rSent " << sequenceCount << " sequences..." << std::flush;
            }
        }
        
        // Send last sequence
        if (!sequence.empty()) {
            sendSequence(sequence, format);
            sequenceCount++;
        }
        
        std::cout << "\rSent " << sequenceCount << " sequences from " << filename << std::endl;
        return true;
    }
};

//=============================================================================
// Utility Functions
//=============================================================================

std::string generateRandomSequence(size_t length) {
    static const char nucleotides[] = "ACGT";
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 3);
    
    std::string sequence;
    sequence.reserve(length);
    
    for (size_t i = 0; i < length; i++) {
        sequence += nucleotides[dis(gen)];
    }
    
    return sequence;
}

void interactiveMode(DNAClient& client) {
    std::cout << "\n=== Interactive Mode ===" << std::endl;
    std::cout << "Enter DNA sequences (or 'quit' to exit):" << std::endl;
    
    std::string line;
    int count = 0;
    
    while (true) {
        std::cout << "\nSequence > ";
        if (!std::getline(std::cin, line)) break;
        
        if (line == "quit" || line == "exit" || line == "q") {
            break;
        }
        
        if (line.empty()) continue;
        
        // Remove whitespace
        line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());
        
        if (client.sendSequence(line)) {
            count++;
            std::cout << "Sent sequence #" << count << " (" << line.length() << " bp)" << std::endl;
        } else {
            std::cerr << "Failed to send sequence" << std::endl;
            break;
        }
    }
    
    std::cout << "\nTotal sequences sent: " << count << std::endl;
}

void stressTest(DNAClient& client, int numSequences, size_t sequenceLength = 1000) {
    std::cout << "\n=== Stress Test ===" << std::endl;
    std::cout << "Sending " << numSequences << " random sequences of " 
              << sequenceLength << " bp each..." << std::endl;
    
    auto startTime = std::chrono::steady_clock::now();
    
    for (int i = 0; i < numSequences; i++) {
        std::string sequence = generateRandomSequence(sequenceLength);
        
        if (!client.sendSequence(sequence)) {
            std::cerr << "Failed at sequence " << i << std::endl;
            break;
        }
        
        if ((i + 1) % 100 == 0) {
            std::cout << "\rSent " << (i + 1) << " / " << numSequences << "..." << std::flush;
        }
    }
    
    auto endTime = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    double seconds = duration.count() / 1000.0;
    double throughputSeq = numSequences / seconds;
    double throughputKB = (numSequences * sequenceLength) / 1024.0 / seconds;
    
    std::cout << "\n\nStress Test Complete!" << std::endl;
    std::cout << "Time: " << seconds << " seconds" << std::endl;
    std::cout << "Throughput: " << throughputSeq << " sequences/sec" << std::endl;
    std::cout << "Throughput: " << throughputKB << " KB/sec" << std::endl;
}

void printUsage(const char* program) {
    std::cout << "Usage: " << program << " <server> [port] [options]" << std::endl;
    std::cout << "\nOptions:" << std::endl;
    std::cout << "  --file <filename>       Send sequences from file" << std::endl;
    std::cout << "  --interactive           Interactive mode" << std::endl;
    std::cout << "  --stress <count>        Stress test with N random sequences" << std::endl;
    std::cout << "  --length <size>         Sequence length for stress test (default: 1000)" << std::endl;
    std::cout << "\nExamples:" << std::endl;
    std::cout << "  " << program << " localhost 9090" << std::endl;
    std::cout << "  " << program << " 192.168.1.100 9090 --file genome.fasta" << std::endl;
    std::cout << "  " << program << " localhost 9090 --interactive" << std::endl;
    std::cout << "  " << program << " localhost 9090 --stress 1000 --length 500" << std::endl;
}

//=============================================================================
// Main
//=============================================================================

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }
    
    std::string server = argv[1];
    int port = DEFAULT_PORT;
    std::string mode = "single";
    std::string filename;
    int stressCount = 1000;
    size_t sequenceLength = 1000;
    
    // Parse arguments
    for (int i = 2; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "--file" && i + 1 < argc) {
            mode = "file";
            filename = argv[++i];
        } else if (arg == "--interactive") {
            mode = "interactive";
        } else if (arg == "--stress" && i + 1 < argc) {
            mode = "stress";
            stressCount = std::atoi(argv[++i]);
        } else if (arg == "--length" && i + 1 < argc) {
            sequenceLength = std::atoi(argv[++i]);
        } else if (arg[0] != '-') {
            port = std::atoi(arg.c_str());
        }
    }
    
    // Validate port
    if (port <= 0 || port > 65535) {
        std::cerr << "Invalid port number: " << port << std::endl;
        return 1;
    }
    
    std::cout << "=== DNA Client ===" << std::endl;
    std::cout << "Server: " << server << ":" << port << std::endl;
    std::cout << "Mode: " << mode << std::endl;
    
    // Create and connect client
    DNAClient client(server, port);
    
    if (!client.connect()) {
        return 1;
    }
    
    // Execute based on mode
    if (mode == "file") {
        client.sendFile(filename);
    } else if (mode == "interactive") {
        interactiveMode(client);
    } else if (mode == "stress") {
        stressTest(client, stressCount, sequenceLength);
    } else {
        // Single sequence example
        std::string testSeq = "ATCGATCGATCGATCGATCG";
        std::cout << "\nSending test sequence: " << testSeq << std::endl;
        
        if (client.sendSequence(testSeq)) {
            std::cout << "Sequence sent successfully!" << std::endl;
        } else {
            std::cerr << "Failed to send sequence" << std::endl;
        }
    }
    
    // Keep connection alive for a moment
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    std::cout << "\nDisconnecting..." << std::endl;
    client.disconnect();
    
    return 0;
}
