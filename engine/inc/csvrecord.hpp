#pragma once

#include <unordered_map>
#include <memory>
#include <optional>
#include <vector>

class CSVRecord {
public:
    using ColumnIndexMap = std::unordered_map<std::string, size_t>;

    explicit CSVRecord(const size_t fieldCount);
    CSVRecord(const std::vector<std::string> fields,
             std::shared_ptr<const ColumnIndexMap> columnMap = nullptr);

    CSVRecord(const CSVRecord&) = default;
    CSVRecord(CSVRecord&&) noexcept = default;
    CSVRecord& operator=(const CSVRecord&) = default;
    CSVRecord& operator=(CSVRecord&&) noexcept = default;

    template<typename T = std::string>
    std::optional<T> get(const size_t index) const;

    template<typename T = std::string>
    std::optional<T> get(const std::string& columnName) const;

    std::string_view operator[](size_t index) const;
    std::string_view operator[](std::string_view column) const;

    size_t size() const;

private:
    template<typename T>
    std::optional<T> convert(const std::string& value) const;

    std::vector<std::string> fields; // fields as strings
    std::shared_ptr<const ColumnIndexMap> columnMap; // for mapping name to index - more memory, but O(1) lookup
};