#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <iostream>
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
    std::size_t currentLineNum () const; // currentRecordIndex + 1
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

    CSVRecord _currentRecord;
    int _currentRecordIndex = -1;

    std::ifstream _csvFile;
    const CSVConfig _config;
    std::vector<std::string> _columnNames;
};