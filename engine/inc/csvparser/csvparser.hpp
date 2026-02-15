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
    virtual ~Parser() = default;

    /// @brief function parse doesn't reset the parser state
    [[nodiscard]] virtual ParseStatus parse(std::string_view buffer) = 0;

    [[nodiscard]] size_t consumed() const noexcept;
    [[nodiscard]] std::string_view err_msg() const noexcept;
    std::vector<std::string> move_fields() noexcept;
    const std::vector<std::string>& peek_fields() const noexcept;
    const std::vector<std::string_view> fields_view() const noexcept;

    virtual void reset() noexcept;

protected:
    const Config config_;
    std::string err_msg_;
    std::vector<std::string> fields_;

    bool pending_cr_ = false;
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



class SimdParser : public Parser {
public:
    explicit SimdParser(const Config& config);

    /// @brief function parse doesn't reset the parser state
    [[nodiscard]] ParseStatus parse(std::string_view buffer) override;
};



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

std::unique_ptr<Parser> make_parser(const Config& config);

}