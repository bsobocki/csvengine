#pragma once

#include <unordered_map>
#include <memory>
#include <optional>
#include <vector>

namespace csv {

class Record {
public:
    Record()=default;
    explicit Record(const std::vector<std::string>& fields, const std::vector<std::string>& headers = {});
    explicit Record(std::vector<std::string>&& fields, std::vector<std::string>&& headers = {});
    explicit Record(const std::vector<std::string_view>& fields, const std::vector<std::string_view>& headers = {});
    explicit Record(std::vector<std::string_view>&& fields, std::vector<std::string>&& headers = {});

    Record(const Record&) = default;
    Record(Record&&) noexcept = default;
    Record& operator=(const Record&) = default;
    Record& operator=(Record&&) noexcept = default;

    template<typename T = std::string>
    std::optional<T> get(const size_t index) const;

    template<typename T = std::string>
    std::optional<T> get(const std::string_view& column_name) const;

    std::string_view operator[](size_t index) const;
    std::string_view operator[](std::string_view column) const;

    const std::vector<std::string>& fields() const;
    const std::vector<std::string>& headers() const;

    size_t size() const;

private:
    template<typename T>
    std::optional<T> convert(const std::string_view& value) const;
    std::size_t column_to_index(const std::string_view& column_name) const;

    std::vector<std::string> fields_;
    std::vector<std::string> headers_;
};

// TODO: separate class for Record with only std::string_view
using RecordView = Record;

}