#pragma once

#include <string>
#include <vector>
#include <string_view>
#include <memory>

#include <csvconfig.hpp>

namespace csv {

enum class ParseStatus {complete, need_more_data, fail};

class Parser {
public:
    explicit Parser(const Config& config);

    /// @brief function parse doesn't reset the parser state
    [[nodiscard]] virtual ParseStatus parse(std::string_view buffer) = 0;

    [[nodiscard]] size_t consumed() const;

    [[nodiscard]] std::string err_msg() const;

    std::vector<std::string> move_fields();
    std::vector<std::string_view> peek_fields();

    virtual void reset();

protected:
    Config config_;
    std::string err_msg_;
    std::vector<std::string> fields_;

    bool incomplete_last_read_ = false;
    size_t consumed_ = 0;
};



class SimpleParser : public Parser {
public:
    explicit SimpleParser(const Config& config);

    /// @brief function parse doesn't reset the parser state
    [[nodiscard]] ParseStatus parse(std::string_view buffer) override;
    
private:
    void insert_fields(const std::vector<std::string_view>& fields);
    std::vector<std::string_view> split(std::string_view str, const char delim) const;
};



class QuotingParser : public Parser {
public:
    explicit QuotingParser(const Config& config);

    void reset() override;

protected:
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

std::unique_ptr<Parser> make_parser(const Config& config);

}