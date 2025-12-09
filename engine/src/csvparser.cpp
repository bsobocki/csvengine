#include <csvparser.hpp>

namespace {
constexpr char quoting_char = '\"';

std::vector<std::string_view> split(std::string_view str, const char delim) {
    std::vector<std::string_view> result;

    size_t start = 0;
    size_t end = str.find(delim);

    while(end != std::string_view::npos) {
        result.push_back(str.substr(start, end - start));
        start = end + 1;
        end = str.find(delim, start);
    }

    result.push_back(str.substr(start));

    return result;
}
}

CsvParser::CsvParser(CsvConfig config): config_(config) {   
}

CsvParser::ParseStatus CsvParser::parse(std::string_view buffer) {
    // TODO: implement parsing with quotes

    // naive implementation without quotes
    auto newline_pos = buffer.find('\n');

    if (newline_pos == std::string_view::npos) {
        if (buffer.empty()) return CsvParser::ParseStatus::eob;

        for (auto& field: split(buffer, config_.delimiter))
            fields_.push_back(field);
        return CsvParser::ParseStatus::record;
    }

    auto line = buffer.substr(0, newline_pos);

    if (line.empty()) {
        return CsvParser::ParseStatus::record;
    }

    for (auto& field: split(line, config_.delimiter))
        fields_.push_back(field);
    return CsvParser::ParseStatus::record;
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