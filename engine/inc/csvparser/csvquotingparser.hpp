#pragma once

#include "csvparserbase.hpp"

namespace csv {

class QuotingParser : public Parser {
public:
    explicit QuotingParser(const Config& config);

    void reset() noexcept override;
    
protected:
    void remove_last_saved_char();
    bool is_quote(char c);
    bool is_delim(char c);
    bool is_newline(char c);

    bool in_quotes_ = false;
    bool pending_quote_ = false;
};


class StrictQuotingParser : public QuotingParser {
public:
    explicit StrictQuotingParser(const Config& config);

    /// @brief function parse doesn't reset the parser state
    [[nodiscard]] ParseStatus parse(std::string_view buffer) override;
};


class LenientQuotingParser : public QuotingParser {
public:
    explicit LenientQuotingParser(const Config& config);

    /// @brief function parse doesn't reset the parser state
    [[nodiscard]] ParseStatus parse(std::string_view buffer) override;
};

}