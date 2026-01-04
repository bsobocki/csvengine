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

constexpr size_t DEFAULT_CAPACITY = 65536; // 64 KB chunk

class IBuffer {
public:
    virtual ~IBuffer() = default;

    virtual ReadingResult refill() = 0;
    virtual std::string_view view() const = 0;
    virtual void consume(size_t bytes) = 0;
    virtual size_t available() const = 0;
    virtual bool empty() const = 0;
    virtual bool eof() const = 0;
    virtual bool good() const = 0;
    virtual bool reset() = 0;
};

template <size_t N = DEFAULT_CAPACITY>
class Buffer : public IBuffer {
    public:
        explicit Buffer(std::string_view filename)
            : stream_(std::make_unique<std::ifstream>(std::string(filename), std::ios::binary))
            , data_(std::make_unique<char[]>(N))
        {
            if (!stream_->good()) {
                throw FileStreamError(filename);
            }
        }

        // for testing
        explicit Buffer(std::unique_ptr<std::istream> stream)
            : stream_(std::move(stream))
            , data_(std::make_unique<char[]>(N))
        {

            if (!stream_->good()) {
                throw FileStreamError();
            }
        }

        Buffer(const Buffer&) = delete;
        Buffer& operator=(const Buffer&) = delete;

        Buffer(Buffer&&) noexcept = default;
        Buffer& operator=(Buffer&&) noexcept = default;

        ReadingResult refill() override {
            compact();

            size_t space = free_space();
            if (space == 0) {
                return ReadingResult::buffer_full;
            }

            stream_->read(data_.get() + size_, space);

            size_t bytes_read = static_cast<size_t>(stream_->gcount());

            if (bytes_read == 0)
                return stream_->eof() ? ReadingResult::eof : ReadingResult::fail;

            size_ += bytes_read;

            return ReadingResult::ok;
        }

        std::string_view view() const noexcept override {
            return  {data_.get() + start_, available()};
        }

        void consume(size_t bytes) noexcept override {
            start_ += std::min(bytes, available());
        }

        size_t available() const noexcept override {
            return size_ - start_;
        }

        bool empty() const noexcept override {
            return start_ == size_;
        }

        bool eof() const noexcept override {
            return available() == 0 && stream_->eof();
        }

        bool good() const noexcept override {
            return stream_->good() || !empty();
        }

        bool reset() override {
            stream_->clear();
            stream_->seekg(0);
            start_ = 0;
            size_ = 0;
            return stream_->good();
        }

    private:
        void compact() noexcept {
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

template<size_t N = DEFAULT_CAPACITY>
std::unique_ptr<IBuffer> make_buffer(std::string_view filename) {
    return std::make_unique<Buffer<N>>(filename);
}

template<size_t N = DEFAULT_CAPACITY>
std::unique_ptr<IBuffer> make_buffer(std::unique_ptr<std::istream> stream) {
    return std::make_unique<Buffer<N>>(std::move(stream));
}

}