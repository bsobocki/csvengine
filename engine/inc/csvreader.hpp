#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <iostream>
#include <memory>
#include <iterator>
#include <optional>
#include <csvconfig.hpp>
#include <csvrecord.hpp>
#include <csvbuffer.hpp>
#include <functional>

template<typename Func>
concept CsvRecordViewBoolCallback = 
    std::invocable<Func, const CsvRecordView&> &&
    std::convertible_to<std::invoke_result_t<Func, const CsvRecordView&>, bool> &&
    (!std::same_as<std::invoke_result_t<Func, const CsvRecordView&>, void>);

template<typename Func>
concept CsvRecordViewVoidCallback = 
    std::invocable<Func, const CsvRecordView&> &&
    std::same_as<std::invoke_result_t<Func, const CsvRecordView&>, void>;

class CsvReader {
public:
    // Construction & Configuration
    explicit CsvReader(const std::string& filePath, const CsvConfig = {});
    explicit CsvReader(std::unique_ptr<std::istream> stream, const CsvConfig = {});

    // No copy (owns file handle)
    CsvReader(const CsvReader&) = delete;
    CsvReader& operator=(const CsvReader&) = delete;
    // Move-only
    CsvReader(CsvReader&&) noexcept = default;
    CsvReader& operator=(CsvReader&&) noexcept = default;

    bool good() const;
    bool has_header() const;
    std::size_t line_number () const; // currentRecordIndex + 1
    explicit operator bool() const; // return good()

    // getters
    CsvConfig config() const;
    const CsvRecord& current_record() const;
    bool read_next_record();
    const std::vector<std::string>& headers() const;

    template<CsvRecordViewBoolCallback Func>
    void for_each(Func&& iteration);

    template<CsvRecordViewVoidCallback Func>
    void for_each(Func&& iteration);

    class CsvIterator {
        public:
            // Iterator Traits (Required for STL compatibility)
            using iterator_category = std::input_iterator_tag;
            using value_type        = CsvRecord;
            using difference_type   = std::ptrdiff_t;
            using pointer           = const CsvRecord*;
            using reference         = const CsvRecord&;

            explicit CsvIterator(CsvReader* reader);
            CsvIterator& operator++();
            const CsvRecord& operator*() const;
            bool operator!=(const CsvIterator& other) const;
        
        private:
            CsvReader* reader_;
    };
    CsvIterator begin();
    CsvIterator end();

private:
    void read_headers();

    friend class CsvIterator;

    CsvRecord current_record_;
    long long current_record_idx_ = -1;

    std::string csv_file_path_;
    CsvBuffer<> buffer_;
    const CsvConfig config_;
    std::vector<std::string> headers_;
};