#pragma once

#include "csvparserbase.hpp"

namespace csv {

template <typename FieldType>
class QuotingParser : public Parser<FieldType> {
public:
    explicit QuotingParser(const Config& config);

    void reset() noexcept override;

protected:
    bool is_quote(char c);
    bool is_delim(char c);
    bool is_newline(char c);

    bool in_quotes_ = false;
    bool pending_quote_ = false;
};


class StrictQuotingParser : public QuotingParser<std::string> {
public:
    explicit StrictQuotingParser(const Config& config);

    /// @brief function parse doesn't reset the parser state
    [[nodiscard]] ParseStatus parse(std::string_view buffer) override;

protected:
    void remove_last_char_from_fields() override;
};


class LenientQuotingParser : public QuotingParser<std::string> {
public:
    explicit LenientQuotingParser(const Config& config);

    /// @brief function parse doesn't reset the parser state
    [[nodiscard]] ParseStatus parse(std::string_view buffer) override;

protected:
    void remove_last_char_from_fields() override;
};

}