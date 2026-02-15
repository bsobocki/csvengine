#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <memory>
#include <iterator>
#include <optional>
#include <functional>

#include <csvconfig.hpp>
#include <csvrecord/csvrecord.hpp>
#include <csvrecord/csvrecordview.hpp>
#include <csvbuffer/csvbuffer.hpp>
#include <csvparser.hpp>

namespace csv {

template <typename RecordType>
class ReaderBase {
protected:
    explicit ReaderBase(const std::string& filePath, const Config& config = {});
    explicit ReaderBase(std::unique_ptr<std::istream> stream, const Config& config = {});
    explicit ReaderBase(std::unique_ptr<IBuffer> buffer, const Config& config = {});

public:
    bool good() const noexcept;
    bool has_header() const noexcept;
    std::size_t line_number() const noexcept;
    std::size_t record_size() const noexcept;
    explicit operator bool() const noexcept;

    Config config() const noexcept;
    const RecordType& current_record() const noexcept;
    const std::vector<std::string>& headers() const noexcept;
    
    virtual bool next() = 0;

    class Iterator {
        public:
            // Iterator Traits (Required for STL compatibility)
            using iterator_category = std::input_iterator_tag;
            using value_type        = RecordType;
            using difference_type   = std::ptrdiff_t;
            using pointer           = const RecordType*;
            using reference         = const RecordType&;

            explicit Iterator(ReaderBase* reader);
            Iterator& operator++();
            const RecordType& operator*() const;
            bool operator!=(const Iterator& other) const;

        private:
            ReaderBase* reader_;
    };
    Iterator begin();
    Iterator end();

protected:
    void init();
    void read_headers();
    void validate_config() const;
    void create_buffer(const std::string& filepath);
    size_t expected_record_size(size_t record_size) const noexcept;

    friend class Iterator;

    RecordType current_record_;
    size_t line_number_ = 0;
    size_t record_size_ = 0;

    std::string csv_file_path_;
    std::unique_ptr<IBuffer> buffer_;
    const Config config_;
    std::vector<std::string> headers_;
};

class Reader : public ReaderBase<Record> {
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

    [[nodiscard]] bool next() override;

private:
    std::unique_ptr<Parser> parser_;
};

class ViewReader : public ReaderBase<RecordView> {
public:
    // Construction & Configuration
    explicit ViewReader(const std::string& filePath, const Config& config = {});
    explicit ViewReader(std::unique_ptr<std::istream> stream, const Config& config = {});
    explicit ViewReader(std::unique_ptr<IBuffer> buffer, const Config& config = {});

    // No copy (owns file handle)
    ViewReader(const ViewReader&) noexcept = delete;
    ViewReader& operator=(const ViewReader&) noexcept = delete;
    // Move-only
    ViewReader(ViewReader&&) noexcept = default;
    ViewReader& operator=(ViewReader&&) noexcept = default;

    [[nodiscard]] bool next() override;

private:
    std::unique_ptr<ViewSimpleParser> parser_;
};

}