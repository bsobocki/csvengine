#pragma once

#include <string>
#include <memory>
#include <fstream>
#include <cstddef>
#include <string_view>
#include <cstring>
#include <stdexcept>

namespace csv {

constexpr size_t DEFAULT_CAPACITY = 65536; // 64 KB chunk

template <size_t N = DEFAULT_CAPACITY>
class Buffer {
    public:
        explicit Buffer(std::string_view filename)
            : stream_(std::make_unique<std::ifstream>(std::string(filename), std::ios::binary))
            , data_(std::make_unique<char[]>(N))
        {
            if (!stream_->good()) {
                throw std::runtime_error("Failed to open file: " + std::string(filename));
            }
        }

        // for testing
        explicit Buffer(std::unique_ptr<std::istream> stream)
            : stream_(std::move(stream))
            , data_(std::make_unique<char[]>(N)) {}

        Buffer(const Buffer&) = delete;
        Buffer& operator=(const Buffer&) = delete;

        Buffer(Buffer&&) noexcept = default;
        Buffer& operator=(Buffer&&) noexcept = default;

        enum class ReadingResult {ok, eof, buffer_full, fail};

        ReadingResult refill() {
            compact();

            size_t space = free_space();
            if (space == 0) {
                return ReadingResult::buffer_full;
            }

            stream_->read(data_.get() + size_, space);

            auto bytes_read = stream_->gcount();

            if (bytes_read == 0)
                return stream_->eof() ? ReadingResult::eof : ReadingResult::fail;

            size_ += bytes_read;

            return ReadingResult::ok;
        }

        std::string_view view() const {
            return  {data_.get() + start_, available()};
        }

        void consume(size_t bytes) {
            start_ += std::min(bytes, available());
        }

        size_t available() const {
            return size_ - start_;
        }

        bool empty() const {
            return start_ == size_;
        }

        bool eof() const {
            return available() == 0 && stream_->eof();
        }

        bool good() const {
            return stream_->good() || !empty();
        }

        void reset() {
            stream_->clear();
            stream_->seekg(0);
            start_ = 0;
            size_ = 0;
        }

    private:
        void compact() {
            size_t leftover = available();

            // move leftover data to the beggining
            if (leftover && start_) {
                std::memmove(data_.get(), data_.get() + start_, leftover);
            }

            start_ = 0;
            size_ = leftover;
        }

        size_t free_space() const {
            return N - size_;
        }

        std::unique_ptr<std::istream> stream_;
        std::unique_ptr<char[]> data_;
        size_t start_ = 0;
        size_t size_ = 0;
};

}