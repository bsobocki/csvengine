#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <iostream>
#include <memory>
#include <iterator>
#include <optional>
#include <csvconfig.hpp>
#include <csvrecord.hpp>

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
    std::size_t lineNumber () const; // currentRecordIndex + 1
    explicit operator bool() const; // return good()

    // getters
    CSVConfig getConfig() const;
    const CSVRecord& currentRecord() const;
    std::optional<CSVRecord> nextRecord();
    const std::vector<std::string>& headers() const;

    class CSVIterator {
        public:
            // Iterator Traits (Required for STL compatibility)
            using iterator_category = std::input_iterator_tag;
            using value_type        = CSVRecord;
            using difference_type   = std::ptrdiff_t;
            using pointer           = const CSVRecord*;
            using reference         = const CSVRecord&;

            explicit CSVIterator(CSVReader* reader);
            CSVIterator& operator++();
            const CSVRecord& operator*() const;
            bool operator!=(const CSVIterator& other) const;
        
        private:
            CSVReader* _reader;
    };
    CSVIterator begin();
    CSVIterator end(); 

private:
    friend class CSVIterator;

    CSVRecord _currentRecord;
    int _currentRecordIndex = -1;

    std::ifstream _csvFile;
    const CSVConfig _config;
    std::vector<std::string> _headers;
};