#include <csvparser.hpp>

namespace {
    constexpr char quoting_char = '\"';
}

CsvParser::CsvParser(CsvConfig config): config_(config) {   
}

CsvParser::ParseStatus CsvParser::parse(std::string_view buffer) {
    return CsvParser::ParseStatus::fail;
}

void CsvParser::reset() {
    in_quotes_ = false;
    fields_ = {};
    field_start_ = 0;
    consumed_ = 0;
    err_msg_ = "";
}

size_t CsvParser::consumed() const {
    return consumed_;
}

std::string CsvParser::err_msg() const {
    return err_msg_;
}

std::vector<std::string_view> CsvParser::fields() const {
    return fields_;
}

std::vector<std::string> CsvParser::fields_copy() const {
    return std::vector<std::string>(fields_.begin(), fields_.end());
}