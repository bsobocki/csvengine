#pragma once

#include <string>
#include <memory>
#include <fstream>
#include <cstddef>
#include <string_view>
#include <cstring>
#include <stdexcept>

#include <csverrors.hpp>

namespace csv {

enum class ReadingResult { ok, eof, buffer_full, fail };

// 2KB chosen to optimize for L1 Cache locality (See BENCHMARKING.md)
constexpr size_t DEFAULT_CAPACITY = 2048;

class IBuffer {
public:
    virtual ~IBuffer() = default;

    virtual ReadingResult refill() = 0;
    virtual std::string_view view() const = 0;
    virtual void consume(size_t bytes) = 0;
    virtual size_t available() const = 0;
    virtual size_t capacity() const = 0;
    virtual bool empty() const = 0;
    virtual bool eof() const = 0;
    virtual bool good() const = 0;
    virtual bool reset() = 0;
};

}