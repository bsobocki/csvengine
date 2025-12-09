#pragma once

#include <string>
#include <csvconfig.hpp>

class CsvParser {
public:
    enum class ParseStatus {record, eob, fail};

    explicit CsvParser(CsvConfig config);

    // function parse doesn't reset the parser state
    ParseStatus parse(std::string_view buffer);

    size_t consumed() const;

    std::string err_msg() const;

    // reset parser state
    void reset();

private:
    CsvConfig config_;
    bool in_quotes_ = false;
    size_t field_start_ = 0;
    size_t consumed_ = 0;
    std::string err_msg_ = "";
};