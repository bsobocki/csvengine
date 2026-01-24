# Benchmark Results

## Parser Performance Comparison

### Throughput (rows per second)

| Parser | Small Data | Medium Data | Large Data | Average |
|--------|-----------|-------------|-----------|---------|
| StrictParser | 8,060 k/s | 9,077 k/s | 9,186 k/s | **8,774 k/s** |
| SimpleParser | 8,591 k/s | 8,657 k/s | 8,522 k/s | **8,590 k/s** |
| LenientParser | 7,131 k/s | 7,281 k/s | 7,187 k/s | **7,200 k/s** |

### Absolute Time (nanoseconds per iteration)

| Data Size | StrictParser | SimpleParser | LenientParser |
|-----------|-------------|-------------|--------------|
| 100 | 771 μs | 723 μs | 871 μs |
| 1000 | 6855 μs | 7187 μs | 8545 μs |
| 5000 | 33873 μs | 36509 μs | 43299 μs |

### Key Findings

1. **StrictParser is fastest** (~7.2% faster than SimpleParser on big data)
   - Single-pass character iteration
   - Better cache locality
   - Fewer function calls than memchr()-based approach

2. **SimpleParser is competitive** (competitive with StrictParser)
   - Simpler implementation
   - No quote handling overhead
   - Multi-pass but still efficient

3. **LenientParser has overhead** (~18% slower than StrictParser)
   - More complex quote state management
   - Additional branching per character
   - Trade-off for lenient parsing rules

4. **All scale linearly** with input size
   - No unexpected performance cliffs
   - Predictable behavior

