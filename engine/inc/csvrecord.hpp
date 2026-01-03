#pragma once

#include <memory>
#include <optional>
#include <vector>
#include <charconv>
#include <type_traits>
#include <sstream>
#include <algorithm>

namespace csv {

class Record {
public:
    Record()=default;
    Record(const Record&) = default;
    Record(Record&&) noexcept = default;
    Record& operator=(const Record&) = default;
    Record& operator=(Record&&) noexcept = default;

    explicit Record(const std::vector<std::string>& fields, const std::vector<std::string>& headers = {})
        :fields_(fields), headers_(headers) {}

    explicit Record(std::vector<std::string>&& fields, std::vector<std::string>&& headers = {})
        :fields_(std::move(fields)), headers_(std::move(headers)) {}

    explicit Record(const std::vector<std::string_view>& fields, const std::vector<std::string_view>& headers = {})
        :fields_(fields.begin(), fields.end())
        ,headers_(headers.begin(), headers.end()) {}

    explicit Record(std::vector<std::string_view>&& fields, std::vector<std::string>&& headers = {})
        :fields_(fields.begin(), fields.end())
        ,headers_(headers.begin(), headers.end()) {}

    template<typename T = std::string>
    std::optional<T> get(const size_t index) const {
        if (index >= fields_.size()) {
            return std::nullopt;
        }
        return convert<T>(fields_[index]);
    }

    template<typename T = std::string>
    std::optional<T> get(const std::string& column_name) const {
        auto it = std::find(headers_.begin(), headers_.end(), column_name);
        if (it == headers_.end()) {
            return std::nullopt;
        }
        return get<T>(std::distance(headers_.begin(), it));
    }

    std::string_view operator[](size_t index) const {
        if (index >= fields_.size()) {
            return {};
        }
        return std::string_view(fields_[index].begin(), fields_[index].end());
    }

    std::string_view operator[](const std::string& column_name) const {
        auto it = std::find(headers_.begin(), headers_.end(), column_name);
        if (it == headers_.end()) {
            return {};
        }
        return operator[](std::distance(headers_.begin(), it));
    }

    const std::vector<std::string>& fields() const {
        return fields_;
    }

    const std::vector<std::string>& headers() const {
        return headers_;
    }

    size_t size() const {
        return fields_.size();
    }

private:
    template<typename T>
    std::optional<T> convert(const std::string& str) const {
        if constexpr (std::is_same_v<T, std::string>) {
            return str;
        }
        else if constexpr (std::is_arithmetic_v<T>) {
            return convert_arithmetic<T>(str);
        }
        else {
            T value;
            std::istringstream iss(str);
            if (iss >> value && iss.eof()) {
                return value;
            }
            return std::nullopt;
        }
    }

    template<typename T>
    static std::optional<T> convert_arithmetic(const std::string& str) {
        if (str.empty()) {
            return std::nullopt;
        }

        T value;
        const char* first = str.data();
        const char* last = str.data() + str.size();

        // skip trailing spaces
        while (first != last && std::isspace(*first)) {
            ++first;
        }

        auto [conversion_end, error_code] = std::from_chars(first, last, value);

        if (error_code != std::errc{}) {
            return std::nullopt;
        }

        if (conversion_end == last) {
            // all chars consumed
            return value;
        }

        // move through whitespaces
        while (conversion_end != last && std::isspace(*conversion_end)) {
            conversion_end++;
        }

        if (conversion_end == last) {
            // there is no more non-whitespace chars
            return value;
        }

        return std::nullopt;
    }

    std::size_t column_to_index(const std::string& column_name) const;

    std::vector<std::string> fields_;
    std::vector<std::string> headers_;
};

// TODO: separate class for Record with only std::string_view
using RecordView = Record;

}