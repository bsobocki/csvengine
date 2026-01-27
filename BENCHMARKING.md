# Benchmark Results

## Overview

Performance benchmarks for `csvengine` parser implementations across different data sizes, parsing modes, and buffer configurations.\
Current Default Buffer Size is 65536 -> 64 KB chunk.

---

## How to Run Benchmarks

### Basic Run
```bash
./go.sh benchmarks
```

### Export Results to File
```bash
# CSV format
./go.sh benchmarks csv

# JSON format
./go.sh benchmarks json
```

### Run Specific Benchmark
```bash
./go.sh run_benchmarks ParserComparison
# or with format
./go.sh run_benchmarks ParserComparison csv
```

### Tips for Consistent Results
- Close other applications (browsers, IDEs)
- Plug in laptop (avoid battery power saving)
- Let system cool down before running
- Run multiple times and compare

---

## How to Read Benchmark Results

### Raw Output Example
```
BM_SimpleData_ParserComparison_StrictParser/10000/iterations:50   61675026 ns   61842958 ns   50   bytes_per_second=21.2808Mi/s   items_per_second=970.183k/s
```

| Field | Value | Meaning |
|-------|-------|---------|
| `BM_SimpleData_...` | Benchmark name | What's being tested |
| `StrictParser` | Parser type | Which parser implementation |
| `/1000` | Data multiplier | 1000 × base data size |
| `/iterations:50` | Repetitions | Test repeated 50 times |
| `61675026 ns` | Time per iteration | ~61.7 ms to parse all rows |
| `61842958 ns` | CPU Time per iteration | ~61.8 ms to parse all rows CPU time |
| `bytes_per_second=21.2808Mi/s` | Throughput | ~22 MB processed per second |
| `items_per_second=970.183k/s` | Throughput | 970 000 rows processed per second |

### Units Reference

| Unit | Meaning | Conversion |
|------|---------|------------|
| **k/s** | Thousands per second | `967 k/s` = 967 000 rows/sec |
| **M/s** | Millions per second | `1.02 M/s` = 1 020 000 rows/sec |
| **μs** | Microseconds | `6192 μs` = 6.192 ms = 0.006 sec |
| **ns** | Nanoseconds | `6192000 ns` = 6192 μs |
| **Mi/s** | Mebibytes per second | `21 Mi/s` ≈ 22 MB/s |

### Performance Rating (according to Claude Opus 4.5)

| Throughput | Rating | Description |
|------------|--------|-------------|
| > 500 k/s | 🟢 **Fast** | Excellent performance |
| 100 - 500 k/s | 🟡 **Acceptable** | Good for most use cases |
| < 100 k/s | 🔴 **Slow** | May need optimization |

---

## Test Configuration

| Property | Value |
|----------|-------|
| **CPU** | 13th Gen Intel Core i7-13700H (laptop) |
| **Framework** | Google Benchmark |
| **Iterations** | 50 per test |
| **Data Multipliers** | 100 (small), 1000 (medium), 10000 (large) |

### Test Data

| Dataset | Records per Base | Fields | Description |
|---------|------------------|--------|-------------|
| `simple_csv_data` | 6 records | 3 | Unquoted, simple fields |
| `quoted_csv_data` | 3 records | 3 | Quoted fields, embedded newlines, escaped quotes |

### Actual Row Counts

| Multiplier | Simple Data | Quoted Data |
|------------|-------------|-------------|
| 100 | 600 rows | 300 rows |
| 1000 | 6 000 rows | 3 000 rows |
| 10 000 | 60 000 rows | 30 000 rows |

---

## Parser Performance Comparison

### Simple Data (Unquoted CSV)

| Parser | Small (600 rows) | Medium (6K rows) | Large (60K rows) | Average |
|--------|------------------|------------------|------------------|---------|
| **StrictParser** | 931 k/s | 967 k/s | 970 k/s | **~956 k/s** 🟢 |
| **SimpleParser** | 885 k/s | 916 k/s | 923 k/s | **~908 k/s** 🟢 |
| **LenientParser** | 731 k/s | 771 k/s | 781 k/s | **~761 k/s** 🟢 |

### Quoted Data (Complex CSV)

| Parser | Small (300 rows) | Medium (3K rows) | Large (30K rows) | Average |
|--------|------------------|------------------|------------------|---------|
| **StrictParser** | 682 k/s | 717 k/s | 713 k/s | **~704 k/s** 🟢 |
| **LenientParser** | 461 k/s | 485 k/s | 482 k/s | **~476 k/s** 🟡 |

### Parser Comparison Summary

| Comparison | Simple Data | Quoted Data |
|------------|-------------|-------------|
| Strict vs Simple | **5% faster** | N/A |
| Strict vs Lenient | **26% faster** | **48% faster** |

### Key Findings

1. **StrictParser is fastest** — Peak throughput of **~970 k rows/second**
   - Best choice for well-formed CSV data
   - Consistent performance across all data sizes

2. **SimpleParser is competitive** — ~908 k/s average
   - Good choice when quoting is not needed
   - Slightly simpler code path

3. **LenientParser trades speed for flexibility** — ~24-48% slower
   - Handles malformed CSV gracefully
   - Recommended for real-world messy data

4. **All parsers scale linearly** — No performance cliffs
   - Predictable behavior regardless of file size

---

## End-to-End Reader Performance

Full pipeline benchmark including file I/O, buffering, parsing, and record construction.

### Simple Data

| Data Size | Throughput | Time per 10K rows |
|-----------|------------|-------------------|
| 600 rows | 933 k/s | 10.7 ms |
| 6 000 rows | 960 k/s | 10.41 ms |
| 60 000 rows | 955 k/s | 10.47 ms |

### Quoted Data

| Data Size | Throughput | Time per 10K rows |
|-----------|------------|-------------------|
| 300 rows | 676 k/s | 14.79 ms |
| 3 000 rows | 723 k/s | 13.83 ms |
| 30 000 rows | 709 k/s | 14.1 ms |

---

## Buffer Size Impact

Testing different internal buffer sizes to find optimal configuration.

### Throughput by Buffer Size (Large Data - 50K rows)

| Buffer Size | Throughput | Relative Performance |
|-------------|------------|----------------------|
| 64 bytes | 906 k/s | Baseline |
| 256 bytes | 955 k/s | +5.4% |
| 1024 bytes | 959 k/s | +5.9% |
| **2048 bytes** | **970 k/s** | **+7.1%** ⭐ |
| 4096 bytes | 969 k/s | +7.0% |
| 65536 bytes | 948 k/s | +4.6% |

### Key Findings

1. **Optimal buffer size: 2048-4096 bytes (2-4 KB)**
   - Best balance of performance and memory usage
   - ~7% faster than minimum buffer size
   - Both 2KB and 4KB perform nearly identically

2. **Diminishing returns above 4 KB**
   - 64 KB buffer is ~2% slower than optimal
   - Likely due to CPU cache effects

3. **Very small buffers (64 bytes) have measurable overhead**
   - ~7% slower due to frequent refills
   - Still acceptable for memory-constrained environments

4. **256 bytes is a good compromise**
   - Only ~1.5% slower than optimal
   - Suitable for embedded or memory-limited systems

## Throughput Summary

| Metric | Value |
|--------|-------|
| **Peak row throughput** | ~970 k rows/sec |
| **Peak byte throughput** | ~21 Mi/s (~22 MB/s) |
| **Quoted data overhead** | ~26% slower |
| **Lenient mode overhead** | ~24-48% slower |


## Recommendations

| Use Case | Recommended Configuration |
|----------|---------------------------|
| Clean, well-formed CSV | `StrictParser` + 2-4 KB buffer |
| Simple CSV without quotes | `SimpleParser` + default buffer |
| Real-world messy data | `LenientParser` (~48% slower) |
| Memory-constrained | 256-byte buffer (~1.5% slower) |
| Maximum throughput | `StrictParser` + 2 KB buffer |

---

## Notes

> **Laptop Benchmark Variance**
> 
> Results obtained on a laptop CPU (i7-13700H) may vary ±10% between runs due to:
> - Thermal throttling
> - Turbo boost duration
> - Background processes
> - Power management
>
> For most consistent results, run on desktop hardware or lock CPU frequency.

---

## Raw Benchmark Output

<details>
<summary>Click to expand full benchmark output</summary>

```
BM_SimpleData_ParserComparison_SimpleParser/100/iterations:50        675694 ns       676610 ns           50 bytes_per_second=19.4509Mi/s items_per_second=885.296k/s
BM_SimpleData_ParserComparison_StrictParser/100/iterations:50        642308 ns       643208 ns           50 bytes_per_second=20.461Mi/s items_per_second=931.27k/s
BM_SimpleData_ParserComparison_LenientParser/100/iterations:50       818692 ns       819848 ns           50 bytes_per_second=16.0526Mi/s items_per_second=730.623k/s
BM_SimpleData_ParserComparison_SimpleParser/1000/iterations:50      6538379 ns      6547006 ns           50 bytes_per_second=20.1019Mi/s items_per_second=916.297k/s
BM_SimpleData_ParserComparison_StrictParser/1000/iterations:50      6192617 ns      6200792 ns           50 bytes_per_second=21.2242Mi/s items_per_second=967.457k/s
BM_SimpleData_ParserComparison_LenientParser/1000/iterations:50     7774738 ns      7785132 ns           50 bytes_per_second=16.9049Mi/s items_per_second=770.571k/s
BM_SimpleData_ParserComparison_SimpleParser/10000/iterations:50    64667186 ns     64983390 ns           50 bytes_per_second=20.2524Mi/s items_per_second=923.297k/s
BM_SimpleData_ParserComparison_StrictParser/10000/iterations:50    61675026 ns     61842958 ns           50 bytes_per_second=21.2808Mi/s items_per_second=970.183k/s
BM_SimpleData_ParserComparison_LenientParser/10000/iterations:50   76803069 ns     76854814 ns           50 bytes_per_second=17.1241Mi/s items_per_second=780.68k/s
BM_QuotedData_ParserComparison_StrictParser/100/iterations:50        438289 ns       438410 ns           50 bytes_per_second=38.5029Mi/s items_per_second=682.01k/s
BM_QuotedData_ParserComparison_LenientParser/100/iterations:50       647880 ns       648044 ns           50 bytes_per_second=26.0477Mi/s items_per_second=461.388k/s
BM_QuotedData_ParserComparison_StrictParser/1000/iterations:50      4181943 ns      4182680 ns           50 bytes_per_second=40.357Mi/s items_per_second=717.004k/s
BM_QuotedData_ParserComparison_LenientParser/1000/iterations:50     6185239 ns      6186212 ns           50 bytes_per_second=27.2865Mi/s items_per_second=484.788k/s
BM_QuotedData_ParserComparison_StrictParser/10000/iterations:50    42078439 ns     42086608 ns           50 bytes_per_second=40.1079Mi/s items_per_second=712.792k/s
BM_QuotedData_ParserComparison_LenientParser/10000/iterations:50   62192214 ns     62207176 ns           50 bytes_per_second=27.1352Mi/s items_per_second=482.243k/s

BM_Reader_Stream_EndToEnd/50                                         320409 ns       320501 ns         2175 bytes_per_second=20.5315Mi/s items_per_second=932.916k/s
BM_Reader_Stream_EndToEnd/1000                                      6241932 ns      6247243 ns           96 bytes_per_second=21.0664Mi/s items_per_second=960.264k/s
BM_Reader_Stream_EndToEnd/10000                                    62819526 ns     62801791 ns           11 bytes_per_second=20.9559Mi/s items_per_second=955.371k/s
BM_Reader_QuotedData_EndToEnd/50                                     220573 ns       220391 ns         3165 bytes_per_second=38.2956Mi/s items_per_second=676.07k/s
BM_Reader_QuotedData_EndToEnd/1000                                  4153977 ns      4150615 ns          168 bytes_per_second=40.6688Mi/s items_per_second=722.544k/s
BM_Reader_QuotedData_EndToEnd/10000                                42342710 ns     42308671 ns           17 bytes_per_second=39.8973Mi/s items_per_second=709.051k/s

BM_Reader_BufferSized_EndToEnd<64>/100                               666910 ns       663800 ns         1037 bytes_per_second=19.8263Mi/s items_per_second=902.381k/s
BM_Reader_BufferSized_EndToEnd<64>/1000                             6765188 ns      6700634 ns          103 bytes_per_second=19.641Mi/s items_per_second=895.288k/s
BM_Reader_BufferSized_EndToEnd<64>/10000                           66823755 ns     66259200 ns           10 bytes_per_second=19.8625Mi/s items_per_second=905.52k/s
BM_Reader_BufferSized_EndToEnd<256>/100                              636596 ns       633381 ns         1070 bytes_per_second=20.7785Mi/s items_per_second=945.719k/s
BM_Reader_BufferSized_EndToEnd<256>/1000                            6370742 ns      6346827 ns          109 bytes_per_second=20.7359Mi/s items_per_second=945.197k/s
BM_Reader_BufferSized_EndToEnd<256>/10000                          63049836 ns     62853900 ns           11 bytes_per_second=20.9386Mi/s items_per_second=954.579k/s
BM_Reader_BufferSized_EndToEnd<1024>/100                             629246 ns       627723 ns         1091 bytes_per_second=20.9658Mi/s items_per_second=954.243k/s
BM_Reader_BufferSized_EndToEnd<1024>/1000                           6345749 ns      6323305 ns          107 bytes_per_second=20.813Mi/s items_per_second=948.713k/s
BM_Reader_BufferSized_EndToEnd<1024>/10000                         62617055 ns     62539282 ns           11 bytes_per_second=21.0439Mi/s items_per_second=959.381k/s
BM_Reader_BufferSized_EndToEnd<2048>/100                             624270 ns       623758 ns         1093 bytes_per_second=21.0991Mi/s items_per_second=960.309k/s
BM_Reader_BufferSized_EndToEnd<2048>/1000                           6226889 ns      6222474 ns          110 bytes_per_second=21.1503Mi/s items_per_second=964.086k/s
BM_Reader_BufferSized_EndToEnd<2048>/10000                         61955171 ns     61857345 ns           11 bytes_per_second=21.2759Mi/s items_per_second=969.958k/s
BM_Reader_BufferSized_EndToEnd<4096>/100                             623886 ns       623059 ns         1112 bytes_per_second=21.1227Mi/s items_per_second=961.386k/s
BM_Reader_BufferSized_EndToEnd<4096>/1000                           6237737 ns      6223775 ns          109 bytes_per_second=21.1459Mi/s items_per_second=963.884k/s
BM_Reader_BufferSized_EndToEnd<4096>/10000                         61930665 ns     61912700 ns           11 bytes_per_second=21.2569Mi/s items_per_second=969.09k/s
BM_Reader_BufferSized_EndToEnd<65536>/100                            637067 ns       637105 ns         1098 bytes_per_second=20.657Mi/s items_per_second=940.191k/s
BM_Reader_BufferSized_EndToEnd<65536>/1000                          6245788 ns      6236082 ns          109 bytes_per_second=21.1041Mi/s items_per_second=961.982k/s
BM_Reader_BufferSized_EndToEnd<65536>/10000                        63260847 ns     63288645 ns           11 bytes_per_second=20.7947Mi/s items_per_second=948.022k/s
BM_Reader_BufferSized_EndToEnd<64>/100/iterations:50                 684463 ns       684940 ns           50 bytes_per_second=19.2144Mi/s items_per_second=874.529k/s
BM_Reader_BufferSized_EndToEnd<256>/100/iterations:50                640369 ns       640826 ns           50 bytes_per_second=20.5371Mi/s items_per_second=934.731k/s
BM_Reader_BufferSized_EndToEnd<1024>/100/iterations:50               650686 ns       651230 ns           50 bytes_per_second=20.209Mi/s items_per_second=919.798k/s
BM_Reader_BufferSized_EndToEnd<2048>/100/iterations:50               643245 ns       643684 ns           50 bytes_per_second=20.4459Mi/s items_per_second=930.581k/s
BM_Reader_BufferSized_EndToEnd<4096>/100/iterations:50               633670 ns       634154 ns           50 bytes_per_second=20.7532Mi/s items_per_second=944.566k/s
BM_Reader_BufferSized_EndToEnd<65536>/100/iterations:50              647576 ns       648056 ns           50 bytes_per_second=20.308Mi/s items_per_second=924.303k/s
BM_Reader_BufferSized_EndToEnd<64>/1000/iterations:50               6680978 ns      6685244 ns           50 bytes_per_second=19.6862Mi/s items_per_second=897.349k/s
BM_Reader_BufferSized_EndToEnd<256>/1000/iterations:50              6371853 ns      6376022 ns           50 bytes_per_second=20.6409Mi/s items_per_second=940.869k/s
BM_Reader_BufferSized_EndToEnd<1024>/1000/iterations:50             6414567 ns      6420156 ns           50 bytes_per_second=20.499Mi/s items_per_second=934.401k/s
BM_Reader_BufferSized_EndToEnd<2048>/1000/iterations:50             6325942 ns      6298628 ns           50 bytes_per_second=20.8946Mi/s items_per_second=952.43k/s
BM_Reader_BufferSized_EndToEnd<4096>/1000/iterations:50             6296813 ns      6302304 ns           50 bytes_per_second=20.8824Mi/s items_per_second=951.874k/s
BM_Reader_BufferSized_EndToEnd<65536>/1000/iterations:50            6333703 ns      6340530 ns           50 bytes_per_second=20.7565Mi/s items_per_second=946.135k/s
BM_Reader_BufferSized_EndToEnd<64>/10000/iterations:50             66233801 ns     66337664 ns           50 bytes_per_second=19.839Mi/s items_per_second=904.448k/s
BM_Reader_BufferSized_EndToEnd<256>/10000/iterations:50            63234566 ns     63428082 ns           50 bytes_per_second=20.749Mi/s items_per_second=945.937k/s
BM_Reader_BufferSized_EndToEnd<1024>/10000/iterations:50           65163044 ns     65367056 ns           50 bytes_per_second=20.1335Mi/s items_per_second=917.878k/s
BM_Reader_BufferSized_EndToEnd<2048>/10000/iterations:50           62827465 ns     62935332 ns           50 bytes_per_second=20.9115Mi/s items_per_second=953.344k/s
BM_Reader_BufferSized_EndToEnd<4096>/10000/iterations:50           63179781 ns     63286986 ns           50 bytes_per_second=20.7953Mi/s items_per_second=948.046k/s
BM_Reader_BufferSized_EndToEnd<65536>/10000/iterations:50          63190286 ns     63009562 ns           50 bytes_per_second=20.8868Mi/s items_per_second=952.221k/s
```

<details>