#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <iostream>
#include <memory>
#include <iterator>
#include <optional>
#include <functional>

#include <csvconfig.hpp>
#include <csvrecord.hpp>
#include <csvbuffer.hpp>
#include <csvparser.hpp>

namespace csv {

template<typename Func>
concept RecordViewBoolCallback = 
    std::invocable<Func, const RecordView&> &&
    std::convertible_to<std::invoke_result_t<Func, const RecordView&>, bool> &&
    (!std::same_as<std::invoke_result_t<Func, const RecordView&>, void>);

template<typename Func>
concept RecordViewVoidCallback = 
    std::invocable<Func, const RecordView&> &&
    std::same_as<std::invoke_result_t<Func, const RecordView&>, void>;

class Reader {
public:
    // Construction & Configuration
    explicit Reader(const std::string& filePath, const Config = {});
    explicit Reader(std::unique_ptr<std::istream> stream, const Config = {});
    explicit Reader(std::unique_ptr<IBuffer> buffer, const Config = {});

    // No copy (owns file handle)
    Reader(const Reader&) = delete;
    Reader& operator=(const Reader&) = delete;
    // Move-only
    Reader(Reader&&) noexcept = default;
    Reader& operator=(Reader&&) noexcept = default;

    bool good() const;
    bool has_header() const;
    std::size_t line_number() const;
    std::size_t record_size() const;
    explicit operator bool() const;

    // getters
    Config config() const;
    const Record& current_record() const;
    [[nodiscard]] bool next();
    const std::vector<std::string>& headers() const;

    template<RecordViewBoolCallback Func>
    void for_each(Func&& iteration);

    template<RecordViewVoidCallback Func>
    void for_each(Func&& iteration);

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

    friend class Iterator;

    Record current_record_;
    long long current_record_idx_ = 0;
    long long record_size_ = -1;

    std::string csv_file_path_;
    std::unique_ptr<IBuffer> buffer_;
    const Config config_;
    Parser parser_;
    std::vector<std::string> headers_;
};

}