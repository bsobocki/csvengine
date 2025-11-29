# CSV Engine - Technical Specification

**Version:** 1.0

**Date:** 2025-10-22

**Author:** Bartosz Sobocki

**Status:**  Review

---

# 1. Project Overview

## 1.1 Purpose

**Learning Goals:** (if applicable)
- advanced C++,
- optimization,
- specification,
- architecture,
- interface/API specification,
- implementation design

**Programming Goals:**
- implementation of production-quality tool.

**Product Goal:**
- handle all provided real-world CSV files
- work fast and efficient

## 1.2 Scope

### Must Have (MVP)

- ✅ Parse RFC 4180 compliant CSVs
- ✅ Row-by-row iteration with STL-compatible iterators
- ✅ Streaming mode (constant memory usage) - read row by row from file
- ✅ Access field by index within record (row)
- ✅ Access field by column name within record (row)
- ✅ Basic type conversion (string, int, double, bool)
- ✅ Clear error messages with line/column information
- ✅ Custom delimiters support
- ✅ Handle quoted expressions with embedded commas, newlines, escaped quotes in fields
- ✅ Configurable (option as constructor argument) header position (-1 for no_header)
- ✅ Support Linux OS
- ✅ Support for row and column where errors occurs with suggestion ("if this is not the root cause - check quotes that ends here or delimiters")

### Should Have (Phase 2)

- ✅ Support access by column name
- ✅ Additional reading mode - in-memory mode (full load, enables random access as static key-value database)
- ✅ Memory-mapped file I/O for performance
- ✅ Column-wise access (iterate entire column)
- ✅ Configurable error handling (strict/lenient/callback)
- ✅ Custom type converters via template specialization
- ✅ Settings and restrictions
- ✅ Schema validation with custom rules (string in standard constructor or from file in special constructor or function that loads schema from file)
- ✅ Statistics during parsing (min/max/count per column)
- ✅ Warining queue (as additional info return by getter function)
- ✅ Different line ending support (LF, CRLF, CR)
- ✅ on-reading filtering and data manipulation - using callback functions

### Could Have (Future)

- ⚪ Support Windows OS
- ⚪ Expanded syntax by restrictions in file header (column names)
- ⚪ Write CSV support (export data)
- ⚪ Compressed file support (gzip)
- ⚪ Multi-threaded parsing (chunk-based parallelism)
- ⚪ Alternative quote characters (configurable)
- ⚪ Unicode support (UTF-8, UTF-16)
- ⚪ Real-time streaming (network sources)
- ⚪ JSON/XML conversion
- ⚪ on-reading filtering and data manipulation - expanded with special syntax parsing

### Won't Have (Out of Scope)

- ❌ Excel binary format support (.xls, .xlsx)
- ❌ GUI or command-line tool (library only)
- ❌ Database integration (export to SQL)

## 1.3 Constraints

### Time & Resources

- **Timeline:** 3 weeks (23.10.2025 - 13.11.2025)
- **Phase breakdown:**
    - Phase 1: 2 Weeks
    - Phase 2: 1 Week

### Memory Usage (Phase 1)

- Streaming mode: O(1) - single row in memory
- Target: Parse 10GB file with <100MB RAM

### Memory Usage (Phase 2)

- InMemory mode: O(n) - entire file in memory
- Seekable mode: O(1) - single row in memory

### Platform

**Primary:** Linux - Ubuntu (checking on WSL)

**Compiler:** GCC 11+

**Standard:** C++20 (concepts, ranges, string_view)

### Dependencies

**Build System:** CMake 3.20+

**Testing:**
- Google Test (gtest)
- Google Mock (gmock)
- Google Benchmark (optional, Phase 2)

**External Libraries:**
- MVP - only standard
- Boost.Iostreams for memory mapping (or platform API)

### Performance Targets (TODO: fill it after first benchmarks - as a plan for improvement)

- **Metric 1:** 
- **Metric 2:** 

### Code Quality

- **Test Coverage:** 90% for parser logic
- **Documentation:** API & Usage Documentation


# 2. Domain Specification

## 2.1 Base Standard: RFC 4180

### Key Rules:

1. Each record (row) on separate line
2. Fields in record separated by delimiter
3. Each record has to have the same number of fields (the same number as header)
4. Optional header row
5. Quoted fields for special values

### Settings *(to constant update/expanding)*:

**Description**: Data structure that user can prepare and put into csvengine via CSVReader constructor.

**Structure**: A key-value map with properties names as keys and their values. Every property is optional.

**Purpose**: This data structure will provide properties/settings like:
- error handling
- restrictions
- [Phase 2] data schema (types for every column)
- [Phase 2] data reading mode

**Phase 1**: Config Class:
```cpp
struct CSVConfig {
    char delimiter = ',';
    bool has_header = true;
    enum class ErrorHandlerMode { strict, lenient };
    ErrorHandlerMode errorHandlerMode = ErrorHandlerMode::strict;
    enum class LineEnding { Auto, LF, CRLF, CR }
};
```
**Phase 2**:
```cpp
struct CSVConfig {
    /* Phase 1 fields */
    std::optional<Schema> dataSchema;

    enum class DataReadingMode { streaming, inMemoryDatabase };
    DataReadingMode dataReadingMode = DataReadingMode::streaming;

    using ErrorHandlerCallback = std::function<void(size_t lineNum, const ParseError&)>;
    ErrorHandlerCallback onError;
};
```
**Future**:
```cpp
struct CSVConfig {
    /* Phase 2 fields */
    std::string schemaFilePath;
};
```

### On-read Filtering

**Description**: With this feature data can be filtered by using filtering options (Phase 2+). User will be able to define filtering function (or rule in the future) for each row.
**Dependencies**: Filtering can be done only when schema is known and validated - without clear types we cannot compare or check data.
**Input**: User can use filtering by creating and injecting:
- filtering callback function for records filtering
```cpp
CSVReader reader("data.csv")
    .withFilter([](const Row& row) {
        auto age = row.get<int>("age");
        auto name = row.get<std::string>("name");
        bool validData = age && name && name.length() > 0;
        return validData && *age > 25 && name.front() == 'A';
    });

for (const auto& row : reader) {
    // Only rows passing filter
}
```
- filtering callback function for fields (for specific column with data type check if function returns true)
- [future] filtering rule string (Can be done in future as an extension) like sql "where name = 'Ala' and age < 50" or special simpler notation, like "[= #name 'Ala'] && [< #age 50]" etc.
```cpp
// Future: parse expression string
reader.filter_by_expression("age > 25 AND name != 'Bob'");
// or simpler notation
reader.filter("[= #name 'Ala'] && [< #age 50]");
```

### On-read Data Manipulation

**Description**: With this feature data can be manipulated by using manipulating function (Phase 2+). User will be able to define manipulating function for each row.

**Output**: This function needs to return valid record that fulfill schema.

**Dependencies**: Data manipulation can be done only when schema is known and validated - without clear types we cannot be sure that changed data will be valid.

**Input**: User can use data manipulation by creating and injecting:
- manipulating callback function for records manipulation
- manipulating callback function for fields (for specific column with specific data type cheange value and return a new record with changed value in that field)

## 2.2 Format Components

### 2.2.1 Table

**Definition**: All records in one structure.

**Representation**: Data in ordered table structure.

**Termination**: EOF

**Structure:**
```csv
[Header Row (optional)]  # -- defines columns names
[Data Row 1]
[Data Row 2]
[Data Row 3]
```

### 2.2.2 Record (Row)

**Definition**: Single line of data in Table

**Representation**: Sequence of fields

**Termination**: New line character [EOL] or end of file [EOF] (The last row can terminate with table termination)

**Structure**:
```csv
[Field 1],[Field 1],[Field 1],[Field 1],[Field 1][EOL]
```

### 2.2.3 Field (Cell)

**Definition**: Single data value in Record

**Representation**: Sequence of characters between delimiters or end of record

**Types**: Quoted or unquoted


### 2.2.4 Delimiter

**Definition**: Character separating fields (in the future it can be a sequence of characters).

**Common Values**: `,` `;` `\t` `|`

**Limitations**: Whitespaces except tab (`\t`) are not allowed, because it could break structure and engine mechanisms (like restrictions separated from names using spaces or newlines as separator between records).

**Occurence**: The same number of delimiters must occur in each record between fields and not at start/end

### 2.2.5 Quote Characters

**Definition**: Characters describing where quoting value starts and ends

**Standard**: Double quotes `""`

**Purpose**: Allow special characters in fields

### 2.2.6 Line Eding

**Definition**: Characters separating records

**Types**: `\n` (LF), `\r\n` (CRLF), `\r` (CR)

## 2.3 Format Ambiguities

### Quotes in the middle of field

If quoting characters occurs inside field but not at start and end then they are treated as literal characters.
It will simplify parsing rules and help organize data (if user needs to quote then they should do it properly in organized way).
It can be changed in the future.

### Empty fields

An empty field is a field that doesn't contain any value between two delimiters or a delimiter and the end of the record.
Empty fields are allowed and respected if there is no restrictions set.

### Empty Rows

An empty row is treated as an additional space and it is skipped.
It doesn't break table structure, so no error needs to be raised (optionally a warning can be added into warninng queue - Phase 2).

### Different Field Counts

Every row must have the same amount of fields, otherwise an error will be thrown (Phase 1).
In Phase 2 it can be expanded by setting an additional behaviour: skip line + add warning into queue - if user is aware of broken records.
The number of fields are defined by:
- number of columns in header
- number of fields in the first record (if header is not available)
- in settings (Phase 2) 

### Whitespaced

Spaces inside quotes will be read as they are, but spaces around delimiters will be removed.

## 2.4 Parsing Protocol

### High-level process

**Input**: Text stream (sile, string, buffer)

**Output**: Sequence of records (rows), each record containing fields

**Steps**:
1. **Buffering:** The engine will maintain an internal fixed-size buffer (e.g., `4KB` or `64KB`). It will read chunks from `std::ifstream` into this buffer to reduce system calls. The parser iterates over this buffer (Phase 1). Memory mapping will be introduced for optimization in Phase 2+.
2. If first line is a header: store as a column names - defined by user (num of line or -1 for no_header option) with line 0 as default (Phase 1) + detected (all strings in the first line, but second line has differen data types - Phase 2+)
3. Split line into fields ("respecting quotes")
4. Convert fields to requested types
5. Return row to user
6. Repeat until EOF

**Challenge1:** Cannot use `std::getline` because of quotes (newline may appear inside a quote).

**Idea:** If we get into quoting mode before newline then we read until the end of the quote with treating everything as literals. Only newlines outside quoting are valid as record close.

**Challenge2:** Quoting inside quotes.

**Idea:** User should use double quote for quoting.

**Solution:** Unified read-parse loop with character-by-character state machine.

**State Machine:**
- `FieldStart`: Beginning of field
- `Unquoted`: Reading normal characters
- `Quoted`: Inside quotes (ignore delimiters, newlines)
- `QuoteEscape`: Saw `"` inside quotes (check for `""`)

**Key Rule:** Only return row when closing quote is followed by 
delimiter/newline, not when `\n` appears inside quotes.


## 2.5 Key Decisions

**Decision: [Quote Handling]**
- **Context:** What happens when quotes appear mid-field?
- **Decision:** Treat as literals unless at field start

**Decision: [Empty Row Behavior]**
- **Context:** How to handle blank lines in CSV?
- **Decision:** Skip silently (Phase 1), warn optionally (Phase 2)

**Decision: [Field Count Mismatch]**
- **Context:** Row has wrong number of fields
- **Decision:** Error on wrong number of fields (silent)

**Decision: [Memory Strategy]**
- **Context:** Streaming vs in-memory
- **Decision:** Streaming-only for Phase 1, database mode for Phase 2

**Decision: [Delimiter Restrictions]**
- **Context:** Which delimiters to allow?
- **Decision:** No whitespace except `\t`

**Decision: [Reading Strategy]**
- **Context:** 
- **Decision:** Phase 1: read chunk-by-chunk, Phase 2+: memory mapping for limited IO calls/operations

**Decision: [Data Ownership vs Zero-Copy]**
- **Context:** Should CSVRecord own the strings (safe) or view them (fast)?
- **Conflict:** "Zero-copy" requires views, but easy API requires ownership.
- **Decision:**
    - **Phase 1:** CSVRecord will own data (`std::vector<std::string>`). "Zero-copy" applies only to internal parsing (parsing the line to find delimiters without creating substrings), but the final record is copied.
    - **Phase 2:** Introduce a specialized CSVView class that is strictly zero-copy (`std::vector<std::string_view>`) for advanced users who understand lifetime management.

**Decision: [Type Conversion Strategy]**
- **Context:** How to convert string fields to numbers (int, double) efficiently?
- **Options:**
  - A) `std::stringstream` (Slow, heavy overhead)
  - B) `std::stoi`/`std::stod` (Throws exceptions, requires `std::string` allocation, locale dependent)
  - C) `std::from_chars` (C++17/20, non-allocating, works on `string_view`, locale independent, fastest)
- **Decision:** Option C (`std::from_chars`)
- **Rationale:** Since we target C++20, we should use the most efficient standard tool. It allows us to parse numbers directly from the internal string storage without creating temporary string copies.

**Decision: Multi-Pass Iterator Behavior**
- **Context:** What happens when begin() is called after iteration completes?
- **Options:**
  - A) Throw exception on second begin()
  - B) Return end() silently (input iterator semantics)
  - C) Cache all rows in memory for multi-pass iteration
  - D) Seek file back to start and re-parse on each begin()
- **Decision:** 
  - **Phase 1:** Option B (return end() silently)
    - Matches std::istream_iterator behavior
    - Simple, predictable, efficient
    - Users collect to std::vector for multi-pass needs
  - **Phase 2:** Add configurable ReadMode with three strategies:
    - `ReadMode::Streaming` (default) - Option B behavior
    - `ReadMode::InMemory` - Option C (load once, iterate many times)
    - `ReadMode::Seekable` - Option D (re-parse on each begin())
- **Rationale:**
  - **Phase 1:** Start with standard C++ input iterator pattern
    - No surprises for experienced C++ developers
    - Minimal implementation complexity
    - Optimal for large files (constant memory)
  - **Phase 2:** Provide flexibility for different use cases:
    - `Streaming`: Large files, single analysis pass, minimal RAM
    - `InMemory`: Small-medium files, multiple passes, fast random access
    - `Seekable`: Medium files, multiple passes, moderate memory
- **Trade-offs:**
  | Mode | Memory | Speed | Multi-Pass | Random Access |
  |------|--------|-------|------------|---------------|
  | Streaming | O(1) | Fast | ❌ | ❌ |
  | InMemory | O(n) | Fastest | ✅ | ✅ |
  | Seekable | O(1) | Slow (re-parse) | ✅ | ❌ |
  

## 2.6 Future Extensions

### Restrictions:

**MVP Limitation**: In phase 1 csvenginge doesn't provide any mechanism for respecting and managing restrictions.

**Description**: Restriction mechanism (Phase 2+) will provide a more database-like workflow for values in CSV files.

**Accessibility**: Adding restrictions will be available via:
- properties data structure as constructor argument
- setter functions or additional functions
- [future] adnotations in column names (options to choose before implementation):
    - highlited by `${}` in column names within header
    - specific syntax with types, like `#SCHEMA: id:int:not_null, age:int:positive, name:string` in header

**Restrictions**:
- not_null
- positives_only
- not_zero

**Example**:
```csv
id ${not_null},age${positives_only},...
```


# 3. Use Cases

## UC1: Read Simple CSV Without Headers

**Scenario:** Parse temperature readings with no header row

**Actor:** Data scientist analyzing sensor logs

**Example Code:**
```cpp
CSVReader reader("temps.csv", CSVConfig{.has_header = false});
for (const auto& row : reader) {
    auto timestamp = row.get<std::string>(0);
    auto temp = row.get<double>(1);

    if (temp && *temp > 100.0) {
        std::cout << "Alert at " << timestamp << "\n";
    }
}
```

**Requirements:**
- Must work without headers
- Must handle missing/invalid numbers gracefully
- Must iterate lazily (streaming)

## UC2: Access Columns by Name

**Scenario:** Process employee CSV with known schema

**Actor:** HR system importing new hires

**Example Code:**
```cpp
CSVReader reader("employees.csv"); // has_header defaults to true

for (const auto& row : reader) {
    auto name = row.getstd::string("name");
    auto salary = row.get<int>("salary");
    auto start_date = row.getstd::string("start_date");

    if (name && salary && *salary > 50000) {
        processHighEarner(*name, *salary);
    }
}
```

**Requirements:**
- Must map column names to indices
- Must validate column names exist
- Must provide clear errors if column missing

## UC3: Handle Quoted Fields with Commas

**Scenario:** Parse addresses with embedded commas

**Actor:** Mailing system

**Example Code:**
```cpp
// Input file:
// name,address
// John,"123 Main St, Apt 4"
// Jane,"456 Elm St, Suite 200"

CSVReader reader("addresses.csv");
for (const auto& row : reader) {
    auto name = row.getstd::string("name");
    auto addr = row.getstd::string("address");

// addr should be: "123 Main St, Apt 4" (comma preserved)
}
```

**Requirements:**
- Must parse quoted fields correctly
- Must handle `""` as escaped quote
- Must preserve whitespace inside quotes

## UC4: Custom Delimiter (TSV)

**Scenario:** Parse tab-separated log file

**Actor:** Log analyzer

**Example Code:**
```cpp
CSVReader reader("access.log", CSVConfig{
    .delimiter = '\t',
    .has_header = false
});

for (const auto& row : reader) {
    auto ip = row.get<std::string>(0);
    auto timestamp = row.get<std::string>(1);
    auto endpoint = row.get<std::string>(2);
    // ...
}
```

**Requirements:**
- Must support any single-char delimiter
- Must handle tabs in quoted fields

## UC5: Error Handling with Line Numbers

**Scenario:** Debug malformed CSV

**Actor:** Developer troubleshooting import

**Example Code:**
```cpp
try {
    CSVReader reader("broken.csv");
    for (const auto& row : reader) {
    // ...
    }
} catch (const CSVParseError& e) {
    std::cerr << "Error at line " << e.line()
    << ", column " << e.column()
    << ": " << e.what() << "\n";
    // Output: "Error at line 42, column 7: Unclosed quote"
}
```

**Requirements:**
- Must track current line/column during parsing
- Must provide actionable error messages
- Must suggest fixes (e.g., "check for missing closing quote")

## UC6: Empty Fields

**Scenario:** Parse CSV with missing values

**Example Code:**
```cpp
// Input: name,age,city
// Alice,30,NYC
// Bob,,LA
// Carol,25,

CSVReader reader("data.csv");
for (const auto& row : reader) {
    auto name = row.getstd::string("name");
    auto age = row.get<int>("age"); // may be nullopt
    auto city = row.getstd::string("city");

    std::cout << *name << " is ";
    if (age) {
        std::cout << *age;
    } else {
        std::cout << "unknown age";
    }
    std::cout << "\n";
}
```

**Requirements:**
- Empty fields return `std::nullopt` for typed access
- Empty fields return `""` for string access
- Must distinguish between empty and malformed

## UC7: Large File Streaming

**Scenario:** Process 10GB CSV without loading into memory

**Example Code:**
```cpp
CSVReader reader("huge.csv"); // Only reads current row

size_t count = 0;
for (const auto& row : reader) {
    ++count;
    // Memory usage stays constant
}

std::cout << "Processed " << count << " rows\n";
```

**Requirements:**
- Must use constant memory (O(1) for row size)
- Must not load entire file
- Must work with files larger than RAM

## UC8: Multi-Pass Iteration (Phase 2)

**Scenario:** Analyze CSV twice without reloading

**Actor:** Data analyst computing statistics

**Example Code:**
```cpp
CSVReader reader("sales.csv", CSVConfig{
    .read_mode = ReadMode::InMemory
});

// First pass: compute total
double total = 0;
for (const auto& row : reader) {
    total += row.get<double>("amount").value_or(0);
}

// Second pass: compute percentage
for (const auto& row : reader) { // Works!
    double amount = row.get<double>("amount").value_or(0);
    double percent = (amount / total) * 100;
    std::cout << row["product"] << ": " << percent << "%\n";
}
```

**Requirements:**
- Must support multiple begin() calls
- Must not re-parse file (performance)
- Must maintain row order across passes
   

# 4. Public API Design

## 4.1 Core Classes

**Class: [CSVReader]**

**Purpose:** Entry point for parsing CSV files with streaming iteration

**Responsibilities:**
- Open file and manage I/O
- Handle Settings
- Tokenize rows respecting quotes/delimiters
- Provide iterator interface
- Track line/column for errors
- **Collaborators:** CSVConfig, CSVRecord, CSVParseError, CSVReadError

**Design:**
```cpp
class CSVReader {
public:
    // Construction & Configuration
    explicit CSVReader(const std::string& filePath, const CSVConfig = {});

    // No copy (owns file handle)
    CSVReader(const CSVReader&) = delete;
    CSVReader& operator=(const CSVReader&) = delete;
    // Move-only
    CSVReader(CSVReader&&) noexcept = default;
    CSVReader& operator=(CSVReader&&) noexcept = default;

    bool good() const;
    bool hasHeader() const;
    int currentLineNum () const; // currentRecordIndex + 1
    explicit operator bool() const; // return good()

    // getters
    CSVConfig getConfig() const;
    CSVRecord currentRecord() const;
    std::optional<CSVRecord> nextRecord();
    const std::vector<std::string>& headers() const;
    
    class CSVIterator;
    CSVIterator begin();
    CSVIterator end(); 

private:
    friend class CSVIterator;

    CSVRecord currentRecord;
    int currentRecordIndex = -1;

    std::ifstream csvFile;
    const CSVConfig config;
    std::vector<std::string> columnNames;
};
```

**Class: [CSVRecord]**

**Purpose:** A record that stores fields as std::string and convert on demand

**Responsibilities:**
- stores fields of current record
- random access of fields
- access to field by its column name (only when CSVReader exists or schema is provided)

**Design:**
```cpp
class CSVRecord {
public:
    using ColumnIndexMap = std::unordered_map<std::string, size_t>;

    explicit CSVRecord(const size_t fieldCount);
    CSVRecord(const std::vector<std::string> fields,
             std::shared_ptr<const ColumnIndexMap> columnMap = nullptr);

    CSVRecord(const CSVRecord&) = default;
    CSVRecord(CSVRecord&&) noexcept = default;
    CSVRecord& operator=(const CSVRecord&) = default;
    CSVRecord& operator=(CSVRecord&&) noexcept = default;

    template<typename T = std::string>
    std::optional<T> get(const size_t index) const;
    template<typename T = std::string>
    std::optional<T> get(const std::string& columnName) const;

    std::string_view operator[](size_t index) const;
    std::string_view operator[](std::string_view column) const;

    size_t size() const;

private:
    template<typename T>
    std::optional<T> convert(const std::string& value) const;

    std::vector<std::string> fields; // fields as strings
    std::shared_ptr<const ColumnIndexMap> columnMap; // for mapping name to index - more memory, but O(1) lookup
};
```


## 4.2 Usage Examples

**Example 1: Basic Usage**
```cpp
// Most common scenario
```

**Example 2: Advanced Usage**
```cpp
// Complex scenario
```

**Example 3: Error Handling**
```cpp
// How errors are handled
```

---

# 5. Architecture

## 5.1 Component Overview

[ASCII diagram or description of major components]

```
[User Code]
     ↓
[Public API]
     ↓
[Core Logic] ←→ [Utilities]
     ↓
[I/O Layer]
```

## 5.2 Component Details

**Component: [CSVIterator]**

**Responsibility:** Iterates through csv data only forward.

**Dependencies:** CSVReader state.

**Design:**
```cpp
class CSVReader {
// ...

public:
    class CSVIterator {
    public:
        const CSVRecord& operator*() const;
        const CSVRecord* operator->() const;

        // forward only
        Iterator& operator++();     // pre-increment ++iter
        Iterator operator++(int);  // post-increment iter++

        bool operator==(const Iterator&) const;
        bool operator==(Iterator&&) const;
        bool operator!=(const Iterator&) const;
        bool operator!=(Iterator&&) const;

    private:
        friend class CSVReader;

        CSVIterator(CSVReader*, bool);

        CSVReader* csvReader; // non-const pointer for calling csvReader->nextRecord()
        CSVRecord currentRecord; // for save dereferencing
        bool isEnd;
    }

// ...
}
```

[Repeat for each component]

## 5.3 Data Flow

[Describe how data moves through the system]

**Input → Processing → Output**

---

# 6. Key Design Decisions

## Decision 1: [Name]

**Problem:** 

**Considered:**
- Option A: ...
- Option B: ...

**Chosen:** 

**Rationale:** 

**Implications:** 

**Revisit Trigger:** 

## Decision 2: [Name]
...

[Document 5-10 most important decisions]

---

# 7. Implementation Plan

## Phase 1: [Name] (Timeline)

**Goal:**

**Success Criteria:** 

### Milestone 1.1: [Name] (Time)

**Tasks:**
- [ ] Task 1
- [ ] Task 2

**Tests:**
```cpp
TEST(...) { }
```
**Deliverable:** 
**Done When:** 

### Milestone 1.2: [Name]
...

## Phase 2: [Name]
...

## Phase 3: [Name]
...

---

# 8. Testing Strategy

## 8.1 Unit Tests

**Coverage:** 

**Focus Areas:**
- ...
- ...

**Example Tests:**
```cpp
TEST(Component, Behavior) { }
```

## 8.2 Integration Tests

**What:** 

**How:** 

## 8.3 Performance Tests

**Benchmarks:**
- ...

**Acceptance:**
- ...

## 8.4 Test Data

**Sources:**
- ...

**Edge Cases:**
- ...


# 9. Success Criteria

**MVP Complete When:**
- [ ] Criterion 1
- [ ] Criterion 2
- [ ] Criterion 3

**Project Complete When:**
- [ ] Criterion 1
- [ ] Criterion 2

---

# 10. Open Questions

- [ ] Question 1
- [ ] Question 2

[Update as you resolve questions]

---

# 11. References

- [Link to relevant standards/docs]
- [Link to similar projects]
- [Link to research/articles]

---

# Appendix A: Glossary

**Term:** Definition

---

# Appendix B: Change Log

**v1.1 (Date):**
- CHANGED: ...
- REASON: ...
- IMPACT: ...

**v1.0 (Date):**
- Initial specification
