#include <iostream>
#include <cstring>
#include <csvparser.hpp>

namespace csv {

SimpleParser::SimpleParser(const Config& config): Parser(config) {}

std::vector<std::string_view> SimpleParser::split(std::string_view str, const char delim) const {
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

ParseStatus SimpleParser::parse(std::string_view buffer) {
    consumed_ = 0;

    auto newline_pos = buffer.find('\n');

    if (newline_pos == std::string_view::npos) {
        if (!buffer.empty()) {
            auto fields = split(buffer, config_.delimiter);
            insert_fields(fields);
            consumed_ = buffer.size();
        }
        
        incomplete_last_read_ = true;
        return ParseStatus::need_more_data;
    }

    auto line = buffer.substr(0, newline_pos);

    if (line.empty()) {
        consumed_ = 1;
        incomplete_last_read_ = false;
        return ParseStatus::complete;
    }

    auto fields = split(line, config_.delimiter);
    insert_fields(fields);
    consumed_ += line.size()  + 1; // + newline character

    incomplete_last_read_ = false;
    return ParseStatus::complete;
}

void SimpleParser::insert_fields(const std::vector<std::string_view>& fields) {
    auto field = fields.begin();
    if (incomplete_last_read_) {
        fields_[fields_.size()-1] += *field++;
    }
    while(field != fields.end()) {
        fields_.emplace_back(*field++);
    }
}

}