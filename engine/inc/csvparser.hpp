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
    ParseStatus parse(std::string_view buffer);

    size_t consumed() const;

    std::string err_msg() const;

    std::vector<std::string> move_fields() const;

    void reset();

private:
    ParseStatus naive_parse(std::string_view buffer);
    ParseStatus csv_quotes_parse(std::string_view buffer);
    void insert_fields(const std::vector<std::string_view>& fields);

    Config config_;
    bool in_quotes_ = false;
    bool is_last_field_not_full_ = false;
    std::vector<std::string> fields_ = {};
    size_t field_start_ = 0;
    size_t consumed_ = 0;
    std::string err_msg_ = "";
};

}