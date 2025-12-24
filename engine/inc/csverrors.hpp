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
    FileStreamError(): std::runtime_error(msg) {}
    FileStreamError(const std::string& filename): std::runtime_error(msg + " for file: " + filename) {}
    FileStreamError(const std::string_view& filename): std::runtime_error(msg + " for file: " + std::string(filename)) {}
private:
    std::string msg = "Stream failed";
}; 

}