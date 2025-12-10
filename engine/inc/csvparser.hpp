#pragma once

#include <string>
#include <csvconfig.hpp>
#include <vector>
#include <string_view>

class CsvParser {
public:
    enum class ParseStatus {record, eob, fail};

    explicit CsvParser(CsvConfig config);

    // function parse doesn't reset the parser state
    ParseStatus parse(std::string_view buffer);

    size_t consumed() const;

    std::string err_msg() const;

    std::vector<std::string_view> fields() const;
    std::vector<std::string> fields_copy() const;

    // reset parser state
    void reset();

private:
    ParseStatus naive_parse(std::string_view buffer);
    ParseStatus csv_quotes_parse(std::string_view buffer);

    CsvConfig config_;
    bool in_quotes_ = false;
    std::vector<std::string_view> fields_ = {};
    size_t field_start_ = 0;
    size_t consumed_ = 0;
    std::string err_msg_ = "";
};