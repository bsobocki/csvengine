#pragma once

#include "csvparserbase.hpp"

namespace csv {

class SimpleParserBase : public Parser {
public:
    /// @brief function parse doesn't reset the parser state
    [[nodiscard]] ParseStatus parse(std::string_view buffer) override;

protected:
    explicit SimpleParserBase(const Config& config);

    void insert_fields(const std::vector<std::string_view>& fields);
    std::vector<std::string_view> split(std::string_view str, const char delim) const;
    virtual bool has_fields() const = 0;

    virtual void merge_incomplete_field(const std::string_view& field) = 0;
    virtual void add_field(const std::string_view& field) = 0;
    virtual void remove_last_char_from_fields() = 0;
};

class SimpleParser : public SimpleParserBase {
public:
    explicit SimpleParser(const Config& config);

private:
    void merge_incomplete_field(const std::string_view& field) override;
    void add_field(const std::string_view& field) override;
    void remove_last_char_from_fields() override;
    bool has_fields() const;
};


class ViewSimpleParser : public SimpleParserBase {
public:
    explicit ViewSimpleParser(const Config& config);

    void shift_views(const char* buffer_start);
    const std::vector<std::string_view>& fields() const noexcept;
    bool has_fields() const;
    void reset() noexcept override;

private:
    void merge_incomplete_field(const std::string_view& field) override;
    void add_field(const std::string_view& field) override;
    void remove_last_char_from_fields() override;

    std::vector<std::string_view> fields_;
};

}