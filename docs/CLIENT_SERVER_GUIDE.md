# DNA Client-Server System

## Client-Server Architecture for DNA Processing

This implementation provides a distributed DNA processing system where:
- **Server (Master)**: Receives DNA sequences, processes with hardware acceleration, stores results
- **Client (Slave)**: Sends DNA sequences to server for processing

## Architecture

```
┌──────────────┐         TCP/IP         ┌──────────────┐
│              │  DNA Sequences (port   │              │
│   Client 1   │ ─────────────────────> │              │
│   (Slave)    │        9090)           │              │
└──────────────┘                        │              │
                                        │    Server    │
┌──────────────┐                        │   (Master)   │
│              │                        │              │
│   Client 2   │ ─────────────────────> │  - NEON SIMD │
│   (Slave)    │                        │  - HW CRC32  │
└──────────────┘                        │  - HW SHA256 │
                                        │  - Storage   │
┌──────────────┐                        │              │
│              │                        │              │
│   Client N   │ ─────────────────────> │              │
│   (Slave)    │                        │              │
└──────────────┘                        └──────────────┘
```

## Files

1. **`dna_server.cpp`** - Server implementation (Master)
   - TCP server on port 9090
   - Multi-client support (up to 16 connections)
   - Hardware-accelerated processing (NEON, CRC32)
   - Real-time statistics
   - Thread-safe queue management

2. **`dna_client.cpp`** - Client implementation (Slave)
   - TCP client
   - Multiple modes: file, interactive, stress test
   - Progress tracking
   - Error handling

3. **`build_client_server.sh`** - Build script
   - Compiles both server and client
   - Checks hardware acceleration
   - Shows usage examples

## Building

### Quick Build
```bash
./build_client_server.sh
```

### Manual Build

```bash
# Build server (with ARM optimizations)
g++ -std=c++17 -O3 -march=armv8.2-a -mtune=cortex-a76 \
    -pthread -o dna_server dna_server.cpp

# Build client
g++ -std=c++17 -O3 -pthread -o dna_client dna_client.cpp
```

## Usage

### Start Server

```bash
# Default port (9090)
./dna_server

# Custom port
./dna_server 8080
```

Server output:
```
DNA Server started on port 9090
Worker threads: 4
Hardware acceleration: Enabled (NEON + CRC32)
Waiting for clients...

Connections: 2/5 | Sequences: 1523 | Received: 1234 KB | Errors: 0 | Throughput: 45.3 KB/s | Uptime: 27s
```

### Client Modes

#### 1. Single Sequence (Test)
```bash
./dna_client localhost 9090
```

#### 2. Interactive Mode
```bash
./dna_client localhost 9090 --interactive
```

Example session:
```
=== Interactive Mode ===
Enter DNA sequences (or 'quit' to exit):

Sequence > ATCGATCGATCG
Sent sequence #1 (12 bp)

Sequence > GGCCTTAACCGG
Sent sequence #2 (12 bp)

Sequence > quit
Total sequences sent: 2
```

#### 3. File Mode (FASTA/FASTQ)
```bash
./dna_client localhost 9090 --file genome.fasta
```

Example FASTA file:
```fasta
>seq1 Human chromosome 1
ATCGATCGATCGATCG
>seq2 Human chromosome 2
GGCCTTAACCGGTTAA
```

#### 4. Stress Test
```bash
# Send 1000 random sequences of 1000 bp each
./dna_client localhost 9090 --stress 1000

# Custom sequence length
./dna_client localhost 9090 --stress 5000 --length 500
```

Output:
```
=== Stress Test ===
Sending 1000 random sequences of 1000 bp each...
Sent 1000 / 1000...

Stress Test Complete!
Time: 2.34 seconds
Throughput: 427.4 sequences/sec
Throughput: 417.4 KB/sec
```

## Features

### Server Features

✅ **Hardware Acceleration**
- NEON SIMD for nucleotide validation (16 bytes parallel)
- ARM CRC32 for checksums (7.5× faster)
- Multi-threaded processing (one thread per core)

✅ **Multi-Client Support**
- Up to 16 simultaneous connections
- Thread per client
- Thread-safe queue management

✅ **Format Support**
- FASTA (>header)
- FASTQ (@header)
- RAW sequences

✅ **Storage**
- Inchrosil encoding (2-bit per nucleotide)
- Metadata (ID, client, checksum, timestamp)
- File output (.ich format)

✅ **Statistics**
- Active connections
- Total sequences processed
- Bytes received
- Validation errors
- Throughput (KB/s)
- Uptime

### Client Features

✅ **Multiple Modes**
- Single sequence test
- Interactive input
- File upload (FASTA/FASTQ)
- Stress testing

✅ **Progress Tracking**
- Real-time progress
- Throughput calculation
- Error reporting

✅ **Flexible Input**
- Raw sequences
- FASTA format
- FASTQ format
- Random generation (stress test)

## Protocol

### Message Format

**Client → Server:**
```
<sequence>\n
```

For FASTA:
```
>header\n
<sequence>\n
```

For FASTQ:
```
@header\n
<sequence>\n
+\n
<quality>\n
```

### Connection Flow

1. Client connects to server (TCP)
2. Server accepts connection, spawns handler thread
3. Client sends sequences (newline-separated)
4. Server processes each sequence:
   - Validate with NEON
   - Calculate CRC32
   - Encode to Inchrosil
   - Store to file
5. Connection remains open for multiple sequences
6. Client disconnects when done

## Performance

### Server Performance (RPi 5)

```
Hardware: 4× Cortex-A76 @ 2.4 GHz
Acceleration: NEON + CRC32

Single Client:
  - Throughput: ~100 KB/s per client
  - Latency: ~5-10 ms per sequence
  - CPU: ~25% per client

Multi-Client (4 clients):
  - Total Throughput: ~400 KB/s
  - CPU: ~100% (all cores utilized)
  - Memory: ~50 MB
```

### Network Performance

```
Localhost (127.0.0.1):
  - Bandwidth: ~1 GB/s
  - Latency: <1 ms
  - Best for testing

LAN (Gigabit):
  - Bandwidth: ~100-120 MB/s
  - Latency: ~1-2 ms
  - Suitable for production

WiFi (802.11ac):
  - Bandwidth: ~30-50 MB/s
  - Latency: ~5-10 ms
  - May limit throughput
```

## Testing

### Basic Test

Terminal 1 (Server):
```bash
./dna_server
```

Terminal 2 (Client):
```bash
./dna_client localhost 9090 --stress 100
```

### Multi-Client Test

Terminal 1 (Server):
```bash
./dna_server
```

Terminal 2-5 (4 Clients):
```bash
# Each in separate terminal
./dna_client localhost 9090 --stress 1000 &
./dna_client localhost 9090 --stress 1000 &
./dna_client localhost 9090 --stress 1000 &
./dna_client localhost 9090 --stress 1000 &
```

### File Processing Test

Create test file `test.fasta`:
```fasta
>seq1
ATCGATCGATCGATCGATCGATCG
>seq2
GGCCTTAACCGGTTAACCGGTTAA
>seq3
TTAACCGGTTAACCGGTTAACCGG
```

Send to server:
```bash
./dna_client localhost 9090 --file test.fasta
```

## Output Files

Server creates files in current directory:

```
dna_output_1.ich
dna_output_2.ich
dna_output_3.ich
...
```

File format:
```
INCHROSIL
ID: 1
Client: 192.168.1.100:54321
Format: FASTA
Length: 24
Checksum: 0x12345678
Timestamp: 1732492800
---
<binary encoded data>
```

## Troubleshooting

### Issue: Connection refused

**Symptom**: Client cannot connect to server

**Solutions**:
1. Check server is running: `ps aux | grep dna_server`
2. Check port: `netstat -tuln | grep 9090`
3. Check firewall: `sudo ufw allow 9090`
4. Try localhost first: `./dna_client localhost 9090`

### Issue: Low throughput

**Symptom**: Throughput < 50 KB/s

**Solutions**:
1. Check CPU governor: `cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor`
2. Set performance mode: `echo performance | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor`
3. Use localhost for testing
4. Check network with: `iperf3 -s` (server) and `iperf3 -c <ip>` (client)

### Issue: Server crashes

**Symptom**: Server exits unexpectedly

**Solutions**:
1. Check for port already in use: `lsof -i :9090`
2. Increase file descriptor limit: `ulimit -n 65536`
3. Check memory: `free -h`
4. Run with verbose output

## Advanced Usage

### Remote Server

On server machine (192.168.1.100):
```bash
./dna_server 9090
```

On client machine:
```bash
./dna_client 192.168.1.100 9090 --file genome.fasta
```

### Custom Port

```bash
# Server
./dna_server 8080

# Client
./dna_client localhost 8080 --interactive
```

### Batch Processing

```bash
# Process multiple files
for file in *.fasta; do
    ./dna_client localhost 9090 --file "$file"
done
```

### Continuous Streaming

```bash
# Generate and send sequences continuously
while true; do
    ./dna_client localhost 9090 --stress 100
    sleep 1
done
```

## Integration with Serial System

This client-server system can be integrated with the serial acquisition system:

```
Serial Ports → Local Processing → Network → Central Server → Storage
   (RPi)         (Light encode)    (TCP)    (Full process)   (Archive)
```

Example workflow:
1. RPi receives DNA from serial port
2. RPi does basic validation/formatting
3. RPi sends to central server via network
4. Central server does full processing with hardware acceleration
5. Central server stores to high-capacity storage

## Next Steps

1. Add authentication (API keys)
2. Add encryption (TLS/SSL)
3. Add compression (gzip)
4. Add request/response protocol
5. Add database storage (SQLite/PostgreSQL)
6. Add web interface (REST API)
7. Add monitoring dashboard

## References

- Main implementation: `dna_serial_processor_optimized.hpp`
- Hardware analysis: `RPI5_HARDWARE_ANALYSIS.md`
- System architecture: `DNA_SERIAL_ANALYSIS.md`

---

**Version**: 1.0  
**Date**: 2025-11-24  
**Platform**: Raspberry Pi 5 (optimized for Cortex-A76)
