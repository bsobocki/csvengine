# csvengine

![Language](https://img.shields.io/badge/language-C%2B%2B20-blue.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)
![Build](https://img.shields.io/badge/build-CMake-orange)
![Status](https://img.shields.io/badge/status-Alpha%20%2F%20Under%20Development-yellow)

**High-performance C++20 CSV parser with streaming and type-safe conversions.**

<p align="center">
<img src="docs/logo.png" />
</p>

`csvengine` is a modern C++ library designed to parse RFC 4180 compliant CSV files efficiently. It focuses on low memory usage (streaming mode), ease of integration (header-only style architecture), and type safety.

---

## Table of Contents
- [Features](#-features)
- [Requirements](#-requirements)
- [Building the Project](#-building-the-project)
- [Running Tests & Demo](#-running-tests--demo)
- [Usage Examples](#-usage-examples)
- [Integration](#-integration)
- [Project Structure](#-project-structure)
- [Roadmap](#-roadmap)

---

## Features

*   **Standards Compliant:** Fully supports RFC 4180 (quoted fields, embedded newlines, escaped quotes).
*   **Modern C++:** Written in C++20 using concepts, `std::string_view`, and `std::optional`.
*   **Streaming Architecture:** Constant memory usage (O(1)) regardless of file size. Reads row-by-row.
*   **Type Safety:** Automatic conversion to types (int, double, string, etc.) via `row.get<T>()`.
*   **Flexible Access:** Access fields by column index or header name.
*   **Configurable:** Support for custom delimiters, header toggling, and strict/lenient error handling.

---

## Requirements

*   **Operating System:** Linux (Primary), Windows (WSL recommended), macOS.
*   **Compiler:** Must support **C++20**.
    *   GCC 11+
    *   Clang 14+
    *   MSVC 2022+
*   **Build System:** CMake 3.20 or higher.
*   **Dependencies:**
    *   **GoogleTest:** Automatically fetched via CMake for unit testing.

---

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

### 4. Run Demo
```bash
./go.sh demo
```

---

## Running Tests & Demo

### Running Unit Tests
The project uses GoogleTest. After building, you can run the test suite using CTest (bundled with CMake) or the executable directly.

```bash
cd build
ctest --output-on-failure
# OR directly:
./tests/run_tests
# OR using script
./go.sh tests
```

### Running the Demo
A sample application is provided in the `demo` folder.

```bash
cd build
./demo/csvengine_demo
# OR using script
./go.sh demo
```

---

## Usage Examples

### 1. Basic Iteration
Read a file row by row using C++ range-based for loops.

```cpp
#include <csvengine.hpp>
#include <iostream>

int main() {
    try {
        CSVReader reader("data.csv"); // Defaults: comma delimiter, has header

        for (const auto& row : reader) {
            // Access raw string_view by index
            std::cout << "Field 0: " << row[0] << "\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
}
```

### 2. Type Conversion & Column Names
Access data safely using types and header names.

```cpp
CSVReader reader("employees.csv");

for (const auto& row : reader) {
    // get<T> returns std::optional<T>
    auto name = row.get<std::string>("name");
    auto age  = row.get<int>("age");
    auto salary = row.get<double>("salary");

    if (name && age) {
        std::cout << *name << " is " << *age << " years old.\n";
    }
}
```

### 3. Custom Configuration
Handle TSV files or files without headers.

```cpp
CSVConfig config;
config.delimiter = '\t';
config.has_header = false;

CSVReader reader("data.tsv", config);

for (const auto& row : reader) {
    // Since there are no headers, access by index
    auto id = row.get<int>(0);
}
```

---

## Integration

### Using `add_subdirectory`
If you are using CMake, the easiest way to use `csvengine` is to include the source tree:

```cmake
add_subdirectory(path/to/csvengine)
target_link_libraries(your_app PRIVATE csvengine)
```

### Compile Options
The library is configured with strict warning flags (`-Wall -Wextra -Wpedantic`) during development to ensure code quality.

---

## Project Structure

```text
csvengine/
├── CMakeLists.txt           # Main build configuration
├── README.md                # Documentation
├── SPECIFICATION.md         # Detailed technical requirements
├── engine/                  # Library Source Code
│   ├── CMakeLists.txt
│   ├── inc/                 # Public Headers
│   │   ├── csvconfig.hpp
│   │   ├── csvengine.hpp    # Main include file
│   │   ├── csvreader.hpp
│   │   └── csvrecord.hpp
│   └── src/                 # Implementation
├── demo/                    # Usage examples
│   └── main.cpp
└── tests/                   # Unit Tests (GoogleTest)
    └── tests.cpp
```

---

## License

This project is open-source. [License details to be added].