#pragma once

#include <cstddef>
#include <string_view>
#include <memory>
#include <csvbuffer/csvbuffer.hpp>

namespace csv {

class MappedBuffer : public IBuffer {
    public:
        explicit MappedBuffer(std::string_view filename);
        ~MappedBuffer();

        MappedBuffer(const MappedBuffer&) = delete;
        MappedBuffer& operator=(const MappedBuffer&) = delete;

        MappedBuffer(MappedBuffer&&) noexcept;
        MappedBuffer& operator=(MappedBuffer&&) noexcept;

        ReadingResult refill() override;
        std::string_view view() const noexcept override;
        void consume(size_t bytes) noexcept override;
        size_t available() const noexcept override;
        size_t capacity() const noexcept override;
        bool empty() const noexcept override;
        bool eof() const noexcept override;
        bool good() const noexcept override;
        bool reset() override;

    private:
        size_t start_ = 0;
        size_t size_ = 0;
        char* data_ = nullptr;
};

inline std::unique_ptr<IBuffer> make_mapped_buffer(std::string_view filename) {
    return std::make_unique<MappedBuffer>(filename);
}

}