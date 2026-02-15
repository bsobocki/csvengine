#pragma once

#include "csvparserbase.hpp"

namespace csv {

class SimpleParser : public Parser {
public:
    explicit SimpleParser(const Config& config);

    /// @brief function parse doesn't reset the parser state
    [[nodiscard]] ParseStatus parse(std::string_view buffer) override;

private:
    void insert_fields(const std::vector<std::string_view>& fields);
    std::vector<std::string_view> split(std::string_view str, const char delim) const;
};


class ViewSimpleParser : public Parser {
public:
    explicit ViewSimpleParser(const Config& config);

    /// @brief function parse doesn't reset the parser state
    [[nodiscard]] ParseStatus parse(std::string_view buffer) override;

    void shift_views(const char* buffer_start);
    const std::vector<std::string_view>& fields() const noexcept;
    void reset() noexcept override;

private:
    void insert_fields(const std::vector<std::string_view>& fields);
    std::vector<std::string_view> split(std::string_view str, const char delim) const;

    std::vector<std::string_view> fields_;
};

}