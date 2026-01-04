#pragma once

#include <string>
#include <vector>
#include <string_view>

#include <csvconfig.hpp>

namespace csv {

class Parser {
public:
    enum class ParseStatus {complete, need_more_data, fail};

    explicit Parser(Config config);

    /// @brief function parse doesn't reset the parser state
    [[nodiscard]] ParseStatus parse(std::string_view buffer);

    [[nodiscard]] size_t consumed() const;

    [[nodiscard]] std::string err_msg() const;

    std::vector<std::string> move_fields();
    std::vector<std::string_view> peek_fields();

    void reset();

private:
    ParseStatus simple_parse(std::string_view buffer);
    ParseStatus csv_quotes_strict_parse(std::string_view buffer);
    ParseStatus csv_quotes_lenient_parse(std::string_view buffer);
    void insert_fields(const std::vector<std::string_view>& fields);

    Config config_;

    bool in_quotes_ = false;
    bool pending_quote_ = false;
    bool incomplete_last_read_ = false;

    std::vector<std::string> fields_;
    size_t consumed_ = 0;
    std::string err_msg_;
};

}