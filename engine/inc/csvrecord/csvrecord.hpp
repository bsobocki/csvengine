#pragma once

#include <csverrors.hpp>

#include <memory>
#include <optional>
#include <vector>
#include <unordered_map>
#include <charconv>
#include <type_traits>
#include <sstream>
#include <algorithm>

namespace csv {

template <typename FieldType>
class RecordBase {
public:
    RecordBase()=default;
    RecordBase(const RecordBase&) = default;
    RecordBase(RecordBase&&) noexcept = default;
    RecordBase& operator=(const RecordBase&) = default;
    RecordBase& operator=(RecordBase&&) noexcept = default;

    explicit RecordBase(const std::vector<std::string>& fields, const std::vector<std::string>& headers = {})
        :fields_(fields) {
            init_headers(headers);
        }
        
    explicit RecordBase(std::vector<std::string>&& fields, std::vector<std::string>&& headers = {})
        :fields_(std::move(fields)) {
            init_headers(headers);
        }

    explicit RecordBase(const std::vector<std::string_view>& fields, const std::vector<std::string>& headers = {})
        :fields_(fields.begin(), fields.end()) {
            init_headers(headers);
        }

    explicit RecordBase(std::vector<std::string_view>&& fields, std::vector<std::string>&& headers = {})
        :fields_(fields.begin(), fields.end()) {
            init_headers(headers);
        }
    
    void init_headers(const std::vector<std::string>& headers) {
        for(size_t i = 0; i < headers.size(); i++) {
            headers_[headers[i]] = i;
        }
    }

    template<typename T = FieldType>
    std::optional<T> get(const size_t index) const {
        if (index >= fields_.size()) {
            return std::nullopt;
        }
        return convert<T>(fields_[index]);
    }

    template<typename T = FieldType>
    std::optional<T> get(const std::string& column_name) const {
        if (headers_.contains(column_name)) {
            return get<T>(headers_.at(column_name));
        }
        return std::nullopt;
    }

    const FieldType& at(size_t index) const {
        if (index >= fields_.size()) {
            throw std::out_of_range("Field index out of range");
        }
        return fields_[index];
    }

    const FieldType& at(const std::string& column_name) const {
        if (headers_.contains(column_name)) {
            return at(headers_.at(column_name));
        }
        throw RecordColumnNameError(column_name);
    }

    const FieldType& operator[](size_t index) const noexcept {
        return fields_[index];
    }

    const FieldType& operator[](const std::string& column_name) const {
        return at(column_name);
    }

    const std::vector<FieldType>& fields() const noexcept {
        return fields_;
    }

    const std::unordered_map<std::string, size_t>& headers() const noexcept {
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
    std::optional<T> convert(const std::string_view str) const {
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
    static std::optional<T> convert_arithmetic(const std::string_view str) {
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

    std::vector<FieldType> fields_;
    std::unordered_map<std::string, size_t> headers_;
};

using Record = RecordBase<std::string>;
using RecordView = RecordBase<std::string_view>;

}