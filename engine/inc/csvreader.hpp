#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <memory>
#include <iterator>
#include <optional>
#include <functional>

#include <csvconfig.hpp>
#include <csvrecord.hpp>
#include <csvbuffer.hpp>
#include <csvparser.hpp>

namespace csv {

class Reader {
public:
    // Construction & Configuration
    explicit Reader(const std::string& filePath, const Config& config = {});
    explicit Reader(std::unique_ptr<std::istream> stream, const Config& config = {});
    explicit Reader(std::unique_ptr<IBuffer> buffer, const Config& config = {});

    // No copy (owns file handle)
    Reader(const Reader&) noexcept = delete;
    Reader& operator=(const Reader&) noexcept = delete;
    // Move-only
    Reader(Reader&&) noexcept = default;
    Reader& operator=(Reader&&) noexcept = default;

    bool good() const noexcept;
    bool has_header() const noexcept;
    std::size_t line_number() const noexcept;
    std::size_t record_size() const noexcept;
    explicit operator bool() const noexcept;

    Config config() const noexcept;
    const Record& current_record() const noexcept;
    const std::vector<std::string>& headers() const noexcept;
    
    [[nodiscard]] bool next();

    class Iterator {
        public:
            // Iterator Traits (Required for STL compatibility)
            using iterator_category = std::input_iterator_tag;
            using value_type        = Record;
            using difference_type   = std::ptrdiff_t;
            using pointer           = const Record*;
            using reference         = const Record&;

            explicit Iterator(Reader* reader);
            Iterator& operator++();
            const Record& operator*() const;
            bool operator!=(const Iterator& other) const;
        
        private:
            Reader* reader_;
    };
    Iterator begin();
    Iterator end();

private:
    void read_headers();
    void init();
    void validate_config() const;
    size_t expected_record_size(size_t record_size) const noexcept;

    friend class Iterator;

    Record current_record_;
    size_t line_number_ = 0;
    size_t record_size_ = 0;

    std::string csv_file_path_;
    std::unique_ptr<IBuffer> buffer_;
    const Config config_;
    std::unique_ptr<Parser> parser_;
    std::vector<std::string> headers_;
};

}