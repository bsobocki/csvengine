#pragma once

#include <stdexcept>
#include <string>

namespace csv {

class BufferError : public std::runtime_error {
public:
    BufferError(): std::runtime_error("Buffer is in bad state during initialization") {}
};

class FileHeaderError : public std::runtime_error {
public:
    FileHeaderError(): std::runtime_error("There is a problem with reading headers, that should be available according to config!") {}
}; 

}