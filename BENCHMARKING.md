# Benchmark Results

# Parser Performance Comparison

## Throughput (rows per second)

| Parser | Small Data | Medium Data | Large Data | Average |
|--------|-----------|-------------|-----------|---------|
| StrictParser | 8,060 k/s | 9,077 k/s | 9,186 k/s | **8,774 k/s** |
| SimpleParser | 8,591 k/s | 8,657 k/s | 8,522 k/s | **8,590 k/s** |
| LenientParser | 7,131 k/s | 7,281 k/s | 7,187 k/s | **7,200 k/s** |

## Absolute Time (nanoseconds per iteration)

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

### SimpleParser vs StrictParser:

Small data:  SimpleParser 6.5% faster (723 < 771)
Medium data: StrictParser 4.6% faster (6,855 < 7,187)
Large data:  StrictParser 7.2% faster (33,873 < 36,509)

## Quoted Data Results

For quoted data we have benchmarks results for `StrictQuotingParser` and `LenintQuotingParser`:

```
BM_QuotedData_ParserComparison_StrictParser/100/iterations:100       592819 ns       547246 ns          100 bytes_per_second=315.857Ki/s items_per_second=5.46372k/s
BM_QuotedData_ParserComparison_LenientParser/100/iterations:100      837913 ns       773454 ns          100 bytes_per_second=223.48Ki/s items_per_second=3.86578k/s
BM_QuotedData_ParserComparison_StrictParser/1000/iterations:100     5395188 ns      5216397 ns          100 bytes_per_second=331.362Ki/s items_per_second=5.74918k/s
BM_QuotedData_ParserComparison_LenientParser/1000/iterations:100    7475170 ns      7474834 ns          100 bytes_per_second=231.245Ki/s items_per_second=4.01213k/s
BM_QuotedData_ParserComparison_StrictParser/5000/iterations:100    26275889 ns     26275850 ns          100 bytes_per_second=328.917Ki/s items_per_second=5.70828k/s
BM_QuotedData_ParserComparison_LenientParser/5000/iterations:100   39482449 ns     37155272 ns          100 bytes_per_second=232.607Ki/s items_per_second=4.03684k/s
```

#### As we can see Strict parser is much faster:

| Data Size	| Performance Gain |
|-----------|------------------|
| 100 rows	|  29.25% faster   |
| 1000 rows	|  27.83% faster   |
| 5000 rows	|  33.45% faster   |

***Key Insight***: The performance advantage increases with larger datasets, suggesting better scalability for the strict parser implementation.


### Second Try

```
BM_QuotedData_ParserComparison_StrictParser/100/iterations:100       566867 ns       542261 ns          100 bytes_per_second=318.761Ki/s items_per_second=5.51395k/s
BM_QuotedData_ParserComparison_LenientParser/100/iterations:100      788755 ns       754516 ns          100 bytes_per_second=229.089Ki/s items_per_second=3.96281k/s
BM_QuotedData_ParserComparison_StrictParser/1000/iterations:100     5402972 ns      5170468 ns          100 bytes_per_second=334.305Ki/s items_per_second=5.80025k/s
BM_QuotedData_ParserComparison_LenientParser/1000/iterations:100    7672361 ns      7342650 ns          100 bytes_per_second=235.408Ki/s items_per_second=4.08436k/s
BM_QuotedData_ParserComparison_StrictParser/5000/iterations:100    27090091 ns     25925779 ns          100 bytes_per_second=333.358Ki/s items_per_second=5.78536k/s
BM_QuotedData_ParserComparison_LenientParser/5000/iterations:100   38618041 ns     36958275 ns          100 bytes_per_second=233.847Ki/s items_per_second=4.05836k/s
```

#### Summary:

| Data Size	| Performance Gain |
|-----------|------------------|
| 100 rows	|  28.13% faster   |
| 1000 rows	|  29.58% faster   |
| 5000 rows	|  29.85% faster   |

***Key Finding***: The `StrictQuotingParser` maintains a consistent `~29%` performance advantage across all dataset sizes, indicating stable and predictable performance characteristics.