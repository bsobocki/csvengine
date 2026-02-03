
<p align="center">
<img src="docs/logo.png" height="200"/>
</p>

# csvengine
![Language](https://img.shields.io/badge/language-C%2B%2B20-blue.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)
![Build](https://img.shields.io/badge/build-CMake-orange)
![Platform](https://img.shields.io/badge/platform-Linux-lightgrey)
![Status](https://img.shields.io/badge/status-Alpha%20%2F%20Under%20Development-yellow)


**High-performance C++20 CSV parser with streaming and type-safe conversions.**


`csvengine` is a modern C++ library designed to parse RFC 4180 compliant CSV files efficiently. It focuses on low memory usage (streaming mode), ease of integration, and type safety via `std::optional` and C++20 concepts.

## Table of Contents
- [Features](#features)
- [Performance Benchmarks](#performance-benchmarks)
- [Requirements](#requirements)
- [Quick Start](#quick-start)
- [Building the Project](#building-the-project)
- [API Reference](#api-reference)
- [Usage Examples](#usage-examples)
- [Building from Source](#building-from-source)
- [Testing](#testing)
- [Benchmarks](#benchmarks)
- [Project Structure](#project-structure)
- [Roadmap](#roadmap)
- [License](#license)


## Features

| Feature | Description |
|---------|-------------|
| **RFC 4180 Compliant** | Full support for quoted fields, embedded newlines, escaped quotes |
| **Streaming Architecture** | $O(\text{record.size()})$ memory usage - parse multi-GB files with constant RAM |
| **Type-Safe Conversions** | `row.get<T>("age")` returns `std::optional<T>` (int, double, string, etc.) |
| **Modern C++20** | Concepts, `string_view`, `from_chars`, ranges-compatible iterators |
| **Flexible Access** | Access fields by index `row[0]` or column name `row["name"]` |
| **Configurable** | Custom delimiters, quote chars, line endings (LF/CRLF/CR) |
| **Strict & Lenient Modes** | Choose between RFC-strict parsing or forgiving real-world mode |
| **Zero Dependencies** | Only standard library (GoogleTest/Benchmark for development) |


## Performance Benchmarks

For information about performance please read [BENCHMARKING.md](./BENCHMARKING.md)


## Requirements

*   **Operating System:** Linux (Primary), Windows (WSL recommended), macOS.
*   **Compiler:** Must support **C++20**.
    *   GCC 11+
    *   Clang 14+
    *   MSVC 2022+
*   **Build System:** CMake 3.20 or higher.
*   **Dependencies:**
    *   **GoogleTest:** Automatically fetched via CMake for unit testing.


## Quick Start

### Installation

#### Option 1: CMake FetchContent (Recommended)
```cmake
include(FetchContent)
FetchContent_Declare(
  csvengine
  GIT_REPOSITORY https://github.com/bsobocki/csvengine.git
  GIT_TAG        v1.0.0
)
FetchContent_MakeAvailable(csvengine)

target_link_libraries(your_app PRIVATE csvengine)
```

#### Option 2: Git Submodule
```bash
git submodule add https://github.com/bsobocki/csvengine.git external/csvengine
```
```cmake
add_subdirectory(external/csvengine)
target_link_libraries(your_app PRIVATE csvengine)
```

#### Option 3: Header Include
Copy `engine/inc/` to your project and include directly.


## Building the Project

This project does not allow in-source builds. You must create a build directory.

### 1. Clone
```bash
git clone https://github.com/bsobocki/csvengine.git
cd csvengine
```

### 2. Configure & Build
```bash
./go.sh build
```

### 3. Run Tests
```bash
./go.sh tests
```

### 4. Run Benchmarks
```bash
./go.sh benchmarks
```

### 5. Run Demo
```bash
./go.sh demo
```

---

## API Reference

### `csv::Reader`

| Method | Description |
|--------|-------------|
| `Reader(path, config)` | Construct from file path |
| `Reader(stream, config)` | Construct from `std::istream` |
| `next()` | Advance to next record, returns `false` at EOF |
| `current_record()` | Get current `Record` reference |
| `headers()` | Get column names (if `has_header=true`) |
| `line_number()` | Current line number (1-indexed) |
| `record_size()` | Number of fields per record |
| `good()` | Check if reader is in valid state |
| `begin()` / `end()` | Range-based for loop support |

### `csv::Record`

| Method | Description |
|--------|-------------|
| `get<T>(index)` | Get field by index as `std::optional<T>` |
| `get<T>(name)` | Get field by column name as `std::optional<T>` |
| `at(index)` | Get field by index, throws on out-of-range |
| `at(name)` | Get field by name, throws if not found |
| `operator[](index)` | Direct access by index (no bounds check) |
| `operator[](name)` | Direct access by name (throws if not found) |
| `fields()` | Get all fields as `vector<string>` |
| `size()` | Number of fields |
| `empty()` | Check if record has no fields |

### `csv::Config`

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| `delimiter` | `char` | `,` | Field separator |
| `has_header` | `bool` | `true` | First row contains column names |
| `has_quoting` | `bool` | `true` | Enable quoted field parsing |
| `quote_char` | `char` | `"` | Quote character |
| `parse_mode` | `ParseMode` | `strict` | `strict` or `lenient` |
| `line_ending` | `LineEnding` | `lf` | `lf`, `crlf`, or `cr` |
| `record_size_policy` | `RecordSizePolicy` | `strict_to_first` | Field count validation |
| `record_size` | `size_t` | `0` | Expected fields (for `strict_to_value`) |

### Supported Types for `get<T>()`

- `std::string`, `std::string_view`
- `int`, `long`, `long long`
- `unsigned int`, `unsigned long`, `unsigned long long`
- `float`, `double`
- Any type with `operator>>` from `std::istream`

---

## Usage Examples

### 1. Basic Iteration
Read a file row by row using C++ range-based for loops.

```cpp
#include <csvengine.hpp>
#include <iostream>

int main() {
    try {
        csv::Reader reader("data.csv"); // Defaults: comma delimiter, has header

        for (const auto& record : reader) {
            // Access raw string_view by index
            std::cout << "Field 0: " << record[0] << "\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
}
```

### 2. Type Conversion & Column Names
Access data safely using types and header names.

```cpp
csv::Reader reader("employees.csv");

for (const auto& record : reader) {
    // get<T> returns std::optional<T>
    auto name = record.get<std::string>("name");
    auto age  = record.get<int>("age");
    auto salary = record.get<double>("salary");

    if (name && age) {
        std::cout << *name << " is " << *age << " years old.\n";
    }
}
```

### 3. Custom Configuration
Handle TSV files or files without headers.

```cpp
csv::Config config{
    .delimiter = ';',                                    // Semicolon-separated
    .has_header = false,
    .has_quoting = true,
    .quote_char = '"',
    .parse_mode = csv::Config::ParseMode::lenient,       // Forgiving mode
    .line_ending = csv::Config::LineEnding::crlf,        // Windows line endings
    .record_size_policy = csv::Config::RecordSizePolicy::strict_to_header
};

csv::Reader reader("data.tsv", config);

for (const auto& record : reader) {
    // Since there are no headers, access by index
    auto id = record.get<int>(0);
}
```


### Compile Options

The library is configured with strict warning flags (`-Wall -Wextra -Wpedantic`) during development to ensure code quality.


## Building from Source

### Requirements

| Requirement | Minimum Version |
|-------------|-----------------|
| CMake | 3.20+ |
| C++ Compiler | GCC 11+, Clang 14+, MSVC 2022+ |
| OS | Linux (primary), macOS, Windows (WSL) |

### Build Commands

```bash
# Clone repository
git clone https://github.com/bsobocki/csvengine.git
cd csvengine

# Build (using provided script)
./go.sh build

# Run all tests
./go.sh tests

# Run benchmarks
./go.sh benchmarks

# Run demo
./go.sh demo
```

### Manual CMake Build

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel
ctest --output-on-failure
```

## Testing

The project currently includes **289 unit tests** covering:

- Basic parsing (simple fields, empty fields)
- Quoting (escaped quotes, embedded delimiters, newlines in fields)
- Buffer management (partial reads, boundary conditions)
- Line endings (LF, CRLF, CR)
- Error handling (malformed input, configuration errors)
- Type conversions (integers, floats, strings)

```bash
# Run all tests
./go.sh tests

# Run specific test
./go.sh run_tests StrictParser

# Run with verbose output
cd build && ctest --output-on-failure --verbose

# Run specific test manually
./build/tests/run_tests --gtest_filter="StrictParserTest.*"
```

## Benchmarks

The project currently contains several different benchmark tests.\
You can run them using:
```bash
./go.sh benchmarks
```
If you want to have detailed result summary in JSON or CSV formats you can just add format name as the next argument.
```bash
./go.sh benchmarks csv

./go.sh benchmarks json
```

If you want to run a specific benchmark:
```bash
./go.sh run_benchmarks ParserComparison
# or with format
./go.sh run_benchmarks ParserComparison csv
```

## Project Structure

```text
csvengine/
├── CMakeLists.txt              # Main build configuration
├── README.md                   # This file
├── SPECIFICATION.md            # Technical specification
├── BENCHMARKING.md             # Performance analysis
├── LICENSE                     # MIT License
├── go.sh                       # Build/test helper script
│
├── engine/                     # Core library
│   ├── CMakeLists.txt
|   |
│   ├── inc/                    # Public headers
│   │   ├── csvbuffer/
│   │   │   ├── csvbuffer.hpp         # I/O Buffer interface declaration
│   │   │   ├── csvmappedbuffer.hpp   # Buffer as mapped file
│   │   │   └── csvstreambuffer.hpp   # Chunk based buffer
│   │   │
│   │   ├── csvengine.hpp       # Main include
│   │   ├── csvreader.hpp       # Reader class
│   │   ├── csvrecord.hpp       # Record class
│   │   ├── csvconfig.hpp       # Configuration
│   │   ├── csvparser.hpp       # Parser interface
│   │   └── csverrors.hpp       # Exception types
│   │
│   └── src/                    # Implementation
│       ├── csvreader.cpp
│       ├── csvparser.cpp
│       ├── csvmappedbuffer.cpp
│       ├── csvparser_simple_parser.cpp
│       ├── csvparser_quoting_strict_parser.cpp
│       └── csvparser_quoting_lenient_parser.cpp
│
├── tests/                      # Unit tests
│   ├── CMakeLists.txt
|   |
│   ├── src/
|   |   ├── csvbuffer_tests/
|   |   |   ├── csvmappedbuffer_test.cpp
|   |   |   └── csvstreambuffer_test.cpp
|   |   |
│   │   ├── csvparser_tests/
|   |   |   ├── csvparser_simple_test.cpp
|   |   |   ├── csvparser_quoting_strict_test.cpp
|   |   |   └── csvparser_quoting_lenient_test.cpp
|   |   |
│   │   ├── csvreader_test.cpp
│   │   └── csvrecord_test.cpp
|   |
│   ├── mocks/
│   │   └── csvbuffer_mock.hpp
|   |
│   └── test_data/
│       ├── simple_file.csv
│       ├── quoting.csv
│       └── testdata.hpp
│
├── benchmarks/                 # Performance benchmarks
│   ├── CMakeLists.txt
|   |
│   ├── inc/
│   |   └── helpers.hpp         # helper functions definitions
|   | 
│   └── src/
│       ├── helpers.cpp
│       ├── buffers_comparison_benchmark.cpp
│       ├── parser_benchmark.cpp
│       ├── reader_benchmark.cpp
│       └── streambuffer_benchmark.cpp
│
├── demo/                       # Example application
│   ├── CMakeLists.txt
│   └── main.cpp
│
└── docs/                       # Documentation assets
    └── logo.png
```

## Roadmap

### Version 1.0 (Current)
- [x] RFC 4180 compliant parsing
- [x] Streaming architecture
- [x] Type-safe field access
- [x] Configurable delimiters and line endings
- [x] Strict and lenient parsing modes
- [x] Comprehensive test suite
- [x] Performance benchmarks

### Version 1.1 (Planned)
- [x] Memory-mapped file I/O for improved performance
- [ ] Schema validation with custom rules
- [ ] Column-wise iteration
- [ ] Statistics during parsing (min/max/count)
- [ ] Warning queue for non-fatal issues

### Future
- [ ] In-memory database mode with random access
- [ ] CSV writing support
- [ ] Compressed file support (gzip)
- [ ] Multi-threaded chunk-based parsing
- [ ] Unicode support (UTF-8, UTF-16)



## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
