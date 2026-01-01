#pragma once

#include <stdexcept>
#include <string>
#include <string_view>

namespace csv {

class BufferError : public std::runtime_error {
public:
    BufferError(): std::runtime_error("Buffer is in bad state during initialization") {}
};

class FileHeaderError : public std::runtime_error {
public:
    FileHeaderError(): std::runtime_error("There is a problem with reading headers, that should be available according to config!") {}
}; 

class FileStreamError : public std::runtime_error {
public:
    FileStreamError(): std::runtime_error("Stream failed") {}
    FileStreamError(const std::string& filename): std::runtime_error("Stream failed for file: " + filename) {}
    FileStreamError(const std::string_view& filename): std::runtime_error("Stream failed for file: " + std::string(filename)) {}
    FileStreamError(const std::string& filename, size_t line_number)
        : std::runtime_error("Stream failed for file: " + filename  + " at line " + std::to_string(line_number)) {}
    FileStreamError(const std::string_view& filename, size_t line_number)
        : std::runtime_error("Stream failed for file: " + std::string(filename) + " at line " + std::to_string(line_number)) {}
};

class RecordDataError : public std::runtime_error {
public:
    RecordDataError(): std::runtime_error("Cannot read record.") {}
    RecordDataError(size_t line_number): std::runtime_error("Cannot read record at line " + std::to_string(line_number)) {}
};

class RecordSizeError : public std::runtime_error {
public:
    RecordSizeError(): std::runtime_error("Unexpected record size.") {}
    RecordSizeError(size_t line_number, size_t expected_size, size_t actual_size)
        : std::runtime_error(
            "Record size mismatch: expected " + std::to_string(expected_size) +
            ", got " + std::to_string(actual_size) +
            " at line " + std::to_string(line_number)) {}
};

}