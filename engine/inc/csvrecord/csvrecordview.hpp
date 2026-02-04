#pragma once

#include <csverrors.hpp>

#include <memory>
#include <optional>
#include <vector>
#include <charconv>
#include <type_traits>
#include <sstream>
#include <algorithm>

namespace csv {

class RecordView {
public:
    RecordView()=default;
    RecordView(const RecordView&) = default;
    RecordView(RecordView&&) noexcept = default;
    RecordView& operator=(const RecordView&) = default;
    RecordView& operator=(RecordView&&) noexcept = default;

    explicit RecordView(const std::vector<std::string_view>& fields, const std::vector<std::string_view>& headers = {})
        :fields_(fields), headers_(headers) {}

    explicit RecordView(std::vector<std::string_view>&& fields, std::vector<std::string_view>&& headers = {})
        :fields_(std::move(fields)), headers_(std::move(headers)) {}

    explicit RecordView(const std::vector<std::string>& fields, const std::vector<std::string>& headers = {})
        :fields_(fields.begin(), fields.end())
        ,headers_(headers.begin(), headers.end()) {}

    explicit RecordView(std::vector<std::string>&& fields, std::vector<std::string>&& headers = {})
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
    std::optional<T> get(std::string_view column_name) const {
        auto it = std::find(headers_.begin(), headers_.end(), column_name);
        if (it == headers_.end()) {
            return std::nullopt;
        }
        return get<T>(std::distance(headers_.begin(), it));
    }

    const std::string_view& at(size_t index) const {
        if (index >= fields_.size()) {
            throw std::out_of_range("Field index out of range");
        }
        return fields_[index];
    }

    const std::string_view& at(std::string_view column_name) const {
        auto it = std::find(headers_.begin(), headers_.end(), column_name);
        if (it == headers_.end()) {
            throw RecordColumnNameError(column_name);
        }
        return fields_[std::distance(headers_.begin(), it)];
    }

    const std::string_view& operator[](size_t index) const noexcept {
        return fields_[index];
    }

    const std::string_view& operator[](std::string_view column_name) const {
        return at(column_name);
    }

    const std::vector<std::string_view>& fields() const noexcept {
        return fields_;
    }

    const std::vector<std::string_view>& headers() const noexcept {
        return headers_;
    }

    size_t size() const noexcept {
        return fields_.size();
    }

    bool empty() const noexcept {
        return fields_.empty();
    }

    bool has_headers() const noexcept {
        return !headers_.empty();
    }

private:
    template<typename T>
    std::optional<T> convert(const std::string_view& str) const {
        if constexpr (std::is_convertible_v<const std::string_view&, T>) {
            return str;
        }
        else if constexpr (std::is_convertible_v<const std::string&, T>) {
            return std::string(str.begin(), str.end());
        }
        else if constexpr (std::is_arithmetic_v<T>) {
            return convert_arithmetic<T>(str);
        }
        else {
            T value;
            std::istringstream iss(std::string(str.begin(), str.end()));
            if (iss >> value && iss.eof()) {
                return value;
            }
            return std::nullopt;
        }
    }

    template<typename T>
    static std::optional<T> convert_arithmetic(const std::string_view& str) {
        if (str.empty()) {
            return std::nullopt;
        }

        T value;
        const char* first = str.data();
        const char* last = str.data() + str.size();

        // skip leading spaces
        while (first != last && std::isspace(static_cast<unsigned char>(*first))) {
            ++first;
        }

        auto [conversion_end, error_code] = std::from_chars(first, last, value);

        if (error_code != std::errc{}) {
            return std::nullopt;
        }

        // move through trailing spaces
        while (conversion_end != last && std::isspace(static_cast<unsigned char>(*conversion_end))) {
            conversion_end++;
        }

        // if there are more chars that are not whitespace then the value is not a number
        return (conversion_end == last) ? std::optional<T>(value) : std::nullopt;
    }

    std::vector<std::string_view> fields_;
    std::vector<std::string_view> headers_;
};

}