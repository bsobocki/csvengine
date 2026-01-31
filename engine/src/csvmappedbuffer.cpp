#include <csvbuffer/csvmappedbuffer.hpp>
#include <algorithm>
#include <string>
#include <stdexcept>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

namespace csv {

MappedBuffer::MappedBuffer(std::string_view filename) {
    std::string safe_name(filename);

    int fd = open(safe_name.c_str(), O_RDONLY);
    if (fd == -1) {
        throw std::runtime_error("Could not open file: " + safe_name);
    }

    struct stat stat_buff;
    if (fstat(fd, &stat_buff) == -1) {
        close(fd);
        throw std::runtime_error("Could not get the file size.");
    }

    size_ = static_cast<size_t>(stat_buff.st_size);

    if (size_ == 0) {
        close(fd);
        data_ = nullptr;
        return;
    }

    void* addr = mmap(nullptr, size_, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);

    if (addr == MAP_FAILED) {
        throw std::runtime_error("File memory mapping failed.");
    }

    madvise(addr, size_, MADV_SEQUENTIAL);

    data_ = static_cast<char*>(addr);
}

MappedBuffer::~MappedBuffer() {
    if (data_) {
        munmap(data_, size_);
    }
}

MappedBuffer::MappedBuffer(MappedBuffer&& other) noexcept {
    *this = std::move(other);
}

MappedBuffer& MappedBuffer::operator=(MappedBuffer&& other) noexcept {
    if (this != &other) {
        if (data_) munmap(data_, size_);

        start_ = other.start_;
        size_ = other.size_;
        data_ = other.data_;

        other.data_ = nullptr;
        other.size_ = 0;
        other.start_ = 0;
    }
    return *this;
}

ReadingResult MappedBuffer::refill() {
    if (!data_) return ReadingResult::fail;

    if (start_ >= size_) {
        return ReadingResult::eof;
    }

    return ReadingResult::ok;
}

std::string_view MappedBuffer::view() const noexcept {
    if (!data_) return {};
    return {data_ + start_, available()};
}

void MappedBuffer::consume(size_t bytes) noexcept {
    start_ += std::min(bytes, available());
}

size_t MappedBuffer::available() const noexcept {
    return size_ - start_;
}

bool MappedBuffer::empty() const noexcept {
    return start_ >= size_;
}

bool MappedBuffer::eof() const noexcept {
    return empty();
}

bool MappedBuffer::good() const noexcept {
    return data_ != nullptr && !eof();
}

bool MappedBuffer::reset() {
    start_ = 0;
    return (data_ != nullptr);
}

}
