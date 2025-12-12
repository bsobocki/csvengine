#pragma once

#include <csvbuffer.hpp>
#include <string_view>

namespace csv {

template<size_t BufferSize>
class DataView{
public:
    DataView(const std::string& mem, BufferView<BufferSize> buffer)
    : mem_(mem), buffer_(buffer) {}

    // [0..k] -> mem
    // [k+1...n] -> buffer

private:
    std::string_view mem_;
    BufferView<BufferSize> buffer_;
};

}