#pragma once

#include <unordered_map>
#include <memory>
#include <optional>
#include <vector>

class CsvRecord {
public:
    using ColumnIndexMap = std::unordered_map<std::string, size_t>;

    CsvRecord()=default;
    explicit CsvRecord(const std::vector<std::string>& fields, const std::vector<std::string>& headers = {});

    CsvRecord(const CsvRecord&) = default;
    CsvRecord(CsvRecord&&) noexcept = default;
    CsvRecord& operator=(const CsvRecord&) = default;
    CsvRecord& operator=(CsvRecord&&) noexcept = default;

    template<typename T = std::string>
    std::optional<T> get(const size_t index) const;

    template<typename T = std::string>
    std::optional<T> get(const std::string& columnName) const;

    std::string_view operator[](size_t index) const;
    std::string_view operator[](std::string_view column) const;

    std::vector<std::string> fields() const;
    std::vector<std::string> headers() const;

    size_t size() const;

private:
    template<typename T>
    std::optional<T> convert(const std::string& value) const;

    std::vector<std::string> fields_; // fields as strings
    std::vector<std::string> headers_; // for mapping name to index - more memory, but O(1) lookup
};